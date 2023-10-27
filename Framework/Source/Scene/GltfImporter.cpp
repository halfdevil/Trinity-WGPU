#include "Scene/GltfImporter.h"
#include "Scene/Node.h"
#include "Scene/Scene.h"
#include "Scene/ComponentFactory.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/Light.h"
#include "Scene/Components/Mesh.h"
#include "Scene/Components/SubMesh.h"
#include "Scene/Components/PerspectiveCamera.h"
#include "Scene/Model.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/PBRMaterial.h"
#include "Graphics/Sampler.h"
#include "Graphics/Texture2D.h"
#include "Graphics/BindGroup.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/BindGroup.h"
#include "Graphics/BindGroupLayout.h"
#include "Animation/AnimationTransform.h"
#include "Animation/AnimationClip.h"
#include "Animation/AnimationPose.h"
#include "Animation/Skeleton.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"
#include "Core/Image.h"
#include "Core/Utils.h"
#include "Core/ResourceCache.h"
#include "VFS/FileSystem.h"
#include "VFS/DiskFile.h"
#include <format>
#include <queue>

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

#define KHR_LIGHTS_PUNCTUAL_EXTENSION "KHR_lights_punctual"

namespace Trinity
{
	struct AnimationSampler
	{
		Interpolation type{ Interpolation::Linear };
		std::vector<float> inputs;
		std::vector<float> outputs;
	};

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

	std::unordered_map<int32_t, int32_t> getParentMap(const tinygltf::Model& gltfModel, const tinygltf::Scene& gltfScene)
	{
		std::unordered_map<int32_t, int32_t> parentMap;
		std::queue<int> traverseNodes;
		int32_t rootNode = -1;

		for (auto node : gltfScene.nodes)
		{
			parentMap.insert(std::make_pair(node, -1));
			traverseNodes.push(node);
		}

		while (!traverseNodes.empty())
		{
			auto currentNode = traverseNodes.front();
			traverseNodes.pop();

			for (auto childNode : gltfModel.nodes[currentNode].children)
			{
				parentMap.insert(std::make_pair(childNode, currentNode));
				traverseNodes.push(childNode);
			}
		}

		return parentMap;
	}

	std::vector<std::string> parseJointNames(const tinygltf::Model& gltfModel)
	{
		uint32_t nodeCount = (uint32_t)gltfModel.nodes.size();
		std::vector<std::string> names(nodeCount);

		for (uint32_t idx = 0; idx < nodeCount; idx++)
		{
			auto& node = gltfModel.nodes[idx];
			if (node.name.empty())
			{
				names[idx] = "EMPTY NODE";
			}
			else
			{
				names[idx] = node.name;
			}
		}

		return names;
	}

	AnimationTransform parseAnimTransform(const tinygltf::Node& gltfNode)
	{
		AnimationTransform transform;

		if (!gltfNode.translation.empty())
		{
			glm::vec3 translation;
			std::transform(gltfNode.translation.begin(), gltfNode.translation.end(),
				glm::value_ptr(translation), TypeCast<double, float>{});

			transform.translation = translation;
		}

		if (!gltfNode.rotation.empty())
		{
			glm::quat rotation;
			std::transform(gltfNode.rotation.begin(), gltfNode.rotation.end(),
				glm::value_ptr(rotation), TypeCast<double, float>{});

			transform.rotation = rotation;
		}

		if (!gltfNode.scale.empty())
		{
			glm::vec3 scale;
			std::transform(gltfNode.scale.begin(), gltfNode.scale.end(),
				glm::value_ptr(scale), TypeCast<double, float>{});

			transform.scale = scale;
		}

		if (!gltfNode.matrix.empty())
		{
			glm::mat4 matrix;
			std::transform(gltfNode.matrix.begin(), gltfNode.matrix.end(),
				glm::value_ptr(matrix), TypeCast<double, float>{});

			transform.fromMatrix(matrix);
		}

		return transform;
	}

	std::unique_ptr<AnimationPose> parseRestPose(const tinygltf::Model& gltfModel, const std::unordered_map<int32_t, int32_t>& parentMap)
	{
		uint32_t nodeCount = (uint32_t)gltfModel.nodes.size();
		auto pose = std::make_unique<AnimationPose>(nodeCount);

		for (uint32_t idx = 0; idx < nodeCount; idx++)
		{
			auto& node = gltfModel.nodes[idx];

			AnimationTransform transform = parseAnimTransform(node);
			pose->setLocalTransform(idx, transform);
			pose->setParent(idx, parentMap.at((int32_t)idx));
		}

		return pose;
	}

