#pragma once

#include <memory>
#include <mutex>

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

		std::unique_ptr<Scene> loadScene(const std::string& fileName);
	};
}