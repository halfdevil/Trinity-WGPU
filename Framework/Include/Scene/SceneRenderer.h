#pragma once

#include "Math/Types.h"
#include <vector>
#include <string>
#include <map>

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
	class Camera;
	class Light;
	class Animator;
	class RenderPass;
	class UniformBuffer;
	class StorageBuffer;
	class ResourceCache;

	class SceneRenderer
	{
	public:

		static constexpr uint32_t kSceneBindGroupIndex = 0;
		static constexpr uint32_t kMaterialBindGroupIndex = 1;
		static constexpr uint32_t kTransformBindGroupIndex = 2;

		struct LightBufferData
		{
			glm::vec4 position;
			glm::vec4 color;
			glm::vec4 direction;
			glm::vec2 info;
			glm::vec2 padding;
		};

		struct SceneBufferData
		{
			glm::mat4 view;
			glm::mat4 projection;
			glm::vec3 cameraPos{ 0 };
			uint32_t numLights{ 0 };
		};

		struct TransformBufferData
		{
			glm::mat4 transform;
			glm::mat4 rotation;
		};

		struct LightData
		{
			Light* light{ nullptr };
			uint32_t storageIndex{ 0 };
		};

		struct SceneData
		{
			Scene* scene{ nullptr };
			Camera* camera{ nullptr };
			ResourceCache* cache{ nullptr };
			BindGroup* sceneBindGroup{ nullptr };
			BindGroupLayout* sceneBindGroupLayout{ nullptr };
			UniformBuffer* sceneBuffer{ nullptr };
			StorageBuffer* lightsBuffer{ nullptr };
		};

		struct RenderData
		{
			SubMesh* subMesh{ nullptr };
			Mesh* mesh{ nullptr };
			UniformBuffer* transformBuffer{ nullptr };
			RenderPipeline* pipeline{ nullptr };
			BindGroup* materialBindGroup{ nullptr };
			BindGroup* meshBindGroup{ nullptr };
			BindGroupLayout* meshBindGroupLayout{ nullptr };
			StorageBuffer* meshBindPoseBuffer{ nullptr };
			StorageBuffer* meshInvBindPoseBuffer{ nullptr };
		};

		SceneRenderer() = default;
		virtual ~SceneRenderer() = default;

		SceneRenderer(const SceneRenderer&) = delete;
		SceneRenderer& operator = (const SceneRenderer&) = delete;

		SceneRenderer(SceneRenderer&&) = default;
		SceneRenderer& operator = (SceneRenderer&&) = default;

		bool prepare(Scene& scene, ResourceCache& cache);
		void setCamera(const std::string& nodeName);
		void draw(RenderPass& renderPass);

	protected:

		bool setupRenderData(Mesh* mesh, SubMesh* subMesh, RenderData& renderData);
		bool updateMeshData(Mesh* mesh, Node* node, RenderData& renderData);

		bool updateSceneData();
		bool setupLights();
		bool updateLightData(Light* light, uint32_t index);

		void draw(RenderPass& renderPass, RenderData& renderer);
		void getSortedRenderers(std::multimap<float, RenderData*>& opaqueRenderers, 
			std::multimap<float, RenderData*>& transparentRenderers);

	protected:

		SceneData mSceneData;
		std::vector<RenderData> mRenderers;
		std::vector<LightData> mLights;
	};
}