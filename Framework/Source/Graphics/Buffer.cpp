#include "Graphics/Buffer.h"
#include "Graphics/GraphicsDevice.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"

namespace Trinity
{
    Buffer::~Buffer()
    {
    }

    void Buffer::mapAsync(uint32_t offset, uint32_t size)
    {
        mHandle.MapAsync(wgpu::MapMode::Write, offset, size,
            [](WGPUBufferMapAsyncStatus status, void* userdata) {
                Assert(status == WGPUBufferMapAsyncStatus_Success, "wgpu::Buffer::MapAsync() failed!!");

                Buffer* buffer = reinterpret_cast<Buffer*>(userdata);
                buffer->onMapAsyncCompleted.notify(buffer->mHandle.GetMappedRange());
        }, this);
    }

    void Buffer::unmap()
    {
        mHandle.Unmap();
    }

    void Buffer::write(uint32_t offset, uint32_t size, const void* data) const
    {
        const wgpu::Queue& queue = GraphicsDevice::get().getQueue();
        queue.WriteBuffer(mHandle, offset, data, size);
    }
}