#include "Scene/Skybox/SkyboxMaterial.h"
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
	bool SkyboxMaterial::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		return Material::create(fileName, cache, loadContent);
	}

	void SkyboxMaterial::destroy()
	{
		Material::destroy();
	}

	bool SkyboxMaterial::write()
	{
		return Material::write();
	}

	void SkyboxMaterial::setBaseColorFactor(const glm::vec4& baseColorFactor)
	{
		mBaseColorFactor = baseColorFactor;
	}

	bool SkyboxMaterial::compile(ResourceCache& cache)
	{
		struct MaterialParams
		{
			glm::vec4 baseColor{ 0.0f };
		};

		MaterialParams params = {
			.baseColor = mBaseColorFactor
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

		if (auto it = mTextures.find("env_map_texture"); it != mTextures.end())
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
					.viewDimension = wgpu::TextureViewDimension::Cube
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

	bool SkyboxMaterial::read(FileReader& reader, ResourceCache& cache)
	{
		if (!Material::read(reader, cache))
		{
			return false;
		}

		reader.read(&mBaseColorFactor);
		return true;
	}

	bool SkyboxMaterial::write(FileWriter& writer)
	{
		if (!Material::write(writer))
		{
			return false;
		}

		writer.write(&mBaseColorFactor);
		return true;
	}
}
