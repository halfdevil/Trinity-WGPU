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

		ComponentCreator& getCreator(const std::string& type);
		void registerCreator(const std::string& type, ComponentCreator creator);
		void removeCreator(const std::string& type);

		std::unique_ptr<Component> createComponent(const std::string& type);

	public:

		template <typename T>
		inline void registerCreator()
		{
			registerCreator(T::getStaticType(), []() {
				return std::make_unique<T>();
			});
		}

		template <typename T>
		inline T& getCreator()
		{
			return dynamic_cast<T&>(T::getStaticType());
		}

	protected:

		std::unordered_map<std::string, ComponentCreator> mCreators;
	};
}