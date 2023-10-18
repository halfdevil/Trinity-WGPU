#pragma once

#include "Core/ConsoleApplication.h"
#include <string>
#include <vector>

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

namespace Trinity
{
	class Scene;
	class Light;
	class Node;
	class Camera;
	class Model;
	class Material;
	class Image;
	class Sampler;
	class Shader;
	class ResourceCache;

	template <typename T, typename Y>
	struct TypeCast
	{
		Y operator()(T value) const noexcept
		{
			return static_cast<Y>(value);
		}
	};

	class SceneConverter : public ConsoleApplication
	{
	public:

		SceneConverter() = default;
		~SceneConverter();

		SceneConverter(const SceneConverter&) = delete;
		SceneConverter& operator = (const SceneConverter&) = delete;

		SceneConverter(SceneConverter&&) noexcept = default;
		SceneConverter& operator = (SceneConverter&&) noexcept = default;

		void setFileName(const std::string& fileName);
		void setOutputFileName(const std::string& fileName);

	protected:

		virtual void execute() override;

		std::unique_ptr<Scene> loadScene();
		std::vector<std::unique_ptr<Light>> parseLights();
		std::unique_ptr<Node> parseNode(const tinygltf::Node& gltfNode);
		std::unique_ptr<Camera> parseCamera(const tinygltf::Camera& gltfCamera);
		std::unique_ptr<Model> parseMesh(const tinygltf::Mesh& gltfMesh, ResourceCache& cache);
		std::unique_ptr<Material> parseMaterial(const tinygltf::Material& gltfMaterial, ResourceCache& cache);
		std::unique_ptr<Image> parseImage(const tinygltf::Image& gltfImage, ResourceCache& cache);
		std::unique_ptr<Sampler> parseSampler(const tinygltf::Sampler& gltfSampler, ResourceCache& cache);
		std::unique_ptr<Shader> createDefaultShader(ResourceCache& cache);
		std::unique_ptr<Material> createDefaultMaterial(ResourceCache& cache);
		std::unique_ptr<Sampler> createDefaultSampler(ResourceCache& cache);
		std::unique_ptr<Camera> createDefaultCamera();

	private:

		std::string mFileName;
		std::string mOutputFileName;
		std::string mInputPath;
		std::string mOutputPath;
		std::string mTexturesPath;
		std::string mModelsPath;
		std::string mImagesPath;
		std::string mSamplersPath;
		std::string mMaterialsPath;
		tinygltf::Model mModel;
	};
}