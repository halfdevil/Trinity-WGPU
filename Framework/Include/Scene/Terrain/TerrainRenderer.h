#pragma once

#include "Math/Types.h"
#include <vector>
#include <string>
#include <map>
#include <memory>

namespace Trinity
{
	class Terrain;
	class QuadTree;
	class QuadGridMesh;
	class Material;
	class BindGroupLayout;
	class BindGroup;
	class RenderPipeline;
	class RenderPass;
	class UniformBuffer;
	class StorageBuffer;
	class ResourceCache;
	class VertexLayout;
	class Node;
	class Scene;
	class Camera;
	class Light;
	struct SelectedLOD;
	struct SelectedNode;

	class TerrainRenderer
	{
	public:

		static constexpr uint32_t kMaxGridMesh = 7;
		static constexpr uint32_t kMaxSelectionNodes = 4096;
		static constexpr uint32_t kSceneBindGroupIndex = 0;
		static constexpr uint32_t kMaterialBindGroupIndex = 1;

		struct LightBufferData
		{
			glm::vec4 position;
			glm::vec4 color;
			glm::vec4 direction;
			glm::vec2 info;
			glm::vec2 padding;
		};

		struct TerrainBufferData
		{
			glm::vec4 terrainScale{ 0.0f };
			glm::vec4 terrainOffset{ 0.0f };
			glm::vec2 quadWorldMax{ 0.0f };
			glm::vec2 worldToTextureScale{ 0.0f };
			glm::vec4 gridDimension{ 0.0f };
			glm::vec4 heightMapTextureInfo{ 0.0f };
		};

		struct QuadBufferData
		{
			glm::vec4 morphConsts{ 0.0f };
			glm::vec4 quadOffset{ 0.0f };
			glm::vec4 quadScale{ 0.0f };
		};

		struct SceneBufferData
		{
			glm::mat4 view;
			glm::mat4 projection;
			glm::vec3 cameraPos{ 0 };
			uint32_t numLights{ 0 };
		};

		struct LightData
		{
			Light* light{ nullptr };
			uint32_t storageIndex{ 0 };
		};

		struct SceneData
		{
			Scene* scene{ nullptr };
			Terrain* terrain{ nullptr };
			Camera* camera{ nullptr };
			ResourceCache* cache{ nullptr };
			VertexLayout* gridMeshLayout{ nullptr };
			BindGroup* sceneBindGroup{ nullptr };
			BindGroup* materialBindGroup{ nullptr };
			BindGroupLayout* sceneBindGroupLayout{ nullptr };
			UniformBuffer* sceneBuffer{ nullptr };
			UniformBuffer* terrainBuffer{ nullptr };
			UniformBuffer* quadBuffer{ nullptr };
			StorageBuffer* lightsBuffer{ nullptr };
			RenderPipeline* pipeline{ nullptr };
		};

		TerrainRenderer() = default;
		virtual ~TerrainRenderer() = default;

		TerrainRenderer(const TerrainRenderer&) = delete;
		TerrainRenderer& operator = (const TerrainRenderer&) = delete;

		TerrainRenderer(TerrainRenderer&&) = default;
		TerrainRenderer& operator = (TerrainRenderer&&) = default;

		virtual bool prepare(Terrain& terrain, Scene& scene, ResourceCache& cache);
		virtual void setCamera(const std::string& nodeName);
		virtual void draw(const RenderPass& renderPass);

	protected:

		virtual bool setupPipeline();
		virtual bool setupSceneData();
		virtual bool setupLights();
		virtual bool setupTerrainData();
		virtual bool setupQuadData();
		virtual bool setupGridMesh();
		virtual bool setupQuadTree();

		virtual void updateSceneData();
		virtual void updateLights();
		virtual void updateTerrainData(const TerrainBufferData& bufferData);
		virtual void updateQuadData(const QuadBufferData& bufferData);
		virtual void drawSelection(const RenderPass& renderPass, uint32_t lodLevel);

	protected:

		SceneData mSceneData;
		QuadBufferData mQuadBufferData;
		std::vector<LightData> mLights;
		std::unique_ptr<SelectedLOD> mSelectedLOD{ nullptr };
		std::unique_ptr<QuadTree> mQuadTree{ nullptr };
		std::unique_ptr<QuadGridMesh> mGridMesh{ nullptr };
	};
}