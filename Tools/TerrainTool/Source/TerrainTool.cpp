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
#include "VFS/DiskFile.h"
#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

namespace Trinity
{
	void TerrainTool::setConfigFileName(const std::string& configFileName)
	{
		mConfigFileName = configFileName;
	}

	void TerrainTool::execute()
	{
		auto& fileSystem = FileSystem::get();

		mResult = true;
		mShouldExit = true;

		DiskFile configFile;
		if (!configFile.create(mConfigFileName, mConfigFileName, FileOpenMode::OpenRead))
		{
			LogError("PhysicalFile::create() failed for: %s!!", mConfigFileName.c_str());
			mResult = false;
			return;
		}

		FileReader reader(configFile);
		mConfig = json::parse(reader.readAsString());

		auto mapDimsJson = mConfig["mapDims"];
		auto outputFileName = mConfig["output"].get<std::string>();

		MapDimension mapDims = {
			.min = {
				mapDimsJson["minX"].get<float>(),
				mapDimsJson["minY"].get<float>(),
				mapDimsJson["minZ"].get<float>(),
			},
			.size = {
				mapDimsJson["sizeX"].get<float>(),
				mapDimsJson["sizeY"].get<float>(),
				mapDimsJson["sizeZ"].get<float>(),
			}
		};

		std::vector<std::string> layerMaps;
		for (auto layerMap : mConfig["layerMaps"])
		{
			layerMaps.push_back(layerMap.get<std::string>());
		}

		auto resourceCache = std::make_unique<ResourceCache>();
		auto terrain = TerrainImporter().importTerrain(
			outputFileName,
			mapDims,
			mConfig["size"].get<uint32_t>(),
			mConfig["numLODs"].get<uint32_t>(),
			mConfig["leafNodeSize"].get<uint32_t>(),
			mConfig["gridResolutionMult"].get<uint32_t>(),
			mConfig["lodLevelDistanceRatio"].get<float>(),
			mConfig["heightMap"].get<std::string>(),
			mConfig["blendMap"].get<std::string>(),
			layerMaps,
			*resourceCache,
			false
		);

		if (!terrain)
		{
			LogError("TerrainImporter::importTerrain() failed for: %s!!", outputFileName.c_str());
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
			LogError("Terrain::write() failed for: %s!!", outputFileName.c_str());
			mResult = false;
			return;
		}
	}
}

using namespace Trinity;

int main(int argc, char* argv[])
{
	CLI::App cliApp{ "Terrain Tool" };
	std::string configFileName;

	cliApp.add_option<std::string>("-c, --config, config", configFileName, "Config Filename")->required();
	CLI11_PARSE(cliApp, argc, argv);

	static TerrainTool app;
	app.setConfigFileName(configFileName);

	if (!app.run(LogLevel::Info))
	{
		return -1;
	}

	return 0;
}