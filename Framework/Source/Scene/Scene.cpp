#include "Scene/Scene.h"
#include "Scene/Node.h"
#include "Scene/Component.h"
#include "Graphics/ResourceCache.h"
#include <queue>

namespace Trinity
{
	Scene::~Scene()
	{
	}

	bool Scene::hasComponent(const std::type_index& type) const
	{
		auto it = mComponents.find(type);
		return (it != mComponents.end() && it->second.empty());
	}

	Node* Scene::findNode(const std::string& name)
	{
		for (auto rootNode : mRoot->getChildren())
		{
			std::queue<Node*> traverseNodes;
			traverseNodes.push(rootNode);

			while (!traverseNodes.empty())
			{
				auto node = traverseNodes.front();
				traverseNodes.pop();

				if (node->getName() == name)
				{
					return node;
				}

				for (auto childNode : node->getChildren())
				{
					traverseNodes.push(childNode);
				}
			}
		}

		return nullptr;
	}

	const std::vector<std::unique_ptr<Component>>& Scene::getComponents(const std::type_index& type) const
	{
		return mComponents.at(type);
	}

	void Scene::setName(const std::string& name)
	{
		mName = name;
	}

	void Scene::addNode(std::unique_ptr<Node> node)
	{
		mNodes.emplace_back(std::move(node));
	}

	void Scene::addChild(Node& child)
	{
		if (mRoot != nullptr)
		{
			mRoot->addChild(child);
		}
	}

	void Scene::setNodes(std::vector<std::unique_ptr<Node>> nodes)
	{
		mNodes = std::move(nodes);
	}

	void Scene::setRoot(Node& node)
	{
		mRoot = &node;
	}

	void Scene::setResourceCache(std::unique_ptr<ResourceCache> resourceCache)
	{
		mResourceCache = std::move(resourceCache);
	}

	void Scene::addComponent(std::unique_ptr<Component> component)
	{
		mComponents[component->getType()].push_back(std::move(component));
	}

	void Scene::addComponent(std::unique_ptr<Component> component, Node& node)
	{
		node.addComponent(*component);
		addComponent(std::move(component));
	}

	void Scene::setComponents(const std::type_index& type, std::vector<std::unique_ptr<Component>> components)
	{
		mComponents[type] = std::move(components);
	}
}