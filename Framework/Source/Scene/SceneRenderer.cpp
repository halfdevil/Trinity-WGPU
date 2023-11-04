#include "Scene/SceneRenderer.h"
#include "Scene/Components/Mesh.h"
#include "Scene/Components/SubMesh.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/Light.h"
#include "Scene/Node.h"
#include "Scene/Scene.h"
#include "Scene/Model.h"
#include "Graphics/PBRMaterial.h"
#include "Graphics/RenderPipeline.h"
#include "Graphics/Material.h"
#include "Graphics/BindGroup.h"
#include "Graphics/BindGroupLayout.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/StorageBuffer.h"
#include "Graphics/SwapChain.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/RenderPass.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimationPose.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"
#include "Core/ResourceCache.h"

namespace Trinity
{
	bool SceneRenderer::prepare(Scene& scene, ResourceCache& cache)
	{
		mSceneData.scene = &scene;
		mSceneData.cache = &cache;

		if (!updateSceneData())
		{
			LogError("SceneRenderer::updateSceneData() failed!!");
			return false;
		}

		auto meshes = mSceneData.scene->getComponents<Mesh>();
		for (auto& mesh : meshes)
		{
			const auto& subMeshes = mesh->getSubMeshes();
			for (auto* subMesh : subMeshes)
			{
				RenderData renderData{};
				if (!setupRenderData(mesh, subMesh, renderData))
				{
					LogError("setupRenderData() failed!!");
					return false;
				}

				mRenderers.push_back(std::move(renderData));
			}
		}

		return true;
	}

	void SceneRenderer::setCamera(const std::string& nodeName)
	{
		auto cameraNode = mSceneData.scene->findNode(nodeName);
		if (!cameraNode)
		{
			LogWarning("Camera node '%s' not found. Looking for 'default_camera' node", nodeName.c_str());
			cameraNode = mSceneData.scene->findNode("default_camera");
		}

		if (cameraNode != nullptr)
		{
			mSceneData.camera = &cameraNode->getComponent<Camera>();
		}
	}

	void SceneRenderer::draw(RenderPass& renderPass)
	{
		if (!updateSceneData())
		{
			LogError("updateSceneData() failed!!");
			return;
		}

		std::multimap<float, RenderData*> opaqueRenderers;
		std::multimap<float, RenderData*> transparentRenderers;

		getSortedRenderers(opaqueRenderers, transparentRenderers);
		renderPass.setBindGroup(kSceneBindGroupIndex, *mSceneData.sceneBindGroup);

		for (auto it = opaqueRenderers.begin(); it != opaqueRenderers.end(); it++)
		{
			auto* renderer = it->second;
			draw(renderPass, *renderer);
		}

		for (auto it = transparentRenderers.rbegin(); it != transparentRenderers.rend(); it++)
		{
			auto* renderer = it->second;
			draw(renderPass, *renderer);
		}
	}

	bool SceneRenderer::setupRenderData(Mesh* mesh, SubMesh* subMesh, RenderData& renderData)
	{
		renderData.subMesh = subMesh;
		renderData.mesh = mesh;

		if (!updateMeshData(mesh, mesh->getNode(), renderData))
		{
			LogError("updateTransformData() failed!!");
			return false;
		}

		auto& graphics = GraphicsDevice::get();
		const SwapChain& swapChain = graphics.getSwapChain();
		const Material* material = subMesh->getMaterial();
		const BindGroupLayout* materialLayout = material->getBindGroupLayout();
		const BindGroupLayout* meshLayout = renderData.meshBindGroupLayout;

		RenderPipelineProperties renderProps = {
			.shader = material->getShader(),
			.bindGroupLayouts = {
				mSceneData.sceneBindGroupLayout,
				materialLayout,
				meshLayout
			},
			.vertexLayouts = { subMesh->getVertexLayout() },
			.primitive = {
				.topology = wgpu::PrimitiveTopology::TriangleList,
				.cullMode = wgpu::CullMode::Back
			}
		};

		if (material->getAlphaMode() == AlphaMode::Blend)
		{
			renderProps.colorTargets = {{
				.format = swapChain.getColorFormat(),
				.blendState = wgpu::BlendState {
					.color = {
						.srcFactor = wgpu::BlendFactor::SrcAlpha,
						.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha
					},
					.alpha = {
						.srcFactor = wgpu::BlendFactor::OneMinusSrcAlpha
					}
				}
			}};
		}
		else
		{
			renderProps.colorTargets = {{
				.format = swapChain.getColorFormat()
			}};
		}

		wgpu::TextureFormat depthFormat = swapChain.getDepthFormat();
		if (depthFormat != wgpu::TextureFormat::Undefined)
		{
			renderProps.depthStencil = DepthStencilState{
				.format = depthFormat,
				.depthWriteEnabled = true,
				.depthCompare = wgpu::CompareFunction::Less
			};
		}

		auto pipeline = std::make_unique<RenderPipeline>();
		if (!pipeline->create(renderProps))
		{
			LogError("RenderPipeline::create() failed!!");
			return false;
		}

		renderData.pipeline = pipeline.get();
		renderData.materialBindGroup = material->getBindGroup();
		mSceneData.cache->addResource(std::move(pipeline));

		return true;
	}

