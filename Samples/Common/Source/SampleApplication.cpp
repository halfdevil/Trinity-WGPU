#include "SampleApplication.h"
#include "Scene/Scene.h"
#include "Scene/SceneLoader.h"
#include "Scene/SceneRenderer.h"
#include "Scene/ComponentFactory.h"
#include "Scene/Components/Script.h"
#include "Graphics/RenderPass.h"
#include "Core/Logger.h"
#include "Core/ResourceCache.h"

namespace Trinity
{
	bool SampleApplication::init()
	{
		if (!Application::init())
		{
			return false;
		}

		if (mConfig.contains("scene"))
		{
			std::string sceneFile = mConfig["scene"].get<std::string>();

			SceneLoader sceneLoader;
			mScene = sceneLoader.loadScene(sceneFile, *mResourceCache);

			if (!mScene)
			{
				LogError("SceneLoader::loadScene() failed for '%s'", sceneFile.c_str());
				return false;
			}

			onSceneLoaded();
		}
		else if (mConfig.contains("model"))
		{
			std::string modelFile = mConfig["model"].get<std::string>();

			SceneLoader sceneLoader;
			mScene = sceneLoader.loadSceneWithModel(modelFile, *mResourceCache);

			if (!mScene)
			{
				LogError("SceneLoader::loadScene() failed for model '%s'", modelFile.c_str());
				return false;
			}

			onSceneLoaded();
		}

		if (mScene != nullptr)
		{
			mScripts = mScene->getComponents<Script>();
			for (auto& script : mScripts)
			{
				script->init();
			}

			mSceneRenderer = std::make_unique<SceneRenderer>();
			if (!mSceneRenderer->prepare(*mScene, *mResourceCache))
			{
				LogError("SceneRenderer::prepare() failed!!");
				return false;
			}
		}

		return true;
	}

	void SampleApplication::update(float deltaTime)
	{
		Application::update(deltaTime);

		if (mScene != nullptr)
		{
			for (auto& script : mScripts)
			{
				script->update(deltaTime);
			}
		}
	}

	void SampleApplication::onResize()
	{
		Application::onResize();

		if (mScene != nullptr)
		{
			for (auto& script : mScripts)
			{
				script->resize(mWindow->getWidth(), mWindow->getHeight());
			}
		}
	}

	void SampleApplication::onSceneLoaded()
	{
	}
}