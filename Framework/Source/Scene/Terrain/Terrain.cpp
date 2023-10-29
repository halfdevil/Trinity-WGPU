#include "Scene/Terrain/Terrain.h"
#include "Scene/Terrain/HeightMap.h"
#include "Scene/Terrain/TerrainMaterial.h"
#include "Scene/Components/Transform.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/StorageBuffer.h"
#include "Graphics/BindGroupLayout.h"
#include "Graphics/BindGroup.h"
#include "Graphics/RenderPipeline.h"
#include "VFS/FileSystem.h"
#include "Core/ResourceCache.h"
#include "Core/Logger.h"

#define _USE_MATH_DEFINES
#include <cmath>

namespace Trinity
{
	bool Terrain::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		return Resource::create(fileName, cache, loadContent);
	}

	void Terrain::destroy()
	{
		Resource::destroy();
		mPatches.clear();
		mDistanceThreshold.clear();
	}

	bool Terrain::write()
	{
		return Resource::write();
	}

	bool Terrain::load(ResourceCache& cache, HeightMap& heightMap, Material& material, uint32_t maxLOD, 
		uint32_t patchSize, float cellSpacing)
	{
		setHeightMap(heightMap);
		setMaterial(material);
		setMaxLOD(maxLOD);
		setPatchSize(patchSize);
		setCellSpacing(cellSpacing);

		if (!setupDeviceObjects(cache))
		{
			LogError("Terrain::setupDeviceObjects() failed");
			return false;
		}

		return true;
	}

	void Terrain::update(const Transform& transform)
	{
		preDrawCalculations(transform);
	}

	std::type_index Terrain::getType() const
	{
		return typeid(Terrain);
	}

	void Terrain::setHeightMap(HeightMap& heightMap)
	{
		mHeightMap = &heightMap;
	}

	void Terrain::setMaterial(Material& material)
	{
		mMaterial = &material;
	}

	void Terrain::setMaxLOD(uint32_t maxLOD)
	{
		mMaxLOD = maxLOD;
	}

	void Terrain::setPatchSize(uint32_t patchSize)
	{
		mPatchSize = patchSize;
		mCalcPatchSize = patchSize - 1;
	}

	void Terrain::setCellSpacing(float cellSpacing)
	{
		mCellSpacing = cellSpacing;
	}

	void Terrain::preDrawCalculations(const Transform& transform)
	{
		preDrawLODCalculations(transform);
		preDrawIndicesCalculations();
	}

	void Terrain::preDrawLODCalculations(const Transform& transform)
	{
		const auto& position = transform.getTranslation();
		const auto& count = mNumPatches * mNumPatches;

		for (uint32_t idx = 0; idx < count; idx++)
		{
			auto& patch = mPatches[idx];
			auto a = position - mPatches[idx].center;
			
			float distance = glm::dot(a, a);
			patch.currentLOD = 0;

			for (uint32_t lod = mMaxLOD - 1; lod > 0; lod--)
			{
				if (distance >= mDistanceThreshold[lod])
				{
					patch.currentLOD = lod;
					break;
				}
			}
		}
	}

	void Terrain::preDrawIndicesCalculations()
	{
		if (!mIndexBuffer)
		{
			return;
		}

		uint32_t index{ 0 };
		mIndicesToDraw = 0;
		
		for (uint32_t idx = 0; idx < mNumPatches; idx++)
		{
			for (uint32_t jdx = 0; jdx < mNumPatches; jdx++)
			{
				if (mPatches[index].currentLOD >= 0)
				{
					uint32_t x{ 0 };
					uint32_t z{ 0 };
					uint32_t step = 1 << mPatches[index].currentLOD;

					while (z < mCalcPatchSize)
					{
						uint32_t index11 = getIndex(jdx, idx, index, x, z);
						uint32_t index21 = getIndex(jdx, idx, index, x + step, z);
						uint32_t index12 = getIndex(jdx, idx, index, x, z + step);
						uint32_t index22 = getIndex(jdx, idx, index, x + step, z + step);

						mIndices[mIndicesToDraw++] = index12;
						mIndices[mIndicesToDraw++] = index11;
						mIndices[mIndicesToDraw++] = index22;
						mIndices[mIndicesToDraw++] = index22;
						mIndices[mIndicesToDraw++] = index11;
						mIndices[mIndicesToDraw++] = index21;

						x += step;
						if (x >= mCalcPatchSize)
						{
							x = 0;
							z += step;
						}
					}
				}

				index++;
			}
		}

		mIndexBuffer->write(0, sizeof(uint32_t) * mIndicesToDraw, mIndices.data());
	}

	uint32_t Terrain::getIndex(uint32_t patchX, uint32_t patchZ, uint32_t patchIndex, uint32_t x, uint32_t z) const
	{
		auto& patch = mPatches[patchIndex];

		if (z == 0)
		{
			if (patch.top && patch.currentLOD < patch.top->currentLOD &&
				(x % (1 << patch.top->currentLOD)) != 0)
			{
				x -= x % (1 << patch.top->currentLOD);
			}
		}
		else if (z == mCalcPatchSize)
		{
			if (patch.bottom && patch.currentLOD < patch.bottom->currentLOD &&
				(x % (1 << patch.bottom->currentLOD)) != 0)
			{
				x -= x % (1 << patch.bottom->currentLOD);
			}
		}

		if (x == 0)
		{
			if (patch.left && patch.currentLOD < patch.left->currentLOD &&
				(z % (1 << patch.left->currentLOD)) != 0)
			{
				z -= z % (1 << patch.left->currentLOD);
			}
		}
		else if (x == mCalcPatchSize)
		{
			if (patch.right && patch.currentLOD < patch.right->currentLOD &&
				(z % (1 << patch.right->currentLOD)) != 0)
			{
				z -= z % (1 << patch.right->currentLOD);
			}
		}

		if (z >= mCalcPatchSize)
		{
			z = mCalcPatchSize;
		}

		if (x >= mCalcPatchSize)
		{
			x = mCalcPatchSize;
		}

		return (z + (mCalcPatchSize * patchZ)) * mSize + (x + (mCalcPatchSize * patchX));
	}

	bool Terrain::setupDeviceObjects(ResourceCache& cache)
	{
		mSize = mHeightMap->getWidth();
		mMaxLOD = (uint32_t)std::log2(mCalcPatchSize);

		auto vertexLayout = std::make_unique<VertexLayout>();
		vertexLayout->setAttributes({
			{ wgpu::VertexFormat::Float32x3, 0, 0 },
			{ wgpu::VertexFormat::Float32x3, 12, 1 },
			{ wgpu::VertexFormat::Float32x2, 24, 2 },
		});

		const auto& heightData = mHeightMap->getData();

		uint32_t numVertices = mSize * mSize;
		std::vector<Vertex> vertices(numVertices);

		float x{ 0.0f };
		float x2{ 0.0f };
		float td{ 1.0f / (float)(mSize - 1) };

		for (uint32_t idx = 0; idx < mSize; idx++)
		{
			float z{ 0.0f };
			float z2{ 0.0f };

			for (uint32_t jdx = 0; jdx < mSize; jdx++)
			{
				auto& vertex = vertices[idx * mSize + jdx];

				vertex.position = glm::vec3{ x * mCellSpacing, heightData[idx * mSize + jdx], z * mCellSpacing };
				vertex.normal = { 0.0f, 1.0f, 0.0f };
				vertex.uv = { 1.0f - x2, z2 };

				z++;
				z2 += td;
			}

			x++;
			x2 += td;
		}

		calculateNormals(vertices);
		calculateDistanceThresholds();
		createPatches();
		calculatePatchData(vertices);

		auto vertexBuffer = std::make_unique<VertexBuffer>();
		if (!vertexBuffer->create(*vertexLayout, numVertices, vertices.data()))
		{
			LogError("VertexBuffer::create() failed");
			return false;
		}

		uint32_t numIndices = mNumPatches * mNumPatches * mCalcPatchSize * mCalcPatchSize * 6;
		mIndices.resize(numIndices);

		auto indexBuffer = std::make_unique<IndexBuffer>();
		if (!indexBuffer->create(wgpu::IndexFormat::Uint32, numIndices))
		{
			LogError("IndexBuffer::create() failed");
			return false;
		}

		mIndicesToDraw = 0;
		preDrawIndicesCalculations();

		return false;
	}

	void Terrain::calculateNormals(std::vector<Vertex>& vertices)
	{
		uint32_t count{ 0 };
		glm::vec3 a, b, c, t;

		for (uint32_t x = 0; x < mSize; x++)
		{
			for (uint32_t z = 0; z < mSize; z++)
			{
				count = 0;
				glm::vec3 normal{ 0.0f };

				if (x > 0 && z > 0)
				{
					a = vertices[(x - 1) * mSize + z - 1].position;
					b = vertices[(x - 1) * mSize + z].position;
					c = vertices[x * mSize + z].position;
					t = glm::normalize(glm::cross((b - a), (c - a)));					
					normal += t;

					a = vertices[(x - 1) * mSize + z - 1].position;
					b = vertices[x * mSize + z - 1].position;
					c = vertices[x * mSize + z].position;
					t = glm::normalize(glm::cross((b - a), (c - a)));
					normal += t;

					count += 2;
				}

				if (x > 0 && z < mSize - 1)
				{
					a = vertices[(x - 1) * mSize + z].position;
					b = vertices[(x - 1) * mSize + z + 1].position;
					c = vertices[x * mSize + z + 1].position;
					t = glm::normalize(glm::cross((b - a), (c - a)));
					normal += t;

					a = vertices[(x - 1) * mSize + z].position;
					b = vertices[x * mSize + z + 1].position;
					c = vertices[x * mSize + z].position;
					t = glm::normalize(glm::cross((b - a), (c - a)));
					normal += t;

					count += 2;
				}

				if (x < mSize - 1 && z < mSize - 1)
				{
					a = vertices[x * mSize + z + 1].position;
					b = vertices[x * mSize + z].position;
					c = vertices[(x + 1) * mSize + z + 1].position;
					t = glm::normalize(glm::cross((b - a), (c - a)));
					normal += t;

					a = vertices[x * mSize + z + 1].position;
					b = vertices[(x + 1) * mSize + z + 1].position;
					c = vertices[(x + 1) * mSize + z].position;
					t = glm::normalize(glm::cross((b - a), (c - a)));
					normal += t;

					count += 2;
				}

				if (x < mSize - 1 && z > 0)
				{
					a = vertices[x * mSize + z - 1].position;
					b = vertices[x * mSize + z].position;
					c = vertices[(x + 1) * mSize + z].position;
					t = glm::normalize(glm::cross((b - a), (c - a)));
					normal += t;

					a = vertices[x * mSize + z - 1].position;
					b = vertices[(x + 1) * mSize + z].position;
					c = vertices[(x + 1) * mSize + z - 1].position;
					t = glm::normalize(glm::cross((b - a), (c - a)));
					normal += t;

					count += 2;
				}

				if (count != 0)
				{
					normal = glm::normalize(normal);
				}
				else
				{
					normal = { 0.0f, 1.0f, 0.0f };
				}

				vertices[x * mSize + z].normal = normal;
			}
		}
	}

	void Terrain::createPatches()
	{
		mNumPatches = (mSize - 1) / mCalcPatchSize;
		mPatches.resize(mNumPatches * mNumPatches);
	}

	void Terrain::calculatePatchData(std::vector<Vertex>& vertices)
	{
		mBoundingBox.min = glm::vec3{ std::numeric_limits<float>::max() };
		mBoundingBox.max = glm::vec3{ std::numeric_limits<float>::lowest() };

		for (uint32_t x = 0; x < mNumPatches; x++)
		{
			for (uint32_t z = 0; z < mNumPatches; z++)
			{
				uint32_t index = x * mNumPatches + z;

				auto& patch = mPatches[index];
				patch.currentLOD = 0;

				auto xStart = x * mCalcPatchSize;
				auto xEnd = xStart + mCalcPatchSize;
				auto zStart = z * mCalcPatchSize;
				auto zEnd = zStart + mCalcPatchSize;

				patch.boundingBox.min = glm::vec3{ std::numeric_limits<float>::max() };
				patch.boundingBox.max = glm::vec3{ std::numeric_limits<float>::lowest() };

				for (uint32_t xx = xStart; xx <= xEnd; xx++)
				{
					for (uint32_t zz = zStart; zz <= zEnd; zz++)
					{
						patch.boundingBox.combinePoint(vertices[xx * mSize + zz].position);
					}
				}

				patch.center = mBoundingBox.getCenter();
				mBoundingBox.combineBox(patch.boundingBox);

				if (x > 0)
				{
					patch.top = &mPatches[(x - 1) * mNumPatches + z];
				}
				else
				{
					patch.top = nullptr;
				}

				if (x < mNumPatches - 1)
				{
					patch.bottom = &mPatches[(x + 1) * mNumPatches + z];
				}
				else
				{
					patch.bottom = nullptr;
				}

				if (z > 0)
				{
					patch.left = &mPatches[x * mNumPatches + (z - 1)];
				}
				else
				{
					patch.left = nullptr;
				}

				if (z < mNumPatches - 1)
				{
					patch.right = &mPatches[x * mNumPatches + (z + 1)];
				}
				else
				{
					patch.right = nullptr;
				}
			}
		}

		mCenter = mBoundingBox.getCenter();
	}

	void Terrain::calculateDistanceThresholds()
	{
		double size = mPatchSize * mPatchSize * mCellSpacing * mCellSpacing;
		mDistanceThreshold.resize(mMaxLOD);

		for (uint32_t idx = 0; idx < mMaxLOD; idx++)
		{
			mDistanceThreshold[idx] = size * ((idx + 1 + idx / 2) * ((idx + 1 + idx / 2)));
		}
	}

	void Terrain::setCurrentLODOfPatches(int32_t lod)
	{
		uint32_t count = mNumPatches * mNumPatches;
		for (uint32_t idx = 0; idx < count; idx++)
		{
			mPatches[idx].currentLOD = lod;
		}
	}

	void Terrain::setCurrentLODOfPatches(const std::vector<int32_t>& lods)
	{
		uint32_t count = mNumPatches * mNumPatches;
		for (uint32_t idx = 0; idx < count; idx++)
		{
			mPatches[idx].currentLOD = lods[idx];
		}
	}

	bool Terrain::read(FileReader& reader, ResourceCache& cache)
	{
		if (!Resource::read(reader, cache))
		{
			return false;
		}

		reader.read(&mPatchSize);
		reader.read(&mMaxLOD);
		reader.read(&mCellSpacing);

		auto& fileSystem = FileSystem::get();

		auto heightMapFileName = Resource::getReadPath(reader.getPath(), reader.readString());
		if (!cache.isLoaded<HeightMap>(heightMapFileName))
		{
			auto heightMap = std::make_unique<HeightMap>();
			if (!heightMap->create(heightMapFileName, cache))
			{
				LogError("HeightMap::create() failed for '%s'", heightMapFileName.c_str());
				return false;
			}

			cache.addResource(std::move(heightMap));
		}

		auto materialFileName = Resource::getReadPath(reader.getPath(), reader.readString());
		if (!cache.isLoaded<Material>(materialFileName))
		{
			auto material = std::make_unique<TerrainMaterial>();
			if (!material->create(materialFileName, cache))
			{
				LogError("TerrainMaterial::create() failed for '%s'", materialFileName.c_str());
				return false;
			}

			cache.addResource(std::move(material));
		}

		auto* heightMap = cache.getResource<HeightMap>(heightMapFileName);
		auto* material = cache.getResource<Material>(materialFileName);

		if (!load(cache, *heightMap, *material, mMaxLOD, mPatchSize, mCellSpacing))
		{
			LogError("Terrain::load() failed.");
			return false;
		}

		return true;
	}

	bool Terrain::write(FileWriter& writer)
	{
		if (!Resource::write(writer))
		{
			return false;
		}

		writer.write(&mPatchSize);
		writer.write(&mMaxLOD);
		writer.write(&mCellSpacing);

		auto& fileSystem = FileSystem::get();

		if (mHeightMap != nullptr)
		{
			auto fileName = Resource::getWritePath(writer.getPath(), mHeightMap->getFileName());
			writer.writeString(fileName);
		}

		if (mMaterial != nullptr)
		{
			auto fileName = Resource::getWritePath(writer.getPath(), mMaterial->getFileName());
			writer.writeString(fileName);
		}

		return true;
	}
}