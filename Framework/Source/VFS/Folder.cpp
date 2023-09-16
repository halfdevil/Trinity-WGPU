#include "VFS/Folder.h"
#include "VFS/DiskFile.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"

namespace Trinity
{
	Folder::~Folder()
	{
		destroy();
	}

	bool Folder::create(const std::string& alias, const std::string& path)
	{
		mAlias = alias;
		mPath = path;

		if (!fs::exists(mPath))
		{
			LogError("Path doesn't exists: %s", mPath.c_str());
			return false;
		}

		return true;
	}

	void Folder::destroy()
	{
		mAlias.clear();
		mPath.clear();
	}

	bool Folder::isExist(const std::string& filePath) const
	{
		return fs::exists(getActualPath(filePath));
	}

	bool Folder::isDirectory(const std::string& filePath) const
	{
		std::string actualPath = getActualPath(filePath);
		return fs::is_directory(actualPath);
	}

	bool Folder::getFiles(const std::string& dir, bool recurse, std::vector<FileEntry>& files) const
	{
		std::string actualDir = getActualPath(dir);
		if (!fs::is_directory(actualDir))
		{
			LogError("Folder::getFiles() failed, not a valid directory: %s", dir.c_str());
			return false;
		}

		if (recurse)
		{
			for (const auto& dirEntry : fs::recursive_directory_iterator(actualDir))
			{
				const fs::path& p = dirEntry.path();
				const std::string virtualPath = getVirtualPath(p.string());

				files.push_back({
					.name = fs::path(virtualPath).filename().string(),
					.path = virtualPath,
					.directory = dirEntry.is_directory()
				});
			}
		}
		else
		{
			for (const auto& dirEntry : fs::directory_iterator(actualDir))
			{
				const fs::path& p = dirEntry.path();
				const std::string virtualPath = getVirtualPath(p.string());

				files.push_back({
					.name = fs::path(virtualPath).filename().string(),
					.path = virtualPath,
					.directory = dirEntry.is_directory()
				});
			}
		}

		return true;
	}

	std::unique_ptr<File> Folder::openFile(const std::string& filePath, FileOpenMode openMode)
	{
		std::string actualPath = getActualPath(filePath);

		auto file = std::make_unique<DiskFile>();
		if (!file->create(filePath, actualPath, openMode))
		{
			LogError("DiskFile::create() failed for: %s!!", actualPath.c_str());
			return nullptr;
		}

		return file;
	}

	bool Folder::createDir(const std::string& dir)
	{
		std::string actualPath = getActualPath(dir);
		return fs::create_directories(actualPath);
	}

	bool Folder::copyFile(const std::string& from, const std::string& to)
	{
		std::string actualFrom = getActualPath(from);
		std::string actualTo = getActualPath(to);

		fs::copy_options options = fs::copy_options::update_existing |
			fs::copy_options::recursive;

		std::error_code ec;
		fs::copy_file(actualFrom, actualTo, options, ec);

		return ec.value() == 0;
	}

	bool Folder::copyFiles(const std::string& from, const std::string& to)
	{
		std::string actualFrom = getActualPath(from);
		std::string actualTo = getActualPath(to);

		fs::copy_options options = fs::copy_options::update_existing |
			fs::copy_options::recursive;

		std::error_code ec;
		fs::copy(actualFrom, actualTo, options, ec);

		return ec.value() == 0;
	}

	std::string Folder::getActualPath(const std::string& virtualPath) const
	{
		std::string alias = mAlias;
		std::string filePath = fs::path(virtualPath).generic_string();
		fs::path actualPath(mPath);

		if (filePath.starts_with(alias))
		{
			if (filePath != alias)
			{
				actualPath.append(filePath.substr(alias.length() + 1));
			}
		}
		else
		{
			actualPath.append(filePath);
		}

		return actualPath.make_preferred().string();
	}

	std::string Folder::getVirtualPath(const std::string& actualPath) const
	{
		std::string filePath = actualPath;
		std::string folderPath = mPath;
		fs::path virtualPath(mAlias);

		if (filePath.starts_with(folderPath))
		{
			if (filePath != folderPath)
			{
				virtualPath.append(filePath.substr(folderPath.length() + 1));
			}
		}
		else
		{
			virtualPath.append(filePath);
		}

		return virtualPath.generic_string();
	}
}