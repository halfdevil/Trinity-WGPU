#pragma once

#include "Scene/Component.h"
#include "Math/Types.h"
#include <glm/gtx/quaternion.hpp>

namespace Trinity
{
	class Node;

	class Transform : public Component
	{
	public:

		Transform() = default;
		virtual ~Transform() = default;

		Transform(const Transform&) = delete;
		Transform& operator = (const Transform&) = delete;

		Transform(Transform&&) = default;
		Transform& operator = (Transform&&) = default;

		Node* getNode() const
		{
			return mNode;
		}

		const glm::vec3& getTranslation() const
		{
			return mTranslation;
		}

		const glm::quat& getRotation() const
		{
			return mRotation;
		}

		const glm::vec3& getScale() const
		{
			return mScale;
		}

		const glm::mat4& getWorldMatrix() const
		{
			const_cast<Transform*>(this)->updateWorldTransform();
			return mWorldMatrix;
		}

		virtual std::type_index getType() const override;
		virtual std::string getTypeStr() const override;

		glm::mat4 getMatrix() const;
		void setMatrix(const glm::mat4& matrix);

		void setNode(Node& node);
		void setTranslation(const glm::vec3& translation);
		void setRotation(const glm::quat& rotation);
		void setScale(const glm::vec3& scale);
		void invalidateWorldMatrix();

		virtual bool read(FileReader& reader, ResourceCache& cache, Scene& scene) override;
		virtual bool write(FileWriter& writer, Scene& scene) override;

	public:

		static std::string getStaticType();

	protected:

		void updateWorldTransform();

	protected:

		Node* mNode{ nullptr };
		glm::vec3 mTranslation{ 0.0f, 0.0f, 0.0f };
		glm::quat mRotation{ 1.0f, 0.0f, 0.0f, 0.0f };
		glm::vec3 mScale{ 1.0f, 1.0f, 1.0f };
		glm::mat4 mWorldMatrix{ 1.0f };

	private:

		bool mUpdateMatrix{ false };
	};
}