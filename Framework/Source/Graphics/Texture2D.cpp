#include "Graphics/Texture2D.h"
#include "Graphics/GraphicsDevice.h"
#include "VFS/FileSystem.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"
#include "Core/Image.h"
#include "Core/ResourceCache.h"

namespace Trinity
{
	bool Texture2D::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		return Texture::create(fileName, cache, loadContent);
	}

	void Texture2D::destroy()
	{
		Texture::destroy();

		if (mHandle)
		{
			mHandle.Destroy();
			mHandle = nullptr;
			mView = nullptr;
		}
	}

	bool Texture2D::write()
	{
		return Texture::write();
	}

	bool Texture2D::create(uint32_t width, uint32_t height, wgpu::TextureFormat format, wgpu::TextureUsage usage)
	{
		const wgpu::Device& device = GraphicsDevice::get();

		mFormat = format;
		mWidth = width;
		mHeight = height;

		wgpu::Extent3D size = {
			.width = mWidth,
			.height = mHeight,
			.depthOrArrayLayers = 1
		};

		wgpu::TextureDescriptor textureDesc = {
			.usage = usage,
			.dimension = wgpu::TextureDimension::e2D,
			.size = size,
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

		wgpu::TextureViewDescriptor textureViewDesc = {
			.format = mFormat,
			.dimension = wgpu::TextureViewDimension::e2D,
			.baseMipLevel = 0,
			.mipLevelCount = 1,
			.baseArrayLayer = 0,
			.arrayLayerCount = 1
		};

		mView = mHandle.CreateView(&textureViewDesc);
		if (!mView)
		{
			LogError("wgpu::Texture::CreateView() failed!!");
			return false;
		}

		return true;
	}

	bool Texture2D::load(Image* image, wgpu::TextureFormat format, bool hasMipmaps)
	{
		const wgpu::Device& device = GraphicsDevice::get();
		const wgpu::Queue& queue = GraphicsDevice::get().getQueue();

		mImage = image;
		mFormat = format;
		mHasMipmaps = hasMipmaps;
		mWidth = image->getWidth();
		mHeight = image->getHeight();

		std::vector<Mipmap> mipmaps{};
		if (hasMipmaps)
		{
			mipmaps = image->generateMipmaps();
		}

		wgpu::Extent3D size = {
			.width = mWidth,
			.height = mHeight,
			.depthOrArrayLayers = 1
		};

		wgpu::TextureDescriptor textureDesc = {
			.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst,
			.dimension = wgpu::TextureDimension::e2D,
			.size = size,
			.format = mFormat,
			.mipLevelCount = mHasMipmaps ? (uint32_t)mipmaps.size() : 1,
			.sampleCount = 1
		};

		mHandle = device.CreateTexture(&textureDesc);
		if (!mHandle)
		{
			LogError("wgpu::Device::CreateTexture() failed!!");
			return false;
		}

		if (mHasMipmaps)
		{
			for (auto& mipmap : mipmaps)
			{
				upload(mipmap.level, mipmap.width, mipmap.height, image->getChannels(), 
					mipmap.data.data(), mipmap.data.size());
			}
		}
		else
		{
			const auto& imageData = image->getData();
			upload(image->getChannels(), imageData.data(), (uint32_t)imageData.size());
		}

		wgpu::TextureViewDescriptor textureViewDesc = {
			.format = mFormat,
			.dimension = wgpu::TextureViewDimension::e2D,
			.baseMipLevel = 0,
			.mipLevelCount = 1,
			.baseArrayLayer = 0,
			.arrayLayerCount = 1
		};

		mView = mHandle.CreateView(&textureViewDesc);
		if (!mView)
		{
			LogError("wgpu::Texture::CreateView() failed!!");
			return false;
		}

		return true;
	}

	void Texture2D::upload(uint32_t channels, const void* data, uint32_t size)
	{
		upload(0, mWidth, mHeight, channels, data, size);
	}

	void Texture2D::upload(uint32_t level, uint32_t width, uint32_t height, uint32_t channels, 
		const void* data, uint32_t size)
	{
		wgpu::ImageCopyTexture destination = {
			.texture = mHandle,
			.mipLevel = level,
			.aspect = wgpu::TextureAspect::All
		};

		wgpu::TextureDataLayout dataLayout = {
			.offset = 0,
			.bytesPerRow = width * channels,
			.rowsPerImage = height
		};

		wgpu::Extent3D texSize = {
			.width = width,
			.height = height,
			.depthOrArrayLayers = 1
		};

		const wgpu::Queue& queue = GraphicsDevice::get().getQueue();
		queue.WriteTexture(&destination, data, size, &dataLayout, &texSize);
	}

	void Texture2D::setImage(Image* image)
	{
		mImage = image;
	}

	void Texture2D::setHasMipmaps(bool hasMipmaps)
	{
		mHasMipmaps = hasMipmaps;
	}

	bool Texture2D::read(FileReader& reader, ResourceCache& cache)
	{
		if (!Texture::read(reader, cache))
		{
			return false;
		}

		reader.read(&mHasMipmaps);

		auto& fileSystem = FileSystem::get();
		auto imageFileName = Resource::getReadPath(reader.getPath(), reader.readString());

		if (!imageFileName.empty())
		{
			if (!cache.isLoaded<Image>(imageFileName))
			{
				auto image = std::make_unique<Image>();
				if (!image->create(imageFileName, cache))
				{
					LogError("Image::create() failed for: %s!!", imageFileName.c_str());
					return false;
				}

				cache.addResource(std::move(image));
			}

			auto* image = cache.getResource<Image>(imageFileName);
			if (!load(image, mFormat, mHasMipmaps))
			{
				LogError("Texture2D::load() failed for image: %s!!", imageFileName.c_str());
				return false;
			}
		}

		return true;
	}

	bool Texture2D::write(FileWriter& writer)
	{
		if (!Texture::write(writer))
		{
			return false;
		}

		writer.write(&mHasMipmaps);

		if (mImage != nullptr)
		{
			auto fileName = Resource::getWritePath(writer.getPath(), mImage->getFileName());
			writer.writeString(fileName);
		}

		return true;
	}
}