#include "Graphics/TextureCube.h"
#include "Graphics/GraphicsDevice.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"
#include "Core/Image.h"

namespace Trinity
{
    TextureCube::~TextureCube()
    {
        destroy();
    }

    bool TextureCube::create(const std::vector<std::string>& fileNames, wgpu::TextureFormat format)
    {
        Assert(fileNames.size() >= kNumCubeMapFaces, "Invalid number of fileNames passed!!");

        std::vector<Image> images;
        for (const std::string& fileName : fileNames)
        {
            Image image;
            if (!image.create(fileName))
            {
                LogError("Image::create() failed for: %s!!", fileName.c_str());
                return false;
            }

            Assert(image.getWidth() == image.getHeight(),
                "Cubemap images must be of same width and height!!");

            mSize = image.getWidth();
            images.push_back(std::move(image));
        }

        const wgpu::Device& device = GraphicsDevice::get();
        const wgpu::Queue& queue = GraphicsDevice::get().getQueue();

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

        for (uint32_t idx = 0; idx < kNumCubeMapFaces; idx++)
        {
            const Image& image = images[idx];
            const auto& imageData = image.getData();

            write(image.getChannels(), idx, imageData.data(),
                static_cast<uint32_t>(imageData.size()));
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

    bool TextureCube::create(const std::string& fileName, wgpu::TextureFormat format)
    {
        Image image;
        if (!image.create(fileName, ImageType::Cube))
        {
            LogError("Image::create() failed for: %s!!", fileName.c_str());
            return false;
        }

        Assert(image.getType() == ImageType::Cube, "Only cube image supported");

        const wgpu::Device& device = GraphicsDevice::get();
        const wgpu::Queue& queue = GraphicsDevice::get().getQueue();

        mSize = image.getWidth();
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

        const uint8_t* data = image.getData().data();
        const uint32_t dataSize = image.getWidth() * image.getHeight() *
            image.getChannels();

        for (uint32_t idx = 0; idx < kNumCubeMapFaces; idx++)
        {
            write(image.getChannels(), idx, data, dataSize);
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

    void TextureCube::destroy()
    {
        if (mHandle)
        {
            mHandle.Destroy();
            mHandle = nullptr;
            mView = nullptr;
        }
    }

    void TextureCube::write(uint32_t channels, uint32_t face, const void* data, uint32_t size)
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
}