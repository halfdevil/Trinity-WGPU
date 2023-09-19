#pragma once

#include "Graphics/Resource.h"
#include <vector>
#include <webgpu/webgpu_cpp.h>

namespace Trinity
{
    class VertexLayout : public Resource
    {
    public:

        VertexLayout() = default;
        VertexLayout(std::vector<wgpu::VertexAttribute> attributes);

        const std::vector<wgpu::VertexAttribute>& getAttributes() const
        {
            return mAttributes;
        }

        uint32_t getSize() const
        {
            return mSize;
        }

        virtual std::type_index getType() const override;
        virtual void setAttributes(std::vector<wgpu::VertexAttribute> attributes);

    private:

        std::vector<wgpu::VertexAttribute> mAttributes;
        uint32_t mSize{ 0 };
    };
}