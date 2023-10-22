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

	std::string ScriptContainer::getTypeStr() const
	{
		return getStaticType();
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

	Script& ScriptContainer::getScript(const std::string& type)
	{
		return *mScripts.at(type);
	}

	bool ScriptContainer::hasScript(const std::string& type)
	{
		return mScripts.contains(type);
	}

	void ScriptContainer::setScript(Script& script)
	{
		auto it = mScripts.find(script.getTypeStr());
		if (it != mScripts.end())
		{
			it->second = &script;
		}
		else
		{
			mScripts.insert(std::make_pair(script.getTypeStr(), &script));
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

			setScript(dynamic_cast<Script&>(*component));
			scene.addComponent(std::move(component));
		}

		return true;
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
			const std::string type = it.second->getTypeStr();
			writer.writeString(type);

			if (!it.second->write(writer, scene))
			{
				LogError("Component::write() failed for type: %s!!", type.c_str());
				return false;
			}
		}

		return true;
	}

	std::string ScriptContainer::getStaticType()
	{
		return "ScriptContainer";
	}
}