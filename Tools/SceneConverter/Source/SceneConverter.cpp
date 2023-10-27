#include "SceneConverter.h"
#include "Scene/Scene.h"
#include "Scene/Model.h"
#include "Scene/GltfImporter.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/PBRMaterial.h"
#include "Graphics/Sampler.h"
#include "Graphics/Texture2D.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"
#include "Core/ResourceCache.h"
#include "Core/Image.h"
#include "VFS/FileSystem.h"
#include "VFS/DiskFile.h"
#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

namespace Trinity
{
	void SceneConverter::setFileName(const std::string& fileName)
	{
		mFileName = fileName;
	}

	void SceneConverter::setOutputFileName(const std::string& fileName)
	{
		mOutputFileName = fileName;
	}

	void SceneConverter::execute()
	{
		auto& fileSystem = FileSystem::get();

		mResult = true;
		mShouldExit = true;

		if (!fileSystem.isExist(mFileName))
		{
			LogError("Input file doesn't exists: %s!!", mFileName.c_str());
			mResult = false;
			return;
		}
		
		auto file = fileSystem.openFile(mFileName, FileOpenMode::OpenRead);
		if (!file)
		{
			LogError("FileSystem::openFile() failed for: %s", mFileName.c_str());
			mResult = false;
			return;
		}

		auto scene = GltfImporter().importScene(mFileName, mOutputFileName, *mResourceCache, false);
		if (!scene)
		{
			LogError("GltfImporter::importScene() failed for: %s!!", mFileName.c_str());
			mResult = false;
			return;
		}

		auto images = mResourceCache->getResources<Image>();
		auto samplers = mResourceCache->getResources<Sampler>();
		auto textures = mResourceCache->getResources<Texture>();
		auto materials = mResourceCache->getResources<Material>();
		auto models = mResourceCache->getResources<Model>();

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

		for (auto* model : models)
		{
			if (!model->write())
			{
				LogError("Material::write() failed!!");
				mResult = false;
				return;
			}
		}

		if (!scene->write())
		{
			LogError("Scene::write() failed for: %s!!", mOutputFileName.c_str());
			mResult = false;
			return;
		}
	}
}

using namespace Trinity;

int main(int argc, char* argv[])
{
	CLI::App cliApp{ "Scene Converter" };
	std::string fileName;
	std::string outputFileName;

	cliApp.add_option<std::string>("-f, --filename, filename", fileName, "Filename")->required();
	cliApp.add_option<std::string>("-o, --output, output", outputFileName, "Output Filename")->required();
	CLI11_PARSE(cliApp, argc, argv);

	static SceneConverter app;
	app.setFileName(fileName);
	app.setOutputFileName(outputFileName);

	if (!app.run(LogLevel::Info))
	{
		return -1;
	}

	return 0;
}