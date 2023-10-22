#include "Scene/Node.h"
#include "Scene/ComponentFactory.h"
#include "Scene/Scene.h"
#include "VFS/FileReader.h"
#include "VFS/FileWriter.h"
#include "Core/Logger.h"

namespace Trinity
{
	Node::Node()
	{
		mTransform.setNode(*this);
		mScriptContainer.setNode(*this);

		setComponent(mTransform);
		setComponent(mScriptContainer);
	}

	void Node::setName(const std::string& name)
	{
		mName = name;
	}

	void Node::setId(uint32_t id)
	{
		mId = id;
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

	void Node::setComponent(Component& component)
	{
		if (component.getType() == typeid(Script))
		{
			mScriptContainer.setScript(dynamic_cast<Script&>(component));
			return;
		}

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

	bool Node::read(FileReader& reader, Scene& scene)
	{
		mName = reader.readString();

		uint32_t numChildren{ 0 };
		reader.read(&numChildren);

		for (uint32_t idx = 0; idx < numChildren; idx++)
		{
			auto child = std::make_unique<Node>();
			if (!child->read(reader, scene))
			{
				LogError("Node::read() failed for child: %d!!", idx);
				return false;
			}

			addChild(*child);
			scene.addNode(std::move(child));
		}

		return true;
	}

	bool Node::write(FileWriter& writer, Scene& scene)
	{
		writer.writeString(mName);

		const uint32_t numChildren = (uint32_t)mChildren.size();
		writer.write(&numChildren);

		for (auto* child : mChildren)
		{
			if (!child->write(writer, scene))
			{
				LogError("Node::write() failed!!");
				return false;
			}
		}

		return true;
	}

	bool Node::readComponents(FileReader& reader, Scene& scene)
	{
		if (!mTransform.read(reader, scene))
		{
			LogError("Transform::read() failed!!");
			return false;
		}

		if (!mScriptContainer.read(reader, scene))
		{
			LogError("ScriptContainer::read() failed!!");
			return false;
		}

		uint32_t numComponents{ 0 };
		reader.read(&numComponents);

		for (uint32_t idx = 0; idx < numComponents; idx++)
		{
			std::string type = reader.readString();

			auto component = scene.getComponentFactory().createComponent(type);
			if (!component)
			{
				LogError("ComponentFactory::createComponent() failed for type: %s!!", type.c_str());
				return false;
			}

			if (!component->read(reader, scene))
			{
				LogError("Component::read() failed for type: %s!!", type.c_str());
				return false;
			}

			setComponent(*component);
			scene.addComponent(std::move(component), *this);
		}

		for (auto* child : mChildren)
		{
			child->readComponents(reader, scene);
		}

		return true;
	}

	bool Node::writeComponents(FileWriter& writer, Scene& scene)
	{
		if (!mTransform.write(writer, scene))
		{
			LogError("Transform::write() failed!!");
			return false;
		}

		if (!mScriptContainer.write(writer, scene))
		{
			LogError("ScriptContainer::write() failed!!");
			return false;
		}

		const uint32_t numComponents = (uint32_t)mComponents.size() - 2;
		writer.write(&numComponents);

		for (auto& it : mComponents)
		{
			if (it.first == typeid(Transform) || it.first == typeid(ScriptContainer))
			{
				continue;
			}

			const std::string type = it.second->getTypeStr();
			writer.writeString(type);

			if (!it.second->write(writer, scene))
			{
				LogError("Component::write() failed for type: %s!!", type.c_str());
				return false;
			}
		}

		for (auto* child : mChildren)
		{
			child->writeComponents(writer, scene);
		}

		return true;
	}
}