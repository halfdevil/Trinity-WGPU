#pragma once

#include "Core/Resource.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace Trinity
{
	class Texture2D;
	struct MapDimension;

	class HeightMap : public Resource
	{
	public:

		HeightMap() = default;
		virtual ~HeightMap() = default;

		HeightMap(const HeightMap&) = delete;
		HeightMap& operator = (const HeightMap&) = delete;

		HeightMap(HeightMap&&) = default;
		HeightMap& operator = (HeightMap&&) = default;

		const glm::uvec2& getSize() const
		{
			return mSize;
		}

		const std::vector<uint16_t>& getData() const
		{
			return mData;
		}

		virtual bool create(const std::string& fileName, ResourceCache& cache, bool loadContent = true) override;
		virtual void destroy() override;
		virtual bool write() override;

		virtual bool load(const std::string& fileName);
		virtual bool load(const std::string& fileName, uint32_t width, uint32_t height);

		virtual uint16_t getHeight(uint32_t x, uint32_t z) const;
		virtual void getMinMaxHeight(uint32_t x, uint32_t z, uint32_t sizeX, uint32_t sizeZ,
			uint16_t& minZ, uint16_t& maxZ) const;

		virtual void setSize(const glm::uvec2& size);
		virtual void setData(std::vector<uint16_t>&& data);

		virtual std::type_index getType() const override;

	protected:

		virtual bool read(FileReader& reader, ResourceCache& cache) override;
		virtual bool write(FileWriter& writer) override;

	protected:

		glm::uvec2 mSize{ 0 };
		std::vector<uint16_t> mData;
	};
}