#include "SampleApplication.h"
#include "Scene/Scene.h"
#include "Scene/SceneLoader.h"
#include "Scene/Components/Script.h"

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
			mScene = sceneLoader.loadScene(sceneFile);
		}

		if (mScene != nullptr)
		{
			mScripts = mScene->getComponents<NodeScript>();
			for (auto& script : mScripts)
			{
				script->init();
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
}