#include "Graphics/PBRMaterial.h"

namespace Trinity
{
	void PBRMaterial::setBaseColorFactor(const glm::vec4& baseColorFactor)
	{
		mBaseColorFactor = baseColorFactor;
	}

	void PBRMaterial::setMetallicFactor(float metallicFactor)
	{
		mMettalicFactor = metallicFactor;
	}

	void PBRMaterial::setRoughnessFactor(float roughnessFactor)
	{
		mRoughnessFactor = roughnessFactor;
	}
}