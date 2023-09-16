#include "VFS/FileWriter.h"

namespace Trinity
{
	bool FileWriter::writeString(const std::string& str)
	{
		uint32_t len = (uint32_t)str.length();

		if (!write(&len))
		{
			return false;
		}

		if (len > 0)
		{
			return write(str.c_str(), len);
		}

		return true;
	}

	bool FileWriter::seek(SeekOrigin origin, uint32_t offset)
	{
		return mFile.seek(origin, offset);
	}
}