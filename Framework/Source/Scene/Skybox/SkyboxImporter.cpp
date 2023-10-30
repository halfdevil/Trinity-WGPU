#include "Scene/Skybox/SkyboxImporter.h"
#include "Scene/Skybox/Skybox.h"
#include "Scene/Skybox/SkyboxMaterial.h"
#include "Graphics/Sampler.h"
#include "Graphics/TextureCube.h"
#include "Graphics/Shader.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"
#include "Core/Image.h"
#include "Core/ResourceCache.h"
#include "VFS/FileSystem.h"

namespace Trinity
{
	static std::unique_ptr<Image> createImage(const std::string& imagePath, ResourceCache& cache,
		const std::string& imagesPath, bool loadContent = true)
	{
		fs::path fileName(imagesPath);
		fileName.append(fs::path(imagePath).filename().string());
		fileName.replace_extension("png");

		auto image = std::make_unique<Image>();
		if (!image->create(FileSystem::get().sanitizePath(fileName.string()), cache, loadContent))
		{
			LogError("Image::create() failed for: '%s'", fileName.string().c_str());
			return nullptr;
		}

		if (!image->load(imagePath))
		{
			LogError("Image::load() failed for: '%s'", imagePath.c_str());
			return nullptr;
		}

		return image;
	}

	static std::unique_ptr<Sampler> createSampler(ResourceCache& cache, const std::string& samplersPath, bool loadContent = true)
	{
		auto fileName = fs::path(samplersPath);
		fileName.append("default.tsamp");

		auto sampler = std::make_unique<Sampler>();
		if (!sampler->create(FileSystem::get().sanitizePath(fileName.string()), cache, loadContent))
		{
			LogError("Sampler::create() failed for: '%s'", fileName.string().c_str());
			return nullptr;
		}

		SamplerProperties properties{
			.magFilter = wgpu::FilterMode::Linear,
			.minFilter = wgpu::FilterMode::Linear,
			.mipmapFilter = wgpu::MipmapFilterMode::Linear
		};

		sampler->setProperties(properties);
		return sampler;
	}

	static std::unique_ptr<Texture> createTexture(std::vector<Image*>&& images, ResourceCache& cache,
		const std::string& texturesPath, bool loadContent = true)
	{
		auto fileName = fs::path(texturesPath);
		fileName.append(fs::path(images[0]->getFileName()).filename().string());
		fileName.replace_extension("ttex");

		auto texture = std::make_unique<TextureCube>();
		if (!texture->create(FileSystem::get().sanitizePath(fileName.string()), cache, loadContent))
		{
			LogError("Sampler::create() failed!!");
			return nullptr;
		}

		texture->setImages(std::move(images));
		return texture;
	}

	std::unique_ptr<Shader> createShader(ResourceCache& cache, const std::vector<std::string>& defines, bool loadContent = true)
	{
		auto shader = std::make_unique<Shader>();
		if (!shader->create(SkyboxMaterial::kDefaultShader, cache, false))
		{
			LogError("Shader::create() failed for: %s!!", SkyboxMaterial::kDefaultShader);
			return nullptr;
		}

		if (loadContent)
		{
			ShaderPreProcessor processor;
			processor.addDefines(defines);

			if (!shader->load(SkyboxMaterial::kDefaultShader, processor))
			{
				LogError("Shader::load() failed for: %s!!", SkyboxMaterial::kDefaultShader);
				return nullptr;
			}
		}

		return shader;
	}

