#include "Graphics/Sampler.h"
#include "Graphics/GraphicsDevice.h"
#include "VFS/FileSystem.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"

namespace Trinity
{
	bool Sampler::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		return Resource::create(fileName, cache, loadContent);
	}

	void Sampler::destroy()
	{
		Resource::destroy();
		mHandle = nullptr;
	}

	bool Sampler::write()
	{
		return Resource::write();
	}

	bool Sampler::load(const SamplerProperties& samplerProps)
	{
		const wgpu::Device& device = GraphicsDevice::get();

		mProperties = samplerProps;
		wgpu::SamplerDescriptor samplerDesc = {
			.addressModeU = samplerProps.addressModeU,
			.addressModeV = samplerProps.addressModeV,
			.addressModeW = samplerProps.addressModeW,
			.magFilter = samplerProps.magFilter,
			.minFilter = samplerProps.minFilter,
			.mipmapFilter = samplerProps.mipmapFilter,
			.compare = samplerProps.compare
		};

		mHandle = device.CreateSampler(&samplerDesc);
		if (!mHandle)
		{
			LogError("wgpu::Device::CreateSampler() failed!!");
			return false;
		}

		return true;
	}

	void Sampler::setProperties(SamplerProperties& samplerProps)
	{
		mProperties = samplerProps;
	}

	std::type_index Sampler::getType() const
	{
		return typeid(Sampler);
	}

	bool Sampler::read(FileReader& reader, ResourceCache& cache)
	{
		if (!Resource::read(reader, cache))
		{
			return false;
		}

		reader.read(&mProperties);
		if (!load(mProperties))
		{
			LogError("Sampler::load() failed!!");
			return false;
		}

		return true;
	}

	bool Sampler::write(FileWriter& writer)
	{
		if (!Resource::write(writer))
		{
			return false;
		}

		writer.write(&mProperties);
		return true;
	}
}