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

#pragma once

#include <stdexcept>
#include <string>
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/phoenix/bind/bind_function.hpp>
#include <boost/core/null_deleter.hpp>
#include "tools/json.hpp"

namespace lgraph_log {

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

using boost::shared_ptr;
using json = nlohmann::json;

typedef sinks::synchronous_sink< sinks::text_file_backend > file_sink;
typedef sinks::synchronous_sink< sinks::text_ostream_backend > stream_sink;
typedef sinks::synchronous_sink< sinks::text_ostream_backend > ut_sink;

// Define log macro
#define LGRAPH_LOG(LEVEL) BOOST_LOG_SEV(::lgraph_log::debug_logger::get(), \
  ::lgraph_log::severity_level::LEVEL) \
  << ::lgraph_log::logging::add_value("Line", __LINE__) \
  << ::lgraph_log::logging::add_value("File", __FILE__) \

#define LOG_DEBUG() LGRAPH_LOG(DEBUG)
#define LOG_INFO() LGRAPH_LOG(INFO)
#define LOG_WARN() LGRAPH_LOG(WARNING)
#define LOG_ERROR() LGRAPH_LOG(ERROR)

#define LOG_FATAL() lgraph_log::FatalLogger(__FILE__, __LINE__)

#define FMA_UT_LOG(LEVEL) BOOST_LOG_SEV(::lgraph_log::debug_logger::get(), \
  LEVEL) \
  << ::lgraph_log::logging::add_value("Line", __LINE__) \
  << ::lgraph_log::logging::add_value("File", __FILE__) \

#define AUDIT_LOG() BOOST_LOG(::lgraph_log::audit_logger::get())

#define AUDIT_PREFIX "audit_"

// LogType includes the following type: debug, audit
// debug type for general log
// audit type for audit log
#define DEBUG_TYPE "debug"
#define AUDIT_TYPE "audit"
BOOST_LOG_ATTRIBUTE_KEYWORD(log_type_attr, "LogType", std::string)

enum severity_level {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class LoggerManager {
 private:
    std::string log_dir_;
    severity_level level_ = severity_level::INFO;
    size_t rotation_size_ = 256*1024*1024;
    boost::shared_ptr< file_sink > file_sink_;
    boost::shared_ptr< stream_sink > stream_sink_;
    std::ostringstream log_buffer_;
    boost::shared_ptr< std::ostream > console_stream_;
    boost::shared_ptr< std::ostream > buffer_log_stream_;
    bool buffer_log_mode_ = false;
    bool global_inited_ = false;

    static std::string level_to_string(logging::value_ref< severity_level > const& level) {
        switch (level.get()) {
        case TRACE:
            return "TRACE";
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case WARNING:
            return "WARN";
        case ERROR:
            return "ERROR";
        case FATAL:
            return "FATAL";
        default:
            return "Invalid severity level";
        }
    }

    static void formatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
        auto actor = expr::stream << "[" <<
                     expr::format_date_time< boost::posix_time::ptime >(
                         "TimeStamp", "%Y%m%d %H:%M:%S.%f") << " ";
        actor(rec, strm);
        strm << logging::extract< attrs::current_thread_id::value_type >("ThreadID", rec) << " ";
        strm << std::left << std::setw(5) << std::setfill(' ')
             << level_to_string(logging::extract< severity_level >("Severity", rec)) << " ";
        strm << logging::extract<std::string>("File", rec) << ":"
             << logging::extract<int>("Line", rec) << "] ";
        strm << rec[expr::smessage];
    }

 public:
    LoggerManager() {
        console_stream_ = boost::shared_ptr< std::ostream >(&std::clog, boost::null_deleter());
        buffer_log_stream_ = boost::shared_ptr< std::ostream >(&log_buffer_, boost::null_deleter());
    }

