#pragma once

#include "Math/Types.h"
#include <vector>
#include <string>
#include <map>

namespace Trinity
{
	class Terrain;
	class Material;
	class BindGroupLayout;
	class BindGroup;
	class RenderPipeline;
	class RenderPass;
	class UniformBuffer;
	class StorageBuffer;
	class ResourceCache;
	class Node;
	class Scene;
	class Camera;
	class Light;

	class TerrainRenderer
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

		struct LightData
		{
			Light* light{ nullptr };
			uint32_t storageIndex{ 0 };
		};

		struct SceneData
		{
			Terrain* terrain{ nullptr };
			Scene* scene{ nullptr };
			Camera* camera{ nullptr };
			ResourceCache* cache{ nullptr };
			BindGroup* sceneBindGroup{ nullptr };
			BindGroup* materialBindGroup{ nullptr };
			BindGroupLayout* sceneBindGroupLayout{ nullptr };
			UniformBuffer* sceneBuffer{ nullptr };
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
		virtual void draw(RenderPass& renderPass);

	protected:

		virtual bool setupSceneData();
		virtual bool updateSceneData();
		virtual bool setupLights();
		virtual bool updateLightData(Light* light, uint32_t index);

	protected:

		SceneData mSceneData;
		std::vector<LightData> mLights;
	};
}