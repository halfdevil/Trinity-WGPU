#include "Graphics/TextureCube.h"
#include "Graphics/GraphicsDevice.h"
#include "VFS/FileSystem.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"
#include "Core/Image.h"
#include "Core/ResourceCache.h"

namespace Trinity
{
    TextureCube::~TextureCube()
    {
        destroy();
	}

	bool TextureCube::create(const std::string& fileName, ResourceCache& cache)
	{
		auto& fileSystem = FileSystem::get();
		mFileName = fileName;

		if (fileSystem.isExist(fileName))
		{
			auto file = fileSystem.openFile(fileName, FileOpenMode::OpenRead);
			if (!file)
			{
				LogError("Error opening texture file: %s", fileName.c_str());
				return false;
			}

			FileReader reader(*file);
			if (!read(reader, cache))
			{
				LogError("TextureCube::read() failed for: %s!!", fileName.c_str());
				return false;
			}
		}

		return true;
	}

	bool TextureCube::write()
	{
		if (mFileName.empty())
		{
			LogError("Cannot write to file as filename is empty!!");
			return false;
		}

		auto file = FileSystem::get().openFile(mFileName, FileOpenMode::OpenWrite);
		if (!file)
		{
			LogError("Error opening texture file: %s", mFileName.c_str());
			return false;
		}

		FileWriter writer(*file);
		if (!write(writer))
		{
			LogError("TextureCube::write() failed for: %s!!", mFileName.c_str());
			return false;
		}

		return true;
	}

	void TextureCube::destroy()
	{
		if (mHandle)
		{
			mHandle.Destroy();
			mHandle = nullptr;
			mView = nullptr;
		}
	}

	bool TextureCube::load(const std::vector<Image*>& images, wgpu::TextureFormat format)
	{
		if (images.size() == 1)
		{
			return load(images[0], format);
		}

		Assert(images.size() == kNumCubeMapFaces, "Invalid number of images passed: %d!!",
			(uint32_t)images.size());

		const wgpu::Device& device = GraphicsDevice::get();
		const wgpu::Queue& queue = GraphicsDevice::get().getQueue();

		mImages = images;
		mFormat = format;
		mSize = images[0]->getWidth();

		wgpu::Extent3D texSize = {
			.width = mSize,
			.height = mSize,
			.depthOrArrayLayers = kNumCubeMapFaces
		};

		wgpu::TextureDescriptor textureDesc = {
			.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst,
			.dimension = wgpu::TextureDimension::e2D,
			.size = texSize,
			.format = mFormat,
			.mipLevelCount = 1,
			.sampleCount = 1
		};

		mHandle = device.CreateTexture(&textureDesc);
		if (!mHandle)
		{
			LogError("wgpu::Device::CreateTexture() failed!!");
			return false;
		}

		for (uint32_t idx = 0; idx < kNumCubeMapFaces; idx++)
		{
			auto* image = images[idx];
			const auto& imageData = image->getData();

			upload(image->getChannels(), idx, imageData.data(),
				(uint32_t)imageData.size());
		}

		wgpu::TextureViewDescriptor textureViewDesc = {
			.format = mFormat,
			.dimension = wgpu::TextureViewDimension::Cube,
			.baseMipLevel = 0,
			.mipLevelCount = 1,
			.baseArrayLayer = 0,
			.arrayLayerCount = kNumCubeMapFaces
		};

		mView = mHandle.CreateView(&textureViewDesc);
		if (!mView)
		{
			LogError("wgpu::Texture::CreateView() failed!!");
			return false;
		}

		return true;
	}

