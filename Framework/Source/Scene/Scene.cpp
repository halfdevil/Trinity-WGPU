#include "Scene/Scene.h"
#include "Scene/Node.h"
#include "Scene/Component.h"
#include "Scene/Components/Light.h"
#include "Scene/Components/Mesh.h"
#include "Scene/Components/ScriptContainer.h"
#include "Scene/Components/PerspectiveCamera.h"
#include "Scene/Components/Scripts/FreeCamera.h"
#include "Scene/Components/Scripts/Animator.h"
#include "Scene/ComponentFactory.h"
#include "VFS/FileSystem.h"
#include "Core/ResourceCache.h"
#include "Core/Logger.h"
#include <queue>

namespace Trinity
{
	bool Scene::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		mComponentFactory = std::make_unique<ComponentFactory>();
		registerDefaultComponents();

		return Resource::create(fileName, cache, loadContent);
	}

	void Scene::destroy()
	{
		Resource::destroy();

		mNodes.clear();
		mComponents.clear();
	}

	bool Scene::write()
	{
		return Resource::write();
	}

	std::type_index Scene::getType() const
	{
		return typeid(Scene);
	}

	void Scene::registerDefaultComponents()
	{
		mComponentFactory->registerCreator<Light>();
		mComponentFactory->registerCreator<Mesh>();
		mComponentFactory->registerCreator<PerspectiveCamera>();
		mComponentFactory->registerCreator<FreeCamera>();
		mComponentFactory->registerCreator<Animator>();
	}

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

	Node* Scene::getNode(uint32_t idx) const
	{
		if (idx < (uint32_t)mNodes.size())
		{
			return mNodes[idx].get();
		}

		return nullptr;
	}

	const std::vector<std::unique_ptr<Component>>& Scene::getComponents(const std::type_index& type) const
	{
		return mComponents.at(type);
	}

	void Scene::addNode(std::unique_ptr<Node> node)
	{
		node->setId((uint32_t)mNodes.size());
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
		uint32_t id{ 0 };
		for (auto& node : nodes)
		{
			node->setId(id++);
		}

		mNodes = std::move(nodes);
	}

	void Scene::setRoot(Node& node)
	{
		mRoot = &node;
	}

	void Scene::addComponent(std::unique_ptr<Component> component)
	{
		mComponents[component->getType()].push_back(std::move(component));
	}

	void Scene::addComponent(std::unique_ptr<Component> component, Node& node)
	{
		node.setComponent(*component);
		addComponent(std::move(component));
	}

	void Scene::setComponents(const std::type_index& type, std::vector<std::unique_ptr<Component>> components)
	{
		mComponents[type] = std::move(components);
	}

	Mesh* Scene::addMesh(const std::string& nodeName, const std::string& modelFileName, ResourceCache& cache, const glm::vec3& position,
		const glm::quat& rotation, const glm::vec3& scale, Node* parent)
	{
		auto meshPtr = std::make_unique<Mesh>();
		auto meshNode = std::make_unique<Node>();

		if (parent != nullptr)
		{
			meshNode->setParent(*parent);
		}

		if (!meshPtr->load(modelFileName, cache, *this))
		{
			LogError("Mesh::load() failed for: %s!!", modelFileName.c_str());
			return nullptr;
		}

		meshPtr->setName("mesh");
		meshPtr->setNode(*meshNode);
		meshNode->setName(nodeName);

		auto& transform = meshNode->getTransform();
		transform.setTranslation(position);
		transform.setRotation(rotation);
		transform.setScale(scale);

		auto* mesh = meshPtr.get();
		meshNode->setComponent(*mesh);

		addChild(*meshNode);
		addComponent(std::move(meshPtr), *meshNode);
		addNode(std::move(meshNode));

		return mesh;
	}

	Light* Scene::addLight(LightType type, const glm::vec3& position, const glm::quat& rotation,
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

		auto* light = lightPtr.get();
		lightNode->setComponent(*light);

		addChild(*lightNode);
		addComponent(std::move(lightPtr), *lightNode);
		addNode(std::move(lightNode));

		return light;
	}

	Light* Scene::addPointLight(const glm::vec3& position, const LightProperties& properties,
		Node* parent)
	{
		return addLight(LightType::Point, position, {}, properties, parent);
	}

	Light* Scene::addDirectionalLight(const glm::quat& rotation, const LightProperties& properties,
		Node* parent)
	{
		return addLight(LightType::Directional, {}, rotation, properties, parent);
	}

	Light* Scene::addSpotLight(const glm::vec3& position, const glm::quat& rotation,
		const LightProperties& properties, Node* parent)
	{
		return addLight(LightType::Spot, position, rotation, properties, parent);
	}

	PerspectiveCamera* Scene::addPerspectiveCamera(const std::string& nodeName, float aspectRatio, float fov, float nearPlane, 
		float farPlane, const glm::vec3& position,	const glm::quat& rotation, Node* parent)
	{
		auto cameraPtr = std::make_unique<PerspectiveCamera>();
		auto cameraNode = std::make_unique<Node>();

		if (parent != nullptr)
		{
			cameraNode->setParent(*parent);
		}

		cameraPtr->setName("camera");
		cameraNode->setName(nodeName);

		cameraPtr->setNode(*cameraNode);
		cameraPtr->setAspectRatio(aspectRatio);
		cameraPtr->setFOV(fov);
		cameraPtr->setNearPlane(nearPlane);
		cameraPtr->setFarPlane(farPlane);
		cameraPtr->updateProjection();

		auto& transform = cameraNode->getTransform();
		transform.setTranslation(position);
		transform.setRotation(rotation);

		auto* camera = cameraPtr.get();
		cameraNode->setComponent(*camera);

		addChild(*cameraNode);
		addComponent(std::move(cameraPtr), *cameraNode);
		addNode(std::move(cameraNode));

		return camera;
	}

	FreeCamera* Scene::addFreeCameraScript(const std::string& nodeName, const glm::uvec2& extent)
	{
		auto cameraNode = findNode(nodeName);
		if (!cameraNode)
		{
			LogWarning("Camera node '%s' not found. Looking for 'default_camera' node", nodeName.c_str());
			cameraNode = findNode("default_camera");
		}

		if (!cameraNode)
		{
			LogError("Default camera node 'default_camera' not found.");
			return nullptr;
		}

		auto freeCameraPtr = std::make_unique<FreeCamera>();
		freeCameraPtr->setNode(*cameraNode);
		freeCameraPtr->resize(extent.x, extent.y);

		auto* freeCamera = freeCameraPtr.get();	
		addComponent(std::move(freeCameraPtr));

		auto& scritpContainer = cameraNode->getScriptContainer();
		scritpContainer.setScript(*freeCamera);

		return freeCamera;
	}

	Animator* Scene::addAnimatorScript(const std::string& nodeName)
	{
		auto meshNode = findNode(nodeName);
		if (!meshNode)
		{
			LogError("Mesh node '%s' not foud. Adding new node", nodeName.c_str());
			return nullptr;
		}

		auto animatorPtr = std::make_unique<Animator>();
		animatorPtr->setNode(*meshNode);

		if (meshNode->hasComponent<Mesh>())
		{
			auto& mesh = meshNode->getComponent<Mesh>();
			if (mesh.isAnimated())
			{
				animatorPtr->setMesh(mesh);
			}
			else
			{
				LogError("Mesh node '%s' don't have a animated mesh component attached", nodeName.c_str());
				return nullptr;
			}
		}
		else
		{
			LogError("Mesh node '%s' don't have a mesh component attached", nodeName.c_str());
			return nullptr;
		}

		auto* animator = animatorPtr.get();
		addComponent(std::move(animatorPtr));

		auto& scritpContainer = meshNode->getScriptContainer();
		scritpContainer.setScript(*animator);

		return animator;
	}

	bool Scene::read(FileReader& reader, ResourceCache& cache)
	{
		mNodes.clear();
		mComponents.clear();

		auto root = std::make_unique<Node>();
		mRoot = root.get();

		addNode(std::move(root));

		if (!mRoot->read(reader, cache, *this))
		{
			LogError("Node::read() failed for root!!");
			return false;
		}

		if (!mRoot->readComponents(reader, cache, *this))
		{
			LogError("Node::readComponents() failed for root!!");
			return false;
		}

		return true;
	}

	bool Scene::write(FileWriter& writer)
	{
		if (!mRoot->write(writer, *this))
		{
			LogError("Node::write() failed for root!!");
			return false;
		}

		if (!mRoot->writeComponents(writer, *this))
		{
			LogError("Node::write() failed for root!!");
			return false;
		}

		return true;
	}
}