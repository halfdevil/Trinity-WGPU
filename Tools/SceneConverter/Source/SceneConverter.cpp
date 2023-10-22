#include "SceneConverter.h"
#include "Scene/Node.h"
#include "Scene/Scene.h"
#include "Scene/ComponentFactory.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/Light.h"
#include "Scene/Components/Mesh.h"
#include "Scene/Components/SubMesh.h"
#include "Scene/Components/PerspectiveCamera.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/PBRMaterial.h"
#include "Graphics/Sampler.h"
#include "Graphics/Texture2D.h"
#include "Graphics/BindGroup.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/BindGroup.h"
#include "Graphics/BindGroupLayout.h"
#include "Graphics/Model.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"
#include "Core/Image.h"
#include "Core/Utils.h"
#include "Core/ResourceCache.h"
#include "VFS/FileSystem.h"
#include "VFS/DiskFile.h"
#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"
#include <format>
#include <queue>

#define KHR_LIGHTS_PUNCTUAL_EXTENSION "KHR_lights_punctual"

namespace Trinity
{
	bool fileExists(const std::string& fileName, void* userData)
	{
		auto& fileSystem = FileSystem::get();
		return fileSystem.isExist(fileName);
	}

	std::string expandFilePath(const std::string& filePath, void* userData)
	{
		return filePath;
	}

	bool readWholeFile(std::vector<unsigned char>* out, std::string* err, const std::string& filePath, void* userData)
	{
		auto file = FileSystem::get().openFile(filePath, FileOpenMode::OpenRead);
		if (!file)
		{
			*err = "FileSystem::openFile() failed";
			return false;
		}

		FileReader reader(*file);
		out->resize(file->getSize());
		reader.read(out->data(), file->getSize());

		return true;
	}

	bool writeWholeFile(std::string* err, const std::string& filePath, const std::vector<unsigned char>& contents, void* userData)
	{
		return true;
	}

	bool getFileSizeInBytes(size_t* sizeOut, std::string* err, const std::string& filePath, void* userData)
	{
		auto file = FileSystem::get().openFile(filePath, FileOpenMode::OpenRead);
		if (!file)
		{
			*err = "FileSystem::openFile() failed";
			return false;
		}

		*sizeOut = (size_t)file->getSize();
		return true;
	}

	wgpu::FilterMode findMinFilter(int minFilter)
	{
		switch (minFilter)
		{
		case TINYGLTF_TEXTURE_FILTER_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
			return wgpu::FilterMode::Nearest;

		case TINYGLTF_TEXTURE_FILTER_LINEAR:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
			return wgpu::FilterMode::Linear;

		default:
			return wgpu::FilterMode::Linear;
		}
	}

	wgpu::FilterMode findMagFilter(int magFilter)
	{
		switch (magFilter)
		{
		case TINYGLTF_TEXTURE_FILTER_NEAREST:
			return wgpu::FilterMode::Nearest;

		case TINYGLTF_TEXTURE_FILTER_LINEAR:
			return wgpu::FilterMode::Linear;

		default:
			return wgpu::FilterMode::Linear;
		}
	}

	wgpu::MipmapFilterMode findMipMapMode(int minFilter)
	{
		switch (minFilter)
		{
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
			return wgpu::MipmapFilterMode::Nearest;

		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
			return wgpu::MipmapFilterMode::Linear;

		default:
			return wgpu::MipmapFilterMode::Linear;
		}
	}

	wgpu::AddressMode findWrapMode(int wrap)
	{
		switch (wrap)
		{
		case TINYGLTF_TEXTURE_WRAP_REPEAT:
			return wgpu::AddressMode::Repeat;

		case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
			return wgpu::AddressMode::ClampToEdge;

		case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
			return wgpu::AddressMode::MirrorRepeat;

		default:
			return wgpu::AddressMode::Repeat;
		}
	}

	std::vector<uint8_t> getAttributeData(const tinygltf::Model& model, uint32_t accessorId)
	{
		auto& accessor = model.accessors[accessorId];
		auto& bufferView = model.bufferViews[accessor.bufferView];
		auto& buffer = model.buffers[bufferView.buffer];

		size_t stride = accessor.ByteStride(bufferView);
		size_t startByte = accessor.byteOffset + bufferView.byteOffset;
		size_t endByte = startByte + accessor.count * stride;

		return { buffer.data.begin() + startByte, buffer.data.begin() + endByte };
	}

