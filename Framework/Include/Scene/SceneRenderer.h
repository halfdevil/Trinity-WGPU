#pragma once

#include "Math/Types.h"
#include <vector>

namespace Trinity
{
	class Material;
	class BindGroupLayout;
	class BindGroup;
	class RenderPipeline;
	class SubMesh;
	class Mesh;
	class Node;
	class Scene;
	class PerspectiveCamera;
	class RenderPass;
	class UniformBuffer;

	class SceneRenderer
	{
	public:

		static constexpr uint32_t kSceneBindGroupIndex = 0;
		static constexpr uint32_t kMaterialBindGroupIndex = 1;
		static constexpr uint32_t kTransformBindGroupIndex = 2;

		struct SceneBufferData
		{
			glm::mat4 view;
			glm::mat4 projection;
		};

		struct TransformBufferData
		{
			glm::mat4 transform;
			glm::mat4 rotation;
		};

		struct SceneData
		{
			Scene* scene{ nullptr };
			PerspectiveCamera* camera{ nullptr };
			BindGroup* sceneBindGroup{ nullptr };
			BindGroupLayout* sceneBindGroupLayout{ nullptr };
			UniformBuffer* sceneBuffer{ nullptr };
		};

		struct RenderData
		{
			SubMesh* subMesh{ nullptr };
			Node* node{ nullptr };
			UniformBuffer* transformBuffer{ nullptr };
			RenderPipeline* pipeline{ nullptr };
			BindGroup* materialBindGroup{ nullptr };
			BindGroup* transformBindGroup{ nullptr };
			BindGroupLayout* transformBindGroupLayout{ nullptr };
		};

		bool prepare(Scene& scene);
		void draw(RenderPass* renderPass);

	protected:

		bool setupRenderData(Node* node, Mesh* mesh, SubMesh* subMesh, RenderData& renderData);
		bool updateTransformData(Node* node, RenderData& renderData);
		bool updateSceneData();

	protected:

		SceneData mSceneData;
		std::vector<RenderData> mRenderers;
	};
}