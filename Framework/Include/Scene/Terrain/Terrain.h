#pragma once

#include "Core/Resource.h"
#include "Math/BoundingBox.h"

namespace Trinity
{
	class HeightMap;
	class Material;
	class Texture2D;

	struct MapDimension
	{
		glm::vec3 min;
		glm::vec3 size;

		glm::vec3 getMax() const
		{
			return min + size;
		}
	};

	class Terrain : public Resource
	{
	public:

		Terrain() = default;
		virtual ~Terrain() = default;

		Terrain(const Terrain&) = delete;
		Terrain& operator = (const Terrain&) = delete;

		Terrain(Terrain&&) = default;
		Terrain& operator = (Terrain&&) = default;

		const MapDimension& getMapDimension() const
		{
			return mMapDimension;
		}
		
		HeightMap* getHeightMap() const
		{
			return mHeightMap;
		}

		Material* getMaterial() const
		{
			return mMaterial;
		}

		uint32_t getNumLODs() const
		{
			return mNumLODs;
		}

		uint32_t getGridResolutionMult() const
		{
			return mGridResolutionMult;
		}

		uint32_t getLeafNodeSize() const
		{
			return mLeafNodeSize;
		}

		float getLODDistanceRatio() const
		{
			return mLODDistanceRatio;
		}

		virtual bool create(const std::string& fileName, ResourceCache& cache, bool loadContent = true) override;
		virtual void destroy() override;
		virtual bool write() override;

		virtual std::type_index getType() const override;

		virtual void setMapDimension(const MapDimension& mapDims);
		virtual void setHeightMap(HeightMap& heightMap);
		virtual void setMaterial(Material& material);
		virtual void setNumLODs(uint32_t numLODs);
		virtual void setLeafNodeSize(uint32_t leafNodeSize);
		virtual void setGridResolutionMult(uint32_t gridResoultionMult);
		virtual void setLODDistanceRatio(float lodDistanceRatio);

	protected:

		virtual bool read(FileReader& reader, ResourceCache& cache) override;
		virtual bool write(FileWriter& writer) override;

	protected:

		MapDimension mMapDimension;
		uint32_t mNumLODs{ 0 };
		uint32_t mLeafNodeSize{ 0 };
		uint32_t mGridResolutionMult{ 0 };
		float mLODDistanceRatio{ 0.0f };
		HeightMap* mHeightMap{ nullptr };
		Material* mMaterial{ nullptr };
	};
}