	std::unique_ptr<AnimationPose> parseBindPose(const tinygltf::Model& gltfModel, const std::unordered_map<int32_t, int32_t>& parentMap)
	{
		auto restPose = parseRestPose(gltfModel, parentMap);

		uint32_t numJoints = restPose->getNumJoints();
		std::vector<glm::mat4> worldBindPose(numJoints);

		restPose->getMatrixPalette(worldBindPose);

		uint32_t numSkins = (uint32_t)gltfModel.skins.size();
		for (uint32_t idx = 0; idx < numSkins; idx++)
		{
			auto& gltfSkin = gltfModel.skins[idx];
			auto matricesData = getAttributeData(gltfModel, gltfSkin.inverseBindMatrices);
			const float* invBindAccessor = (const float*)matricesData.data();
			
			uint32_t numSkinJoints = (uint32_t)gltfSkin.joints.size();
			for (uint32_t jdx = 0; jdx < numSkinJoints; jdx++)
			{
				const float* matrix = &(invBindAccessor[jdx * 16]);
				auto invBindMatrix = glm::make_mat4(matrix);
				auto bindMatrix = glm::inverse(invBindMatrix);

				worldBindPose[gltfSkin.joints[jdx]] = bindMatrix;
			}
		}

		auto bindPose = std::make_unique<AnimationPose>(*restPose);
		for (uint32_t idx = 0; idx < numJoints; idx++)
		{
			auto current = worldBindPose[idx];
			auto p = bindPose->getParent(idx);

			if (p >= 0)
			{
				auto parent = worldBindPose[p];
				current = glm::inverse(parent) * current;
			}

			bindPose->setLocalTransform(idx, AnimationTransform(current));
		}

		return bindPose;
	}

	std::vector<AnimationSampler> getAnimationSamplers(const tinygltf::Model& gltfModel, const tinygltf::Animation& gltfAnim)
	{
		std::vector<AnimationSampler> samplers;

		for (const auto& gltfSampler : gltfAnim.samplers)
		{
			AnimationSampler sampler;
			if (gltfSampler.interpolation == "LINEAR")
			{
				sampler.type = Interpolation::Linear;
			}
			else if (gltfSampler.interpolation == "STEP")
			{
				sampler.type = Interpolation::Constant;
			}
			else if (gltfSampler.interpolation == "CUBICSPLINE")
			{
				sampler.type = Interpolation::Cubic;
			}

			const auto& inputAccessor = gltfModel.accessors[gltfSampler.input];
			auto inputAccessorData = getAttributeData(gltfModel, gltfSampler.input);

			const float* inData = (const float*)inputAccessorData.data();
			for (size_t idx = 0; idx < inputAccessor.count; idx++)
			{
				sampler.inputs.push_back(inData[idx]);
			}

			const auto& outputAccessor = gltfModel.accessors[gltfSampler.output];
			auto outputAccessorData = getAttributeData(gltfModel, gltfSampler.output);

			size_t outCount{ 0 };
			switch (outputAccessor.type)
			{
			case TINYGLTF_TYPE_VEC3:
				outCount = outputAccessor.count * 3;
				break;

			case TINYGLTF_TYPE_VEC4:
				outCount = outputAccessor.count * 4;
				break;
			}

			const float* outData = (const float*)outputAccessorData.data();
			for (size_t idx = 0; idx < outCount; idx++)
			{
				sampler.outputs.push_back(outData[idx]);
			}

			samplers.push_back(std::move(sampler));
		}

		return samplers;
	}

	template <typename T, int N>
	void getTrackFromChannel(Track<T>& inOutTrack, const tinygltf::AnimationChannel& gltfChannel, 
		const std::vector<AnimationSampler>& samplers)
	{
		auto& sampler = samplers[gltfChannel.sampler];
		bool isSamplerCubic = sampler.type == Interpolation::Cubic;

		uint32_t numFrames = (uint32_t)sampler.inputs.size();
		uint32_t numValuesPerFrame = (uint32_t)sampler.outputs.size() / numFrames;

		inOutTrack.setInterpolation(sampler.type);
		inOutTrack.resize(numFrames);

		for (uint32_t idx = 0; idx < numFrames; idx++)
		{
			uint32_t baseIndex = idx * numValuesPerFrame;
			uint32_t offset = 0;

			Frame<T>& frame = inOutTrack[idx];
			frame.time = sampler.inputs[idx];

			for (int c = 0; c < N; c++)
			{
				frame.in[c] = isSamplerCubic ? sampler.outputs[baseIndex + offset++] : 0.0f;
			}

			for (int c = 0; c < N; c++)
			{
				frame.value[c] = sampler.outputs[baseIndex + offset++];
			}

			for (int c = 0; c < N; c++)
			{
				frame.out[c] = isSamplerCubic ? sampler.outputs[baseIndex + offset++] : 0.0f;
			}

			if (offset != numValuesPerFrame)
			{
				LogWarning("Wrong number of frame components");
			}
		}
	}

