#include "ModelConverter.h"
#include "Scene/Scene.h"
#include "Scene/GltfImporter.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/PBRMaterial.h"
#include "Graphics/Sampler.h"
#include "Graphics/Texture2D.h"
#include "Graphics/Model.h"
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
	void ModelConverter::setFileName(const std::string& fileName)
	{
		mFileName = fileName;
	}

	void ModelConverter::setOutputFileName(const std::string& fileName)
	{
		mOutputFileName = fileName;
	}

	void ModelConverter::setModelIndex(uint32_t modelIndex)
	{
		mModelIndex = modelIndex;
	}

	void ModelConverter::execute()
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

		auto resourceCache = std::make_unique<ResourceCache>();
		auto model = GltfImporter().importModel(mFileName, mOutputFileName, *resourceCache, mModelIndex, false);

		if (!model)
		{
			LogError("GltfImporter::importModel() failed for: %s!!", mFileName.c_str());
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

		if (!model->write())
		{
			LogError("Model::write() failed for: %s!!", mOutputFileName.c_str());
			mResult = false;
			return;
		}
	}
}

using namespace Trinity;

int main(int argc, char* argv[])
{
	CLI::App cliApp{ "Model Converter" };
	std::string fileName;
	std::string outputFileName;
	uint32_t modelIndex{ 0 };

	cliApp.add_option<std::string>("-f, --filename, filename", fileName, "Filename")->required();
	cliApp.add_option<std::string>("-o, --output, output", outputFileName, "Output Filename")->required();
	cliApp.add_option<uint32_t>("-m, --model, model", modelIndex, "Model Index");
	CLI11_PARSE(cliApp, argc, argv);

	static ModelConverter app;
	app.setFileName(fileName);
	app.setOutputFileName(outputFileName);
	app.setModelIndex(modelIndex);

	if (!app.run(LogLevel::Info))
	{
		return -1;
	}

	return 0;
}