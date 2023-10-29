#include "Scene/Model.h"
#include "Graphics/PBRMaterial.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/BindGroupLayout.h"
#include "Graphics/BindGroup.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimationClip.h"
#include "VFS/FileSystem.h"
#include "Core/ResourceCache.h"
#include "Core/Logger.h"

namespace Trinity
{
	bool Model::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		return Resource::create(fileName, cache, loadContent);
	}

	void Model::destroy()
	{
		Resource::destroy();
		mMeshes.clear();
		mMaterials.clear();
	}

	bool Model::write()
	{
		return Resource::write();
	}

	std::type_index Model::getType() const
	{
		return typeid(Model);
	}

	void Model::setMeshes(std::vector<Mesh>&& meshes)
	{
		mMeshes = std::move(meshes);
	}

	void Model::setMaterials(std::vector<Material*>&& materials)
	{
		mMaterials = std::move(materials);
	}

	void Model::setSkeleton(Skeleton& skeleton)
	{
		mSkeleton = &skeleton;
	}

	void Model::setClips(std::vector<AnimationClip*>&& clips)
	{
		mClips = std::move(clips);
	}

	void Model::addClip(AnimationClip& clip)
	{
		mClips.push_back(&clip);
	}

	bool Model::read(FileReader& reader, ResourceCache& cache)
	{
		if (!Resource::read(reader, cache))
		{
			return false;
		}

		auto& fileSystem = FileSystem::get();
		uint32_t numMaterials{ 0 };
		reader.read(&numMaterials);

		std::vector<std::string> materialFileNames;
		for (uint32_t idx = 0; idx < numMaterials; idx++)
		{
			auto fileName = Resource::getReadPath(reader.getPath(), reader.readString());
			materialFileNames.push_back(fileName);
		}

		for (auto& materialFileName : materialFileNames)
		{
			if (!cache.isLoaded<Material>(materialFileName))
			{
				auto material = std::make_unique<PBRMaterial>();
				if (!material->create(materialFileName, cache))
				{
					LogError("PBRMaterial::create() failed for: %s!!", materialFileName.c_str());
					return false;
				}

				if (!material->compile(cache))
				{
					LogError("PBRMaterial::compile() failed for: %s!!", materialFileName.c_str());
					return false;
				}

				cache.addResource(std::move(material));
			}
			
			auto* material = cache.getResource<Material>(materialFileName);
			mMaterials.push_back(material);
		}

		bool hasSkeleton{ 0 };
		reader.read(&hasSkeleton);

		if (hasSkeleton)
		{
			auto skeletonFileName = Resource::getReadPath(reader.getPath(), reader.readString());
			if (!cache.isLoaded<Skeleton>(skeletonFileName))
			{
				auto skeleton = std::make_unique<Skeleton>();
				if (!skeleton->create(skeletonFileName, cache))
				{
					LogError("Skeleton::create() failed for: %s!!", skeletonFileName.c_str());
					return false;
				}

				cache.addResource(std::move(skeleton));
			}

			uint32_t numClips{ 0 };
			reader.read(&numClips);

			std::vector<std::string> clipFileNames(numClips);
			for (uint32_t idx = 0; idx < numClips; idx++)
			{
				auto fileName = Resource::getReadPath(reader.getPath(), reader.readString());
				clipFileNames[idx] = fileName;
			}

			for (const auto& fileName : clipFileNames)
			{
				if (!cache.isLoaded<AnimationClip>(fileName))
				{
					auto clip = std::make_unique<AnimationClip>();
					if (!clip->create(fileName, cache))
					{
						LogError("AnimationClip::create() failed for: %s!!", fileName.c_str());
						return false;
					}

					cache.addResource(std::move(clip));
				}

				auto* clip = cache.getResource<AnimationClip>(fileName);
				mClips.push_back(clip);
			}

			mSkeleton = cache.getResource<Skeleton>(skeletonFileName);
		}

		auto vertexLayout = std::make_unique<VertexLayout>();
		if (hasSkeleton)
		{
			vertexLayout->setAttributes({
				{ wgpu::VertexFormat::Float32x3, 0, 0 },
				{ wgpu::VertexFormat::Float32x3, 12, 1 },
				{ wgpu::VertexFormat::Float32x2, 24, 2 },
				{ wgpu::VertexFormat::Uint32x4, 32, 3 },
				{ wgpu::VertexFormat::Float32x4, 48, 4 }
			});
		}
		else
		{
			vertexLayout->setAttributes({
				{ wgpu::VertexFormat::Float32x3, 0, 0 },
				{ wgpu::VertexFormat::Float32x3, 12, 1 },
				{ wgpu::VertexFormat::Float32x2, 24, 2 },
			});
		}		

		uint32_t numMeshes{ 0 };
		reader.read(&numMeshes);

		for (uint32_t idx = 0; idx < numMeshes; idx++)
		{
			Mesh mesh{};
			mesh.name = reader.readString();

			uint32_t vertexDataCount{ 0 };
			reader.read(&vertexDataCount);

			mesh.vertexData.resize(vertexDataCount);
			reader.read(mesh.vertexData.data(), vertexDataCount);

			uint32_t indexDataCount{ 0 };
			reader.read(&indexDataCount);

			if (indexDataCount > 0)
			{
				mesh.indexData.resize(indexDataCount);
				reader.read(mesh.indexData.data(), indexDataCount);
			}

			reader.read(&mesh.vertexSize);
			reader.read(&mesh.indexSize);
			reader.read(&mesh.numVertices);
			reader.read(&mesh.numIndices);
			reader.read(&mesh.materialIndex);

			auto vertexBuffer = std::make_unique<VertexBuffer>();
			if (!vertexBuffer->create(*vertexLayout, mesh.numVertices, mesh.vertexData.data()))
			{
				LogError("VertexBuffer::create() failed!!");
				return false;
			}

			mesh.vertexBuffer = vertexBuffer.get();
			cache.addResource(std::move(vertexBuffer));

			if (mesh.numIndices > 0)
			{
				auto indexFormat = mesh.indexSize == sizeof(uint32_t) ? wgpu::IndexFormat::Uint32 : wgpu::IndexFormat::Uint16;
				auto indexBuffer = std::make_unique<IndexBuffer>();

				if (!indexBuffer->create(indexFormat, mesh.numIndices, mesh.indexData.data()))
				{
					LogError("IndexBuffer::create() failed!!");
					return false;
				}

				mesh.indexBuffer = indexBuffer.get();
				cache.addResource(std::move(indexBuffer));
			}

			mMeshes.push_back(std::move(mesh));
		}

		cache.addResource(std::move(vertexLayout));
		return true;
	}

	bool Model::write(FileWriter& writer)
	{
		if (!Resource::write(writer))
		{
			return false;
		}

		auto& fileSystem = FileSystem::get();
		const uint32_t numMaterials = (uint32_t)mMaterials.size();
		writer.write(&numMaterials);

		for (auto* material : mMaterials)
		{
			auto fileName = Resource::getWritePath(writer.getPath(), material->getFileName());
			writer.writeString(fileName);
		}

		const bool hasSkeleton = mSkeleton != nullptr;
		writer.write(&hasSkeleton);

		if (hasSkeleton)
		{
			if (mSkeleton != nullptr)
			{
				auto fileName = Resource::getWritePath(writer.getPath(), mSkeleton->getFileName());
				writer.writeString(fileName);
			}

			const uint32_t numClips = (uint32_t)mClips.size();
			writer.write(&numClips);

			for (auto* clip : mClips)
			{
				auto fileName = Resource::getWritePath(writer.getPath(), clip->getFileName());
				writer.writeString(fileName);
			}
		}

		const uint32_t numMeshes = (uint32_t)mMeshes.size();
		writer.write(&numMeshes);

		for (auto& mesh : mMeshes)
		{
			writer.writeString(mesh.name);

			const uint32_t vertexDataCount = (uint32_t)mesh.vertexData.size();
			writer.write(&vertexDataCount);
			writer.write(mesh.vertexData.data(), vertexDataCount);

			const uint32_t indexDataCount = (uint32_t)mesh.indexData.size();
			writer.write(&indexDataCount);

			if (indexDataCount > 0)
			{
				writer.write(mesh.indexData.data(), indexDataCount);
			}

			writer.write(&mesh.vertexSize);
			writer.write(&mesh.indexSize);
			writer.write(&mesh.numVertices);
			writer.write(&mesh.numIndices);
			writer.write(&mesh.materialIndex);
		}

		return true;
	}
}