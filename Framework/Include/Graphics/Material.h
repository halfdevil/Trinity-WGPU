#pragma once

#include "Graphics/Resource.h"
#include <string>
#include <glm/glm.hpp>
#include <optional>
#include <unordered_map>
#include <memory>

namespace Trinity
{
	class BindGroup;
	class Texture;
	class Sampler;

	enum class AlphaMode : uint32_t
	{
		Opaque,
		Mask,
		Blend
	};

	class Material : public Resource
	{
	public:

		Material() = default;
		virtual ~Material() = default;

		Material(const Material&) = delete;
		Material& operator = (const Material&) = delete;

		Material(Material&&) = default;
		Material& operator = (Material&&) = default;

		const glm::vec3& getEmissive() const
		{
			return mEmissive;
		}

		bool IsDoubleSided() const
		{
			return mDoubleSided;
		}

		float getAlphaCutoff() const
		{
			return mAlphaCutoff;
		}

		AlphaMode getAlphaMode() const
		{
			return mAlphaMode;
		}

		Texture* getTexture(const std::string& name);
		Sampler* getSampler(const std::string& name);

		virtual std::type_index getType() const override;

		virtual void setEmissive(const glm::vec3& emissive);
		virtual void setDoubleSided(bool doubleSided);
		virtual void setAlphaCutoff(float alphaCutoff);
		virtual void setAlphaMode(AlphaMode alphaMode);
		virtual void setTexture(const std::string& name, Texture& texture);
		virtual void setSampler(const std::string& name, Sampler& sampler);

	protected:

		glm::vec3 mEmissive{ 0.0f, 0.0f, 0.0f };
		bool mDoubleSided{ false };
		float mAlphaCutoff{ 0.5f };
		AlphaMode mAlphaMode{ AlphaMode::Opaque };
		std::unordered_map<std::string, Texture*> mTextures;
		std::unordered_map<std::string, Sampler*> mSamplers;
	};
}