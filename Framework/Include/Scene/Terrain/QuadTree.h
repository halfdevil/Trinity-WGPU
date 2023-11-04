#pragma once

#include "Math/BoundingBox.h"
#include "Math/BoundingSphere.h"
#include "Math/Frustum.h"
#include <memory>
#include <vector>

namespace Trinity
{
	struct MapDimension;
	struct QuadTreeNode;
	class HeightMap;
	class QuadTree;
	class Camera;

	enum class SelectResult
	{
		Undefined,
		OutOfFrustum,
		OutOfRange,
		Selected
	};

	struct SelectedNode
	{
	public:

		SelectedNode() = default;
		SelectedNode(const QuadTreeNode& node, uint32_t inLodLevel, bool inTL, 
			bool inTR, bool inBL, bool inBR);

		BoundingBox getBoundingBox(const glm::uvec2& size, const MapDimension& mapDims) const;

	public:

		uint32_t x{ 0 };
		uint32_t y{ 0 };
		uint16_t size{ 0 };
		uint16_t minZ{ 0 };
		uint16_t maxZ{ 0 };

		bool tl{ false };
		bool tr{ false };
		bool bl{ false };
		bool br{ false };

		float minDistToCamera{ 0.0f };
		uint32_t lodLevel{ 0 };
	};

	struct SelectedLOD
	{
	public:

		SelectedLOD(uint32_t inMaxNumSelections);

	public:

		std::vector<SelectedNode> selectedNodes;
		uint32_t maxNumSelections{ 0 };
		uint32_t numSelections{ 0 };
		uint32_t minLODLevel{ 0 };
		uint32_t maxLODLevel{ 0 };
		bool visDistTooSmall{ false };		
	};

	struct QuadTreeNode
	{
	public:

		QuadTreeNode() = default;
		~QuadTreeNode() = default;

		QuadTreeNode(const QuadTreeNode&) = delete;
		QuadTreeNode& operator = (const QuadTreeNode&) = delete;

		QuadTreeNode(QuadTreeNode&&) = default;
		QuadTreeNode& operator = (QuadTreeNode&&) = default;

		uint16_t getLevel() const
		{
			return (uint16_t)(level & 0x7FFF);
		}

		bool isLeaf() const
		{
			return (level & 0x8000) != 0;
		}

		void create(QuadTree* quadTree, uint32_t inX, uint32_t inY, uint32_t inSize, 
			uint32_t inLevel, uint32_t lastIndex);

		void getAreaMinMaxHeight(const QuadTree* quadTree, const glm::uvec2& from, 
			const glm::uvec2& to, float& outMinZ, float& outMaxZ) const;

		SelectResult selectLOD(QuadTree* quadTree, SelectedLOD* selected, uint32_t stopAtLevel, 
			Camera* camera);

		glm::vec2 getWorldMinMaxX(uint32_t sizeX, const MapDimension& mapDims) const;
		glm::vec2 getWorldMinMaxY(uint32_t sizeY, const MapDimension& mapDims) const;
		glm::vec2 getWorldMinMaxZ(const MapDimension& mapDims) const;

		BoundingBox getBoundingBox(const glm::uvec2& inSize, const MapDimension& mapDims) const;
		BoundingSphere getBoundingSphere(const glm::uvec2& inSize, const MapDimension& mapDims) const;

	public:

		uint16_t x{ 0 };
		uint16_t y{ 0 };
		uint16_t size{ 0 };
		uint16_t level{ 0 };
		uint16_t minZ{ 0 };
		uint16_t maxZ{ 0 };

		QuadTreeNode* subTL{ nullptr };
		QuadTreeNode* subTR{ nullptr };
		QuadTreeNode* subBL{ nullptr };
		QuadTreeNode* subBR{ nullptr };
	};

	class QuadTree
	{
	public:

		static constexpr uint32_t kMaxSupportedLOD = 15;

		struct SelectionParams
		{
			SelectedLOD* selected{ nullptr };
			Camera* camera{ nullptr };
			float visibilityDistance{ 0.0f };
			float lodDistanceRatio{ 0.0f };
			float morphStartRatio{ 0.66f };
			bool sortByDistance{ false };
		};

		QuadTree() = default;
		virtual ~QuadTree();

		QuadTree(const QuadTree&) = delete;
		QuadTree& operator = (const QuadTree&) = delete;

		QuadTree(QuadTree&&) = default;
		QuadTree& operator = (QuadTree&&) = default;

		uint32_t getMaxLOD() const
		{
			return mNumLODs;
		}

		const MapDimension* getMapDimension() const
		{
			return mMapDimension;
		}

		const HeightMap* getHeightMap() const
		{
			return mHeightMap;
		}

		uint32_t getLeafNodeSize() const
		{
			return mLeafNodeSize;
		}

		std::vector<QuadTreeNode>& getNodes()
		{
			return mNodes;
		}

		virtual bool create(const MapDimension& mapDimension, const HeightMap& heightMap, 
			uint32_t leafNodeSize, uint32_t maxLOD);

		virtual void destroy();
		virtual void selectLOD(const SelectionParams& selectionParams);

		virtual float getVisibilityRange(uint32_t lodLevel) const;
		virtual float getMorphStart(uint32_t lodLevel) const;
		virtual float getMorphEnd(uint32_t lodLevel) const;
		virtual float getLODLevelNodeDiagonalSize(uint32_t lodLevel) const;

		virtual glm::vec4 getMorphConstant(uint32_t lodLevel);
		virtual void getAreaMinMaxHeight(const glm::uvec2& from, const glm::uvec2& size, float& minZ, float& maxZ) const;

	protected:

		const MapDimension* mMapDimension{ nullptr };
		const HeightMap* mHeightMap{ nullptr };
		uint32_t mNumLODs{ 0 };
		uint32_t mLeafNodeSize{ 0 };
		uint32_t mTopNodeSize{ 0 };
		glm::uvec2 mNumTopNodes{ 0 };
		glm::vec2 mLeafNodeWorldSize{ 0.0f };
		std::vector<QuadTreeNode> mNodes;
		std::vector<std::vector<QuadTreeNode*>> mTopNodes;
		float mVisibilityRanges[kMaxSupportedLOD];
		float mMorphStart[kMaxSupportedLOD];
		float mMorphEnd[kMaxSupportedLOD];
		float mLODLevelNodeDiagSizes[kMaxSupportedLOD];
	};
}