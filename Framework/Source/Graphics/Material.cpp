#include "Graphics/Material.h"
#include "Graphics/Texture2D.h"
#include "Graphics/TextureCube.h"
#include "Graphics/BindGroup.h"
#include "Graphics/BindGroupLayout.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/Shader.h"
#include "Graphics/Sampler.h"
#include "VFS/FileSystem.h"
#include "Core/ResourceCache.h"
#include "Core/Logger.h"

namespace Trinity
{
	bool Material::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		return Resource::create(fileName, cache, loadContent);
	}

	void Material::destroy()
	{
		Resource::destroy();

		mShader = nullptr;
		mBindGroup = nullptr;
		mBindGroupLayout = nullptr;
		mParamsBuffer = nullptr;
		mTextures.clear();
		mShaderDefines.clear();
	}

	bool Material::write()
	{
		return Resource::write();
	}

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

	void Material::addShaderDefine(const std::string& define)
	{
		mShaderDefines.push_back(define);
	}

	void Material::setShaderDefines(std::vector<std::string>&& defines)
	{
		mShaderDefines = std::move(defines);
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

	bool Material::addTexture(const std::string& name, TextureType type, const std::string& textureFileName,
		const std::string& samplerFileName, ResourceCache& cache)
	{
		if (!cache.isLoaded<Texture>(textureFileName))
		{
			std::unique_ptr<Texture> texture{ nullptr };
			if (type == TextureType::TwoD)
			{
				texture = std::make_unique<Texture2D>();
			}
			else
			{
				texture = std::make_unique<TextureCube>();
			}

			if (!texture->create(textureFileName, cache))
			{
				LogError("Texture2D::create() failed for: %s!!", textureFileName.c_str());
				return false;
			}

			cache.addResource(std::move(texture));
		}

		if (!cache.isLoaded<Sampler>(samplerFileName))
		{
			auto sampler = std::make_unique<Sampler>();
			if (!sampler->create(samplerFileName, cache))
			{
				LogError("Sampler::create() failed for: %s!!", samplerFileName.c_str());
				return false;
			}

			cache.addResource(std::move(sampler));
		}

		auto* texture = cache.getResource<Texture>(textureFileName);
		auto* sampler = cache.getResource<Sampler>(samplerFileName);

		mTextures.insert(std::make_pair(name, MaterialTexture{
			.texture = texture,
			.sampler = sampler
		}));

		return true;
	}

	bool Material::load(const std::string& shaderFileName, const std::vector<std::string>& defines, ResourceCache& cache)
	{
		ShaderPreProcessor processor;
		processor.addDefines(defines);

		auto shader = std::make_unique<Shader>();
		if (!shader->load(shaderFileName, processor))
		{
			LogError("Shader::create() failed for: %s!!", shaderFileName.c_str());
			return false;
		}

		mShader = shader.get();
		cache.addResource(std::move(shader));

		return true;
	}

	bool Material::read(FileReader& reader, ResourceCache& cache)
	{
		if (!Resource::read(reader, cache))
		{
			return false;
		}

		auto& fileSystem = FileSystem::get();

		reader.read(&mEmissive);
		reader.read(&mDoubleSided);
		reader.read(&mAlphaCutoff);
		reader.read(&mAlphaMode);

		uint32_t numDefines{ 0 };
		reader.read(&numDefines);

		std::vector<std::string> defines;
		for (uint32_t idx = 0; idx < numDefines; idx++)
		{
			defines.push_back(reader.readString());
		}

		auto shaderFileName = Resource::getReadPath(reader.getPath(), reader.readString());
		if (!shaderFileName.empty())
		{
			if (!load(shaderFileName, defines, cache))
			{
				LogError("Shader::load() failed for: %s!!", shaderFileName.c_str());
				return false;
			}
		}

		uint32_t numTextures{ 0 };
		reader.read(&numTextures);

		for (uint32_t idx = 0; idx < numTextures; idx++)
		{
			TextureType type{ TextureType::TwoD };
			reader.read(&type);

			auto key = reader.readString();
			auto textureFileName = Resource::getReadPath(reader.getPath(), reader.readString());
			auto samplerFileName = Resource::getReadPath(reader.getPath(), reader.readString());

			if (!addTexture(key, type, textureFileName, samplerFileName, cache))
			{
				LogError("Material::addTexture() failed for: %s!!", key.c_str());
				return false;
			}
		}

		return true;
	}

	bool Material::write(FileWriter& writer)
	{
		if (!Resource::write(writer))
		{
			return false;
		}

		auto& fileSystem = FileSystem::get();

		writer.write(&mEmissive);
		writer.write(&mDoubleSided);
		writer.write(&mAlphaCutoff);
		writer.write(&mAlphaMode);

		const uint32_t numDefines = (uint32_t)mShaderDefines.size();
		writer.write(&numDefines);

		for (const auto& define : mShaderDefines)
		{
			writer.writeString(define);
		}

		if (mShader != nullptr)
		{
			auto fileName = Resource::getWritePath(writer.getPath(), mShader->getFileName());
			writer.writeString(fileName);
		}

		const uint32_t numTextures = (uint32_t)mTextures.size();
		writer.write(&numTextures);

		for (auto& it : mTextures)
		{
			auto key = it.first;
			auto type = it.second.texture->getTextureType();
			auto textureFileName = it.second.texture->getFileName();
			auto samplerFileName = it.second.sampler->getFileName();

			textureFileName = Resource::getWritePath(writer.getPath(), textureFileName);
			samplerFileName = Resource::getWritePath(writer.getPath(), samplerFileName);

			writer.write(&type);
			writer.writeString(key);
			writer.writeString(textureFileName);
			writer.writeString(samplerFileName);
		}

		return true;
	}
}