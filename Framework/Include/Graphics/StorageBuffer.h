#pragma once

#include "Graphics/Buffer.h"

namespace Trinity
{
	class StorageBuffer : public Buffer
	{
	public:

		StorageBuffer() = default;
		~StorageBuffer();

		StorageBuffer(const StorageBuffer&) = default;
		StorageBuffer& operator = (const StorageBuffer&) = default;

		StorageBuffer(StorageBuffer&&) = default;
		StorageBuffer& operator = (StorageBuffer&&) = default;

		uint32_t getSize() const
		{
			return mSize;
		}

		bool create(uint32_t size, const void* data = nullptr);
		void destroy();

	private:

		uint32_t mSize{ 0 };
	};
}