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

#include <condition_variable>
#include <atomic>
#include <mutex>
#include <queue>
#include <thread>

#include "fma-common/file_system.h"
#include "lmdb/lmdb.h"
#include "lgraph/lgraph_types.h"

#include "core/data_type.h"
#include "core/defs.h"
#include "core/kv_store_exception.h"
#include "core/kv_store_iterator.h"
#include "core/kv_store_table.h"
#include "core/kv_store_transaction.h"


namespace lgraph {

class Wal;

class KvStore {
    friend class KvTable;
    friend class KvTransaction;
    friend class Wal;

    MDB_env* env_ = nullptr;

    std::mutex mutex_;
    std::string path_;
    size_t db_size_;
    bool durable_;

    std::queue<KvTransaction*> queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::atomic<bool> finished_;
    std::thread validator_;

    size_t wal_log_rotate_interval_ms_;
    size_t wal_batch_commit_interval_ms_;
    std::unique_ptr<Wal> wal_;

    void Open(bool create_if_not_exist);

    void ReopenFromSnapshot(const std::string& snapshot_path);

    void ServeValidation();

    // last op id of all stores, monotonically increasing id so we don't repeatedly apply the same
    // request During initialization, this is updated when any store is opened. Larger id wins. This
    // value is written in HandleRequest with the op id, and When any transaction commits, this is
    // written to the store.
    static std::atomic<int64_t> last_op_id_;

 public:
    DISABLE_COPY(KvStore);
    DISABLE_MOVE(KvStore);

    /**
     * Constructor
     *
     * \param           path    Location where data is stored. For LMDB, this is a directory.
     * \param           db_size (Optional) Maximum size of the database. If 0, then 4TB is used.
     * \param           durable (Optional) If true, DB will always call fsync on write transaction,
     *                          making the store durable. Otherwise, fsync will not be called, and
     *                          you may lose some data if the OS crashes or power is lost.
     */
    KvStore(const std::string& path,
#ifdef USE_VALGRIND
            size_t db_size = (size_t)1 << 30,
#else
            size_t db_size = _detail::DEFAULT_GRAPH_SIZE,
#endif
            bool durable = false,
            bool create_if_not_exist = true,
            size_t wal_log_rotate_interval_ms = 60 * 1000,
            size_t wal_batch_commit_interval_ms = 10);

    ~KvStore();

    /**
     * Creates read transaction. A read-only transaction can only read, but not write any table. Any
     * write operation will cause an KvExcpetion to be thrown.
     *
     * \return  The new read transaction.
     */
    KvTransaction CreateReadTxn();

    /**
     * Creates write transaction.
     & Please note that optimistic write transactions cannot support DUPSORT tables right now.
     *
     * \param   optimistic    (Optional) True to issue optimistic writes.
     *
     * \return  The new write transaction.
     */
    KvTransaction CreateWriteTxn(bool optimistic = false);

    /**
     * Opens a table and adds it to the OpendTable list. If create_if_not_exist is true, then the
     * table is created if it does not exist.
     *
     * \note    This operation is guarded by mutex
     *
     * \param [in,out]  txn                 The transaction.
     * \param           table_name          Name of the table.
     * \param           create_if_not_exist True to create if not exist.
     * \param           desc                Decription of comparator.
     *
     * \return  A Table object.
     */
    KvTable OpenTable(KvTransaction& txn,
        const std::string& table_name,
        bool create_if_not_exist,
        const ComparatorDesc& desc);

    /*
    * Just for test. Should only be used in KvStore unit tests. This call will not be recorded
    * for replayed in WAL.
    */
    KvTable _OpenTable_(KvTransaction& txn,
        const std::string& table_name,
        bool create_if_not_exist,
        const KeySortFunc& func);

    /**
     * Deletes a table in the KvStore.
     *
     * \param [in,out]  txn                     The transaction.
     * \param           table_name              Name of the table.
     *
     * \return  An ErrorCode. OK, WRITE_IN_READ_TXN.
     */
    bool DeleteTable(KvTransaction& txn, const std::string& table_name);

    /**
     * Get the names of all the tables in the KvStore.
     *
     * \param [in,out]  txn The transaction.
     *
     * \return  A std::vector&lt;std::string&gt;
     */
    std::vector<std::string> ListAllTables(KvTransaction& txn);

    /**
     * Flushes this db to disk. Useful if the kv store is opened with durable=false.
     *
     * \return  An ErrorCode.
     */
    void Flush();

    void DropAll(KvTransaction& txn);

    void DumpStat(KvTransaction& txn, size_t& memory_size, size_t& height);

    size_t Backup(const std::string& path, bool compact = false);

    void Snapshot(KvTransaction& txn, const std::string& path, bool compaction = false);

    void LoadSnapshot(const std::string& snapshot_path);

    void WarmUp(size_t* size);

    static int64_t GetLastOpIdOfAllStores() { return last_op_id_.load(std::memory_order_acquire); }

    // update last op id, should be called only by one thread
    // and caller must ensure that stored value is larger than existing one
    static void SetLastOpIdOfAllStores(int64_t id) {
        last_op_id_.store(id, std::memory_order_release);
    }

    // called when a store is opened.
    // Replaces last_op_id_ with stored iff stored > last_op_id_
    static void UpdateLastOpIdWithStoredValue(int64_t stored) {
        int64_t existing = last_op_id_.load(std::memory_order_acquire);
        while (existing < stored) {
            if (last_op_id_.compare_exchange_strong(existing, stored)) break;
        }
    }

    Wal* GetWal() const { return wal_.get();}
};

struct KvTypes {
    typedef KvStore Store;
    typedef KvTransaction Transaction;
    typedef KvTable Table;
    typedef KvIterator Iterator;
};
}  // namespace lgraph
