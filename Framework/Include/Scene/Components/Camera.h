#pragma once

#include "Scene/Component.h"
#include "Math/Types.h"

namespace Trinity
{
	class Node;

	class Camera : public Component
	{
	public:

		Camera() = default;
		virtual ~Camera() = default;

		Camera(const Camera&) = delete;
		Camera& operator = (const Camera&) = delete;

		Camera(Camera&&) = default;
		Camera& operator = (Camera&&) = default;

		Node* getNode() const
		{
			return mNode;
		}

		glm::mat4 getView();

		virtual std::type_index getType() const override;
		virtual size_t getHashCode() const override;

		virtual glm::mat4 getProjection() const = 0;
		virtual void setNode(Node& node);

		virtual bool read(FileReader& reader, Scene& scene) override;
		virtual bool write(FileWriter& writer, Scene& scene) override;

	protected:

		Node* mNode{ nullptr };
	};
}