	size_t getAttributeSize(const tinygltf::Model& model, uint32_t accessorId)
	{
		return model.accessors[accessorId].count;
	}

	size_t getAttributeStride(const tinygltf::Model& model, uint32_t accessorId)
	{
		auto& accessor = model.accessors[accessorId];
		auto& bufferView = model.bufferViews[accessor.bufferView];

		return accessor.ByteStride(bufferView);
	}

	wgpu::IndexFormat getIndexFormat(const tinygltf::Model& model, uint32_t accessorId)
	{
		auto& accessor = model.accessors[accessorId];
		switch (accessor.componentType)
		{
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			return wgpu::IndexFormat::Uint16;

		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
			return wgpu::IndexFormat::Uint32;
		}

		return wgpu::IndexFormat::Undefined;
	}

	std::vector<uint8_t> convertDataStride(const std::vector<uint8_t>& srcData, uint32_t srcStride, uint32_t dstStride)
	{
		auto indexCount = (uint32_t)srcData.size() / srcStride;
		std::vector<uint8_t> result(indexCount * dstStride);

		for (uint32_t idxSrc = 0, idxDst = 0;
			idxSrc < srcData.size() && idxDst < result.size();
			idxSrc += srcStride, idxDst += dstStride)
		{
			std::copy(srcData.begin() + idxSrc, srcData.begin() + idxSrc + srcStride, result.begin() + idxDst);
		}

		return result;
	}

	SceneConverter::~SceneConverter()
	{
	}

	void SceneConverter::setFileName(const std::string& fileName)
	{
		mFileName = fileName;
	}

	void SceneConverter::setOutputFileName(const std::string& fileName)
	{
		mOutputFileName = fileName;
	}

	void SceneConverter::execute()
	{
		auto& fileSystem = FileSystem::get();

		mResult = true;
		mShouldExit = true;

		if (!fileSystem.isExist(mFileName))
		{
			LogError("Input file doesn't exists: %s!!", mFileName.c_str());
			mResult = false;
			return;
		}
		
		auto file = fileSystem.openFile(mFileName, FileOpenMode::OpenRead);
		if (!file)
		{
			LogError("FileSystem::openFile() failed for: %s", mFileName.c_str());
			mResult = false;
			return;
		}

		fs::path dir{ mFileName };
		dir.remove_filename();

		FileReader reader{ *file };
		std::string source = reader.readAsString();

		std::string err;
		std::string warn;

		tinygltf::TinyGLTF gltfLoader;
		gltfLoader.SetFsCallbacks({
			.FileExists = &fileExists,
			.ExpandFilePath = &expandFilePath,
			.ReadWholeFile = &readWholeFile,
			.WriteWholeFile = &writeWholeFile,
			.GetFileSizeInBytes = &getFileSizeInBytes
		});
		
		gltfLoader.LoadASCIIFromString(&mModel, &err, &warn, source.c_str(), 
			(uint32_t)source.length(), dir.string());

		if (!err.empty())
		{
			LogError("Error loading GLTF file: '%s', with error: %s", mFileName.c_str(), err.c_str());
			mResult = false;
			return;
		}

		if (!warn.empty())
		{
			LogWarning("Warning loading GLTF file: %s", warn.c_str());
		}

		auto outputPath = fs::path(mOutputFileName);
		outputPath.remove_filename();

		auto texturesPath = outputPath;
		texturesPath.append("Textures");

		auto modelsPath = outputPath;
		modelsPath.append("Meshes");

		auto imagesPath = outputPath;
		imagesPath.append("Images");

		auto samplersPath = outputPath;
		samplersPath.append("Samplers");

		auto materialsPath = outputPath;
		materialsPath.append("Materials");

		mInputPath = dir.string();
		mOutputPath = outputPath.string();
		mTexturesPath = texturesPath.string();
		mModelsPath = modelsPath.string();
		mImagesPath = imagesPath.string();
		mSamplersPath = samplersPath.string();
		mMaterialsPath = materialsPath.string();

		fileSystem.createDirs(mOutputPath);
		fileSystem.createDirs(mTexturesPath);
		fileSystem.createDirs(mModelsPath);
		fileSystem.createDirs(mImagesPath);
		fileSystem.createDirs(mSamplersPath);
		fileSystem.createDirs(mMaterialsPath);

		auto scene = loadScene();
		if (!scene)
		{
			LogError("SceneConverter::loadScene() failed for: %s!!", mFileName.c_str());
			mResult = false;
			return;
		}

		auto& resourceCache = scene->getResourceCache();
		auto images = resourceCache.getResources<Image>();
		auto samplers = resourceCache.getResources<Sampler>();
		auto textures = resourceCache.getResources<Texture>();
		auto materials = resourceCache.getResources<Material>();
		auto models = resourceCache.getResources<Model>();

		for (auto* image : images)
		{
			if (!image->write())
			{
				LogError("Image::write() failed!!");
				mResult = false;
				return;
			}
		}

		for (auto* sampler : samplers)
		{
			if (!sampler->write())
			{
				LogError("Sampler::write() failed!!");
				mResult = false;
				return;
			}
		}

		for (auto* texture : textures)
		{
			if (!texture->write())
			{
				LogError("Texture::write() failed!!");
				mResult = false;
				return;
			}
		}

		for (auto* material : materials)
		{
			if (!material->write())
			{
				LogError("Material::write() failed!!");
				mResult = false;
				return;
			}
		}

		for (auto* model : models)
		{
			if (!model->write())
			{
				LogError("Material::write() failed!!");
				mResult = false;
				return;
			}
		}

		if (!scene->write())
		{
			LogError("Scene::write() failed for: %s!!", mOutputFileName.c_str());
			mResult = false;
			return;
		}
	}

