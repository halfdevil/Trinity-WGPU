#include "Scene/Components/Mesh.h"
#include "Scene/Components/SubMesh.h"
#include "Scene/ComponentFactory.h"
#include "Scene/Scene.h"
#include "VFS/FileSystem.h"
#include "Graphics/Model.h"
#include "Core/ResourceCache.h"
#include "Core/Logger.h"

namespace Trinity
{
	std::type_index Mesh::getType() const
	{
		return typeid(Mesh);
	}

	std::string Mesh::getTypeStr() const
	{
		return getStaticType();
	}

	void Mesh::setBounds(const BoundingBox& bounds)
	{
		mBounds = bounds;
	}

	void Mesh::addSubMesh(SubMesh& subMesh)
	{
		mSubMeshes.push_back(&subMesh);
	}

	void Mesh::addNode(Node& node)
	{
		mNodes.push_back(&node);
	}

	void Mesh::setModel(Model& model)
	{
		mModel = &model;
	}

	bool Mesh::read(FileReader& reader, Scene& scene)
	{
		auto& fileSystem = FileSystem::get();

		if (!Component::read(reader, scene))
		{
			return false;
		}

		auto& cache = scene.getResourceCache();
		auto modelFileName = fileSystem.combinePath(reader.getPath(), reader.readString());
		modelFileName = fileSystem.canonicalPath(modelFileName);
		modelFileName = fileSystem.sanitizePath(modelFileName);

		if (!cache.isLoaded<Model>(modelFileName))
		{
			auto model = std::make_unique<Model>();
			if (!model->create(modelFileName, cache))
			{
				LogError("Model::create() failed for: %s!!", modelFileName.c_str());
				return false;
			}

			cache.addResource(std::move(model));
		}

		mModel = cache.getResource<Model>(modelFileName);
		const auto& materials = mModel->getMaterials();
		const auto& meshes = mModel->getMeshes();

		for (const auto& mesh : meshes)
		{
			auto subMesh = std::make_unique<SubMesh>();
			subMesh->setName(mesh.name);
			subMesh->setMaterial(*materials[mesh.materialIndex]);
			subMesh->setVertexBuffer(*mesh.vertexBuffer);

			if (mesh.numIndices > 0)
			{
				subMesh->setIndexBuffer(*mesh.indexBuffer);
			}

			subMesh->setNumVertices(mesh.numVertices);
			subMesh->setNumIndices(mesh.numIndices);

			mSubMeshes.push_back(subMesh.get());
			scene.addComponent(std::move(subMesh));
		}

		std::vector<uint32_t> nodes;
		reader.readVector(nodes);

		for (auto& node : nodes)
		{
			mNodes.push_back(scene.getNode(node));
		}

		return true;
	}

	bool Mesh::write(FileWriter& writer, Scene& scene)
	{
		auto& fileSystem = FileSystem::get();

		if (!Component::write(writer, scene))
		{
			return false;
		}

		if (mModel != nullptr)
		{
			auto fileName = fileSystem.relativePath(mModel->getFileName(), writer.getPath());
			fileSystem.sanitizePath(fileName);

			writer.writeString(fileName);
		}

		std::vector<uint32_t> nodes;
		for (auto* node : mNodes)
		{
			nodes.push_back(node->getId());
		}
		
		writer.writeVector(nodes);
		return true;
	}

	std::string Mesh::getStaticType()
	{
		return "Mesh";
	}
}