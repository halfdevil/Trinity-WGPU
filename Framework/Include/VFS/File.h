#pragma once

#include <cstdint>
#include <string>

#include <filesystem>
namespace fs = std::filesystem;

namespace Trinity
{
	enum class FileOpenMode
	{
		OpenRead,
		OpenWrite,
		Append
	};

	enum class SeekOrigin
	{
		Beginning,
		Current,
		End
	};

	class File
	{
	public:

		File() = default;
		virtual ~File() = default;

		FileOpenMode getOpenMode() const
		{
			return mOpenMode;
		}

		bool canRead() const
		{
			return mOpenMode == FileOpenMode::OpenRead;
		}

		bool canWrite() const
		{
			return mOpenMode == FileOpenMode::OpenWrite;
		}

		uint32_t getSize() const
		{
			return mSize;
		}

		uint32_t getPosition() const
		{
			return mPosition;
		}

		const std::string& getPath() const
		{
			return mPath;
		}

		virtual bool isEOF() const = 0;
		virtual bool seek(SeekOrigin origin, int32_t offset) = 0;
		virtual bool read(void* data, uint32_t size, uint32_t* readSize = nullptr) = 0;
		virtual bool write(const void* data, uint32_t size, uint32_t* writeSize = nullptr) = 0;

	protected:

		FileOpenMode mOpenMode{ FileOpenMode::OpenRead };
		uint32_t mSize{ 0 };
		uint32_t mPosition{ 0 };
		std::string mPath;
	};
}