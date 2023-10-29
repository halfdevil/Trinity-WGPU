#include "ModelViewerSample.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"
#include "Core/Clock.h"
#include "Core/Window.h"
#include "Core/ResourceCache.h"
#include "VFS/FileSystem.h"
#include "Input/Input.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/SwapChain.h"
#include "Graphics/RenderPass.h"
#include "Gui/ImGuiRenderer.h"
#include "Scene/Scene.h"
#include "Scene/SceneRenderer.h"
#include "Scene/ComponentFactory.h"
#include "Scene/Components/Scripts/FreeCamera.h"
#include "Scene/Components/Scripts/Animator.h"
#include <glm/glm.hpp>

namespace Trinity
{
	bool ModelViewerSample::init()
	{
		if (!SampleApplication::init())
		{
			return false;
		}

		setupInput();

		mSceneRenderer->setCamera("default_camera");
		mCamera->moveSpeed = 0.1f;
		mCamera->rotationSpeed = 0.001f;

		return true;
	}

	void ModelViewerSample::render(float deltaTime)
    {
        mMainPass->begin();
		
		if (mScene != nullptr)
		{
			mSceneRenderer->draw(*mMainPass);
		}

        mMainPass->end();
    }

	void ModelViewerSample::onSceneLoaded()
	{
		SampleApplication::onSceneLoaded();

		if (mScene != nullptr)
		{
			mScene->addPerspectiveCamera("default_camera", 1.77f, 1.0f, 0.1f, 10000.0f, glm::vec3(0.0f));
			mScene->addDirectionalLight(glm::quat({ glm::radians(0.0f), 0.0f, glm::radians(0.0f) }));

			auto* meshNode = mScene->findNode("mesh_node");
			if (meshNode != nullptr)
			{
				meshNode->getTransform().setScale(glm::vec3{ 30.0f });
			}

			mCamera = mScene->addFreeCameraScript("default_camera", mWindow->getSize());
			if (mCamera != nullptr)
			{
				mCamera->getNode()->getTransform().setTranslation(glm::vec3(0.0f, 30.0f, 70.0f));
			}

			mAnimator = mScene->addAnimatorScript("mesh_node");
			if (mAnimator != nullptr)
			{
				mAnimator->setCurrentClip(0);
				mAnimator->setLooping(true);
			}
		}
	}

	void ModelViewerSample::setupInput()
	{		
		mInput->bindAction("exit", InputEvent::Pressed, [this](int32_t key) {
            exit();
		});

		mInput->bindAxis("moveForward", [this](float scale) {
			moveForward(scale);
		});

		mInput->bindAxis("moveRight", [this](float scale) {
			moveRight(scale);
		}); 
		
		mInput->bindAxis("moveUp", [this](float scale) {
			moveUp(scale);
		});

		mInput->bindAxis("turn", [this](float scale) {
			turn(scale);
		});

		mInput->bindAxis("lookUp", [this](float scale) {
			lookUp(scale);
		});
	}

	void ModelViewerSample::moveForward(float scale)
	{
		if (mCamera != nullptr)
		{
			mCamera->moveForward(scale);
		}
	}

	void ModelViewerSample::moveRight(float scale)
	{
		if (mCamera != nullptr)
		{
			mCamera->moveRight(scale);
		}
	}

	void ModelViewerSample::moveUp(float scale)
	{
		if (mCamera != nullptr)
		{
			mCamera->moveUp(scale);
		}
	}

	void ModelViewerSample::turn(float scale)
	{
		if (mCamera != nullptr)
		{
			mCamera->turn(scale);
		}
	}

	void ModelViewerSample::lookUp(float scale)
	{
		if (mCamera != nullptr)
		{
			mCamera->lookUp(scale);
		}
	}
}

int main(int argc, char* argv[])
{
    static Trinity::ModelViewerSample app;
    app.run({
        .title = "Trinity - Model Viewer",
#ifdef __EMSCRIPTEN__
		.configFile = "/Assets/AppConfig.json",
#else
		.configFile = "AppConfig.json",
#endif
    });

    return 0;
}