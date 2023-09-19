#include "Scene/Components/Mesh.h"

namespace Trinity
{
	std::type_index Mesh::getType() const
	{
		return typeid(Mesh);
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
}