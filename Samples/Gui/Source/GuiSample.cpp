#include "GuiSample.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"
#include "Core/Clock.h"
#include "Core/Window.h"
#include "Core/ResourceCache.h"
#include "VFS/FileSystem.h"
#include "Input/Input.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Texture2D.h"
#include "Scene/Scene.h"
#include "Scene/SceneRenderer.h"
#include "Gui/GuiRenderer.h"
#include "Gui/Font.h"
#include <glm/glm.hpp>

namespace Trinity
{
	bool GuiSample::init()
	{
		if (!SampleApplication::init())
		{
			return false;
		}

		setupInput();
		return true;
	}

	void GuiSample::render(float deltaTime)
	{
		mMainPass->begin();		
		drawGui(deltaTime);
		mMainPass->end();
	}

	void GuiSample::onGui()
	{
		ImGui::ShowDemoWindow();
		ImGui::Render();
	}

	void GuiSample::setupInput()
	{
		mInput->bindAction("exit", InputEvent::Pressed, [this](int32_t key) {
			exit();
		});
	}
}

using namespace Trinity;

int main(int argc, char* argv[])
{
	static GuiSample app;
	app.run({
		.title = "Trinity - Gui",
#ifdef __EMSCRIPTEN__
		.configFile = "/Assets/AppConfig.json",
#else
		.configFile = "AppConfig.json",
#endif
	});

	return 0;
}