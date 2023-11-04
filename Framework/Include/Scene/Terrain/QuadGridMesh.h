#pragma once

#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>

namespace Trinity
{
	class VertexBuffer;
	class IndexBuffer;
	class VertexLayout;

	class QuadGridMesh
	{
	public:

		struct Vertex
		{
			glm::vec3 position{ 0.0f };
		};

		QuadGridMesh() = default;
		virtual ~QuadGridMesh();

		QuadGridMesh(const QuadGridMesh&) = delete;
		QuadGridMesh& operator = (const QuadGridMesh&) = delete;

		QuadGridMesh(QuadGridMesh&&) = default;
		QuadGridMesh& operator = (QuadGridMesh&&) = default;

		uint32_t getDimension() const
		{
			return mDimension;
		}

		VertexBuffer* getVertexBuffer() const
		{
			return mVertexBuffer.get();
		}

		IndexBuffer* getIndexBuffer() const
		{
			return mIndexBuffer.get();
		}

		uint32_t getIndexEndTL() const
		{
			return mIndexEndTL;
		}

		uint32_t getIndexEndTR() const
		{
			return mIndexEndTR;
		}

		uint32_t getIndexEndBL() const
		{
			return mIndexEndBL;
		}

		uint32_t getIndexEndBR() const
		{
			return mIndexEndBR;
		}

		virtual bool create(uint32_t dimension, VertexLayout& vertexLayout);
		virtual void destroy();

	protected:

		std::unique_ptr<VertexBuffer> mVertexBuffer{ nullptr };
		std::unique_ptr<IndexBuffer> mIndexBuffer{ nullptr };
		uint32_t mDimension{ 0 };
		uint32_t mIndexEndTL{ 0 };
		uint32_t mIndexEndTR{ 0 };
		uint32_t mIndexEndBL{ 0 };
		uint32_t mIndexEndBR{ 0 };
	};
}