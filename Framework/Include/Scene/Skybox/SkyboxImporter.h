#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Trinity
{
	class Skybox;
	class ResourceCache;

	class SkyboxImporter
	{
	public:

		SkyboxImporter() = default;
		~SkyboxImporter() = default;

		SkyboxImporter(const SkyboxImporter&) = delete;
		SkyboxImporter& operator = (const SkyboxImporter&) = delete;

		SkyboxImporter(SkyboxImporter&&) noexcept = default;
		SkyboxImporter& operator = (SkyboxImporter&&) noexcept = default;

		Skybox* importSkybox(
			const std::string& outputFileName,
			float size,
			const std::vector<std::string>& envMapFileNames,
			ResourceCache& cache,
			bool loadContent = true
		);
	};
}