#include "Core/Image.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"
#include "VFS/FileSystem.h"

#define _USE_MATH_DEFINES
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
#include <stb_image_resize.h>

namespace Trinity
{
	bool Image::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		return Resource::create(fileName, cache, loadContent);
	}

	void Image::destroy()
	{
		Resource::destroy();
		mData.clear();
	}

	bool Image::write()
	{
		if (FileSystem::get().isExist(mFileName))
		{
			LogWarning("Image file '%s' already exists, skipping re-write!!", mFileName.c_str());
			return true;
		}

		return Resource::write();
	}

	std::type_index Image::getType() const
	{
		return typeid(Image);
	}

	bool Image::load(const std::string& filePath)
	{
		auto file = FileSystem::get().openFile(filePath, FileOpenMode::OpenRead);
		if (!file)
		{
			LogError("FileSystem::openFile() failed for: %s", filePath.c_str());
			return false;
		}

		FileReader reader(*file);
		std::vector<uint8_t> buffer(reader.getSize());
		reader.read(buffer.data(), reader.getSize());

		return load(buffer);
	}

	bool Image::load(const std::vector<uint8_t>& data)
	{
		int32_t width{ 0 };
		int32_t height{ 0 };
		int32_t numChannels{ 0 };

		auto* image = stbi_load_from_memory(data.data(), (int)data.size(), &width, 
			&height, &numChannels, STBI_rgb_alpha);

		if (!image)
		{
			LogError("stbi_load_from_memory() failed!!");
			return false;
		}

		mWidth = (uint32_t)width;
		mHeight = (uint32_t)height;
		mChannels = 4;

		mData.resize(mWidth * mHeight * mChannels);
		memcpy(mData.data(), image, mData.size());

		stbi_image_free(image);

		return true;
	}

	bool Image::load(uint32_t width, uint32_t height, uint32_t depth, uint32_t channels,
		ImageType type, const uint8_t* data)
	{
		mImageType = type;
		mWidth = width;
		mDepth = depth;
		mHeight = height;
		mChannels = channels;

		const uint32_t dataSize = mWidth * mHeight * mDepth * mChannels;
		mData.resize(dataSize);

		if (data)
		{
			memcpy(mData.data(), data, dataSize);
		}

		return true;
	}

	std::vector<Mipmap> Image::generateMipmaps() const
	{
		std::vector<Mipmap> mipmaps;
		mipmaps.push_back({
			.width = mWidth,
			.height = mHeight,
			.level = 0,
			.data = mData
		});

		uint32_t offset{ 0 };
		uint32_t nextWidth = std::max(1u, mWidth / 2);
		uint32_t nextHeight = std::max(1u, mHeight / 2);
		uint32_t nextSize = nextWidth * nextHeight * mChannels;

		while (true)
		{
			auto& prevMipmap = mipmaps.back();
			Mipmap nextMipmap = {
				.width = nextWidth,
				.height = nextHeight,
				.level = prevMipmap.level + 1,
				.data = std::vector<uint8_t>(nextSize)
			};

			auto& prevData = prevMipmap.data;
			auto& nextData = nextMipmap.data;

			stbir_resize_uint8(prevData.data(), prevMipmap.width, prevMipmap.height, 0,
				nextData.data(), nextMipmap.width, nextMipmap.height, 0, mChannels);

			mipmaps.push_back(std::move(nextMipmap));
			nextWidth = std::max(1u, nextWidth / 2);
			nextHeight = std::max(1u, nextWidth / 2);
			nextSize = nextWidth * nextHeight * mChannels;

			if (nextWidth == 1 && nextHeight == 1)
			{
				break;
			}
		}

		return mipmaps;
	}

	glm::vec4 Image::getPixel(uint32_t x, uint32_t y) const
	{
		const uint32_t ofs = mChannels * (y * mWidth + x);

		return {
			mChannels > 0 ? float(mData[ofs + 0]) / 255.0f : 0.0f,
			mChannels > 1 ? float(mData[ofs + 1]) / 255.0f : 0.0f,
			mChannels > 2 ? float(mData[ofs + 2]) / 255.0f : 0.0f,
			mChannels > 3 ? float(mData[ofs + 3]) / 255.0f : 0.0f
		};

		return glm::vec4(0.0f);
	}

	uint32_t Image::getPixelAsRGBA(uint32_t x, uint32_t y) const
	{
		const uint32_t ofs = mChannels * (y * mWidth + x);
		uint32_t result = 0;

		result |= (mChannels > 0) ? mData[ofs + 0] << 0 : 0;
		result |= (mChannels > 1) ? mData[ofs + 1] << 8 : 0;
		result |= (mChannels > 2) ? mData[ofs + 2] << 16 : 0;
		result |= (mChannels > 3) ? mData[ofs + 3] << 24 : 0;

		return result;
	}

	void Image::setPixel(uint32_t x, uint32_t y, const glm::vec4& c)
	{
		const uint32_t ofs = mChannels * (y * mWidth + x);

		if (mChannels > 0) mData[ofs + 0] = uint8_t(c.x * 255.0f);
		if (mChannels > 1) mData[ofs + 1] = uint8_t(c.y * 255.0f);
		if (mChannels > 2) mData[ofs + 2] = uint8_t(c.z * 255.0f);
		if (mChannels > 3) mData[ofs + 3] = uint8_t(c.w * 255.0f);
	}

	void Image::convertToCube()
	{
		mImageType = ImageType::Cube;

		if (mWidth == 2 * mHeight)
		{
			convertToVerticalCross();
		}

		convertToCubeMapFaces();
	}

	void Image::convertToVerticalCross()
	{
		const uint32_t faceSize = mWidth / 4;
		const uint32_t w = faceSize * 3;
		const uint32_t h = faceSize * 4;

		const glm::ivec2 faceOffsets[] =
		{
			glm::ivec2(faceSize, faceSize * 3),
			glm::ivec2(0, faceSize),
			glm::ivec2(faceSize, faceSize),
			glm::ivec2(faceSize * 2, faceSize),
			glm::ivec2(faceSize, 0),
			glm::ivec2(faceSize, faceSize * 2)
		};

		auto faceCoordsToXYZ = [](uint32_t i, uint32_t j, uint32_t faceId, uint32_t faceSize)
		{
			const float a = 2.0f * float(i) / faceSize;
			const float b = 2.0f * float(j) / faceSize;

			if (faceId == 0) return glm::vec3(-1.0f, a - 1.0f, b - 1.0f);
			if (faceId == 1) return glm::vec3(a - 1.0f, -1.0f, 1.0f - b);
			if (faceId == 2) return glm::vec3(1.0f, a - 1.0f, 1.0f - b);
			if (faceId == 3) return glm::vec3(1.0f - a, 1.0f, 1.0f - b);
			if (faceId == 4) return glm::vec3(b - 1.0f, a - 1.0f, 1.0f);
			if (faceId == 5) return glm::vec3(1.0f - b, a - 1.0f, -1.0f);

			return glm::vec3();
		};

		Image result;
		result.load(w, h, 1, mChannels, mImageType);

		const uint32_t clampW = mWidth - 1;
		const uint32_t clampH = mHeight - 1;

		for (uint32_t face = 0; face < 6; face++)
		{
			for (uint32_t i = 0; i != faceSize; i++)
			{
				for (uint32_t j = 0; j != faceSize; j++)
				{
					const glm::vec3 p = faceCoordsToXYZ(i, j, face, faceSize);
					const float r = std::hypot(p.x, p.y);
					const float theta = std::atan2(p.y, p.x);
					const float phi = std::atan2(p.z, r);

					const float uf = float(2.0f * faceSize * (theta + M_PI) / M_PI);
					const float vf = float(2.0f * faceSize * (M_PI / 2.0f - phi) / M_PI);

					const uint32_t u1 = std::clamp(uint32_t(std::floor(uf)), uint32_t(0), clampW);
					const uint32_t v1 = std::clamp(uint32_t(std::floor(vf)), uint32_t(0), clampH);
					const uint32_t u2 = std::clamp(u1 + 1, uint32_t(0), clampW);
					const uint32_t v2 = std::clamp(v1 + 1, uint32_t(0), clampH);

					const float s = uf - u1;
					const float t = vf - v1;

					const glm::vec4 a = getPixel(u1, v1);
					const glm::vec4 b = getPixel(u2, v1);
					const glm::vec4 c = getPixel(u1, v2);
					const glm::vec4 d = getPixel(u2, v2);

					const glm::vec4 color = a * (1 - s) * (1 - t) + b * (s) * (1 - t) +
						c * (1 - s) * t + d * (s) * (t);

					result.setPixel(i + faceOffsets[face].x, j + faceOffsets[face].y, color);
				}
			}
		}

		load(w, h, 1, mChannels, mImageType, result.getData().data());
	}

	void Image::convertToCubeMapFaces()
	{
		const bool isHorizontalCross = mWidth > mHeight;
		const uint32_t faceWidth = isHorizontalCross ? mWidth / 4 : mWidth / 3;
		const uint32_t faceHeight = isHorizontalCross ? mHeight / 3 : mHeight / 4;

		Image result;
		result.load(faceWidth, faceHeight, 6, mChannels, ImageType::Cube);

		const uint8_t* src = mData.data();
		uint8_t* dst = result.mData.data();

		for (uint32_t face = 0; face < 6; face++)
		{
			for (uint32_t j = 0; j != faceHeight; j++)
			{
				for (uint32_t i = 0; i != faceWidth; i++)
				{
					uint32_t x = 0;
					uint32_t y = 0;

					switch (face)
					{
					case 0:
						x = i;
						y = faceHeight + j;
						break;

					case 1:
						x = 2 * faceWidth + i;
						y = 1 * faceHeight + j;
						break;

					case 2:
						x = 2 * faceWidth - (i + 1);
						y = 1 * faceHeight - (j + 1);
						break;

					case 3:
						x = 2 * faceWidth - (i + 1);
						y = 3 * faceHeight - (j + 1);
						break;

					case 4:
						x = isHorizontalCross ? 3 * faceWidth + i : 2 * faceWidth - (i + 1);
						y = isHorizontalCross ? faceHeight + j : mHeight - (j + 1);
						break;

					case 5:
						x = faceWidth + i;
						y = faceHeight + j;
						break;
					}

					memcpy(dst, src + (y * mWidth + x) * mChannels, mChannels);
					dst += mChannels;
				}
			}
		}

		load(faceWidth, faceHeight, 6, mChannels, mImageType, result.getData().data());
	}

	bool Image::read(FileReader& reader, ResourceCache& cache)
	{
		std::vector<uint8_t> buffer(reader.getSize());
		reader.read(buffer.data(), reader.getSize());

		return load(buffer);
	}

	bool Image::write(FileWriter& writer)
	{
		auto writePng = [](void* context, void* data, int len) {
			FileWriter* writer = (FileWriter*)context;

			if (writer)
			{
				writer->write((uint8_t*)data, len);
			}
		};

		if (!stbi_write_png_to_func(writePng, &writer, mWidth, mHeight, mChannels,
			mData.data(), mWidth * mChannels))
		{
			LogError("stbi_write_png_to_func() failed for: %s!!", mFileName.c_str());
			return false;
		}

		return true;
	}

}