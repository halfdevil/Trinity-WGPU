#pragma once

#include "Graphics/Material.h"

namespace Trinity
{
	class SkyboxMaterial : public Material
	{
	public:

		static constexpr const char* kDefaultShader = "/Assets/Framework/Shaders/Skybox.wgsl";
		static constexpr uint32_t kMaxLayers = 5;

		SkyboxMaterial() = default;
		virtual ~SkyboxMaterial() = default;

		SkyboxMaterial(const SkyboxMaterial&) = delete;
		SkyboxMaterial& operator = (const SkyboxMaterial&) = delete;

		SkyboxMaterial(SkyboxMaterial&&) = default;
		SkyboxMaterial& operator = (SkyboxMaterial&&) = default;

		const glm::vec4& getBaseColorFactor() const
		{
			return mBaseColorFactor;
		}

		virtual bool create(const std::string& fileName, ResourceCache& cache, bool loadContent = true) override;
		virtual void destroy() override;
		virtual bool write() override;

		virtual void setBaseColorFactor(const glm::vec4& baseColorFactor);
		virtual bool compile(ResourceCache& cache) override;

	protected:

		virtual bool read(FileReader& reader, ResourceCache& cache) override;
		virtual bool write(FileWriter& writer) override;

	protected:

		glm::vec4 mBaseColorFactor{ 0.0f };
	};
}