#pragma once

#include "Scene/Components/Transform.h"
#include "Scene/Components/ScriptContainer.h"
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Trinity
{
	class Node
	{
	public:

		Node();
		virtual ~Node() = default;

		Node(const Node&) = delete;
		Node& operator = (const Node&) = delete;

		Node(Node&&) = default;
		Node& operator = (Node&&) = default;

		const std::string& getName() const
		{
			return mName;
		}

		uint32_t getId() const
		{
			return mId;
		}

		Transform& getTransform()
		{
			return mTransform;
		}

		ScriptContainer& getScriptContainer()
		{
			return mScriptContainer;
		}

		Node* getParent() const
		{
			return mParent;
		}

		const std::vector<Node*>& getChildren() const
		{
			return mChildren;
		}

		virtual void setName(const std::string& name);
		virtual void setId(uint32_t id);
		virtual void setParent(Node& parent);
		virtual void addChild(Node& child);

		virtual Component& getComponent(const std::type_index& type);
		virtual bool hasComponent(const std::type_index& type);
		virtual void setComponent(Component& component);

		virtual bool read(FileReader& reader, ResourceCache& cache, Scene& scene);
		virtual bool write(FileWriter& writer, Scene& scene);

		virtual bool readComponents(FileReader& reader, ResourceCache& cache, Scene& scene);
		virtual bool writeComponents(FileWriter& writer, Scene& scene);

	public:

		template <typename T>
		inline T& getComponent()
		{
			return dynamic_cast<T&>(getComponent(typeid(T)));
		}

		template <typename T>
		inline bool hasComponent()
		{
			return hasComponent(typeid(T));
		}

	protected:

		std::string mName;
		uint32_t mId{ (uint32_t)-1 };
		Node* mParent{ nullptr };
		Transform mTransform;
		ScriptContainer mScriptContainer;
		std::vector<Node*> mChildren;
		std::unordered_map<std::type_index, Component*> mComponents;
	};
}