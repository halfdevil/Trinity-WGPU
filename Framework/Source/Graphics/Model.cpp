#include "Graphics/Model.h"
#include "Graphics/PBRMaterial.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/BindGroupLayout.h"
#include "Graphics/BindGroup.h"
#include "VFS/FileSystem.h"
#include "Core/ResourceCache.h"
#include "Core/Logger.h"

namespace Trinity
{
	bool Model::create(const std::string& fileName, ResourceCache& cache)
	{
		auto& fileSystem = FileSystem::get();
		mFileName = fileName;

		if (fileSystem.isExist(fileName))
		{
			auto file = fileSystem.openFile(fileName, FileOpenMode::OpenRead);
			if (!file)
			{
				LogError("Error opening model file: %s", fileName.c_str());
				return false;
			}

			FileReader reader(*file);
			if (!read(reader, cache))
			{
				LogError("Model::read() failed for: %s!!", fileName.c_str());
				return false;
			}
		}

		return true;
	}

	bool Model::write()
	{
		auto file = FileSystem::get().openFile(mFileName, FileOpenMode::OpenWrite);
		if (!file)
		{
			LogError("Error opening model file: %s", mFileName.c_str());
			return false;
		}

		FileWriter writer(*file);
		if (!write(writer))
		{
			LogError("Model::write() failed for: %s!!", mFileName.c_str());
			return false;
		}

		return true;
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

	bool Model::read(FileReader& reader, ResourceCache& cache)
	{
		auto& fileSystem = FileSystem::get();

		uint32_t numMaterials{ 0 };
		reader.read(&numMaterials);

		std::vector<std::string> materialFileNames;
		for (uint32_t idx = 0; idx < numMaterials; idx++)
		{
			auto fileName = fileSystem.combinePath(reader.getPath(), reader.readString());
			fileName = fileSystem.canonicalPath(fileName);
			fileName = fileSystem.sanitizePath(fileName);

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

				if (!material->compile())
				{
					LogError("PBRMaterial::compile() failed for: %s!!", materialFileName.c_str());
					return false;
				}

				cache.addResource(std::move(material));
			}
			
			auto* material = cache.getResource<Material>(materialFileName);
			mMaterials.push_back(material);
		}

		auto vertexLayout = std::make_unique<VertexLayout>();
		vertexLayout->setAttributes({
			{ wgpu::VertexFormat::Float32x3, 0, 0 },
			{ wgpu::VertexFormat::Float32x3, 12, 1 },
			{ wgpu::VertexFormat::Float32x2, 24, 2 },
		});

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
				auto indexBuffer = std::make_unique<IndexBuffer>();
				if (!indexBuffer->create(wgpu::IndexFormat::Uint32, mesh.numIndices, mesh.indexData.data()))
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
		auto& fileSystem = FileSystem::get();

		const uint32_t numMaterials = (uint32_t)mMaterials.size();
		writer.write(&numMaterials);

		for (auto* material : mMaterials)
		{
			auto fileName = fileSystem.relativePath(material->getFileName(), writer.getPath());
			fileName = fileSystem.sanitizePath(fileName);

			writer.writeString(fileName);
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
			writer.write(&mesh.numVertices);
			writer.write(&mesh.numIndices);
			writer.write(&mesh.materialIndex);
		}

		return true;
	}
}