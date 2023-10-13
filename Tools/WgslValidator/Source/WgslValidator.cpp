#include "WgslValidator.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"
#include "VFS/FileSystem.h"
#include "VFS/DiskFile.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/Shader.h"
#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

namespace Trinity
{
	WgslValidator::~WgslValidator()
	{
	}

	void WgslValidator::setFileName(const std::string& fileName)
	{
		mFileName = fileName;
	}

	void WgslValidator::setDefines(std::vector<std::string>&& defines)
	{
		mDefines = std::move(defines);
	}

	void WgslValidator::execute()
	{
		mResult = true;
		mShouldExit = true;

		if (mFileName.empty())
		{
			LogError("No filename specified!!");
			mResult = false;
			return;
		}

		ShaderPreProcessor processor;
		processor.addDefines(mDefines);
		
		Shader shader;
		if (!shader.create(mFileName, processor))
		{
			LogError("Shader::create() failed for: %s!!", mFileName.c_str());
			mResult = false;
			return;
		}
	}
}

using namespace Trinity;

int main(int argc, char* argv[])
{
	CLI::App cliApp{ "Wgsl Validator" };
	std::string fileName;
	std::vector<std::string> defines;

	cliApp.add_option<std::string>("-f, --filename, filename", fileName, "Filename")->required();
	cliApp.add_option<std::vector<std::string>>("-d, --defines, defines", defines, "Defines");
	CLI11_PARSE(cliApp, argc, argv);

	static WgslValidator app;
	app.setFileName(fileName);
	app.setDefines(std::move(defines));

	if (!app.run(LogLevel::Info))
	{
		return -1;
	}

	return 0;
}