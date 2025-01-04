#pragma once
#include <chrono>
#include <cstdarg>
#include <format>
#include <iostream>

namespace lune
{
	namespace logging
	{
		enum class log_level : uint8_t
		{
			NoLogs,
			Verbose,
			Info,
			Warning,
			Error,
			Fatal
		};

		static log_level gLogLevel = log_level::Info;

		static std::string lex_to_string(const log_level level)
		{
			switch (level)
			{
			case log_level::Error:
				return "Error";
			case log_level::Warning:
				return "Warning";
			case log_level::Info:
				return "Info";
			case log_level::Verbose:
				return "Verbose";
			default:
				return "Unknown";
			}
		}

		template <typename... Args>
		void log(const log_level level, const std::string_view category, const std::string_view format, Args... args)
		{
			if (level <= log_level::NoLogs || level < gLogLevel)
				return;

			std::ostream& ostream = level == log_level::Fatal || level == log_level::Error || level == log_level::Warning
										? std::cerr
										: std::cout;

			const auto now = std::chrono::utc_clock::now();
			const auto log_level_str = lex_to_string(level);

			ostream << std::vformat("[{0:%F}T{0:%T}] {1}: {2}: ", std::make_format_args(now, category, log_level_str)) << std::vformat(format, std::make_format_args(args...)) << '\n';

			if (level == log_level::Fatal)
			{
				std::cout.flush();
				std::cerr.flush();
				std::abort();
			}
		}
	} // namespace logging
} // namespace lune

#define LUNE_LOG(level, category, format, ...) lune::logging::log(lune::logging::log_level::level, #category, format, ##__VA_ARGS__);
#define LN_LOG(level, category, format, ...) LUNE_LOG(level, category, format, ##__VA_ARGS__)