	std::unique_ptr<Scene> SceneConverter::loadScene()
	{
		auto scene = std::make_unique<Scene>();
		if (!scene->create(mOutputFileName, false))
		{
			LogError("Scene::create() failed for: %s!!", mOutputFileName.c_str());
			return nullptr;
		}

		auto lights = parseLights();
		scene->setComponents(std::move(lights));

		auto defaultSampler = createDefaultSampler(scene->getResourceCache());
		scene->getResourceCache().addResource(std::move(defaultSampler));

		for (const auto& gltfSampler : mModel.samplers)
		{
			auto sampler = parseSampler(gltfSampler, scene->getResourceCache());
			scene->getResourceCache().addResource(std::move(sampler));
		}

		for (const auto& gltfImage : mModel.images)
		{
			auto image = parseImage(gltfImage, scene->getResourceCache());
			scene->getResourceCache().addResource(std::move(image));
		}

		std::unordered_map<size_t, size_t> texSampMap;
		size_t numSamplers = mModel.samplers.size();
		size_t numTextures = 0;

		auto images = scene->getResourceCache().getResources<Image>();
		for (const auto& gltfTexture : mModel.textures)
		{
			auto* image = images[gltfTexture.source];
			auto fileName = fs::path(mTexturesPath);

			auto imageFileName = fs::path(image->getFileName());

			fileName.append(imageFileName.filename().string());
			fileName.replace_extension("ttex");

			auto texture = std::make_unique<Texture2D>();
			if (!texture->create(FileSystem::get().sanitizePath(fileName.string()), scene->getResourceCache(), false))
			{
				Exit("Texture2D::create() failed for: %s!!", fileName.string().c_str());
				return nullptr;
			}

			size_t samplerIndex = gltfTexture.sampler < numSamplers ? gltfTexture.sampler + 1 : 0;
			texSampMap.insert(std::make_pair(numTextures, samplerIndex));

			texture->setImage(image);
			scene->getResourceCache().addResource(std::move(texture));

			numTextures++;
		}

		std::vector<Texture*> textures;
		std::vector<Sampler*> samplers;

		bool hasTextures = scene->getResourceCache().hasResource<Texture>();
		bool hasSamplers = scene->getResourceCache().hasResource<Sampler>();

		if (hasTextures)
		{
			textures = scene->getResourceCache().getResources<Texture>();
		}

		if (hasSamplers)
		{
			samplers = scene->getResourceCache().getResources<Sampler>();
		}

		for (const auto& gltfMaterial : mModel.materials)
		{
			std::vector<std::string> shaderDefines;

			auto material = parseMaterial(gltfMaterial, scene->getResourceCache());
			for (const auto& gltfValue : gltfMaterial.values)
			{
				if (gltfValue.first.find("Texture") != std::string::npos)
				{
					const std::string textureName = toSnakeCase(gltfValue.first);
					Texture* texture = textures[gltfValue.second.TextureIndex()];
					Sampler* sampler = samplers[texSampMap.at(gltfValue.second.TextureIndex())];

					shaderDefines.push_back("has_" + textureName);
					material->setTexture(textureName, *texture, *sampler);
				}
			}

			for (const auto& gltfValue : gltfMaterial.additionalValues)
			{
				if (gltfValue.first.find("Texture") != std::string::npos)
				{
					const std::string textureName = toSnakeCase(gltfValue.first);
					Texture* texture = textures[gltfValue.second.TextureIndex()];
					Sampler* sampler = samplers[texSampMap.at(gltfValue.second.TextureIndex())];

					shaderDefines.push_back("has_" + textureName);
					material->setTexture(textureName, *texture, *sampler);
				}
			}

			auto shader = createDefaultShader(scene->getResourceCache());
			material->setShader(*shader);
			material->setShaderDefines(std::move(shaderDefines));

			scene->getResourceCache().addResource(std::move(shader));
			scene->getResourceCache().addResource(std::move(material));
		}

		auto defaultShader = createDefaultShader(scene->getResourceCache());
		auto defaultMaterial = createDefaultMaterial(scene->getResourceCache());
		defaultMaterial->setShader(*defaultShader);
		
		scene->getResourceCache().addResource(std::move(defaultShader));
		scene->getResourceCache().addResource(std::move(defaultMaterial));

		auto materials = scene->getResourceCache().getResources<Material>();
		for (const auto& gltfMesh : mModel.meshes)
		{
			auto model = parseMesh(gltfMesh, scene->getResourceCache());
			auto mesh = std::make_unique<Mesh>();

			std::vector<Model::Mesh> modelMeshes;
			for (size_t idx = 0; idx < gltfMesh.primitives.size(); idx++)
			{
				Model::Mesh modelMesh{};

				const auto& gltfPrimitive = gltfMesh.primitives[idx];
				modelMesh.name = std::format("'{}' mesh, primitive #{}", gltfMesh.name, idx);

				const float* positions = nullptr;
				const float* normals = nullptr;
				const float* uvs = nullptr;

				auto& accessor = mModel.accessors[gltfPrimitive.attributes.find("POSITION")->second];
				auto& bufferView = mModel.bufferViews[accessor.bufferView];
				size_t numVertices = accessor.count;

				positions = reinterpret_cast<const float*>(&(mModel.buffers[bufferView.buffer].
					data[accessor.byteOffset + bufferView.byteOffset]));

				if (gltfPrimitive.attributes.find("NORMAL") != gltfPrimitive.attributes.end())
				{
					accessor = mModel.accessors[gltfPrimitive.attributes.find("NORMAL")->second];
					bufferView = mModel.bufferViews[accessor.bufferView];
					normals = reinterpret_cast<const float*>(&(mModel.buffers[bufferView.buffer].
						data[accessor.byteOffset + bufferView.byteOffset]));
				}

				if (gltfPrimitive.attributes.find("TEXCOORD_0") != gltfPrimitive.attributes.end())
				{
					accessor = mModel.accessors[gltfPrimitive.attributes.find("TEXCOORD_0")->second];
					bufferView = mModel.bufferViews[accessor.bufferView];
					uvs = reinterpret_cast<const float*>(&(mModel.buffers[bufferView.buffer].
						data[accessor.byteOffset + bufferView.byteOffset]));
				}

				std::vector<Scene::Vertex> vertexData;
				for (size_t v = 0; v < numVertices; v++)
				{
					Scene::Vertex vertex{};
					vertex.position = glm::make_vec3(&positions[v * 3]);
					vertex.normal = normals ? glm::normalize(glm::make_vec3(&normals[v * 3])) : glm::vec3(0.0f);
					vertex.uv = uvs ? glm::make_vec2(&uvs[v * 2]) : glm::vec2(0.0f);

					vertexData.push_back(vertex);
				}

				const uint32_t numFloats = sizeof(Scene::Vertex) / sizeof(float);
			
				modelMesh.vertexData.resize(vertexData.size() * numFloats);
				std::memcpy(modelMesh.vertexData.data(), vertexData.data(), 
					modelMesh.vertexData.size() * sizeof(float));

				modelMesh.numVertices = (uint32_t)numVertices;
				modelMesh.vertexSize = sizeof(Scene::Vertex);

				if (gltfPrimitive.indices >= 0)
				{
					auto numIndices = getAttributeSize(mModel, gltfPrimitive.indices);
					auto indexFormat = getIndexFormat(mModel, gltfPrimitive.indices);
					auto indexData = getAttributeData(mModel, gltfPrimitive.indices);

					if (indexFormat == wgpu::IndexFormat::Uint16)
					{
						indexFormat = wgpu::IndexFormat::Uint32;
						indexData = convertDataStride(indexData, 2, 4);
					}

					modelMesh.indexData.resize(numIndices);
					std::memcpy(modelMesh.indexData.data(), indexData.data(), 
						numIndices * sizeof(uint32_t));

					modelMesh.numIndices = (uint32_t)numIndices;
				}

				if (gltfPrimitive.material < 0)
				{
					modelMesh.materialIndex = (uint32_t)materials.size() - 1;
				}
				else
				{
					modelMesh.materialIndex = (uint32_t)gltfPrimitive.material;
				}

				modelMeshes.push_back(std::move(modelMesh));
			}

			model->setMeshes(std::move(modelMeshes));
			model->setMaterials(std::move(materials));
			mesh->setModel(*model);

			scene->getResourceCache().addResource(std::move(model));
			scene->addComponent(std::move(mesh));
		}

		for (const auto& gltfCamera : mModel.cameras)
		{
			auto camera = parseCamera(gltfCamera);
			scene->addComponent(std::move(camera));
		}

		auto meshes = scene->getComponents<Mesh>();
		auto cameras = scene->getComponents<Camera>();

		std::vector<std::unique_ptr<Node>> nodes;
		for (const auto& gltfNode : mModel.nodes)
		{
			auto node = parseNode(gltfNode);

			if (gltfNode.mesh >= 0)
			{
				auto mesh = meshes[gltfNode.mesh];

				node->setComponent(*mesh);
				mesh->addNode(*node);
			}

			if (gltfNode.camera >= 0)
			{
				auto camera = cameras[gltfNode.camera];

				node->setComponent(*camera);
				camera->setNode(*node);
			}

			nodes.push_back(std::move(node));
		}

		tinygltf::Scene* gltfScene{ nullptr };
		if (mModel.defaultScene >= 0 && mModel.defaultScene < (int)mModel.scenes.size())
		{
			gltfScene = &mModel.scenes[mModel.defaultScene];
		}
		else
		{
			gltfScene = &mModel.scenes[0];
		}

		if (!gltfScene)
		{
			LogError("No scene found!!");
			return nullptr;
		}

		std::queue<std::pair<Node&, int>> traverseNodes;
		auto rootNode = std::make_unique<Node>();

		for (auto node : gltfScene->nodes)
		{
			traverseNodes.push(std::make_pair(std::ref(*rootNode), node));
		}

		while (!traverseNodes.empty())
		{
			auto it = traverseNodes.front();
			traverseNodes.pop();

			auto& currentNode = *nodes[it.second];
			auto& traverseRootNode = it.first;

			currentNode.setParent(traverseRootNode);
			traverseRootNode.addChild(currentNode);

			for (auto childNode : mModel.nodes[it.second].children)
			{
				traverseNodes.push(std::make_pair(std::ref(currentNode), childNode));
			}
		}

		scene->setRoot(*rootNode);
		nodes.push_back(std::move(rootNode));
		scene->setNodes(std::move(nodes));

		auto cameraNode = std::make_unique<Node>();
		cameraNode->setName("default_camera");

		auto defaultCamera = createDefaultCamera();
		defaultCamera->setNode(*cameraNode);
		cameraNode->setComponent(*defaultCamera);
		scene->addComponent(std::move(defaultCamera));

		scene->getRoot()->addChild(*cameraNode);
		scene->addNode(std::move(cameraNode));

		if (!scene->hasComponent<Light>())
		{
			scene->addDirectionalLight(glm::quat({ glm::radians(-90.0f), 0.0f, glm::radians(30.0f) }));
		}

		return scene;
	}

