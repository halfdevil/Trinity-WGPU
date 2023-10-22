#pragma once

#include "VFS/File.h"
#include <vector>
#include <unordered_map>

namespace Trinity
{
	class FileReader
	{
	public:

		FileReader(File& file)
			: mFile(file)
		{
		}

		const File& getFile() const
		{
			return mFile;
		}

		uint32_t getSize() const
		{
			return mFile.getSize();
		}

		uint32_t getPosition() const
		{
			return mFile.getPosition();
		}

		const std::string& getPath() const
		{
			return mFile.getPath();
		}

		std::string getDirectory() const
		{
			return mFile.getDirectory();
		}

		bool isEOF() const
		{
			return mFile.isEOF();
		}

		template <typename T>
		bool read(T* data, uint32_t count = 1, uint32_t* readSize = nullptr)
		{
			return mFile.read(data, sizeof(T) * count, readSize);
		}

		template <typename T>
		bool readVector(std::vector<T>& v)
		{
			uint32_t count{ 0 };
			read(&count);

			v.resize(count);
			read(v.data(), count);

			return true;
		}

		template <typename K, typename V>
		bool readMap(std::unordered_map<K, V>& mp)
		{
			uint32_t count{ 0 };
			read(&count);

			std::vector<K> keys(count);
			std::vector<V> values(count);

			read(keys.data(), count);
			read(values.data(), count);

			for (uint32_t idx = 0; idx < count; idx++)
			{
				mp.insert(std::make_pair(keys[idx], values[idx]));
			}

			return true;
		}

		std::string readAsString();
		std::string readString();

		bool seek(SeekOrigin origin, uint32_t offset);

	private:

		File& mFile;
	};
}