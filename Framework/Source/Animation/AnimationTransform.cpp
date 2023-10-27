#include "Animation/AnimationTransform.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace Trinity
{
	glm::mat4 AnimationTransform::toMatrix() const
	{
		return glm::translate(glm::mat4(1.0f), translation) *
			glm::mat4_cast(rotation) *
			glm::scale(glm::mat4(1.0f), scale);
	}

	void AnimationTransform::fromMatrix(const glm::mat4& m)
	{
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(m, scale, rotation, translation, skew, perspective);
	}

	glm::dualquat AnimationTransform::toDualQuat() const
	{
		glm::quat d(translation.x, translation.y, translation.z, 0.0f);
		glm::quat qr = rotation;
		glm::quat qd = qr * d * 0.5f;

		return glm::dualquat(qr, qd);
	}

	void AnimationTransform::fromDualQuat(const glm::dualquat& d)
	{
		glm::quat dq = -d.real * (d.dual * 2.0f);
		translation = glm::vec3(dq.x, dq.y, dq.z);
		rotation = d.real;
	}

	bool AnimationTransform::operator==(const AnimationTransform& t) const
	{
		return translation == t.translation &&
			rotation == t.rotation &&
			scale == t.scale;
	}

	bool AnimationTransform::operator!=(const AnimationTransform& t) const
	{
		return translation != t.translation ||
			rotation != t.rotation ||
			scale != t.scale;
	}

	AnimationTransform AnimationTransform::lerp(const AnimationTransform& a, const AnimationTransform& b, float t)
	{
		glm::quat bRot = b.rotation;
		if (glm::dot(a.rotation, bRot) < 0.0f)
		{
			bRot = -bRot;
		}

		return {
			glm::mix(a.translation, b.translation, t),
			glm::normalize(glm::lerp(a.rotation, bRot, t)),
			glm::mix(a.scale, b.scale, t)
		};
	}
}