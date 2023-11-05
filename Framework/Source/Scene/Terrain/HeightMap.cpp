#include "Scene/Terrain/HeightMap.h"
#include "Scene/Terrain/Terrain.h"
#include "Graphics/Texture2D.h"
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

	bool HeightMap::load(const std::string& fileName)
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
			mData[idx] = (uint16_t)(image[idx] * 257.0f);
		}

		stbi_image_free(image);
		mSize = { width, height };
		
		return true;
	}

	bool HeightMap::load(const std::string& fileName, uint32_t width, uint32_t height)
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
			mData[idx] = (uint16_t)(buffer[idx] * 257.0f);
		}

		mSize = { width, height };
		return true;
	}

	uint16_t HeightMap::getHeight(uint32_t x, uint32_t z) const
	{
		return mData[z * mSize.x + x];
	}

	void HeightMap::getMinMaxHeight(uint32_t x, uint32_t z, uint32_t sizeX, uint32_t sizeZ,
		uint16_t& minY, uint16_t& maxY) const
	{
		minY = 65535;
		maxY = 0;

		for (uint32_t rz = 0; rz < sizeZ; rz++)
		{
			auto* scanLine = &mData[x + (rz + z) * mSize.x];
			for (uint32_t rx = 0; rx < sizeX; rx++)
			{
				minY = std::min(minY, scanLine[rx]);
				maxY = std::max(maxY, scanLine[rz]);
			}
		}
	}

	void HeightMap::setSize(const glm::uvec2& size)
	{
		mSize = size;
	}

	void HeightMap::setData(std::vector<uint16_t>&& data)
	{
		mData = std::move(data);
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

		reader.read(&mSize);
		reader.readVector(mData);
		
		return true;
	}

	bool HeightMap::write(FileWriter& writer)
	{
		if (!Resource::write(writer))
		{
			return false;
		}

		writer.write(&mSize);
		writer.writeVector(mData);

		return true;
	}
}