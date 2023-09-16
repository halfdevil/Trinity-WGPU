#include "VFS/DiskFile.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"

namespace Trinity
{
	DiskFile::~DiskFile()
	{
		close();
	}

	bool DiskFile::create(const std::string& filePath, const std::string& actualPath,
		FileOpenMode fileOpenMode)
	{
		std::ios_base::openmode fileMode = std::ios::binary;

		switch (fileOpenMode)
		{
		case FileOpenMode::OpenRead:
			fileMode = std::ios::in | fileMode;
			break;

		case FileOpenMode::OpenWrite:
			fileMode = std::ios::out | fileMode;
			break;

		case FileOpenMode::Append:
			fileMode = std::ios::app | fileMode;
			break;

		default:
			break;
		}

		mFile.open(actualPath, fileMode);
		if (!mFile.good())
		{
			LogError("Unable to open file: %s", actualPath.c_str());
			return false;
		}

		mFile.seekg(0, mFile.end);
		mSize = (uint32_t)mFile.tellg();
		mFile.seekg(0, mFile.beg);

		mOpenMode = fileOpenMode;
		mPath = filePath;
		mActualPath = actualPath;

		return true;
	}

	void DiskFile::close()
	{
		if (mFile.is_open())
		{
			mFile.close();
		}
	}

	bool DiskFile::isEOF() const
	{
		return mFile.eof() || !mFile.good();
	}

	bool DiskFile::seek(SeekOrigin origin, int32_t offset)
	{
		switch (origin)
		{
		case SeekOrigin::Beginning:
			mFile.seekg(offset, mFile.beg);
			break;

		case SeekOrigin::Current:
			mFile.seekg(offset, mFile.cur);
			break;

		case SeekOrigin::End:
			mFile.seekg(offset, mFile.end);
			break;

		default:
			break;
		}

		mPosition = (uint32_t)mFile.tellg();
		return true;
	}

	bool DiskFile::read(void* data, uint32_t size, uint32_t* readSize)
	{
		if (!canRead())
		{
			LogError("File not opened for reading: %s", mPath.c_str());
			return false;
		}

		if (mFile.eof())
		{
			if (readSize)
			{
				*readSize = 0;
			}

			return true;
		}

		mFile.read((char*)data, size);

		if (readSize)
		{
			*readSize = (uint32_t)mFile.gcount();
		}

		if (!mFile.good() && !mFile.eof())
		{
			LogError("Error reading from file: %s", mPath.c_str());
			return false;
		}

		mPosition = (uint32_t)mFile.tellg();
		return true;
	}

	bool DiskFile::write(const void* data, uint32_t size, uint32_t* writeSize)
	{
		if (!canWrite())
		{
			LogError("File not opened for writing: %s", mPath.c_str());
			return false;
		}

		mFile.write(reinterpret_cast<const char*>(data), size);

		if (writeSize)
		{
			*writeSize = (uint32_t)mFile.gcount();
		}

		if (!mFile.good())
		{
			LogError("Error writing to file: %s", mPath.c_str());
			return false;
		}

		mPosition = (uint32_t)mFile.tellg();
		return true;
	}
}