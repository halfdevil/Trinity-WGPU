#include "Scene/Terrain/TerrainRenderer.h"
#include "Scene/Terrain/Terrain.h"
#include "Scene/Terrain/TerrainMaterial.h"
#include "Scene/Terrain/HeightMap.h"
#include "Scene/Terrain/QuadTree.h"
#include "Scene/Terrain/QuadGridMesh.h"
#include "Scene/Components/PerspectiveCamera.h"
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
		mSceneData.scene = &scene;
		mSceneData.terrain = &terrain;
		mSceneData.cache = &cache;

		if (!setupGridMesh())
		{
			LogError("TerrainRenderer::setupGridMeshes() failed");
			return false;
		}

		if (!setupQuadTree())
		{
			LogError("TerrainRenderer::setupQuadTree() failed");
			return false;
		}

		if (!setupLights())
		{
			LogError("TerrainRenderer::setupLights() failed");
			return false;
		}

		if (!setupTerrainData())
		{
			LogError("TerrainRenderer::setupTerrainData() failed");
			return false;
		}

		if (!setupQuadData())
		{
			LogError("TerrainRenderer::setupQuadData() failed");
			return false;
		}

		if (!setupSceneData())
		{
			LogError("TerrainRenderer::setupSceneData() failed");
			return false;
		}

		if (!setupPipeline())
		{
			LogError("TerrainRenderer::setupPipeline() failed");
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

	void TerrainRenderer::draw(const RenderPass& renderPass)
	{
		updateSceneData();
		updateLights();

		renderPass.setBindGroup(kSceneBindGroupIndex, *mSceneData.sceneBindGroup);
		renderPass.setPipeline(*mSceneData.pipeline);
		renderPass.setBindGroup(kMaterialBindGroupIndex, *mSceneData.materialBindGroup);
		renderPass.setVertexBuffer(0, *mGridMesh->getVertexBuffer());
		renderPass.setIndexBuffer(*mGridMesh->getIndexBuffer());

		auto* terrain = mSceneData.terrain;
		auto* camera = (PerspectiveCamera*)mSceneData.camera;

		mQuadTree->selectLOD({
			.selected = mSelectedLOD.get(),
			.camera = mSceneData.camera,
			.visibilityDistance = camera->getFarPlane() * 0.95f,
			.lodDistanceRatio = terrain->getLODDistanceRatio()
		});

		for (uint32_t idx = mSelectedLOD->minLODLevel; idx <= mSelectedLOD->maxLODLevel; idx++)
		{
			drawSelection(renderPass, idx);
		}
	}

	bool TerrainRenderer::setupPipeline()
	{
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
			.vertexLayouts = { mSceneData.gridMeshLayout },
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

	bool TerrainRenderer::setupSceneData()
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

		sceneLayoutItems.push_back({
			.binding = 2,
			.shaderStages = wgpu::ShaderStage::Vertex,
			.bindingLayout = BufferBindingLayout {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(TerrainBufferData)
			}
		});

		sceneLayoutItems.push_back({
			.binding = 3,
			.shaderStages = wgpu::ShaderStage::Vertex,
			.bindingLayout = BufferBindingLayout {
				.type = wgpu::BufferBindingType::ReadOnlyStorage,
				.minBindingSize = sizeof(QuadBufferData)
			}
		});

		sceneItems.push_back({
			.binding = 2,
			.size = sizeof(TerrainBufferData),
			.resource = BufferBindingResource(*mSceneData.terrainBuffer)
		});

		sceneItems.push_back({
			.binding = 3,
			.size = sizeof(TerrainBufferData),
			.resource = BufferBindingResource(*mSceneData.quadBuffer)
		});

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

	bool TerrainRenderer::setupTerrainData()
	{
		auto* terrain = mSceneData.terrain;
		auto* heightMap = terrain->getHeightMap();
		const auto& mapDims = terrain->getMapDimension();
		const auto& size = heightMap->getSize();
		auto mapMax = mapDims.getMax();

		TerrainBufferData terrainBufferData = {
			.terrainScale = {
				mapDims.size.x,
				mapDims.size.y,
				mapDims.size.z,
				0.0f
			},
			.terrainOffset = {
				mapDims.min.x,
				mapDims.min.y,
				mapDims.min.z,
				0.0f
			},
			.quadWorldMax = {
				mapMax.x,
				mapMax.y
			},
			.worldToTextureScale = {
				(size.x - 1.0f) / (float)size.x,
				(size.y - 1.0f) / (float)size.y
			},
			.gridDimension = {
				(float)mGridMesh->getDimension(),
				mGridMesh->getDimension() * 0.5f,
				2.0f / mGridMesh->getDimension(),
				0.0f
			},
			.heightMapTextureInfo = {
				size.x,
				size.y,
				1.0f / (float)size.x,
				1.0f / (float)size.y
			}
		};

		auto terrainBuffer = std::make_unique<UniformBuffer>();
		if (!terrainBuffer->create(sizeof(TerrainBufferData), &terrainBufferData))
		{
			LogError("UniformBuffer::create() failed");
			return false;
		}

		mSceneData.terrainBuffer = terrainBuffer.get();
		mSceneData.cache->addResource(std::move(terrainBuffer));

		return true;
	}

	bool TerrainRenderer::setupQuadData()
	{
		QuadBufferData bufferData{};

		auto quadBuffer = std::make_unique<UniformBuffer>();
		if (!quadBuffer->create(sizeof(QuadBufferData), &bufferData))
		{
			LogError("UniformBuffer::create() failed");
			return false;
		}

		mSceneData.quadBuffer = quadBuffer.get();
		mSceneData.cache->addResource(std::move(quadBuffer));

		return true;
	}

	bool TerrainRenderer::setupGridMesh()
	{
		auto* terrain = mSceneData.terrain;
		auto dimension = terrain->getLeafNodeSize() * terrain->getGridResolutionMult();

		auto vertexLayout = std::make_unique<VertexLayout>();
		vertexLayout->setAttributes({
			{ wgpu::VertexFormat::Float32x3, 0, 0 }
		});
		
		mGridMesh = std::make_unique<QuadGridMesh>();
		if (!mGridMesh->create(dimension, *vertexLayout))
		{
			LogError("QuadGridMesh::create() failed");
			return false;
		}

		mSceneData.gridMeshLayout = vertexLayout.get();
		mSceneData.cache->addResource(std::move(vertexLayout));

		return true;
	}

	bool TerrainRenderer::setupQuadTree()
	{
		auto* terrain = mSceneData.terrain;	
		mQuadTree = std::make_unique<QuadTree>();

		if (!mQuadTree->create(terrain->getMapDimension(), *terrain->getHeightMap(), 
			terrain->getLeafNodeSize(), terrain->getNumLODs()))
		{
			LogError("QuadTree::create() failed");
			return false;
		}

		mSelectedLOD = std::make_unique<SelectedLOD>(kMaxSelectionNodes);
		return true;
	}

	void TerrainRenderer::updateSceneData()
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

	void TerrainRenderer::updateTerrainData(const TerrainBufferData& bufferData)
	{
		mSceneData.terrainBuffer->write(0, sizeof(TerrainBufferData), &bufferData);
	}

	void TerrainRenderer::updateQuadData(const QuadBufferData& bufferData)
	{
		mSceneData.quadBuffer->write(0, sizeof(QuadBufferData), &bufferData);
	}

	void TerrainRenderer::updateLights()
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
			}

			mSceneData.lightsBuffer->write(0, sizeof(LightBufferData) * 
				(uint32_t)lightsData.size(), lightsData.data());
		}		
	}

	void TerrainRenderer::drawSelection(const RenderPass& renderPass, uint32_t lodLevel)
	{
		auto* terrain = mSceneData.terrain;
		auto* heightMap = terrain->getHeightMap();
		const auto& size = heightMap->getSize();
		const auto& mapDims = mSceneData.terrain->getMapDimension();
		const auto* selectionBuffer = mSelectedLOD->selectedNodes.data();

		int32_t prevMorphConstLevel = -1;
		QuadBufferData quadData;

		for (uint32_t idx = 0; idx < mSelectedLOD->numSelections; idx++)
		{
			const auto& selectedNode = selectionBuffer[idx];
			if (lodLevel != selectedNode.lodLevel)
			{
				continue;
			}

			if (prevMorphConstLevel != lodLevel)
			{
				prevMorphConstLevel = lodLevel;
				quadData.morphConsts = mQuadTree->getMorphConstant(lodLevel);
			}

			bool drawFull = selectedNode.tl && selectedNode.tr && selectedNode.bl && selectedNode.br;
			auto nodeBox = selectedNode.getBoundingBox(size, mapDims);

			quadData.quadScale = {
				nodeBox.max.x - nodeBox.min.x,
				nodeBox.max.y - nodeBox.min.y,
				(float)lodLevel,
				0.0f
			};

			quadData.quadOffset = {
				nodeBox.min.x,
				nodeBox.min.y,
				(nodeBox.min.z + nodeBox.max.z) * 0.5f,
				0.0f
			};

			updateQuadData(quadData);

			auto gridDims = mGridMesh->getDimension();
			auto totalIndices = gridDims * 2 * 3;
			auto numIndices = ((gridDims + 1) / 2) * ((gridDims + 1) / 2) * 2 * 3;

			if (drawFull)
			{
				renderPass.drawIndexed(totalIndices);
			}
			else
			{
				if (selectedNode.tl)
				{
					renderPass.drawIndexed(numIndices);
				}

				if (selectedNode.tr)
				{
					renderPass.drawIndexed(numIndices, 1, mGridMesh->getIndexEndTL());
				}

				if (selectedNode.bl)
				{
					renderPass.drawIndexed(numIndices, 1, mGridMesh->getIndexEndTR());
				}

				if (selectedNode.br)
				{
					renderPass.drawIndexed(numIndices, 1, mGridMesh->getIndexEndBL());
				}
			}
		}
	}
}