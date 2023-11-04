#include "Scene/Terrain/QuadTree.h"
#include "Scene/Terrain/Terrain.h"
#include "Scene/Terrain/HeightMap.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/Transform.h"
#include "Scene/Node.h"
#include <queue>

namespace Trinity
{
	SelectedNode::SelectedNode(const QuadTreeNode& node, uint32_t inLodLevel, bool inTL, bool inTR, bool inBL, bool inBR) :
		lodLevel(inLodLevel), tl(inTL), tr(inTR), bl(inBL), br(inBR)
	{
		this->x = node.x;
		this->y = node.y;
		this->size = node.size;
		this->minZ = node.minZ;
		this->maxZ = node.maxZ;
	}

	BoundingBox SelectedNode::getBoundingBox(const glm::uvec2& inSize, const MapDimension& mapDims) const
	{
		glm::vec3 min { 
			mapDims.min.x + x * mapDims.size.x / (float)(inSize.x - 1),
			mapDims.min.y + y * mapDims.size.y / (float)(inSize.y - 1),
			mapDims.min.z + minZ * mapDims.size.z / 65535.0f
		};

		glm::vec3 max {
			mapDims.min.x + (x + size) * mapDims.size.x / (float)(inSize.x - 1),
			mapDims.min.y + (y + size) * mapDims.size.y / (float)(inSize.y - 1),
			mapDims.min.z + maxZ * mapDims.size.z / 65535.0f
		};

		return { min, max };
	}

	SelectedLOD::SelectedLOD(uint32_t inMaxNumSelections) :
		maxNumSelections(inMaxNumSelections)
	{
		selectedNodes.resize(inMaxNumSelections);
	}

	void QuadTreeNode::create(QuadTree* quadTree, uint32_t inX, uint32_t inY, uint32_t inSize, uint32_t inLevel, uint32_t lastIndex)
	{
		x = (uint16_t)inX;
		y = (uint16_t)inY;
		size = (uint16_t)inSize;
		level = (uint16_t)inLevel;

		const auto* heightMap = quadTree->getHeightMap();
		const auto* mapDims = quadTree->getMapDimension();
		auto& nodes = quadTree->getNodes();
		auto& hmSize = heightMap->getSize();

		if (size == quadTree->getLeafNodeSize())
		{
			level |= 0x8000;

			uint32_t limitX = std::min(hmSize.x, (uint32_t)x + size + 1);
			uint32_t limitY = std::min(hmSize.y, (uint32_t)y + size + 1);

			heightMap->getMinMaxHeight(x, y, limitX - x, limitX - y, minZ, maxZ);

			float* pTLZ = (float*)&subTL;
			float* pTRZ = (float*)&subTR;
			float* pBLZ = (float*)&subBL;
			float* pBRZ = (float*)&subBR;

			limitX = std::min(hmSize.x - 1, (uint32_t)x + size);
			limitY = std::max(hmSize.y - 1, (uint32_t)y + size);

			*pTLZ = mapDims->min.z + heightMap->getHeight(x, y) * mapDims->size.z / 65535.0f;
			*pTRZ = mapDims->min.z + heightMap->getHeight(limitX, y) * mapDims->size.z / 65535.0f;
			*pBLZ = mapDims->min.z + heightMap->getHeight(x, limitY) * mapDims->size.z / 65535.0f;
			*pBRZ = mapDims->min.z + heightMap->getHeight(limitX, limitY) * mapDims->size.z / 65535.0f;
		}
		else
		{
			uint32_t subSize = size / 2;

			subTL = &nodes[lastIndex++];
			subTL->create(quadTree, x, y, subSize, level + 1, lastIndex);
			minZ = subTL->minZ;
			maxZ = subTL->maxZ;

			if ((x + subSize) < hmSize.x)
			{
				subTR = &nodes[lastIndex++];
				subTR->create(quadTree, x + subSize, y, subSize, level + 1, lastIndex);
				minZ = std::min(minZ, subTR->minZ);
				maxZ = std::max(maxZ, subTR->maxZ);
			}

			if ((y + subSize) < hmSize.y)
			{
				subBL = &nodes[lastIndex++];
				subBL->create(quadTree, x, y + subSize, subSize, level + 1, lastIndex);
				minZ = std::min(minZ, subBL->minZ);
				maxZ = std::max(maxZ, subBL->maxZ);
			}

			if ((x + subSize) < hmSize.x && (y + subSize) < hmSize.y)
			{
				subBR = &nodes[lastIndex++];
				subBR->create(quadTree, x + subSize, y + subSize, subSize, level + 1, lastIndex);
				minZ = std::min(minZ, subBR->minZ);
				maxZ = std::max(maxZ, subBR->maxZ);
			}
		}
	}

