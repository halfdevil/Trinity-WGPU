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
	class FileSystem;
	class GraphicsDevice;
	class ResourceCache;

	class ConsoleApplication : public Singleton<ConsoleApplication>
	{
	public:

		ConsoleApplication() = default;
		virtual ~ConsoleApplication() = 0;

		ConsoleApplication(const ConsoleApplication&) = delete;
		ConsoleApplication& operator = (const ConsoleApplication&) = delete;

		ConsoleApplication(ConsoleApplication&&) = delete;
		ConsoleApplication& operator = (ConsoleApplication&&) = delete;

		const json& getConfig() const
		{
			return mConfig;
		}

		Logger* getLogger() const
		{
			return mLogger.get();
		}

		Debugger* getDebugger() const
		{
			return mDebugger.get();
		}

		Window* getWindow() const
		{
			return mWindow.get();
		}

		FileSystem* getFileSystem() const
		{
			return mFileSystem.get();
		}

		GraphicsDevice* getGraphicsDevice() const
		{
			return mGraphicsDevice.get();
		}

		ResourceCache* getResourceCache() const
		{
			return mResourceCache.get();
		}

		virtual bool run(LogLevel logLevel, const std::string& configFile = "");

	protected:

		virtual bool init();
		virtual void execute();

	protected:

		json mConfig;
		bool mShouldExit{ false };
		bool mResult{ true };
		std::unique_ptr<Logger> mLogger{ nullptr };
		std::unique_ptr<Debugger> mDebugger{ nullptr };
		std::unique_ptr<Window> mWindow{ nullptr };
		std::unique_ptr<FileSystem> mFileSystem{ nullptr };
		std::unique_ptr<GraphicsDevice> mGraphicsDevice{ nullptr };
		std::unique_ptr<ResourceCache> mResourceCache{ nullptr };
	};
}