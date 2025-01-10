#pragma once
#include <chrono>
#include <cstdarg>
#include <format>
#include <iostream>

namespace lune
{
	namespace logging
	{
		enum class LogLevel : uint8_t
		{
			NoLogs,
			Verbose,
			Info,
			Warning,
			Error,
			Fatal
		};

		static LogLevel gLogLevel = LogLevel::Info;

		static std::string lex_to_string(const LogLevel level)
		{
			switch (level)
			{
			case LogLevel::Verbose:
				return "Verbose";
			case LogLevel::Info:
				return "Info";
			case LogLevel::Warning:
				return "Warning";
			case LogLevel::Error:
				return "Error";
			case LogLevel::Fatal:
				return "Fatal";
			default:
				return "Unknown";
			}
		}

		template <typename... Args>
		void log(const LogLevel level, const std::string_view category, const std::string_view format, Args... args)
		{
			if (level <= LogLevel::NoLogs || level < gLogLevel)
				return;

			std::ostream& ostream = level == LogLevel::Fatal || level == LogLevel::Error || level == LogLevel::Warning
										? std::cerr
										: std::cout;

			const auto now = std::chrono::utc_clock::now();
			const auto log_level_str = lex_to_string(level);

			ostream << std::vformat("[{0:%F}T{0:%T}] {1}: {2}: ", std::make_format_args(now, category, log_level_str)) << std::vformat(format, std::make_format_args(args...)) << std::endl;

			if (level == LogLevel::Fatal)
			{
				std::cout.flush();
				std::cerr.flush();
				std::abort();
			}
		}
	} // namespace logging
} // namespace lune

#define LUNE_LOG(level, category, format, ...) lune::logging::log(lune::logging::LogLevel::level, #category, format, ##__VA_ARGS__);
#define LN_LOG(level, category, format, ...) LUNE_LOG(level, category, format, ##__VA_ARGS__)