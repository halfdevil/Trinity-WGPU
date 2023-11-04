#pragma once

#include "Core/ConsoleApplication.h"

namespace Trinity
{
	class TerrainTool : public ConsoleApplication
	{
	public:

		TerrainTool() = default;
		~TerrainTool() = default;

		TerrainTool(const TerrainTool&) = delete;
		TerrainTool& operator = (const TerrainTool&) = delete;

		TerrainTool(TerrainTool&&) noexcept = default;
		TerrainTool& operator = (TerrainTool&&) noexcept = default;

		void setConfigFileName(const std::string& configFileName);

	protected:

		virtual void execute() override;

	private:

		std::string mConfigFileName;
	};
}