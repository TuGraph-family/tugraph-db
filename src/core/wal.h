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

#include "fma-common/timed_task.h"
#include "core/kv_table_comparators.h"
#include "./sync_file.h"

namespace lgraph {

class Wal {
    // Each wal will be access by at most one thread at a time. This is guaranteed by
    // the single-writer design of LMDB. So the WriteXXX() functions do not need locking.
    // The only data exception is the access of the list of waiting txns, which
    // is always guarded with a mutex and a condition variable.

    // transaction status waiting for batch commit
    struct WaitingTxn {
        WaitingTxn(mdb_size_t tid, SyncFile* f) : txn_id(tid), file(f) {}

        mdb_size_t txn_id;
        std::promise<void> promise;
        SyncFile* file;
    };

 private:
    MDB_env* env_;
    std::string log_dir_;
    std::atomic<bool> exit_flag_;
    // the log_file records txn and kv operations
    uint64_t next_log_file_id_ = 0;
    std::atomic<SyncFile*> log_file_;
    // this file records only meta-operations like opening and dropping tables
    SyncFile dbi_file_;
    mdb_size_t curr_txn_id_;
    std::atomic<int64_t> op_id_;    // -1 indicates completion of current txn

    std::mutex mutex_;
    std::condition_variable cond_;
    std::deque<WaitingTxn> waiting_txns_;
    size_t log_rotate_interval_;
    std::chrono::system_clock::time_point last_log_rotate_time_;
    size_t batch_time_ms_ = 50;
    std::chrono::system_clock::time_point last_batch_time_;
    // The flusher thread flushes wal on constant intervals and notifies waiting
    // txns. It also prepares new log files for rotation.
    std::thread wal_flusher_;

 public:
    Wal(MDB_env* env,
        const std::string& log_dir,
        size_t log_rotate_interval_ms,
        size_t batch_commit_interval_ms);

    ~Wal();

    void WriteKvPut(MDB_dbi dbi, const Value& key, const Value& value);

    void WriteKvDel(MDB_dbi, const Value& key);

    void WriteTableOpen(MDB_dbi dbi,
                        const std::string& name, const ComparatorDesc& desc);

    void WriteTableDrop(MDB_dbi dbi);

    void WriteTxnBegin(mdb_size_t txn_id, bool is_child = false);

    // write txn commit message
    std::future<void> WriteTxnCommit(mdb_size_t txn_id,
                                     bool is_child);

    void WriteTxnAbort(mdb_size_t txn_id, bool is_child = false);

    // wait for flush, used with WriteTxnCommit(force_sync=false)
    void WaitForWalFlush(std::future<void>& future);

 private:
    // replay all logs, called by constructor
    void ReplayLogs();

    // Called on transaction commit, when we flush the wal. This guarantees that each
    // log file will contain a complete transaction.
    // This function checks whether the last flush time is too long ago. If that is the
    // case, close current log file and create a new one. It also schedules an asynchronous
    // task to flush the kv store and delete the old log file.
    void FlusherThread();

    // open next log file, changing curr_log_path_ and next_log_file_id
    void OpenNextLogForWrite();

    // Get log path from id
    std::string GetLogFilePathFromId(uint64_t log_file_id) const;

    // Get dbi file path
    std::string GetDbiFilePath() const;
};

}  // namespace lgraph

