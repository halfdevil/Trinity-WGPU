#include "Scene/Terrain/Terrain.h"
#include "Scene/Terrain/HeightMap.h"
#include "Scene/Terrain/TerrainMaterial.h"
#include "Scene/Node.h"
#include "VFS/FileSystem.h"
#include "Core/ResourceCache.h"
#include "Core/Logger.h"

#define _USE_MATH_DEFINES
#include <cmath>

namespace Trinity
{
	bool Terrain::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		return Resource::create(fileName, cache, loadContent);
	}

	void Terrain::destroy()
	{
		Resource::destroy();
	}

	bool Terrain::write()
	{
		return Resource::write();
	}

	std::type_index Terrain::getType() const
	{
		return typeid(Terrain);
	}

	void Terrain::setMapDimension(const MapDimension& mapDims)
	{
		mMapDimension = mapDims;
	}

	void Terrain::setHeightMap(HeightMap& heightMap)
	{
		mHeightMap = &heightMap;
	}

	void Terrain::setMaterial(Material& material)
	{
		mMaterial = &material;
	}

	void Terrain::setNumLODs(uint32_t numLODs)
	{
		mNumLODs = numLODs;
	}

	void Terrain::setLeafNodeSize(uint32_t leafNodeSize)
	{
		mLeafNodeSize = leafNodeSize;
	}

	void Terrain::setGridResolutionMult(uint32_t gridResoultionMult)
	{
		mGridResolutionMult = gridResoultionMult;
	}

	void Terrain::setLODDistanceRatio(float lodDistanceRatio)
	{
		mLODDistanceRatio = lodDistanceRatio;
	}

	bool Terrain::read(FileReader& reader, ResourceCache& cache)
	{
		if (!Resource::read(reader, cache))
		{
			return false;
		}

		reader.read(&mMapDimension);
		reader.read(&mNumLODs);
		reader.read(&mLeafNodeSize);
		reader.read(&mGridResolutionMult);
		reader.read(&mLODDistanceRatio);

		auto& fileSystem = FileSystem::get();
		auto heightMapFileName = Resource::getReadPath(reader.getPath(), reader.readString());
		
		if (!cache.isLoaded<HeightMap>(heightMapFileName))
		{
			auto heightMap = std::make_unique<HeightMap>();
			if (!heightMap->create(heightMapFileName, cache))
			{
				LogError("HeightMap::create() failed for '%s'", heightMapFileName.c_str());
				return false;
			}

			cache.addResource(std::move(heightMap));
		}
		
		auto materialFileName = Resource::getReadPath(reader.getPath(), reader.readString());
		if (!cache.isLoaded<Material>(materialFileName))
		{
			auto* heightMap = cache.getResource<HeightMap>(heightMapFileName);
			const auto& data = heightMap->getData();
			const auto& size = heightMap->getSize();

			std::vector<float> heightMapData(data.size());
			for (uint32_t idx = 0; idx < (uint32_t)data.size(); idx++)
			{
				heightMapData[idx] = data[idx] / 65535.0f;
			}

			auto material = std::make_unique<TerrainMaterial>();
			if (!material->create(materialFileName, cache))
			{
				LogError("TerrainMaterial::create() failed for '%s'", materialFileName.c_str());
				return false;
			}

			if (!material->addHeightMapTexture(heightMapData, size, mMapDimension, cache))
			{
				LogError("TerrainMaterial::addHeightMapTexture() failed for '%s'", materialFileName.c_str());
				return false;
			}

			if (!material->addNormalMapTexture(heightMapData, size, mMapDimension, cache))
			{
				LogError("TerrainMaterial::addNormalMapTexture() failed for '%s'", materialFileName.c_str());
				return false;
			}

			if (!material->compile(cache))
			{
				LogError("TerrainMaterial::compile() failed for '%s'", materialFileName.c_str());
				return false;
			}

			cache.addResource(std::move(material));
		}

		mHeightMap = cache.getResource<HeightMap>(heightMapFileName);
		mMaterial = cache.getResource<Material>(materialFileName);

		return true;
	}

	bool Terrain::write(FileWriter& writer)
	{
		if (!Resource::write(writer))
		{
			return false;
		}

		writer.write(&mMapDimension);
		writer.write(&mNumLODs);
		writer.write(&mLeafNodeSize);
		writer.write(&mGridResolutionMult);
		writer.write(&mLODDistanceRatio);

		if (mHeightMap != nullptr)
		{
			auto fileName = Resource::getWritePath(writer.getPath(), mHeightMap->getFileName());
			writer.writeString(fileName);
		}

		if (mMaterial != nullptr)
		{
			auto fileName = Resource::getWritePath(writer.getPath(), mMaterial->getFileName());
			writer.writeString(fileName);
		}

		return true;
	}
}