#include "Scene/SceneLoader.h"
#include "Scene/Node.h"
#include "Scene/Scene.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/Light.h"
#include "Scene/Components/Mesh.h"
#include "Scene/Components/SubMesh.h"
#include "Scene/Components/PerspectiveCamera.h"
#include "Graphics/ResourceCache.h"
#include "Graphics/PBRMaterial.h"
#include "Graphics/Sampler.h"
#include "Graphics/Texture2D.h"
#include "Graphics/BindGroup.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/BindGroup.h"
#include "Graphics/BindGroupLayout.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"
#include "VFS/FileSystem.h"
#include <format>
#include <queue>

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

namespace Trinity
{
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

	std::unique_ptr<Node> parseNode(const tinygltf::Node& gltfNode)
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

	std::unique_ptr<Camera> parseCamera(const tinygltf::Camera& gltfCamera)
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

	std::unique_ptr<Mesh> parseMesh(const tinygltf::Mesh& gltfMesh)
	{
		auto mesh = std::make_unique<Mesh>();
		mesh->setName(gltfMesh.name);

		return mesh;
	}

	std::unique_ptr<PBRMaterial> parseMaterial(const tinygltf::Material& gltfMaterial)
	{
		auto material = std::make_unique<PBRMaterial>();

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

	std::unique_ptr<Image> parseImage(const std::string& modelPath, const tinygltf::Image& gltfImage)
	{
		std::unique_ptr<Image> image{ nullptr };

		if (!gltfImage.image.empty())
		{
			image = std::make_unique<Image>();
			if (!image->create(gltfImage.image))
			{
				LogError("Image::create() failed");
			}
		}
		else
		{
			auto imageUri = modelPath + "/" + gltfImage.uri;

			image = std::make_unique<Image>();
			if (!image->create(imageUri))
			{
				LogError("Image::create() failed for: %s", imageUri.c_str());
			}
		}

		return image;
	}

	std::unique_ptr<Sampler> parseSampler(const tinygltf::Sampler& gltfSampler)
	{
		SamplerProperties samplerProps =
		{
			.addressModeU = findWrapMode(gltfSampler.wrapS),
			.addressModeV = findWrapMode(gltfSampler.wrapT),
			.magFilter = findMagFilter(gltfSampler.magFilter),
			.minFilter = findMinFilter(gltfSampler.minFilter),
			.mipmapFilter = findMipMapMode(gltfSampler.minFilter),
		};

		auto sampler = std::make_unique<Sampler>();
		if (!sampler->create(samplerProps))
		{
			LogError("Sampler::create() failed!!");
			return nullptr;
		}

		return sampler;
	}

	std::unique_ptr<Texture> parseTexture(const tinygltf::Texture& gltfTexture)
	{
		return std::make_unique<Texture2D>();
	}

	std::unique_ptr<Shader> createDefaultShader(const std::vector<std::string>& defines)
	{
		ShaderPreProcessor processor;
		processor.addDefines(defines);

		auto shader = std::make_unique<Shader>();
		if (!shader->create(PBRMaterial::kDefaultShader, processor))
		{
			LogError("Shader::create() failed!!");
			return nullptr;
		}

		return shader;
	}

	std::unique_ptr<PBRMaterial> createDefaultMaterial()
	{
		tinygltf::Material gltfMaterial;
		return parseMaterial(gltfMaterial);
	}

	std::unique_ptr<Sampler> createDefaultSampler()
	{
		tinygltf::Sampler gltfSampler;

		gltfSampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
		gltfSampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
		gltfSampler.wrapS = TINYGLTF_TEXTURE_WRAP_REPEAT;
		gltfSampler.wrapT = TINYGLTF_TEXTURE_WRAP_REPEAT;

		return parseSampler(gltfSampler);
	}

	std::unique_ptr<Camera> createDefaultCamera()
	{
		tinygltf::Camera gltfCamera;

		gltfCamera.name = "default_camera";
		gltfCamera.type = "perspective";
		gltfCamera.perspective.aspectRatio = 1.77f;
		gltfCamera.perspective.yfov = 1.0f;
		gltfCamera.perspective.znear = 0.1f;
		gltfCamera.perspective.zfar = 1000.0f;

		return parseCamera(gltfCamera);
	}

	Scene readScene(const std::string& modelPath, tinygltf::Model* model)
	{
		auto scene = Scene();
		scene.setResourceCache(std::make_unique<ResourceCache>());

		auto defaultSampler = createDefaultSampler();
		scene.getResourceCache().addResource(std::move(defaultSampler));

		for (const auto& gltfSampler : model->samplers)
		{
			auto sampler = parseSampler(gltfSampler);
			scene.getResourceCache().addResource(std::move(sampler));
		}

		std::vector<std::unique_ptr<Image>> images;
		for (const auto& gltfImage : model->images)
		{
			auto image = parseImage(modelPath, gltfImage);
			images.push_back(std::move(image));
		}

		std::unordered_map<size_t, size_t> texSampMap;
		size_t numSamplers = model->samplers.size();
		size_t numTextures = 0;

		for (const auto& gltfTexture : model->textures)
		{
			Assert(gltfTexture.source < images.size(), "Invalid gltfTexture.source");

			auto texture = std::make_unique<Texture2D>();
			if (!texture->create(*images[gltfTexture.source]))
			{
				Exit("Texture2D::create() failed");
				return scene;
			}

			size_t samplerIndex = gltfTexture.sampler < numSamplers ? gltfTexture.sampler + 1 : 0;
			texSampMap.insert(std::make_pair(numTextures, samplerIndex));

			scene.getResourceCache().addResource(std::move(texture));
			numTextures++;
		}

		std::vector<Texture*> textures;
		std::vector<Sampler*> samplers;
		std::vector<std::string> shaderDefines;

		bool hasTextures = scene.getResourceCache().hasResource<Texture>();
		bool hasSamplers = scene.getResourceCache().hasResource<Sampler>();

		if (hasTextures)
		{
			textures = scene.getResourceCache().getResources<Texture>();
		}

		if (hasSamplers)
		{
			samplers = scene.getResourceCache().getResources<Sampler>();
		}

		for (const auto& gltfMaterial : model->materials)
		{
			auto material = parseMaterial(gltfMaterial);
			for (const auto& gltfValue : gltfMaterial.values)
			{
				if (gltfValue.first.find("Texture") != std::string::npos)
				{
					Assert(gltfValue.second.TextureIndex() < textures.size(), "Invalid texture index");

					Texture* texture = textures[gltfValue.second.TextureIndex()];
					Sampler* sampler = samplers[texSampMap.at(gltfValue.second.TextureIndex())];
					
					shaderDefines.push_back(std::string("has_") + gltfValue.first);
					material->setTexture(gltfValue.first, *texture, *sampler);
				}
			}

			for (const auto& gltfValue : gltfMaterial.additionalValues)
			{
				if (gltfValue.first.find("Texture") != std::string::npos)
				{
					Assert(gltfValue.second.TextureIndex() < textures.size(), "Invalid texture index");

					Texture* texture = textures[gltfValue.second.TextureIndex()];
					Sampler* sampler = samplers[texSampMap.at(gltfValue.second.TextureIndex())];

					shaderDefines.push_back(std::string("has_") + gltfValue.first);
					material->setTexture(gltfValue.first, *texture, *sampler);
				}
			}

			auto shader = createDefaultShader(shaderDefines);
			material->setShader(*shader);

			if (!material->compile())
			{
				LogError("PBRMaterial::compile() failed!!");
				return scene;
			}

			scene.getResourceCache().addResource(std::move(shader));
			scene.getResourceCache().addResource(std::move(material));
		}

		auto defaultMaterial = createDefaultMaterial();
		auto materials = scene.getResourceCache().getResources<PBRMaterial>();

		auto vertexLayout = std::make_unique<VertexLayout>();
		vertexLayout->setAttributes({
			{ wgpu::VertexFormat::Float32x3, 0, 0 },
			{ wgpu::VertexFormat::Float32x3, 12, 1 },
			{ wgpu::VertexFormat::Float32x2, 24, 2 },
		});

		const auto* vertexLayoutPtr = vertexLayout.get();
		scene.getResourceCache().addResource(std::move(vertexLayout));

		for (const auto& gltfMesh : model->meshes)
		{
			auto mesh = std::make_unique<Mesh>();
			mesh->setName(gltfMesh.name);

			for (size_t idx = 0; idx < gltfMesh.primitives.size(); idx++)
			{
				const auto& gltfPrimitive = gltfMesh.primitives[idx];
#ifndef __EMSCRIPTEN__
				auto subMeshName = std::format("'{}' mesh, primitive #{}", gltfMesh.name, idx);
#else
				char buffer[256];
				snprintf(buffer, 256, "'%s' mesh, primitive #%ld", gltfMesh.name.c_str(), idx);

				std::string subMeshName(buffer);
#endif
				auto subMesh = std::make_unique<SubMesh>();
				subMesh->setName(subMeshName);

				const float* positions = nullptr;
				const float* normals = nullptr;
				const float* uvs = nullptr;

				auto& accessor = model->accessors[gltfPrimitive.attributes.find("POSITION")->second];
				auto& bufferView = model->bufferViews[accessor.bufferView];
				size_t numVertices = accessor.count;

				positions = reinterpret_cast<const float*>(&(model->buffers[bufferView.buffer].
					data[accessor.byteOffset + bufferView.byteOffset]));

				if (gltfPrimitive.attributes.find("NORMAL") != gltfPrimitive.attributes.end())
				{
					accessor = model->accessors[gltfPrimitive.attributes.find("NORMAL")->second];
					bufferView = model->bufferViews[accessor.bufferView];
					normals = reinterpret_cast<const float*>(&(model->buffers[bufferView.buffer].
						data[accessor.byteOffset + bufferView.byteOffset]));
				}

				if (gltfPrimitive.attributes.find("TEXCOORD_0") != gltfPrimitive.attributes.end())
				{
					accessor = model->accessors[gltfPrimitive.attributes.find("TEXCOORD_0")->second];
					bufferView = model->bufferViews[accessor.bufferView];
					uvs = reinterpret_cast<const float*>(&(model->buffers[bufferView.buffer].
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

				auto vertexBuffer = std::make_unique<VertexBuffer>();
				if (!vertexBuffer->create(*vertexLayoutPtr, (uint32_t)numVertices, vertexData.data()))
				{
					Exit("VertexBuffer::create() failed!!");
					return scene;
				}

				subMesh->setVertexBuffer("vertexBuffer", std::move(vertexBuffer));
				subMesh->setVertexLayout(*vertexLayoutPtr);
				subMesh->setNumVertices((uint32_t)numVertices);

				if (gltfPrimitive.indices >= 0)
				{
					auto numIndices = getAttributeSize(*model, gltfPrimitive.indices);
					auto indexFormat = getIndexFormat(*model, gltfPrimitive.indices);
					auto indexData = getAttributeData(*model, gltfPrimitive.indices);

					auto indexBuffer = std::make_unique<IndexBuffer>();
					if (!indexBuffer->create(indexFormat, (uint32_t)numIndices, indexData.data()))
					{
						LogError("IndexBuffer::create() failed!!");
						return scene;
					}

					subMesh->setIndexBuffer(std::move(indexBuffer));
					subMesh->setNumIndices((uint32_t)numIndices);
				}

				if (gltfPrimitive.material < 0)
				{
					subMesh->setMaterial(*defaultMaterial);
				}
				else
				{
					subMesh->setMaterial(*materials[gltfPrimitive.material]);
				}

				mesh->addSubMesh(*subMesh);
				scene.addComponent(std::move(subMesh));
			}

			scene.addComponent(std::move(mesh));
		}

		scene.getResourceCache().addResource(std::move(defaultMaterial));

		for (const auto& gltfCamera : model->cameras)
		{
			auto camera = parseCamera(gltfCamera);
			scene.addComponent(std::move(camera));
		}

		auto meshes = scene.getComponents<Mesh>();
		auto cameras = scene.getComponents<Camera>();
		
		std::vector<std::unique_ptr<Node>> nodes;

		for (const auto& gltfNode : model->nodes)
		{
			auto node = parseNode(gltfNode);

			if (gltfNode.mesh >= 0)
			{
				auto mesh = meshes[gltfNode.mesh];

				node->addComponent(*mesh);
				mesh->addNode(*node);
			}

			if (gltfNode.camera >= 0)
			{
				auto camera = cameras[gltfNode.camera];

				node->addComponent(*camera);
				camera->setNode(*node);
			}

			nodes.push_back(std::move(node));
		}

		tinygltf::Scene* gltfScene{ nullptr };

		if (model->defaultScene >= 0 && model->defaultScene < (int)model->scenes.size())
		{
			gltfScene = &model->scenes[model->defaultScene];
		}
		else
		{
			gltfScene = &model->scenes[0];
		}

		if (!gltfScene)
		{
			Exit("No scene found!!");
			return scene;
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

			for (auto childNode : model->nodes[it.second].children)
			{
				traverseNodes.push(std::make_pair(std::ref(currentNode), childNode));
			}
		}

		scene.setRoot(*rootNode);
		nodes.push_back(std::move(rootNode));
		scene.setNodes(std::move(nodes));

		auto cameraNode = std::make_unique<Node>();
		cameraNode->setName("defaultCamera");

		auto defaultCamera = createDefaultCamera();
		defaultCamera->setNode(*cameraNode);
		cameraNode->addComponent(*defaultCamera);
		scene.addComponent(std::move(defaultCamera));

		scene.getRoot()->addChild(*cameraNode);
		scene.addNode(std::move(cameraNode));

		return scene;
	}

	std::unique_ptr<Scene> SceneLoader::loadScene(const std::string& fileName)
	{
		auto file = FileSystem::get().openFile(fileName, FileOpenMode::OpenRead);
		if (!file)
		{
			LogError("FileSystem::openFile() failed for: %s", fileName.c_str());
			return nullptr;
		}

		fs::path dir{ fileName };
		dir.remove_filename();

		FileReader reader{ *file };
		std::string source = reader.readAsString();

		std::string err;
		std::string warn;
		tinygltf::TinyGLTF gltfLoader;

		tinygltf::Model model{};
		if (!gltfLoader.LoadASCIIFromString(&model, &err, &warn, source.c_str(), (uint32_t)source.length(), dir.string()))
		{
			LogError("Failed to load GLTF file: %s", fileName.c_str());
			return nullptr;
		}

		if (!err.empty())
		{
			LogError("Error loading GLTF file: %s", err.c_str());
			return nullptr;
		}

		if (!warn.empty())
		{
			LogWarning("Warning loading GLTF file: %s", warn.c_str());
		}

		return std::make_unique<Scene>(readScene(dir.string(), &model));
	}
}