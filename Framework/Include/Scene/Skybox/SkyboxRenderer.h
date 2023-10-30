#pragma once

#include "Math/Types.h"
#include <vector>
#include <string>
#include <map>

namespace Trinity
{
	class Skybox;
	class Material;
	class BindGroupLayout;
	class BindGroup;
	class RenderPipeline;
	class RenderPass;
	class UniformBuffer;
	class ResourceCache;
	class Camera;

	class SkyboxRenderer
	{
	public:

		static constexpr uint32_t kSceneBindGroupIndex = 0;
		static constexpr uint32_t kMaterialBindGroupIndex = 1;

		struct SceneBufferData
		{
			glm::mat4 view;
			glm::mat4 projection;
			glm::mat4 transform;
		};

		struct SceneData
		{
			Skybox* skybox{ nullptr };
			Camera* camera{ nullptr };
			ResourceCache* cache{ nullptr };
			BindGroup* sceneBindGroup{ nullptr };
			BindGroup* materialBindGroup{ nullptr };
			BindGroupLayout* sceneBindGroupLayout{ nullptr };
			UniformBuffer* sceneBuffer{ nullptr };
			RenderPipeline* pipeline{ nullptr };
		};

		SkyboxRenderer() = default;
		virtual ~SkyboxRenderer() = default;

		SkyboxRenderer(const SkyboxRenderer&) = delete;
		SkyboxRenderer& operator = (const SkyboxRenderer&) = delete;

		SkyboxRenderer(SkyboxRenderer&&) = default;
		SkyboxRenderer& operator = (SkyboxRenderer&&) = default;

		virtual bool prepare(Skybox& skybox, Camera& camera, ResourceCache& cache);
		virtual void setCamera(Camera& camera);
		virtual void draw(RenderPass& renderPass);

	protected:

		virtual bool setupSceneData();
		virtual bool updateSceneData();

	protected:

		SceneData mSceneData;
	};
}