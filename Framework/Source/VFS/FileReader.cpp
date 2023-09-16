#include "VFS/FileReader.h"
#include <vector>
#include <memory>

namespace Trinity
{
	std::string FileReader::readAsString()
	{
		std::vector<char> buffer(mFile.getSize() + 1);
		read(buffer.data(), mFile.getSize());
		buffer[mFile.getSize()] = 0;

		return { buffer.data() };
	}

	std::string FileReader::readString()
	{
		uint32_t len{ 0 };
		read(&len);

		if (len == 0)
		{
			return {};
		}

		std::vector<char> stringBytes;
		stringBytes.resize(len + 1);
		stringBytes[len] = 0;

		read(stringBytes.data(), len);
		return std::string{ stringBytes.data() };
	}

	bool FileReader::seek(SeekOrigin origin, uint32_t offset)
	{
		return mFile.seek(origin, offset);
	}
}