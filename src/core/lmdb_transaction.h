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
#if (!LGRAPH_USE_MOCK_KV)
#include <vector>
#include <mutex>
#include <condition_variable>

#include "tools/lgraph_log.h"
#include "fma-common/type_traits.h"
#include "core/kv_engine.h"
#include "core/lmdb_exception.h"
#include "core/lmdb_table.h"
#include "core/lmdb_profiler.h"
#include "core/lmdb/lmdb.h"


namespace lgraph {
class LMDBKvIterator;

class LMDBKvTable;

class KvTransaction;

class LMDBKvStore;

class Wal;

class DeltaStore {
    friend class LMDBKvTable;
    friend class LMDBKvIterator;
    friend class LMDBKvStore;

    std::map<std::string, std::string, LMDBKvTable> write_set_;

 public:
    explicit DeltaStore(const LMDBKvTable& table);

    DeltaStore(const DeltaStore& rhs) = delete;

    DeltaStore(DeltaStore&& rhs) = delete;

    void Put(const Value& key, size_t version, const Value& value);

    void Delete(const Value& key, size_t version);

    void GetForUpdate(const Value& key, size_t version);

    std::pair<int8_t, Value> Get(const Value& key);
};

class LMDBKvTransaction final : public KvTransaction {
    friend class LMDBKvIterator;
    friend class LMDBKvStore;
    friend class LMDBKvTable;

    MDB_txn* txn_ = nullptr;
    bool read_only_ = true;
    bool optimistic_ = false;

    size_t version_ = 0;

    std::unordered_map<MDB_dbi, DeltaStore> deltas_;
    std::mutex mutex_;
    std::condition_variable cv_;
    int8_t commit_status_ = 0;
    int commit_ec_ = 0;

    LMDBKvStore* store_ = nullptr;
    Wal* wal_ = nullptr;

    DeltaStore& GetDelta(LMDBKvTable& table);

    Wal* GetWal() const { return wal_; }

 public:
    MDB_txn* GetTxn() { return txn_; }
    LMDBKvTransaction() = default;

    DISABLE_COPY(LMDBKvTransaction);

    LMDBKvTransaction(LMDBKvStore& store, bool read_only, bool optimistic);

    LMDBKvTransaction(LMDBKvTransaction&& rhs) noexcept;

    LMDBKvTransaction& operator=(LMDBKvTransaction&& rhs) noexcept;

    ~LMDBKvTransaction() override;

    /**
     * Gets the fork of this transaction. Can only be used in read transactions.
     *
     * \return  A KvTransaction.
     */
    std::unique_ptr<KvTransaction> Fork() override;

    bool IsReadOnly() const { return read_only_; }

    bool IsOptimistic() const override { return optimistic_; }

    /**
     * Commits the transaction. All KvIterators will be invalidated.
     */
    void Commit() override;

    /**
     * Aborts the transaction. All KvIterators will be invalidated.
     */
    void Abort() override;

    bool IsValid() const override { return txn_ != nullptr; }

    size_t TxnId() override { return mdb_txn_id(txn_); }

    int64_t LastOpId() const override { return mdb_txn_last_op_id(txn_); }
};
}  // namespace lgraph
#endif
