#pragma once

#include "Core/Application.h"
#include "Scene/Scene.h"

namespace Trinity
{
	class NodeScript;

	class SampleApplication : public Application
	{
	public:

		SampleApplication() = default;
		virtual ~SampleApplication() = default;

		SampleApplication(const SampleApplication&) = delete;
		SampleApplication& operator = (const SampleApplication&) = delete;

		SampleApplication(SampleApplication&&) = delete;
		SampleApplication& operator = (SampleApplication&&) = delete;

	protected:

		virtual bool init() override;
		virtual void update(float deltaTime) override;

	protected:

		std::unique_ptr<Scene> mScene{ nullptr };
		std::vector<NodeScript*> mScripts;
	};
}