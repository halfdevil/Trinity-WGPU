#include "Scene/Components/Transform.h"
#include "Scene/Node.h"
#include "Scene/Scene.h"
#include "Core/Debugger.h"
#include "VFS/FileReader.h"
#include "VFS/FileWriter.h"
#include "glm/gtx/matrix_decompose.hpp"

namespace Trinity
{
	std::type_index Transform::getType() const
	{
		return typeid(Transform);
	}

	std::string Transform::getTypeStr() const
	{
		return getStaticType();
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

	void Transform::setNode(Node& node)
	{
		mNode = &node;
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

	bool Transform::read(FileReader& reader, Scene& scene)
	{
		if (!Component::read(reader, scene))
		{
			return false;
		}

		uint32_t nodeId{ 0 };
		reader.read(&nodeId);
		mNode = scene.getNode(nodeId);

		reader.read(&mTranslation);
		reader.read(&mRotation);
		reader.read(&mScale);

		mUpdateMatrix = true;
		return true;
	}

	bool Transform::write(FileWriter& writer, Scene& scene)
	{
		if (!Component::write(writer, scene))
		{
			return false;
		}

		const uint32_t nodeId = mNode->getId();
		writer.write(&nodeId);

		writer.write(&mTranslation);
		writer.write(&mRotation);
		writer.write(&mScale);

		return true;
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

	std::string Transform::getStaticType()
	{
		return "Transform";
	}
}