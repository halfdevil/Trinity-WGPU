#include "Scene/Terrain/TerrainMaterial.h"
#include "Graphics/Texture2D.h"
#include "Graphics/BindGroupLayout.h"
#include "Graphics/BindGroup.h"
#include "Graphics/UniformBuffer.h"
#include "VFS/FileSystem.h"
#include "Core/Logger.h"
#include "Core/ResourceCache.h"
#include <format>

namespace Trinity
{
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

		if (auto it = mTextures.find("blend_map"); it != mTextures.end())
		{
			layoutItems.push_back({
				.binding = 1,
				.shaderStages = wgpu::ShaderStage::Fragment,
				.bindingLayout = SamplerBindingLayout {
					.type = wgpu::SamplerBindingType::Filtering
				}
			});

			layoutItems.push_back({
				.binding = 2,
				.shaderStages = wgpu::ShaderStage::Fragment,
				.bindingLayout = TextureBindingLayout {
					.sampleType = wgpu::TextureSampleType::Float,
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

		for (uint32_t idx = 0; idx < kMaxLayers; idx++)
		{
			auto name = std::format("layer_{}", idx);			 
			if (auto it = mTextures.find(name); it != mTextures.end())
			{
				layoutItems.push_back({
					.binding = 2 * idx + 3,
					.shaderStages = wgpu::ShaderStage::Fragment,
					.bindingLayout = SamplerBindingLayout {
						.type = wgpu::SamplerBindingType::Filtering
					}
				});

				layoutItems.push_back({
					.binding = 2 * idx + 4,
					.shaderStages = wgpu::ShaderStage::Fragment,
					.bindingLayout = TextureBindingLayout {
						.sampleType = wgpu::TextureSampleType::Float,
						.viewDimension = wgpu::TextureViewDimension::e2D
					}
				});

				items.push_back({
					.binding = 2 * idx + 3,
					.resource = SamplerBindingResource(*(it->second.sampler))
				});

				items.push_back({
					.binding = 2 * idx + 4,
					.resource = TextureBindingResource(*(it->second.texture))
				});
			}
			else
			{
				break;
			}
		}
 
		auto bindGroupLayout = std::make_unique<BindGroupLayout>();
		if (!mBindGroupLayout->create(std::move(layoutItems)))
		{
			LogError("BindGroupLayout::create() failed!!");
			return false;
		}

		auto bindGroup = std::make_unique<BindGroup>();
		if (!mBindGroup->create(*mBindGroupLayout, std::move(items)))
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
