#pragma once

#include "Scene/Component.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/Material.h"
#include "Graphics/Shader.h"

namespace Trinity
{
	class SubMesh : public Component
	{
	public:

		SubMesh() = default;
		virtual ~SubMesh() = default;

		SubMesh(const SubMesh&) = delete;
		SubMesh& operator = (const SubMesh&) = delete;

		SubMesh(SubMesh&&) = default;
		SubMesh& operator = (SubMesh&&) = default;

		Material* getMaterial()
		{
			return mMaterial;
		}

		VertexBuffer* getVertexBuffer() const
		{
			return mVertexBuffer;
		}

		IndexBuffer* getIndexBuffer() const
		{
			return mIndexBuffer;
		}

		uint32_t getNumVertices() const
		{
			return mNumVertices;
		}

		uint32_t getIndexOffset() const
		{
			return mIndexOffset;
		}

		uint32_t getNumIndices() const
		{
			return mNumIndices;
		}

		bool hasIndexBuffer() const
		{
			return mNumIndices > 0;
		}

		virtual std::type_index getType() const override;
		virtual std::string getTypeStr() const override;

		virtual void setMaterial(Material& material);
		virtual void setVertexBuffer(VertexBuffer& vertexBuffer);
		virtual void setIndexBuffer(IndexBuffer& indexBuffer);
		virtual void setNumVertices(uint32_t numVertices);
		virtual void setIndexOffset(uint32_t indexOffset);
		virtual void setNumIndices(uint32_t numIndices);

	public:

		static std::string getStaticType();

	protected:

		Material* mMaterial{ nullptr };
		VertexBuffer* mVertexBuffer{ nullptr };
		IndexBuffer* mIndexBuffer{ nullptr };
		uint32_t mNumVertices{ 0 };
		uint32_t mIndexOffset{ 0 };
		uint32_t mNumIndices{ 0 };
	};
}