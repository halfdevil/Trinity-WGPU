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

		const VertexLayout* getVertexLayout() const
		{
			return mVertexLayout;
		}

		const Material* getMaterial() const
		{
			return mMaterial;
		}

		const Shader* getShader() const
		{
			return mShader;
		}

		IndexBuffer* getIndexBuffer() const
		{
			return mIndexBuffer.get();
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

		VertexBuffer* getVertexBuffer(const std::string& type);
		virtual std::type_index getType() const override;

		virtual void setVertexLayout(const VertexLayout& vertexLayout);
		virtual void setMaterial(const Material& material);
		virtual void setShader(const Shader& shader);

		virtual void setVertexBuffer(const std::string& type, std::unique_ptr<VertexBuffer> vertexBuffer);
		virtual void setIndexBuffer(std::unique_ptr<IndexBuffer> indexBuffer);
		virtual void setNumVertices(uint32_t numVertices);
		virtual void setIndexOffset(uint32_t indexOffset);
		virtual void setNumIndices(uint32_t numIndices);

	protected:

		const Material* mMaterial{ nullptr };
		const Shader* mShader{ nullptr };
		const VertexLayout* mVertexLayout{ nullptr };

		std::unordered_map<std::string, std::unique_ptr<VertexBuffer>> mVertexBuffers;
		std::unique_ptr<IndexBuffer> mIndexBuffer;

		uint32_t mNumVertices{ 0 };
		uint32_t mIndexOffset{ 0 };
		uint32_t mNumIndices{ 0 };
	};
}