	bool SceneRenderer::updateMeshData(Mesh* mesh, Node* node, RenderData& renderData)
	{
		if (renderData.transformBuffer == nullptr)
		{
			TransformBufferData transformData{};
			if (node != nullptr)
			{
				transformData.transform = node->getTransform().getMatrix();
				transformData.rotation = glm::transpose(glm::inverse(transformData.transform));
			}

			auto transformBuffer = std::make_unique<UniformBuffer>();
			if (!transformBuffer->create(sizeof(TransformBufferData), &transformData))
			{
				LogError("UniformBuffer::create() failed!!");
				return false;
			}

			std::vector<BindGroupLayoutItem> meshLayoutItems = {
				{
					.binding = 0,
					.shaderStages = wgpu::ShaderStage::Vertex,
					.bindingLayout = BufferBindingLayout {
						.type = wgpu::BufferBindingType::Uniform,
						.minBindingSize = sizeof(TransformBufferData)
					}
				}
			};

			std::vector<BindGroupItem> meshItems = {
				{
					.binding = 0,
					.size = sizeof(TransformBufferData),
					.resource = BufferBindingResource(*transformBuffer)
				}
			};

			if (renderData.mesh->isAnimated())
			{
				const auto& invBindPose = renderData.mesh->getInvBindPose();
				const auto& bindPose = renderData.mesh->getBindPose();

				auto invBindPoseBuffer = std::make_unique<StorageBuffer>();
				if (!invBindPoseBuffer->create(sizeof(glm::mat4) * (uint32_t)invBindPose.size(), invBindPose.data()))
				{
					LogError("StorageBuffer::create() failed");
					return false;
				}

				auto bindPoseBuffer = std::make_unique<StorageBuffer>();
				if (!bindPoseBuffer->create(sizeof(glm::mat4) * (uint32_t)bindPose.size(), bindPose.data()))
				{
					LogError("StorageBuffer::create() failed");
					return false;
				}

				meshLayoutItems.push_back({
					.binding = 1,
					.shaderStages = wgpu::ShaderStage::Vertex,
					.bindingLayout = BufferBindingLayout {
						.type = wgpu::BufferBindingType::ReadOnlyStorage,
						.minBindingSize = (uint32_t)invBindPose.size() * (uint32_t)sizeof(glm::mat4)
					}
				});

				meshLayoutItems.push_back({
					.binding = 2,
					.shaderStages = wgpu::ShaderStage::Vertex,
					.bindingLayout = BufferBindingLayout {
						.type = wgpu::BufferBindingType::ReadOnlyStorage,
						.minBindingSize = (uint32_t)bindPose.size() * (uint32_t)sizeof(glm::mat4)
					}
				});

				meshItems.push_back({
					.binding = 1,
					.size = (uint32_t)invBindPose.size() * sizeof(glm::mat4),
					.resource = BufferBindingResource(*invBindPoseBuffer)
				});

				meshItems.push_back({
					.binding = 2,
					.size = (uint32_t)invBindPose.size() * sizeof(glm::mat4),
					.resource = BufferBindingResource(*bindPoseBuffer)
				});

				renderData.meshInvBindPoseBuffer = invBindPoseBuffer.get();
				renderData.meshBindPoseBuffer = bindPoseBuffer.get();
				mSceneData.cache->addResource(std::move(invBindPoseBuffer));
				mSceneData.cache->addResource(std::move(bindPoseBuffer));
			}

			auto bindGroupLayout = std::make_unique<BindGroupLayout>();
			if (!bindGroupLayout->create(meshLayoutItems))
			{
				LogError("BindGroupLayout::create() failed!!");
				return false;
			}

			auto bindGroup = std::make_unique<BindGroup>();
			if (!bindGroup->create(*bindGroupLayout, meshItems))
			{
				LogError("BindGroup::create() failed!!");
				return false;
			}

			renderData.transformBuffer = transformBuffer.get();
			renderData.meshBindGroup = bindGroup.get();
			renderData.meshBindGroupLayout = bindGroupLayout.get();

			mSceneData.cache->addResource(std::move(transformBuffer));
			mSceneData.cache->addResource(std::move(bindGroupLayout));
			mSceneData.cache->addResource(std::move(bindGroup));
		}
		else 
		{
			if (renderData.mesh->isAnimated())
			{
				auto& bindPose = renderData.mesh->getBindPose();
				renderData.meshBindPoseBuffer->write(0, (uint32_t)bindPose.size() *
					sizeof(glm::mat4), bindPose.data());
			}

			TransformBufferData transformData{};
			if (node != nullptr)
			{
				transformData.transform = node->getTransform().getMatrix();
				transformData.rotation = glm::transpose(glm::inverse(transformData.transform));
			}

			renderData.transformBuffer->write(0, sizeof(TransformBufferData), &transformData);
		}

		return true;
	}