	std::unique_ptr<Material> createMaterial(
		const std::vector<std::string>& envMapFileNames,
		ResourceCache& cache,
		const std::string& materialsPath,
		const std::string& imagesPath,
		const std::string& texturesPath,
		const std::string& samplersPath,
		bool loadContent = true
	)
	{
		auto defaultSampler = createSampler(cache, samplersPath, loadContent);
		if (!defaultSampler)
		{
			LogError("createDefaultSampler() failed");
			return nullptr;
		}

		std::vector<Image*> images;
		if (envMapFileNames.size() == 1)
		{
			auto envMapImage = createImage(envMapFileNames[0], cache, imagesPath, loadContent);
			if (!envMapImage)
			{
				LogError("createImage() failed for: '%s'", envMapFileNames[0].c_str());
				return nullptr;
			}

			images.push_back(envMapImage.get());
			cache.addResource(std::move(envMapImage));
		}
		else
		{
			std::vector<Image*> images;
			for (auto& envMapFileName : envMapFileNames)
			{
				auto envMapImage = createImage(envMapFileName, cache, imagesPath, loadContent);
				if (!envMapImage)
				{
					LogError("createImage() failed for: '%s'", envMapFileName.c_str());
					return nullptr;
				}

				images.push_back(envMapImage.get());
				cache.addResource(std::move(envMapImage));
			}
		}
		
		auto envMapTexture = createTexture(std::move(images), cache, texturesPath, loadContent);
		if (!envMapTexture)
		{
			LogError("createTexture() failed for: '%s'", envMapFileNames[0].c_str());
			return nullptr;
		}

		auto fileName = fs::path(materialsPath);
		fileName.append("skybox.mat");

		auto material = std::make_unique<SkyboxMaterial>();
		if (!material->create(fileName.string(), cache, loadContent))
		{
			LogError("SkyboxMaterial::create() failed for: '%s'", fileName.string().c_str());
			return nullptr;
		}

		std::vector<std::string> shaderDefines = {
			"has_env_map_texture"
		};

		material->setBaseColorFactor(glm::vec4{ 0.5f, 0.5f, 0.5f, 1.0f });
		material->setTexture("env_map_texture", *envMapTexture, *defaultSampler);

		auto shader = createShader(cache, shaderDefines, loadContent);
		if (!shader)
		{
			LogError("createDefaultShader() failed");
			return nullptr;
		}

		material->setShaderDefines(std::move(shaderDefines));
		material->setShader(*shader);

		cache.addResource(std::move(envMapTexture));
		cache.addResource(std::move(defaultSampler));
		cache.addResource(std::move(shader));

		return material;
	}

	static std::unique_ptr<Skybox> createSkybox(
		float size,
		const std::string& outputFileName,
		const std::vector<std::string>& envMapFileNames,
		const std::string& outputPath,
		const std::string& materialsPath,
		const std::string& imagesPath,
		const std::string& texturesPath,
		const std::string& samplersPath,
		ResourceCache& cache,
		bool loadContent = true
	)
	{
		auto material = createMaterial(envMapFileNames, cache, materialsPath, imagesPath, 
			texturesPath, samplersPath, loadContent);

		if (!material)
		{
			LogError("createMaterial() failed for: '%s'", outputFileName.c_str());
			return nullptr;
		}

		auto terrain = std::make_unique<Skybox>();
		if (!terrain->create(outputFileName, cache, loadContent))
		{
			LogError("Skybox::create() failed for: '%s'", outputFileName.c_str());
			return nullptr;
		}

		if (loadContent)
		{
			if (!terrain->load(cache, *material, size))
			{
				LogError("Skybox::load() failed for: '%s'", outputFileName.c_str());
				return nullptr;
			}
		}
		else
		{
			terrain->setSize(size);
			terrain->setMaterial(*material);
		}

		cache.addResource(std::move(material));
		return terrain;
	}

	Skybox* SkyboxImporter::importSkybox(
		const std::string& outputFileName,
		float size,
		const std::vector<std::string>& envMapFileNames,
		ResourceCache& cache,
		bool loadContent
	)
	{
		auto outputPath = fs::path(outputFileName);
		outputPath.remove_filename();

		auto texturesPath = outputPath;
		texturesPath.append("Textures");

		auto imagesPath = outputPath;
		imagesPath.append("Images");

		auto samplersPath = outputPath;
		samplersPath.append("Samplers");

		auto materialsPath = outputPath;
		materialsPath.append("Materials");

		auto& fileSystem = FileSystem::get();
		fileSystem.createDirs(outputPath.string());
		fileSystem.createDirs(texturesPath.string());
		fileSystem.createDirs(imagesPath.string());
		fileSystem.createDirs(samplersPath.string());
		fileSystem.createDirs(materialsPath.string());

		auto terrain = createSkybox(
			size,
			outputFileName,
			envMapFileNames,
			outputPath.string(),
			materialsPath.string(),
			imagesPath.string(),
			texturesPath.string(),
			samplersPath.string(),
			cache,
			loadContent
		);

		if (!terrain)
		{
			LogError("createSkybox() failed for: '%s'", outputFileName.c_str());
			return nullptr;
		}

		auto* terrainPtr = terrain.get();
		cache.addResource(std::move(terrain));

		return terrainPtr;
	}
}