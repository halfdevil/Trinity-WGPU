#include "Scene/Components/Light.h"

namespace Trinity
{
	std::type_index Light::getType() const
	{
		return typeid(Light);
	}

	void Light::setNode(Node& node)
	{
		mNode = &node;
	}

	void Light::setLightType(LightType lightType)
	{
		mLightType = lightType;
	}

	void Light::setLightProperties(const LightProperties& properties)
	{
		mProperties = properties;
	}
}