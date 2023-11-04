#pragma once

#include "Core/Resource.h"

namespace Trinity
{
	class Material;
	class VertexLayout;
	class VertexBuffer;
	class IndexBuffer;
	class Skeleton;
	class AnimationClip;
	class FileReader;
	class FileWriter;

	class Model : public Resource
	{
	public:

		struct Mesh
		{
			std::string name;
			std::vector<uint8_t> vertexData;
			std::vector<uint8_t> indexData;
			VertexLayout* vertexLayout{ nullptr };
			VertexBuffer* vertexBuffer{ nullptr };
			IndexBuffer* indexBuffer{ nullptr };
			uint32_t vertexSize{ 0 };
			uint32_t indexSize{ 0 };
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

		const std::vector<AnimationClip*>& getClips() const
		{
			return mClips;
		}

		Skeleton* getSkeleton() const
		{
			return mSkeleton;
		}

		std::vector<Mesh>& getMeshes()
		{
			return mMeshes;
		}

		std::vector<Material*>& getMaterials()
		{
			return mMaterials;
		}

		std::vector<AnimationClip*>& getClips()
		{
			return mClips;
		}

		bool isAnimated() const
		{
			return mSkeleton != nullptr;
		}

		virtual bool create(const std::string& fileName, ResourceCache& cache, bool loadContent = true) override;
		virtual void destroy() override;
		virtual bool write() override;

		virtual std::type_index getType() const override;
		
		virtual void setMeshes(std::vector<Mesh>&& meshes);
		virtual void setMaterials(std::vector<Material*>&& materials);
		virtual void setSkeleton(Skeleton& skeleton);
		virtual void setClips(std::vector<AnimationClip*>&& clips);
		virtual void addClip(AnimationClip& clip);

	protected:

		virtual bool read(FileReader& reader, ResourceCache& cache) override;
		virtual bool write(FileWriter& writer) override;

	protected:

		std::vector<Mesh> mMeshes;
		std::vector<Material*> mMaterials; 
		Skeleton* mSkeleton{ nullptr };
		std::vector<AnimationClip*> mClips;
	};
}