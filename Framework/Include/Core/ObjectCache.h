#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <optional>

namespace Trinity
{
	template <typename Key, typename Type, int MaxCacheSize>
	class ObjectCache
	{
	public:

		struct CachedObject
		{
			std::shared_ptr<Type> object;
			Key key;
			uint32_t cacheHit{ 0 };
		};

		ObjectCache() = default;
		~ObjectCache()
		{
			destroy();
		}

		ObjectCache(const ObjectCache&) = delete;
		ObjectCache& operator = (const ObjectCache&) = delete;

		ObjectCache(ObjectCache&&) = default;
		ObjectCache& operator = (ObjectCache&&) = default;

		void destroy()
		{
			mCache.clear();
			mCacheArray.clear();
		}

		bool exists(const Key& key)
		{
			return mCache.find(key) != mCache.end();
		}

		std::weak_ptr<Type> get(const Key& key) const
		{
			auto it = mCache.find(key);
			if (it != mCache.end())
			{
				return mCacheArray[it->second].object;
			}

			return {};
		}

		template <typename V>
		std::weak_ptr<V> getAs(const Key& key) const
		{
			auto it = mCache.find(key);
			if (it != mCache.end())
			{
				return std::dynamic_pointer_cast<V>(mCacheArray[it->second].object);
			}

			return {};
		}

		std::weak_ptr<Type> add(const Key& key, std::shared_ptr<Type> object)
		{
			auto it = mCache.find(key);
			if (it != mCache.end())
			{
				CachedObject& cachedObject = mCacheArray[it->second];
				cachedObject.cacheHit = mHitCounter++;

				return cachedObject.object;
			}

			if (mCacheArray.size() >= MaxCacheSize)
			{
				uint32_t oldCacheHit = mHitCounter;
				uint32_t oldCacheIdx = MaxCacheSize;
				std::string oldCacheKey;

				for (uint32_t idx = 0; idx < static_cast<uint32_t>(mCacheArray.size()); idx++)
				{
					CachedObject& cachedObject = mCacheArray[idx];
					if (cachedObject.cacheHit < oldCacheHit)
					{
						oldCacheHit = cachedObject.cacheHit;
						oldCacheKey = cachedObject.key;
						oldCacheIdx = idx;
					}
				}

				if (oldCacheIdx != MaxCacheSize)
				{
					CachedObject& cachedObject = mCacheArray[oldCacheIdx];
					cachedObject.object = std::move(object);
					cachedObject.cacheHit = mHitCounter++;
					cachedObject.key = key;

					mCache.erase(mCache.find(oldCacheKey));
					mCache.insert({ key, oldCacheIdx });

					return cachedObject.object;
				}
			}
			else
			{
				const uint32_t cacheIdx = static_cast<uint32_t>(mCacheArray.size());
				mCacheArray.push_back({
					.object = std::move(object),
					.key = key,
					.cacheHit = mHitCounter++
				});

				mCache.insert({ key, cacheIdx });

				CachedObject& cachedObject = mCacheArray[cacheIdx];
				return cachedObject.object;
			}

			return {};
		}

		void remove(const Key& key)
		{
			auto it = mCache.find(key);
			if (it != mCache.end())
			{
				mCacheArray.erase(mCacheArray.begin() + it->second);
				mCache.erase(it);
			}
		}

		void clear()
		{
			mCacheArray.clear();
			mCache.clear();
			mHitCounter = 0;
		}

	private:

		uint32_t mHitCounter{ 0 };
		Allocator mAllocator;
		std::unordered_map<Key, uint32_t> mCache;
		std::vector<CachedObject> mCacheArray;
	};
}