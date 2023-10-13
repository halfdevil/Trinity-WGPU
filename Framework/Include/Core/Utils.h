#pragma once

#include <string>

namespace Trinity
{
	void ltrim(std::string& s);
	void rtrim(std::string& s);
	void trim(std::string& s);
	std::string toSnakeCase(const std::string& s);
}