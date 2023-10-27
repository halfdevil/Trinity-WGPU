#pragma once

#include <memory>
#include <string>
#include <typeindex>
#include <vector>

namespace Trinity
{
	class ResourceCache;
	class FileReader;
	class FileWriter;

	class Resource
	{
	public:

		Resource() = default;
		virtual ~Resource();

		Resource(const Resource&) = delete;
		Resource& operator = (const Resource&) = delete;

		Resource(Resource&&) = default;
		Resource& operator = (Resource&&) = default;

		const std::string& getName() const
		{
			return mName;
		}

		const std::string& getFileName() const
		{
			return mFileName;
		}

		virtual void setName(const std::string& name);
		virtual void setFileName(const std::string& fileName);
		virtual std::type_index getType() const = 0;

		virtual bool create(const std::string& fileName, ResourceCache& cache, bool loadContent = true);
		virtual void destroy();
		virtual bool write();

	protected:

		virtual bool read(FileReader& reader, ResourceCache& cache);
		virtual bool write(FileWriter& writer);

	protected:

		std::string mName;
		std::string mFileName;
	};
}