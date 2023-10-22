#pragma once

#include "Core/Resource.h"

namespace Trinity
{
	class Material;
	class VertexBuffer;
	class IndexBuffer;
	class FileReader;
	class FileWriter;

	class Model : public Resource
	{
	public:

		struct Mesh
		{
			std::string name;
			std::vector<float> vertexData;
			std::vector<uint32_t> indexData;
			VertexBuffer* vertexBuffer{ nullptr };
			IndexBuffer* indexBuffer{ nullptr };
			uint32_t vertexSize{ 0 };
			uint32_t numVertices{ 0 };
			uint32_t numIndices{ 0 };
			uint32_t materialIndex{ (uint32_t)-1 };
		};

		Model() = default;
		virtual ~Model() = default;

		Model(const Model&) = delete;
		Model& operator = (const Model&) = delete;

		Model(Model&&) = default;
		Model& operator = (Model&&) = default;

		const std::vector<Mesh>& getMeshes() const
		{
			return mMeshes;
		}

		const std::vector<Material*>& getMaterials() const
		{
			return mMaterials;
		}

		virtual bool create(const std::string& fileName, ResourceCache& cache, bool loadContent = true) override;
		virtual bool write() override;

		virtual std::type_index getType() const override;
		
		virtual void setMeshes(std::vector<Mesh>&& meshes);
		virtual void setMaterials(std::vector<Material*>&& materials);

	protected:

		virtual bool read(FileReader& reader, ResourceCache& cache);
		virtual bool write(FileWriter& writer);

	protected:

		std::vector<Mesh> mMeshes;
		std::vector<Material*> mMaterials;
	};
}