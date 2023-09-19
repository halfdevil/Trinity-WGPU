#pragma once

#include "Graphics/Resource.h"
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

    class Texture : public Resource
    {
    public:

        static const uint32_t kInvalidTexture = (uint32_t)-1;

        Texture() = default;
        Texture(TextureType type) : 
            mTextureType(type)
        {
        }

        virtual ~Texture() = 0;

        Texture(const Texture&) = delete;
        Texture& operator = (const Texture&) = delete;

        Texture(Texture&&) = default;
        Texture& operator = (Texture&&) = default;

        TextureType getTextureType() const
        {
            return mTextureType;
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

        virtual std::type_index getType() const override;

    protected:

        TextureType mTextureType{ TextureType::Undefined };
        wgpu::TextureFormat mFormat{ wgpu::TextureFormat::RGBA8UnormSrgb };
        wgpu::Texture mHandle;
        wgpu::TextureView mView;
    };
}