	std::vector<std::unique_ptr<Light>> SceneConverter::parseLights()
	{
		if (mModel.extensions.find(KHR_LIGHTS_PUNCTUAL_EXTENSION) == mModel.extensions.end() ||
			!mModel.extensions.at(KHR_LIGHTS_PUNCTUAL_EXTENSION).Has("lights"))
		{
			return {};
		}

		auto& khrLights = mModel.extensions.at(KHR_LIGHTS_PUNCTUAL_EXTENSION).Get("Lights");
		std::vector<std::unique_ptr<Light>> lights(khrLights.ArrayLen());

		for (size_t idx = 0; idx < khrLights.ArrayLen(); idx++)
		{
			auto& khrLight = khrLights.Get((int)idx);
			if (!khrLight.Has("type"))
			{
				LogWarning("KHR_lights_punctual extension: light doesn't have a type! ignoring");
				continue;
			}

			auto light = std::make_unique<Light>();
			light->setName(khrLight.Get("name").Get<std::string>());

			LightType type;
			LightProperties properties;

			auto& gltfType = khrLight.Get("type").Get<std::string>();
			if (gltfType == "point")
			{
				type = LightType::Point;
			}
			else if (gltfType == "spot")
			{
				type = LightType::Spot;
			}
			else if (gltfType == "directional")
			{
				type = LightType::Directional;
			}
			else
			{
				LogError("KHR_lights_punctual extension: light type '%s' is invalid! ignoring", gltfType.c_str());
				continue;
			}

			if (khrLight.Has("color"))
			{
				properties.color = glm::vec3(
					(float)(khrLight.Get("color").Get(0).Get<double>()),
					(float)(khrLight.Get("color").Get(1).Get<double>()),
					(float)(khrLight.Get("color").Get(2).Get<double>())
				);
			}

			if (khrLight.Has("intensity"))
			{
				properties.intensity = (float)(khrLight.Get("intensity").Get<double>());
			}

			if (type != LightType::Directional)
			{
				properties.range = (float)(khrLight.Get("range").Get<double>());
				if (type != LightType::Point)
				{
					if (!khrLight.Has("spot"))
					{
						LogError("KHR_lights_punctual extension: spot light doesn't have a spot property set");
						continue;
					}

					properties.innerConeAngle = (float)(khrLight.Get("spot").Get("innerConeAngle").Get<double>());
					if (khrLight.Get("spot").Has("outerConeAngle"))
					{
						properties.outerConeAngle = (float)(khrLight.Get("spot").Get("outerConeAngle").Get<double>());
					}
					else
					{
						properties.outerConeAngle = glm::pi<float>() / 4.0f;
					}
				}
			}
			else if (type == LightType::Directional || type == LightType::Spot)
			{
				properties.direction = glm::vec3(0.0f, 0.0f, -1.0f);
			}

			light->setLightType(type);
			light->setLightProperties(properties);
			lights[idx] = std::move(light);
		}

		return lights;
	}

