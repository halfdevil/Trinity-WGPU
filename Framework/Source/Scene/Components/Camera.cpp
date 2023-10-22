#include "Scene/Components/Camera.h"
#include "Scene/Node.h"
#include "Scene/Scene.h"
#include "VFS/FileReader.h"
#include "VFS/FileWriter.h"

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

	std::string Camera::getTypeStr() const
	{
		return getStaticType();
	}

	void Camera::setNode(Node& node)
	{
		mNode = &node;
	}

	bool Camera::read(FileReader& reader, Scene& scene)
	{
		if (!Component::read(reader, scene))
		{
			return false;
		}

		uint32_t nodeId{ 0 };
		reader.read(&nodeId);
		mNode = scene.getNode(nodeId);

		return true;
	}

	bool Camera::write(FileWriter& writer, Scene& scene)
	{
		if (!Component::write(writer, scene))
		{
			return false;
		}

		const uint32_t nodeId = mNode->getId();
		writer.write(&nodeId);

		return true;
	}

	std::string Camera::getStaticType()
	{
		return "Camera";
	}
}