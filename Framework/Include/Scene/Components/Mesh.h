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

		Model* getModel() const
		{
			return mGltfModel;
		}

		const std::vector<SubMesh*>& getSubMeshes() const
		{
			return mSubMeshes;
		}

		const std::vector<Node*>& getNodes() const
		{
			return mNodes;
		}

		virtual std::type_index getType() const override;
		virtual std::string getTypeStr() const override;

		virtual void setBounds(const BoundingBox& bounds);
		virtual void addSubMesh(SubMesh& subMesh);
		virtual void addNode(Node& node);
		virtual void setModel(Model& model);

		virtual bool read(FileReader& reader, Scene& scene) override;
		virtual bool write(FileWriter& writer, Scene& scene) override;

	public:

		static std::string getStaticType();

	protected:

		BoundingBox mBounds;
		Model* mGltfModel{ nullptr };
		std::vector<SubMesh*> mSubMeshes;
		std::vector<Node*> mNodes;
	};
}