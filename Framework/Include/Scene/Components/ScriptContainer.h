#pragma once

#include "Scene/Component.h"
#include <unordered_map>

namespace Trinity
{
	class Node;
	class Script;

	class ScriptContainer : public Component
	{
	public:

		ScriptContainer() = default;
		virtual ~ScriptContainer() = default;

		ScriptContainer(const ScriptContainer&) = delete;
		ScriptContainer& operator = (const ScriptContainer&) = delete;

		ScriptContainer(ScriptContainer&&) = default;
		ScriptContainer& operator = (ScriptContainer&&) = default;

		Node* getNode() const
		{
			return mNode;
		}

		virtual std::type_index getType() const override;
		virtual std::string getTypeStr() const override;

		virtual void init();
		virtual void update(float deltaTime);
		virtual void resize(uint32_t width, uint32_t height);

		virtual Script& getScript(const std::string& type);
		virtual bool hasScript(const std::string& type);
		virtual void setScript(Script& script);

		virtual void setNode(Node& node);
		virtual bool read(FileReader& reader, Scene& scene) override;
		virtual bool write(FileWriter& writer, Scene& scene) override;

	public:

		static std::string getStaticType();

		template <typename T>
		inline T& getScript()
		{
			return dynamic_cast<T&>(getScript(T::getStaticType()));
		}

		template <typename T>
		inline bool hasComponent()
		{
			return hasScript(T::getStaticType());
		}

	protected:

		Node* mNode{ nullptr };
		std::unordered_map<std::string, Script*> mScripts;
	};
}