#include <iostream>
#include <cstdarg>
#include <spdlog/sinks/rotating_file_sink.h>

#include "logger.h"

bool SetupLogger() {
    try {
        auto logger = spdlog::rotating_logger_mt(
            "lgraph_logger", "log/lgraph.log",
            64*1024*1024, 10);
        logger->set_level(spdlog::level::from_str("info"));
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e %t %l %s:%#] %v");
        spdlog::set_default_logger(logger);
        spdlog::flush_every(std::chrono::seconds(1));
    } catch (const std::exception& e) {
        std::cerr << "Log init failed: " << e.what() << std::endl;
        return false;
    }
    return true;
}
using spdlog::level::level_enum;
namespace eraft {

char* args_format(const char *format, va_list args) {
    thread_local char logbuf[1024];
    vsnprintf(logbuf, 1024, format, args);
    return logbuf;
}
void log_debug(const char *file, int line, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    if (spdlog::should_log(level_enum::debug))
        spdlog::log(spdlog::source_loc{file, line, SPDLOG_FUNCTION},  level_enum::debug, args_format(msg, args));
    va_end(args);
}
void log_info(const char *file, int line, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    if (spdlog::should_log(level_enum::info))
        spdlog::log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, level_enum::info, args_format(msg, args));
    va_end(args);
}
void log_warn(const char *file, int line, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    if (spdlog::should_log(level_enum::warn))
        spdlog::log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, level_enum::warn, args_format(msg, args));
    va_end(args);
}
void log_error(const char *file, int line, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    if (spdlog::should_log(level_enum::err))
        spdlog::log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, level_enum::err, args_format(msg, args));
    va_end(args);
}
void log_fatal(const char *file, int line, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    if (spdlog::should_log(level_enum::critical))
        spdlog::log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, level_enum::critical, args_format(msg, args));
    va_end(args);
}

}