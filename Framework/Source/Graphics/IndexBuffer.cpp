#include "Graphics/IndexBuffer.h"
#include "Graphics/GraphicsDevice.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"

namespace Trinity
{
    IndexBuffer::~IndexBuffer()
    {
        destroy();
    }

    bool IndexBuffer::create(wgpu::IndexFormat indexFormat, uint32_t numIndices, const void* data)
    {
        const wgpu::Device& device = GraphicsDevice::get();

        mIndexFormat = indexFormat;
        mNumIndices = numIndices;
        mIndexSize = mIndexFormat == wgpu::IndexFormat::Uint16 ? 2 : 4;

        wgpu::BufferDescriptor bufferDescriptor{};
        bufferDescriptor.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        bufferDescriptor.size = mIndexSize * mNumIndices;
        bufferDescriptor.mappedAtCreation = false;

        mHandle = device.CreateBuffer(&bufferDescriptor);
        if (!mHandle)
        {
            LogError("wgpu::Device::CreateBuffer() failed!!");
            return false;
        }

        if (data)
        {
            write(0, mIndexSize * mNumIndices, data);
        }

        return true;
    }

    void IndexBuffer::destroy()
    {
        if (mHandle)
        {
            mHandle.Destroy();
            mHandle = nullptr;
        }
    }
}