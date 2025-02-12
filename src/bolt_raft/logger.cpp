/**
 * Copyright 2022 AntGroup CO., Ltd.
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

// written by botu.wzy

#include <cstdarg>
#include <cstdio>
#include "tools/lgraph_log.h"

namespace eraft {
using namespace lgraph_log;
#define log_buffer_size 1024
char *args_format(const char *format, va_list args) {
    thread_local std::unique_ptr<char[]> buf(new char[log_buffer_size]);
    vsnprintf(buf.get(), log_buffer_size, format, args);
    return buf.get();
}
void log_debug(const char *file, int line, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    BOOST_LOG_SEV(debug_logger::get(), severity_level::DEBUG)
        << logging::add_value("File", file) << logging::add_value("Line", line)
        << args_format(msg, args);
    va_end(args);
}
void log_info(const char *file, int line, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    BOOST_LOG_SEV(debug_logger::get(), severity_level::INFO)
        << logging::add_value("File", file) << logging::add_value("Line", line)
        << args_format(msg, args);
    va_end(args);
}
void log_warn(const char *file, int line, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    BOOST_LOG_SEV(debug_logger::get(), severity_level::WARNING)
        << logging::add_value("File", file) << logging::add_value("Line", line)
        << args_format(msg, args);
    va_end(args);
}
void log_error(const char *file, int line, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    BOOST_LOG_SEV(debug_logger::get(), severity_level::ERROR)
        << logging::add_value("File", file) << logging::add_value("Line", line)
        << args_format(msg, args);
    va_end(args);
}
void log_fatal(const char *file, int line, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    FatalLogger(file, line) << args_format(msg, args);
    va_end(args);
}

}  // namespace eraft
