#include "VFS/FileSystem.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"

namespace Trinity
{
	FileSystem::~FileSystem()
	{
	}

	std::string FileSystem::getFileName(const std::string& filePath) const
	{
		return fs::path(filePath).filename().string();
	}

	std::string FileSystem::getDirectory(const std::string& filePath) const
	{
		fs::path dir(filePath);
		dir.remove_filename();

		return dir.string();
	}

	std::string FileSystem::combinePath(const std::string& pathA, const std::string& pathB) const
	{
		fs::path fullPath(pathA);
		fullPath /= pathB;

		return fullPath.generic_string();
	}

	std::string FileSystem::relativePath(const std::string& path, const std::string& basePath) const
	{
		fs::path relPath = fs::relative(path, basePath);
		return relPath.string();
	}

	std::string FileSystem::sanitizePath(const std::string& path) const
	{
		return fs::path(path).generic_string();
	}

	bool FileSystem::isExist(const std::string& filePath) const
	{
		for (auto& it : mStorages)
		{
			std::string alias = it.first;
			std::string path = filePath;

			if (path.starts_with(alias))
			{
				return it.second->isExist(filePath);
			}
		}

		return false;
	}

	bool FileSystem::isDirectory(const std::string& filePath) const
	{
		for (auto& it : mStorages)
		{
			std::string alias = it.first;
			std::string path = filePath;

			if (path.starts_with(alias))
			{
				return it.second->isDirectory(filePath);
			}
		}

		return false;
	}

	bool FileSystem::getFiles(const std::string& dir, bool recurse, std::vector<FileEntry>& files) const
	{
		for (auto& it : mStorages)
		{
			std::string alias = it.first;
			std::string path = dir;

			if (path.starts_with(alias))
			{
				return it.second->getFiles(dir, recurse, files);
			}
		}

		return false;
	}

	bool FileSystem::addFolder(const std::string& alias, const std::string& path)
	{
		auto folder = std::make_unique<Folder>();
		if (!folder->create(alias, path))
		{
			LogError("Folder::create() failed!!");
			return false;
		}

		mStorages.insert({ alias, std::move(folder) });
		return true;
	}

	bool FileSystem::createDirs(const std::string& dir) const
	{
		for (auto& it : mStorages)
		{
			std::string alias = it.first;
			std::string path = dir;

			if (path.starts_with(alias))
			{
				return it.second->createDir(dir);
			}
		}

		return false;
	}

	bool FileSystem::copyFile(const std::string& from, const std::string& to) const
	{
		for (auto& it : mStorages)
		{
			std::string alias = it.first;
			std::string path = from;

			if (path.starts_with(alias))
			{
				return it.second->copyFile(from, to);
			}
		}

		return false;
	}

	bool FileSystem::copyFiles(const std::string& from, const std::string& to) const
	{
		for (auto& it : mStorages)
		{
			std::string alias = it.first;
			std::string path = from;

			if (path.starts_with(alias))
			{
				return it.second->copyFiles(from, to);
			}
		}

		return false;
	}

	std::unique_ptr<File> FileSystem::openFile(const std::string& filePath, FileOpenMode openMode)
	{
		for (auto& it : mStorages)
		{
			std::string alias = it.first;
			std::string path = filePath;

			if (path.starts_with(alias))
			{
				return it.second->openFile(filePath, openMode);
			}
		}

		return nullptr;
	}
}