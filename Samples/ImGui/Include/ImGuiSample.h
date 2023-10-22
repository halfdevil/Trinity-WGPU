#pragma once

#include "SampleApplication.h"

namespace Trinity
{
	class ImGuiSample : public SampleApplication
	{
	public:

		ImGuiSample() = default;
		virtual ~ImGuiSample() = default;

		ImGuiSample(const ImGuiSample&) = delete;
		ImGuiSample& operator = (const ImGuiSample&) = delete;

		ImGuiSample(ImGuiSample&&) = default;
		ImGuiSample& operator = (ImGuiSample&&) = default;

	protected:

		virtual bool init() override;
		virtual void render(float deltaTime) override;
		virtual void onGui() override;
		virtual void setupInput() override;
	};
}