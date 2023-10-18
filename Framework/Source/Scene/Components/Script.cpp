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

	size_t Script::getHashCode() const
	{
		return typeid(Script).hash_code();
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

	size_t NodeScript::getHashCode() const
	{
		return typeid(NodeScript).hash_code();
	}

	void NodeScript::setNode(Node& node)
	{
		mNode = &node;
	}

	bool NodeScript::read(FileReader& reader, Scene& scene)
	{
		if (!Script::read(reader, scene))
		{
			return false;
		}

		uint32_t nodeId{ 0 };
		reader.read(&nodeId);
		mNode = scene.getNode(nodeId);

		return true;
	}

	bool NodeScript::write(FileWriter& writer, Scene& scene)
	{
		if (!Script::write(writer, scene))
		{
			return false;
		}

		const uint32_t nodeId = mNode->getId();
		writer.write(&nodeId);

		return true;
	}
}