	std::unique_ptr<Node> SceneConverter::parseNode(const tinygltf::Node& gltfNode)
	{
		auto node = std::make_unique<Node>();
		node->setName(gltfNode.name);

		auto& transform = node->getTransform();

		if (!gltfNode.translation.empty())
		{
			glm::vec3 translation;
			std::transform(gltfNode.translation.begin(), gltfNode.translation.end(),
				glm::value_ptr(translation), TypeCast<double, float>{});

			transform.setTranslation(translation);
		}

		if (!gltfNode.rotation.empty())
		{
			glm::quat rotation;
			std::transform(gltfNode.rotation.begin(), gltfNode.rotation.end(),
				glm::value_ptr(rotation), TypeCast<double, float>{});

			transform.setRotation(rotation);
		}

		if (!gltfNode.scale.empty())
		{
			glm::vec3 scale;
			std::transform(gltfNode.scale.begin(), gltfNode.scale.end(),
				glm::value_ptr(scale), TypeCast<double, float>{});

			transform.setTranslation(scale);
		}

		if (!gltfNode.matrix.empty())
		{
			glm::mat4 matrix;
			std::transform(gltfNode.matrix.begin(), gltfNode.matrix.end(),
				glm::value_ptr(matrix), TypeCast<double, float>{});

			transform.setMatrix(matrix);
		}

		return node;
	}

