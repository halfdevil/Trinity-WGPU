#include "RenderPassSample.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"
#include "Core/Clock.h"
#include "Core/Window.h"
#include "VFS/FileSystem.h"
#include "Input/Input.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/SwapChain.h"
#include "Graphics/RenderPass.h"
#include "Scene/Scene.h"
#include "Scene/SceneRenderer.h"
#include <glm/glm.hpp>

namespace Trinity
{
	bool RenderPassSample::init()
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

	void RenderPassSample::render(float deltaTime)
    {
        mMainPass->begin();
		
		if (mScene != nullptr)
		{
			mSceneRenderer->draw(*mMainPass);
		}

        mMainPass->end();
    }

	void RenderPassSample::onSceneLoaded()
	{
		SampleApplication::onSceneLoaded();

		if (mScene != nullptr)
		{
			auto& freeCamera = mScene->addFreeCamera("default_camera", mWindow->getSize());
			mCamera = &freeCamera;
		}
	}

	void RenderPassSample::setupInput()
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

	void RenderPassSample::moveForward(float scale)
	{
		if (mCamera != nullptr)
		{
			mCamera->moveForward(scale);
		}
	}

	void RenderPassSample::moveRight(float scale)
	{
		if (mCamera != nullptr)
		{
			mCamera->moveRight(scale);
		}
	}

	void RenderPassSample::moveUp(float scale)
	{
		if (mCamera != nullptr)
		{
			mCamera->moveUp(scale);
		}
	}

	void RenderPassSample::turn(float scale)
	{
		if (mCamera != nullptr)
		{
			mCamera->turn(scale);
		}
	}

	void RenderPassSample::lookUp(float scale)
	{
		if (mCamera != nullptr)
		{
			mCamera->lookUp(scale);
		}
	}
}

int main(int argc, char* argv[])
{
    static Trinity::RenderPassSample app;
    app.run({
        .title = "Trinity - Render Pass",
#ifdef __EMSCRIPTEN__
		.configFile = "/Assets/AppConfig.json",
#else
		.configFile = "AppConfig.json",
#endif
    });

    return 0;
}