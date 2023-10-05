#include "Scene/Components/Transform.h"
#include "Scene/Node.h"
#include "Core/Debugger.h"
#include "glm/gtx/matrix_decompose.hpp"

namespace Trinity
{
	Transform::Transform(Node& node)
		: mNode(&node)
	{
		mNode->addComponent(*this);
	}

	std::type_index Transform::getType() const
	{
		return typeid(Transform);
	}

	glm::mat4 Transform::getMatrix() const
	{
		return glm::translate(glm::mat4(1.0f), mTranslation) *
			glm::mat4_cast(mRotation) *
			glm::scale(glm::mat4(1.0f), mScale);
	}

	void Transform::setMatrix(const glm::mat4& matrix)
	{
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(matrix, mScale, mRotation, mTranslation, skew, perspective);

		invalidateWorldMatrix();
	}

	void Transform::setTranslation(const glm::vec3& translation)
	{
		mTranslation = translation;
		invalidateWorldMatrix();
	}

	void Transform::setRotation(const glm::quat& rotation)
	{
		mRotation = rotation;
		invalidateWorldMatrix();
	}

	void Transform::setScale(const glm::vec3& scale)
	{
		mScale = scale;
		invalidateWorldMatrix();
	}

	void Transform::invalidateWorldMatrix()
	{
		mUpdateMatrix = true;
	}

	void Transform::updateWorldTransform()
	{
		if (!mUpdateMatrix)
		{
			return;
		}

		mWorldMatrix = getMatrix();

		auto parent = mNode->getParent();
		if (parent != nullptr)
		{
			auto& transform = parent->getTransform();
			mWorldMatrix = transform.getWorldMatrix() * mWorldMatrix;
		}

		mUpdateMatrix = false;
	}
}