	void QuadTreeNode::getAreaMinMaxHeight(const QuadTree* quadTree, const glm::uvec2& from,
		const glm::uvec2& to, float& outMinZ, float& outMaxZ) const
	{
		if (to.x < (uint32_t)x || to.y < (uint32_t)y || from.x > (uint32_t)(x + size) ||
			from.y > (uint32_t)(y + size))
		{
			return;
		}

		if (isLeaf() || (from.x < (uint32_t)x && from.y < (uint32_t)y && 
			to.x > (uint32_t)(x + size) && to.y > (uint32_t)(y + size)))
		{
			auto worldZ = getWorldMinMaxZ(*quadTree->getMapDimension());
			
			outMinZ = std::min(outMinZ, worldZ.x);
			outMaxZ = std::max(outMaxZ, worldZ.y);

			return;
		}

		if (subTL != nullptr)
		{
			subTL->getAreaMinMaxHeight(quadTree, from, to, outMinZ, outMaxZ);
		}

		if (subTR != nullptr)
		{
			subTR->getAreaMinMaxHeight(quadTree, from, to, outMinZ, outMaxZ);
		}

		if (subBL != nullptr)
		{
			subBL->getAreaMinMaxHeight(quadTree, from, to, outMinZ, outMaxZ);
		}

		if (subBR != nullptr)
		{
			subBR->getAreaMinMaxHeight(quadTree, from, to, outMinZ, outMaxZ);
		}
	}

	SelectResult QuadTreeNode::selectLOD(QuadTree* quadTree, SelectedLOD* selected, uint32_t stopAtLevel, Camera* camera)
	{
		auto* heightMap = quadTree->getHeightMap();
		auto* mapDims = quadTree->getMapDimension();
		auto& hmSize = heightMap->getSize();

		auto box = getBoundingBox(hmSize, *mapDims);
		auto& frustum = camera->getFrustum();
		auto& position = camera->getNode()->getTransform().getTranslation();
		
		if (!frustum.contains(box))
		{
			return SelectResult::OutOfFrustum;
		}

		auto distLimit = quadTree->getVisibilityRange(getLevel());
		if (!box.isIntersectSphere({ position, distLimit * distLimit }))
		{
			return SelectResult::OutOfRange;
		}

		SelectResult subTLRes = SelectResult::Undefined;
		SelectResult subTRRes = SelectResult::Undefined;
		SelectResult subBLRes = SelectResult::Undefined;
		SelectResult subBRRes = SelectResult::Undefined;

		if (getLevel() != stopAtLevel)
		{
			float nextDistLimit = quadTree->getVisibilityRange(getLevel() + 1);
			if (box.isIntersectSphere({ position, nextDistLimit * nextDistLimit }))
			{
				if (subTL != nullptr)
				{
					subTLRes = subTL->selectLOD(quadTree, selected, stopAtLevel, camera);
				}

				if (subTR != nullptr)
				{
					subTRRes = subTR->selectLOD(quadTree, selected, stopAtLevel, camera);
				}

				if (subBL != nullptr)
				{
					subBLRes = subBL->selectLOD(quadTree, selected, stopAtLevel, camera);
				}

				if (subBR != nullptr)
				{
					subBRRes = subBR->selectLOD(quadTree, selected, stopAtLevel, camera);
				}
			}
		}

		bool removeSubTL = subTLRes == SelectResult::OutOfFrustum || subTLRes == SelectResult::Selected;
		bool removeSubTR = subTRRes == SelectResult::OutOfFrustum || subTRRes == SelectResult::Selected;
		bool removeSubBL = subBLRes == SelectResult::OutOfFrustum || subBLRes == SelectResult::Selected;
		bool removeSubBR = subBRRes == SelectResult::OutOfFrustum || subBRRes == SelectResult::Selected;

		if (!(removeSubTL && removeSubTR && removeSubBL && removeSubBR) && 
			selected->numSelections < selected->maxNumSelections)
		{
			auto lodLevel = stopAtLevel - getLevel();
			selected->selectedNodes[selected->numSelections++] = { 
				*this, 
				lodLevel, 
				!removeSubTL, 
				!removeSubTR, 
				!removeSubBL, 
				!removeSubBR 
			};

			if (!selected->visDistTooSmall && getLevel() != 0)
			{
				float maxDistFromCamera = std::sqrtf(box.getMaxDistanceFromPointSq(position));
				float morphStartRange = quadTree->getMorphStart(stopAtLevel - getLevel() + 1);

				if (maxDistFromCamera > morphStartRange)
				{
					selected->visDistTooSmall = true;
				}
			}

			return SelectResult::Selected;
		}

		if (subTLRes == SelectResult::Selected || subTRRes == SelectResult::Selected ||
			subBLRes == SelectResult::Selected || subBRRes == SelectResult::Selected)
		{
			return SelectResult::Selected;
		}

		return SelectResult::OutOfFrustum;
	}

