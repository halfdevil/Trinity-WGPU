#include "Scene/Terrain/QuadGridMesh.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Core/Logger.h"

namespace Trinity
{
	QuadGridMesh::~QuadGridMesh()
	{
		destroy();
	}

	bool QuadGridMesh::create(uint32_t dimension, VertexLayout& vertexLayout)
	{
		mDimension = dimension;

		uint32_t numVertices = (dimension + 1) * (dimension + 1);
		std::vector<Vertex> vertices(numVertices);

		for (uint32_t z = 0; z < dimension + 1; z++)
		{
			for (uint32_t x = 0; x < dimension + 1; x++)
			{
				vertices[x + (dimension + 1) * z] = { 
					.position = { 
						x / (float)dimension,
						0.0f,
						z / (float)dimension 
					} 
				};
			}
		}

		uint32_t numIndices = dimension * dimension * 2 * 3;
		std::vector<uint16_t> indices(numIndices);

		uint32_t index{ 0 };
		uint32_t halfDim = (dimension + 1) / 2;
		uint32_t fullDim = dimension;

		for (uint32_t z = 0; z < halfDim; z++)
		{
			for (uint32_t x = 0; x < halfDim; x++)
			{
				indices[index++] = (uint16_t)(x + (dimension + 1) * z);
				indices[index++] = (uint16_t)((x + 1) + (dimension + 1) * z);
				indices[index++] = (uint16_t)((x + 1) + (dimension + 1) * z);
				indices[index++] = (uint16_t)((x + 1) + (dimension + 1) * (z + 1));
			}
		}

		mIndexEndTL = index;

		for (uint32_t z = 0; z < halfDim; z++)
		{
			for (uint32_t x = halfDim; x < fullDim; x++)
			{
				indices[index++] = (uint16_t)(x + (dimension + 1) * z);
				indices[index++] = (uint16_t)((x + 1) + (dimension + 1) * z);
				indices[index++] = (uint16_t)((x + 1) + (dimension + 1) * z);
				indices[index++] = (uint16_t)((x + 1) + (dimension + 1) * (z + 1));
			}
		}

		mIndexEndTR = index;

		for (uint32_t z = halfDim; z < fullDim; z++)
		{
			for (uint32_t x = 0; x < halfDim; x++)
			{
				indices[index++] = (uint16_t)(x + (dimension + 1) * z);
				indices[index++] = (uint16_t)((x + 1) + (dimension + 1) * z);
				indices[index++] = (uint16_t)((x + 1) + (dimension + 1) * z);
				indices[index++] = (uint16_t)((x + 1) + (dimension + 1) * (z + 1));
			}
		}

		mIndexEndBL = index;

		for (uint32_t z = halfDim; z < fullDim; z++)
		{
			for (uint32_t x = halfDim; x < fullDim; x++)
			{
				indices[index++] = (uint16_t)(x + (dimension + 1) * z);
				indices[index++] = (uint16_t)((x + 1) + (dimension + 1) * z);
				indices[index++] = (uint16_t)((x + 1) + (dimension + 1) * z);
				indices[index++] = (uint16_t)((x + 1) + (dimension + 1) * (z + 1));
			}
		}

		mIndexEndBR = index;

		mVertexBuffer = std::make_unique<VertexBuffer>();
		if (!mVertexBuffer->create(vertexLayout, numVertices, vertices.data()))
		{
			LogError("VertexBuffer::create() failed");
			return false;
		}

		mIndexBuffer = std::make_unique<IndexBuffer>();
		if (!mIndexBuffer->create(wgpu::IndexFormat::Uint16, numIndices, indices.data()))
		{
			LogError("IndexBuffer::create() failed");
			return false;
		}

		return true;
	}

	void QuadGridMesh::destroy()
	{
		mVertexBuffer = nullptr;
		mIndexBuffer = nullptr;
	}
}