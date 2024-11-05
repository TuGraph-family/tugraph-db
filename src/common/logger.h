/**
* Copyright 2024 AntGroup CO., Ltd.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*/

/*
 * written by botu.wzy
 */

#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ranges.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <gflags/gflags.h>
DECLARE_bool(enable_query_log);

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

#define QUERY_LOG(fmt, ...) \
    if (FLAGS_enable_query_log) \
    spdlog::get("query_logger")->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::level_enum::info, fmt, ##__VA_ARGS__)

void SetupQueryLogger();
bool SetupLogger();