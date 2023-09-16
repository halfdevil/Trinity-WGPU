#pragma once

#include "VFS/Storage.h"
#include <memory>

namespace Trinity
{
	class Folder : public Storage
	{
	public:

		Folder() = default;
		~Folder();

		Folder(const Folder&) = delete;
		Folder& operator = (const Folder&) = delete;

		Folder(Folder&&) = delete;
		Folder& operator = (Folder&&) = delete;

		bool create(const std::string& alias, const std::string& path);
		void destroy();

		virtual bool isExist(const std::string& filePath) const override;
		virtual bool isDirectory(const std::string& filePath) const override;

		virtual bool getFiles(const std::string& dir, bool recurse,
			std::vector<FileEntry>& files) const override;

		virtual std::unique_ptr<File> openFile(const std::string& filePath,
			FileOpenMode openMode) override;

		virtual bool createDir(const std::string& dir) override;
		virtual bool copyFile(const std::string& from, const std::string& to) override;
		virtual bool copyFiles(const std::string& from, const std::string& to) override;

	private:

		std::string getActualPath(const std::string& virtualPath) const;
		std::string getVirtualPath(const std::string& actualPath) const;

	private:

		std::string mPath;
	};
}