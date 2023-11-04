#include "Graphics/VertexBuffer.h"
#include "Graphics/GraphicsDevice.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"

namespace Trinity
{
    VertexBuffer::~VertexBuffer()
    {
        destroy();
    }

    bool VertexBuffer::create(const VertexLayout& vertexLayout, uint32_t numVertices, const void* data)
    {
        const wgpu::Device& device = GraphicsDevice::get();
        mNumVertices = numVertices;

        wgpu::BufferDescriptor bufferDescriptor{};
        bufferDescriptor.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        bufferDescriptor.size = vertexLayout.getSize() * mNumVertices;
        bufferDescriptor.mappedAtCreation = false;

        mHandle = device.CreateBuffer(&bufferDescriptor);
        if (!mHandle)
        {
            LogError("wgpu::Device::CreateBuffer() failed!!");
            return false;
        }

        if (data)
        {
            write(0, vertexLayout.getSize() * mNumVertices, data);
        }

        return true;
    }

    void VertexBuffer::destroy()
    {
        if (mHandle)
        {
            mHandle.Destroy();
            mHandle = nullptr;
        }
    }
}