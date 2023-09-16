#pragma once

#include "VFS/FileWriter.h"
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

	class Image
	{
	public:

		Image() = default;
		~Image();

		Image(const Image&) = default;
		Image& operator = (const Image&) = default;

		Image(Image&&) = default;
		Image& operator = (Image&&) = default;

		ImageType getType() const
		{
			return mType;
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

		bool create(const std::string& filePath, ImageType type = ImageType::TwoD);
		bool create(const uint8_t* data, uint32_t dataSize, ImageType type = ImageType::TwoD);

		bool create(uint32_t width, uint32_t height, uint32_t depth, uint32_t channels,
			ImageType type, const uint8_t* data = nullptr);

		void destroy();
		bool write(FileWriter& writer);

		glm::vec4 getPixel(uint32_t x, uint32_t y) const;
		uint32_t getPixelAsRGBA(uint32_t x, uint32_t y) const;

		void setPixel(uint32_t x, uint32_t y, const glm::vec4& c);

	private:

		void convertToVerticalCross();
		void convertToCubeMapFaces();

	private:

		ImageType mType{ ImageType::TwoD };
		uint32_t mWidth{ 0 };
		uint32_t mHeight{ 0 };
		uint32_t mDepth{ 1 };
		uint32_t mChannels{ 0 };
		std::vector<uint8_t> mData;
	};
}