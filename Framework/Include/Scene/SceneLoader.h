#pragma once

#include <memory>
#include <mutex>

namespace Trinity
{
	class Scene;
	class ResourceCache;

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

		std::unique_ptr<Scene> loadScene(const std::string& fileName, ResourceCache& cache);
		std::unique_ptr<Scene> loadSceneWithModel(const std::string& fileName, ResourceCache& cache);
		std::unique_ptr<Scene> loadEmptyScene(ResourceCache& cache);
	};
}