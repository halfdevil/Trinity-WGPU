#pragma once

#include <vector>
#include <string>

namespace Trinity
{
	class HeightMap
	{
	public:

		HeightMap() = default;
		virtual ~HeightMap();

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

		virtual bool create(const std::string& fileName, float heightScale);
		virtual bool create(const std::string& fileName, uint32_t width, uint32_t height, float heightScale);
		virtual void destroy();
		virtual void smooth();

	protected:

		uint32_t mWidth{ 0 };
		uint32_t mHeight{ 0 };
		std::vector<float> mData;
	};
}