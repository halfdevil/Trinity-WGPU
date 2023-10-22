#include "Scene/Components/PerspectiveCamera.h"
#include "Scene/ComponentFactory.h"
#include "VFS/FileReader.h"
#include "VFS/FileWriter.h"

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

	bool PerspectiveCamera::read(FileReader& reader, Scene& scene)
	{
		if (!Camera::read(reader, scene))
		{
			return false;
		}

		reader.read(&mFOV);
		reader.read(&mAspectRatio);
		reader.read(&mFarPlane);
		reader.read(&mNearPlane);

		return true;
	}

	bool PerspectiveCamera::write(FileWriter& writer, Scene& scene)
	{
		if (!Camera::write(writer, scene))
		{
			return false;
		}

		writer.write(&mFOV);
		writer.write(&mAspectRatio);
		writer.write(&mFarPlane);
		writer.write(&mNearPlane);

		return true;
	}

	size_t PerspectiveCamera::getHashCode() const
	{
		return typeid(PerspectiveCamera).hash_code();
	}
}