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
#include <chrono>
#include <map>
#include <thread>

#include "tools/lgraph_log.h"
#include "core/data_type.h"
#include "fma-common/file_system.h"
#include "fma-common/fma_stream.h"
#include "fma-common/rw_lock.h"
#include "fma-common/stream_base.h"
#include "fma-common/type_traits.h"
#include "lgraph/lgraph_types.h"
#include "protobuf/ha.pb.h"

#ifdef _WIN32
#include <time.h>

#include <iomanip>
#include <sstream>
inline char* strptime(const char* s, const char* f, struct tm* tm) {
    // Isn't the C++ standard lib nice? std::get_time is defined such that its
    // format parameters are the exact same as strptime. Of course, we have to
    // create a string stream first, and imbue it with the current C locale, and
    // we also have to make sure we return the right things if it fails, or
    // if it succeeds, but this is still far simpler an implementation than any
    // of the versions in any of the C standard libraries.
    std::istringstream input(s);
    input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
    input >> std::get_time(tm, f);
    if (input.fail()) {
        return nullptr;
    }
    return (char*)(s + input.tellg());
}
#endif

/*
 * All time (represented by seconds<int64_t>) and string used in this .h should be local
 */
namespace lgraph {

struct AuditLog {
    int64_t index = -1;
    std::string begin_time;
    std::string end_time;
    std::string user;
    std::string graph;
    std::string type;
    std::string read_write;
    bool success = false;
    std::string content;
};

class AuditLogger {
    template <typename LOCK>
    using AutoReadLock = fma_common::AutoReadLock<LOCK>;
    template <typename LOCK>
    using AutoWriteLock = fma_common::AutoWriteLock<LOCK>;

    using RWLock = fma_common::RWLock;

    static std::atomic<bool> enabled_;

    fma_common::OutputFmaStream file_;
    std::string file_name_;
    const size_t file_size_limit_ = 1024l * 1024l * 64l;  // 64MB
    RWLock lock_;
    std::string dir_;
    int64_t index_;
    char line_[64 * 1024];
    int64_t last_log_time_;
    size_t expire_second_;  // s

    AuditLogger() {}
    DISABLE_COPY(AuditLogger);
    DISABLE_MOVE(AuditLogger);

 public:
    // take string of local time, then convert it to second (int64_t) of local time
    static inline bool FilePathToTime(int64_t& t, const std::string& path) {
        if (path.size() != 25) return false;
        if (path.find(AUDIT_PREFIX) == 0) return false;
        const std::string s = path.substr(6, 15);

#ifdef _WIN32
        lgraph_api::DateTime::YMDHMSF ymdhms;
        const char* p = s.c_str();
        size_t r = fma_common::TextParserUtils::ParseT(p, p + 4, ymdhms.year) +
                   fma_common::TextParserUtils::ParseT(p + 4, p + 6, ymdhms.month) +
                   fma_common::TextParserUtils::ParseT(p + 6, p + 8, ymdhms.day) +
                   fma_common::TextParserUtils::ParseT(p + 9, p + 11, ymdhms.hour) +
                   fma_common::TextParserUtils::ParseT(p + 11, p + 13, ymdhms.minute) +
                   fma_common::TextParserUtils::ParseT(p + 13, p + 15, ymdhms.second);
        if (r != 14) return false;
        t = lgraph_api::DateTime(ymdhms).MicroSecondsSinceEpoch();
        return true;
#else
        struct tm timeinfo;
        memset(&timeinfo, 0, sizeof(struct tm));
        char* p = strptime(s.c_str(), "%Y%m%d_%H%M%S", &timeinfo);
        if (p - s.c_str() != 15) return false;
        time_t tmp = mktime(&timeinfo);
        t = lgraph_api::DateTime(std::chrono::system_clock::from_time_t(tmp))
                .ConvertToLocal()
                .MicroSecondsSinceEpoch();
        return true;
#endif
    }

