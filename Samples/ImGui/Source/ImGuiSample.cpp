#include "ImGuiSample.h"
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
#include "Scene/ComponentFactory.h"
#include "Gui/ImGuiRenderer.h"
#include <glm/glm.hpp>

namespace Trinity
{
	bool ImGuiSample::init()
	{
		if (!SampleApplication::init())
		{
			return false;
		}

		setupInput();
		return true;
	}

	void ImGuiSample::onGui()
	{
		ImGui::ShowDemoWindow();
	}

	void ImGuiSample::setupInput()
	{
		SampleApplication::setupInput();

		mInput->bindAction("exit", InputEvent::Pressed, [this](int32_t key) {
			exit();
		});
	}
}

using namespace Trinity;

int main(int argc, char* argv[])
{
	static ImGuiSample app;
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