#include "Graphics/Texture.h"
#include "VFS/FileReader.h"
#include "VFS/FileWriter.h"
#include "Core/ResourceCache.h"

namespace Trinity
{
	bool Texture::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		return Resource::create(fileName, cache, loadContent);
	}

	void Texture::destroy()
	{
		Resource::destroy();
	}

	bool Texture::write()
	{
		return Resource::write();
	}

    std::type_index Texture::getType() const
    {
        return typeid(Texture);
    }

	bool Texture::read(FileReader& reader, ResourceCache& cache)
	{
        reader.read((uint32_t*)&mTextureType);
        reader.read((uint32_t*)&mFormat);

        return true;
	}

	bool Texture::write(FileWriter& writer)
	{
        writer.write((uint32_t*)&mTextureType);
        writer.write((uint32_t*)&mFormat);

        return true;
	}
}