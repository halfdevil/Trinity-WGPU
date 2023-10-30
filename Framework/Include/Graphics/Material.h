#pragma once

#include "Core/Resource.h"
#include "Graphics/Texture.h"
#include <string>
#include <glm/glm.hpp>
#include <optional>
#include <unordered_map>
#include <memory>

namespace Trinity
{
	class BindGroup;
	class BindGroupLayout;
	class Sampler;
	class Shader;
	class UniformBuffer;
	class FileReader;
	class FileWriter;
	class ResourceCache;

	enum class AlphaMode : uint32_t
	{
		Opaque,
		Mask,
		Blend
	};

	struct MaterialTexture
	{
		Texture* texture{ nullptr };
		Sampler* sampler{ nullptr };
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

		Shader* getShader() const
		{
			return mShader;
		}

		BindGroup* getBindGroup() const
		{
			return mBindGroup;
		}

		BindGroupLayout* getBindGroupLayout() const
		{
			return mBindGroupLayout;
		}

		UniformBuffer* getParamsBuffer() const
		{
			return mParamsBuffer;
		}

		const std::vector<std::string>& getShaderDefines() const
		{
			return mShaderDefines;
		}

		virtual bool create(const std::string& fileName, ResourceCache& cache, bool loadContent = true) override;
		virtual void destroy() override;
		virtual bool write() override;

		MaterialTexture* getTexture(const std::string& name);
		virtual std::type_index getType() const override;

		virtual void setEmissive(const glm::vec3& emissive);
		virtual void setDoubleSided(bool doubleSided);
		virtual void setAlphaCutoff(float alphaCutoff);
		virtual void setAlphaMode(AlphaMode alphaMode);
		virtual void setShader(Shader& shader);
		virtual void setShaderDefines(std::vector<std::string>&& defines);

		virtual void setTexture(const std::string& name, Texture& texture, Sampler& sampler);
		virtual bool addTexture(const std::string& name, TextureType type, const std::string& textureFileName, 
			const std::string& samplerFileName, ResourceCache& cache);

		virtual bool load(const std::string& shaderFileName, const std::vector<std::string>& defines, 
			ResourceCache& cache);

		virtual bool compile(ResourceCache& cache) = 0;

	protected:

		virtual bool read(FileReader& reader, ResourceCache& cache) override;
		virtual bool write(FileWriter& writer) override;

	protected:

		glm::vec3 mEmissive{ 0.0f, 0.0f, 0.0f };
		bool mDoubleSided{ false };
		float mAlphaCutoff{ 0.5f };
		AlphaMode mAlphaMode{ AlphaMode::Opaque };
		Shader* mShader{ nullptr };
		std::vector<std::string> mShaderDefines;
		BindGroup* mBindGroup{ nullptr };
		BindGroupLayout* mBindGroupLayout{ nullptr };
		UniformBuffer* mParamsBuffer{ nullptr };
		std::unordered_map<std::string, MaterialTexture> mTextures;
	};
}