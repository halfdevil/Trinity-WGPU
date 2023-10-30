#pragma once

#include "Core/ConsoleApplication.h"

namespace Trinity
{
	class SkyboxTool : public ConsoleApplication
	{
	public:

		SkyboxTool() = default;
		~SkyboxTool() = default;

		SkyboxTool(const SkyboxTool&) = delete;
		SkyboxTool& operator = (const SkyboxTool&) = delete;

		SkyboxTool(SkyboxTool&&) noexcept = default;
		SkyboxTool& operator = (SkyboxTool&&) noexcept = default;

		void setSize(float size);
		void setOutputFileName(const std::string& fileName);
		void setEnvMapFileNames(std::vector<std::string>&& fileNames);

	protected:

		virtual void execute() override;

	private:

		float mSize{ 0 };
		std::string mOutputFileName;
		std::vector<std::string> mEnvMapFileNames;
	};
}