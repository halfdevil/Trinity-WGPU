#pragma once

#include "Math/BoundingBox.h"

namespace Trinity
{
	class Frustum
	{
	public:

		Frustum() = default;
		Frustum(const glm::mat4& m);

		bool contains(const BoundingBox& box) const;
		void fromMatrix(const glm::mat4& m);

	public:

		glm::vec4 planes[6];
		glm::vec4 points[8];
	};
}