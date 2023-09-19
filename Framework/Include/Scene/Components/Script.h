#pragma once

#include "Scene/Component.h"

namespace Trinity
{
	class Node;

	class Script : public Component
	{
	public:

		Script() = default;
		virtual ~Script() = default;

		Script(const Script&) = delete;
		Script& operator = (const Script&) = delete;

		Script(Script&&) = delete;
		Script& operator = (Script&&) = delete;

		virtual std::type_index getType() const override;

		virtual void init();
		virtual void update(float deltaTime);
		virtual void resize(uint32_t width, uint32_t height);
	};

	class NodeScript : public Script
	{
	public:

		NodeScript(Node& node);
		virtual ~NodeScript() = default;

		Node& getNode();

	protected:

		Node& mNode;
	};
}