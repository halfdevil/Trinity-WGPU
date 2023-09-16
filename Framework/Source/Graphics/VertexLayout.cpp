#include "Graphics/VertexLayout.h"

namespace Trinity
{
    VertexLayout::VertexLayout(std::vector<wgpu::VertexAttribute> attributes)
    {
        setAttributes(std::move(attributes));
    }

    void VertexLayout::setAttributes(std::vector<wgpu::VertexAttribute> attributes)
    {
        mAttributes = std::move(attributes);
        mSize = 0;

        for (const wgpu::VertexAttribute& attribute : mAttributes)
        {
            switch (attribute.format)
            {
            case wgpu::VertexFormat::Float16x2:
                mSize += 4;
                break;

            case wgpu::VertexFormat::Float16x4:
                mSize += 8;
                break;

            case wgpu::VertexFormat::Float32:
                mSize += 4;
                break;

            case wgpu::VertexFormat::Float32x2:
                mSize += 8;
                break;

            case wgpu::VertexFormat::Float32x3:
                mSize += 12;
                break;

            case wgpu::VertexFormat::Float32x4:
                mSize += 16;
                break;

            case wgpu::VertexFormat::Uint32x4:
                mSize += 16;
                break;

            case wgpu::VertexFormat::Unorm8x4:
                mSize += 4;

            default:
                break;
            }
        }
    }
}