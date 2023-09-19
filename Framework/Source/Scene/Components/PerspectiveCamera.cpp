#include "Scene/Components/PerspectiveCamera.h"

namespace Trinity
{
	glm::mat4 PerspectiveCamera::getProjection() const
	{
		return glm::perspective(mFOV, mAspectRatio, mNearPlane, mFarPlane);
	}

	void PerspectiveCamera::setAspectRatio(float aspectRatio)
	{
		mAspectRatio = aspectRatio;
	}

	void PerspectiveCamera::setFOV(float fov)
	{
		mFOV = fov;
	}

	void PerspectiveCamera::setFarPlane(float farPlane)
	{
		mFarPlane = farPlane;
	}

	void PerspectiveCamera::setNearPlane(float nearPlane)
	{
		mNearPlane = nearPlane;
	}
}