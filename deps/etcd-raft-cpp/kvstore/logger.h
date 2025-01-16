#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ranges.h>
#include <spdlog/fmt/bin_to_hex.h>

#define LOG_TRACE(fmt, ...) \
    if (spdlog::should_log(spdlog::level::level_enum::trace)) \
    spdlog::log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::level_enum::trace, fmt, ##__VA_ARGS__)

#define LOG_DEBUG(fmt, ...) \
    if (spdlog::should_log(spdlog::level::level_enum::debug)) \
    spdlog::log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::level_enum::debug, fmt, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...) \
    if (spdlog::should_log(spdlog::level::level_enum::info)) \
    spdlog::log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::level_enum::info, fmt, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) \
    if (spdlog::should_log(spdlog::level::level_enum::warn)) \
    spdlog::log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::level_enum::warn, fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
    if (spdlog::should_log(spdlog::level::level_enum::err)) \
    spdlog::log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::level_enum::err, fmt, ##__VA_ARGS__)

#define LOG_FATAL(fmt, ...) \
    { \
        spdlog::log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::level_enum::critical, fmt, ##__VA_ARGS__); \
        spdlog::default_logger()->flush(); \
        std::abort(); \
    }

bool SetupLogger();