//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

#ifdef __linux__
#include <execinfo.h>
#include <signal.h>
#endif

#include "fma-common/env.h"

namespace fma_common {
enum LogLevel {
    LL_FATAL = 1,
    LL_ERROR = 2,
    LL_WARNING = 3,
    LL_INFO = 4,
    LL_DEBUG = 5,
    _LL_LAST_INVALID = 6
};

/** A log device writes a log line each time. The line is guaranteed to be written atomically. */
class LogDevice {
 public:
    virtual ~LogDevice() = default;
    virtual void WriteLine(const char* p, size_t s, LogLevel level) = 0;
    virtual void Flush() {}
};

class ConsoleLogDevice : public LogDevice {
    DISABLE_COPY(ConsoleLogDevice);
    DISABLE_MOVE(ConsoleLogDevice);
    ConsoleLogDevice() = default;
    std::mutex mutex_;

 public:
    ~ConsoleLogDevice() override { std::cerr.flush(); }

    void WriteLine(const char* p, size_t s, LogLevel level) override {
        std::lock_guard<std::mutex> l(mutex_);
        std::cerr.write(p, s);
    }

    void Flush() override { std::cerr.flush(); }

    static std::shared_ptr<ConsoleLogDevice> GetInstance() {
        static std::shared_ptr<ConsoleLogDevice> instance(new ConsoleLogDevice());
        return instance;
    }
};

class FileLogDevice : public LogDevice {
    std::ofstream file_;
    std::mutex mutex_;

 public:
    explicit FileLogDevice(const std::string& path,
                           std::ofstream::openmode mode = std::ofstream::app)
        : file_(path, mode) {
        if (!file_.good()) throw std::runtime_error("Failed to open log file for writing.");
    }

    ~FileLogDevice() {
        std::lock_guard<std::mutex> l(mutex_);
        file_.flush();
    }

    void WriteLine(const char* p, size_t s, LogLevel level) override {
        std::lock_guard<std::mutex> l(mutex_);
        file_.write(p, s);
    }

    void Flush() override {
        std::lock_guard<std::mutex> l(mutex_);
        file_.flush();
    }
};

class ConsoleAndFileLogDevice : public LogDevice {
    FileLogDevice& fld;
    std::shared_ptr<ConsoleLogDevice> cld;

 public:
    ~ConsoleAndFileLogDevice() {}

    explicit ConsoleAndFileLogDevice(FileLogDevice& dev) : fld(dev) {
        cld = ConsoleLogDevice::GetInstance();
    }

    void WriteLine(const char* p, size_t s, LogLevel level) override {
        fld.WriteLine(p, s, level);
        cld->WriteLine(p, s, level);
    }

    void Flush() override {
        fld.Flush();
        cld->Flush();
    }
};

/** A log formatter takes a long buffer and writes it to the log device line by line. */
class LogFormatter {
 protected:
    /**
     * Writes one line to the device. Overridden by different formatters to format each line. By
     * default, it does not decorate the output, but simply write the line as is.
     *
     * \param          p        Pointer to the line begin.
     * \param          s        Size of the line.
     * \param [in,out] device   The device.
     * \param          module   The module name.
     * \param          level    The log level.
     */
    virtual void WriteLine(const char* p, size_t s, LogDevice& device, const std::string& module,
                           LogLevel level) {
        device.WriteLine(p, s, level);
    }

 public:
    virtual ~LogFormatter() {}

    virtual void Write(const char* p, size_t s, LogDevice& device, const std::string& module,
                       LogLevel level) {
        const char* e = p + s;
        const char* le = p;
        while (true) {
            while (le < e && *le != '\r' && *le != '\n') le++;
            while (le < e && (*le == '\r' || *le == '\n')) le++;
            if (le != p) WriteLine(p, le - p, device, module, level);
            p = le;
            if (le == e) break;
        }
    }
};

/** A log formatter that prints the time at the beginning of each log line. */
class TimedLogFormatter : public LogFormatter {
 protected:
    inline size_t PrintTimeHead(char* buf, size_t buf_size) {
        time_t rawtime;
        time(&rawtime);
        tm* timeinfo = std::localtime(&rawtime);
        size_t s = std::strftime(buf, buf_size, "%Y%m%d%H%M%S", timeinfo);
        assert(s < buf_size);
        auto t = std::chrono::system_clock::now();
        time_t tnow = std::chrono::system_clock::to_time_t(t);
        tm* date = std::localtime(&tnow);
        date->tm_hour = 0;
        date->tm_min = 0;
        date->tm_sec = 0;
        auto midnight = std::chrono::system_clock::from_time_t(std::mktime(date));
        size_t elapsed_milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(t - midnight).count();
        return s + snprintf(buf + s, buf_size - s, ".%03d: ", (int)(elapsed_milliseconds % 1000));
    }

