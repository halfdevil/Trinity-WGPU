#include "Graphics/Texture2D.h"
#include "Graphics/GraphicsDevice.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"
#include "Core/Image.h"

namespace Trinity
{
    Texture2D::~Texture2D()
    {
        destroy();
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

    bool Texture2D::create(const std::string& fileName, wgpu::TextureFormat format)
    {
        Image image;
        if (!image.create(fileName))
        {
            LogError("Image::create() failed for: %s!!", fileName.c_str());
            return false;
        }

        return create(image, format);
    }

    bool Texture2D::create(const Image& image, wgpu::TextureFormat format)
    {
        const wgpu::Device& device = GraphicsDevice::get();
        const wgpu::Queue& queue = GraphicsDevice::get().getQueue();

        mFormat = format;
        mWidth = image.getWidth();
        mHeight = image.getHeight();

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
            .mipLevelCount = 1,
            .sampleCount = 1
        };

        mHandle = device.CreateTexture(&textureDesc);
        if (!mHandle)
        {
            LogError("wgpu::Device::CreateTexture() failed!!");
            return false;
        }

        write(image.getChannels(), image.getData().data(),
            static_cast<uint32_t>(image.getData().size()));

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

    void Texture2D::destroy()
    {
        if (mHandle)
        {
            mHandle.Destroy();
            mHandle = nullptr;
            mView = nullptr;
        }
    }

    void Texture2D::write(uint32_t channels, const void* data, uint32_t size)
    {
        wgpu::ImageCopyTexture destination = {
            .texture = mHandle,
            .mipLevel = 0,
            .aspect = wgpu::TextureAspect::All
        };

        wgpu::TextureDataLayout dataLayout = {
            .offset = 0,
            .bytesPerRow = mWidth * channels,
            .rowsPerImage = mHeight
        };

        wgpu::Extent3D texSize = {
            .width = mWidth,
            .height = mHeight,
            .depthOrArrayLayers = 1
        };

        const wgpu::Queue& queue = GraphicsDevice::get().getQueue();
        queue.WriteTexture(&destination, data, size, &dataLayout, &texSize);
    }
}