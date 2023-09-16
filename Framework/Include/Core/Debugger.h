#pragma once

#include "Core/Singleton.h"
#include <cstdint>

#if DEBUG_BUILD
	#define Assert(condition, format, ...) Trinity::Debugger::get().assertFunc((condition), \
		__FILE__, __LINE__, format, ##__VA_ARGS__)
#else
	#define Assert(condition, format, ...) ((void)(condition));
#endif

#define Fatal(condition, format, ...) Trinity::Debugger::get().fatalFunc((condition), \
	__FILE__, __LINE__, format, ##__VA_ARGS__)

#define Exit(format, ...) Trinity::Debugger::get().fatalFunc(false, \
	__FILE__, __LINE__, format, ##__VA_ARGS__)

namespace Trinity
{
	class Debugger : public Singleton<Debugger>
	{
	public:

		void assertFunc(bool condition, const char* fileName, uint32_t lineNo, const char* format, ...);
		void fatalFunc(bool condition, const char* fileName, uint32_t lineNo, const char* format, ...);
	};
}