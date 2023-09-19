#pragma once

#include "Graphics/Material.h"

namespace Trinity
{
	class PBRMaterial : public Material
	{
	public:

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

		virtual void setBaseColorFactor(const glm::vec4& baseColorFactor);
		virtual void setMetallicFactor(float metallicFactor);
		virtual void setRoughnessFactor(float roughnessFactor);

	protected:

		glm::vec4 mBaseColorFactor{ 0.0f, 0.0f, 0.0f, 0.0f };
		float mMettalicFactor{ 0.0f };
		float mRoughnessFactor{ 0.0f };
	};
}