	std::unique_ptr<Camera> SceneConverter::parseCamera(const tinygltf::Camera& gltfCamera)
	{
		std::unique_ptr<Camera> camera{ nullptr };

		if (gltfCamera.type == "perspective")
		{
			auto perspectiveCamera = std::make_unique<PerspectiveCamera>();
			perspectiveCamera->setAspectRatio((float)gltfCamera.perspective.aspectRatio);
			perspectiveCamera->setFOV((float)gltfCamera.perspective.yfov);
			perspectiveCamera->setNearPlane((float)gltfCamera.perspective.znear);
			perspectiveCamera->setFarPlane((float)gltfCamera.perspective.zfar);

			camera = std::move(perspectiveCamera);
		}
		else
		{
			LogDebug("Camera type not supported: %s!", gltfCamera.type.c_str());
		}

		return camera;
	}

	std::unique_ptr<Model> SceneConverter::parseMesh(const tinygltf::Mesh& gltfMesh, ResourceCache& cache)
	{
		auto fileName = fs::path(mModelsPath);
		if (!gltfMesh.name.empty())
		{
			fileName.append(gltfMesh.name);
		}
		else
		{
			auto meshes = cache.getResources<Mesh>();
			fileName.append(std::format("Mesh_{}", meshes.size()));
		}

		fileName.replace_extension("tmesh");

		auto model = std::make_unique<Model>();
		if (!model->create(FileSystem::get().sanitizePath(fileName.string()), cache, false))
		{
			LogError("Model::create() failed for: %s!!", fileName.c_str());
			return nullptr;
		}

		return model;
	}