	glm::vec2 QuadTreeNode::getWorldMinMaxX(uint32_t sizeX, const MapDimension& mapDims) const
	{
		return {
			mapDims.min.x + x * mapDims.size.x / (float)(sizeX - 1),
			mapDims.min.x + (x + size) * mapDims.size.x / (float)(sizeX - 1)
		};
	}

	glm::vec2 QuadTreeNode::getWorldMinMaxY(uint32_t sizeY, const MapDimension& mapDims) const
	{
		return {
			mapDims.min.y + y * mapDims.size.y / (float)(sizeY - 1),
			mapDims.min.y + (y + size) * mapDims.size.y / (float)(sizeY - 1)
		};
	}

	glm::vec2 QuadTreeNode::getWorldMinMaxZ(const MapDimension& mapDims) const
	{
		return {
			mapDims.min.z + minZ * mapDims.size.z / 65535.0f,
			mapDims.min.z + maxZ * mapDims.size.z / 65535.0f
		};
	}

	BoundingBox QuadTreeNode::getBoundingBox(const glm::uvec2& inSize, const MapDimension& mapDims) const
	{
		auto minMaxX = getWorldMinMaxX(inSize.x, mapDims);
		auto minMaxY = getWorldMinMaxY(inSize.y, mapDims);
		auto minMaxZ = getWorldMinMaxZ(mapDims);

		return {
			{ minMaxX.x, minMaxY.x, minMaxZ.x },
			{ minMaxX.y, minMaxY.y, minMaxZ.y }
		};
	}

	BoundingSphere QuadTreeNode::getBoundingSphere(const glm::uvec2& inSize, const MapDimension& mapDims) const
	{
		float scaleX = mapDims.size.x / (float)(inSize.x - 1);
		float scaleY = mapDims.size.y / (float)(inSize.y - 1);
		float scaleZ = mapDims.size.z / 65535.0f;

		float sizeHalfX = size / 2.0f;
		float sizeHalfY = size / 2.0f;
		float sizeHalfZ = (maxZ - minZ) / 2.0f;

		float midX = x + sizeHalfX;
		float midY = y + sizeHalfY;
		float midZ = minZ + sizeHalfZ;

		sizeHalfX *= scaleX;
		sizeHalfY *= scaleY;
		sizeHalfZ *= scaleZ;

		glm::vec3 center = {
			mapDims.min.x + midX * scaleX,
			mapDims.min.y + midY * scaleY,
			mapDims.min.z + midZ * scaleZ
		};

		float radius = sizeHalfX * sizeHalfX + sizeHalfY * sizeHalfY + 
			sizeHalfZ * sizeHalfZ;

		return { center, radius };
	}

