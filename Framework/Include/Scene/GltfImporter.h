#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Trinity
{
	class Scene;
	class Model;
	class ResourceCache;

	template <typename T, typename Y>
	struct TypeCast
	{
		Y operator()(T value) const noexcept
		{
			return static_cast<Y>(value);
		}
	};

	class GltfImporter
	{
	public:

		GltfImporter() = default;
		~GltfImporter() = default;

		GltfImporter(const GltfImporter&) = delete;
		GltfImporter& operator = (const GltfImporter&) = delete;

		GltfImporter(GltfImporter&&) noexcept = default;
		GltfImporter& operator = (GltfImporter&&) noexcept = default;

		std::unique_ptr<Scene> importScene(const std::string& inputFileName, const std::string& outputFileName, 
			bool loadContent = true);

		std::unique_ptr<Model> importModel(const std::string& inputFileName, const std::string& outputFileName, 
			ResourceCache& cache, uint32_t meshIndex = 0, bool loadContent = true);
	};
}