#include "Scene/Components/ScriptContainer.h"
#include "Scene/Components/Script.h"
#include "Scene/ComponentFactory.h"
#include "Scene/Node.h"
#include "Scene/Scene.h"
#include "VFS/FileReader.h"
#include "VFS/FileWriter.h"
#include "Core/Logger.h"

namespace Trinity
{
	std::type_index ScriptContainer::getType() const
	{
		return typeid(ScriptContainer);
	}

	size_t ScriptContainer::getHashCode() const
	{
		return typeid(ScriptContainer).hash_code();
	}

	void ScriptContainer::init()
	{
		for (auto& it : mScripts)
		{
			it.second->init();
		}
	}

	void ScriptContainer::update(float deltaTime)
	{
		for (auto& it : mScripts)
		{
			it.second->update(deltaTime);
		}
	}

	void ScriptContainer::resize(uint32_t width, uint32_t height)
	{
		for (auto& it : mScripts)
		{
			it.second->resize(width, height);
		}
	}

	Script& ScriptContainer::getScript(size_t hashCode)
	{
		return *mScripts.at(hashCode);
	}

	bool ScriptContainer::hasScript(size_t hashCode)
	{
		return mScripts.contains(hashCode);
	}

	void ScriptContainer::setScript(Script& script)
	{
		auto it = mScripts.find(script.getHashCode());
		if (it != mScripts.end())
		{
			it->second = &script;
		}
		else
		{
			mScripts.insert(std::make_pair(script.getHashCode(), &script));
		}
	}

	void ScriptContainer::setNode(Node& node)
	{
		mNode = &node;

		for (auto& it : mScripts)
		{
			it.second->setNode(node);
		}
	}

	bool ScriptContainer::read(FileReader& reader, Scene& scene)
	{
		if (!Component::read(reader, scene))
		{
			return false;
		}

		uint32_t numScripts{ 0 };
		reader.read(&numScripts);

		for (uint32_t idx = 0; idx < numScripts; idx++)
		{
			size_t type;
			reader.read(&type);

			auto component = scene.getComponentFactory().createComponent(type);
			if (!component)
			{
				LogError("ComponentFactory::createComponent() failed for type: %ld!!", type);
				return false;
			}

			if (!component->read(reader, scene))
			{
				LogError("Component::read() failed for type: %ld!!", type);
				return false;
			}

			setScript(dynamic_cast<Script&>(*component));
			scene.addComponent(std::move(component));
		}

		return false;
	}

	bool ScriptContainer::write(FileWriter& writer, Scene& scene)
	{
		if (!Component::write(writer, scene))
		{
			return false;
		}

		const uint32_t numScripts = (uint32_t)mScripts.size();
		writer.write(&numScripts);

		for (auto& it : mScripts)
		{
			const size_t type = it.second->getHashCode();
			writer.write(&type);

			if (!it.second->write(writer, scene))
			{
				LogError("Component::write() failed for type: %ld!!", type);
				return false;
			}
		}

		return true;
	}
}