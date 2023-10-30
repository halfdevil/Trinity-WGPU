#include "Scene/Skybox/SkyboxRenderer.h"
#include "Scene/Skybox/Skybox.h"
#include "Scene/Skybox/SkyboxMaterial.h"
#include "Scene/Components/Camera.h"
#include "Graphics/RenderPipeline.h"
#include "Graphics/Material.h"
#include "Graphics/BindGroup.h"
#include "Graphics/BindGroupLayout.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/SwapChain.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/RenderPass.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"
#include "Core/ResourceCache.h"

namespace Trinity
{
	bool SkyboxRenderer::prepare(Skybox& skybox, Camera& camera, ResourceCache& cache)
	{
		mSceneData.skybox = &skybox;
		mSceneData.camera = &camera;
		mSceneData.cache = &cache;

		if (!setupSceneData())
		{
			LogError("TerrainRenderer::setupSceneData() failed!!");
			return false;
		}

		return true;
	}

	void SkyboxRenderer::setCamera(Camera& camera)
	{
		mSceneData.camera = &camera;
	}

	void SkyboxRenderer::draw(RenderPass& renderPass)
	{
		if (!updateSceneData())
		{
			LogError("updateSceneData() failed!!");
			return;
		}

		renderPass.setBindGroup(kSceneBindGroupIndex, *mSceneData.sceneBindGroup);
		renderPass.setPipeline(*mSceneData.pipeline);
		renderPass.setBindGroup(kMaterialBindGroupIndex, *mSceneData.materialBindGroup);
		renderPass.setVertexBuffer(0, *mSceneData.skybox->getVertexBuffer());
		renderPass.setIndexBuffer(*mSceneData.skybox->getIndexBuffer());
		renderPass.drawIndexed(mSceneData.skybox->getIndexBuffer()->getNumIndices(), 1, 0, 0, 0);
	}

	bool SkyboxRenderer::setupSceneData()
	{
		if (!updateSceneData())
		{
			LogError("TerrainRenderer::updateSceneData() failed");
			return false;
		}

		auto& graphics = GraphicsDevice::get();
		const SwapChain& swapChain = graphics.getSwapChain();
		const Skybox* skybox = mSceneData.skybox;
		const Material* material = skybox->getMaterial();
		const BindGroupLayout* materialLayout = material->getBindGroupLayout();

		RenderPipelineProperties renderProps = {
			.shader = material->getShader(),
			.bindGroupLayouts = {
				mSceneData.sceneBindGroupLayout,
				materialLayout
			},
			.vertexBuffers = { skybox->getVertexBuffer() },
			.colorTargets = {{
				.format = swapChain.getColorFormat()
			}},
			.primitive = {
				.topology = wgpu::PrimitiveTopology::TriangleList,
				.cullMode = wgpu::CullMode::None
			}
		};

		wgpu::TextureFormat depthFormat = swapChain.getDepthFormat();
		if (depthFormat != wgpu::TextureFormat::Undefined)
		{
			renderProps.depthStencil = DepthStencilState{
				.format = depthFormat,
				.depthWriteEnabled = false
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

	bool SkyboxRenderer::updateSceneData()
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
			Skybox* skybox = mSceneData.skybox;

			if (camera != nullptr)
			{
				SceneBufferData bufferData = {
					.view = camera->getView(),
					.projection = camera->getProjection(),
					.transform = glm::translate(glm::mat4(1.0f), skybox->getPosition())
				};

				mSceneData.sceneBuffer->write(0, sizeof(SceneBufferData), &bufferData);
			}
		}

		return true;
	}
}