#include "Scene/Components/Camera.h"
#include "Scene/Node.h"

namespace Trinity
{
	glm::mat4 Camera::getView()
	{
		if (mNode == nullptr)
		{
			return glm::mat4(1.0f);
		}

		auto& transform = mNode->getComponent<Transform>();
		return glm::inverse(transform.getWorldMatrix());
	}

	std::type_index Camera::getType() const
	{
		return typeid(Camera);
	}

	void Camera::setNode(Node& node)
	{
		mNode = &node;
	}
}