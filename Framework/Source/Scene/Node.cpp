#include "Scene/Node.h"

namespace Trinity
{
	Node::Node() :
		mTransform{ *this }
	{
	}

	void Node::setName(const std::string& name)
	{
		mName = name;
	}

	void Node::setParent(Node& parent)
	{
		mParent = &parent;
		mTransform.invalidateWorldMatrix();
	}

	void Node::addChild(Node& child)
	{
		mChildren.push_back(&child);
	}

	Component& Node::getComponent(const std::type_index& type)
	{
		return *mComponents.at(type);
	}

	bool Node::hasComponent(const std::type_index& type)
	{
		return mComponents.contains(type);
	}

	void Node::addComponent(Component& component)
	{
		auto it = mComponents.find(component.getType());
		if (it != mComponents.end())
		{
			it->second = &component;
		}
		else
		{
			mComponents.insert(std::make_pair(component.getType(), &component));
		}
	}
}