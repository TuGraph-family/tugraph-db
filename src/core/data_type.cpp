/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "fma-common/file_system.h"

#include "core/data_type.h"
#include "core/task_tracker.h"

namespace lgraph {
AutoCleanDir::AutoCleanDir(const std::string& dir, bool create_empty) {
    dir_ = dir;
    fma_common::file_system::RemoveDir(dir_);
    if (!fma_common::file_system::MkDir(dir_))
        throw std::runtime_error("Failed to create dir " + dir_);
}

void AutoCleanDir::Clean() { fma_common::file_system::RemoveDir(dir_); }

void AutoCleanDir::ReCreate() {
    fma_common::file_system::RemoveDir(dir_);
    fma_common::file_system::MkDir(dir_);
}

AutoCleanDir::~AutoCleanDir() {
    if (!dir_.empty()) fma_common::file_system::RemoveDir(dir_);
}

SubProcess::SubProcess(const std::string& cmd, bool print_output) {
    proc_.reset(new TinyProcessLib::Process(
        cmd, "./",
        [this, print_output](const char* b, size_t s) {
            if (print_output) FMA_LOG() << std::string(b, s);
            std::lock_guard<std::mutex> l(out_mtx_);
            stdout_.append(b, s);
            out_cv_.notify_all();
        },
        [this, print_output](const char* b, size_t s) {
            if (print_output) FMA_LOG() << std::string(b, s);
            std::lock_guard<std::mutex> l(out_mtx_);
            stderr_.append(b, s);
            out_cv_.notify_all();
        }));
}

SubProcess::~SubProcess() {
    Kill();
    Wait();
}

bool SubProcess::ExpectOutput(const std::string& pattern, size_t n_milliseconds) {
    int64_t timeout = static_cast<int64_t>(n_milliseconds);
    if (timeout <= 0) timeout = std::numeric_limits<int64_t>::max();
    size_t out_offset = 0;
    size_t err_offset = 0;
    auto start = std::chrono::system_clock::now();
    std::unique_lock<std::mutex> l(out_mtx_);
    bool found = true;
    bool process_dead = false;
    while (stdout_.find(pattern, out_offset) == stdout_.npos &&
           stderr_.find(pattern, err_offset) == stderr_.npos) {
        if (!CheckIsAlive()) {
            if (process_dead) return false;
            process_dead = true;
            out_cv_.wait_for(l, std::chrono::milliseconds(500));
        }
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() -
                                                                  start)
                .count() > timeout)
            return false;
        out_offset = stdout_.size() >= pattern.size() ? stdout_.size() - pattern.size() : 0;
        err_offset = stderr_.size() >= pattern.size() ? stderr_.size() - pattern.size() : 0;
        out_cv_.wait_for(l, std::chrono::milliseconds(100));
    }
    return true;
}

void SubProcess::Kill() {
    if (proc_) proc_->kill();
}

bool SubProcess::Wait(size_t n_ms) {
    int64_t timeout = static_cast<int64_t>(n_ms);
    if (timeout <= 0) timeout = std::numeric_limits<int64_t>::max();
    auto t1 = std::chrono::system_clock::now();
    while (proc_) {
        auto t2 = std::chrono::system_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() > timeout) {
            return false;
        }
        if (proc_->try_get_exit_status(exit_code_)) {
            proc_.reset();
            return true;
        }
        ThrowIfTaskKilled();
        fma_common::SleepUs(100000);
    }
    return true;
}

int SubProcess::GetExitCode() const { return exit_code_; }

bool SubProcess::CheckIsAlive() {
    if (proc_) {
        if (proc_->try_get_exit_status(exit_code_)) {
            proc_.reset();
            return false;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

std::string SubProcess::Stdout() {
    std::lock_guard<std::mutex> l(out_mtx_);
    return stdout_;
}

std::string SubProcess::Stderr() {
    std::lock_guard<std::mutex> l(out_mtx_);
    return stderr_;
}

}  // namespace lgraph
