#include "Graphics/Material.h"
#include "Graphics/Texture2D.h"
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

	bool Material::addTexture(const std::string& name, const std::string& textureFileName, 
		const std::string& samplerFileName, ResourceCache& cache)
	{
		if (!cache.isLoaded<Texture>(textureFileName))
		{
			auto texture = std::make_unique<Texture2D>();
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

		auto shaderFileName = fileSystem.combinePath(reader.getPath(), reader.readString());
		shaderFileName = fileSystem.canonicalPath(shaderFileName);
		shaderFileName = fileSystem.sanitizePath(shaderFileName);

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
			auto key = reader.readString();
			auto textureFileName = reader.readString();
			auto samplerFileName = reader.readString();

			textureFileName = fileSystem.combinePath(reader.getPath(), textureFileName);
			textureFileName = fileSystem.canonicalPath(textureFileName);
			textureFileName = fileSystem.sanitizePath(textureFileName);

			samplerFileName = fileSystem.combinePath(reader.getPath(), samplerFileName);
			samplerFileName = fileSystem.canonicalPath(samplerFileName);
			samplerFileName = fileSystem.sanitizePath(samplerFileName);

			if (!addTexture(key, textureFileName, samplerFileName, cache))
			{
				LogError("Material::addTexture() failed for: %s!!", key.c_str());
				return false;
			}
		}

		return true;
	}

	bool Material::write(FileWriter& writer)
	{
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
			auto fileName = fileSystem.relativePath(mShader->getFileName(), writer.getPath());
			fileName = fileSystem.sanitizePath(fileName);

			writer.writeString(fileName);
		}

		const uint32_t numTextures = (uint32_t)mTextures.size();
		writer.write(&numTextures);

		for (auto& it : mTextures)
		{
			auto key = it.first;
			auto textureFileName = it.second.texture->getFileName();
			auto samplerFileName = it.second.sampler->getFileName();

			textureFileName = fileSystem.relativePath(textureFileName, writer.getPath());
			textureFileName = fileSystem.sanitizePath(textureFileName);

			samplerFileName = fileSystem.relativePath(samplerFileName, writer.getPath());
			samplerFileName = fileSystem.sanitizePath(samplerFileName);

			writer.writeString(key);
			writer.writeString(textureFileName);
			writer.writeString(samplerFileName);
		}

		return true;
	}
}