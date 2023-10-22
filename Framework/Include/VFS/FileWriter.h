#pragma once

#include "VFS/File.h"
#include <vector>
#include <unordered_map>

namespace Trinity
{
	class FileWriter
	{
	public:

		FileWriter(File& file)
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

		template <typename T>
		bool write(const T* data, uint32_t count = 1, uint32_t* writeSize = nullptr)
		{
			return mFile.write(data, sizeof(T) * count, writeSize);
		}

		template <typename T>
		bool writeVector(const std::vector<T>& v)
		{
			uint32_t count = static_cast<uint32_t>(v.size());

			write(&count);
			write(v.data(), count);

			return true;
		}

		template <typename K, typename V>
		bool writeMap(const std::unordered_map<K, V>& mp)
		{
			uint32_t count = static_cast<uint32_t>(mp.size());
			write(&count);

			std::vector<K> keys;
			std::vector<V> values;

			keys.reserve(count);
			values.reserve(count);

			for (auto& pair : mp)
			{
				keys.push_back(pair.first);
				values.push_back(pair.second);
			}

			write(keys.data(), count);
			write(values.data(), count);

			return true;
		}

		bool writeString(const std::string& str);
		bool seek(SeekOrigin origin, uint32_t offset);

	private:

		File& mFile;
	};
}