#pragma once

#include "Core/Resource.h"
#include "Math/BoundingBox.h"

namespace Trinity
{
	class HeightMap;
	class VertexLayout;
	class VertexBuffer;
	class IndexBuffer;
	class Material;
	class Camera;
	class Transform;

	class Terrain : public Resource
	{
	public:

		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;
		};

		struct Patch
		{
			Patch* top{ nullptr };
			Patch* bottom{ nullptr };
			Patch* right{ nullptr };
			Patch* left{ nullptr };
			int32_t currentLOD{ -1 };
			BoundingBox boundingBox;
			glm::vec3 center{ 0.0f };
		};

		Terrain() = default;
		virtual ~Terrain() = default;

		Terrain(const Terrain&) = delete;
		Terrain& operator = (const Terrain&) = delete;

		Terrain(Terrain&&) = default;
		Terrain& operator = (Terrain&&) = default;

		uint32_t getSize() const
		{
			return mSize;
		}

		uint32_t getPatchSize() const
		{
			return mPatchSize;
		}

		uint32_t getCalcPatchSize() const
		{
			return mCalcPatchSize;
		}

		uint32_t getNumPatches() const
		{
			return mNumPatches;
		}

		uint32_t getMaxLOD() const
		{
			return mMaxLOD;
		}

		float getCellSpacing() const
		{
			return mCellSpacing;
		}

		HeightMap* getHeightMap() const
		{
			return mHeightMap;
		}

		Material* getMaterial() const
		{
			return mMaterial;
		}

		const BoundingBox& getBoundingBox() const
		{
			return mBoundingBox;
		}

		VertexBuffer* getVertexBuffer() const
		{
			return mVertexBuffer;
		}

		IndexBuffer* getIndexBuffer() const
		{
			return mIndexBuffer;
		}

		uint32_t getIndicesToDraw() const
		{
			return mIndicesToDraw;
		}

		virtual bool create(const std::string& fileName, ResourceCache& cache, bool loadContent = true) override;
		virtual void destroy() override;
		virtual bool write() override;

		virtual bool load(ResourceCache& cache, HeightMap& heightMap, Material& material, 
			uint32_t patchSize, float cellSpacing);

		virtual void preDrawCalculations(const Camera& camera);
		virtual std::type_index getType() const override;

		virtual void setHeightMap(HeightMap& heightMap);
		virtual void setMaterial(Material& material);
		virtual void setPatchSize(uint32_t patchSize);
		virtual void setCellSpacing(float cellSpacing);

	protected:

		virtual void preDrawLODCalculations(const Camera& camera);
		virtual void preDrawIndicesCalculations();

		uint32_t getIndex(uint32_t patchX, uint32_t patchZ, uint32_t patchIndex, 
			uint32_t x, uint32_t z) const;

		virtual bool setupDeviceObjects(ResourceCache& cache);
		virtual void calculateNormals(std::vector<Vertex>& vertices);
		virtual void createPatches();
		virtual void calculatePatchData(std::vector<Vertex>& vertices);
		virtual void calculateDistanceThresholds();
		virtual void setCurrentLODOfPatches(int32_t lod);
		virtual void setCurrentLODOfPatches(const std::vector<int32_t>& lods);

		virtual bool read(FileReader& reader, ResourceCache& cache) override;
		virtual bool write(FileWriter& writer) override;

	protected:

		uint32_t mSize{ 0 };
		uint32_t mPatchSize{ 0 };
		uint32_t mCalcPatchSize{ 0 };
		uint32_t mNumPatches{ 0 };
		uint32_t mMaxLOD{ 0 };
		float mCellSpacing{ 1.0f };
		uint32_t mIndicesToDraw{ 0 };

		std::vector<Patch> mPatches;
		std::vector<uint32_t> mIndices;
		std::vector<double> mDistanceThreshold;
		BoundingBox mBoundingBox;
		glm::vec3 mCenter{ 0.0f };
		HeightMap* mHeightMap{ nullptr };
		Material* mMaterial{ nullptr };
		VertexBuffer* mVertexBuffer{ nullptr };
		IndexBuffer* mIndexBuffer{ nullptr };

		glm::vec3 oldCameraPosition;
		glm::vec3 oldCameraRotation;
		float cameraMovementDelta{ 10.0f };
		float cameraRotationDelta{ glm::radians(1.0f) };
	};
}