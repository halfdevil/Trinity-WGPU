#include "Core/Logger.h"
#include <stdarg.h>
#include <string>
#include <iostream>

#include <filesystem>
namespace fs = std::filesystem;

namespace Trinity
{
	bool Logger::canLog(LogLevel logLevel) const
	{
		return logLevel >= mMaxLogLevel;
	}

	void Logger::setMaxLogLevel(LogLevel logLevel)
	{
		mMaxLogLevel = logLevel;
	}

	void Logger::write(LogLevel logLevel, const char* file, uint32_t lineNo, const char* format, ...)
	{
		constexpr uint32_t MAX_BUFFER_LEN = 8192;

		char buffer[MAX_BUFFER_LEN];
		va_list args;

		va_start(args, format);
		vsnprintf(buffer, MAX_BUFFER_LEN, format, args);
		va_end(args);

		fs::path filePath(file);
		std::string fileName = filePath.filename().string();

		switch (logLevel)
		{
		case LogLevel::Info:
			std::cout << "[INFO]:" << " (" << fileName << ":" << lineNo << ") " << buffer << "\n";
			break;

		case LogLevel::Debug:
			std::cout << "[DEBUG]:" << " (" << fileName << ":" << lineNo << ") " << buffer << "\n";
			break;

		case LogLevel::Warning:
			std::cout << "[WARNING]:" << " (" << fileName << ":" << lineNo << ") " << buffer << "\n";
			break;

		case LogLevel::Error:
			std::cout << "[ERROR]:" << " (" << fileName << ":" << lineNo << ") " << buffer << "\n";
			break;

		case LogLevel::Fatal:
			std::cout << "[FATAL]:" << " (" << fileName << ":" << lineNo << ") " << buffer << "\n";
			break;

		default:
			break;
		}
	}
}