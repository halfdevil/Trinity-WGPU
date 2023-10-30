#include "TerrainTool.h"
#include "Scene/Scene.h"
#include "Scene/Terrain/TerrainImporter.h"
#include "Scene/Terrain/Terrain.h"
#include "Scene/Terrain/TerrainMaterial.h"
#include "Scene/Terrain/HeightMap.h"
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
	void TerrainTool::setSize(uint32_t size)
	{
		mSize = size;
	}

	void TerrainTool::setPatchSize(uint32_t patchSize)
	{
		mPatchSize = patchSize;
	}

	void TerrainTool::setHeightScale(float heightScale)
	{
		mHeightScale = heightScale;
	}

	void TerrainTool::setCellSpacing(float cellSpacing)
	{
		mCellSpacing = cellSpacing;
	}

	void TerrainTool::setOutputFileName(const std::string& fileName)
	{
		mOutputFileName = fileName;
	}

	void TerrainTool::setHeightMapFileName(const std::string& fileName)
	{
		mHeightMapFileName = fileName;
	}

	void TerrainTool::setBlendMapFileName(const std::string& fileName)
	{
		mBlendMapFileName = fileName;
	}

	void TerrainTool::setLayerFileNames(std::vector<std::string>&& fileNames)
	{
		mLayerFileNames = fileNames;
	}

	void TerrainTool::execute()
	{
		auto& fileSystem = FileSystem::get();

		mResult = true;
		mShouldExit = true;

		auto resourceCache = std::make_unique<ResourceCache>();
		auto terrain = TerrainImporter().importTerrain(
			mOutputFileName,
			mSize,
			mPatchSize,
			mHeightScale,
			mCellSpacing,
			mHeightMapFileName,
			mBlendMapFileName,
			mLayerFileNames,
			*resourceCache,
			false
		);

		if (!terrain)
		{
			LogError("TerrainImporter::importTerrain() failed for: %s!!", mOutputFileName.c_str());
			mResult = false;
			return;
		}

		auto images = resourceCache->getResources<Image>();
		auto samplers = resourceCache->getResources<Sampler>();
		auto textures = resourceCache->getResources<Texture>();
		auto materials = resourceCache->getResources<Material>();
		auto heightMaps = resourceCache->getResources<HeightMap>();

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

		for (auto* heightMap : heightMaps)
		{
			if (!heightMap->write())
			{
				LogError("HeightMap::write() failed!!");
				mResult = false;
				return;
			}
		}

		if (!terrain->write())
		{
			LogError("Terrain::write() failed for: %s!!", mOutputFileName.c_str());
			mResult = false;
			return;
		}
	}
}

using namespace Trinity;

int main(int argc, char* argv[])
{
	CLI::App cliApp{ "Model Converter" };
	uint32_t size{ 0 };
	uint32_t patchSize{ 0 };
	float heightScale{ 0.0f };
	float cellSpacing{ 0.0f };
	std::string outputFileName;
	std::string heightMapFileName;
	std::string blendMapFileName;
	std::vector<std::string> layerFileNames;

	cliApp.add_option<uint32_t>("-s, --size, size", size, "Size")->required();
	cliApp.add_option<uint32_t>("-p, --patchSize, patchSize", patchSize, "Size")->required();
	cliApp.add_option<float>("-t, --heightScale, heightScale", heightScale, "Height Scale")->required();
	cliApp.add_option<float>("-c, --cellSpacing, cellSpacing", cellSpacing, "Cell Spacing")->required();
	cliApp.add_option<std::string>("-o, --output, output", outputFileName, "Output Filename")->required();
	cliApp.add_option<std::string>("-m, --heightMap, heightMap", heightMapFileName, "HeightMap Filename")->required();
	cliApp.add_option<std::string>("-b, --blendMap, blendMap", blendMapFileName, "BlendMap Filename")->required();
	cliApp.add_option<std::vector<std::string>>("-l, --layers, layers", layerFileNames, "Layers Filename")->required();

	CLI11_PARSE(cliApp, argc, argv);

	static TerrainTool app;
	app.setSize(size);
	app.setPatchSize(patchSize);
	app.setHeightScale(heightScale);
	app.setCellSpacing(cellSpacing);
	app.setOutputFileName(outputFileName);
	app.setHeightMapFileName(heightMapFileName);
	app.setBlendMapFileName(blendMapFileName);
	app.setLayerFileNames(std::move(layerFileNames));

	if (!app.run(LogLevel::Info))
	{
		return -1;
	}

	return 0;
}