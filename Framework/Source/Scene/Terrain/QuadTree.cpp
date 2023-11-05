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
		this->z = node.z;
		this->size = node.size;
		this->minY = node.minY;
		this->maxY = node.maxY;
	}

	BoundingBox SelectedNode::getBoundingBox(const glm::uvec2& inSize, const MapDimension& mapDims) const
	{
		glm::vec3 min { 
			mapDims.min.x + x * mapDims.size.x / (float)(inSize.x - 1),
			mapDims.min.y + minY * mapDims.size.y / 65535.0f,
			mapDims.min.z + z * mapDims.size.z / (float)(inSize.y - 1)
		};

		glm::vec3 max {
			mapDims.min.x + (x + size) * mapDims.size.x / (float)(inSize.x - 1),
			mapDims.min.y + maxY * mapDims.size.y / 65535.0f,
			mapDims.min.z + (z + size) * mapDims.size.y / (float)(inSize.y - 1)
		};

		return { min, max };
	}

	SelectedLOD::SelectedLOD(uint32_t inMaxNumSelections) :
		maxNumSelections(inMaxNumSelections)
	{
		selectedNodes.resize(inMaxNumSelections);
	}

	void QuadTreeNode::create(QuadTree* quadTree, uint32_t inX, uint32_t inZ, uint32_t inSize, uint32_t inLevel, uint32_t lastIndex)
	{
		x = (uint16_t)inX;
		z = (uint16_t)inZ;
		size = (uint16_t)inSize;
		level = (uint16_t)inLevel;

		const auto* heightMap = quadTree->getHeightMap();
		const auto* mapDims = quadTree->getMapDimension();
		auto& nodes = quadTree->getNodes();
		auto& hmSize = heightMap->getSize();

		subTL = nullptr;
		subTR = nullptr;
		subBL = nullptr;
		subBR = nullptr;

		if (size == quadTree->getLeafNodeSize())
		{
			level |= 0x8000;

			uint32_t limitX = std::min(hmSize.x, (uint32_t)x + size + 1);
			uint32_t limitZ = std::min(hmSize.y, (uint32_t)z + size + 1);

			heightMap->getMinMaxHeight(x, z, limitX - x, limitZ - z, minY, maxY);

			float* pTLZ = (float*)&subTL;
			float* pTRZ = (float*)&subTR;
			float* pBLZ = (float*)&subBL;
			float* pBRZ = (float*)&subBR;

			limitX = std::min(hmSize.x - 1, (uint32_t)x + size);
			limitZ = std::min(hmSize.y - 1, (uint32_t)z + size);

			*pTLZ = mapDims->min.y + heightMap->getHeight(x, z) * mapDims->size.y / 65535.0f;
			*pTRZ = mapDims->min.y + heightMap->getHeight(limitX, z) * mapDims->size.y / 65535.0f;
			*pBLZ = mapDims->min.y + heightMap->getHeight(x, limitZ) * mapDims->size.y / 65535.0f;
			*pBRZ = mapDims->min.y + heightMap->getHeight(limitX, limitZ) * mapDims->size.y / 65535.0f;
		}
		else
		{
			uint32_t subSize = size / 2;

			subTL = &nodes[lastIndex++];
			subTL->create(quadTree, x, z, subSize, level + 1, lastIndex);
			minY = subTL->minY;
			maxY = subTL->maxY;

			if ((x + subSize) < hmSize.x)
			{
				subTR = &nodes[lastIndex++];
				subTR->create(quadTree, x + subSize, z, subSize, level + 1, lastIndex);
				minY = std::min(minY, subTR->minY);
				maxY = std::max(maxY, subTR->maxY);
			}

			if ((z + subSize) < hmSize.y)
			{
				subBL = &nodes[lastIndex++];
				subBL->create(quadTree, x, z + subSize, subSize, level + 1, lastIndex);
				minY = std::min(minY, subBL->minY);
				maxY = std::max(maxY, subBL->maxY);
			}

			if ((x + subSize) < hmSize.x && (z + subSize) < hmSize.y)
			{
				subBR = &nodes[lastIndex++];
				subBR->create(quadTree, x + subSize, z + subSize, subSize, level + 1, lastIndex);
				minY = std::min(minY, subBR->minY);
				maxY = std::max(maxY, subBR->maxY);
			}
		}
	}

	void QuadTreeNode::getAreaMinMaxHeight(const QuadTree* quadTree, const glm::uvec2& from,
		const glm::uvec2& to, float& outMinY, float& outMaxY) const
	{
		if (to.x < (uint32_t)x || to.y < (uint32_t)z || from.x > (uint32_t)(x + size) ||
			from.y > (uint32_t)(z + size))
		{
			return;
		}

		if (isLeaf() || (from.x < (uint32_t)x && from.y < (uint32_t)z && 
			to.x > (uint32_t)(x + size) && to.y > (uint32_t)(z + size)))
		{
			auto worldY = getWorldMinMaxY(*quadTree->getMapDimension());			
			outMinY = std::min(outMinY, worldY.x);
			outMaxY = std::max(outMaxY, worldY.y);

			return;
		}

		if (subTL != nullptr)
		{
			subTL->getAreaMinMaxHeight(quadTree, from, to, outMinY, outMaxY);
		}

		if (subTR != nullptr)
		{
			subTR->getAreaMinMaxHeight(quadTree, from, to, outMinY, outMaxY);
		}

		if (subBL != nullptr)
		{
			subBL->getAreaMinMaxHeight(quadTree, from, to, outMinY, outMaxY);
		}

		if (subBR != nullptr)
		{
			subBR->getAreaMinMaxHeight(quadTree, from, to, outMinY, outMaxY);
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

	glm::vec2 QuadTreeNode::getWorldMinMaxZ(uint32_t sizeZ, const MapDimension& mapDims) const
	{
		return {
			mapDims.min.z + z * mapDims.size.y / (float)(sizeZ - 1),
			mapDims.min.z + (z + size) * mapDims.size.y / (float)(sizeZ - 1)
		};
	}

	glm::vec2 QuadTreeNode::getWorldMinMaxY(const MapDimension& mapDims) const
	{
		return {
			mapDims.min.y + minY * mapDims.size.y / 65535.0f,
			mapDims.min.y + maxY * mapDims.size.y / 65535.0f
		};
	}

	BoundingBox QuadTreeNode::getBoundingBox(const glm::uvec2& inSize, const MapDimension& mapDims) const
	{
		auto minMaxX = getWorldMinMaxX(inSize.x, mapDims);
		auto minMaxY = getWorldMinMaxY(mapDims);
		auto minMaxZ = getWorldMinMaxZ(inSize.y, mapDims);

		return {
			{ minMaxX.x, minMaxY.x, minMaxZ.x },
			{ minMaxX.y, minMaxY.y, minMaxZ.y }
		};
	}

	BoundingSphere QuadTreeNode::getBoundingSphere(const glm::uvec2& inSize, const MapDimension& mapDims) const
	{
		float scaleX = mapDims.size.x / (float)(inSize.x - 1);
		float scaleY = mapDims.size.y / 65535.0f;
		float scaleZ = mapDims.size.z / (float)(inSize.y - 1);

		float sizeHalfX = size / 2.0f;
		float sizeHalfY = (maxY - minY) / 2.0f;
		float sizeHalfZ = size / 2.0f;

		float midX = x + sizeHalfX;
		float midY = minY + sizeHalfY;
		float midZ = z + sizeHalfZ;

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

		for (uint32_t z = 0; z < mNumTopNodes.y; z++)
		{
			mTopNodes[z].resize(mNumTopNodes.x);
			for (uint32_t x = 0; x < mNumTopNodes.x; x++)
			{
				mTopNodes[z][x] = &mNodes[nodeCounter];
				nodeCounter++;

				mTopNodes[z][x]->create(this, x * mTopNodeSize, z * mTopNodeSize, 
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
			total += currentDetailBalance;
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

		for (uint32_t z = 0; z < mNumTopNodes.y; z++)
		{
			for (uint32_t x = 0; x < mNumTopNodes.x; x++)
			{
				mTopNodes[z][x]->selectLOD(this, selected, mNumLODs - 1, camera);
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

			selected->minLODLevel = std::min(selected->minLODLevel, selectionBuffer[idx].lodLevel);
			selected->maxLODLevel = std::max(selected->maxLODLevel, selectionBuffer[idx].lodLevel);
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

	void QuadTree::getAreaMinMaxHeight(const glm::uvec2& from, const glm::uvec2& size, float& minY, float& maxY) const
	{
		float bfx = (from.x - mMapDimension->min.x) / mMapDimension->size.x;
		float bfz = (from.y - mMapDimension->min.y) / mMapDimension->size.y;
		float btx = (from.x + size.x - mMapDimension->min.x) / mMapDimension->size.x;
		float btz = (from.y + size.y - mMapDimension->min.y) / mMapDimension->size.y;

		auto& hmSize = mHeightMap->getSize();

		auto sizeFromX = std::clamp((uint32_t)(bfx * hmSize.x), 0u, hmSize.x - 1);
		auto sizeFromZ = std::clamp((uint32_t)(bfz * hmSize.y), 0u, hmSize.y - 1);
		auto sizeToX = std::clamp((uint32_t)(btx * hmSize.x), 0u, hmSize.x - 1);
		auto sizeToZ = std::clamp((uint32_t)(btz * hmSize.y), 0u, hmSize.y - 1);

		auto baseFromX = sizeFromX / mTopNodeSize;
		auto baseFromZ = sizeFromZ / mTopNodeSize;
		auto baseToX = sizeToX / mTopNodeSize;
		auto baseToZ = sizeToZ / mTopNodeSize;

		minY = FLT_MAX;
		maxY = -FLT_MAX;

		for (uint32_t z = baseFromZ; z <= baseToZ; z++)
		{
			for (uint32_t x = baseFromX; x <= baseToX; x++)
			{
				mTopNodes[z][x]->getAreaMinMaxHeight(this, { sizeFromX, sizeFromZ }, { sizeToX, sizeToZ }, minY, maxY);
			}
		}
	}
}