#pragma once

#include "Core/Singleton.h"
#include "Scene/Component.h"
#include <functional>
#include <memory>

namespace Trinity
{
	class ComponentFactory : public Singleton<ComponentFactory>
	{
	public:

		using ComponentCreator = std::function<std::unique_ptr<Component>()>;

		ComponentCreator& getCreator(size_t type);
		void registerCreator(size_t type, ComponentCreator creator);
		void removeCreator(size_t type);

		std::unique_ptr<Component> createComponent(size_t type);

	public:

		template <typename T>
		inline void registerCreator()
		{
			registerCreator(typeid(T).hash_code(), []() {
				return std::make_unique<T>();
			});
		}

		template <typename T>
		inline T& getCreator()
		{
			return dynamic_cast<T&>(getCreator(typeid(T).hash_code()));
		}

	protected:

		std::unordered_map<size_t, ComponentCreator> mCreators;
	};
}