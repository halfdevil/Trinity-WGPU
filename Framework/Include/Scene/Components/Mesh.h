#pragma once

#include "Scene/Component.h"
#include "Math/BoundingBox.h"

namespace Trinity
{
	class SubMesh;
	class Node;
	class Model;

	class Mesh : public Component
	{
	public:

		Mesh() = default;
		virtual ~Mesh() = default;

		Mesh(const Mesh&) = delete;
		Mesh& operator = (const Mesh&) = delete;

		Mesh(Mesh&&) = default;
		Mesh& operator = (Mesh&&) = default;

		const BoundingBox& getBounds() const
		{
			return mBounds;
		}

		Node* getNode() const
		{
			return mNode;
		}

		Model* getModel() const
		{
			return mModel;
		}

		const std::vector<SubMesh*>& getSubMeshes() const
		{
			return mSubMeshes;
		}

		std::vector<glm::mat4>& getBindPose()
		{
			return mBindPose;
		}

		const std::vector<glm::mat4>& getInvBindPose() const;
		const std::vector<glm::mat4>& getBindPose() const;

		virtual bool isAnimated() const;
		virtual bool load(const std::string& modelFileName, ResourceCache& cache, Scene& scene);
		virtual std::type_index getType() const override;
		virtual std::string getTypeStr() const override;

		virtual void setNode(Node& node);
		virtual void setBounds(const BoundingBox& bounds);
		virtual void addSubMesh(SubMesh& subMesh);
		virtual void setModel(Model& model);

		virtual bool read(FileReader& reader, ResourceCache& cache, Scene& scene) override;
		virtual bool write(FileWriter& writer, Scene& scene) override;

	public:

		static std::string getStaticType();

	protected:

		BoundingBox mBounds;
		Node* mNode{ nullptr };
		Model* mModel{ nullptr };
		std::vector<SubMesh*> mSubMeshes;
		std::vector<glm::mat4> mBindPose;
	};
}