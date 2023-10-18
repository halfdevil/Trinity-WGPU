#include "Scene/Components/SubMesh.h"
#include "Scene/Scene.h"
#include "VFS/FileReader.h"
#include "VFS/FileWriter.h"
#include "Core/ResourceCache.h"

namespace Trinity
{
	std::type_index SubMesh::getType() const
	{
		return typeid(SubMesh);
	}

	size_t SubMesh::getHashCode() const
	{
		return typeid(SubMesh).hash_code();
	}

	void SubMesh::setMaterial(Material& material)
	{
		mMaterial = &material;
	}

	void SubMesh::setVertexBuffer(VertexBuffer& vertexBuffer)
	{
		mVertexBuffer = &vertexBuffer;
	}

	void SubMesh::setIndexBuffer(IndexBuffer& indexBuffer)
	{
		mIndexBuffer = &indexBuffer;
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