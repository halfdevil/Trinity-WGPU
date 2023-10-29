#include "Scene/Terrain/HeightMap.h"
#include "VFS/FileSystem.h"
#include "Core/Logger.h"
#include <stb_image.h>

namespace Trinity
{
	bool HeightMap::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		return Resource::create(fileName, cache, loadContent);
	}

	void HeightMap::destroy()
	{
		Resource::destroy();
		mData.clear();
	}

	bool HeightMap::write()
	{
		return Resource::write();
	}

	bool HeightMap::load(const std::string& fileName, float heightScale)
	{
		auto file = FileSystem::get().openFile(fileName, FileOpenMode::OpenRead);
		if (!file)
		{
			LogError("FileSystem::openFile() failed for '%s'", fileName.c_str());
			return false;
		}

		FileReader reader(*file);
		std::vector<uint8_t> buffer(reader.getSize());		
		reader.read(buffer.data(), reader.getSize());

		int32_t width{ 0 };
		int32_t height{ 0 };

		auto* image = stbi_load_from_memory(buffer.data(), reader.getSize(), 
			&width, &height, nullptr, STBI_grey);

		if (!image)
		{
			LogError("stbi_load_from_memory() failed for '%s'", fileName.c_str());
			return false;
		}

		mData.resize(width * height);
		for (int32_t idx = 0; idx < width * height; idx++)
		{
			mData[idx] = (image[idx] / 255.0f) * heightScale;
		}

		stbi_image_free(image);

		mWidth = (uint32_t)width;
		mHeight = (uint32_t)height;

		return true;
	}

	bool HeightMap::load(const std::string& fileName, uint32_t width, uint32_t height, float heightScale)
	{
		auto file = FileSystem::get().openFile(fileName, FileOpenMode::OpenRead);
		if (!file)
		{
			LogError("FileSystem::openFile() failed for '%s'", fileName.c_str());
			return false;
		}

		FileReader reader(*file);
		std::vector<uint8_t> buffer(reader.getSize());

		reader.read(buffer.data(), reader.getSize());
		mData.resize(width * height);

		for (uint32_t idx = 0; idx < width * height; idx++)
		{
			mData[idx] = (buffer[idx] / 255.0f) * heightScale;
		}

		mWidth = width;
		mHeight = height;

		return true;
	}

	void HeightMap::smooth()
	{
		auto inBounds = [this](int32_t i, int32_t j)
		{
			return i >= 0 && i < (int32_t)mWidth &&
				j >= 0 && j < (int32_t)mWidth;
		};

		auto average = [this, &inBounds](int32_t i, int32_t j)
		{
			float avg = 0.0f;
			float num = 0.0f;

			for (int32_t m = i - 1; m <= i + 1; m++)
			{
				for (int32_t n = j - 1; n <= j + 1; n++)
				{
					if (inBounds(m, n))
					{
						avg += mData[m * mWidth + n];
						num += 1.0f;
					}
				}
			}

			return avg / num;
		};

		std::vector<float> dest(mData.size());
		for (uint32_t idx = 0; idx < mHeight; idx++)
		{
			for (uint32_t jdx = 0; jdx < mWidth; jdx++)
			{
				dest[idx * mWidth + jdx] = average(idx, jdx);
			}
		}

		mData = std::move(dest);
	}

	std::type_index HeightMap::getType() const
	{
		return typeid(HeightMap);
	}

	bool HeightMap::read(FileReader& reader, ResourceCache& cache)
	{
		if (!Resource::read(reader, cache))
		{
			return false;
		}

		reader.read(&mWidth);
		reader.read(&mHeight);
		reader.readVector(mData);
		
		return true;
	}

	bool HeightMap::write(FileWriter& writer)
	{
		if (!Resource::write(writer))
		{
			return false;
		}

		writer.write(&mWidth);
		writer.write(&mHeight);
		writer.writeVector(mData);

		return true;
	}
}