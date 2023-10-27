#pragma once

#include "Math/Types.h"

namespace Trinity
{
	class Math
	{
	public:

		template <typename T>
		static T Hermite(float t, const T& p1, const T& s1, const T& p2, const T& s2)
		{
			float tt = t * t;
			float ttt = tt * t;

			float h1 = 2.0f * ttt - 3.0f * tt + 1.0f;
			float h2 = -2.0f * ttt + 3.0f * tt;
			float h3 = ttt - 2.0f * tt + t;
			float h4 = ttt - tt;

			return p1 * h1 + p2 * h2 + s1 * h3 + s2 * h4;
		}

		static glm::quat Hermite(float t, const glm::quat& p1, const glm::quat& s1,
			const glm::quat& p2, const glm::quat& s2)
		{
			glm::quat p2n = p2;
			if (glm::dot(p1, p2) < 0.0f)
			{
				p2n = -p2;
			}

			float tt = t * t;
			float ttt = tt * t;

			float h1 = 2.0f * ttt - 3.0f * tt + 1.0f;
			float h2 = -2.0f * ttt + 3.0f * tt;
			float h3 = ttt - 2.0f * tt + t;
			float h4 = ttt - tt;

			return glm::normalize(p1 * h1 + p2n * h2 + s1 * h3 + s2 * h4);
		}

		static float interpolate(float a, float b, float t)
		{
			return a + (b - a) * t;
		}

		static glm::vec3 interpolate(const glm::vec3& a, const glm::vec3& b, float t)
		{
			return glm::mix(a, b, t);
		}

		static glm::quat interpolate(const glm::quat& a, const glm::quat& b, float t)
		{
			if (glm::dot(a, b) < 0.0f)
			{
				return glm::normalize(glm::lerp(a, -b, t));
			}

			return glm::normalize(glm::lerp(a, b, t));
		}
	};
}