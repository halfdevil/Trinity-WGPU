#pragma once

#include "Core/Image.h"
#include <webgpu/webgpu_cpp.h>

namespace Trinity
{
    enum class TextureType : uint32_t
    {
        Undefined = 0,
        TwoD,
        Cube
    };

    class Texture
    {
    public:

        static const uint32_t kInvalidTexture = (uint32_t)-1;

        Texture() = default;
        Texture(TextureType type)
            : mType(type)
        {
        }

        virtual ~Texture() = 0;

        TextureType getType() const
        {
            return mType;
        }

        const wgpu::Texture& getHandle() const
        {
            return mHandle;
        }

        const wgpu::TextureView& getView() const
        {
            return mView;
        }

        wgpu::TextureFormat getFormat() const
        {
            return mFormat;
        }

    protected:

        TextureType mType{ TextureType::Undefined };
        wgpu::TextureFormat mFormat;
        wgpu::Texture mHandle;
        wgpu::TextureView mView;
    };
}