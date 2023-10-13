#include "Core/ConsoleApplication.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"
#include "Core/Window.h"
#include "VFS/FileSystem.h"
#include "VFS/DiskFile.h"
#include "Graphics/GraphicsDevice.h"
#include <iostream>

namespace Trinity
{
	ConsoleApplication::~ConsoleApplication()
	{
	}

	bool ConsoleApplication::run(LogLevel logLevel, const std::string& configFile)
	{
		mResult = true;
		mLogger = std::make_unique<Logger>();
		mLogger->setMaxLogLevel(logLevel);

		mDebugger = std::make_unique<Debugger>();
		mFileSystem = std::make_unique<FileSystem>();
		mWindow = std::make_unique<Window>();
		mGraphicsDevice = std::make_unique<GraphicsDevice>();

		if (!configFile.empty())
		{
			DiskFile file;
			if (!file.create(configFile, configFile, FileOpenMode::OpenRead))
			{
				LogError("PhysicalFile::create() failed for: %s!!", configFile.c_str());
				return false;
			}

			FileReader reader(file);
			mConfig = json::parse(reader.readAsString());
		}

		if (!Window::initialize())
		{
			LogFatal("Window::initialize() failed!!");
			return false;
		}

		if (!mWindow->create("", 1, 1, DisplayMode::Windowed))
		{
			LogFatal("Window::create() failed!!");
			return false;
		}

		mGraphicsDevice->onCreated.subscribe([this](bool result) {
			if (result)
			{
				if (init())
				{
					execute();	
					
					while (!mShouldExit)
					{
						mWindow->poll();
					}
				}
			}
		});

		mGraphicsDevice->create(*mWindow);
		return mResult;
	}

	bool ConsoleApplication::init()
	{
		if (!mFileSystem->addFolder("/Assets", "Assets"))
		{
			LogError("FileSystem::addFolder() failed!!");
			return false;
		}

		return true;
	}

	void ConsoleApplication::execute()
	{
	}
}