#pragma once

#include "Core/Window.h"
#include "Core/Logger.h"
#include <memory>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace Trinity
{
    class Debugger;
    class Clock;
    class FileSystem;
    class Input;
    class GraphicsDevice;

    struct ApplicationOptions
    {
        LogLevel logLevel{ LogLevel::Error };
        std::string title;
        uint32_t width{ 1024 };
        uint32_t height{ 768 };
        DisplayMode displayMode{ DisplayMode::Windowed };
        std::string configFile;
    };

    class Application
    {
    public:

        Application() = default;
        virtual ~Application() = default;

        Application(const Application&) = delete;
        Application& operator = (const Application&) = delete;

        Application(Application&&) = delete;
        Application& operator = (Application&&) = delete;

        const json& getConfig() const
        {
            return mConfig;
        }

        const ApplicationOptions& getOptions() const
        {
            return mOptions;
        }

        virtual void run(const ApplicationOptions& options);

    protected:

        virtual bool init();
        virtual void update(float deltaTime);
        virtual void render(float deltaTime);
        virtual void frame();
        virtual void exit();

        virtual void onClose();
        virtual void onResize();
        virtual void setupInput();

    protected:

        json mConfig;
        ApplicationOptions mOptions;
        std::unique_ptr<Logger> mLogger{ nullptr };
        std::unique_ptr<Debugger> mDebugger{ nullptr };
        std::unique_ptr<Clock> mClock{ nullptr };
        std::unique_ptr<Window> mWindow{ nullptr };
        std::unique_ptr<FileSystem> mFileSystem{ nullptr };
        std::unique_ptr<Input> mInput{ nullptr };
        std::unique_ptr<GraphicsDevice> mGraphicsDevice{ nullptr };
    };
}