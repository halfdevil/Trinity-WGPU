#include "Core/Resource.h"

namespace Trinity
{
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

	bool Resource::create(const std::string& fileName, ResourceCache& cache)
	{
		return false;
	}

	bool Resource::write()
	{
		return false;
	}
}