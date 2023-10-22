#pragma once

#include <memory>
#include <string>
#include <typeindex>
#include <vector>

namespace Trinity
{
	class Scene;
	class FileReader;
	class FileWriter;

	class Component
	{
	public:

		Component() = default;
		virtual ~Component() = default;

		Component(const Component&) = delete;
		Component& operator = (const Component&) = delete;

		Component(Component&&) = default;
		Component& operator = (Component&&) = default;

		const std::string& getName() const
		{
			return mName;
		}

		virtual std::type_index getType() const = 0;
		virtual std::string getTypeStr() const = 0;

		virtual void setName(const std::string& name);
		virtual bool read(FileReader& reader, Scene& scene);
		virtual bool write(FileWriter& writer, Scene& scene);

	protected:

		std::string mName;
	};
}