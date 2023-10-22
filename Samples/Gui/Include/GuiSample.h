#pragma once

#include "SampleApplication.h"

namespace Trinity
{
	class GuiRenderer;

	class GuiSample : public SampleApplication
	{
	public:

		GuiSample() = default;
		virtual ~GuiSample() = default;

		GuiSample(const GuiSample&) = delete;
		GuiSample& operator = (const GuiSample&) = delete;

		GuiSample(GuiSample&&) = default;
		GuiSample& operator = (GuiSample&&) = default;

	protected:

		virtual bool init() override;
		virtual void render(float deltaTime) override;
		virtual void onGui() override;
		virtual void setupInput() override;

	protected:

		std::unique_ptr<GuiRenderer> mGui{ nullptr };
	};
}