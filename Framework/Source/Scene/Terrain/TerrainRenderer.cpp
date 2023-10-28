#include "Scene/Terrain/TerrainRenderer.h"
#include "Scene/Terrain/Terrain.h"
#include "Scene/Terrain/TerrainMaterial.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/Light.h"
#include "Scene/Scene.h"
#include "Graphics/RenderPipeline.h"
#include "Graphics/Material.h"
#include "Graphics/BindGroup.h"
#include "Graphics/BindGroupLayout.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/StorageBuffer.h"
#include "Graphics/SwapChain.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/RenderPass.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"
#include "Core/ResourceCache.h"

namespace Trinity
{
	bool TerrainRenderer::prepare(Terrain& terrain, Scene& scene, ResourceCache& cache)
	{
		mSceneData.terrain = &terrain;
		mSceneData.scene = &scene;
		mSceneData.cache = &cache;

		if (!setupSceneData())
		{
			LogError("TerrainRenderer::setupSceneData() failed!!");
			return false;
		}

		return true;
	}

	void TerrainRenderer::setCamera(const std::string& nodeName)
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

	void TerrainRenderer::draw(RenderPass& renderPass)
	{
		if (!updateSceneData())
		{
			LogError("updateSceneData() failed!!");
			return;
		}
	}

	bool TerrainRenderer::setupSceneData()
	{
		if (!updateSceneData())
		{
			LogError("TerrainRenderer::updateSceneData() failed");
			return false;
		}

		auto& graphics = GraphicsDevice::get();
		const SwapChain& swapChain = graphics.getSwapChain();
		const Terrain* terrain = mSceneData.terrain;
		const Material* material = terrain->getMaterial();
		const BindGroupLayout* materialLayout = material->getBindGroupLayout();

		RenderPipelineProperties renderProps = {
			.shader = material->getShader(),
			.bindGroupLayouts = {
				mSceneData.sceneBindGroupLayout,
				materialLayout
			},
			.vertexBuffers = { terrain->getVertexBuffer() },
			.primitive = {
				.topology = wgpu::PrimitiveTopology::TriangleList,
				.cullMode = wgpu::CullMode::Back
			}
		};

		if (material->getAlphaMode() == AlphaMode::Blend)
		{
			renderProps.colorTargets = { {
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
			} };
		}
		else
		{
			renderProps.colorTargets = { {
				.format = swapChain.getColorFormat()
			} };
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

		mSceneData.pipeline = pipeline.get();
		mSceneData.materialBindGroup = material->getBindGroup();
		mSceneData.cache->addResource(std::move(pipeline));

		return true;
	}

	bool TerrainRenderer::updateSceneData()
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
						.minBindingSize = (uint32_t)mLights.size() * (uint32_t)sizeof(LightBufferData)
					}
				});

				sceneItems.push_back({
					.binding = 1,
					.size = (uint32_t)mLights.size() * (uint32_t)sizeof(LightBufferData),
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

	bool TerrainRenderer::setupLights()
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

	bool TerrainRenderer::updateLightData(Light* light, uint32_t index)
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
}