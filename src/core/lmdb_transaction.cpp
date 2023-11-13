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

#if (!LGRAPH_USE_MOCK_KV)
#include <chrono>

#include "core/lmdb_transaction.h"
#include "core/lmdb_table.h"
#include "core/kv_store.h"
#include "core/wal.h"

using namespace std::chrono_literals;

namespace lgraph {

DeltaStore::DeltaStore(const LMDBKvTable& table) : write_set_(table) {}

void DeltaStore::Put(const Value& key, size_t version, const Value& value) {
    std::string packed_value(sizeof(size_t) + sizeof(int8_t) + value.Size(), 0);
    *(size_t*)(packed_value.data()) = version;
    *(int8_t*)(packed_value.data() + sizeof(size_t)) = 1;
    memcpy((char*)packed_value.data() + sizeof(size_t) + sizeof(int8_t), value.Data(),
           value.Size());
    auto it = write_set_.find(key);
    if (it == write_set_.end()) {
        write_set_.emplace(key.AsString(), std::move(packed_value));
    } else {
        it->second = std::move(packed_value);
    }
}

void DeltaStore::Delete(const Value& key, size_t version) {
    auto it = write_set_.find(key);
    if (it != write_set_.end()) {
        size_t version = *(size_t*)(it->second.data());
        int8_t op_type = *(int8_t*)(it->second.data() + sizeof(size_t));
        if (version == 0 && op_type == 1) {
            write_set_.erase(it);
            return;
        }
    }
    std::string packed_value(sizeof(size_t) + sizeof(int8_t), 0);
    *(size_t*)(packed_value.data()) = version;
    *(int8_t*)(packed_value.data() + sizeof(size_t)) = -1;
    if (it != write_set_.end()) {
        it->second = std::move(packed_value);
    } else {
        write_set_.emplace(key.AsString(), std::move(packed_value));
    }
}

void DeltaStore::GetForUpdate(const Value& key, size_t version) {
    auto it = write_set_.find(key);
    if (it != write_set_.end()) return;
    std::string packed_value(sizeof(size_t) + sizeof(int8_t), 0);
    *(size_t*)(packed_value.data()) = version;
    *(int8_t*)(packed_value.data() + sizeof(size_t)) = 0;
    write_set_.emplace(key.AsString(), std::move(packed_value));
}

std::pair<int8_t, Value> DeltaStore::Get(const Value& key) {
    auto it = write_set_.find(key);
    if (it == write_set_.end()) return std::make_pair(0, Value());
    int8_t op_type = *(int8_t*)(it->second.data() + sizeof(size_t));
    if (op_type == 1) {
        return std::make_pair(1, Value(it->second.data() + sizeof(size_t) + sizeof(int8_t),
                                       it->second.size() - sizeof(size_t) - sizeof(int8_t)));
    }
    return std::make_pair(-1, Value());
}

DeltaStore& LMDBKvTransaction::GetDelta(LMDBKvTable& table) {
    MDB_dbi dbi = table.GetDbi();
    auto it = deltas_.find(dbi);
    if (it != deltas_.end()) return it->second;
    return deltas_
        .emplace(std::piecewise_construct, std::forward_as_tuple(table.GetDbi()),
                 std::forward_as_tuple(table))
        .first->second;
}

LMDBKvTransaction::LMDBKvTransaction(LMDBKvStore& store, bool read_only, bool optimistic) {
    store_ = &store;
    wal_ = store_->GetWal();
    read_only_ = read_only;
    optimistic_ = optimistic;
    int flags = MDB_NOSYNC;
    if (read_only_ || optimistic_) flags |= MDB_RDONLY;
    THROW_ON_ERR(MdbTxnBegin(store.env_, nullptr, flags, &txn_));
    version_ = mdb_txn_id(txn_);
    // write wal if this is a write txn and not optimisitc
    // Optimistic txns are actually read-only txns that read from lmdb and write to
    // DeltaStore. The changes are written to lmdb in KvStore::ServerValidation, so
    // we can just record the wal in ServerValidation.
    if (!read_only && !optimistic && wal_) {
        wal_->WriteTxnBegin(version_);
    }
}

LMDBKvTransaction::LMDBKvTransaction(LMDBKvTransaction&& rhs) noexcept {
    txn_ = rhs.txn_;
    read_only_ = rhs.read_only_;
    optimistic_ = rhs.optimistic_;
    version_ = rhs.version_;
    if (!read_only_ && optimistic_) deltas_ = std::move(rhs.deltas_);
    store_ = rhs.store_;
    wal_ = rhs.wal_;
    rhs.txn_ = nullptr;
}

LMDBKvTransaction& LMDBKvTransaction::operator=(LMDBKvTransaction&& rhs) noexcept {
    if (this == &rhs) return *this;
    LMDBKvTransaction::Abort();
    txn_ = rhs.txn_;
    store_ = rhs.store_;
    wal_ = rhs.wal_;
    read_only_ = rhs.read_only_;
    optimistic_ = rhs.optimistic_;
    version_ = rhs.version_;
    rhs.txn_ = nullptr;
    return *this;
}

LMDBKvTransaction::~LMDBKvTransaction() { LMDBKvTransaction::Abort(); }

std::unique_ptr<KvTransaction> LMDBKvTransaction::Fork() {
    if (!read_only_) throw KvException("Write transactions cannot be forked.");
    auto txn = std::make_unique<LMDBKvTransaction>();
    txn->store_ = store_;
    txn->wal_ = wal_;
    txn->read_only_ = true;
    txn->optimistic_ = false;
    THROW_ON_ERR(MdbTxnFork(txn_, &txn->txn_));
    return txn;
}

void LMDBKvTransaction::Commit() {
    if (txn_) {
        if (read_only_ || !optimistic_) {
            if (!read_only_) {
                mdb_txn_set_last_op_id(txn_, LMDBKvStore::GetLastOpIdOfAllStores());
                if (wal_) {
                    std::future<void> future;
                    if (wal_) future = wal_->WriteTxnCommit(version_, false);
                    int r = MdbTxnCommit(txn_);
                    if (wal_) wal_->WaitForWalFlush(future);
                    if (r != MDB_SUCCESS)
                        THROW_ERR(r);
                } else {
                    THROW_ON_ERR(MdbTxnCommit(txn_));
                }
            } else {
                mdb_txn_abort(txn_);
            }
            txn_ = nullptr;
        } else {
            commit_status_ = 0;
            commit_ec_ = 0;
            std::unique_lock<std::mutex> queue_lock(store_->queue_mutex_);
            store_->queue_.push(this);
            store_->queue_cv_.notify_one();
            queue_lock.unlock();
            std::unique_lock<std::mutex> my_lock(mutex_);
            while (!cv_.wait_for(my_lock, 100ms, [&]() { return commit_status_ != 0; })) {
            }
            if (commit_status_ != 1) {
                if (commit_ec_ == MDB_CONFLICTS)
                    throw lgraph_api::TxnConflictError();
                else
                    THROW_ERR(commit_ec_);
            }
            MdbTxnAbort(txn_);
            txn_ = nullptr;
        }
    }
}

void LMDBKvTransaction::Abort() {
    deltas_.clear();
    if (txn_) {
        if (!read_only_ && !optimistic_ && wal_) wal_->WriteTxnAbort(version_);
        MdbTxnAbort(txn_);
        txn_ = nullptr;
    }
}
}  // namespace lgraph
#endif
