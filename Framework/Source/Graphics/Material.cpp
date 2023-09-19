#include "Graphics/Material.h"
#include "Graphics/Texture.h"
#include "Graphics/BindGroup.h"

namespace Trinity
{
	Texture* Material::getTexture(const std::string& name)
	{
		return mTextures.at(name);
	}

	Sampler* Material::getSampler(const std::string& name)
	{
		return mSamplers.at(name);
	}

	std::type_index Material::getType() const
	{
		return typeid(Material);
	}

	void Material::setEmissive(const glm::vec3& emissive)
	{
		mEmissive = emissive;
	}

	void Material::setDoubleSided(bool doubleSided)
	{
		mDoubleSided = doubleSided;
	}

	void Material::setAlphaCutoff(float alphaCutoff)
	{
		mAlphaCutoff = alphaCutoff;
	}

	void Material::setAlphaMode(AlphaMode alphaMode)
	{
		mAlphaMode = alphaMode;
	}

	void Material::setTexture(const std::string& name, Texture& texture)
	{
		auto it = mTextures.find(name);
		if (it != mTextures.end())
		{
			it->second = &texture;
		}
		else
		{
			mTextures.insert(std::make_pair(name, &texture));
		}
	}

	void Material::setSampler(const std::string& name, Sampler& sampler)
	{
		auto it = mSamplers.find(name);
		if (it != mSamplers.end())
		{
			it->second = &sampler;
		}
		else
		{
			mSamplers.insert(std::make_pair(name, &sampler));
		}
	}
}