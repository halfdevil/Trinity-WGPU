#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Trinity
{
	class BoundingSphere
	{
	public:

		BoundingSphere() = default;
		BoundingSphere(const glm::vec3& inCenter, float inRadius) :
			center(inCenter), radius(inRadius)
		{
		}		

	public:

		glm::vec3 center{ 0.0f };
		float radius{ 0.0f };
	};
}