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
		virtual size_t getHashCode() const override;

		virtual void init();
		virtual void update(float deltaTime);
		virtual void resize(uint32_t width, uint32_t height);

		virtual Script& getScript(size_t hashCode);
		virtual bool hasScript(size_t hashCode);
		virtual void setScript(Script& script);

		virtual void setNode(Node& node);
		virtual bool read(FileReader& reader, Scene& scene) override;
		virtual bool write(FileWriter& writer, Scene& scene) override;

	public:

		template <typename T>
		inline T& getScript()
		{
			return dynamic_cast<T&>(getScript(typeid(T).hash_code()));
		}

		template <typename T>
		inline bool hasComponent()
		{
			return hasScript(typeid(T).hash_code());
		}

	protected:

		Node* mNode{ nullptr };
		std::unordered_map<size_t, Script*> mScripts;
	};
}