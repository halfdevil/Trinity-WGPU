#include "Scene/ComponentFactory.h"
#include "Scene/Components/Light.h"
#include "Scene/Components/Mesh.h"
#include "Scene/Components/PerspectiveCamera.h"

namespace Trinity
{
	void ComponentFactory::registerCreator(size_t type, ComponentCreator creator)
	{
		mCreators.insert(std::make_pair(type, std::move(creator)));
	}

	void ComponentFactory::removeCreator(size_t type)
	{
		if (auto it = mCreators.find(type); it != mCreators.end())
		{
			mCreators.erase(it);
		}
	}

	std::unique_ptr<Component> ComponentFactory::createComponent(size_t type)
	{
		if (auto it = mCreators.find(type); it != mCreators.end())
		{
			auto& creator = it->second;
			return creator();
		}

		return nullptr;
	}
}