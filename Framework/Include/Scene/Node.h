#pragma once

#include "Scene/Components/Transform.h"
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

		Transform& getTransform()
		{
			return mTransform;
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
		virtual void setParent(Node& parent);
		virtual void addChild(Node& child);

		Component& getComponent(const std::type_index& type);
		bool hasComponent(const std::type_index& type);
		void addComponent(Component& component);

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
		Node* mParent{ nullptr };
		Transform mTransform;
		std::vector<Node*> mChildren;
		std::unordered_map<std::type_index, Component*> mComponents;
	};
}