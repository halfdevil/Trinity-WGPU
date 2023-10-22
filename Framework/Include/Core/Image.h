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
		~Image();

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
		virtual bool write() override;
		virtual void destroy();

		virtual std::type_index getType() const override;

		virtual bool load(const std::string& filePath);
		virtual bool load(const std::vector<uint8_t>& data);
		virtual bool load(uint32_t width, uint32_t height, uint32_t depth, uint32_t channels,
			ImageType type, const uint8_t* data = nullptr);

		glm::vec4 getPixel(uint32_t x, uint32_t y) const;
		uint32_t getPixelAsRGBA(uint32_t x, uint32_t y) const;

		void setPixel(uint32_t x, uint32_t y, const glm::vec4& c);
		void convertToCube();

	private:

		void convertToVerticalCross();
		void convertToCubeMapFaces();

	private:

		ImageType mImageType{ ImageType::TwoD };
		uint32_t mWidth{ 0 };
		uint32_t mHeight{ 0 };
		uint32_t mDepth{ 1 };
		uint32_t mChannels{ 0 };
		std::vector<uint8_t> mData;
	};
}