    // take second (int64_t) of local time, then convert it to string
    static inline bool TimeToString(std::string& ret, int64_t rt = 0) {
        if (rt < 0) return false;
        lgraph_api::DateTime::YMDHMSF t =
            (rt == 0)
            ? lgraph_api::DateTime::LocalNow().GetYMDHMSF()
            : lgraph_api::DateTime(rt).GetYMDHMSF();
        ret.resize(16);
        int i = snprintf(&ret[0], ret.size(), "%4d%02d%02d_%02d%02d%02d",
            t.year, t.month, t.day, t.hour, t.minute, t.second);
        ret.resize(static_cast<size_t>(i));
        return true;
    }

    inline bool SetLogFileName(int64_t rt = 0) {
        std::string name;
        bool r = TimeToString(name, rt);
        if (r) file_name_ = dir_ + "/" + name;
        return r;
    }

    inline bool ReadNextAuditLog(fma_common::InputFmaStream& input, lgraph::LogMessage& msg) {
        int len;
        // fix #55 [AUDIT_LOG] Error reading audit log's size from file
        // Read() return 0 when EOF is reached, in this case, just return instead of issuing an
        // error message
        size_t s = input.Read((char*)(&len), sizeof(len));
        if (s == 0) return false;
        if (s != sizeof(len)) {
            LOG_DEBUG() << "Error reading audit log's size from file " << input.Path();
            return false;
        }

        if (len >= (1 << 16)) {  // > 64KB
            std::string buf(len, 0);
            if (input.Read(&buf[0], len) != (size_t)len) {
                LOG_DEBUG() << "Error reading audit log from file " << input.Path();
                return false;
            }
            return msg.ParseFromArray(&buf[0], len);
        } else {
            if (input.Read(line_, len) != (size_t)len) {
                LOG_DEBUG() << "Error reading audit log from file " << input.Path();
                return false;
            }
            return msg.ParseFromArray(line_, len);
        }
    }

    inline bool ParseAuditLog(std::string path, std::string& log_line, lgraph::LogMessage& msg) {
        if (log_line == "") {
            LOG_DEBUG() << "Error parsing audit log from string " << path;
            return false;
        }
        lgraph_log::json log_msg = lgraph_log::json::parse(log_line);
        msg.set_index(log_msg["index"]);
        msg.set_time(log_msg["time"]);
        msg.set_begin_end(log_msg["is_end_log"]);
        msg.set_success(log_msg["success"]);
        msg.set_content(log_msg["content"]);
        msg.set_read_write(log_msg["read_write"]);
        if (!log_msg["is_end_log"]) {
            msg.set_user(log_msg["user"]);
            msg.set_graph(log_msg["graph"]);
            msg.set_type(log_msg["type"]);
        }
        return true;
    }

    static bool IsEnabled() { return enabled_.load(std::memory_order_acquire); }

    static void SetEnable(bool enable) { enabled_.store(enable, std::memory_order_release); }

    static AuditLogger& GetInstance() {
        static AuditLogger instance_;
        return instance_;
    }

