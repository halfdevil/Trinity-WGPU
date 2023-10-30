#pragma once

#include <string>

namespace Trinity
{
	class StringHelper
	{
	public:

		static void ltrim(std::string& s);
		static void rtrim(std::string& s);
		static void trim(std::string& s);
		static std::string toSnakeCase(const std::string& s);
	};
}