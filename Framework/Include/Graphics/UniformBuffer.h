#pragma once

#include "Graphics/Buffer.h"

namespace Trinity
{
    class UniformBuffer : public Buffer
    {
    public:

        UniformBuffer() = default;
        ~UniformBuffer();

        UniformBuffer(const UniformBuffer&) = default;
        UniformBuffer& operator = (const UniformBuffer&) = default;

        UniformBuffer(UniformBuffer&&) noexcept = default;
        UniformBuffer& operator = (UniformBuffer&&) noexcept = default;

        uint32_t getSize() const
        {
            return mSize;
        }

        bool create(uint32_t size, const void* data = nullptr);
        void destroy();

    private:

        uint32_t mSize{ 0 };
    };
}