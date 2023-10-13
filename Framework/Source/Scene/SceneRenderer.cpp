#include "Scene/SceneRenderer.h"
#include "Scene/Components/Mesh.h"
#include "Scene/Components/SubMesh.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/Light.h"
#include "Scene/Node.h"
#include "Scene/Scene.h"
#include "Graphics/PBRMaterial.h"
#include "Graphics/RenderPipeline.h"
#include "Graphics/Material.h"
#include "Graphics/BindGroup.h"
#include "Graphics/BindGroupLayout.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/StorageBuffer.h"
#include "Graphics/ResourceCache.h"
#include "Graphics/SwapChain.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/RenderPass.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"

namespace Trinity
{
	bool SceneRenderer::prepare(Scene& scene)
	{
		mSceneData.scene = &scene;
		Assert(mSceneData.scene != nullptr, "Scene cannot be null here!!");

		if (!updateSceneData())
		{
			LogError("updateSceneData() failed!!");
			return false;
		}

		auto meshes = mSceneData.scene->getComponents<Mesh>();
		for (auto& mesh : meshes)
		{
			const auto& subMeshes = mesh->getSubMeshes();
			const auto& nodes = mesh->getNodes();

			for (auto* node : nodes)
			{
				for (auto* subMesh : subMeshes)
				{
					RenderData renderData{};
					if (!setupRenderData(node, mesh, subMesh, renderData))
					{
						LogError("setupRenderData() failed!!");
						return false;
					}

					mRenderers.push_back(std::move(renderData));
				}
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

	bool SceneRenderer::setupRenderData(Node* node, Mesh* mesh, SubMesh* subMesh, RenderData& renderData)
	{
		if (!updateTransformData(node, renderData))
		{
			LogError("updateTransformData() failed!!");
			return false;
		}

		auto& graphics = GraphicsDevice::get();
		const SwapChain& swapChain = graphics.getSwapChain();
		const Material* material = subMesh->getMaterial();
		const BindGroupLayout* materialLayout = material->getBindGroupLayout();
		const BindGroupLayout* transformLayout = renderData.transformBindGroupLayout;

		RenderPipelineProperties renderProps = {
			.shader = material->getShader(),
			.bindGroupLayouts = {
				mSceneData.sceneBindGroupLayout,
				materialLayout,
				transformLayout
			},
			.vertexBuffers = { subMesh->getVertexBuffer("vertexBuffer") },
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

		renderData.subMesh = subMesh;
		renderData.mesh = mesh;
		renderData.node = node;
		renderData.pipeline = pipeline.get();
		renderData.materialBindGroup = material->getBindGroup();
		mSceneData.scene->getResourceCache().addResource(std::move(pipeline));

		return true;
	}

	bool SceneRenderer::updateTransformData(Node* node, RenderData& renderData)
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

			const std::vector<BindGroupLayoutItem> transformLayoutItems = {
				{
					.binding = 0,
					.shaderStages = wgpu::ShaderStage::Vertex,
					.bindingLayout = BufferBindingLayout {
						.type = wgpu::BufferBindingType::Uniform,
						.minBindingSize = sizeof(TransformBufferData)
					}
				}
			};

			std::vector<BindGroupItem> transformItems = {
				{
					.binding = 0,
					.size = sizeof(TransformBufferData),
					.resource = BufferBindingResource(*transformBuffer)
				}
			};

			auto bindGroupLayout = std::make_unique<BindGroupLayout>();
			if (!bindGroupLayout->create(transformLayoutItems))
			{
				LogError("BindGroupLayout::create() failed!!");
				return false;
			}

			auto bindGroup = std::make_unique<BindGroup>();
			if (!bindGroup->create(*bindGroupLayout, transformItems))
			{
				LogError("BindGroup::create() failed!!");
				return false;
			}

			renderData.transformBuffer = transformBuffer.get();
			renderData.transformBindGroup = bindGroup.get();
			renderData.transformBindGroupLayout = bindGroupLayout.get();

			mSceneData.scene->getResourceCache().addResource(std::move(transformBuffer));
			mSceneData.scene->getResourceCache().addResource(std::move(bindGroupLayout));
			mSceneData.scene->getResourceCache().addResource(std::move(bindGroup));
		}
		else 
		{
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

			if (!setupLights())
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
						.minBindingSize = (uint32_t)mLights.size() * sizeof(LightBufferData)
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

			mSceneData.scene->getResourceCache().addResource(std::move(sceneBuffer));
			mSceneData.scene->getResourceCache().addResource(std::move(bindGroup));
			mSceneData.scene->getResourceCache().addResource(std::move(bindGroupLayout));
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
		}

		return true;
	}

	bool SceneRenderer::setupLights()
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
			mSceneData.scene->getResourceCache().addResource(std::move(lightsBuffer));
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
		if (!updateTransformData(renderer.node, renderer))
		{
			LogError("updateTransformData() failed!!");
			return;
		}

		renderPass.setPipeline(*renderer.pipeline);
		renderPass.setBindGroup(kMaterialBindGroupIndex, *renderer.materialBindGroup);
		renderPass.setBindGroup(kTransformBindGroupIndex, *renderer.transformBindGroup);
		renderPass.setVertexBuffer(0, *renderer.subMesh->getVertexBuffer("vertexBuffer"));

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
			auto transform = renderData.node->getTransform().getWorldMatrix();

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