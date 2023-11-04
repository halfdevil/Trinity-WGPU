#include "Scene/Skybox/Skybox.h"
#include "Scene/Skybox/SkyboxMaterial.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/BindGroupLayout.h"
#include "Graphics/BindGroup.h"
#include "Graphics/RenderPipeline.h"
#include "VFS/FileSystem.h"
#include "Core/ResourceCache.h"
#include "Core/Logger.h"

namespace Trinity
{
	bool Skybox::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		return Resource::create(fileName, cache, loadContent);
	}

	void Skybox::destroy()
	{
		Resource::destroy();
	}

	bool Skybox::write()
	{
		return Resource::write();
	}

	void Skybox::setPosition(const glm::vec3& position)
	{
		mPosition = position;
	}

	std::type_index Skybox::getType() const
	{
		return typeid(Skybox);
	}

	bool Skybox::load(ResourceCache& cache, Material& material, float size)
	{
		setMaterial(material);
		setSize(size);

		if (!setupDeviceObjects(cache))
		{
			LogError("Skybox::setupDeviceObjects() failed");
			return false;
		}

		return true;
	}

	void Skybox::setMaterial(Material& material)
	{
		mMaterial = &material;
	}

	void Skybox::setSize(float size)
	{
		mSize = size;
	}

	bool Skybox::setupDeviceObjects(ResourceCache& cache)
	{
		const float halfSize = mSize * 0.5f;

		const std::vector<glm::vec3> vertices =
		{
			{ -halfSize, -halfSize, halfSize },
			{ halfSize, -halfSize, halfSize },
			{ halfSize, halfSize, halfSize },
			{ -halfSize, halfSize, halfSize },

			{ -halfSize, halfSize, -halfSize },
			{ halfSize, halfSize, -halfSize },
			{ halfSize, -halfSize, -halfSize },
			{ -halfSize, -halfSize, -halfSize },

			{ halfSize, -halfSize, -halfSize },
			{ halfSize, halfSize, -halfSize },
			{ halfSize, halfSize, halfSize },
			{ halfSize, -halfSize, halfSize },

			{ -halfSize, -halfSize, halfSize },
			{ -halfSize, halfSize, halfSize },
			{ -halfSize, halfSize, -halfSize },
			{ -halfSize, -halfSize, -halfSize },

			{ halfSize, halfSize, -halfSize },
			{ -halfSize, halfSize, -halfSize },
			{ -halfSize, halfSize, halfSize },
			{ halfSize, halfSize, halfSize },

			{ halfSize, -halfSize, halfSize },
			{ -halfSize, -halfSize, halfSize },
			{ -halfSize, -halfSize, -halfSize },
			{ halfSize, -halfSize, -halfSize },
		};

		const std::vector<uint16_t> indices =
		{
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4,
			8, 9, 10, 10, 11, 8,
			12, 13, 14, 14, 15, 12,
			16, 17, 18, 18, 19, 16,
			20, 21, 22, 22, 23, 20,
		};

		auto vertexLayout = std::make_unique<VertexLayout>();
		vertexLayout->setAttributes({
			{ wgpu::VertexFormat::Float32x3, 0, 0 }
		});

		auto vertexBuffer = std::make_unique<VertexBuffer>();
		if (!vertexBuffer->create(*vertexLayout, (uint32_t)vertices.size(), vertices.data()))
		{
			LogError("VertexBuffer::create() failed");
			return false;
		}

		auto indexBuffer = std::make_unique<IndexBuffer>();
		if (!indexBuffer->create(wgpu::IndexFormat::Uint16, (uint32_t)indices.size(), indices.data()))
		{
			LogError("IndexBuffer::create() failed");
			return false;
		}

		mVertexLayout = vertexLayout.get();
		mVertexBuffer = vertexBuffer.get();
		mIndexBuffer = indexBuffer.get();

		cache.addResource(std::move(vertexLayout));
		cache.addResource(std::move(vertexBuffer));
		cache.addResource(std::move(indexBuffer));

		return true;
	}

	bool Skybox::read(FileReader& reader, ResourceCache& cache)
	{
		if (!Resource::read(reader, cache))
		{
			return false;
		}

		reader.read(&mSize);

		auto& fileSystem = FileSystem::get();
		auto materialFileName = Resource::getReadPath(reader.getPath(), reader.readString());

		if (!cache.isLoaded<Material>(materialFileName))
		{
			auto material = std::make_unique<SkyboxMaterial>();
			if (!material->create(materialFileName, cache))
			{
				LogError("SkyboxMaterial::create() failed for '%s'", materialFileName.c_str());
				return false;
			}

			if (!material->compile(cache))
			{
				LogError("SkyboxMaterial::compile() failed for '%s'", materialFileName.c_str());
				return false;
			}

			cache.addResource(std::move(material));
		}

		auto* material = cache.getResource<Material>(materialFileName);
		if (!load(cache, *material, mSize))
		{
			LogError("Skybox::load() failed.");
			return false;
		}

		return true;
	}

	bool Skybox::write(FileWriter& writer)
	{
		if (!Resource::write(writer))
		{
			return false;
		}

		writer.write(&mSize);

		if (mMaterial != nullptr)
		{
			auto fileName = Resource::getWritePath(writer.getPath(), mMaterial->getFileName());
			writer.writeString(fileName);
		}

		return true;
	}
}