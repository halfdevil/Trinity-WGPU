#pragma once

#include "Graphics/BindGroupLayout.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/Texture.h"
#include "Graphics/Sampler.h"
#include <optional>

namespace Trinity
{
    struct BufferBindingResource
    {
        BufferBindingResource(const Buffer& inBuffer)
            : buffer(inBuffer)
        {
        }

        const Buffer& buffer;
    };

    struct TextureBindingResource
    {
        TextureBindingResource(const Texture& inTexture)
            : texture(inTexture)
        {
        }

        const Texture& texture;
    };

    struct SamplerBindingResource
    {
        SamplerBindingResource(const Sampler& inSampler)
            : sampler(inSampler)
        {
        }

        const Sampler& sampler;
    };

    struct EmptyBindingResource
    {
        EmptyBindingResource() = default;
    };

    struct BindGroupItem
    {
        uint32_t binding{ 0 };
        uint64_t offset{ 0 };
        uint64_t size{ 0 };
        std::variant<
            BufferBindingResource,
            TextureBindingResource,
            SamplerBindingResource,
            EmptyBindingResource> resource;
    };

    class BindGroup
    {
    public:

        BindGroup() = default;
        ~BindGroup();

        BindGroup(const BindGroup&) = delete;
        BindGroup& operator = (const BindGroup&) = delete;

        BindGroup(BindGroup&&) noexcept = default;
        BindGroup& operator = (BindGroup&&) noexcept = default;

        const wgpu::BindGroup& getHandle() const
        {
            return mHandle;
        }

        bool isValid() const
        {
            return mHandle != nullptr;
        }

        bool create(const BindGroupLayout& layout, const std::vector<BindGroupItem>& items);
        void destroy();

    private:

        wgpu::BindGroup mHandle;
    };
}