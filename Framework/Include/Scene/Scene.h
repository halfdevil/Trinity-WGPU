#pragma once

#include "Math/Types.h"
#include <algorithm>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Trinity
{
	class Node;
	class Component;
	class ResourceCache;

	class Scene
	{
	public:

		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;
		};

		Scene() = default;
		virtual ~Scene() = default;

		Scene(const Scene&) = delete;
		Scene& operator = (const Scene&) = delete;

		Scene(Scene&&) = default;
		Scene& operator = (Scene&&) = default;

		const std::string& getName() const
		{
			return mName;
		}

		Node* getRoot() const
		{
			return mRoot;
		}

		ResourceCache& getResourceCache()
		{
			return *mResourceCache;
		}

		bool hasComponent(const std::type_index& type) const;
		Node* findNode(const std::string& name);
		const std::vector<std::unique_ptr<Component>>& getComponents(const std::type_index& type) const;

		virtual void setName(const std::string& name);
		virtual void addNode(std::unique_ptr<Node> node);
		virtual void addChild(Node& child);
		virtual void setNodes(std::vector<std::unique_ptr<Node>> nodes);
		virtual void setRoot(Node& node);
		virtual void setResourceCache(std::unique_ptr<ResourceCache> resourceCache);

		virtual void addComponent(std::unique_ptr<Component> component);
		virtual void addComponent(std::unique_ptr<Component> component, Node& node);
		virtual void setComponents(const std::type_index& type, std::vector<std::unique_ptr<Component>> components);

	public:

		template <typename T>
		void setComponents(std::vector<std::unique_ptr<T>> components)
		{
			std::vector<std::unique_ptr<Component>> result(components.size());
			std::transform(components.begin(), components.end(), result.begin(),
				[](std::unique_ptr<T>& component) -> std::unique_ptr<Component> {
					return std::unique_ptr<Component>(std::move(component));
				}
			);

			setComponents(typeid(T), std::move(result));
		}

		template <typename T>
		void clearComponents()
		{
			setComponents(typeid(T), {});
		}

		template <typename T>
		std::vector<T*> getComponents() const
		{
			std::vector<T*> result;

			if (hasComponent(typeid(T)))
			{
				auto& sceneComponents = getComponents(typeid(T));

				result.resize(sceneComponents.size());
				std::transform(sceneComponents.begin(), sceneComponents.end(), result.begin(),
					[](const std::unique_ptr<Component>& component) -> T* {
						return dynamic_cast<T*>(component.get());
					}
				);
			}

			return result;
		}

		template <typename T>
		bool hasComponent() const
		{
			return hasComponent(typeid(T));
		}

	protected:

		std::string mName;
		Node* mRoot{ nullptr };
		std::unique_ptr<ResourceCache> mResourceCache{ nullptr };
		std::vector<std::unique_ptr<Node>> mNodes;
		std::unordered_map<std::type_index, std::vector<std::unique_ptr<Component>>> mComponents;
	};
}