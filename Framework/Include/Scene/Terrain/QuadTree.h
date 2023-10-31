#pragma once

#include "Math/BoundingBox.h"
#include "Math/Frustum.h"
#include <memory>
#include <vector>

namespace Trinity
{
	class QuadTree
	{
	public:

		struct Node
		{
			bool isLeaf() const
			{
				return ul == -1 && ur == -1 && 
					ll == -1 && lr == -1;
			}

			BoundingBox boundingBox;
			
			int32_t index{ -1 };
			int32_t ul{ -1 };
			int32_t ur{ -1 };
			int32_t ll{ -1 };
			int32_t lr{ -1 };
		};

		QuadTree() = default;
		virtual ~QuadTree();

		QuadTree(const QuadTree&) = delete;
		QuadTree& operator = (const QuadTree&) = delete;

		QuadTree(QuadTree&&) = default;
		QuadTree& operator = (QuadTree&&) = default;

		const std::vector<bool>& getVisibility() const
		{
			return mVisibility;
		}

		virtual bool create(float patchSize, const BoundingBox& boundingBox);
		virtual void destroy();

		virtual int32_t getNodeIndex(const BoundingBox& boundingBox);

		virtual bool isVisible(uint32_t nodeIdx);
		virtual void update(const Frustum& frustum);

	protected:

		int32_t mRoot{ -1 };
		std::vector<bool> mVisibility;
		std::vector<std::unique_ptr<Node>> mNodes;
	};
}