    void WriteLine(const char* p, size_t s, LogDevice& device, const std::string& module,
                           LogLevel level) override {
        if (s < 1000) {
            char buf[1024];
            size_t hs = PrintTimeHead(buf, 1024);
            memcpy(buf + hs, p, s);
            device.WriteLine(buf, s + hs, level);
        } else {
            size_t ts = s + 24;
            char* buf = new char[ts];
            size_t hs = PrintTimeHead(buf, 24);
            memcpy(buf + hs, p, s);
            device.WriteLine(buf, s + hs, level);
            delete[] buf;
        }
    }
};

class TimedModuleLogFormatter : public TimedLogFormatter {
 protected:
    inline size_t PrintModuleName(char* buf, size_t off, const std::string& module) {
        if (module.empty()) return 0;
        buf[off++] = '[';
        memcpy(buf + off, module.data(), module.size());
        off += module.size();
        buf[off++] = ']';
        buf[off++] = ' ';
        return module.size() + 3;
    }

    void WriteLine(const char* p, size_t s, LogDevice& device, const std::string& module,
                           LogLevel level) override {
        if (s + module.size() < 1000) {
            char buf[1024];
            size_t hs = PrintTimeHead(buf, 1024);
            hs += PrintModuleName(buf, hs, module);
            memcpy(buf + hs, p, s);
            device.WriteLine(buf, s + hs, level);
        } else {
            size_t ts = s + module.size() + 24;
            char* buf = new char[ts];
            size_t hs = PrintTimeHead(buf, 24);
            hs += PrintModuleName(buf, hs, module);
            memcpy(buf + hs, p, s);
            device.WriteLine(buf, s + hs, level);
            delete[] buf;
        }
    }
};

class LoggerManager;
class Logger {
 private:
    friend class LoggerManager;
    std::shared_ptr<LogDevice> device_;
    std::shared_ptr<LogFormatter> formatter_;
    LogLevel level_;
    std::string name_;

    Logger(const std::string& name = "",
           const std::shared_ptr<LogDevice>& dev = ConsoleLogDevice::GetInstance())
        : device_(dev), level_(LL_INFO), name_(name) {
        formatter_ = std::make_shared<LogFormatter>();
    }

    Logger(const std::string& name, const Logger& rhs)
        : device_(rhs.device_), formatter_(rhs.formatter_), level_(rhs.level_), name_(name) {}

 public:
    inline void SetDevice(const std::shared_ptr<LogDevice>& device) { device_ = device; }

    inline const std::shared_ptr<LogDevice>& GetDevice() const { return device_; }

    inline void SetFormatter(const std::shared_ptr<LogFormatter>& formatter) {
        formatter_ = formatter;
    }

    inline const std::shared_ptr<LogFormatter>& GetFormatter() const { return formatter_; }

    inline void SetLevel(LogLevel level) { level_ = level; }

    inline LogLevel GetLevel() const { return level_; }

    inline const std::string& GetName() const { return name_; }

    static inline Logger& Get(const std::string& name = std::string());

    inline void Write(const std::string& d, LogLevel level);
};

class LoggerStream {
    Logger& logger_;
    std::ostringstream oss_;
    LogLevel level_;
    std::string file_;
    std::string func_;
    int line_;
    bool append_newline_;
    bool has_debug_info_;

 public:
    explicit LoggerStream(Logger& logger, LogLevel level = LL_INFO, bool append_newline = true)
        : logger_(logger), level_(level), append_newline_(append_newline), has_debug_info_(false) {}
    LoggerStream(Logger& logger, std::string file, std::string func, int line,
    LogLevel level = LL_INFO, bool append_newline = true)
        : logger_(logger), level_(level), file_(file), func_(func), line_(line),
        append_newline_(append_newline), has_debug_info_(true) {}

    ~LoggerStream() {
        if (append_newline_) oss_ << "\n";
        if (logger_.GetLevel() == LL_DEBUG && has_debug_info_) {
            std::ostringstream debug_info_;
            debug_info_ << "[" << file_ << ":" << func_ << ":" << line_ << "] ";
            logger_.Write(debug_info_.str() + oss_.str(), level_);
        } else {
            logger_.Write(oss_.str(), level_);
        }
    }

    template <typename T>
    LoggerStream& operator<<(const T& d) {
        oss_ << d;
        return *this;
    }

    void Flush() {
        logger_.Write(oss_.str(), level_);
        oss_.clear();
        oss_.str("");
    }

    operator bool() const { return oss_.good(); }
};

class NullStream {
 public:
    template <typename T>
    NullStream& operator<<(const T& d) {
        return *this;
    }

    operator bool() const { return true; }
};

class LoggerManager {
    std::map<std::string, Logger*> loggers_;
    std::mutex mutex_;

