#pragma once

#include "Scene/Component.h"
#include "Math/Types.h"

namespace Trinity
{
	class Node;

	enum class LightType
	{
		Directional = 0,
		Point,
		Spot,
		MAX
	};

	struct LightProperties
	{
		glm::vec3 direction{ 0.0f, 0.0f, -1.0f };
		glm::vec3 color{ 1.0f, 1.0f, 1.0f };
		float intensity{ 1.0f };
		float range{ 0.0f };
		float innerCoreAngle{ 0.0f };
		float outerCoreAngle{ 0.0f };
	};

	class Light : public Component
	{
	public:

		Light() = default;
		virtual ~Light() = default;

		Light(const Light&) = delete;
		Light& operator = (const Light&) = delete;

		Light(Light&&) = default;
		Light& operator = (Light&&) = default;

		Node* getNode() const
		{
			return mNode;
		}

		LightType getLightType() const
		{
			return mLightType;
		}

		const LightProperties& getProperties() const
		{
			return mProperties;
		}

		virtual std::type_index getType() const override;

		virtual void setNode(Node& node);
		virtual void setLightType(LightType lightType);
		virtual void setLightProperties(const LightProperties& properties);

	protected:

		Node* mNode{ nullptr };
		LightType mLightType{ LightType::Directional };
		LightProperties mProperties;
	};
}