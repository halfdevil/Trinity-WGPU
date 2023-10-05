#include "Scene/SceneRenderer.h"
#include "Scene/Components/Mesh.h"
#include "Scene/Components/SubMesh.h"
#include "Scene/Components/PerspectiveCamera.h"
#include "Scene/Node.h"
#include "Scene/Scene.h"
#include "Graphics/PBRMaterial.h"
#include "Graphics/RenderPipeline.h"
#include "Graphics/Material.h"
#include "Graphics/BindGroup.h"
#include "Graphics/BindGroupLayout.h"
#include "Graphics/UniformBuffer.h"
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

		return false;
	}

	void SceneRenderer::draw(RenderPass* renderPass)
	{
		if (!updateSceneData())
		{
			LogError("updateSceneData() failed!!");
			return;
		}

		renderPass->setBindGroup(kSceneBindGroupIndex, *mSceneData.sceneBindGroup);

		for (auto& renderer : mRenderers)
		{
			if (!updateTransformData(renderer.node, renderer))
			{
				LogError("updateTransformData() failed!!");
				return;
			}

			renderPass->setPipeline(*renderer.pipeline);
			renderPass->setBindGroup(kMaterialBindGroupIndex, *renderer.materialBindGroup);
			renderPass->setBindGroup(kTransformBindGroupIndex, *renderer.transformBindGroup);
			renderPass->setVertexBuffer(0, *renderer.subMesh->getVertexBuffer("vertexBuffer"));
			
			if (renderer.subMesh->hasIndexBuffer())
			{
				renderPass->setIndexBuffer(*renderer.subMesh->getIndexBuffer());
				renderPass->drawIndexed(renderer.subMesh->getNumIndices(), 1, 0, 0, 0);
			}
			else
			{
				renderPass->draw(renderer.subMesh->getNumVertices(), 1, 0, 0);
			}
		}
	}

	bool SceneRenderer::setupRenderData(Node* node, Mesh* mesh, SubMesh* subMesh, RenderData& renderData)
	{
		if (!updateTransformData(node, renderData))
		{
			LogError("updateTransformData() failed!!");
			return false;
		}

		GraphicsDevice& graphics = GraphicsDevice::get();
		const SwapChain& swapChain = graphics.getSwapChain();
		const Material* material = subMesh->getMaterial();
		const BindGroupLayout* transformLayout = renderData.transformBindGroupLayout;

		RenderPipelineProperties renderProps = {
			.shader = material->getShader(),
			.bindGroupLayouts = {
				mSceneData.sceneBindGroupLayout,
				material->getBindGroupLayout(),
				transformLayout
			},
			.vertexBuffers = { subMesh->getVertexBuffer("vertexBuffer") },
			.colorTargets = {{
				.format = swapChain.getColorFormat(),
				.blendState = wgpu::BlendState {
					.color = {
						.operation = wgpu::BlendOperation::Add,
						.srcFactor = wgpu::BlendFactor::SrcAlpha,
						.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha
					}
				}
			}},
			.primitive = {
				.topology = wgpu::PrimitiveTopology::TriangleList,
				.cullMode = wgpu::CullMode::Back
			}
		};

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
			TransformBufferData transform{};
			if (node != nullptr)
			{
				transform.transform = node->getTransform().getMatrix();
				transform.rotation = glm::transpose(glm::inverse(transform.transform));
			}

			auto transformBuffer = std::make_unique<UniformBuffer>();
			if (!transformBuffer->create(sizeof(TransformBufferData), &transform))
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
			TransformBufferData transform{};
			if (node != nullptr)
			{
				transform.transform = node->getTransform().getMatrix();
				transform.rotation = glm::transpose(glm::inverse(transform.transform));
			}

			renderData.transformBuffer->write(0, sizeof(TransformBufferData), &transform);
		}

		return true;
	}

	bool SceneRenderer::updateSceneData()
	{
		if (mSceneData.camera == nullptr)
		{
			auto cameras = mSceneData.scene->getResourceCache().getResources<PerspectiveCamera>();
			Assert(cameras.size() != 0, "No valid camera attached to scene");

			PerspectiveCamera* camera = cameras[0];
			SceneBufferData bufferData = {
				.view = camera->getView(),
				.projection = camera->getProjection()
			};

			auto sceneBuffer = std::make_unique<UniformBuffer>();
			if (!sceneBuffer->create(sizeof(SceneBufferData), &bufferData))
			{
				LogError("UniformBuffer::create() failed!!");
				return false;
			}

			const std::vector<BindGroupLayoutItem> sceneLayoutItems = {
				{
					.binding = 0,
					.shaderStages = wgpu::ShaderStage::Vertex,
					.bindingLayout = BufferBindingLayout {
						.type = wgpu::BufferBindingType::Uniform,
						.minBindingSize = sizeof(TransformBufferData)
					}
				}
			};

			std::vector<BindGroupItem> sceneItems = {
				{
					.binding = 0,
					.size = sizeof(TransformBufferData),
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

			mSceneData.camera = camera;
			mSceneData.sceneBuffer = sceneBuffer.get();
			mSceneData.sceneBindGroup = bindGroup.get();
			mSceneData.sceneBindGroupLayout = bindGroupLayout.get();

			mSceneData.scene->getResourceCache().addResource(std::move(sceneBuffer));
			mSceneData.scene->getResourceCache().addResource(std::move(bindGroup));
			mSceneData.scene->getResourceCache().addResource(std::move(bindGroupLayout));
		}
		else
		{
			PerspectiveCamera* camera = mSceneData.camera;
			SceneBufferData bufferData = {
				.view = camera->getView(),
				.projection = camera->getProjection()
			};

			mSceneData.sceneBuffer->write(0, sizeof(SceneBufferData), &bufferData);
		}

		return true;
	}
}