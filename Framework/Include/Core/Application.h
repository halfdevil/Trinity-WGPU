#pragma once

#include "Core/Singleton.h"
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
    class RenderPass;
    class ImGuiRenderer;
    class ResourceCache;

    struct ApplicationOptions
    {
        LogLevel logLevel{ LogLevel::Error };
        bool headless{ false };
        std::string title;
        uint32_t width{ 1024 };
        uint32_t height{ 768 };
        DisplayMode displayMode{ DisplayMode::Windowed };
        std::string configFile;
    };

    class Application : public Singleton<Application>
    {
    public:

        Application() = default;
        virtual ~Application() = 0;

        Application(const Application&) = delete;
        Application& operator = (const Application&) = delete;

        Application(Application&&) = default;
        Application& operator = (Application&&) = default;

        const json& getConfig() const
        {
            return mConfig;
        }

        const ApplicationOptions& getOptions() const
        {
            return mOptions;
        }

        Logger* getLogger() const
        {
            return mLogger.get();
        }

        Debugger* getDebugger() const
        {
            return mDebugger.get();
        }

        Clock* getClock() const
        {
            return mClock.get();
        }

        Window* getWindow() const
        {
            return mWindow.get();
        }

        FileSystem* getFileSystem() const
        {
            return mFileSystem.get();
        }

        Input* getInput() const
        {
            return mInput.get();
        }

        GraphicsDevice* getGraphicsDevice() const
        {
            return mGraphicsDevice.get();
        }

        ResourceCache* getResourceCache() const
        {
            return mResourceCache.get();
        }

        RenderPass* getMainPass() const
        {
            return mMainPass.get();
        }

        ImGuiRenderer* getImGuiRenderer() const
        {
            return mImGuiRenderer.get();
        }

        virtual void run(const ApplicationOptions& options);

    protected:

        virtual bool init();
        virtual void update(float deltaTime);
        virtual void render(float deltaTime);
        virtual void frame();
		virtual void exit();
		virtual void drawGui(float deltaTime);

        virtual void onClose();
        virtual void onResize();
        virtual void onGui();
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
        std::unique_ptr<ResourceCache> mResourceCache{ nullptr };
        std::unique_ptr<RenderPass> mMainPass{ nullptr };
        std::unique_ptr<ImGuiRenderer> mImGuiRenderer{ nullptr };
    };
}