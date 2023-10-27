#include "Scene/Component.h"
#include "Scene/Node.h"
#include "VFS/FileReader.h"
#include "VFS/FileWriter.h"

namespace Trinity
{
	void Component::setName(const std::string& name)
    {
        mName = name;
    }

	bool Component::read(FileReader& reader, ResourceCache& cache, Scene& scene)
	{
		mName = reader.readString();
		return true;
	}

	bool Component::write(FileWriter& writer, Scene& scene)
	{
		writer.writeString(mName);
		return true;
	}
}