#include "Graphics/Sampler.h"
#include "Graphics/GraphicsDevice.h"
#include "VFS/FileSystem.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"

namespace Trinity
{
    Sampler::~Sampler()
    {
        destroy();
	}

	bool Sampler::create(const std::string& fileName, ResourceCache& cache)
	{
		auto& fileSystem = FileSystem::get();
		mFileName = fileName;

		if (fileSystem.isExist(fileName))
		{
			auto file = FileSystem::get().openFile(fileName, FileOpenMode::OpenRead);
			if (!file)
			{
				LogError("Error opening texture file: %s", fileName.c_str());
				return false;
			}

			FileReader reader(*file);
			if (!read(reader, cache))
			{
				LogError("Sampler::read() failed for: %s!!", fileName.c_str());
				return false;
			}
		}

		return true;
	}

	bool Sampler::write()
	{
		if (mFileName.empty())
		{
			LogError("Cannot write to file as filename is empty!!");
			return false;
		}

		auto file = FileSystem::get().openFile(mFileName, FileOpenMode::OpenWrite);
		if (!file)
		{
			LogError("Error opening sampler file: %s", mFileName.c_str());
			return false;
		}

		FileWriter writer(*file);
		if (!write(writer))
		{
			LogError("Sampler::write() failed for: %s!!", mFileName.c_str());
		}

		return true;
	}

	void Sampler::destroy()
	{
		mHandle = nullptr;
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
		writer.write(&mProperties);
		return true;
	}
}