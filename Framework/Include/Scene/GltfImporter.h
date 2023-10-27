#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Trinity
{
	class Scene;
	class Model;
	class Skeleton;
	class AnimationClip;
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

		Scene* importScene(const std::string& inputFileName, const std::string& outputFileName, 
			ResourceCache& cache, bool loadContent = true);

		Model* importModel(const std::string& inputFileName, const std::string& outputFileName, 
			ResourceCache& cache, bool animated = false, bool loadContent = true);

		Skeleton* importSkeleton(const std::string& inputFileName, const std::string& outputFileName,
			ResourceCache& cache, bool loadContent = true);

		AnimationClip* importAnimation(const std::string& inputFileName, const std::string& outputFileName,
			ResourceCache& cache, bool loadContent = true);
	};
}