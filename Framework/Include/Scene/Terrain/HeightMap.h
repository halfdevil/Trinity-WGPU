#pragma once

#include "Core/Resource.h"
#include <vector>
#include <string>

namespace Trinity
{
	class HeightMap : public Resource
	{
	public:

		HeightMap() = default;
		virtual ~HeightMap() = default;

		HeightMap(const HeightMap&) = delete;
		HeightMap& operator = (const HeightMap&) = delete;

		HeightMap(HeightMap&&) = default;
		HeightMap& operator = (HeightMap&&) = default;

		uint32_t getWidth() const
		{
			return mWidth;
		}

		uint32_t getHeight() const
		{
			return mHeight;
		}

		const std::vector<float>& getData() const
		{
			return mData;
		}

		virtual bool create(const std::string& fileName, ResourceCache& cache, bool loadContent = true) override;
		virtual void destroy() override;
		virtual bool write() override;

		virtual bool load(const std::string& fileName, float heightScale);
		virtual bool load(const std::string& fileName, uint32_t width, uint32_t height, float heightScale);
		virtual void smooth();

		virtual std::type_index getType() const override;

	protected:

		virtual bool read(FileReader& reader, ResourceCache& cache) override;
		virtual bool write(FileWriter& writer) override;

	protected:

		uint32_t mWidth{ 0 };
		uint32_t mHeight{ 0 };
		std::vector<float> mData;
	};
}