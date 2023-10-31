#include "Scene/Terrain/QuadTree.h"
#include <queue>

namespace Trinity
{
	QuadTree::~QuadTree()
	{
		destroy();
	}

	bool QuadTree::create(float patchSize, const BoundingBox& boundingBox)
	{
		auto root = std::make_unique<Node>();
		root->boundingBox = boundingBox;

		mNodes.push_back(std::move(root));
		mRoot = 0;

		std::queue<int32_t> nodes;
		nodes.push(mRoot);

		while (!nodes.empty())
		{
			auto nodeIdx = nodes.front();
			nodes.pop();

			if (nodeIdx == -1)
			{
				continue;
			}

			auto& node = mNodes[nodeIdx];
			auto& boundingBox = node->boundingBox;
			auto size = boundingBox.getSize();

			if (size.x <= patchSize || size.z <= patchSize)
			{
				continue;
			}

			auto startX = boundingBox.min.x;
			auto startZ = boundingBox.min.z;
			auto minY = boundingBox.min.y;
			auto maxY = boundingBox.max.y;

			BoundingBox b1(glm::vec3(startX, minY, startZ + size.z / 2),
				glm::vec3(startX + size.x / 2, maxY, size.z));

			BoundingBox b2(glm::vec3(startX + size.x / 2, minY, startZ + size.z / 2),
				glm::vec3(size.x, maxY, size.z));

			BoundingBox b3(glm::vec3(startX, minY, startZ),
				glm::vec3(startX + size.x / 2, maxY, startZ + size.z / 2));

			BoundingBox b4(glm::vec3(startX + size.x / 2, minY, startZ),
				glm::vec3(size.x, maxY, startZ + size.z / 2));

			auto ul = std::make_unique<Node>();
			ul->boundingBox = b1;

			auto ur = std::make_unique<Node>();
			ur->boundingBox = b2;

			auto ll = std::make_unique<Node>();
			ll->boundingBox = b3;

			auto lr = std::make_unique<Node>();
			lr->boundingBox = b4;

			auto numNodes = (uint32_t)mNodes.size();
			node->ul = numNodes + 0;
			node->ur = numNodes + 1;
			node->ll = numNodes + 2;
			node->lr = numNodes + 3;

			nodes.push(node->ul);
			nodes.push(node->ur);
			nodes.push(node->ll);
			nodes.push(node->lr);

			mNodes.push_back(std::move(ul));
			mNodes.push_back(std::move(ur));
			mNodes.push_back(std::move(ll));
			mNodes.push_back(std::move(lr));
		}

		mVisibility.resize(mNodes.size());
		std::fill(mVisibility.begin(), mVisibility.end(), false);

		return mNodes.size() != 0;
	}

	void QuadTree::destroy()
	{
		mRoot = -1;
		mNodes.clear();
	}

	int32_t QuadTree::getNodeIndex(const BoundingBox& boundingBox)
	{
		for (int32_t idx = 0; idx < (int32_t)mNodes.size(); idx++)
		{
			auto& node = mNodes[idx];
			auto& nodeBox = node->boundingBox;

			if (node->isLeaf() && nodeBox.contains(boundingBox))
			{
				return idx;
			}
		}

		return -1;
	}

	bool QuadTree::isVisible(uint32_t nodeIdx)
	{
		if (nodeIdx < (uint32_t)mVisibility.size())
		{
			return mVisibility[nodeIdx];
		}

		return false;
	}

	void QuadTree::update(const Frustum& frustum)
	{
		std::fill(mVisibility.begin(), mVisibility.end(), false);

		std::queue<int32_t> nodes;
		nodes.push(mRoot);

		while (!nodes.empty())
		{
			auto nodeIdx = nodes.front();
			nodes.pop();

			if (nodeIdx == -1)
			{
				continue;
			}

			auto& node = mNodes[nodeIdx];
			if (frustum.contains(node->boundingBox))
			{
				mVisibility[nodeIdx] = true;
				if (!node->isLeaf())
				{
					nodes.push(node->ul);
					nodes.push(node->ur);
					nodes.push(node->ll);
					nodes.push(node->lr);
				}
			}
		}
	}
}