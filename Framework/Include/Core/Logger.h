#pragma once

#include "Core/Singleton.h"
#include <cstdint>
#include <memory>

#define LogInfo(format, ...)		Trinity::Logger::get().info(__FILE__, __LINE__, #format, ##__VA_ARGS__)
#define LogDebug(format, ...)		Trinity::Logger::get().debug(__FILE__, __LINE__, #format, ##__VA_ARGS__)
#define LogWarning(format, ...)		Trinity::Logger::get().warning(__FILE__, __LINE__, #format, ##__VA_ARGS__)
#define LogError(format, ...)		Trinity::Logger::get().error(__FILE__, __LINE__, #format, ##__VA_ARGS__)
#define LogFatal(format, ...)		Trinity::Logger::get().fatal(__FILE__, __LINE__, #format, ##__VA_ARGS__)

namespace Trinity
{
	enum class LogLevel
	{
		Info,
		Debug,
		Warning,
		Error,
		Fatal
	};

	class Logger : public Singleton<Logger>
	{
	public:

		Logger() = default;
		virtual ~Logger() = default;

		LogLevel getMaxLogLevel() const
		{
			return mMaxLogLevel;
		}

		template <typename ... Args>
		void info(const char* format, Args&&... args)
		{
			if (canLog(LogLevel::Info))
			{
				write(LogLevel::Info, format, std::forward<Args>(args)...);
			}
		}

		template <typename ... Args>
		void debug(const char* format, Args&&... args)
		{
			if (canLog(LogLevel::Debug))
			{
				write(LogLevel::Debug, format, std::forward<Args>(args)...);
			}
		}

		template <typename ... Args>
		void warning(const char* format, Args&&... args)
		{
			if (canLog(LogLevel::Warning))
			{
				write(LogLevel::Warning, format, std::forward<Args>(args)...);
			}
		}

		template <typename ... Args>
		void error(const char* format, Args&&... args)
		{
			if (canLog(LogLevel::Error))
			{
				write(LogLevel::Error, format, std::forward<Args>(args)...);
			}
		}

		template <typename ... Args>
		void fatal(const char* format, Args&&... args)
		{
			if (canLog(LogLevel::Fatal))
			{
				write(LogLevel::Fatal, format, std::forward<Args>(args)...);
			}
		}

		virtual bool canLog(LogLevel logLevel) const;
		virtual void setMaxLogLevel(LogLevel logLevel);

	protected:

		virtual void write(LogLevel logLevel, const char* file, uint32_t lineNo, const char* format, ...);

	protected:

		LogLevel mMaxLogLevel{ LogLevel::Error };
	};
}