    void Init(const std::string& dir, size_t expire_hour) {
        AutoWriteLock<RWLock> lock(lock_);
        file_.Close();
        enabled_ = true;
        expire_second_ = expire_hour * 3600;
        last_log_time_ = (int64_t)0;
        dir_ = dir;
        auto& fs = fma_common::FileSystem::GetFileSystem(dir_);
        if (!fs.IsDir(dir_)) {
            if (!fs.Mkdir(dir_)) {
                LOG_ERROR() << "Failed to create the audit log dir [" << dir_ << "].";
            }
        }

        std::vector<size_t> fsizes;
        auto files = fs.ListFiles(dir_, &fsizes);
        for (size_t i = 0; i < files.size(); i++) {
            if (fsizes[i] == 0) fma_common::file_system::RemoveFile(files[i]);
        }
        sort(files.begin(), files.end());
        index_ = GetLogIdx();
        LOG_DEBUG() << "VertexIndex of audit log : " << index_;

        // init lgraph_log AuditLogger
        lgraph_log::AuditLogger::GetInstance().Init(dir_);

        // thread clear the expired AuditLog in certain time, execute each hour
        if (expire_second_ > 0) {
            std::thread([&]() {
                while (true) {
                    auto x =
                        std::chrono::steady_clock::now() + std::chrono::seconds(3600);  // 1 hour
                    {
                        AutoWriteLock<RWLock> lock(lock_);
                        int64_t tnow = lgraph_api::DateTime::LocalNow().MicroSecondsSinceEpoch();
                        tnow = tnow / 1000000;
                        auto& fs = fma_common::FileSystem::GetFileSystem(dir_);
                        auto files = fs.ListFiles(dir_, nullptr);
                        LOG_DEBUG()
                            << "Clear the expired audit log file. files.size() = " << files.size();
                        if (files.size() > 1) {
                            sort(files.begin(), files.end());
                            for (size_t f_i = 0; f_i < files.size() - 1; f_i++) {
                                std::string f2 = fs.GetFileName(files[f_i + 1]);
                                int64_t t2 = tnow;
                                if (!FilePathToTime(t2, f2)) {
                                    LOG_WARN() << "Failed to get time from audit log file name ["
                                               << f2 << "]";
                                } else {
                                    t2 = t2 / 1000000;
                                    if (tnow - t2 > static_cast<int64_t>(expire_second_)) {
                                        if (!fs.Remove(files[f_i]))
                                            LOG_WARN() << "Clear audit log file "
                                                                     << files[f_i] << " failed.";
                                        else
                                            LOG_WARN()
                                                << "Clear audit log file " << files[f_i]
                                                << " successful.";
                                    }
                                }
                            }
                        }
                    }
                    std::this_thread::sleep_until(x);
                }
            }).detach();
        }
    }

    int64_t GetLogIdx() {
        auto& fs = fma_common::FileSystem::GetFileSystem(dir_);
        auto files = fs.ListFiles(dir_, nullptr);
        if (files.empty()) return 0;
        sort(files.begin(), files.end());

        int64_t begin_idx = 0;
        int64_t end_idx = 0;
        for (int64_t f_i = (int64_t)files.size() - 1; f_i >= 0; f_i--) {
            fma_common::InputFmaStream input(files[f_i]);
            if (!input.Good()) throw std::runtime_error("Failed to open audit log file for index.");
            if (input.Size() == 0) {
                LOG_DEBUG() << "GetLogIdx. Skip " << files[f_i] << " for file_size=0";
                continue;
            }

            lgraph::LogMessage msg;
            fma_common::StreamLineReader reader(input);
            auto lines = reader.ReadAllLines();
            for (auto& line : lines) {
                if (!ParseAuditLog(input.Path(), line, msg)) break;  // broken log entry
                int64_t idx = msg.index();
                if (!msg.begin_end()) {  // begin
                    if (begin_idx < idx) begin_idx = idx;
                } else {  // end
                    if (end_idx < idx) end_idx = idx;
                }
            }
            input.Close();
            if (begin_idx != 0) break;
        }
        return std::max(begin_idx, end_idx);
    }

    int64_t WriteLog(const bool is_end_log, const std::string& user, const std::string& graph,
                     const lgraph::LogApiType type, const bool read_write, const bool success,
                     const std::string& content, int64_t idx = 0) {
        AutoWriteLock<RWLock> lock(lock_);
        int64_t log_time = lgraph_api::DateTime::LocalNow().MicroSecondsSinceEpoch();

        last_log_time_ = log_time;

        int64_t index;
        lgraph::LogMessage msg;
        if (idx != 0)
            index = idx;
        else
            index = ++index_;

        lgraph_log::json log_msg;
        log_msg["index"] = index;
        log_msg["time"] = log_time;
        log_msg["is_end_log"] = is_end_log;
        log_msg["success"] = success;
        log_msg["content"] = content;
        log_msg["read_write"] = read_write;
        if (!is_end_log) {
            log_msg["user"] = user;
            log_msg["graph"] = graph;
            log_msg["type"] = type;
        }
        lgraph_log::AuditLogger::GetInstance().WriteLog(log_msg);

        return index;
    }