    Logger& GetNoLock(const std::string& name) {
        if (name.empty()) return *loggers_.begin()->second;
        auto it = loggers_.find(name);
        if (it == loggers_.end()) {
            size_t pos = name.rfind(".");
            std::string parent_name;
            if (pos != name.npos) {
                parent_name = name.substr(0, pos);
            }
            auto& parent = GetNoLock(parent_name);
            it = loggers_.emplace_hint(it, name, new Logger(name, parent));
        }
        return *it->second;
    }

 public:
    LoggerManager() { loggers_.emplace("", new Logger()); }

    ~LoggerManager() {
        for (auto& kv : loggers_) delete kv.second;
    }

    Logger& Get(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        return GetNoLock(name);
    }
};

namespace _detail {
inline static void PrintBacktraceAndExit(int sig);
}

Logger& Logger::Get(const std::string& name) {
    static LoggerManager manager;
    return manager.Get(name);
}

void Logger::Write(const std::string& d, LogLevel level) {
    if (level > level_) return;
    formatter_->Write(d.data(), d.size(), *device_, name_, level);
    device_->Flush();
    if (level <= LL_FATAL) _detail::PrintBacktraceAndExit(0);
}

namespace _detail {
#ifdef __linux__
inline static void PrintBacktraceAndExit(int sig) {
#ifndef NO_STACKTRACE
    void* array[40];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 40);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    // trigger core dump
    abort();
#else
    exit(1);
#endif
}
#else
inline static void PrintBacktraceAndExit(int sig) { abort(); }
#endif

#ifdef __linux__
inline static void SetSignalHandler(void) __attribute__((constructor));
inline static void SetSignalHandler(void) {
    struct sigaction act;
    act.sa_handler = PrintBacktraceAndExit;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGSEGV, &act, nullptr);
}
#endif

template <typename T1, typename T2, typename Comp>
inline bool _CheckTwo_(const T1& a, const T2& b, const Comp& compare, const char* check,
                       const char* astr, const char* bstr, const char* file, int line) {
    if (!compare(a, b)) {
        LoggerStream(Logger::Get(), LL_ERROR)
            << file << ":" << line << "\n\t" << check << "(" << astr << "," << bstr
            << ") failed: " << astr << "=" << a << ", " << bstr << "=" << b;
        return false;
    }
    return true;
}

#define _FMA_DEF_CHECK_FUNC__(FuncName, func)                                           \
    template <typename T1, typename T2>                                                 \
    inline bool FuncName(const T1& a, const T2& b, const char* check, const char* astr, \
                         const char* bstr, const char* file, int line) {                \
        if (!(func(a, (T1)b))) {                                                            \
            LoggerStream(Logger::Get(), LL_ERROR)                                       \
                << file << ":" << line << "\n\t" << check << "(" << astr << "," << bstr \
                << ") failed: " << astr << "=" << a << ", " << bstr << "=" << b;        \
            return false;                                                               \
        }                                                                               \
        return true;                                                                    \
    }

#define _FMA_CHECK_EQ_FUNC(a, b) a == b
#define _FMA_CHECK_NEQ_FUNC(a, b) a != b
#define _FMA_CHECK_LT_FUNC(a, b) a < b
#define _FMA_CHECK_LE_FUNC(a, b) a <= b
#define _FMA_CHECK_GT_FUNC(a, b) a > b
#define _FMA_CHECK_GE_FUNC(a, b) a >= b
_FMA_DEF_CHECK_FUNC__(CheckEq, _FMA_CHECK_EQ_FUNC);
_FMA_DEF_CHECK_FUNC__(CheckNeq, _FMA_CHECK_NEQ_FUNC);
_FMA_DEF_CHECK_FUNC__(CheckLt, _FMA_CHECK_LT_FUNC);
_FMA_DEF_CHECK_FUNC__(CheckLe, _FMA_CHECK_LE_FUNC);
_FMA_DEF_CHECK_FUNC__(CheckGt, _FMA_CHECK_GT_FUNC);
_FMA_DEF_CHECK_FUNC__(CheckGe, _FMA_CHECK_GE_FUNC);
}  // namespace _detail

#define FMA_GET_LOG_STREAM(LEVEL) \
    ::fma_common::LoggerStream(::fma_common::Logger::Get(), __FILE__, __FUNCTION__, __LINE__, \
    ::fma_common::LEVEL)

#define FMA_LOG_STREAM_WITH_LEVEL(logger, LEVEL) \
    logger.GetLevel() < LEVEL ? true : ::fma_common::LoggerStream(logger, LEVEL)

