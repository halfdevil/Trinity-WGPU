#pragma once

#include "Graphics/Texture.h"

namespace Trinity
{
    class Texture2D : public Texture
    {
    public:

        static constexpr const char* kEmpty = "/Assets/Framework/Textures/Empty.png";

        Texture2D()
            : Texture(TextureType::TwoD)
        {
        }

        ~Texture2D();

        Texture2D(const Texture2D&) = delete;
        Texture2D& operator = (const Texture2D&) = delete;

        Texture2D(Texture2D&&) = default;
        Texture2D& operator = (Texture2D&&) = default;

        bool create(uint32_t width, uint32_t height, wgpu::TextureFormat format, wgpu::TextureUsage usage);
        bool create(const std::string& fileName, wgpu::TextureFormat format);
        void destroy();

        void write(uint32_t channels, const void* data, uint32_t size);

    private:

        uint32_t mWidth{ 0 };
        uint32_t mHeight{ 0 };
    };
}