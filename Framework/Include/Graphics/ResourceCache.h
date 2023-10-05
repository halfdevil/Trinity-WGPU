#pragma once

#include "Graphics/Resource.h"
#include <algorithm>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Trinity
{
	class ResourceCache
	{
	public:

		ResourceCache() = default;
		virtual ~ResourceCache() = default;

		ResourceCache(const ResourceCache&) = delete;
		ResourceCache& operator = (const ResourceCache&) = delete;

		ResourceCache(ResourceCache&&) = default;
		ResourceCache& operator = (ResourceCache&&) = default;

		bool hasResource(const std::type_index& type) const;
		const std::vector<std::unique_ptr<Resource>>& getResources(const std::type_index& type) const;

		virtual void addResource(std::unique_ptr<Resource> resource);
		virtual void setResources(const std::type_index& type, std::vector<std::unique_ptr<Resource>> resources);

	public:

		template <typename T>
		void setResources(std::vector<std::unique_ptr<T>> resources)
		{
			std::vector<std::unique_ptr<Resource>> result(resources.size());
			std::transform(resources.begin(), resources.end(), result.begin(),
				[](std::unique_ptr<T>& resource) -> std::unique_ptr<Resource> {
					return std::unique_ptr<Resource>(std::move(resource));
				}
			);

			setResources(typeid(T), std::move(result));
		}

		template <typename T>
		void clearResources()
		{
			setResources(typeid(T), {});
		}

		template <typename T>
		std::vector<T*> getResources() const
		{
			std::vector<T*> result;

			if (hasResource(typeid(T)))
			{
				auto& resources = getResources(typeid(T));

				result.resize(resources.size());
				std::transform(resources.begin(), resources.end(), result.begin(),
					[](const std::unique_ptr<Resource>& resource) -> T* {
						return dynamic_cast<T*>(resource.get());
					}
				);
			}

			return result;
		}

		template <typename T>
		bool hasResource() const
		{
			return hasResource(typeid(T));
		}

	protected:

		std::unordered_map<std::type_index, std::vector<std::unique_ptr<Resource>>> mResources;
	};
}