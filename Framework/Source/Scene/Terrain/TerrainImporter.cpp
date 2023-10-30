#include "Scene/Terrain/TerrainImporter.h"
#include "Scene/Terrain/Terrain.h"
#include "Scene/Terrain/TerrainMaterial.h"
#include "Scene/Terrain/HeightMap.h"
#include "Graphics/Sampler.h"
#include "Graphics/Texture2D.h"
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

	static std::unique_ptr<HeightMap> createHeightMap(const std::string& heightMapPath, ResourceCache& cache,
		const std::string& outputPath, uint32_t size, float heightScale, bool loadContent = true)
	{
		fs::path fileName(outputPath);
		fileName.append(fs::path(heightMapPath).filename().string());
		fileName.replace_extension("tmap");

		bool isRAW = fs::path(heightMapPath).extension() == ".raw";
		auto heightMap = std::make_unique<HeightMap>();

		if (!heightMap->create(FileSystem::get().sanitizePath(fileName.string()), cache, loadContent))
		{
			LogError("HeightMap::create() failed for: '%s'", fileName.string().c_str());
			return nullptr;
		}

		if (isRAW)
		{
			if (!heightMap->load(heightMapPath, size, size, heightScale))
			{
				LogError("HeightMap::load() failed for: '%s'", fileName.string().c_str());
				return nullptr;
			}
		}
		else
		{
			if (!heightMap->load(heightMapPath, heightScale))
			{
				LogError("HeightMap::load() failed for: '%s'", fileName.string().c_str());
				return nullptr;
			}
		}

		return heightMap;
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

	static std::unique_ptr<Texture> createTexture(Image& image, bool hasMipmaps, ResourceCache& cache, 
		const std::string& texturesPath, bool loadContent = true)
	{
		auto fileName = fs::path(texturesPath);
		fileName.append(fs::path(image.getFileName()).filename().string());
		fileName.replace_extension("ttex");

		auto texture = std::make_unique<Texture2D>();
		if (!texture->create(FileSystem::get().sanitizePath(fileName.string()), cache, loadContent))
		{
			LogError("Sampler::create() failed!!");
			return nullptr;
		}

		texture->setImage(&image);
		texture->setHasMipmaps(hasMipmaps);

		return texture;
	}

	static std::unique_ptr<Shader> createShader(ResourceCache& cache, const std::vector<std::string>& defines, bool loadContent = true)
	{
		auto shader = std::make_unique<Shader>();
		if (!shader->create(TerrainMaterial::kDefaultShader, cache, false))
		{
			LogError("Shader::create() failed for: %s!!", TerrainMaterial::kDefaultShader);
			return nullptr;
		}

		if (loadContent)
		{
			ShaderPreProcessor processor;
			processor.addDefines(defines);

			if (!shader->load(TerrainMaterial::kDefaultShader, processor))
			{
				LogError("Shader::load() failed for: %s!!", TerrainMaterial::kDefaultShader);
				return nullptr;
			}
		}

		return shader;
	}

	static std::unique_ptr<Material> createMaterial(
		const std::string& blendMapFileName, 
		const std::vector<std::string>& layerFileNames,
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

		auto blendMapImage = createImage(blendMapFileName, cache, imagesPath, loadContent);
		if (!blendMapImage)
		{
			LogError("createImage() failed for: '%s'", blendMapFileName.c_str());
			return nullptr;
		}

		std::vector<std::unique_ptr<Image>> layerImages;
		for (auto& layerFileName : layerFileNames)
		{
			auto image = createImage(layerFileName, cache, imagesPath, loadContent);
			if (!image)
			{
				LogError("createImage() failed for: '%s'", layerFileName.c_str());
				return nullptr;
			}

			layerImages.push_back(std::move(image));
		}

		auto blendMapTexture = createTexture(*blendMapImage, false, cache, texturesPath, loadContent);
		if (!blendMapTexture)
		{
			LogError("createTexture() failed for: '%s'", blendMapFileName.c_str());
			return nullptr;
		}

		std::vector<std::unique_ptr<Texture>> layerTextures;
		for (auto& image : layerImages)
		{
			auto texture = createTexture(*image, true, cache, texturesPath, loadContent);
			if (!texture)
			{
				LogError("createTexture() failed for: '%s'", image->getFileName().c_str());
				return nullptr;
			}

			layerTextures.push_back(std::move(texture));
		}

		auto fileName = fs::path(materialsPath);
		fileName.append("terrain.mat");

		auto material = std::make_unique<TerrainMaterial>();
		if (!material->create(fileName.string(), cache, loadContent))
		{
			LogError("TerrainMaterial::create() failed for: '%s'", fileName.string().c_str());
			return nullptr;
		}

		std::vector<std::string> shaderDefines = {
			"has_blend_map_texture"
		};

		material->setBaseColorFactor(glm::vec4{ 0.5f, 0.5f, 0.5f, 1.0f });
		material->setLayerScale(glm::vec4{ 1.0f });
		material->setTexture("blend_map_texture", *blendMapTexture, *defaultSampler);

		const uint32_t numLayerTextures = (uint32_t)layerTextures.size();
		for (uint32_t idx = 0; idx < numLayerTextures; idx++)
		{
			auto name = std::format("layer{}_texture", idx + 1);
			shaderDefines.push_back("has_" + name);
			material->setTexture(name, *layerTextures[idx], *defaultSampler); 
		}

		auto shader = createShader(cache, shaderDefines, loadContent);
		if (!shader)
		{
			LogError("createDefaultShader() failed");
			return nullptr;
		}

		material->setShaderDefines(std::move(shaderDefines));
		material->setShader(*shader);

		cache.addResource(std::move(blendMapImage));
		cache.addResource(std::move(blendMapTexture));

		for (uint32_t idx = 0; idx < numLayerTextures; idx++)
		{
			cache.addResource(std::move(layerImages[idx]));
			cache.addResource(std::move(layerTextures[idx]));
		}

		cache.addResource(std::move(defaultSampler));
		cache.addResource(std::move(shader));

		return material;
	}

	static std::unique_ptr<Terrain> createTerrain(
		uint32_t size,
		uint32_t patchSize,
		float heightScale,
		float cellSpacing,
		const std::string& outputFileName,
		const std::string& heightMapFileName,
		const std::string& blendMapFileName,
		const std::vector<std::string>& layerFileNames,
		const std::string& outputPath,
		const std::string& materialsPath,
		const std::string& imagesPath,
		const std::string& texturesPath,
		const std::string& samplersPath,
		ResourceCache& cache,
		bool loadContent = true
	)
	{
		auto heightMap = createHeightMap(heightMapFileName, cache, outputPath, size, heightScale, loadContent);
		if (!heightMap)
		{
			LogError("createHeightMap() failed for: '%s'", heightMapFileName.c_str());
			return nullptr;
		}

		auto material = createMaterial(blendMapFileName, layerFileNames, cache, materialsPath, 
			imagesPath, texturesPath, samplersPath, loadContent);

		if (!material)
		{
			LogError("createMaterial() failed for: '%s'", outputFileName.c_str());
			return nullptr;
		}

		auto terrain = std::make_unique<Terrain>();
		if (!terrain->create(outputFileName, cache, loadContent))
		{
			LogError("Terrain::create() failed for: '%s'", outputFileName.c_str());
			return nullptr;
		}

		if (loadContent)
		{
			if (!terrain->load(cache, *heightMap, *material, patchSize, cellSpacing))
			{
				LogError("Terrain::load() failed for: '%s'", outputFileName.c_str());
				return nullptr;
			}
		}
		else
		{
			terrain->setPatchSize(patchSize);
			terrain->setCellSpacing(cellSpacing);
			terrain->setHeightMap(*heightMap);
			terrain->setMaterial(*material);
		}

		cache.addResource(std::move(heightMap));
		cache.addResource(std::move(material));

		return terrain;
	}

	Terrain* TerrainImporter::importTerrain(
		const std::string& outputFileName, 
		uint32_t size, 
		uint32_t patchSize, 
		float heightScale, 
		float cellSpacing, 
		const std::string& heightMapFileName, 
		const std::string& blendMapFileName, 
		const std::vector<std::string>& layerFileNames, 
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

		auto terrain = createTerrain(
			size,
			patchSize,
			heightScale,
			cellSpacing,
			outputFileName,
			heightMapFileName,
			blendMapFileName,
			layerFileNames,
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
			LogError("createTerrain() failed for: '%s'", outputFileName.c_str());
			return nullptr;
		}

		auto* terrainPtr = terrain.get();
		cache.addResource(std::move(terrain));

		return terrainPtr;
	}
}