	QuadTree::~QuadTree()
	{
		destroy();
	}

	bool QuadTree::create(const MapDimension& mapDimension, const HeightMap& heightMap, uint32_t leafNodeSize, uint32_t numLODs)
	{
		mMapDimension = &mapDimension;
		mHeightMap = &heightMap;
		mLeafNodeSize = leafNodeSize;
		mNumLODs = numLODs;

		auto& size = mHeightMap->getSize();
		if (size.x > 65535 || size.y > 65535)
		{
			return false;
		}

		if (mNumLODs > kMaxSupportedLOD)
		{
			return false;
		}

		mLeafNodeWorldSize = {
			leafNodeSize * mapDimension.size.x / (float)size.x,
			leafNodeSize * mapDimension.size.y / (float)size.y
		};

		mLODLevelNodeDiagSizes[0] = std::sqrtf(mLeafNodeWorldSize.x * mLeafNodeWorldSize.x +
			mLeafNodeWorldSize.y * mLeafNodeWorldSize.y);

		uint32_t totalNumNodes{ 0 };
		mTopNodeSize = mLeafNodeSize;

		for (uint32_t idx = 0; idx < mNumLODs; idx++)
		{
			if (idx > 0)
			{
				mTopNodeSize *= 2;
				mLODLevelNodeDiagSizes[idx] = 2 * mLODLevelNodeDiagSizes[idx - 1];
			}

			uint32_t numNodesX = (size.x - 1) / mTopNodeSize + 1;
			uint32_t numNodesY = (size.y - 1) / mTopNodeSize + 1;

			totalNumNodes += numNodesX * numNodesY;
		}

		mNodes.resize(totalNumNodes);
		uint32_t nodeCounter = 0;

		mNumTopNodes.x = (size.x - 1) / mTopNodeSize + 1;
		mNumTopNodes.y = (size.y - 1) / mTopNodeSize + 1;
		mTopNodes.resize(mNumTopNodes.y);

		for (uint32_t y = 0; y < mNumTopNodes.y; y++)
		{
			mTopNodes[y].resize(mNumTopNodes.x);
			for (uint32_t x = 0; x < mNumTopNodes.x; x++)
			{
				mTopNodes[y][x] = &mNodes[nodeCounter];
				nodeCounter++;

				mTopNodes[y][x]->create(this, x * mTopNodeSize, y * mTopNodeSize, 
					mTopNodeSize, 0, nodeCounter);
			}
		}

		return true;
	}

	void QuadTree::destroy()
	{
		mNodes.clear();
		mTopNodes.clear();
		mMapDimension = nullptr;
		mHeightMap = nullptr;
	}

	void QuadTree::selectLOD(const SelectionParams& selectionParams)
	{
		SelectedLOD* selected = selectionParams.selected;
		Camera* camera = selectionParams.camera;

		float lodNear{ 0 };
		float lodFar{ selectionParams.visibilityDistance };
		float detailBalance{ selectionParams.lodDistanceRatio };
		float total{ 0.0f };
		float currentDetailBalance{ 1.0f };

		selected->visDistTooSmall = false;
		for (uint32_t idx = 0; idx < mNumLODs; idx++)
		{
			total *= currentDetailBalance;
			currentDetailBalance *= detailBalance;
		}

		float sect = (lodFar - lodNear) / total;
		float prevPos = lodNear;

		currentDetailBalance = 1.0f;
		for (uint32_t idx = 0; idx < mNumLODs; idx++)
		{
			mVisibilityRanges[mNumLODs - idx - 1] = prevPos + sect * currentDetailBalance;
			prevPos = mVisibilityRanges[mNumLODs - idx - 1];
			currentDetailBalance *= detailBalance;
		}

		prevPos = lodNear;
		for (uint32_t idx = 0; idx < mNumLODs; idx++)
		{
			mMorphEnd[idx] = mVisibilityRanges[mNumLODs - idx - 1];
			mMorphStart[idx] = prevPos + (mMorphEnd[idx] - prevPos) * selectionParams.morphStartRatio;
			prevPos = mMorphStart[idx];
		}

		for (uint32_t y = 0; y < mNumTopNodes.y; y++)
		{
			for (uint32_t x = 0; x < mNumTopNodes.x; x++)
			{
				mTopNodes[y][x]->selectLOD(this, selected, mNumLODs - 1, camera);
			}
		}

		const auto& cameraPos = camera->getNode()->getTransform().getTranslation();

		selected->maxLODLevel = 0;
		selected->minLODLevel = kMaxSupportedLOD;

		for (uint32_t idx = 0; idx < selected->numSelections; idx++)
		{
			SelectedNode* selectionBuffer = selected->selectedNodes.data();
			BoundingBox nodeBox = selectionBuffer[idx].getBoundingBox(mHeightMap->getSize(), *mMapDimension);

			if (selectionParams.sortByDistance)
			{
				selectionBuffer[idx].minDistToCamera = std::sqrtf(nodeBox.getMinDistanceFromPointSq(cameraPos));
			}
			else
			{
				selectionBuffer[idx].minDistToCamera = 0.0f;
			}
		}

		if (selectionParams.sortByDistance)
		{
			SelectedNode* selectionBuffer = selected->selectedNodes.data();
			std::sort(selectionBuffer, selectionBuffer + selected->numSelections, [](const auto& arg1, const auto& arg2) {
				return arg1.minDistToCamera > arg2.minDistToCamera;
			});
		}
	}

