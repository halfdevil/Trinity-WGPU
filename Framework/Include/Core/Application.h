#pragma once

#include "Core/Logger.h"
#include "Core/Debugger.h"
#include "Core/Clock.h"
#include "Core/Window.h"
#include "VFS/FileSystem.h"
#include "Input/Input.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/SwapChain.h"

namespace Trinity
{
    class Application
    {
    public:

        Application() = default;
        virtual ~Application() = default;

        Application(const Application&) = delete;
        Application& operator = (const Application&) = delete;

        Application(Application&&) = delete;
        Application& operator = (Application&&) = delete;

        virtual void run(const std::string& title, uint32_t width = 1024, uint32_t height = 768,
            DisplayMode displayMode = DisplayMode::Windowed);

    protected:

        virtual bool init();
        virtual void update();
        virtual void render();
        virtual void frame();
        virtual void exit();

        virtual void onClose();
        virtual void onResize();
        virtual void setupInput();

    protected:

        Logger mLogger;
        Debugger mDebugger;
        Clock mClock;
        Window mWindow;
        FileSystem mFileSystem;
        Input mInput;
        GraphicsDevice mGraphicsDevice;
    };
}