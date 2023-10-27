#include "Scene/Components/Mesh.h"
#include "Scene/Components/SubMesh.h"
#include "Scene/ComponentFactory.h"
#include "Scene/Model.h"
#include "Scene/Scene.h"
#include "Animation/Skeleton.h"
#include "VFS/FileSystem.h"
#include "Core/ResourceCache.h"
#include "Core/Logger.h"

namespace Trinity
{
	const std::vector<glm::mat4>& Mesh::getInvBindPose() const
	{
		return mModel->getSkeleton()->getInvBindPose();
	}

	const std::vector<glm::mat4>& Mesh::getBindPose() const
	{
		return mBindPose;
	}

	bool Mesh::isAnimated() const
	{
		return mModel && mModel->isAnimated();
	}

	bool Mesh::load(const std::string& modelFileName, ResourceCache& cache, Scene& scene)
	{
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

		return true;
	}

	std::type_index Mesh::getType() const
	{
		return typeid(Mesh);
	}

	std::string Mesh::getTypeStr() const
	{
		return getStaticType();
	}

	void Mesh::setNode(Node& node)
	{
		mNode = &node;
	}

	void Mesh::setBounds(const BoundingBox& bounds)
	{
		mBounds = bounds;
	}

	void Mesh::addSubMesh(SubMesh& subMesh)
	{
		mSubMeshes.push_back(&subMesh);
	}

	void Mesh::setModel(Model& model)
	{
		mModel = &model;
	}

	bool Mesh::read(FileReader& reader, ResourceCache& cache, Scene& scene)
	{
		auto& fileSystem = FileSystem::get();
		if (!Component::read(reader, cache, scene))
		{
			return false;
		}

		uint32_t nodeId{ 0 };
		reader.read(&nodeId);
		mNode = scene.getNode(nodeId);

		auto modelFileName = fileSystem.combinePath(reader.getPath(), reader.readString());
		modelFileName = fileSystem.canonicalPath(modelFileName);
		modelFileName = fileSystem.sanitizePath(modelFileName);

		if (!load(modelFileName, cache, scene))
		{
			LogError("Mesh::load() failed for: %s!!", modelFileName.c_str());
			return false;
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

		const uint32_t nodeId = mNode->getId();
		writer.write(&nodeId);

		if (mModel != nullptr)
		{
			auto fileName = fileSystem.relativePath(mModel->getFileName(), writer.getPath());
			fileSystem.sanitizePath(fileName);

			writer.writeString(fileName);
		}
		
		return true;
	}

	std::string Mesh::getStaticType()
	{
		return "Mesh";
	}
}