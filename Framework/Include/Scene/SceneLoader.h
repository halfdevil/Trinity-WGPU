#pragma once

#include <memory>
#include <mutex>

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

namespace Trinity
{
	class Camera;
	class Light;
	class Mesh;
	class Node;
	class PBRMaterial;
	class Scene;
	class SubMesh;
	class Image;
	class Sampler;
	class Texture;
	class BindGroup;
	class Shader;

	template <typename T, typename Y>
	struct TypeCast
	{
		Y operator()(T value) const noexcept
		{
			return static_cast<Y>(value);
		}
	};

	class SceneLoader
	{
	public:

		SceneLoader() = default;
		virtual ~SceneLoader() = default;

		SceneLoader(const SceneLoader&) = delete;
		SceneLoader& operator = (const SceneLoader&) = delete;

		SceneLoader(SceneLoader&&) = default;
		SceneLoader& operator = (SceneLoader&&) = default;

		std::unique_ptr<Scene> readScene(const std::string& fileName);

	protected:

		virtual std::unique_ptr<Node> parseNode(const tinygltf::Node& gltfNode) const;
		virtual std::unique_ptr<Camera> parseCamera(const tinygltf::Camera& gltfCamera) const;
		virtual std::unique_ptr<Mesh> parseMesh(const tinygltf::Mesh& gltfMesh) const;
		virtual std::unique_ptr<PBRMaterial> parseMaterial(const tinygltf::Material& gltfMaterial) const;
		virtual std::unique_ptr<Image> parseImage(const tinygltf::Image& gltfImage) const;
		virtual std::unique_ptr<Sampler> parseSampler(const tinygltf::Sampler& gltfSampler) const;
		virtual std::unique_ptr<Texture> parseTexture(const tinygltf::Texture& gltfTexture) const;

		virtual std::unique_ptr<Shader> createDefaultShader(const std::vector<std::string>& defines) const;
		virtual std::unique_ptr<PBRMaterial> createDefaultMaterial() const;
		virtual std::unique_ptr<Sampler> createDefaultSampler() const;
		virtual std::unique_ptr<Camera> createDefaultCamera() const;
		virtual Scene loadScene();

	protected:

		tinygltf::Model mModel;
		std::string mModelPath;
	};
}