/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once
#if (!LGRAPH_USE_MOCK_KV)
#include <vector>
#include <mutex>
#include <condition_variable>

#include "fma-common/logger.h"
#include "fma-common/type_traits.h"
#include "core/kv_store_exception.h"
#include "core/kv_store_table.h"
#include "core/lmdb_profiler.h"
#include "core/lmdb/lmdb.h"


namespace lgraph {
class KvIterator;

class KvTable;

class KvTransaction;

class KvStore;

class Wal;

class DeltaStore {
    friend class KvTable;
    friend class KvIterator;
    friend class KvStore;

    std::map<std::string, std::string, KvTable> write_set_;

 public:
    explicit DeltaStore(const KvTable& table);

    DeltaStore(const DeltaStore& rhs) = delete;

    DeltaStore(DeltaStore&& rhs) = delete;

    void Put(const Value& key, size_t version, const Value& value);

    void Delete(const Value& key, size_t version);

    void GetForUpdate(const Value& key, size_t version);

    std::pair<int8_t, Value> Get(const Value& key);
};

class KvTransaction {
    friend class KvIterator;
    friend class KvStore;
    friend class KvTable;
    friend class KvTransactionValidator;

    MDB_txn* txn_ = nullptr;
    bool read_only_ = true;
    bool optimistic_ = false;

    size_t version_ = 0;

    std::unordered_map<MDB_dbi, DeltaStore> deltas_;
    std::mutex mutex_;
    std::condition_variable cv_;
    int8_t commit_status_ = 0;
    int commit_ec_ = 0;

    KvStore* store_ = nullptr;
    Wal* wal_ = nullptr;

    DeltaStore& GetDelta(KvTable& table);

    Wal* GetWal() const { return wal_; }

 public:
    MDB_txn* GetTxn() { return txn_; }
    KvTransaction() {}

    DISABLE_COPY(KvTransaction);

    KvTransaction(KvStore& store, bool read_only, bool optimistic);

    KvTransaction(KvTransaction&& rhs);

    KvTransaction& operator=(KvTransaction&& rhs);

    ~KvTransaction();

    /**
     * Gets the fork of this transaction. Can only be used in read transactions.
     *
     * \return  A KvTransaction.
     */
    KvTransaction Fork();

    bool IsReadOnly() const { return read_only_; }

    bool IsOptimistic() const { return optimistic_; }

    /**
     * Commits the transaction. All KvIterators will be invalidated.
     */
    void Commit();

    /**
     * Aborts the transaction. All KvIterators will be invalidated.
     */
    void Abort();

    bool IsValid() const { return txn_ != nullptr; }

    size_t TxnId() { return mdb_txn_id(txn_); }

    int64_t LastOpId() const { return mdb_txn_last_op_id(txn_); }
};
}  // namespace lgraph
#endif
