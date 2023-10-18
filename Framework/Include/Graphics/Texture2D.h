#pragma once

#include "Graphics/Texture.h"

namespace Trinity
{
    class Image;

    class Texture2D : public Texture
    {
    public:

        Texture2D()
            : Texture(TextureType::TwoD)
        {
        }

        ~Texture2D();

        Texture2D(const Texture2D&) = delete;
        Texture2D& operator = (const Texture2D&) = delete;

        Texture2D(Texture2D&&) = default;
        Texture2D& operator = (Texture2D&&) = default;

        Image* getImage() const
        {
            return mImage;
        }

		virtual bool create(const std::string& fileName, ResourceCache& cache) override;
		virtual bool write() override;

		virtual bool create(uint32_t width, uint32_t height, wgpu::TextureFormat format, wgpu::TextureUsage usage);
		virtual void destroy();

		virtual bool load(Image* image, wgpu::TextureFormat format);
        virtual void upload(uint32_t channels, const void* data, uint32_t size);
        virtual void setImage(Image* image);

	protected:

		virtual bool read(FileReader& reader, ResourceCache& cache) override;
		virtual bool write(FileWriter& writer) override;

    protected:

        uint32_t mWidth{ 0 };
        uint32_t mHeight{ 0 };
        Image* mImage{ nullptr };
    };
}