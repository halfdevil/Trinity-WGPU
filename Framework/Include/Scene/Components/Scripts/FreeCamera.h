#pragma once

#include "Scene/Components/Script.h"
#include "Math/Types.h"

namespace Trinity
{
	class Camera;
	class FreeCamera : public Script
	{
	public:

		FreeCamera() = default;
		virtual ~FreeCamera() = default;

		FreeCamera(const FreeCamera&) = delete;
		FreeCamera& operator = (const FreeCamera&) = delete;

		FreeCamera(FreeCamera&&) = delete;
		FreeCamera& operator = (FreeCamera&&) = delete;

		Camera* getCamera() const
		{
			return mCamera;
		}

		virtual std::string getTypeStr() const override;

		virtual void moveForward(float scale);
		virtual void moveRight(float scale);
		virtual void moveUp(float scale);
		virtual void turn(float scale);
		virtual void lookUp(float scale);

		virtual void init() override;
		virtual void update(float deltaTime) override;
		virtual void resize(uint32_t width, uint32_t height) override;

	public:

		static std::string getStaticType();

	public:

		float moveSpeed{ 1.0f };
		float rotationSpeed{ 1.0f };

	protected:

		Camera* mCamera{ nullptr };
		glm::vec3 mTranslation{ 0.0f };
		glm::vec3 mRotation{ 0.0f };
	};
}