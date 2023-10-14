#pragma once

#include "Core/Resource.h"
#include <string>
#include <memory>
#include "imgui.h"

namespace Trinity
{
	class Texture2D;
	class Sampler;
	class BindGroup;
	class BindGroupLayout;

	class Font : public Resource
	{
	public:

		Font() = default;
		~Font();

		Font(const Font&) = default;
		Font& operator = (const Font&) = default;

		Font(Font&&) noexcept = default;
		Font& operator = (Font&&) noexcept = default;

		const std::string& getName() const
		{
			return mName;
		}

		float getSize() const
		{
			return mSize;
		}

		ImFont* getHandle() const
		{
			return mHandle;
		}

		Texture2D* getTexture() const
		{
			return mTexture.get();
		}

		virtual bool create(const std::string& name, const std::string& filePath, float size = 20.0f);
		virtual void destroy();

		virtual void activate();
		virtual void deactivate();

		virtual std::type_index getType() const override;

	protected:

		std::string mName;
		float mSize{ 20.0f };
		ImFont* mHandle{ nullptr };
		std::unique_ptr<Texture2D> mTexture{ nullptr };
	};
}