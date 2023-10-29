#include "Scene/Components/Scripts/FreeCamera.h"
#include "Scene/Components/PerspectiveCamera.h"
#include "Scene/Components/Transform.h"
#include "Scene/Node.h"
#include "Scene/ComponentFactory.h"
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Trinity
{
	std::string FreeCamera::getTypeStr() const
	{
		return getStaticType();
	}

	void FreeCamera::moveForward(float scale)
	{
		mTranslation.z += scale * moveSpeed;
	}

	void FreeCamera::moveRight(float scale)
	{
		mTranslation.x += scale * moveSpeed;
	}

	void FreeCamera::moveUp(float scale)
	{
		mTranslation.y += scale * moveSpeed;
	}

	void FreeCamera::turn(float scale)
	{
		mRotation.y += scale * rotationSpeed;
	}

	void FreeCamera::lookUp(float scale)
	{
		mRotation.x += scale * rotationSpeed;
	}

	void FreeCamera::init()
	{
		Script::init();

		auto* cameraNode = getNode();
		if (cameraNode->hasComponent<Camera>())
		{
			mCamera = &cameraNode->getComponent<Camera>();
		}
	}

	void FreeCamera::update(float deltaTime)
	{
		Script::update(deltaTime);

		mTranslation *= deltaTime;
		mRotation *= deltaTime;

		if (mRotation != glm::vec3(0.0f) || mTranslation != glm::vec3(0.0f))
		{
			auto& transform = getNode()->getTransform();

			glm::quat qx = glm::angleAxis(mRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
			glm::quat qy = glm::angleAxis(mRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::quat orientation = glm::normalize(qy * transform.getRotation() * qx);

			transform.setTranslation(transform.getTranslation() + mTranslation * glm::conjugate(orientation));
			transform.setRotation(orientation);

			mCamera->setView(glm::inverse(transform.getWorldMatrix()));
			mCamera->setFrustum({ mCamera->getProjection() * mCamera->getView() });
		}

		mTranslation = glm::vec3{ 0.0f };
		mRotation = glm::vec3{ 0.0f };
	}

	void FreeCamera::resize(uint32_t width, uint32_t height)
	{
		if (auto camera = dynamic_cast<PerspectiveCamera*>(mCamera))
		{
			camera->setAspectRatio((float)width / height);
			camera->updateProjection();
		}
	}

	std::string FreeCamera::getStaticType()
	{
		return "FreeCamera";
	}
}