	bool SceneRenderer::updateSceneData()
	{
		if (mSceneData.sceneBuffer == nullptr)
		{
			SceneBufferData bufferData{};

			auto sceneBuffer = std::make_unique<UniformBuffer>();
			if (!sceneBuffer->create(sizeof(SceneBufferData), &bufferData))
			{
				LogError("UniformBuffer::create() failed!!");
				return false;
			}

			if (!updateLights())
			{
				LogError("setupLights() failed!!");
				return false;
			}

			std::vector<BindGroupLayoutItem> sceneLayoutItems = {
				{
					.binding = 0,
					.shaderStages = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
					.bindingLayout = BufferBindingLayout {
						.type = wgpu::BufferBindingType::Uniform,
						.minBindingSize = sizeof(SceneBufferData)
					}
				}				
			};

			std::vector<BindGroupItem> sceneItems = {
				{
					.binding = 0,
					.size = sizeof(SceneBufferData),
					.resource = BufferBindingResource(*sceneBuffer)
				}				
			};

			if (mLights.size() > 0)
			{
				sceneLayoutItems.push_back({
					.binding = 1,
					.shaderStages = wgpu::ShaderStage::Fragment,
					.bindingLayout = BufferBindingLayout {
						.type = wgpu::BufferBindingType::ReadOnlyStorage,
						.minBindingSize = (uint32_t)mLights.size() * (uint32_t)sizeof(LightBufferData)
					}
				});

				sceneItems.push_back({
					.binding = 1,
					.size = (uint32_t)mLights.size() * sizeof(LightBufferData),
					.resource = BufferBindingResource(*mSceneData.lightsBuffer)
				});
			}

			auto bindGroupLayout = std::make_unique<BindGroupLayout>();
			if (!bindGroupLayout->create(sceneLayoutItems))
			{
				LogError("BindGroupLayout::create() failed!!");
				return false;
			}

			auto bindGroup = std::make_unique<BindGroup>();
			if (!bindGroup->create(*bindGroupLayout, sceneItems))
			{
				LogError("BindGroup::create() failed!!");
				return false;
			}

			mSceneData.sceneBuffer = sceneBuffer.get();
			mSceneData.sceneBindGroup = bindGroup.get();
			mSceneData.sceneBindGroupLayout = bindGroupLayout.get();

			mSceneData.cache->addResource(std::move(sceneBuffer));
			mSceneData.cache->addResource(std::move(bindGroup));
			mSceneData.cache->addResource(std::move(bindGroupLayout));
		}
		else
		{
			Camera* camera = mSceneData.camera;
			if (camera != nullptr)
			{
				SceneBufferData bufferData = {
					.view = camera->getView(),
					.projection = camera->getProjection(),
					.cameraPos = camera->getNode()->getTransform().getTranslation(),
					.numLights = (uint32_t)mLights.size()
				};

				mSceneData.sceneBuffer->write(0, sizeof(SceneBufferData), &bufferData);
			}

			for (auto& lightData : mLights)
			{
				updateLightData(lightData.light, lightData.storageIndex);
			}
		}

		return true;
	}

