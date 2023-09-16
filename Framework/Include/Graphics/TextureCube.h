#pragma once

#include "Graphics/Texture.h"
#include <vector>

namespace Trinity
{
    class TextureCube : public Texture
    {
    public:

        static constexpr uint32_t kNumCubeMapFaces = 6;

        TextureCube()
            : Texture(TextureType::Cube)
        {
        }

        ~TextureCube();

        TextureCube(const TextureCube&) = delete;
        TextureCube& operator = (const TextureCube&) = delete;

        TextureCube(TextureCube&&) = default;
        TextureCube& operator = (TextureCube&&) = default;

        uint32_t getSize() const
        {
            return mSize;
        }

        bool create(const std::vector<std::string>& fileNames, wgpu::TextureFormat format);
        bool create(const std::string& fileName, wgpu::TextureFormat format);
        void destroy();

        void write(uint32_t channels, uint32_t face, const void* data, uint32_t size);

    private:

        uint32_t mSize{ 0 };
    };
}