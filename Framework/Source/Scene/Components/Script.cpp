#include "Scene/Components/Script.h"
#include "Scene/Node.h"

namespace Trinity
{
	std::type_index Script::getType() const
	{
		return typeid(Script);
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

	NodeScript::NodeScript(Node& node) :
		mNode(node)
	{
	}

	Node& NodeScript::getNode()
	{
		return mNode;
	}
}