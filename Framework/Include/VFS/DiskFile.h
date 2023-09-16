#pragma once

#include "VFS/File.h"
#include <fstream>

namespace Trinity
{
	class DiskFile : public File
	{
	public:

		DiskFile() = default;
		virtual ~DiskFile();

		DiskFile(const DiskFile&) = delete;
		DiskFile& operator = (const DiskFile&) = delete;

		DiskFile(DiskFile&&) = default;
		DiskFile& operator = (DiskFile&&) = default;

		const fs::path& getActualPath() const
		{
			return mActualPath;
		}

		virtual bool create(const std::string& filePath, const std::string& actualPath,
			FileOpenMode fileOpenMode);

		virtual void close();

		virtual bool isEOF() const override;
		virtual bool seek(SeekOrigin origin, int32_t offset) override;
		virtual bool read(void* data, uint32_t size, uint32_t* readSize = nullptr) override;
		virtual bool write(const void* data, uint32_t size, uint32_t* writeSize = nullptr) override;

	private:

		std::string mActualPath;
		std::fstream mFile;
	};
}