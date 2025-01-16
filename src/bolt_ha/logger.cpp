#include <cstdarg>
#include <cstdio>
#include "tools/lgraph_log.h"

namespace eraft {
using namespace lgraph_log;
char* args_format(const char *format, va_list args) {
    thread_local std::unique_ptr<char[]> buf(new char[1024]);
    vsnprintf(buf.get(), 1024, format, args);
    return buf.get();
}
void log_debug(const char *file, int line, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    BOOST_LOG_SEV(debug_logger::get(), severity_level::DEBUG)
        << logging::add_value("File", file)
        << logging::add_value("Line", line)
        << args_format(msg, args);
    va_end(args);
}
void log_info(const char *file, int line, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    BOOST_LOG_SEV(debug_logger::get(), severity_level::INFO)
        << logging::add_value("File", file)
        << logging::add_value("Line", line)
        << args_format(msg, args);
    va_end(args);
}
void log_warn(const char *file, int line, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    BOOST_LOG_SEV(debug_logger::get(), severity_level::WARNING)
        << logging::add_value("File", file)
        << logging::add_value("Line", line)
        << args_format(msg, args);
    va_end(args);
}
void log_error(const char *file, int line, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    BOOST_LOG_SEV(debug_logger::get(), severity_level::ERROR)
        << logging::add_value("File", file)
        << logging::add_value("Line", line)
        << args_format(msg, args);
    va_end(args);
}
void log_fatal(const char *file, int line, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    FatalLogger(file, line) << args_format(msg, args);
    va_end(args);
}

}