    /**
     * @brief   Init LoggerManager. Set log directory, log filtering level.
     *
     * @param   log_dir   The log directory.
     * @param   level     The log filtering level.
     */
    void Init(std::string log_dir = "logs/", severity_level level = severity_level::INFO,
              size_t rotation_size = 256*1024*1024) {
        logging::core::get()->remove_all_sinks();
        // Set up log directory
        log_dir_ = std::move(log_dir);
        level_ = level;

        if (!log_dir_.empty()) {
            if (log_dir_.back() != '/') {
                log_dir_ += '/';
            }
            rotation_size_ = rotation_size;
            // Set up sink for debug log
            file_sink_ = boost::shared_ptr< file_sink > (new file_sink(
                keywords::file_name = log_dir_ + "lgraph_%Y%m%d_%H%M%S%f.log",
                keywords::open_mode = std::ios_base::out | std::ios_base::app,
                keywords::enable_final_rotation = false,
                keywords::auto_flush = true,
                keywords::rotation_size = rotation_size_));
            file_sink_->set_filter(expr::attr< severity_level >("Severity") >= level_ &&
                log_type_attr == DEBUG_TYPE);
            file_sink_->set_formatter(&formatter);

            logging::core::get()->add_sink(file_sink_);
        } else {
            // Set up sink for stream log
            stream_sink_ = boost::shared_ptr< stream_sink > (new stream_sink());
            if (buffer_log_mode_) {
                stream_sink_->locked_backend()->add_stream(
                    boost::shared_ptr< std::ostream >(buffer_log_stream_));
            } else {
                stream_sink_->locked_backend()->add_stream(
                    boost::shared_ptr< std::ostream >(console_stream_));
            }
            stream_sink_->locked_backend()->auto_flush(true);
            stream_sink_->set_filter(expr::attr< severity_level >("Severity") >= level_ &&
                log_type_attr == DEBUG_TYPE);
            stream_sink_->set_formatter(&formatter);

            logging::core::get()->add_sink(stream_sink_);
        }

        // Add some attributes too
        if (!global_inited_) {
            logging::core::get()->add_global_attribute("TimeStamp", attrs::local_clock());
            logging::core::get()->add_global_attribute("ThreadID", attrs::current_thread_id());
        }

        global_inited_ = true;
    }

    /**
     * @brief   Set the log filtering level.
     *
     * @param   level     The log filtering level.
     */
    void SetLevel(severity_level level) {
      level_ = level;
      if (!log_dir_.empty()) {
            file_sink_->set_filter(expr::attr< severity_level >("Severity") >= level_ &&
                log_type_attr == DEBUG_TYPE);
      } else {
        stream_sink_->set_filter(expr::attr< severity_level >("Severity") >= level_ &&
                log_type_attr == DEBUG_TYPE);
      }
    }

    /**
     * @brief   enable buffer log when log_dir_ is empty. log record will be written into a buffer stream. call this method after Init.
     */
    void EnableBufferMode() {
        if (!buffer_log_mode_ && log_dir_.empty()) {
            log_buffer_.str("");
            stream_sink_->locked_backend()->remove_stream(console_stream_);
            stream_sink_->locked_backend()->add_stream(
                boost::shared_ptr< std::ostream >(buffer_log_stream_));
            buffer_log_mode_ = true;
        }
    }

    /**
     * @brief   disable buffer log when log_dir_ is empty. log record will be written into console. call this method after Init.
     */
    void DisableBufferMode() {
        if (buffer_log_mode_ && log_dir_.empty()) {
            log_buffer_.str("");
            stream_sink_->locked_backend()->remove_stream(buffer_log_stream_);
            stream_sink_->locked_backend()->add_stream(
                boost::shared_ptr< std::ostream >(console_stream_));
            buffer_log_mode_ = false;
        }
    }

    /**
     * @brief   flush buffered log record to std::cerr.
     */
    void FlushBufferLog() {
        std::cerr << log_buffer_.str();
        log_buffer_.str("");
    }

