#pragma once

#include "Math/Math.h"

namespace Trinity
{
	struct AnimationTransform
	{
	public:

		AnimationTransform() = default;
		AnimationTransform(const glm::vec3& p, const glm::quat& r, const glm::vec3& s)
			: translation(p), rotation(r), scale(s)
		{
		}

		AnimationTransform(const glm::mat4& m)
		{
			fromMatrix(m);
		}

		AnimationTransform(const AnimationTransform&) = default;
		AnimationTransform& operator = (const AnimationTransform&) = default;

		glm::mat4 toMatrix() const;
		void fromMatrix(const glm::mat4& m);

		glm::dualquat toDualQuat() const;
		void fromDualQuat(const glm::dualquat& d);

		bool operator == (const AnimationTransform& t) const;
		bool operator != (const AnimationTransform& t) const;

	public:

		static AnimationTransform lerp(const AnimationTransform& a, const AnimationTransform& b, float t);

	public:

		glm::vec3 translation;
		glm::quat rotation;
		glm::vec3 scale{ 1.0f };
	};
}