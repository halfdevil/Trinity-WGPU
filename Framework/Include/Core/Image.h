#pragma once

#include "Core/Resource.h"
#include <cstdint>
#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace Trinity
{
	enum class ImageType : uint32_t
	{
		TwoD,
		Cube
	};

	class Image : public Resource
	{
	public:

		Image() = default;
		~Image() = default;

		Image(const Image&) = delete;
		Image& operator = (const Image&) = delete;

		Image(Image&&) = default;
		Image& operator = (Image&&) = default;

		ImageType getImageType() const
		{
			return mImageType;
		}

		uint32_t getWidth() const
		{
			return mWidth;
		}

		uint32_t getHeight() const
		{
			return mHeight;
		}

		uint32_t getDepth() const
		{
			return mDepth;
		}

		uint32_t getChannels() const
		{
			return mChannels;
		}

		const std::vector<uint8_t>& getData() const
		{
			return mData;
		}

		virtual bool create(const std::string& fileName, ResourceCache& cache, bool loadContent = true) override;
		virtual void destroy() override;
		virtual bool write() override;

		virtual std::type_index getType() const override;

		virtual bool load(const std::string& filePath);
		virtual bool load(const std::vector<uint8_t>& data);
		virtual bool load(uint32_t width, uint32_t height, uint32_t depth, uint32_t channels,
			ImageType type, const uint8_t* data = nullptr);

		virtual glm::vec4 getPixel(uint32_t x, uint32_t y) const;
		virtual uint32_t getPixelAsRGBA(uint32_t x, uint32_t y) const;

		virtual void setPixel(uint32_t x, uint32_t y, const glm::vec4& c);
		virtual void convertToCube();

	protected:

		virtual void convertToVerticalCross();
		virtual void convertToCubeMapFaces();
		virtual bool read(FileReader& reader, ResourceCache& cache) override;
		virtual bool write(FileWriter& writer) override;

	protected:

		ImageType mImageType{ ImageType::TwoD };
		uint32_t mWidth{ 0 };
		uint32_t mHeight{ 0 };
		uint32_t mDepth{ 1 };
		uint32_t mChannels{ 0 };
		std::vector<uint8_t> mData;
	};
}