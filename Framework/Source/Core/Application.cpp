#include "Core/Application.h"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace Trinity
{
	void Application::run(const std::string& title, uint32_t width, uint32_t height, DisplayMode displayMode)
	{
#if DEBUG_BUILD
		mLogger.setMaxLogLevel(LogLevel::Debug);
#else
		mLogger.setMaxLogLevel(LogLevel::Error);
#endif

		if (!Window::initialize())
		{
			LogFatal("Window::initialize() failed!!");
			return;
		}

		if (!mWindow.create(title, width, height, displayMode))
		{
			LogFatal("Window::create() failed!!");
			return;
		}

		mGraphicsDevice.onCreated.subscribe([this](bool result) {
			if (result)
			{
				mClock.reset();
				mWindow.show(true);

				if (init())
				{
					setupInput();
#ifdef __EMSCRIPTEN__
					static Application* app = this;
					emscripten_set_main_loop_arg([](void* arg) {
						app->frame();
						}, this, 0, false);
#else
					while (!mWindow.isClosed())
					{
						frame();
						mWindow.poll();
					}
#endif
				}
			}
		});

		mGraphicsDevice.create(mWindow);
	}

	bool Application::init()
	{
		if (!mFileSystem.addFolder("/Assets", "Assets"))
		{
			LogError("FileSystem::addFolder() failed!!");
			return false;
		}

		if (!mInput.create(mWindow))
		{
			LogError("Input::create() failed!!");
			return false;
		}

		WindowCallbacks& callbacks = mWindow.getCallbacks();

		callbacks.onClose.subscribe([this]() {
			onClose();
			});

		callbacks.onResize.subscribe([this](int32_t, int32_t) {
			onResize();
			});

		onResize();

		mGraphicsDevice.setClearColor({ 0.5f, 0.5f, 0.5f, 1.0f });
		mWindow.showMouse(true, false);

		return true;
	}

	void Application::update()
	{		
	}

	void Application::render()
	{
	}

	void Application::frame()
	{
		mClock.update();
		mInput.update();
		update();
		mInput.postUpdate();

		mGraphicsDevice.beginFrame();
		render();
		mGraphicsDevice.endFrame();
		mGraphicsDevice.present();
	}

	void Application::exit()
	{
		mWindow.close();
	}

	void Application::onClose()
	{
	}

	void Application::onResize()
	{
		if (!mGraphicsDevice.setupSwapChain(mWindow, wgpu::PresentMode::Fifo,
			wgpu::TextureFormat::Depth32Float))
		{
			LogError("GraphicsDevice::setupSwapChain() failed!!");
		}
	}

	void Application::setupInput()
	{
	}
}