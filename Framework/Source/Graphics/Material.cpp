#include "Graphics/Material.h"
#include "Graphics/Texture.h"
#include "Graphics/BindGroup.h"
#include "Graphics/BindGroupLayout.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/Shader.h"

namespace Trinity
{
	MaterialTexture* Material::getTexture(const std::string& name)
	{
		return &mTextures.at(name);
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

	void Material::setShader(Shader& shader)
	{
		mShader = &shader;
	}

	void Material::setTexture(const std::string& name, Texture& texture, Sampler& sampler)
	{
		auto it = mTextures.find(name);
		if (it != mTextures.end())
		{
			it->second = {
				.texture = &texture,
				.sampler = &sampler
			};
		}
		else
		{
			mTextures.insert(std::make_pair(name, MaterialTexture{
				.texture = &texture,
				.sampler = &sampler
			}));
		}
	}
}