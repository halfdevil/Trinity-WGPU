#include "RenderPassSample.h"
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
		mMainPass->submit();
    }

	void RenderPassSample::onSceneLoaded()
	{
		SampleApplication::onSceneLoaded();

		if (mScene != nullptr)
		{
			mScene->addPerspectiveCamera("default_camera", 1.77f, 1.0f, 0.1f, 10000.0f, glm::vec3(0.0f));
			mScene->addDirectionalLight(glm::quat({ glm::radians(0.0f), 0.0f, glm::radians(0.0f) }));

			mCamera = mScene->addFreeCameraScript("default_camera", mWindow->getSize());
			if (mCamera != nullptr)
			{
				auto& transform = mCamera->getNode()->getTransform();

				glm::vec3 translation{ 0.0f, 500.0f, 1000.0f };
				glm::quat qx = glm::angleAxis(glm::radians(135.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				glm::quat qy = glm::angleAxis(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::quat orientation = glm::normalize(qy * qx);

				transform.setTranslation(transform.getTranslation() + translation * glm::conjugate(orientation));
				transform.setRotation(orientation);
			}
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