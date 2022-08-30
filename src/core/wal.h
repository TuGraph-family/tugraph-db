/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include <chrono>

#include "fma-common/timed_task.h"
#include "core/kv_table_comparators.h"
#include "./sync_file.h"

namespace lgraph {

class Wal {
    // Each wal will be access by at most one thread at a time. This is guaranteed by
    // the single-writer design of LMDB.
 private:
    MDB_env* env_;
    std::string log_dir_;
    size_t flush_interval_ms_;
    std::chrono::system_clock::time_point last_flush_time_;
    // the log_file records txn and kv operations
    std::string curr_log_path_;
    uint64_t next_log_file_id_ = 0;
    SyncFile log_file_;
    // this file records only meta-operations like opening and dropping tables
    std::string dbi_log_path_;
    SyncFile dbi_file_;
    mdb_size_t curr_txn_id_;

    std::mutex tasks_lock_;
    std::unordered_map<std::string, fma_common::TimedTaskScheduler::TaskPtr> delete_tasks_;
    int64_t op_id_ = 0;

 public:
    Wal(MDB_env* env, const std::string& log_dir, size_t flush_interval_ms);

    ~Wal();

    void WriteKvPut(MDB_dbi dbi, const Value& key, const Value& value);

    void WriteKvDel(MDB_dbi, const Value& key);

    void WriteTableOpen(MDB_dbi dbi,
                        const std::string& name, const ComparatorDesc& desc);

    void WriteTableDrop(MDB_dbi dbi);

    void WriteTxnBegin(mdb_size_t txn_id, bool is_child = false);

    // write txn commit message and wait for the wal to flush
    void WriteTxnCommit(mdb_size_t txn_id, bool is_child = false);

    void WriteTxnAbort(mdb_size_t txn_id, bool is_child = false);

 private:
    // replay all logs, called by constructor
    void ReplayLogs();

    // Called on transaction commit, when we flush the wal. This guarantees that each
    // log file will contain a complete transaction.
    // This function checks whether the last flush time is too long ago. If that is the
    // case, close current log file and create a new one. It also schedules an asynchronous
    // task to flush the kv store and delete the old log file.
    void FlushDbAndRotateLogIfNecessary();

    // open next log file, changing curr_log_path_ and next_log_file_id
    void OpenNextLogForWrite();

    // Get log path from id
    std::string GetLogFilePathFromId(uint64_t log_file_id) const;
};

}  // namespace lgraph