#define FMA_DBG_STREAM(logger) FMA_LOG_STREAM_WITH_LEVEL(logger, ::fma_common::LL_DEBUG)
#define FMA_INFO_STREAM(logger) FMA_LOG_STREAM_WITH_LEVEL(logger, ::fma_common::LL_INFO)
#define FMA_WARN_STREAM(logger) FMA_LOG_STREAM_WITH_LEVEL(logger, ::fma_common::LL_WARNING)
#define FMA_ERR_STREAM(logger) FMA_LOG_STREAM_WITH_LEVEL(logger, ::fma_common::LL_ERROR)
#define FMA_FATAL_STREAM(logger) FMA_LOG_STREAM_WITH_LEVEL(logger, ::fma_common::LL_FATAL)

#ifndef DISABLE_DBG_LOG
#define FMA_DBG() FMA_GET_LOG_STREAM(LL_DEBUG)
#else
#define FMA_DBG() true || ::fma_common::NullStream()
#endif

#ifndef DISABLE_INFO_LOG
#define FMA_LOG() FMA_GET_LOG_STREAM(LL_INFO)
#else
#define FMA_LOG() true || ::fma_common::NullStream()
#endif

#ifndef DISABLE_WARN_LOG
#define FMA_WARN() FMA_GET_LOG_STREAM(LL_WARNING)
#else
#define FMA_WARN() true || ::fma_common::NullStream()
#endif

#ifndef DISABLE_ERR_LOG
#define FMA_ERR() FMA_GET_LOG_STREAM(LL_FATAL)
#else
#define FMA_ERR() true || ::fma_common::NullStream()
#endif

#define FMA_FATAL() FMA_GET_LOG_STREAM(LL_FATAL)

#define FMA_CHECK(pred) \
    if (!(pred)) FMA_ERR() << __FILE__ << ":" << __LINE__ << "\n\tCHECK(" #pred ") failed\n"

#define FMA_CHECK_EQ(a, b) \
    if (!::fma_common::_detail::CheckEq((a), (b), "CHECK_EQ", #a, #b, __FILE__, __LINE__)) FMA_ERR()
#define FMA_CHECK_NEQ(a, b)                                                                  \
    if (!::fma_common::_detail::CheckNeq((a), (b), "CHECK_NEQ", #a, #b, __FILE__, __LINE__)) \
    FMA_ERR()
#define FMA_CHECK_LT(a, b) \
    if (!::fma_common::_detail::CheckLt((a), (b), "CHECK_LT", #a, #b, __FILE__, __LINE__)) FMA_ERR()
#define FMA_CHECK_LE(a, b) \
    if (!::fma_common::_detail::CheckLe((a), (b), "CHECK_LE", #a, #b, __FILE__, __LINE__)) FMA_ERR()
#define FMA_CHECK_GT(a, b) \
    if (!::fma_common::_detail::CheckGt((a), (b), "CHECK_GT", #a, #b, __FILE__, __LINE__)) FMA_ERR()
#define FMA_CHECK_GE(a, b) \
    if (!::fma_common::_detail::CheckGe((a), (b), "CHECK_GE", #a, #b, __FILE__, __LINE__)) FMA_ERR()
#define FMA_ASSERT(pred) \
    if (!(pred)) FMA_ERR() << __FILE__ << ":" << __LINE__ << "\n\tASSERT(" #pred ") failed\n"

#ifndef NDEBUG
#define FMA_DBG_ASSERT(pred) FMA_ASSERT(pred)
#define FMA_DBG_CHECK(pred) FMA_CHECK(pred)
#define FMA_DBG_CHECK_EQ(a, b) FMA_CHECK_EQ(a, b)
#define FMA_DBG_CHECK_NEQ(a, b) FMA_CHECK_NEQ(a, b)
#define FMA_DBG_CHECK_LT(a, b) FMA_CHECK_LT(a, b)
#define FMA_DBG_CHECK_LE(a, b) FMA_CHECK_LE(a, b)
#define FMA_DBG_CHECK_GT(a, b) FMA_CHECK_GT(a, b)
#define FMA_DBG_CHECK_GE(a, b) FMA_CHECK_GE(a, b)
#else
#define FMA_DBG_ASSERT(pred) true || ::fma_common::NullStream()
#define FMA_DBG_CHECK(pred) true || ::fma_common::NullStream()
#define FMA_DBG_CHECK_EQ(a, b) true || ::fma_common::NullStream()
#define FMA_DBG_CHECK_NEQ(a, b) true || ::fma_common::NullStream()
#define FMA_DBG_CHECK_LT(a, b) true || ::fma_common::NullStream()
#define FMA_DBG_CHECK_LE(a, b) true || ::fma_common::NullStream()
#define FMA_DBG_CHECK_GT(a, b) true || ::fma_common::NullStream()
#define FMA_DBG_CHECK_GE(a, b) true || ::fma_common::NullStream()
#endif

#define FMA_EXIT() exit(0)
}  // namespace fma_common
