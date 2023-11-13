//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once

#include <future>
#include <regex>

#include "fma-common/file_system.h"
#include "fma-common/logger.h"

namespace fma_common {
// a log device that writes to rotating files
// file suffix will start from .0 and increase to .1, .2, ...
// a current log file is closed after it reaches max_file_size
// oldest log files are removed if total number of files exceeds max_n_files
class RotatingFileLogDevice : public LogDevice {
    const std::string dir_;
    const std::string file_prefix_;
    size_t max_file_size_;
    size_t max_n_files_;

    int64_t curr_file_idx_;
    std::ofstream curr_file_;
    size_t curr_file_size_ = 0;
    std::mutex mutex_;

    // construct a read-only log device for listing log files
    RotatingFileLogDevice(int, const std::string& dir, const std::string& file_prefix)
        : dir_(dir), file_prefix_(file_prefix) {}

 public:
    RotatingFileLogDevice(const std::string& dir, const std::string& file_prefix,
                          size_t max_log_file_size = (size_t)1 << 30,
                          size_t max_n_files = std::numeric_limits<size_t>::max())
        : RotatingFileLogDevice(0, dir, file_prefix) {
        max_file_size_ = std::max<size_t>(max_log_file_size, 1);
        max_n_files_ = std::max<size_t>(max_n_files, 1);
        // list existing files
        std::vector<std::string> log_files = ListExistingLogFiles();
        curr_file_idx_ = 0;
        if (!log_files.empty()) {
            RemoveLogFilesIfNecessary(log_files);
            std::string last_file = log_files.back();
            std::string last_file_name = _STD_FS::path(last_file).filename().string();
            curr_file_idx_ = GetLogFileIdx(last_file_name);
        }
        // check curr_file size
        std::ifstream in(GetLogFilePath(curr_file_idx_));
        if (in.good()) {
            auto fbeg = in.tellg();
            in.seekg(0, std::ios::end);
            size_t fsize = in.tellg() - fbeg;
            if (fsize >= max_log_file_size) {
                log_files.push_back(GetLogFilePath(curr_file_idx_));
                curr_file_idx_++;
                RemoveLogFilesIfNecessary(log_files);
            } else {
                curr_file_size_ = fsize;
            }
        }
        in.close();
        // now open curr_file
        curr_file_.open(GetLogFilePath(curr_file_idx_), std::ios::app | std::ios::binary);
        assert(curr_file_.good());
    }

    ~RotatingFileLogDevice() override {
        std::lock_guard<std::mutex> l(mutex_);
        curr_file_.flush();
    }

    void WriteLine(const char* p, size_t s, LogLevel level) override {
        std::lock_guard<std::mutex> l(mutex_);
        curr_file_.write(p, s);
        curr_file_size_ += s;
        if (curr_file_size_ >= max_file_size_) {
            RotateFile();
        }
    }

    virtual void Flush() {
        std::lock_guard<std::mutex> l(mutex_);
        curr_file_.flush();
    }

    static std::vector<std::string> ListLogFiles(const std::string& dir,
                                                 const std::string& prefix) {
        return RotatingFileLogDevice(0, dir, prefix).ListExistingLogFiles();
    }

 private:
    void RotateFile() {
        curr_file_.close();
        curr_file_idx_++;
        std::string curr_path = GetLogFilePath(curr_file_idx_);
        curr_file_.open(curr_path, std::ios::app | std::ios::binary);
        assert(curr_file_.good());
        curr_file_size_ = 0;
        // remove files if necessary
        RemoveLogFilesIfNecessary(ListExistingLogFiles());
    }

    std::string GetLogFilePath(int64_t idx) const {
        _STD_FS::path file_path(dir_);
        std::string file_name = file_prefix_ + "." + std::to_string(idx);
        file_path /= file_name;
        return file_path.string();
    }

    std::vector<std::string> ListExistingLogFiles() const {
        std::map<int64_t, std::string> log_files;
        for (_STD_FS::directory_iterator it(dir_); it != _STD_FS::directory_iterator(); ++it) {
#if FMA_USE_BOOST_FS
            if (it->status().type() == _STD_FS::file_type::regular_file) {
#else
            if (it->status().type() == _STD_FS::file_type::regular) {
#endif
                auto& path = it->path();
                std::string name = path.filename().string();
                int64_t idx = GetLogFileIdx(name);
                if (idx == -1) continue;
                log_files.emplace(idx, path.string());
            }
        }
        std::vector<std::string> files;
        for (auto& kv : log_files) {
            files.push_back(kv.second);
        }
        return files;
    }

    // returns log file index
    // returns -1 if not a log file
    int64_t GetLogFileIdx(const std::string& file_name) const {
#if 1
        // regex in gcc 4.8 is buggy
        if (!fma_common::StartsWith(file_name, file_prefix_ + ".")) return -1;
        if (file_name.size() <= file_prefix_.size() + 1) return -1;
        int64_t idx = 0;
        for (size_t i = file_prefix_.size() + 1; i < file_name.size(); i++) {
            char c = file_name[i];
            if (c < '0' || c > '9') return -1;
            idx = idx * 10 + c - '0';
        }
        return idx;
#else
        std::regex file_name_pattern(std::string("(") + file_prefix_ + ")(\\.)([0-9]+)");
        std::smatch sm;
        if (!std::regex_match(file_name, sm, file_name_pattern)) return -1;
        // found a log file
        std::string idx_string = sm[sm.size() - 1];
        return atoll(idx_string.c_str());
#endif
    }

    void RemoveLogFilesIfNecessary(const std::vector<std::string>& log_files) const {
        // remove log files if necessary
        if (log_files.size() > max_n_files_) {
            std::vector<std::future<bool>> remove_actions;
            size_t n_remove = log_files.size() - max_n_files_;
            for (size_t i = 0; i < n_remove; i++) {
                std::string f = log_files[i];
                remove_actions.emplace_back(std::async([f]() {
                    _EC_TYPE_ ec;
                    return _STD_FS::remove(f, ec);
                }));
            }
            for (auto& f : remove_actions) f.wait();
        }
    }
};  //NOLINT
}  // namespace fma_common
