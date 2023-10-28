#include "Math/Frustum.h"

namespace Trinity
{
	Frustum::Frustum(const glm::mat4& m)
	{
		fromMatrix(m);
	}

	bool Frustum::contains(const BoundingBox& box) const
	{
		for (uint32_t i = 0; i < 6; i++)
		{
			uint32_t r = 0;

			r += (glm::dot(planes[i], glm::vec4(box.min.x, box.min.y, box.min.z, 1.0f)) < 0.0) ? 1 : 0;
			r += (glm::dot(planes[i], glm::vec4(box.max.x, box.min.y, box.min.z, 1.0f)) < 0.0) ? 1 : 0;
			r += (glm::dot(planes[i], glm::vec4(box.min.x, box.max.y, box.min.z, 1.0f)) < 0.0) ? 1 : 0;
			r += (glm::dot(planes[i], glm::vec4(box.max.x, box.max.y, box.min.z, 1.0f)) < 0.0) ? 1 : 0;
			r += (glm::dot(planes[i], glm::vec4(box.min.x, box.min.y, box.max.z, 1.0f)) < 0.0) ? 1 : 0;
			r += (glm::dot(planes[i], glm::vec4(box.max.x, box.min.y, box.max.z, 1.0f)) < 0.0) ? 1 : 0;
			r += (glm::dot(planes[i], glm::vec4(box.min.x, box.max.y, box.max.z, 1.0f)) < 0.0) ? 1 : 0;
			r += (glm::dot(planes[i], glm::vec4(box.max.x, box.max.y, box.max.z, 1.0f)) < 0.0) ? 1 : 0;

			if (r == 8)
			{
				return false;
			}
		}

		uint32_t r = 0;

		r = 0; for (uint32_t i = 0; i < 8; i++) r += ((points[i].x > box.max.x) ? 1 : 0); if (r == 8) return false;
		r = 0; for (uint32_t i = 0; i < 8; i++) r += ((points[i].x < box.min.x) ? 1 : 0); if (r == 8) return false;
		r = 0; for (uint32_t i = 0; i < 8; i++) r += ((points[i].y > box.max.y) ? 1 : 0); if (r == 8) return false;
		r = 0; for (uint32_t i = 0; i < 8; i++) r += ((points[i].y < box.min.y) ? 1 : 0); if (r == 8) return false;
		r = 0; for (uint32_t i = 0; i < 8; i++) r += ((points[i].z > box.max.z) ? 1 : 0); if (r == 8) return false;
		r = 0; for (uint32_t i = 0; i < 8; i++) r += ((points[i].z < box.min.z) ? 1 : 0); if (r == 8) return false;

		return true;
	}

	void Frustum::fromMatrix(const glm::mat4& m)
	{
		const glm::mat4 tm = glm::transpose(m);
		const glm::mat4 im = glm::inverse(m);

		planes[0] = glm::vec4(tm[3] + tm[0]);
		planes[1] = glm::vec4(tm[3] - tm[0]);
		planes[2] = glm::vec4(tm[3] + tm[1]);
		planes[3] = glm::vec4(tm[3] - tm[1]);
		planes[4] = glm::vec4(tm[3] + tm[2]);
		planes[5] = glm::vec4(tm[3] - tm[2]);

		const glm::vec4 corners[] = {
			glm::vec4(-1, -1, -1, 1),
			glm::vec4(1, -1, -1, 1),
			glm::vec4(1, 1, -1, 1),
			glm::vec4(-1, 1, -1, 1),
			glm::vec4(-1, -1, 1, 1),
			glm::vec4(1, -1, 1, 1),
			glm::vec4(1, 1, 1, 1),
			glm::vec4(-1, 1, 1, 1)
		};

		for (uint32_t i = 0; i < 8; i++)
		{
			const glm::vec4 q = im * corners[i];
			points[i] = q / q.w;
		}
	}
}