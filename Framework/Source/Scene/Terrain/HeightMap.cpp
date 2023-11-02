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

	uint16_t HeightMap::getHeight(uint32_t x, uint32_t z) const
	{
		return mData[y * mSize.x + x];
	}

	glm::uvec2 HeightMap::getMinMaxHeight(uint32_t x, uint32_t z, uint32_t sizeX, uint32_t sizeZ)
	{
		glm::uvec2 height{ 65535, 0 };

		for (uint32_t rz = 0; rz < sizeZ; rz++)
		{
			auto* scanLine = &mData[x + (rz + z) * mSize.x];
			for (uint32_t rx = 0; rx < sizeX; rx++)
			{
				height.x = std::min(height.x, scanLine[rx]);
				height.z = std::max(height.y, scanLine[rz]);
			}
		}

		return height;
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