#pragma once

#include "Graphics/Texture.h"
#include <vector>

namespace Trinity
{
    class Image;

    class TextureCube : public Texture
    {
    public:

        static constexpr uint32_t kNumCubeMapFaces = 6;

        TextureCube()
            : Texture(TextureType::Cube)
        {
        }

        ~TextureCube() = default;

        TextureCube(const TextureCube&) = delete;
        TextureCube& operator = (const TextureCube&) = delete;

        TextureCube(TextureCube&&) = default;
        TextureCube& operator = (TextureCube&&) = default;

        uint32_t getSize() const
        {
            return mSize;
        }

        const std::vector<Image*>& getImages() const
        {
            return mImages;
        }

		virtual bool create(const std::string& fileName, ResourceCache& cache, bool loadContent = true) override;
		virtual void destroy() override;
		virtual bool write() override;

		virtual bool load(Image* image, wgpu::TextureFormat format);
		virtual bool load(const std::vector<Image*>& images, wgpu::TextureFormat format);
		
        virtual void upload(uint32_t channels, uint32_t face, const void* data, uint32_t size);
        virtual void setImages(std::vector<Image*>&& images);

    protected:

        virtual bool read(FileReader& reader, ResourceCache& cache) override;
        virtual bool write(FileWriter& writer) override;

    protected:

        uint32_t mSize{ 0 };
        std::vector<Image*> mImages;
    };
}