	bool TextureCube::load(Image* image, wgpu::TextureFormat format)
	{
		const wgpu::Device& device = GraphicsDevice::get();
		const wgpu::Queue& queue = GraphicsDevice::get().getQueue();

		if (image->getImageType() != ImageType::TwoD)
		{
			image->convertToCube();
		}

		mImages = { image };
		mSize = image->getWidth();
		mFormat = format;

		wgpu::Extent3D texSize = {
			.width = mSize,
			.height = mSize,
			.depthOrArrayLayers = kNumCubeMapFaces
		};

		wgpu::TextureDescriptor textureDesc = {
			.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst,
			.dimension = wgpu::TextureDimension::e2D,
			.size = texSize,
			.format = mFormat,
			.mipLevelCount = 1,
			.sampleCount = 1
		};

		mHandle = device.CreateTexture(&textureDesc);
		if (!mHandle)
		{
			LogError("wgpu::Device::CreateTexture() failed!!");
			return false;
		}

		const uint8_t* data = image->getData().data();
		const uint32_t dataSize = image->getWidth() * image->getHeight() *
			image->getChannels();

		for (uint32_t idx = 0; idx < kNumCubeMapFaces; idx++)
		{
			upload(image->getChannels(), idx, data, dataSize);
			data += dataSize;
		}

		wgpu::TextureViewDescriptor textureViewDesc = {
			.format = mFormat,
			.dimension = wgpu::TextureViewDimension::Cube,
			.baseMipLevel = 0,
			.mipLevelCount = 1,
			.baseArrayLayer = 0,
			.arrayLayerCount = kNumCubeMapFaces
		};

		mView = mHandle.CreateView(&textureViewDesc);
		if (!mView)
		{
			LogError("wgpu::Texture::CreateView() failed!!");
			return false;
		}

		return true;
	}
	
	void TextureCube::upload(uint32_t channels, uint32_t face, const void* data, uint32_t size)
	{
		wgpu::ImageCopyTexture destination = {
			.texture = mHandle,
			.mipLevel = 0,
			.origin = {
				.x = 0,
				.y = 0,
				.z = face
			},
			.aspect = wgpu::TextureAspect::All
		};

		wgpu::TextureDataLayout dataLayout = {
			.offset = 0,
			.bytesPerRow = mSize * channels,
			.rowsPerImage = mSize
		};

		wgpu::Extent3D texSize = {
			.width = mSize,
			.height = mSize,
			.depthOrArrayLayers = 1
		};

		const wgpu::Queue& queue = GraphicsDevice::get().getQueue();
		queue.WriteTexture(&destination, data, size, &dataLayout, &texSize);
	}

	void TextureCube::setImages(std::vector<Image*>&& images)
	{
		mImages = std::move(images);
	}

	bool TextureCube::read(FileReader& reader, ResourceCache& cache)
	{
		auto& fileSystem = FileSystem::get();

		if (!Texture::read(reader, cache))
		{
			return false;
		}

        uint32_t numImages{ 0 };
        reader.read(&numImages);

		std::vector<std::string> imageFileNames;
        for (uint32_t idx = 0; idx < numImages; idx++)
        {
			auto fileName = fileSystem.combinePath(reader.getPath(), reader.readString());
			fileName = fileSystem.canonicalPath(fileName);
			fileName = fileSystem.sanitizePath(fileName);

            imageFileNames.push_back(fileName);
        }

		for (auto& imageFileName : imageFileNames)
		{
			if (!cache.isLoaded<Image>(imageFileName))
			{
				auto image = std::unique_ptr<Image>();
				if (!image->create(imageFileName, cache))
				{
					LogError("Image::create() failed for: %s!!", imageFileName.c_str());
					return false;
				}

				cache.addResource(std::move(image));
			}
		}

		std::vector<Image*> images;
		for (auto& imageFileName : imageFileNames)
		{
			images.push_back(cache.getResource<Image>(imageFileName));
		}

		if (!load(images, mFormat))
		{
			LogError("TextureCube::load() failed!!");
			return false;
		}

		return true;
	}

	bool TextureCube::write(FileWriter& writer)
	{
		auto& fileSystem = FileSystem::get();

		if (!Texture::write(writer))
		{
			return false;
		}
        
        const uint32_t numImages = (uint32_t)mImages.size();
        writer.write((uint32_t*)&numImages);

		for (auto* image : mImages)
		{
			auto fileName = fileSystem.relativePath(image->getFileName(), writer.getPath());
			fileName = fileSystem.sanitizePath(fileName);

			writer.writeString(fileName);
		}

        return true;
	}
}