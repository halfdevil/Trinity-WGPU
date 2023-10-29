#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Trinity
{
	class Terrain;
	class ResourceCache;

	class TerrainImporter
	{
	public:

		TerrainImporter() = default;
		~TerrainImporter() = default;

		TerrainImporter(const TerrainImporter&) = delete;
		TerrainImporter& operator = (const TerrainImporter&) = delete;

		TerrainImporter(TerrainImporter&&) noexcept = default;
		TerrainImporter& operator = (TerrainImporter&&) noexcept = default;

		Terrain* importTerrain(
			const std::string& outputFileName,
			uint32_t size,
			uint32_t patchSize,
			float heightScale,
			float cellSpacing,
			const std::string& heightMapFileName,
			const std::string& blendMapFileName,
			const std::vector<std::string>& layerFileNames,
			ResourceCache& cache,
			bool loadContent = true
		);
	};
}