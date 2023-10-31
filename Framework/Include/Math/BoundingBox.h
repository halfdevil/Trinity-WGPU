#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define _USE_MATH_DEFINES
#include <cmath>

namespace Trinity
{
	class BoundingBox
	{
	public:

		BoundingBox() = default;
		BoundingBox(const glm::vec3& inMin, const glm::vec3& inMax)
			: min{ inMin }, max{ inMax }
		{
		}

		BoundingBox(const std::vector<glm::vec3>& points);

		glm::vec3 getSize() const;
		glm::vec3 getCenter() const;

		void transform(const glm::mat4& m);
		BoundingBox getTransformed(const glm::mat4& m) const;

		void fromPoints(const std::vector<glm::vec3>& points);
		void combinePoint(const glm::vec3& p);
		void combineBox(const BoundingBox& other);

		bool isPointInside(const glm::vec3& p) const;
		bool isIntersecting(const BoundingBox& other) const;
		bool contains(const BoundingBox& other) const;

	public:

		static BoundingBox combineBoxes(const std::vector<BoundingBox>& boxes);

	public:

		glm::vec3 min;
		glm::vec3 max;
	};
}