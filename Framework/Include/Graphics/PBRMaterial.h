#pragma once

#include "Graphics/Material.h"

namespace Trinity
{
	class UniformBuffer;

	class PBRMaterial : public Material
	{
	public:

		static constexpr const char* kDefault = "/Assets/Framework/Materials/Default.tmat";
		static constexpr const char* kDefaultShader = "/Assets/Framework/Shaders/PBR.wgsl";

		PBRMaterial() = default;
		virtual ~PBRMaterial() = default;

		PBRMaterial(const PBRMaterial&) = delete;
		PBRMaterial& operator = (const PBRMaterial&) = delete;

		PBRMaterial(PBRMaterial&&) = default;
		PBRMaterial& operator = (PBRMaterial&&) = default;

		const glm::vec4& getBaseColorFactor() const
		{
			return mBaseColorFactor;
		}

		float getMetallicFactor() const
		{
			return mMettalicFactor;
		}

		float getRoughnessFactor() const
		{
			return mRoughnessFactor;
		}

		virtual bool create(const std::string& fileName, ResourceCache& cache) override;
		virtual bool write() override;

		virtual void setBaseColorFactor(const glm::vec4& baseColorFactor);
		virtual void setMetallicFactor(float metallicFactor);
		virtual void setRoughnessFactor(float roughnessFactor);
		virtual bool compile() override;

	protected:

		virtual bool read(FileReader& reader, ResourceCache& cache) override;
		virtual bool write(FileWriter& writer) override;

	protected:

		glm::vec4 mBaseColorFactor{ 0.0f, 0.0f, 0.0f, 0.0f };
		float mMettalicFactor{ 0.0f };
		float mRoughnessFactor{ 0.0f };
	};
}