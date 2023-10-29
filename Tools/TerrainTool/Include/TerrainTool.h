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

		void setSize(uint32_t size);
		void setPatchSize(uint32_t patchSize);
		void setHeightScale(float heightScale);
		void setCellSpacing(float cellSpacing);
		void setOutputFileName(const std::string& fileName);
		void setHeightMapFileName(const std::string& fileName);
		void setBlendMapFileName(const std::string& fileName);
		void setLayerFileNames(std::vector<std::string>&& fileNames);

	protected:

		virtual void execute() override;

	private:

		uint32_t mSize{ 0 };
		uint32_t mPatchSize{ 0 };
		float mHeightScale{ 0.0f };
		float mCellSpacing{ 0.0f };
		std::string mOutputFileName;
		std::string mHeightMapFileName;
		std::string mBlendMapFileName;
		std::vector<std::string> mLayerFileNames;
	};
}