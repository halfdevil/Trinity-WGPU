#include "Core/Application.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"
#include "Core/Clock.h"
#include "Core/Window.h"
#include "Core/ResourceCache.h"
#include "VFS/FileSystem.h"
#include "VFS/DiskFile.h"
#include "Input/Input.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/SwapChain.h"
#include "Graphics/RenderPass.h"
#include "Gui/GuiRenderer.h"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace Trinity
{
	Application::~Application()
	{
	}

	void Application::run(const ApplicationOptions& options)
	{
		mOptions = options;

		mLogger = std::make_unique<Logger>();
		mLogger->setMaxLogLevel(options.logLevel);

		mDebugger = std::make_unique<Debugger>();
		mClock = std::make_unique<Clock>();
		mFileSystem = std::make_unique<FileSystem>();
		mInput = std::make_unique<Input>();
		mWindow = std::make_unique<Window>();
		mGraphicsDevice = std::make_unique<GraphicsDevice>();

		if (!Window::initialize())
		{
			LogFatal("Window::initialize() failed!!");
			return;
		}

		if (!mWindow->create(options.title, options.width, options.height, options.displayMode))
		{
			LogFatal("Window::create() failed!!");
			return;
		}

		mGraphicsDevice->onCreated.subscribe([this](bool result) {
			if (result)
			{
				mClock->reset();
				mWindow->show(!mOptions.headless);

				if (init())
				{
					setupInput();
#ifdef __EMSCRIPTEN__
					static Application* app = this;
					emscripten_set_main_loop_arg([](void* arg) {
						app->frame();
					}, this, 0, false);
#else
					while (!mWindow->isClosed())
					{
						frame();
						mWindow->poll();
					}
#endif
				}
			}
		});

		mGraphicsDevice->create(*mWindow);
	}

	bool Application::init()
	{
		if (!mFileSystem->addFolder("/Assets", "Assets"))
		{
			LogError("FileSystem::addFolder() failed!!");
			return false;
		}

		if (!mOptions.configFile.empty())
		{
			DiskFile configFile;
			if (!configFile.create(mOptions.configFile, mOptions.configFile, FileOpenMode::OpenRead))
			{
				LogError("PhysicalFile::create() failed for: %s!!", mOptions.configFile.c_str());
				return false;
			}

			FileReader reader(configFile);
			mConfig = json::parse(reader.readAsString());

			if (mConfig.contains("folders"))
			{
				for (auto& folder : mConfig["folders"])
				{
					const std::string folderAlias = folder["alias"].get<std::string>();
					const std::string folderPath = folder["path"].get<std::string>();

					if (!mFileSystem->addFolder(folderAlias, folderPath))
					{
						LogError("FileSystem::addFolder() failed for: %s!!", folderPath.c_str());
						return false;
					}
				}
			}

			if (mConfig.contains("input"))
			{
				if (!mInput->loadConfig(mConfig["input"]))
				{
					LogError("Input::loadConfig() failed!!");
					return false;
				}
			}
		}

		if (!mInput->create(*mWindow))
		{
			LogError("Input::create() failed!!");
			return false;
		}

		WindowCallbacks& callbacks = mWindow->getCallbacks();

		callbacks.onClose.subscribe([this]() {
			onClose();
		});

		callbacks.onResize.subscribe([this](int32_t, int32_t) {
			onResize();
		});

		onResize();

		mGraphicsDevice->setClearColor({ 0.5f, 0.5f, 0.5f, 1.0f });
		mWindow->showMouse(true, false);
		mMainPass = std::make_unique<RenderPass>();

		mGuiRenderer = std::make_unique<GuiRenderer>();
		if (!mGuiRenderer->create(*mWindow, "/Assets/Fonts/CascadiaCode.ttf"))
		{
			LogError("Gui::create() failed!!");
			return false;
		}

		return true;
	}

	void Application::update(float deltaTime)
	{		
	}

	void Application::render(float deltaTime)
	{
	}

	void Application::frame()
	{
		mClock->update();
		mInput->update();
		update(mClock->getDeltaTime());
		mInput->postUpdate();

		mGraphicsDevice->beginFrame();
		render(mClock->getDeltaTime());
		mGraphicsDevice->endFrame();
		mGraphicsDevice->present();
	}

	void Application::exit()
	{
		mWindow->close();
	}

	void Application::drawGui(float deltaTime)
	{
		mGuiRenderer->newFrame(*mWindow, deltaTime);
		onGui();
		mGuiRenderer->draw(ImGui::GetDrawData(), *mMainPass);
	}

	void Application::onClose()
	{
	}

	void Application::onResize()
	{
		if (!mGraphicsDevice->setupSwapChain(*mWindow, wgpu::PresentMode::Fifo,
			wgpu::TextureFormat::Depth32Float))
		{
			LogError("GraphicsDevice::setupSwapChain() failed!!");
		}
	}

	void Application::onGui()
	{
	}

	void Application::setupInput()
	{
	}
}