	bool SceneRenderer::updateLights()
	{
		auto lights = mSceneData.scene->getComponents<Light>();
		if (lights.size() > 0)
		{
			std::vector<LightBufferData> lightsData;
			for (auto* light : lights)
			{
				const auto& properties = light->getProperties();
				auto& transform = light->getNode()->getTransform();

				lightsData.push_back({
					{ transform.getTranslation(), (float)light->getLightType() },
					{ properties.color, properties.intensity },
					{ transform.getRotation() * properties.direction, properties.range },
					{ properties.innerConeAngle, properties.outerConeAngle }
				});

				mLights.push_back({ light, (uint32_t)lightsData.size() - 1 });
			}

			auto lightsBuffer = std::make_unique<StorageBuffer>();
			if (!lightsBuffer->create((uint32_t)lights.size() * sizeof(LightBufferData), lightsData.data()))
			{
				LogError("StorageBuffer::create() failed for lights!!");
				return false;
			}

			mSceneData.lightsBuffer = lightsBuffer.get();
			mSceneData.cache->addResource(std::move(lightsBuffer));
		}		

		return true;
	}

	bool SceneRenderer::updateLightData(Light* light, uint32_t index)
	{
		const auto& properties = light->getProperties();
		auto& transform = light->getNode()->getTransform();

		LightBufferData bufferData = {
			{ transform.getTranslation(), (float)light->getLightType() },
			{ properties.color, properties.intensity },
			{ transform.getRotation() * properties.direction, properties.range },
			{ properties.innerConeAngle, properties.outerConeAngle }
		};

		mSceneData.lightsBuffer->write(sizeof(LightBufferData) * index,
			sizeof(LightBufferData), &bufferData);

		return true;
	}

	void SceneRenderer::draw(RenderPass& renderPass, RenderData& renderer)
	{
		if (!updateMeshData(renderer.mesh, renderer.mesh->getNode(), renderer))
		{
			LogError("updateTransformData() failed!!");
			return;
		}

		renderPass.setPipeline(*renderer.pipeline);
		renderPass.setBindGroup(kMaterialBindGroupIndex, *renderer.materialBindGroup);
		renderPass.setBindGroup(kTransformBindGroupIndex, *renderer.meshBindGroup);
		renderPass.setVertexBuffer(0, *renderer.subMesh->getVertexBuffer());

		if (renderer.subMesh->hasIndexBuffer())
		{
			renderPass.setIndexBuffer(*renderer.subMesh->getIndexBuffer());
			renderPass.drawIndexed(renderer.subMesh->getNumIndices(), 1, 0, 0, 0);
		}
		else
		{
			renderPass.draw(renderer.subMesh->getNumVertices(), 1, 0, 0);
		}
	}

	void SceneRenderer::getSortedRenderers(std::multimap<float, RenderData*>& opaqueRenderers, 
		std::multimap<float, RenderData*>& transparentRenderers)
	{
		auto cameraTransform = mSceneData.camera->getNode()->getTransform().getWorldMatrix();
		for (auto& renderData : mRenderers)
		{
			const auto& bounds = renderData.mesh->getBounds();
			auto* node = renderData.mesh->getNode();
			auto transform = node->getTransform().getWorldMatrix();

			BoundingBox worldBounds{ bounds.min, bounds.max };
			worldBounds.transform(transform);

			float distance = glm::length(glm::vec3(cameraTransform[3]) - worldBounds.getCenter());
			if (renderData.subMesh->getMaterial()->getAlphaMode() == AlphaMode::Blend)
			{
				transparentRenderers.emplace(distance, &renderData);
			}
			else
			{
				opaqueRenderers.emplace(distance, &renderData);
			}
		}
	}
}