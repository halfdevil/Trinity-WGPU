#include "Scene/Components/SubMesh.h"

namespace Trinity
{
	VertexBuffer* SubMesh::getVertexBuffer(const std::string& type)
	{
		return mVertexBuffers.at(type).get();
	}

	std::type_index SubMesh::getType() const
	{
		return typeid(SubMesh);
	}

	void SubMesh::setVertexLayout(const VertexLayout& vertexLayout)
	{
		mVertexLayout = &vertexLayout;
	}

	void SubMesh::setMaterial(const Material& material)
	{
		mMaterial = &material;
	}

	void SubMesh::setShader(const Shader& shader)
	{
		mShader = &shader;
	}

	void SubMesh::setVertexBuffer(const std::string& type, std::unique_ptr<VertexBuffer> vertexBuffer)
	{
		auto it = mVertexBuffers.find(type);
		if (it != mVertexBuffers.end())
		{
			it->second = std::move(vertexBuffer);
		}
		else
		{
			mVertexBuffers.insert(std::make_pair(type, std::move(vertexBuffer)));
		}
	}

	void SubMesh::setIndexBuffer(std::unique_ptr<IndexBuffer> indexBuffer)
	{
		mIndexBuffer = std::move(indexBuffer);
	}

	void SubMesh::setNumVertices(uint32_t numVertices)
	{
		mNumVertices = numVertices;
	}

	void SubMesh::setIndexOffset(uint32_t indexOffset)
	{
		mIndexOffset = indexOffset;
	}

	void SubMesh::setNumIndices(uint32_t numIndices)
	{
		mNumIndices = numIndices;
	}
}