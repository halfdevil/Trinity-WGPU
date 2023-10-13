#pragma once

#include "Core/ConsoleApplication.h"
#include <string>
#include <vector>

namespace Trinity
{
	class WgslValidator : public ConsoleApplication
	{
	public:

		WgslValidator() = default;
		~WgslValidator();

		WgslValidator(const WgslValidator&) = delete;
		WgslValidator& operator = (const WgslValidator&) = delete;

		WgslValidator(WgslValidator&&) noexcept = default;
		WgslValidator& operator = (WgslValidator&&) noexcept = default;

		void setFileName(const std::string& fileName);
		void setDefines(std::vector<std::string>&& defines);

	protected:

		virtual void execute() override;

	private:

		std::string mFileName;
		std::vector<std::string> mDefines;
	};
}