    void FlushAllSinks() {
        logging::core::get()->flush();
    }

    /**
     * @brief   Get current log filtering level.
     *
     * @returns   current log filtering level.
     */
    severity_level GetLevel() { return level_; }

    /**
     * @brief   Check if logger manager is already inited.
     *
     * @returns   true if inited, flase if not.
     */
    bool IsInited() { return global_inited_; }

    /**
     * @brief   Get a instance of LoggerManager class.
     *
     * @returns   a instance of LoggerManager class.
     */
    static LoggerManager& GetInstance() {
      static LoggerManager instance;
      return instance;
    }
};

BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(debug_logger, src::severity_logger_mt< severity_level >) {
    src::severity_logger_mt< severity_level > lg;
    attrs::constant< std::string > debug_type(DEBUG_TYPE);
    lg.add_attribute("LogType", debug_type);

    // Init empty console log first if not inited
    if (!LoggerManager::GetInstance().IsInited()) {
      boost::shared_ptr< stream_sink > empty_sink =
          boost::shared_ptr< stream_sink > (new stream_sink());
      empty_sink->locked_backend()->add_stream(
          boost::shared_ptr< std::ostream >(&std::cout, boost::null_deleter()));
      empty_sink->locked_backend()->auto_flush(true);
      logging::core::get()->add_sink(empty_sink);
    }
    return lg;
};

BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(audit_logger, src::logger_mt) {
  src::logger_mt lg;
  attrs::constant< std::string > audit_type(AUDIT_TYPE);
  lg.add_attribute("LogType", audit_type);
  return lg;
};

class AuditLogger {
 private:
    std::string log_dir_;
    size_t rotation_size_ = 256*1024*1024;
    boost::shared_ptr< file_sink > audit_sink_;
    bool global_inited_ = false;

 public:
    void Init(std::string log_dir = "logs/", size_t rotation_size = 256*1024*1024) {
        // Set up log directory
        log_dir_ = log_dir;
        if (log_dir_.back() != '/') {
            log_dir_ += '/';
        }
        rotation_size_ = rotation_size;

        // Set up sink for audit log
        audit_sink_ = boost::shared_ptr< file_sink > (new file_sink(
            keywords::file_name = log_dir_ + AUDIT_PREFIX + "%Y%m%d_%H%M%S.log",
            keywords::open_mode = std::ios_base::out | std::ios_base::app,
            keywords::enable_final_rotation = false,
            keywords::auto_flush = true,
            keywords::rotation_size = rotation_size_));
        audit_sink_->locked_backend()->set_file_collector(sinks::file::make_collector(
            keywords::target = log_dir_));
        audit_sink_->locked_backend()->scan_for_files();
        audit_sink_->set_filter(log_type_attr == AUDIT_TYPE);

        // Add sinks to log core
        logging::core::get()->add_sink(audit_sink_);

        global_inited_ = true;
    }

    // write a json record to log file.
    void WriteLog(json log_msg) {
        AUDIT_LOG() << log_msg.dump();
    }

    // remove sink from log core
    void Close() {
        logging::core::get()->remove_sink(audit_sink_);
    }

    static AuditLogger& GetInstance() {
        static AuditLogger instance;
        return instance;
    }
};

class FatalLogger {
 public:
    FatalLogger(std::string file, int line)
        : file_(std::move(file)), line_(line) {}
    ~FatalLogger() {
      BOOST_LOG_SEV(debug_logger::get(), severity_level::FATAL)
          << logging::add_value("Line", line_)
          << logging::add_value("File", file_) << stream_.str();
      LoggerManager::GetInstance().FlushAllSinks();
      std::abort();
    }
    template <typename T>
    FatalLogger& operator<<(const T& value) {
      stream_ << value;
      return *this;
    }

 private:
    std::ostringstream stream_;
    std::string file_;
    int line_;
};

}  // namespace lgraph_log
