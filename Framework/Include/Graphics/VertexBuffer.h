#pragma once

#include "Graphics/Buffer.h"
#include "Graphics/VertexLayout.h"

namespace Trinity
{
    class VertexBuffer : public Buffer
    {
    public:

        VertexBuffer() = default;
        ~VertexBuffer();

        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator = (const VertexBuffer&) = delete;

        VertexBuffer(VertexBuffer&&) = default;
        VertexBuffer& operator = (VertexBuffer&&) = default;

        const VertexLayout* getVertexLayout() const
        {
            return mVertexLayout;
        }

        uint32_t getNumVertices() const
        {
            return mNumVertices;
        }

        const wgpu::VertexBufferLayout& getBufferLayout() const
        {
            return mBufferLayout;
        }

        bool create(const VertexLayout& vertexLayout, uint32_t numVertices, const void* data = nullptr);
        void destroy();

    private:

        const VertexLayout* mVertexLayout{ nullptr };
        uint32_t mNumVertices{ 0 };
        wgpu::VertexBufferLayout mBufferLayout;
    };
}