    std::vector<lgraph::AuditLog> GetLog(const std::string& begin, const std::string& end,
                                         const std::string& user = "", int limit = 100,
                                         bool descending_order = true) {
        int64_t begin_time, end_time, now;
        now = lgraph_api::DateTime::LocalNow().MicroSecondsSinceEpoch();
        lgraph_api::DateTime bt;
        if (!lgraph_api::DateTime::Parse(begin, bt))
            THROW_CODE(InputError, "Failed to parse begin time [{}].", begin);
        begin_time = bt.MicroSecondsSinceEpoch();
        if (!end.empty()) {
            lgraph_api::DateTime et;
            if (!lgraph_api::DateTime::Parse(end, et))
                THROW_CODE(InputError, "Failed to parse begin time [{}].", end);
            end_time = et.MicroSecondsSinceEpoch();
            if (end_time > now) end_time = now;
        } else {
            end_time = now;
        }
        LOG_DEBUG() << "Search audit log for user[" << user << "](" << user.length()
                                << ") from " << begin << " to "
                                << lgraph_api::DateTime(end_time).ToString() << ", limit " << limit
                                << ", log_order " << descending_order;
        return GetLog(begin_time, end_time, user, limit, descending_order);
    }

    std::vector<lgraph::AuditLog> GetLog(int64_t begin, int64_t end, const std::string& user = "",
                                         size_t limit = 100, bool descending_order = true) {
        {
            AutoWriteLock<RWLock> lock(lock_);
            file_.Flush();
        }
        std::map<int64_t, lgraph::AuditLog> logs;
        AutoReadLock<RWLock> lock(lock_);
        auto& fs = fma_common::FileSystem::GetFileSystem(dir_);
        auto files = fs.ListFiles(dir_, nullptr);
        sort(files.begin(), files.end());

        if (!descending_order) {  // old -> new
            bool finished = false;
            size_t finish_logs = 0;
            size_t miss_end_logs = 0;

            for (size_t f_i = 0; f_i < files.size(); f_i++) {
                if (finished) {
                    LOG_DEBUG() << "Search audit log finished.";
                    break;
                }

                // whether this file is later than end time of search
                std::string f = fs.GetFileName(files[f_i]);
                int64_t rawtime1;
                FilePathToTime(rawtime1, f);
                if (rawtime1 > end) {
                    finished = true;
                    LOG_DEBUG()
                        << "Search audit log finished. Skip later log file : " << f;
                    break;
                }

                // whether this file is earlier than begin time of search
                if (f_i < files.size() - 1) {
                    std::string f2 = fs.GetFileName(files[f_i + 1]);
                    int64_t rawtime2;
                    FilePathToTime(rawtime2, f2);
                    if (rawtime2 <= begin) {
                        LOG_DEBUG()
                            << "Search audit log. Skip earlier log file : " << f;
                        continue;
                    }
                }

                fma_common::InputFmaStream input(files[f_i]);
                if (!input.Good())
                    throw std::runtime_error("Failed to open audit log file for read.");
                if (input.Size() == 0) {
                    LOG_DEBUG()
                        << "Search audit log. Skip " << f << " for file size = 0";
                    continue;
                }

                lgraph::LogMessage msg;

                LOG_DEBUG() << "Search audit log " << f;
                fma_common::StreamLineReader reader(input);
                auto lines = reader.ReadAllLines();
                for (auto& line : lines) {
                    if (!ParseAuditLog(input.Path(), line, msg)) break;          // broken log entry
                    if ((user.length() != 0) && (user != msg.user()))  // check user
                        continue;
                    auto msg_time = msg.time();
                    if (msg_time < begin) continue;          // skip earlier log
                    if (!msg.begin_end()) {                  // begin log
                        if (msg_time > end) continue;        // skip later log
                        if (logs.size() == limit) continue;  // get enough logs
                        lgraph::AuditLog log;
                        log.index = msg.index();
                        log.begin_time = lgraph_api::DateTime(msg_time).ToString();
                        log.user = msg.user();
                        log.graph = msg.graph();
                        log.type = lgraph::LogApiType_Name(msg.type());
                        log.read_write = msg.read_write() ? "write" : "read";
                        log.success = msg.success();
                        log.content = msg.content();

                        logs[log.index] = log;
                    } else {  // end log
                        auto log = logs.find(msg.index());
                        if (log == logs.end()) {
                            miss_end_logs++;
                            if (miss_end_logs / 100 > (size_t)limit) {
                                finished = true;
                                LOG_DEBUG()
                                    << "Too many miss(" << miss_end_logs
                                    << "), meaning that there may be broken logs without end_log, "
                                       "so finish and break";
                                break;  // too many miss, meaning that there may be broken log
                                        // without end_log, so finish and break
                            }
                            continue;  // no this log, skip
                        }
                        log->second.end_time = lgraph_api::DateTime(msg_time).ToString();
                        log->second.success = msg.success();

                        if (log->second.success)
                            log->second.content = log->second.content + "    Successful";
                        else
                            log->second.content =
                                log->second.content + "    Failed: " + msg.content();

                        finish_logs++;
                        if (finish_logs == limit) {
                            finished = true;
                            break;
                        }
                    }
                }
                input.Close();
            }
        } else {  // new to old
            std::map<int64_t, lgraph::AuditLog> end_logs;
            // bool finished = false;
            size_t f_i = files.size();
            while (f_i != 0) {
                f_i--;
                // whether this file is later than end time of search
                std::string f = fs.GetFileName(files[f_i]);
                int64_t rawtime1;
                FilePathToTime(rawtime1, f);
                if (rawtime1 > end) {
                    LOG_DEBUG() << "Search audit log. Skip later log file : " << f;
                    continue;
                }

                // whether this file is earlier than begin time of search
                if (f_i < files.size() - 1) {
                    std::string f2 = fs.GetFileName(files[f_i + 1]);
                    int64_t rawtime2;
                    FilePathToTime(rawtime2, f2);
                    if (rawtime2 <= begin) {
                        LOG_DEBUG()
                            << "Search audit log finished. Skip earlier log file : " << f;
                        break;
                    }
                }

                fma_common::InputFmaStream input(files[f_i]);
                if (!input.Good())
                    throw std::runtime_error("Failed to open audit log file for read.");
                if (input.Size() == 0) {
                    LOG_DEBUG()
                        << "Search audit log. Skip " << f << " for file size = 0";
                    continue;
                }

                lgraph::LogMessage msg;

                LOG_DEBUG() << "Search audit log " << f;
                fma_common::StreamLineReader reader(input);
                auto lines = reader.ReadAllLines();
                for (auto& line : lines) {
                    if (!ParseAuditLog(input.Path(), line, msg)) break;          // broken log entry
                    if ((user.length() != 0) && (user != msg.user()))  // check user
                        continue;
                    auto msg_time = msg.time();
                    if (msg_time < begin) continue;    // skip earlier log
                    if (!msg.begin_end()) {            // begin log
                        if (msg_time > end) continue;  // skip later log
                        // if (logs.size() == limit) continue; // get enough logs
                        lgraph::AuditLog log;
                        log.index = msg.index();
                        log.begin_time = lgraph_api::DateTime(msg_time).ToString();
                        log.user = msg.user();
                        log.graph = msg.graph();
                        log.type = lgraph::LogApiType_Name(msg.type());
                        log.read_write = msg.read_write() ? "write" : "read";
                        log.success = msg.success();
                        log.content = msg.content();

                        logs[log.index] = log;
                        if (logs.size() > limit) logs.erase(logs.begin());
                    } else {  // end log
                        lgraph::AuditLog log;
                        log.index = msg.index();
                        log.end_time = lgraph_api::DateTime(msg_time).ToString();
                        log.user = msg.user();
                        log.graph = msg.graph();
                        log.type = lgraph::LogApiType_Name(msg.type());
                        log.read_write = msg.read_write() ? "write" : "read";
                        log.success = msg.success();
                        if (log.success)
                            log.content = "    Successful";
                        else
                            log.content = "    Failed: " + msg.content();

                        end_logs[log.index] = log;
                        if (end_logs.size() > limit) end_logs.erase(end_logs.begin());
                    }
                }
                input.Close();

                if (logs.size() == limit)
                    break;
                else if (logs.size() > limit)
                    LOG_DEBUG() << "Search audit log ERROR : logs.size() > limit";
            }
            auto iter_begin = logs.begin();
            auto iter_end = end_logs.begin();
            while ((iter_begin != logs.end()) && (iter_end != end_logs.end())) {
                if (iter_begin->first == iter_end->first) {
                    iter_begin->second.success = iter_end->second.success;
                    iter_begin->second.end_time = iter_end->second.end_time;
                    iter_begin->second.content += iter_end->second.content;
                    iter_begin++;
                    iter_end++;
                } else if (iter_begin->first > iter_end->first) {
                    iter_end++;
                } else if (iter_begin->first < iter_end->first) {
                    iter_begin++;
                } else {
                    LOG_DEBUG() << "Search audit log combine ERROR";
                }
            }
        }

        std::vector<lgraph::AuditLog> ret;
        if (!descending_order) {  // old -> new
            std::map<int64_t, lgraph::AuditLog>::iterator iter;
            for (iter = logs.begin(); iter != logs.end(); iter++) ret.emplace_back(iter->second);
        } else {  // new -> old
            std::map<int64_t, lgraph::AuditLog>::reverse_iterator iter;
            for (iter = logs.rbegin(); iter != logs.rend(); iter++) ret.emplace_back(iter->second);
        }

        // output
        LOG_DEBUG() << "Search audit log finished. Get " << ret.size() << " result";
        for (auto& r : ret) {
            LOG_DEBUG()
                << r.index << " " << r.begin_time << " " << r.end_time << " " << r.user << " "
                << r.graph << " " << r.type << " " << r.read_write << " " << r.success << "\n"
                << r.content << "\n";
        }

        return ret;
    }

