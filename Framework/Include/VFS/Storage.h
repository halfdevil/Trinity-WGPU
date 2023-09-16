#pragma once

#include "VFS/File.h"
#include <memory>
#include <vector>

namespace Trinity
{
	struct FileEntry
	{
		std::string name;
		std::string path;
		bool directory{ false };
	};

	class Storage
	{
	public:

		Storage() = default;
		virtual ~Storage() = default;

		const std::string& getAlias() const
		{
			return mAlias;
		}

		virtual bool isExist(const std::string& filePath) const = 0;
		virtual bool isDirectory(const std::string& filePath) const = 0;

		virtual bool getFiles(const std::string& dir, bool recurse,
			std::vector<FileEntry>& files) const = 0;

		virtual std::unique_ptr<File> openFile(const std::string& filePath,
			FileOpenMode openMode) = 0;

		virtual bool createDir(const std::string& dir) = 0;
		virtual bool copyFile(const std::string& from, const std::string& to) = 0;
		virtual bool copyFiles(const std::string& from, const std::string& to) = 0;

	protected:

		std::string mAlias;
	};
}