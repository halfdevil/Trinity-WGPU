#pragma once

#include "Core/Resource.h"
#include "Math/Types.h"

namespace Trinity
{
	class VertexLayout;
	class VertexBuffer;
	class IndexBuffer;
	class Material;
	class Camera;
	class Transform;

	class Skybox : public Resource
	{
	public:

		Skybox() = default;
		virtual ~Skybox() = default;

		Skybox(const Skybox&) = delete;
		Skybox& operator = (const Skybox&) = delete;

		Skybox(Skybox&&) = default;
		Skybox& operator = (Skybox&&) = default;

		float getSize() const
		{
			return mSize;
		}

		const glm::vec3& getPosition() const
		{
			return mPosition;
		}

		Material* getMaterial() const
		{
			return mMaterial;
		}

		VertexLayout* getVertexLayout() const
		{
			return mVertexLayout;
		}

		VertexBuffer* getVertexBuffer() const
		{
			return mVertexBuffer;
		}

		IndexBuffer* getIndexBuffer() const
		{
			return mIndexBuffer;
		}

		virtual bool create(const std::string& fileName, ResourceCache& cache, bool loadContent = true) override;
		virtual void destroy() override;
		virtual bool write() override;

		virtual void setPosition(const glm::vec3& position);
		virtual std::type_index getType() const override;

		virtual bool load(ResourceCache& cache, Material& material, float size);
		virtual void setMaterial(Material& material);
		virtual void setSize(float size);

	protected:

		virtual bool setupDeviceObjects(ResourceCache& cache);
		virtual bool read(FileReader& reader, ResourceCache& cache) override;
		virtual bool write(FileWriter& writer) override;

	protected:

		float mSize{ 1.0f };
		glm::vec3 mPosition{ 0.0f };
		Material* mMaterial{ nullptr };
		VertexLayout* mVertexLayout{ nullptr };
		VertexBuffer* mVertexBuffer{ nullptr };
		IndexBuffer* mIndexBuffer{ nullptr };
	};
}