    void Close() {
        AutoWriteLock<RWLock> lock(lock_);
        file_.Close();
        lgraph_log::AuditLogger::GetInstance().Close();
    }
};

class AuditLogTLS {
    int64_t curr_idx_ = 0;
    bool begun_ = false;

 public:
    int BeginAuditLog(const std::string& user, const std::string& graph,
                      const lgraph::LogApiType& type, bool is_write, const std::string& content) {
        if (_F_LIKELY(!begun_)) {
            begun_ = true;
            curr_idx_ = AuditLogger::GetInstance().WriteLog(false, user, graph, type, is_write,
                                                            true, content, 0);
        }
        return 0;
    }

    int EndSuccess() {
        if (_F_LIKELY(begun_)) {
            AuditLogger::GetInstance().WriteLog(true, "", "", lgraph::LogApiType::SingleApi, false,
                                                true, "", curr_idx_);
            begun_ = false;
        }
        return 0;
    }

    int EndFail(const std::string& msg) {
        if (_F_LIKELY(begun_)) {
            AuditLogger::GetInstance().WriteLog(true, "", "", lgraph::LogApiType::SingleApi, false,
                                                false, msg, curr_idx_);
            begun_ = false;
        }
        return 0;
    }

    static thread_local AuditLogTLS instance_;
};

#define BEG_AUDIT_LOG(user, graph, type, is_write, desc)                              \
    do {                                                                              \
        AuditLogger::IsEnabled()                                                      \
            ? AuditLogTLS::instance_.BeginAuditLog(user, graph, type, is_write, desc) \
            : 1;                                                                      \
    } while (0)
#define AUDIT_LOG_SUCC()                                                    \
    do {                                                                    \
        AuditLogger::IsEnabled() ? AuditLogTLS::instance_.EndSuccess() : 1; \
    } while (0)
#define AUDIT_LOG_FAIL(msg)                                                 \
    do {                                                                    \
        AuditLogger::IsEnabled() ? AuditLogTLS::instance_.EndFail(msg) : 1; \
    } while (0)
}  // namespace lgraph
