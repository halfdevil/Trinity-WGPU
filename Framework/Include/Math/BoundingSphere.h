#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Trinity
{
	class BoundingSphere
	{
	public:

		BoundingSphere() = default;
		BoundingSphere(const glm::vec3& inPosition, float inRadius) :
			position(position), radius(inRadius)
		{
		}		

	public:

		glm::vec3 position{ 0.0f };
		float radius{ 0.0f };
	};
}