	std::unique_ptr<Material> SceneConverter::parseMaterial(const tinygltf::Material& gltfMaterial, ResourceCache& cache)
	{
		auto fileName = fs::path(mMaterialsPath);	
		if (!gltfMaterial.name.empty())
		{
			fileName.append(gltfMaterial.name);
		}
		else
		{
			auto materials = cache.getResources<Material>();
			fileName.append(std::format("Material_{}", materials.size()));
		}

		fileName.replace_extension("tmat");

		auto material = std::make_unique<PBRMaterial>();
		if (!material->create(FileSystem::get().sanitizePath(fileName.string()), cache, false))
		{
			LogError("PBRMaterial::create() failed for: %s!!", fileName.string().c_str());
			return nullptr;
		}

		for (auto& gltfValue : gltfMaterial.values)
		{
			if (gltfValue.first == "baseColorFactor")
			{
				const auto& colorFactor = gltfValue.second.ColorFactor();
				material->setBaseColorFactor(glm::vec4(colorFactor[0], colorFactor[1],
					colorFactor[2], colorFactor[3]));
			}
			else if (gltfValue.first == "metallicFactor")
			{
				material->setMetallicFactor((float)gltfValue.second.Factor());
			}
			else if (gltfValue.first == "roughnessFactor")
			{
				material->setRoughnessFactor((float)gltfValue.second.Factor());
			}
		}

		for (auto& gltfValue : gltfMaterial.additionalValues)
		{
			if (gltfValue.first == "emissiveFactor")
			{
				const auto& emissiveFactor = gltfValue.second.number_array;
				material->setEmissive(glm::vec3(emissiveFactor[0], emissiveFactor[1], emissiveFactor[2]));
			}
			else if (gltfValue.first == "alphaMode")
			{
				if (gltfValue.second.string_value == "BLEND")
				{
					material->setAlphaMode(AlphaMode::Blend);
				}
				else if (gltfValue.second.string_value == "OPAQUE")
				{
					material->setAlphaMode(AlphaMode::Opaque);
				}
				else if (gltfValue.second.string_value == "MASK")
				{
					material->setAlphaMode(AlphaMode::Mask);
				}
			}
			else if (gltfValue.first == "alphaCutoff")
			{
				material->setAlphaCutoff((float)gltfValue.second.number_value);
			}
			else if (gltfValue.first == "doubleSided")
			{
				material->setDoubleSided(gltfValue.second.bool_value);
			}
		}

		return material;
	}

