#include "Scene/Components/Light.h"
#include "Scene/ComponentFactory.h"
#include "Scene/Scene.h"
#include "VFS/FileReader.h"
#include "VFS/FileWriter.h"

namespace Trinity
{
	std::type_index Light::getType() const
	{
		return typeid(Light);
	}

	std::string Light::getTypeStr() const
	{
		return getStaticType();
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

	bool Light::read(FileReader& reader, Scene& scene)
	{
		if (!Component::read(reader, scene))
		{
			return false;
		}

		uint32_t nodeId{ 0 };
		reader.read(&nodeId);
		mNode = scene.getNode(nodeId);

		reader.read(&mLightType);
		reader.read(&mProperties);

		return true;
	}

	bool Light::write(FileWriter& writer, Scene& scene)
	{
		if (!Component::write(writer, scene))
		{
			return false;
		}

		const uint32_t nodeId = mNode->getId();
		writer.write(&nodeId);
		
		writer.write(&mLightType);
		writer.write(&mProperties);

		return true;
	}

	std::string Light::getStaticType()
	{
		return "Light";
	}
}