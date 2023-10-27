#pragma once

#include "Core/ConsoleApplication.h"

namespace Trinity
{
	class SceneConverter : public ConsoleApplication
	{
	public:

		SceneConverter() = default;
		~SceneConverter() = default;

		SceneConverter(const SceneConverter&) = delete;
		SceneConverter& operator = (const SceneConverter&) = delete;

		SceneConverter(SceneConverter&&) noexcept = default;
		SceneConverter& operator = (SceneConverter&&) noexcept = default;

		virtual void setFileName(const std::string& fileName);
		virtual void setOutputFileName(const std::string& fileName);

	protected:

		virtual void execute() override;

	private:

		std::string mFileName;
		std::string mOutputFileName;
	};
}