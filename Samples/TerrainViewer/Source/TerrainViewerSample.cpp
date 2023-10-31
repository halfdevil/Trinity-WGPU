#include "TerrainViewerSample.h"
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
#include "Scene/Terrain/Terrain.h"
#include "Scene/Terrain/TerrainRenderer.h"
#include "Scene/Terrain/QuadTree.h"
#include "Scene/Skybox/Skybox.h"
#include "Scene/Skybox/SkyboxRenderer.h"
#include <glm/glm.hpp>

namespace Trinity
{
	bool TerrainViewerSample::init()
	{
		if (!SampleApplication::init())
		{
			return false;
		}

		setupInput();

		mSceneRenderer->setCamera("default_camera");
		mCamera->moveSpeed = 0.1f;
		mCamera->rotationSpeed = 0.001f;

		if (mConfig.contains("terrain"))
		{
			auto terrainFile = mConfig["terrain"].get<std::string>();
			mTerrain = std::make_unique<Terrain>();

			if (!mTerrain->create(terrainFile, *mResourceCache))
			{
				LogError("Terrain::create() failed for: '%s'", terrainFile.c_str());
				return false;
			}

			mTerrainRenderer = std::make_unique<TerrainRenderer>();
			if (!mTerrainRenderer->prepare(*mTerrain, *mScene, *mResourceCache))
			{
				LogError("TerrainRenderer::prepare() failed for: '%s'", terrainFile.c_str());
				return false;
			}

			mTerrainRenderer->setCamera("default_camera");
		}

		if (mConfig.contains("skybox"))
		{
			auto skyboxFile = mConfig["skybox"].get<std::string>();
			mSkybox = std::make_unique<Skybox>();

			if (!mSkybox->create(skyboxFile, *mResourceCache))
			{
				LogError("Skybox::create() failed for: '%s'", skyboxFile.c_str());
				return false;
			}

			mSkyboxRenderer = std::make_unique<SkyboxRenderer>();
			if (!mSkyboxRenderer->prepare(*mSkybox, *mCamera->getCamera(), *mResourceCache))
			{
				LogError("SkyboxRenderer::prepare() failed for: '%s'", skyboxFile.c_str());
				return false;
			}
		}

		return true;
	}

	void TerrainViewerSample::render(float deltaTime)
    {
        mMainPass->begin();

		if (mSkybox != nullptr)
		{
			mSkybox->setPosition(mCamera->getNode()->getTransform().getTranslation());
			mSkyboxRenderer->draw(*mMainPass);
		}
		
		if (mScene != nullptr)
		{
			mSceneRenderer->draw(*mMainPass);
		}

		if (mTerrain != nullptr)
		{
			mTerrainRenderer->draw(*mMainPass);
		}

        mMainPass->end();
		mMainPass->submit();
    }

	void TerrainViewerSample::onSceneLoaded()
	{
		SampleApplication::onSceneLoaded();

		if (mScene != nullptr)
		{
			mScene->addPerspectiveCamera("default_camera", 1.77f, 1.0f, 0.1f, 10000.0f, glm::vec3(0.0f));
			mScene->addDirectionalLight(glm::quat({ glm::radians(0.0f), 0.0f, glm::radians(0.0f) }));

			mCamera = mScene->addFreeCameraScript("default_camera", mWindow->getSize());
			if (mCamera != nullptr)
			{
				mCamera->getNode()->getTransform().setTranslation(glm::vec3(0.0f, 120.0f, 0.0f));
			}
		}
	}

	void TerrainViewerSample::setupInput()
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

	void TerrainViewerSample::moveForward(float scale)
	{
		if (mCamera != nullptr)
		{
			mCamera->moveForward(scale);
		}
	}

	void TerrainViewerSample::moveRight(float scale)
	{
		if (mCamera != nullptr)
		{
			mCamera->moveRight(scale);
		}
	}

	void TerrainViewerSample::moveUp(float scale)
	{
		if (mCamera != nullptr)
		{
			mCamera->moveUp(scale);
		}
	}

	void TerrainViewerSample::turn(float scale)
	{
		if (mCamera != nullptr)
		{
			mCamera->turn(scale);
		}
	}

	void TerrainViewerSample::lookUp(float scale)
	{
		if (mCamera != nullptr)
		{
			mCamera->lookUp(scale);
		}
	}
}

int main(int argc, char* argv[])
{
    static Trinity::TerrainViewerSample app;
    app.run({
        .title = "Trinity - Terrain Viewer",
#ifdef __EMSCRIPTEN__
		.configFile = "/Assets/AppConfig.json",
#else
		.configFile = "AppConfig.json",
#endif
    });

    return 0;
}