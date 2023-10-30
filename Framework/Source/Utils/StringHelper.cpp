#include "Utils/StringHelper.h"
#include <algorithm> 
#include <cctype>
#include <locale>
#include <sstream>

namespace Trinity
{
	void StringHelper::ltrim(std::string& s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
			return !std::isspace(ch);
		}));
	}

	void StringHelper::rtrim(std::string& s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
		}).base(), s.end());
	}

	void StringHelper::trim(std::string& s)
	{
		rtrim(s);
		ltrim(s);
	}

	std::string StringHelper::toSnakeCase(const std::string& s)
	{
		std::stringstream result;

		for (const auto ch : s)
		{
			if (std::isalpha(ch))
			{
				if (std::isspace(ch))
				{
					result << "_";
				}
				else
				{
					if (std::isupper(ch))
					{
						result << "_";
					}

					result << static_cast<char>(std::tolower(ch));
				}
			}
			else
			{
				result << ch;
			}
		}

		return result.str();
	}
}