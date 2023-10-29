#include "Graphics/PBRMaterial.h"
#include "Graphics/Texture2D.h"
#include "Graphics/BindGroupLayout.h"
#include "Graphics/BindGroup.h"
#include "Graphics/UniformBuffer.h"
#include "VFS/FileSystem.h"
#include "Core/Logger.h"
#include "Core/ResourceCache.h"

namespace Trinity
{
	bool PBRMaterial::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		return Material::create(fileName, cache, loadContent);
	}

	void PBRMaterial::destroy()
	{
		Material::destroy();
	}

	bool PBRMaterial::write()
	{
		return Material::write();
	}

	void PBRMaterial::setBaseColorFactor(const glm::vec4& baseColorFactor)
	{
		mBaseColorFactor = baseColorFactor;
	}

	void PBRMaterial::setMetallicFactor(float metallicFactor)
	{
		mMettalicFactor = metallicFactor;
	}

	void PBRMaterial::setRoughnessFactor(float roughnessFactor)
	{
		mRoughnessFactor = roughnessFactor;
	}

	bool PBRMaterial::compile(ResourceCache& cache)
	{
		struct MaterialParams
		{
			glm::vec4 emissive{ 0.0f };
			glm::vec4 baseColor{ 0.0f };
			float metallic{ 0.0f };
			float roughness{ 0.0f };
			glm::vec2 padding{ 0.0f };
		};

		MaterialParams params = {
			.emissive = glm::vec4(mEmissive, 1.0f),
			.baseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
			.metallic = mMettalicFactor,
			.roughness = mRoughnessFactor
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

		if (auto it = mTextures.find("base_color_texture"); it != mTextures.end())
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

		if (auto it = mTextures.find("normal_texture"); it != mTextures.end())
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

		if (auto it = mTextures.find("metallic_roughness_texture"); it != mTextures.end())
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

	bool PBRMaterial::read(FileReader& reader, ResourceCache& cache)
	{
		if (!Material::read(reader, cache))
		{
			return false;
		}

		reader.read(&mBaseColorFactor);
		reader.read(&mMettalicFactor);
		reader.read(&mRoughnessFactor);

		return true;
	}

	bool PBRMaterial::write(FileWriter& writer)
	{
		if (!Material::write(writer))
		{
			return false;
		}

		writer.write(&mBaseColorFactor);
		writer.write(&mMettalicFactor);
		writer.write(&mRoughnessFactor);

		return true;
	}
}