#include "Scene/Scene.h"
#include "Scene/Node.h"
#include "Scene/Component.h"
#include "Scene/Components/Light.h"
#include "Scene/Components/Scripts/FreeCamera.h"
#include "Graphics/ResourceCache.h"
#include "Core/Logger.h"
#include <queue>

namespace Trinity
{
	bool Scene::hasComponent(const std::type_index& type) const
	{
		auto it = mComponents.find(type);
		return (it != mComponents.end() && !it->second.empty());
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

	Light& Scene::addLight(LightType type, const glm::vec3& position, const glm::quat& rotation, 
		const LightProperties& properties, Node* parent)
	{
		auto lightPtr = std::make_unique<Light>();
		auto lightNode = std::make_unique<Node>();

		if (parent != nullptr)
		{
			lightNode->setParent(*parent);
		}

		lightPtr->setName("light");
		lightNode->setName("lightNode");

		lightPtr->setNode(*lightNode);
		lightPtr->setLightType(type);
		lightPtr->setLightProperties(properties);

		auto& transform = lightNode->getTransform();
		transform.setTranslation(position);
		transform.setRotation(rotation);

		auto& light = *lightPtr;
		lightNode->addComponent(*lightPtr);

		addChild(*lightNode);
		addComponent(std::move(lightPtr), *lightNode);
		addNode(std::move(lightNode));

		return light;
	}

	Light& Scene::addPointLight(const glm::vec3& position, const LightProperties& properties, 
		Node* parent)
	{
		return addLight(LightType::Point, position, {}, properties, parent);
	}

	Light& Scene::addDirectionalLight(const glm::quat& rotation, const LightProperties& properties, 
		Node* parent)
	{
		return addLight(LightType::Directional, {}, rotation, properties, parent);
	}

	Light& Scene::addSpotLight(const glm::vec3& position, const glm::quat& rotation, 
		const LightProperties& properties, Node* parent)
	{
		return addLight(LightType::Spot, position, rotation, properties, parent);
	}

	FreeCamera& Scene::addFreeCamera(const std::string& nodeName, const glm::uvec2& extent)
	{
		auto cameraNode = findNode(nodeName);
		if (!cameraNode)
		{
			LogWarning("Camera node '%s' not found. Looking for 'default_camera' node", nodeName.c_str());
			cameraNode = findNode("default_camera");
		}

		auto freeCameraPtr = std::make_unique<FreeCamera>(*cameraNode);
		freeCameraPtr->resize(extent.x, extent.y);

		auto& freeCamera = *freeCameraPtr;
		addComponent(std::move(freeCameraPtr), *cameraNode);

		return freeCamera;
	}
}