#include "Scene/Components/Script.h"
#include "Scene/Node.h"
#include "Scene/Scene.h"
#include "VFS/FileReader.h"
#include "VFS/FileWriter.h"

namespace Trinity
{
	std::type_index Script::getType() const
	{
		return typeid(Script);
	}

	std::string Script::getTypeStr() const
	{
		return getStaticType();
	}

	void Script::init()
	{
	}

	void Script::update(float deltaTime)
	{
	}

	void Script::resize(uint32_t width, uint32_t height)
	{
	}

	void Script::setNode(Node& node)
	{
		mNode = &node;
	}

	bool Script::read(FileReader& reader, Scene& scene)
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

	bool Script::write(FileWriter& writer, Scene& scene)
	{
		if (!Component::write(writer, scene))
		{
			return false;
		}

		const uint32_t nodeId = mNode->getId();
		writer.write(&nodeId);

		return true;
	}

	std::string Script::getStaticType()
	{
		return "Script";
	}
}