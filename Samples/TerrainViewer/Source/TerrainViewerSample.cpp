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

		return true;
	}

	void TerrainViewerSample::render(float deltaTime)
    {
        mMainPass->begin();
		
		if (mScene != nullptr)
		{
			mSceneRenderer->draw(*mMainPass);
		}

		if (mTerrain != nullptr)
		{
			mTerrainRenderer->draw(*mMainPass);
		}

        mMainPass->end();
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