	float QuadTree::getVisibilityRange(uint32_t lodLevel) const
	{
		return mVisibilityRanges[lodLevel];
	}

	float QuadTree::getMorphStart(uint32_t lodLevel) const
	{
		return mMorphStart[lodLevel];
	}

	float QuadTree::getMorphEnd(uint32_t lodLevel) const
	{
		return mMorphEnd[lodLevel];
	}

	float QuadTree::getLODLevelNodeDiagonalSize(uint32_t lodLevel) const
	{
		return mLODLevelNodeDiagSizes[lodLevel];
	}

	glm::vec4 QuadTree::getMorphConstant(uint32_t lodLevel)
	{
		float start = mMorphStart[lodLevel];
		float end = mMorphEnd[lodLevel];

		const float errorFudge = 0.01f;
		end = std::lerp(end, start, errorFudge);

		return { start, 1.0f / (end - start), end / (end - start), 1.0f / (end - start) };
	}

	void QuadTree::getAreaMinMaxHeight(const glm::uvec2& from, const glm::uvec2& size, float& minZ, float& maxZ) const
	{
		float bfx = (from.x - mMapDimension->min.x) / mMapDimension->size.x;
		float bfy = (from.y - mMapDimension->min.y) / mMapDimension->size.y;
		float btx = (from.x + size.x - mMapDimension->min.x) / mMapDimension->size.x;
		float bty = (from.y + size.y - mMapDimension->min.y) / mMapDimension->size.y;

		auto& hmSize = mHeightMap->getSize();

		auto sizeFromX = std::clamp((uint32_t)(bfx * hmSize.x), 0u, hmSize.x - 1);
		auto sizeFromY = std::clamp((uint32_t)(bfy * hmSize.y), 0u, hmSize.y - 1);
		auto sizeToX = std::clamp((uint32_t)(btx * hmSize.x), 0u, hmSize.x - 1);
		auto sizeToY = std::clamp((uint32_t)(bty * hmSize.y), 0u, hmSize.y - 1);

		auto baseFromX = sizeFromX / mTopNodeSize;
		auto baseFromY = sizeFromY / mTopNodeSize;
		auto baseToX = sizeToX / mTopNodeSize;
		auto baseToY = sizeToY / mTopNodeSize;

		minZ = FLT_MAX;
		maxZ = -FLT_MAX;

		for (uint32_t y = baseFromY; y <= baseToY; y++)
		{
			for (uint32_t x = baseFromX; x <= baseToX; x++)
			{
				mTopNodes[y][x]->getAreaMinMaxHeight(this, { sizeFromX, sizeFromY }, { sizeToX, sizeToY }, minZ, maxZ);
			}
		}
	}
}