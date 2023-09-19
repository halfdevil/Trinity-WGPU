#pragma once

#include "Graphics/Resource.h"
#include "Core/Observer.h"
#include <webgpu/webgpu_cpp.h>

namespace Trinity
{
    class Buffer : public Resource
    {
    public:

        Buffer() = default;
        virtual ~Buffer() = 0;

        Buffer(const Buffer&) = delete;
        Buffer& operator = (const Buffer&) = delete;

        Buffer(Buffer&&) noexcept = default;
        Buffer& operator = (Buffer&&) noexcept = default;

        const wgpu::Buffer& getHandle() const
        {
            return mHandle;
        }

        bool isValid() const
        {
            return mHandle != nullptr;
        }

        virtual std::type_index getType() const override;

        void mapAsync(uint32_t offset, uint32_t size);
        void unmap();

        void write(uint32_t offset, uint32_t size, const void* data) const;

    public:

        Observer<void*> onMapAsyncCompleted;

    protected:

        wgpu::Buffer mHandle{};
    };
}