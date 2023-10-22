#include "Scene/SceneLoader.h"
#include "Scene/Scene.h"
#include "Scene/ComponentFactory.h"
#include "Core/Logger.h"
#include "Core/ResourceCache.h"
#include <format>
#include <queue>

namespace Trinity
{	
	std::unique_ptr<Scene> SceneLoader::loadScene(const std::string& fileName)
	{
		LogError("SceneLoader::loadScene() started");

		auto scene = std::make_unique<Scene>();
		if (!scene->create(fileName))
		{
			LogError("Scene::create() failed for: %s!!", fileName.c_str());
			return nullptr;
		}

		LogError("SceneLoader::loadScene() completed");
		return scene;
	}
}