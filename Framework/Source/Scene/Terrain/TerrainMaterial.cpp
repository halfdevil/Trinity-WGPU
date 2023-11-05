#include "Scene/Terrain/TerrainMaterial.h"
#include "Scene/Terrain/Terrain.h"
#include "Scene/Terrain/HeightMap.h"
#include "Graphics/Texture2D.h"
#include "Graphics/BindGroupLayout.h"
#include "Graphics/BindGroup.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/Sampler.h"
#include "VFS/FileSystem.h"
#include "Core/Logger.h"
#include "Core/ResourceCache.h"
#include <format>

namespace Trinity
{
	static glm::vec3 avgNormalFromQuad(float ha, float hb, float hc, float hd, float sizex, float sizey, float scalez)
	{
		glm::vec3 n0 = {
			-(hb - ha) * scalez * sizey,
			-sizex * (hc - ha) * scalez,
			sizex * sizey
		};

		glm::vec3 n1 = {
			-sizey * (hd - hc) * scalez,
			((hb - hc) * sizex - sizex * (hd - hc)) * scalez,
			sizey * sizex
		};

		return n0 + n1;
	}

	static int32_t coordClamp(int32_t val, int32_t limit)
	{
		if (val < 0)
		{
			return 0;
		}

		if (val > limit - 1)
		{
			return limit - 1;
		}

		return val;
	}

	static int32_t coordWrap(int32_t val, int32_t limit)
	{
		if (val < 0)
		{
			return limit + val;
		}

		if (val > limit - 1)
		{
			return val - limit;
		}

		return val;
	}

	static int32_t coordFix(int32_t val, int32_t limit, bool wrap)
	{
		if (wrap)
		{
			return coordWrap(val, limit);
		}

		return coordClamp(val, limit);
	}

