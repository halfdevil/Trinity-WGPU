#include "SkyboxTool.h"
#include "Scene/Scene.h"
#include "Scene/Skybox/SkyboxImporter.h"
#include "Scene/Skybox/Skybox.h"
#include "Scene/Skybox/SkyboxMaterial.h"
#include "Graphics/Sampler.h"
#include "Graphics/Texture2D.h"
#include "Graphics/Shader.h"
#include "Graphics/GraphicsDevice.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"
#include "Core/Image.h"
#include "Core/ResourceCache.h"
#include "VFS/FileSystem.h"
#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

namespace Trinity
{
	void SkyboxTool::setSize(float size)
	{
		mSize = size;
	}

	void SkyboxTool::setOutputFileName(const std::string& fileName)
	{
		mOutputFileName = fileName;
	}

	void SkyboxTool::setEnvMapFileNames(std::vector<std::string>&& fileNames)
	{
		mEnvMapFileNames = fileNames;
	}

	void SkyboxTool::execute()
	{
		auto& fileSystem = FileSystem::get();

		mResult = true;
		mShouldExit = true;

		auto resourceCache = std::make_unique<ResourceCache>();
		auto skybox = SkyboxImporter().importSkybox(
			mOutputFileName,
			mSize,
			mEnvMapFileNames,
			*resourceCache,
			false
		);

		if (!skybox)
		{
			LogError("SkyboxImporter::importSkybox() failed for: %s!!", mOutputFileName.c_str());
			mResult = false;
			return;
		}

		auto images = resourceCache->getResources<Image>();
		auto samplers = resourceCache->getResources<Sampler>();
		auto textures = resourceCache->getResources<Texture>();
		auto materials = resourceCache->getResources<Material>();

		for (auto* image : images)
		{
			if (!image->write())
			{
				LogError("Image::write() failed!!");
				mResult = false;
				return;
			}
		}

		for (auto* sampler : samplers)
		{
			if (!sampler->write())
			{
				LogError("Sampler::write() failed!!");
				mResult = false;
				return;
			}
		}

		for (auto* texture : textures)
		{
			if (!texture->write())
			{
				LogError("Texture::write() failed!!");
				mResult = false;
				return;
			}
		}

		for (auto* material : materials)
		{
			if (!material->write())
			{
				LogError("Material::write() failed!!");
				mResult = false;
				return;
			}
		}

		if (!skybox->write())
		{
			LogError("Skybox::write() failed for: %s!!", mOutputFileName.c_str());
			mResult = false;
			return;
		}
	}
}

using namespace Trinity;

int main(int argc, char* argv[])
{
	CLI::App cliApp{ "Skybox Tool" };
	float size{ 0 };
	std::string outputFileName;
	std::vector<std::string> envMapFileNames;

	cliApp.add_option<float>("-s, --size, size", size, "Size")->required();
	cliApp.add_option<std::string>("-o, --output, output", outputFileName, "Output Filename")->required();
	cliApp.add_option<std::vector<std::string>>("-e, --envMaps, envMaps", envMapFileNames, "EnvMap Filenames")->required();

	CLI11_PARSE(cliApp, argc, argv);

	static SkyboxTool app;
	app.setSize(size);
	app.setOutputFileName(outputFileName);
	app.setEnvMapFileNames(std::move(envMapFileNames));

	if (!app.run(LogLevel::Info))
	{
		return -1;
	}

	return 0;
}