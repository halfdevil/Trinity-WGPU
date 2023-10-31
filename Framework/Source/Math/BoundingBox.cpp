#include "Math/BoundingBox.h"

namespace Trinity
{
	BoundingBox::BoundingBox(const std::vector<glm::vec3>& points)
	{
		fromPoints(points);
	}

	glm::vec3 BoundingBox::getSize() const
	{
		return glm::vec3(max[0] - min[0], max[1] - min[1], max[2] - min[2]);
	}

	glm::vec3 BoundingBox::getCenter() const
	{
		return 0.5f * glm::vec3(max[0] + min[0], max[1] + min[1], max[2] + min[2]);
	}

	void BoundingBox::transform(const glm::mat4& m)
	{
		std::vector<glm::vec3> corners =
		{
			{ min.x, min.y, min.z },
			{ min.x, max.y, min.z },
			{ min.x, min.y, max.z },
			{ min.x, max.y, max.z },
			{ max.x, min.y, min.z },
			{ max.x, max.y, min.z },
			{ max.x, min.y, max.z },
			{ max.x, max.y, max.z }
		};

		for (auto& v : corners)
		{
			v = glm::vec3(m * glm::vec4(v, 1.0f));
		}

		fromPoints(corners);
	}

	BoundingBox BoundingBox::getTransformed(const glm::mat4& m) const
	{
		BoundingBox b = *this;
		b.transform(m);

		return b;
	}

	void BoundingBox::fromPoints(const std::vector<glm::vec3>& points)
	{
		glm::vec3 vmin(std::numeric_limits<float>::max());
		glm::vec3 vmax(std::numeric_limits<float>::lowest());

		for (uint32_t i = 0; i < (uint32_t)points.size(); i++)
		{
			if (points[i].x > vmax.x) vmax.x = points[i].x;
			if (points[i].y > vmax.y) vmax.y = points[i].y;
			if (points[i].z > vmax.z) vmax.z = points[i].z;

			if (points[i].x < vmin.x) vmin.x = points[i].x;
			if (points[i].y < vmin.y) vmin.y = points[i].y;
			if (points[i].z < vmin.z) vmin.z = points[i].z;
		}

		min = vmin;
		max = vmax;
	}

	void BoundingBox::combinePoint(const glm::vec3& p)
	{
		min = glm::min(min, p);
		max = glm::max(max, p);
	}

	void BoundingBox::combineBox(const BoundingBox& other)
	{
		combinePoint(other.min);
		combinePoint(other.max);
	}

	bool BoundingBox::isPointInside(const glm::vec3& p) const
	{
		return p.x >= min.x && p.x <= max.x &&
			p.y >= min.y && p.y <= max.y &&
			p.z >= min.z && p.z <= max.z;
	}

	bool BoundingBox::isIntersecting(const BoundingBox& other) const
	{
		return min.x <= other.max.x && max.x >= other.min.x &&
			min.y <= other.max.y && max.y >= other.min.y &&
			min.z <= other.max.z && max.z >= other.min.z;
	}

	bool BoundingBox::contains(const BoundingBox& other) const
	{
		return min.x <= other.min.x && max.x >= other.max.x &&
			min.y <= other.min.y && max.y >= other.max.y &&
			min.z <= other.min.z && max.z >= other.max.z;
	}

	BoundingBox BoundingBox::combineBoxes(const std::vector<BoundingBox>& boxes)
	{
		std::vector<glm::vec3> allPoints;
		allPoints.reserve(boxes.size() * 8);

		for (const auto& box : boxes)
		{
			allPoints.emplace_back(box.min.x, box.min.y, box.min.z);
			allPoints.emplace_back(box.min.x, box.min.y, box.max.z);
			allPoints.emplace_back(box.min.x, box.max.y, box.min.z);
			allPoints.emplace_back(box.min.x, box.max.y, box.max.z);

			allPoints.emplace_back(box.max.x, box.min.y, box.min.z);
			allPoints.emplace_back(box.max.x, box.min.y, box.max.z);
			allPoints.emplace_back(box.max.x, box.max.y, box.min.z);
			allPoints.emplace_back(box.max.x, box.max.y, box.max.z);
		}

		return { allPoints };
	}
}