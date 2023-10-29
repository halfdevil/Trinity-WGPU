#include "Core/Resource.h"
#include "VFS/FileSystem.h"
#include "Core/ResourceCache.h"
#include "Core/Logger.h"

namespace Trinity
{
	Resource::~Resource()
	{
		destroy();
	}

	void Resource::setName(const std::string& name)
	{
		mName = name;
	}

	void Resource::setFileName(const std::string& fileName)
	{
		mFileName = fileName;
	}

	std::type_index Resource::getType() const
	{
		return typeid(Resource);
	}

	bool Resource::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		auto& fileSystem = FileSystem::get();
		mFileName = fileName;

		if (loadContent)
		{
			if (fileSystem.isExist(fileName))
			{
				auto file = fileSystem.openFile(fileName, FileOpenMode::OpenRead);
				if (!file)
				{
					LogError("Error opening resource file: %s", fileName.c_str());
					return false;
				}

				FileReader reader(*file);
				if (!read(reader, cache))
				{
					LogError("Resource::read() failed for: %s!!", fileName.c_str());
					return false;
				}
			}
			else
			{
				LogError("Resource file '%s' not found", fileName.c_str());
				return false;
			}
		}

		return true;
	}

	bool Resource::write()
	{
		if (mFileName.empty())
		{
			LogError("Cannot write to file as filename is empty!!");
			return false;
		}

		auto file = FileSystem::get().openFile(mFileName, FileOpenMode::OpenWrite);
		if (!file)
		{
			LogError("Error opening resource file: %s", mFileName.c_str());
			return false;
		}

		FileWriter writer(*file);
		if (!write(writer))
		{
			LogError("Resource::write() failed for: %s!!", mFileName.c_str());
			return false;
		}

		return true;
	}

	void Resource::destroy()
	{
	}

	bool Resource::read(FileReader& reader, ResourceCache& cache)
	{
		mName = reader.readString();
		return true;
	}

	bool Resource::write(FileWriter& writer)
	{
		writer.writeString(mName);
		return true;
	}

	std::string Resource::getReadPath(const std::string& basePath, const std::string& fileName)
	{
		if (fileName.starts_with("/Assets/Framework"))
		{
			return fileName;
		}

		auto& fileSystem = FileSystem::get();
		auto readPath = fileSystem.combinePath(basePath, fileName);
		readPath = fileSystem.canonicalPath(readPath);
		readPath = fileSystem.sanitizePath(readPath);

		return readPath;
	}

	std::string Resource::getWritePath(const std::string& basePath, const std::string& fileName)
	{
		if (fileName.starts_with("/Assets/Framework"))
		{
			return fileName;
		}

		auto& fileSystem = FileSystem::get();
		auto writePath = fileSystem.relativePath(fileName, basePath);
		writePath = fileSystem.sanitizePath(writePath);

		return writePath;
	}
}