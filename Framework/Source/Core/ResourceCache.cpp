#include "Core/ResourceCache.h"
#include "Core/Resource.h"

namespace Trinity
{
	bool ResourceCache::hasResource(const std::type_index& type) const
	{
		auto it = mResources.find(type);
		return (it != mResources.end() && !it->second.empty());
	}

	bool ResourceCache::isLoaded(const std::type_index& type, const std::string& fileName) const
	{
		auto it = mFileResourceMap.find(type);
		if (it != mFileResourceMap.end())
		{
			auto it2 = it->second.find(fileName);
			return it2 != it->second.end();
		}

		return false;
	}

	Resource* ResourceCache::getResource(const std::type_index& type, uint32_t id) const
	{
		const auto& resources = mResources.at(type);
		if (id < (uint32_t)resources.size())
		{
			return resources[id].get();
		}

		return nullptr;
	}

	Resource* ResourceCache::getResource(const std::type_index& type, const std::string& fileName) const
	{
		auto it = mFileResourceMap.find(type);
		if (it != mFileResourceMap.end())
		{
			auto it2 = it->second.find(fileName);
			if (it2 != it->second.end())
			{
				return it2->second;
			}
		}

		return nullptr;
	}

	const std::vector<std::unique_ptr<Resource>>& ResourceCache::getResources(const std::type_index& type) const
	{
		return mResources.at(type);
	}

	void ResourceCache::addResource(std::unique_ptr<Resource> resource)
	{
		if (!resource->getFileName().empty())
		{
			auto& resources = mFileResourceMap[resource->getType()];
			resources.insert(std::make_pair(resource->getFileName(), resource.get()));
		}

		auto& resources = mResources[resource->getType()];
		resources.push_back(std::move(resource));
	}

	void ResourceCache::setResources(const std::type_index& type, std::vector<std::unique_ptr<Resource>> resources)
	{
		mResources[type] = std::move(resources);
	}

	void ResourceCache::clear()
	{
		mResources.clear();
	}
}