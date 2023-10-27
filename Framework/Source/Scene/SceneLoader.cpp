#include "Scene/SceneLoader.h"
#include "Scene/Scene.h"
#include "Scene/ComponentFactory.h"
#include "Core/Logger.h"
#include "Core/ResourceCache.h"
#include "VFS/FileSystem.h"
#include <format>
#include <queue>

namespace Trinity
{	
	std::unique_ptr<Scene> SceneLoader::loadScene(const std::string& fileName, ResourceCache& cache)
	{
		auto scene = std::make_unique<Scene>();
		if (!scene->create(fileName, cache))
		{
			LogError("Scene::create() failed for: %s!!", fileName.c_str());
			return nullptr;
		}

		return scene;
	}

	std::unique_ptr<Scene> SceneLoader::loadSceneWithModel(const std::string& fileName, ResourceCache& cache)
	{
		auto scene = std::make_unique<Scene>();		
		if (!scene->create("", cache, false))
		{
			LogError("Scene::create() failed for model '%s'", fileName.c_str());
			return nullptr;
		}

		auto rootNode = std::make_unique<Node>();
		rootNode->setName("root");

		scene->setRoot(*rootNode);
		scene->addNode(std::move(rootNode));

		auto* mesh = scene->addMesh("mesh_node", fileName, cache, glm::vec3(1.0f));
		if (!mesh)
		{
			LogError("Scene::addMesh() failed for model '%s'", fileName.c_str());
			return nullptr;
		}

		scene->addPerspectiveCamera("default_camera", 1.77f, 1.0f, 0.1f, 10000.0f, glm::vec3(0.0f));
		scene->addDirectionalLight(glm::quat({ glm::radians(0.0f), 0.0f, glm::radians(0.0f) }));

		return scene;
	}
}