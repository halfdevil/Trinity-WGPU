#include "Core/Debugger.h"
#include "Core/Logger.h"
#include <stdarg.h>
#include <stdio.h>

namespace Trinity
{
	void Debugger::assertFunc(bool condition, const char* fileName, uint32_t lineNo, const char* format, ...)
	{
		constexpr uint32_t kMaxBufferLine = 4096;

		if (!condition)
		{
			char buffer[kMaxBufferLine];

			va_list args;

			va_start(args, format);
			vsnprintf(buffer, kMaxBufferLine, format, args);
			va_end(args);

			LogError("!!ASSERT!! at (%s, %d): %s", fileName, lineNo, buffer);
			abort();
		}
	}

	void Debugger::fatalFunc(bool condition, const char* fileName, uint32_t lineNo, const char* format, ...)
	{
		constexpr uint32_t kMaxBufferLine = 4096;

		if (!condition)
		{
			char buffer[kMaxBufferLine];

			va_list args;

			va_start(args, format);
			vsnprintf(buffer, kMaxBufferLine, format, args);
			va_end(args);

			LogFatal("(%s, %d): %s", fileName, lineNo, buffer);
			exit(EXIT_FAILURE);
		}
	}
}