	std::unique_ptr<Image> SceneConverter::parseImage(const tinygltf::Image& gltfImage, ResourceCache& cache)
	{
		std::unique_ptr<Image> image{ nullptr };

		if (!gltfImage.image.empty())
		{
			auto images = cache.getResources<Image>();

			fs::path fileName(mImagesPath);
			fileName.append(std::format("Image_{}", images.size()));
			fileName.replace_extension("png");

			image = std::make_unique<Image>();
			if (!image->create(FileSystem::get().sanitizePath(fileName.string()), cache, false))
			{
				LogError("Image::create() failed for: %s!!", fileName.c_str());
				return nullptr;
			}

			if (!image->load(gltfImage.image))
			{
				LogError("Image::load() failed for: %s!!", fileName.c_str());
				return nullptr;
			}
		}
		else
		{
			auto imageUri = mInputPath + "/" + gltfImage.uri;

			fs::path fileName(mImagesPath);
			fileName.append(gltfImage.uri);
			fileName.replace_extension("png");

			image = std::make_unique<Image>();
			if (!image->create(FileSystem::get().sanitizePath(fileName.string()), cache, false))
			{
				LogError("Image::create() failed for: %s!!", fileName.c_str());
				return nullptr;
			}

			if (!image->load(imageUri))
			{
				LogError("Image::load() failed for: %s!!", fileName.c_str());
				return nullptr;
			}
		}

		return image;
	}

	std::unique_ptr<Sampler> SceneConverter::parseSampler(const tinygltf::Sampler& gltfSampler, ResourceCache& cache)
	{
		auto fileName = fs::path(mSamplersPath);
		if (!gltfSampler.name.empty())
		{
			fileName.append(gltfSampler.name);
		}
		else
		{
			auto samplers = cache.getResources<Sampler>();
			fileName.append(std::format("Sampler_{}", samplers.size()));
		}

		fileName.replace_extension("tsamp");

		SamplerProperties samplerProps =
		{
			.addressModeU = findWrapMode(gltfSampler.wrapS),
			.addressModeV = findWrapMode(gltfSampler.wrapT),
			.magFilter = findMagFilter(gltfSampler.magFilter),
			.minFilter = findMinFilter(gltfSampler.minFilter),
			.mipmapFilter = findMipMapMode(gltfSampler.minFilter),
		};

		auto sampler = std::make_unique<Sampler>();
		if (!sampler->create(FileSystem::get().sanitizePath(fileName.string()), cache, false))
		{
			LogError("Sampler::create() failed!!");
			return nullptr;
		}

		sampler->setProperties(samplerProps);
		return sampler;
	}

	std::unique_ptr<Shader> SceneConverter::createDefaultShader(ResourceCache& cache)
	{
		auto shader = std::make_unique<Shader>();
		shader->setFileName(PBRMaterial::kDefaultShader);
		
		return shader;
	}

	std::unique_ptr<Material> SceneConverter::createDefaultMaterial(ResourceCache& cache)
	{		
		auto material = std::make_unique<PBRMaterial>();
		if (!material->create(PBRMaterial::kDefault, cache, false))
		{
			LogError("PBRMaterial::create() failed for: %s!!", PBRMaterial::kDefault);
			return nullptr;
		}

		return material;
	}

	std::unique_ptr<Sampler> SceneConverter::createDefaultSampler(ResourceCache& cache)
	{
		auto sampler = std::make_unique<Sampler>();
		if (!sampler->create(Sampler::kDefault, cache, false))
		{
			LogError("Sampler::create() failed for: %s!!", Sampler::kDefault);
			return nullptr;
		}

		return sampler;
	}

	std::unique_ptr<Camera> SceneConverter::createDefaultCamera()
	{
		tinygltf::Camera gltfCamera;

		gltfCamera.name = "default_camera";
		gltfCamera.type = "perspective";
		gltfCamera.perspective.aspectRatio = 1.77f;
		gltfCamera.perspective.yfov = 1.0f;
		gltfCamera.perspective.znear = 0.1f;
		gltfCamera.perspective.zfar = 10000.0f;

		return parseCamera(gltfCamera);
	}
}

using namespace Trinity;

int main(int argc, char* argv[])
{
	CLI::App cliApp{ "Scene Converter" };
	std::string fileName;
	std::string outputFileName;

	cliApp.add_option<std::string>("-f, --filename, filename", fileName, "Filename")->required();
	cliApp.add_option<std::string>("-o, --output, output", outputFileName, "Output Filename")->required();
	CLI11_PARSE(cliApp, argc, argv);

	static SceneConverter app;
	app.setFileName(fileName);
	app.setOutputFileName(outputFileName);

	if (!app.run(LogLevel::Info))
	{
		return -1;
	}

	return 0;
}