	bool TerrainMaterial::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		return Material::create(fileName, cache, loadContent);
	}

	void TerrainMaterial::destroy()
	{
		Material::destroy();
	}

	bool TerrainMaterial::write()
	{
		return Material::write();
	}

	bool TerrainMaterial::addHeightMapTexture(const std::vector<float>& heightMapData, const glm::uvec2& size, 
		const MapDimension& mapDims, ResourceCache& cache)
	{
		SamplerProperties properties{
			.addressModeU = wgpu::AddressMode::Repeat,
			.addressModeV = wgpu::AddressMode::Repeat,
			.addressModeW = wgpu::AddressMode::Repeat,
			.magFilter = wgpu::FilterMode::Linear,
			.minFilter = wgpu::FilterMode::Linear,
			.mipmapFilter = wgpu::MipmapFilterMode::Linear
		};

		auto sampler = std::make_unique<Sampler>();
		if (!sampler->load(properties))
		{
			LogError("Sampler::load() failed");
			return false;
		}

		const wgpu::TextureUsage usage = wgpu::TextureUsage::TextureBinding |
			wgpu::TextureUsage::CopyDst;

		auto texture = std::make_unique<Texture2D>();
		if (!texture->create(size.x, size.y, wgpu::TextureFormat::R32Float, usage))
		{
			LogError("Texture2D::create() failed");
			return false;
		}

		texture->upload(sizeof(float), heightMapData.data(), sizeof(float) *
			size.x * size.y);

		setTexture("height_map_texture", *texture, *sampler);
		addShaderDefine("has_height_map_texture");

		cache.addResource(std::move(texture));
		cache.addResource(std::move(sampler));

		return true;
	}

	bool TerrainMaterial::addNormalMapTexture(const std::vector<float>& heightMapData, const glm::uvec2& size, 
		const MapDimension& mapDims, ResourceCache& cache, bool wrapEdges)
	{
		SamplerProperties properties{
			.addressModeU = wgpu::AddressMode::Repeat,
			.addressModeV = wgpu::AddressMode::Repeat,
			.addressModeW = wgpu::AddressMode::Repeat,
			.magFilter = wgpu::FilterMode::Linear,
			.minFilter = wgpu::FilterMode::Linear,
			.mipmapFilter = wgpu::MipmapFilterMode::Linear
		};

		auto sampler = std::make_unique<Sampler>();
		if (!sampler->load(properties))
		{
			LogError("Sampler::load() failed");
			return false;
		}

		std::vector<uint32_t> normalMapData(heightMapData.size());
		float stepx = 1.0f / (size.x - 1) * mapDims.size.x;
		float stepy = 1.0f / (size.y - 1) * mapDims.size.y;

		for (int32_t dist = 0; dist < 2; dist++)
		{
			for (int32_t y = 0; y < (int32_t)size.y; y++)
			{
				const float* hmScanLine0 = &heightMapData[coordFix(y - dist, size.y, wrapEdges) * size.x];
				const float* hmScanLine1 = &heightMapData[coordFix(y, size.y, wrapEdges) * size.x];
				const float* hmScanLine2 = &heightMapData[coordFix(y + dist, size.y, wrapEdges) * size.x];

				uint32_t* nmScanLine = &normalMapData[y * size.x];

				for (int32_t x = 0; x < (int32_t)size.x; x++)
				{
					auto xcoordm = coordFix(x - dist, size.x, wrapEdges);
					auto xcoord = coordFix(x, size.x, wrapEdges);
					auto xcoordp = coordFix(x + dist, size.x, wrapEdges);

					float ha = hmScanLine0[xcoordm];
					float hb = hmScanLine1[xcoordm];
					float hc = hmScanLine2[xcoordm];
					float hd = hmScanLine0[xcoord];
					float he = hmScanLine1[xcoord];
					float hf = hmScanLine2[xcoord];
					float hg = hmScanLine0[xcoordp];
					float hh = hmScanLine1[xcoordp];
					float hi = hmScanLine2[xcoordp];

					glm::vec3 norm{ 0.0f };
					norm += avgNormalFromQuad(ha, hb, hd, he, stepx, stepy, mapDims.size.z);
					norm += avgNormalFromQuad(hb, hc, he, hf, stepx, stepy, mapDims.size.z);
					norm += avgNormalFromQuad(hd, he, hg, hh, stepx, stepy, mapDims.size.z);
					norm += avgNormalFromQuad(he, hf, hh, hi, stepx, stepy, mapDims.size.z);
					norm = glm::normalize(norm);

					if (dist > 1)
					{
						glm::vec3 oldNorm{
							((nmScanLine[x] >> 16) / 65535.0f - 0.5f) / 0.5f,
							((nmScanLine[x] & 0xFFFF) / 65535.0f - 0.5f) / 0.5f,
							0.0f
						};

						norm += oldNorm * 1.0f;
						norm = glm::normalize(norm);
					}

					uint16_t a = (uint16_t)std::clamp(65535.0f * (norm.x * 0.5f + 0.5f), 0.0f, 65535.0f);
					uint16_t b = (uint16_t)std::clamp(65535.0f * (norm.y * 0.5f + 0.5f), 0.0f, 65535.0f);

					nmScanLine[x] = (a << 16) | b;
				}
			}
		}

		const wgpu::TextureUsage usage = wgpu::TextureUsage::TextureBinding |
			wgpu::TextureUsage::CopyDst;

		auto texture = std::make_unique<Texture2D>();
		if (!texture->create(size.x, size.y, wgpu::TextureFormat::RG16Float, usage))
		{
			LogError("Texture2D::create() failed");
			return false;
		}

		texture->upload(sizeof(float), normalMapData.data(), sizeof(uint32_t) *
			size.x * size.y);

		setTexture("normal_map_texture", *texture, *sampler);
		addShaderDefine("has_normal_map_texture");

		cache.addResource(std::move(texture));
		cache.addResource(std::move(sampler));

		return true;
	}

	void TerrainMaterial::setBaseColorFactor(const glm::vec4& baseColorFactor)
	{
		mBaseColorFactor = baseColorFactor;
	}

	void TerrainMaterial::setLayerScale(const glm::vec4& layerScale)
	{
		mLayerScale = layerScale;
	}

	bool TerrainMaterial::compile(ResourceCache& cache)
	{
		struct MaterialParams
		{
			glm::vec4 baseColor{ 0.0f };
			glm::vec4 layerScale{ 1.0f };
		};

		MaterialParams params = {
			.baseColor = mBaseColorFactor,
			.layerScale = mLayerScale
		};

		auto paramsBuffer = std::make_unique<UniformBuffer>();
		if (!paramsBuffer->create(sizeof(MaterialParams), &params))
		{
			LogError("UniformBuffer::create() failed!!");
			return false;
		}

		std::vector<BindGroupLayoutItem> layoutItems =
		{
			{
				.binding = 0,
				.shaderStages = wgpu::ShaderStage::Fragment,
				.bindingLayout = BufferBindingLayout {
					.type = wgpu::BufferBindingType::Uniform,
					.minBindingSize = sizeof(MaterialParams)
				}
			}
		};

		std::vector<BindGroupItem> items =
		{
			{
				.binding = 0,
				.size = sizeof(MaterialParams),
				.resource = BufferBindingResource(*paramsBuffer)
			}
		};

		if (auto it = mTextures.find("height_map_texture"); it != mTextures.end())
		{
			layoutItems.push_back({
				.binding = 1,
				.shaderStages = wgpu::ShaderStage::Vertex,
				.bindingLayout = SamplerBindingLayout {
					.type = wgpu::SamplerBindingType::Filtering
				}
			});

			layoutItems.push_back({
				.binding = 2,
				.shaderStages = wgpu::ShaderStage::Vertex,
				.bindingLayout = TextureBindingLayout {
					.sampleType = wgpu::TextureSampleType::UnfilterableFloat,
					.viewDimension = wgpu::TextureViewDimension::e2D
				}
			});

			items.push_back({
				.binding = 1,
				.resource = SamplerBindingResource(*(it->second.sampler))
			});

			items.push_back({
				.binding = 2,
				.resource = TextureBindingResource(*(it->second.texture))
			});
		}

		if (auto it = mTextures.find("normal_map_texture"); it != mTextures.end())
		{
			layoutItems.push_back({
				.binding = 3,
				.shaderStages = wgpu::ShaderStage::Fragment,
				.bindingLayout = SamplerBindingLayout {
					.type = wgpu::SamplerBindingType::Filtering
				}
			});

			layoutItems.push_back({
				.binding = 4,
				.shaderStages = wgpu::ShaderStage::Fragment,
				.bindingLayout = TextureBindingLayout {
					.sampleType = wgpu::TextureSampleType::Float,
					.viewDimension = wgpu::TextureViewDimension::e2D
				}
			});

			items.push_back({
				.binding = 3,
				.resource = SamplerBindingResource(*(it->second.sampler))
			});

			items.push_back({
				.binding = 4,
				.resource = TextureBindingResource(*(it->second.texture))
			});
		}

		if (auto it = mTextures.find("blend_map_texture"); it != mTextures.end())
		{
			layoutItems.push_back({
				.binding = 5,
				.shaderStages = wgpu::ShaderStage::Fragment,
				.bindingLayout = SamplerBindingLayout {
					.type = wgpu::SamplerBindingType::Filtering
				}
			});

			layoutItems.push_back({
				.binding = 6,
				.shaderStages = wgpu::ShaderStage::Fragment,
				.bindingLayout = TextureBindingLayout {
					.sampleType = wgpu::TextureSampleType::Float,
					.viewDimension = wgpu::TextureViewDimension::e2D
				}
			});

			items.push_back({
				.binding = 5,
				.resource = SamplerBindingResource(*(it->second.sampler))
			});

			items.push_back({
				.binding = 6,
				.resource = TextureBindingResource(*(it->second.texture))
			});
		}

		for (uint32_t idx = 0; idx < kMaxLayers; idx++)
		{
			auto name = std::format("layer{}_texture", idx + 1);			 
			if (auto it = mTextures.find(name); it != mTextures.end())
			{
				layoutItems.push_back({
					.binding = 2 * idx + 7,
					.shaderStages = wgpu::ShaderStage::Fragment,
					.bindingLayout = SamplerBindingLayout {
						.type = wgpu::SamplerBindingType::Filtering
					}
				});

				layoutItems.push_back({
					.binding = 2 * idx + 8,
					.shaderStages = wgpu::ShaderStage::Fragment,
					.bindingLayout = TextureBindingLayout {
						.sampleType = wgpu::TextureSampleType::Float,
						.viewDimension = wgpu::TextureViewDimension::e2D
					}
				});

				items.push_back({
					.binding = 2 * idx + 7,
					.resource = SamplerBindingResource(*(it->second.sampler))
				});

				items.push_back({
					.binding = 2 * idx + 8,
					.resource = TextureBindingResource(*(it->second.texture))
				});
			}
			else
			{
				break;
			}
		}
 
		auto bindGroupLayout = std::make_unique<BindGroupLayout>();
		if (!bindGroupLayout->create(std::move(layoutItems)))
		{
			LogError("BindGroupLayout::create() failed!!");
			return false;
		}

		auto bindGroup = std::make_unique<BindGroup>();
		if (!bindGroup->create(*bindGroupLayout, std::move(items)))
		{
			LogError("BindGroup::create() failed!!");
			return false;
		}

		mParamsBuffer = paramsBuffer.get();
		mBindGroupLayout = bindGroupLayout.get();
		mBindGroup = bindGroup.get();

		cache.addResource(std::move(paramsBuffer));
		cache.addResource(std::move(bindGroupLayout));
		cache.addResource(std::move(bindGroup));

		return true;
	}

	bool TerrainMaterial::read(FileReader& reader, ResourceCache& cache)
	{
		if (!Material::read(reader, cache))
		{
			return false;
		}

		reader.read(&mBaseColorFactor);
		reader.read(&mLayerScale);

		return true;
	}

	bool TerrainMaterial::write(FileWriter& writer)
	{
		if (!Material::write(writer))
		{
			return false;
		}

		writer.write(&mBaseColorFactor);
		writer.write(&mLayerScale);

		return true;
	}
}
