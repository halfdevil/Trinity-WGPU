#pragma once

#include "Graphics/Material.h"

namespace Trinity
{
	class TerrainMaterial : public Material
	{
	public:

		static constexpr const char* kDefaultShader = "/Assets/Framework/Shaders/Terrain.wgsl";
		static constexpr uint32_t kMaxLayers = 5;

		TerrainMaterial() = default;
		virtual ~TerrainMaterial() = default;

		TerrainMaterial(const TerrainMaterial&) = delete;
		TerrainMaterial& operator = (const TerrainMaterial&) = delete;

		TerrainMaterial(TerrainMaterial&&) = default;
		TerrainMaterial& operator = (TerrainMaterial&&) = default;

		const glm::vec4& getBaseColorFactor() const
		{
			return mBaseColorFactor;
		}

		const glm::vec4& getLayerScale() const
		{
			return mLayerScale;
		}

		virtual bool create(const std::string& fileName, ResourceCache& cache, bool loadContent = true) override;
		virtual void destroy() override;
		virtual bool write() override;

		virtual void setBaseColorFactor(const glm::vec4& baseColorFactor);
		virtual void setLayerScale(const glm::vec4& layerScale);
		virtual bool compile(ResourceCache& cache) override;

	protected:

		virtual bool read(FileReader& reader, ResourceCache& cache) override;
		virtual bool write(FileWriter& writer) override;

	protected:

		glm::vec4 mBaseColorFactor{ 0.0f };
		glm::vec4 mLayerScale{ 1.0f };
	};
}