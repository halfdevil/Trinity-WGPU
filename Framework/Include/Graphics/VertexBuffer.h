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

        uint32_t getNumVertices() const
        {
            return mNumVertices;
        }

        bool create(const VertexLayout& vertexLayout, uint32_t numVertices, const void* data = nullptr);
        void destroy();

    private:

        uint32_t mNumVertices{ 0 };
    };
}