#include "Graphics/ResourceCache.h"
#include "Graphics/Resource.h"

namespace Trinity
{
	bool ResourceCache::hasResource(const std::type_index& type) const
	{
		auto it = mResources.find(type);
		return (it != mResources.end() && !it->second.empty());
	}

	const std::vector<std::unique_ptr<Resource>>& ResourceCache::getResources(const std::type_index& type) const
	{
		return mResources.at(type);
	}

	void ResourceCache::addResource(std::unique_ptr<Resource> resource)
	{
		mResources[resource->getType()].push_back(std::move(resource));
	}

	void ResourceCache::setResources(const std::type_index& type, std::vector<std::unique_ptr<Resource>> resources)
	{
		mResources[type] = std::move(resources);
	}
}