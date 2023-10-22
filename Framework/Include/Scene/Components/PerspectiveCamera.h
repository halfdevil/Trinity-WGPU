#pragma once

#include "Scene/Components/Camera.h"

namespace Trinity
{
	class PerspectiveCamera : public Camera
	{
	public:

		PerspectiveCamera() = default;
		virtual ~PerspectiveCamera() = default;

		PerspectiveCamera(const PerspectiveCamera&) = delete;
		PerspectiveCamera& operator = (const PerspectiveCamera&) = delete;

		PerspectiveCamera(PerspectiveCamera&&) = default;
		PerspectiveCamera& operator = (PerspectiveCamera&&) = default;

		float getAspectRatio() const
		{
			return mAspectRatio;
		}

		float getFOV() const
		{
			return mFOV;
		}

		float getFarPlane() const
		{
			return mFarPlane;
		}

		float getNearPlane() const
		{
			return mNearPlane;
		}

		virtual glm::mat4 getProjection() const override;
		virtual void setAspectRatio(float aspectRatio);
		virtual void setFOV(float fov);
		virtual void setFarPlane(float farPlane);
		virtual void setNearPlane(float nearPlane);

		virtual bool read(FileReader& reader, Scene& scene) override;
		virtual bool write(FileWriter& writer, Scene& scene) override;

		virtual std::string getTypeStr() const override;

	public:

		static std::string getStaticType();

	protected:

		float mAspectRatio{ 0.0f };
		float mFOV{ glm::radians(60.0f) };
		float mFarPlane{ 1000.0f };
		float mNearPlane{ 0.1f };
	};
}