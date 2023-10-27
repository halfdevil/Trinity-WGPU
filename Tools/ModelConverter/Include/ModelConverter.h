#pragma once

#include "Core/ConsoleApplication.h"

namespace Trinity
{
	class ModelConverter : public ConsoleApplication
	{
	public:

		ModelConverter() = default;
		~ModelConverter() = default;

		ModelConverter(const ModelConverter&) = delete;
		ModelConverter& operator = (const ModelConverter&) = delete;

		ModelConverter(ModelConverter&&) noexcept = default;
		ModelConverter& operator = (ModelConverter&&) noexcept = default;

		void setFileName(const std::string& fileName);
		void setOutputFileName(const std::string& fileName);
		void setAnimated(bool animated);

	protected:

		virtual void execute() override;

	private:

		std::string mFileName;
		std::string mOutputFileName;
		bool mAnimated{ false };
	};
}