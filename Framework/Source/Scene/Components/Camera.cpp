#include "Scene/Components/Camera.h"
#include "Scene/Node.h"
#include "Scene/Scene.h"
#include "VFS/FileReader.h"
#include "VFS/FileWriter.h"

namespace Trinity
{
	std::type_index Camera::getType() const
	{
		return typeid(Camera);
	}

	std::string Camera::getTypeStr() const
	{
		return getStaticType();
	}

	void Camera::setProjection(const glm::mat4& projection)
	{
		mProjection = projection;
	}

	void Camera::setView(const glm::mat4& view)
	{
		mView = view;
	}

	void Camera::setFrustum(const Frustum& frustum)
	{
		mFrustum = frustum;
	}

	void Camera::setNode(Node& node)
	{
		mNode = &node;
	}

	bool Camera::read(FileReader& reader, ResourceCache& cache, Scene& scene)
	{
		if (!Component::read(reader, cache, scene))
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