	std::vector<std::unique_ptr<Light>> parseLights(const tinygltf::Model& gltfModel)
	{
		if (gltfModel.extensions.find(KHR_LIGHTS_PUNCTUAL_EXTENSION) == gltfModel.extensions.end() ||
			!gltfModel.extensions.at(KHR_LIGHTS_PUNCTUAL_EXTENSION).Has("lights"))
		{
			return {};
		}

		auto& khrLights = gltfModel.extensions.at(KHR_LIGHTS_PUNCTUAL_EXTENSION).Get("Lights");
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

			transform.setScale(scale);
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

	std::unique_ptr<Model> parseMesh(const tinygltf::Mesh& gltfMesh, ResourceCache& cache, const std::string& modelsPath, bool loadContent = true)
	{
		auto fileName = fs::path(modelsPath);
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
		if (!model->create(FileSystem::get().sanitizePath(fileName.string()), cache, loadContent))
		{
			LogError("Model::create() failed for: %s!!", fileName.c_str());
			return nullptr;
		}

		return model;
	}

	std::unique_ptr<Material> parseMaterial(const tinygltf::Material& gltfMaterial, ResourceCache& cache, 
		const std::string& materialsPath, bool loadContent = true)
	{
		auto fileName = fs::path(materialsPath);
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
		if (!material->create(FileSystem::get().sanitizePath(fileName.string()), cache, loadContent))
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

	std::unique_ptr<Image> parseImage(const tinygltf::Image& gltfImage, ResourceCache& cache, const std::string& inputPath,
		const std::string& imagesPath, bool loadContent = true)
	{
		std::unique_ptr<Image> image{ nullptr };

		if (!gltfImage.image.empty())
		{
			auto images = cache.getResources<Image>();

			fs::path fileName(imagesPath);
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
			auto imageUri = inputPath + "/" + gltfImage.uri;

			fs::path fileName(imagesPath);
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

	std::unique_ptr<Sampler> parseSampler(const tinygltf::Sampler& gltfSampler, ResourceCache& cache,
		const std::string& samplersPath, bool loadContent = true)
	{
		auto fileName = fs::path(samplersPath);
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
		if (!sampler->create(FileSystem::get().sanitizePath(fileName.string()), cache, loadContent))
		{
			LogError("Sampler::create() failed!!");
			return nullptr;
		}

		sampler->setProperties(samplerProps);
		return sampler;
	}

	std::unique_ptr<Shader> createDefaultShader(ResourceCache& cache, const std::vector<std::string>& defines, bool loadContent = true)
	{
		auto shader = std::make_unique<Shader>();
		if (!shader->create(PBRMaterial::kDefaultShader, cache, false))
		{
			LogError("Shader::create() failed for: %s!!", PBRMaterial::kDefaultShader);
			return nullptr;
		}

		if (loadContent)
		{
			ShaderPreProcessor processor;
			processor.addDefines(defines);

			if (!shader->load(PBRMaterial::kDefaultShader, processor))
			{
				LogError("Shader::load() failed for: %s!!", PBRMaterial::kDefaultShader);
				return nullptr;
			}
		}

		return shader;
	}

	std::unique_ptr<Material> createDefaultMaterial(ResourceCache& cache, bool loadContent = true)
	{
		auto material = std::make_unique<PBRMaterial>();
		if (!material->create(PBRMaterial::kDefault, cache, loadContent))
		{
			LogError("PBRMaterial::create() failed for: %s!!", PBRMaterial::kDefault);
			return nullptr;
		}

		return material;
	}

	std::unique_ptr<Sampler> createDefaultSampler(ResourceCache& cache, bool loadContent = true)
	{
		auto sampler = std::make_unique<Sampler>();
		if (!sampler->create(Sampler::kDefault, cache, loadContent))
		{
			LogError("Sampler::create() failed for: %s!!", Sampler::kDefault);
			return nullptr;
		}

		return sampler;
	}

	std::unique_ptr<Camera> createDefaultCamera()
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

	bool loadMaterials(
		const tinygltf::Model& gltfModel, 
		ResourceCache& cache, 
		const std::string& inputPath, 
		const std::string& materialsPath, 
		const std::string& imagesPath, 
		const std::string& texturesPath, 
		const std::string& samplersPath, 
		bool hasSkin = false,
		bool loadContent = true)
	{
		auto defaultSampler = createDefaultSampler(cache, loadContent);
		cache.addResource(std::move(defaultSampler));

		for (const auto& gltfSampler : gltfModel.samplers)
		{
			auto sampler = parseSampler(gltfSampler, cache, samplersPath, loadContent);
			cache.addResource(std::move(sampler));
		}

		for (const auto& gltfImage : gltfModel.images)
		{
			auto image = parseImage(gltfImage, cache, inputPath, imagesPath, loadContent);
			cache.addResource(std::move(image));
		}

		std::unordered_map<size_t, size_t> texSampMap;
		size_t numSamplers = gltfModel.samplers.size();
		size_t numTextures = 0;

		auto images = cache.getResources<Image>();
		for (const auto& gltfTexture : gltfModel.textures)
		{
			auto* image = images[gltfTexture.source];
			auto fileName = fs::path(texturesPath);

			auto imageFileName = fs::path(image->getFileName());

			fileName.append(imageFileName.filename().string());
			fileName.replace_extension("ttex");

			auto texture = std::make_unique<Texture2D>();
			if (!texture->create(FileSystem::get().sanitizePath(fileName.string()), cache, loadContent))
			{
				LogError("Texture2D::create() failed for: %s!!", fileName.string().c_str());
				return false;
			}

			size_t samplerIndex = gltfTexture.sampler < numSamplers ? gltfTexture.sampler + 1 : 0;
			texSampMap.insert(std::make_pair(numTextures, samplerIndex));

			texture->setImage(image);
			cache.addResource(std::move(texture));

			numTextures++;
		}

		std::vector<Texture*> textures;
		std::vector<Sampler*> samplers;

		bool hasTextures = cache.hasResource<Texture>();
		bool hasSamplers = cache.hasResource<Sampler>();

		if (hasTextures)
		{
			textures = cache.getResources<Texture>();
		}

		if (hasSamplers)
		{
			samplers = cache.getResources<Sampler>();
		}

		for (const auto& gltfMaterial : gltfModel.materials)
		{
			std::vector<std::string> shaderDefines;
			if (hasSkin)
			{
				shaderDefines.push_back("has_skin");
			}

			auto material = parseMaterial(gltfMaterial, cache, materialsPath, loadContent);
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

			auto shader = createDefaultShader(cache, shaderDefines, loadContent);
			material->setShader(*shader);
			material->setShaderDefines(std::move(shaderDefines));

			cache.addResource(std::move(shader));
			cache.addResource(std::move(material));
		}

		auto defaultShader = createDefaultShader(cache, {}, loadContent);
		auto defaultMaterial = createDefaultMaterial(cache, loadContent);
		defaultMaterial->setShader(*defaultShader);

		cache.addResource(std::move(defaultShader));
		cache.addResource(std::move(defaultMaterial));

		return true;
	}
	
	bool loadSkeleton(const tinygltf::Model& gltfModel, const tinygltf::Scene& gltfScene, const std::string& outputFileName,
		ResourceCache& cache, bool loadContent = true)
	{
		auto jointNames = parseJointNames(gltfModel);
		auto parentMap = getParentMap(gltfModel, gltfScene);
		auto restPose = parseRestPose(gltfModel, parentMap);
		auto bindPose = parseBindPose(gltfModel, parentMap);

		auto fileName = fs::path(outputFileName);
		fileName.replace_extension("tskel");

		auto skeleton = std::make_unique<Skeleton>();
		if (!skeleton->create(FileSystem::get().sanitizePath(fileName.string()), cache, loadContent))
		{
			LogError("Skeleton::create() failed for: %s!!", fileName.string().c_str());
			return false;
		}

		skeleton->setJointNames(std::move(jointNames));
		skeleton->setRestPose(std::move(restPose));
		skeleton->setBindPose(std::move(bindPose));

		if (loadContent)
		{
			skeleton->updateInvBindPose();
		}

		cache.addResource(std::move(skeleton));
		return true;
	}

	bool loadAnimationClips(const tinygltf::Model& gltfModel, const std::string& animationsPath,
		ResourceCache& cache, bool loadContent = true)
	{
		uint32_t animCount = (uint32_t)gltfModel.animations.size();
		std::vector<std::unique_ptr<AnimationClip>> clips(animCount);

		for (uint32_t idx = 0; idx < animCount; idx++)
		{
			auto& gltfAnim = gltfModel.animations[idx];
			auto samplers = getAnimationSamplers(gltfModel, gltfAnim);

			auto fileName = fs::path(animationsPath);
			if (!gltfAnim.name.empty())
			{
				fileName.append(gltfAnim.name);
			}
			else
			{
				auto animations = cache.getResources<AnimationClip>();
				fileName.append(std::format("Animation_{}", animations.size()));
			}

			auto clip = std::make_unique<AnimationClip>();
			if (!clip->create(FileSystem::get().sanitizePath(fileName.string()), cache, loadContent))
			{
				LogError("AnimationClip::create() failed for: %s!!", fileName.string().c_str());
				return false;
			}

			for (auto& channel : gltfAnim.channels)
			{
				auto& transformTrack = (*clip)[(uint32_t)channel.target_node];
				if (channel.target_path == "translation")
				{
					TrackVector& track = transformTrack.getPosition();
					getTrackFromChannel<glm::vec3, 3>(track, channel, samplers);
				}
				else if (channel.target_path == "rotation")
				{
					TrackQuaternion& track = transformTrack.getRotation();
					getTrackFromChannel<glm::quat, 4>(track, channel, samplers);
				}
				else if (channel.target_path == "scale")
				{
					TrackVector& track = transformTrack.getScale();
					getTrackFromChannel<glm::vec3, 3>(track, channel, samplers);
				}
			}

			clip->recalculateDuration();
			clips[idx] = std::move(clip);
		}

		cache.setResources(std::move(clips));
		return true;
	}

	Scene* loadScene(
		tinygltf::Model& gltfModel, 
		ResourceCache& cache,
		const std::string& outputFileName, 
		const std::string& inputPath, 
		const std::string& materialsPath, 
		const std::string& modelsPath, 
		const std::string& imagesPath, 
		const std::string& texturesPath, 
		const std::string& samplersPath, 
		bool loadContent = true)
	{
		auto scene = std::make_unique<Scene>();
		if (!scene->create(outputFileName, cache, loadContent))
		{
			LogError("Scene::create() failed for: %s!!", outputFileName.c_str());
			return nullptr;
		}

		auto lights = parseLights(gltfModel);
		scene->setComponents(std::move(lights));

		if (!loadMaterials(gltfModel, cache, inputPath, materialsPath, imagesPath, 
			texturesPath, samplersPath, false, loadContent))
		{
			LogError("GltfImporter::loadMaterials() faile for: %s!!", outputFileName.c_str());
			return nullptr;
		}

		auto materials = cache.getResources<Material>();
		for (const auto& gltfMesh : gltfModel.meshes)
		{
			auto model = parseMesh(gltfMesh, cache, modelsPath, loadContent);
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

				auto& accessor = gltfModel.accessors[gltfPrimitive.attributes.find("POSITION")->second];
				auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
				size_t numVertices = accessor.count;

				positions = reinterpret_cast<const float*>(&(gltfModel.buffers[bufferView.buffer].
					data[accessor.byteOffset + bufferView.byteOffset]));

				if (gltfPrimitive.attributes.find("NORMAL") != gltfPrimitive.attributes.end())
				{
					accessor = gltfModel.accessors[gltfPrimitive.attributes.find("NORMAL")->second];
					bufferView = gltfModel.bufferViews[accessor.bufferView];
					normals = reinterpret_cast<const float*>(&(gltfModel.buffers[bufferView.buffer].
						data[accessor.byteOffset + bufferView.byteOffset]));
				}

				if (gltfPrimitive.attributes.find("TEXCOORD_0") != gltfPrimitive.attributes.end())
				{
					accessor = gltfModel.accessors[gltfPrimitive.attributes.find("TEXCOORD_0")->second];
					bufferView = gltfModel.bufferViews[accessor.bufferView];
					uvs = reinterpret_cast<const float*>(&(gltfModel.buffers[bufferView.buffer].
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
				const uint32_t totalSize = sizeof(Scene::Vertex) * (uint32_t)vertexData.size();
				auto& vertices = modelMesh.vertexData;

				vertices.resize(totalSize);
				std::memcpy(vertices.data(), vertexData.data(), totalSize);

				modelMesh.numVertices = (uint32_t)numVertices;
				modelMesh.vertexSize = sizeof(Scene::Vertex);

				if (gltfPrimitive.indices >= 0)
				{
					auto numIndices = getAttributeSize(gltfModel, gltfPrimitive.indices);
					auto indexFormat = getIndexFormat(gltfModel, gltfPrimitive.indices);
					auto indexData = getAttributeData(gltfModel, gltfPrimitive.indices);

					if (indexFormat == wgpu::IndexFormat::Uint16)
					{
						indexFormat = wgpu::IndexFormat::Uint32;
						indexData = convertDataStride(indexData, 2, 4);
					}

					const uint32_t totalSize = sizeof(uint32_t) * (uint32_t)numIndices;
					auto& indices = modelMesh.indexData;

					indices.resize(totalSize);
					std::memcpy(indices.data(), indexData.data(), totalSize);

					modelMesh.numIndices = (uint32_t)numIndices;
					modelMesh.indexSize = sizeof(uint32_t);
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

			cache.addResource(std::move(model));
			scene->addComponent(std::move(mesh));
		}

		for (const auto& gltfCamera : gltfModel.cameras)
		{
			auto camera = parseCamera(gltfCamera);
			scene->addComponent(std::move(camera));
		}

		auto meshes = scene->getComponents<Mesh>();
		auto cameras = scene->getComponents<Camera>();

		std::vector<std::unique_ptr<Node>> nodes;
		for (const auto& gltfNode : gltfModel.nodes)
		{
			auto node = parseNode(gltfNode);

			if (gltfNode.mesh >= 0)
			{
				auto mesh = meshes[gltfNode.mesh];

				node->setComponent(*mesh);
				mesh->setNode(*node);
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
		if (gltfModel.defaultScene >= 0 && gltfModel.defaultScene < (int)gltfModel.scenes.size())
		{
			gltfScene = &gltfModel.scenes[gltfModel.defaultScene];
		}
		else
		{
			gltfScene = &gltfModel.scenes[0];
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

			for (auto childNode : gltfModel.nodes[it.second].children)
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

		auto* scenePtr = scene.get();
		cache.addResource(std::move(scene));

		return scenePtr;
	}

	Model* loadModel(
		tinygltf::Model& gltfModel, 
		ResourceCache& cache, 
		const std::string& outputFileName, 
		const std::string& inputPath, 
		const std::string& materialsPath, 
		const std::string& imagesPath, 
		const std::string& texturesPath, 
		const std::string& samplersPath, 
		const std::string& animationsPath = "", 
		bool animated = false, 
		bool loadContent = true)
	{
		if (!loadMaterials(gltfModel, cache, inputPath, materialsPath, imagesPath, 
			texturesPath, samplersPath, animated, loadContent))
		{
			LogError("GltfImporter::loadMaterials() failed for: %s", outputFileName.c_str());
			return nullptr;
		}

		if (animated)
		{
			tinygltf::Scene* gltfScene{ nullptr };
			if (gltfModel.defaultScene >= 0 && gltfModel.defaultScene < (int)gltfModel.scenes.size())
			{
				gltfScene = &gltfModel.scenes[gltfModel.defaultScene];
			}
			else
			{
				gltfScene = &gltfModel.scenes[0];
			}

			if (!gltfScene)
			{
				LogError("No scene found!!");
				return nullptr;
			}

			if (!loadSkeleton(gltfModel, *gltfScene, outputFileName, cache, loadContent))
			{
				LogError("loadSkeleton() failed for: %s!!", outputFileName.c_str());
				return nullptr;
			}

			if (!loadAnimationClips(gltfModel, animationsPath, cache, loadContent))
			{
				LogError("loadAnimationClips() failed for: %s!!", outputFileName.c_str());
				return nullptr;
			}
		}

		auto materials = cache.getResources<Material>();
		auto model = std::make_unique<Model>();

		if (!model->create(FileSystem::get().sanitizePath(outputFileName), cache, loadContent))
		{
			LogError("Model::create() failed for: %s!!", outputFileName.c_str());
			return nullptr;
		}

		std::vector<Model::Mesh> modelMeshes;
		for (auto& gltfNode : gltfModel.nodes)
		{
			if (gltfNode.mesh == -1)
			{
				continue;
			}

			auto& gltfMesh = gltfModel.meshes[gltfNode.mesh];
			for (size_t idx = 0; idx < gltfMesh.primitives.size(); idx++)
			{
				Model::Mesh modelMesh{};

				const auto& gltfPrimitive = gltfMesh.primitives[idx];
				modelMesh.name = std::format("'{}' mesh, primitive #{}", gltfMesh.name, idx);

				const float* positions = nullptr;
				const float* normals = nullptr;
				const float* uvs = nullptr;
				const uint16_t* joints = nullptr;
				const float* weights = nullptr;

				auto& accessor = gltfModel.accessors[gltfPrimitive.attributes.find("POSITION")->second];
				auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
				size_t numVertices = accessor.count;

				positions = reinterpret_cast<const float*>(&(gltfModel.buffers[bufferView.buffer].
					data[accessor.byteOffset + bufferView.byteOffset]));

				if (gltfPrimitive.attributes.find("NORMAL") != gltfPrimitive.attributes.end())
				{
					accessor = gltfModel.accessors[gltfPrimitive.attributes.find("NORMAL")->second];
					bufferView = gltfModel.bufferViews[accessor.bufferView];
					normals = reinterpret_cast<const float*>(&(gltfModel.buffers[bufferView.buffer].
						data[accessor.byteOffset + bufferView.byteOffset]));
				}

				if (gltfPrimitive.attributes.find("TEXCOORD_0") != gltfPrimitive.attributes.end())
				{
					accessor = gltfModel.accessors[gltfPrimitive.attributes.find("TEXCOORD_0")->second];
					bufferView = gltfModel.bufferViews[accessor.bufferView];
					uvs = reinterpret_cast<const float*>(&(gltfModel.buffers[bufferView.buffer].
						data[accessor.byteOffset + bufferView.byteOffset]));
				}

				if (animated && gltfNode.skin != -1)
				{
					if (gltfPrimitive.attributes.find("JOINTS_0") != gltfPrimitive.attributes.end())
					{
						accessor = gltfModel.accessors[gltfPrimitive.attributes.find("JOINTS_0")->second];
						bufferView = gltfModel.bufferViews[accessor.bufferView];
						joints = reinterpret_cast<const uint16_t*>(&(gltfModel.buffers[bufferView.buffer].
							data[accessor.byteOffset + bufferView.byteOffset]));
					}

					if (gltfPrimitive.attributes.find("WEIGHTS_0") != gltfPrimitive.attributes.end())
					{
						accessor = gltfModel.accessors[gltfPrimitive.attributes.find("WEIGHTS_0")->second];
						bufferView = gltfModel.bufferViews[accessor.bufferView];
						weights = reinterpret_cast<const float*>(&(gltfModel.buffers[bufferView.buffer].
							data[accessor.byteOffset + bufferView.byteOffset]));
					}

					auto& gltfSkin = gltfModel.skins[gltfNode.skin];
					std::vector<Scene::VertexSkinned> vertexData;

					for (size_t v = 0; v < numVertices; v++)
					{
						glm::ivec4 joint(
							joints[v * 4 + 0],
							joints[v * 4 + 1],
							joints[v * 4 + 2],
							joints[v * 4 + 3]
						);

						joint.x = joint.x < 0 ? 0 : gltfSkin.joints[joint.x];
						joint.y = joint.y < 0 ? 0 : gltfSkin.joints[joint.y];
						joint.z = joint.z < 0 ? 0 : gltfSkin.joints[joint.z];
						joint.w = joint.w < 0 ? 0 : gltfSkin.joints[joint.w];

						Scene::VertexSkinned vertex{};
						vertex.position = glm::make_vec3(&positions[v * 3]);
						vertex.normal = normals ? glm::normalize(glm::make_vec3(&normals[v * 3])) : glm::vec3(0.0f);
						vertex.uv = uvs ? glm::make_vec2(&uvs[v * 2]) : glm::vec2(0.0f);
						vertex.joint = joints ? glm::vec4(glm::make_vec4(&joint.x)) : glm::vec4(0.0f);
						vertex.weight = weights ? glm::make_vec4(&weights[v * 4]) : glm::vec4(0.0f);

						vertexData.push_back(vertex);
					}

					const uint32_t totalSize = sizeof(Scene::VertexSkinned) * (uint32_t)vertexData.size();
					auto& vertices = modelMesh.vertexData;

					vertices.resize(totalSize);
					std::memcpy(vertices.data(), vertexData.data(), totalSize);

					modelMesh.numVertices = (uint32_t)numVertices;
					modelMesh.vertexSize = sizeof(Scene::VertexSkinned);
				}
				else
				{
					std::vector<Scene::Vertex> vertexData;
					for (size_t v = 0; v < numVertices; v++)
					{
						Scene::Vertex vertex{};
						vertex.position = glm::make_vec3(&positions[v * 3]);
						vertex.normal = normals ? glm::normalize(glm::make_vec3(&normals[v * 3])) : glm::vec3(0.0f);
						vertex.uv = uvs ? glm::make_vec2(&uvs[v * 2]) : glm::vec2(0.0f);

						vertexData.push_back(vertex);
					}

					const uint32_t totalSize = sizeof(Scene::Vertex) * (uint32_t)vertexData.size();
					auto& vertices = modelMesh.vertexData;

					vertices.resize(totalSize);
					std::memcpy(vertices.data(), vertexData.data(), totalSize);

					modelMesh.numVertices = (uint32_t)numVertices;
					modelMesh.vertexSize = sizeof(Scene::Vertex);
				}

				if (gltfPrimitive.indices >= 0)
				{
					auto numIndices = getAttributeSize(gltfModel, gltfPrimitive.indices);
					auto indexFormat = getIndexFormat(gltfModel, gltfPrimitive.indices);
					auto indexData = getAttributeData(gltfModel, gltfPrimitive.indices);

					if (indexFormat == wgpu::IndexFormat::Uint16)
					{
						indexFormat = wgpu::IndexFormat::Uint32;
						indexData = convertDataStride(indexData, 2, 4);
					}

					const uint32_t totalSize = sizeof(uint32_t) * (uint32_t)numIndices;
					auto& indices = modelMesh.indexData;

					indices.resize(totalSize);
					std::memcpy(indices.data(), indexData.data(), totalSize);

					modelMesh.numIndices = (uint32_t)numIndices;
					modelMesh.indexSize = sizeof(uint32_t);
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
		}

		model->setMeshes(std::move(modelMeshes));
		model->setMaterials(std::move(materials));

		if (animated)
		{
			auto* skeleton = cache.getResource<Skeleton>();
			auto clips = cache.getResources<AnimationClip>();

			model->setSkeleton(*skeleton);
			model->setClips(std::move(clips));
		}

		auto* modelPtr = model.get();
		cache.addResource(std::move(model));

		return modelPtr;
	}

	Scene* GltfImporter::importScene(const std::string& inputFileName, const std::string& outputFileName, 
		ResourceCache& cache, bool loadContent)
	{
		auto& fileSystem = FileSystem::get();
		if (!fileSystem.isExist(inputFileName))
		{
			LogError("Input file doesn't exists: %s!!", inputFileName.c_str());
			return nullptr;
		}

		auto file = fileSystem.openFile(inputFileName, FileOpenMode::OpenRead);
		if (!file)
		{
			LogError("FileSystem::openFile() failed for: %s", inputFileName.c_str());
			return nullptr;
		}

		fs::path inputPath{ inputFileName };
		inputPath.remove_filename();

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

		tinygltf::Model gltfModel;
		gltfLoader.LoadASCIIFromString(&gltfModel, &err, &warn, source.c_str(),
			(uint32_t)source.length(), inputPath.string());

		if (!err.empty())
		{
			LogError("Error loading GLTF file: '%s', with error: %s", inputFileName.c_str(), err.c_str());
			return nullptr;
		}

		if (!warn.empty())
		{
			LogWarning("Warning loading GLTF file: %s", warn.c_str());
		}

		auto outputPath = fs::path(outputFileName);
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

		fileSystem.createDirs(outputPath.string());
		fileSystem.createDirs(texturesPath.string());
		fileSystem.createDirs(modelsPath.string());
		fileSystem.createDirs(imagesPath.string());
		fileSystem.createDirs(samplersPath.string());
		fileSystem.createDirs(materialsPath.string());

		auto* scene = loadScene(
			gltfModel, 
			cache,
			outputFileName,
			inputPath.string(), 
			materialsPath.string(),
			modelsPath.string(), 
			imagesPath.string(), 
			texturesPath.string(),
			samplersPath.string(), 
			loadContent);

		if (!scene)
		{
			LogError("GltfImporter::loadScene() failed for: %s!!", inputFileName.c_str());
			return nullptr;
		}

		return scene;
	}

	Model* GltfImporter::importModel(const std::string& inputFileName, const std::string& outputFileName,
		ResourceCache& cache, bool animated, bool loadContent)
	{
		auto& fileSystem = FileSystem::get();
		if (!fileSystem.isExist(inputFileName))
		{
			LogError("Input file doesn't exists: %s!!", inputFileName.c_str());
			return nullptr;
		}

		auto file = fileSystem.openFile(inputFileName, FileOpenMode::OpenRead);
		if (!file)
		{
			LogError("FileSystem::openFile() failed for: %s", inputFileName.c_str());
			return nullptr;
		}

		fs::path inputPath{ inputFileName };
		inputPath.remove_filename();

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

		tinygltf::Model gltfModel;
		gltfLoader.LoadASCIIFromString(&gltfModel, &err, &warn, source.c_str(),
			(uint32_t)source.length(), inputPath.string());

		if (!err.empty())
		{
			LogError("Error loading GLTF file: '%s', with error: %s", inputFileName.c_str(), err.c_str());
			return nullptr;
		}

		if (!warn.empty())
		{
			LogWarning("Warning loading GLTF file: %s", warn.c_str());
		}

		auto outputPath = fs::path(outputFileName);
		outputPath.remove_filename();

		auto texturesPath = outputPath;
		texturesPath.append("Textures");

		auto imagesPath = outputPath;
		imagesPath.append("Images");

		auto samplersPath = outputPath;
		samplersPath.append("Samplers");

		auto materialsPath = outputPath;
		materialsPath.append("Materials");

		auto animationsPath = outputPath;
		animationsPath.append("Animations");

		fileSystem.createDirs(outputPath.string());
		fileSystem.createDirs(texturesPath.string());
		fileSystem.createDirs(imagesPath.string());
		fileSystem.createDirs(samplersPath.string());
		fileSystem.createDirs(materialsPath.string());
		fileSystem.createDirs(animationsPath.string());

		auto* model = loadModel(
			gltfModel, 
			cache, 
			outputFileName, 
			inputPath.string(), 
			materialsPath.string(),
			imagesPath.string(), 
			texturesPath.string(), 
			samplersPath.string(),
			animationsPath.string(), 
			animated, 
			loadContent);

		if (!model)
		{
			LogError("GltfImporter::loadModel() failed for: %s!!", inputFileName.c_str());
			return nullptr;
		}

		return model;
	}

	Skeleton* GltfImporter::importSkeleton(const std::string& inputFileName, const std::string& outputFileName, 
		ResourceCache& cache, bool loadContent)
	{
		auto& fileSystem = FileSystem::get();
		if (!fileSystem.isExist(inputFileName))
		{
			LogError("Input file doesn't exists: %s!!", inputFileName.c_str());
			return nullptr;
		}

		auto file = fileSystem.openFile(inputFileName, FileOpenMode::OpenRead);
		if (!file)
		{
			LogError("FileSystem::openFile() failed for: %s", inputFileName.c_str());
			return nullptr;
		}

		fs::path inputPath{ inputFileName };
		inputPath.remove_filename();

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

		tinygltf::Model gltfModel;
		gltfLoader.LoadASCIIFromString(&gltfModel, &err, &warn, source.c_str(),
			(uint32_t)source.length(), inputPath.string());

		if (!err.empty())
		{
			LogError("Error loading GLTF file: '%s', with error: %s", inputFileName.c_str(), err.c_str());
			return nullptr;
		}

		if (!warn.empty())
		{
			LogWarning("Warning loading GLTF file: %s", warn.c_str());
		}

		auto outputPath = fs::path(outputFileName);
		outputPath.remove_filename();
		fileSystem.createDirs(outputPath.string());

		tinygltf::Scene* gltfScene{ nullptr };
		if (gltfModel.defaultScene >= 0 && gltfModel.defaultScene < (int)gltfModel.scenes.size())
		{
			gltfScene = &gltfModel.scenes[gltfModel.defaultScene];
		}
		else
		{
			gltfScene = &gltfModel.scenes[0];
		}

		if (!gltfScene)
		{
			LogError("No scene found!!");
			return nullptr;
		}

		if (!loadSkeleton(gltfModel, *gltfScene, outputFileName, cache, loadContent))
		{
			LogError("loadSkeleton() failed for: %s!!", outputFileName.c_str());
			return nullptr;
		}

		return cache.getResource<Skeleton>();
	}

	AnimationClip* GltfImporter::importAnimation(const std::string& inputFileName, const std::string& outputFileName, 
		ResourceCache& cache, bool loadContent)
	{
		auto& fileSystem = FileSystem::get();
		if (!fileSystem.isExist(inputFileName))
		{
			LogError("Input file doesn't exists: %s!!", inputFileName.c_str());
			return nullptr;
		}

		auto file = fileSystem.openFile(inputFileName, FileOpenMode::OpenRead);
		if (!file)
		{
			LogError("FileSystem::openFile() failed for: %s", inputFileName.c_str());
			return nullptr;
		}

		fs::path inputPath{ inputFileName };
		inputPath.remove_filename();

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

		tinygltf::Model gltfModel;
		gltfLoader.LoadASCIIFromString(&gltfModel, &err, &warn, source.c_str(),
			(uint32_t)source.length(), inputPath.string());

		if (!err.empty())
		{
			LogError("Error loading GLTF file: '%s', with error: %s", inputFileName.c_str(), err.c_str());
			return nullptr;
		}

		if (!warn.empty())
		{
			LogWarning("Warning loading GLTF file: %s", warn.c_str());
		}

		auto outputPath = fs::path(outputFileName);
		outputPath.remove_filename();

		auto animationsPath = outputPath;
		animationsPath.append("Animations");

		fileSystem.createDirs(outputPath.string());
		fileSystem.createDirs(animationsPath.string());

		tinygltf::Scene* gltfScene{ nullptr };
		if (gltfModel.defaultScene >= 0 && gltfModel.defaultScene < (int)gltfModel.scenes.size())
		{
			gltfScene = &gltfModel.scenes[gltfModel.defaultScene];
		}
		else
		{
			gltfScene = &gltfModel.scenes[0];
		}

		if (!gltfScene)
		{
			LogError("No scene found!!");
			return nullptr;
		}

		if (!loadAnimationClips(gltfModel, animationsPath.string(), cache, loadContent))
		{
			LogError("loadAnimationClips() failed for: %s!!", outputFileName.c_str());
			return nullptr;
		}

		return cache.getResource<AnimationClip>();
	}
}