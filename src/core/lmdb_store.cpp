/**
 * Copyright 2024 AntGroup CO., Ltd.
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
#include <filesystem>
#include "core/lmdb_store.h"
#include "core/wal.h"

using namespace std::chrono_literals;

namespace lgraph {

static const std::string DATA_FILE_NAME = "data.mdb";  // NOLINT
std::atomic<int64_t> LMDBKvStore::last_op_id_(-1);

static auto IsDir = [](const std::string& p) {
    std::error_code ec;
    return std::filesystem::is_directory(p, ec);
};
static auto MkDir = [](const std::string& p) {
    std::error_code ec;
    return std::filesystem::create_directories(p, ec);
};
static auto FileExists = [](const std::string& p) {
    std::error_code ec;
    return std::filesystem::is_regular_file(p, ec);
};
static auto RemoveDir = [](const std::string& p) {
    std::error_code ec;
    return std::filesystem::remove_all(p, ec);
};

void LMDBKvStore::Open(bool create_if_not_exist) {
    if (create_if_not_exist) {
        std::error_code ec;
        if (!IsDir(path_) && !MkDir(path_)) {
            THROW_CODE(KvException, std::string("Failed to create data directory ") + path_);
        }
    } else {
        if (!IsDir(path_) || !FileExists(path_ + "/" + DATA_FILE_NAME)) {
            THROW_CODE(KvException, "Data directory " + path_ + " does not contain valid data.");
        }
    }
    THROW_ON_ERR(mdb_env_create(&env_));
    THROW_ON_ERR(mdb_env_set_mapsize(env_, db_size_));
    THROW_ON_ERR(mdb_env_set_maxdbs(env_, 5000));  // HENG: former value is 255
    THROW_ON_ERR(mdb_env_set_maxreaders(env_, 1200));
#if LGRAPH_SHARE_DIR
    unsigned int flags = MDB_NOMEMINIT | MDB_NORDAHEAD | MDB_NOTLS | MDB_NOSYNC;
#else
    unsigned int flags = MDB_NOMEMINIT | MDB_NORDAHEAD | MDB_NOSYNC;
#endif
    THROW_ON_ERR(mdb_env_open(env_, path_.c_str(), flags, 0664));
    // update last op id of all stores with the value stored in this one
    MDB_txn* txn;
    THROW_ON_ERR(mdb_txn_begin(env_, nullptr, MDB_RDONLY, &txn));
    int64_t last_op_id = mdb_txn_last_op_id(txn);
    mdb_txn_abort(txn);
    LMDBKvStore::UpdateLastOpIdWithStoredValue(last_op_id);
    // start wal
    wal_.reset();
    if (durable_) {
        wal_.reset(new Wal(env_, path_,
                           wal_log_rotate_interval_ms_, wal_batch_commit_interval_ms_));
    }
}

void LMDBKvStore::ReopenFromSnapshot(const std::string& snapshot_path) {
    wal_.reset();
    mdb_env_close(env_);
    // copy the snapshot file
    std::string src = snapshot_path + "/" + DATA_FILE_NAME;
    std::string dst = path_ + "/" + DATA_FILE_NAME;
    RemoveDir(dst);
    std::error_code ec;
    if (!std::filesystem::copy_file(src, dst, ec)) {
        THROW_CODE(KvException, "Failed to copy snapshot file from " + src + " to " + dst);
    }
    Open(false);
}

LMDBKvStore::LMDBKvStore(const std::string& path, size_t db_size, bool durable,
                 bool create_if_not_exist,
                 size_t wal_log_rotate_interval_ms,
                 size_t wal_batch_commit_interval_ms)
    : path_(path),
    db_size_(db_size),
    durable_(durable),
    wal_log_rotate_interval_ms_(wal_log_rotate_interval_ms),
    wal_batch_commit_interval_ms_(wal_batch_commit_interval_ms) {
    Open(create_if_not_exist);
    finished_ = false;
    validator_ = std::thread([this]() { this->ServeValidation(); });
}

LMDBKvStore::~LMDBKvStore() {
    if (env_) {
        wal_.reset();
        mdb_env_close(env_);
    }
    finished_ = true;
    queue_cv_.notify_one();
    validator_.join();
}

std::unique_ptr<KvTransaction> LMDBKvStore::CreateReadTxn() {
    return std::make_unique<LMDBKvTransaction>(*this, true, false);
}

std::unique_ptr<KvTransaction> LMDBKvStore::CreateWriteTxn(bool optimistic) {
    return std::make_unique<LMDBKvTransaction>(*this, false, optimistic);
}

std::unique_ptr<KvTable> LMDBKvStore::OpenTable(KvTransaction& txn, const std::string& table_name,
                           bool create_if_not_exist, const ComparatorDesc& desc) {
    std::lock_guard<std::mutex> l(mutex_);
    auto& lmdb_txn = static_cast<LMDBKvTransaction&>(txn);
    return std::make_unique<LMDBKvTable>(lmdb_txn, table_name, create_if_not_exist, desc);
}

std::unique_ptr<KvTable> LMDBKvStore::_OpenTable_(KvTransaction& txn, const std::string& table_name,
                           bool create_if_not_exist, const KeySortFunc& func) {
    std::lock_guard<std::mutex> l(mutex_);
    auto& lmdb_txn = static_cast<LMDBKvTransaction&>(txn);
    auto t = std::make_unique<LMDBKvTable>(lmdb_txn, table_name, create_if_not_exist,
        ComparatorDesc::DefaultComparator());
    if (func) mdb_set_compare(lmdb_txn.GetTxn(), t->dbi_, func);
    return t;
}

bool LMDBKvStore::DeleteTable(KvTransaction& txn, const std::string& table_name) {
    std::lock_guard<std::mutex> l(mutex_);
    auto& lmdb_txn = static_cast<LMDBKvTransaction&>(txn);
    auto tbl = LMDBKvTable(lmdb_txn, table_name, true, ComparatorDesc::DefaultComparator());
    tbl.Delete(lmdb_txn);
    return true;
}

std::vector<std::string> LMDBKvStore::ListAllTables(KvTransaction& txn) {
    std::vector<std::string> tables;
    auto& lmdb_txn = static_cast<LMDBKvTransaction&>(txn);
    auto unamed_table = std::make_unique<LMDBKvTable>(
        lmdb_txn, "", false, ComparatorDesc::DefaultComparator());
    auto it = unamed_table->GetIterator(lmdb_txn);
    it->GotoFirstKey();
    while (it->IsValid()) {
        Value name = it->GetKey();
        tables.emplace_back(name.AsString());
        it->Next();
    }
    return tables;
}

void LMDBKvStore::Flush() { THROW_ON_ERR(mdb_env_sync(env_, true)); }

void LMDBKvStore::DropAll(KvTransaction& txn) {
    auto all_tables = ListAllTables(txn);
    for (auto& tbl : all_tables) {
        DeleteTable(txn, tbl);
    }
}

void LMDBKvStore::DumpStat(KvTransaction& txn, size_t& memory_size, size_t& height) {
    memory_size = 0;
    height = 0;
    MDB_stat stat;
    stat.ms_psize = 4096;
    auto tables = ListAllTables(txn);
    auto& lmdb_txn = static_cast<LMDBKvTransaction&>(txn);
    for (const auto& tbl : tables) {
        auto t = std::make_unique<LMDBKvTable>(
            lmdb_txn, tbl, false, ComparatorDesc::DefaultComparator());
        THROW_ON_ERR(mdb_stat(lmdb_txn.GetTxn(), t->GetDbi(), &stat));
        // LOG() << "Table (" << tbl << "):"
        //    << "\n\tbranch_pages: " << stat.ms_branch_pages
        //    << "\n\tleaf_pages: " << stat.ms_leaf_pages
        //    << "\n\toverflow_pages: " << stat.ms_overflow_pages
        //    << "\n\tdepth: " << stat.ms_depth;
        memory_size += stat.ms_branch_pages + stat.ms_leaf_pages + stat.ms_overflow_pages;
        height = std::max<size_t>(height, stat.ms_depth);
    }
    memory_size *= stat.ms_psize;
}

size_t LMDBKvStore::Backup(const std::string& path, bool compact) {
    size_t last_txn_id = 0;
    THROW_ON_ERR(mdb_env_copy2(env_, path.c_str(), compact ? MDB_CP_COMPACT : 0, &last_txn_id));
    return last_txn_id;
}

void LMDBKvStore::Snapshot(KvTransaction& txn, const std::string& path, bool compaction) {
    int flags = 0;
    if (compaction) flags |= MDB_CP_COMPACT;
    auto& lmdb_txn = static_cast<LMDBKvTransaction&>(txn);
    THROW_ON_ERR(mdb_env_copy_txn(env_, path.c_str(), lmdb_txn.GetTxn(), flags));
}

void LMDBKvStore::LoadSnapshot(const std::string& snapshot_path) {
    ReopenFromSnapshot(snapshot_path);
}

void LMDBKvStore::WarmUp(size_t* size) {
    auto txn = CreateReadTxn();
    auto tables = ListAllTables(*txn);
    size_t s = 0;
    auto& lmdb_txn = static_cast<LMDBKvTransaction&>(*txn);
    for (const auto& tbl : tables) {
        auto t = std::make_unique<LMDBKvTable>(
            lmdb_txn, tbl, false, ComparatorDesc::DefaultComparator());
        auto it = t->GetIterator(lmdb_txn);
        for (it->GotoFirstKey(); it->IsValid(); it->Next()) {
            s += it->GetKey().Size();
            s += it->GetValue().Size();
        }
    }
    if (size) *size = s;
}

void LMDBKvStore::ServeValidation() {
    #ifndef _WIN32
    pthread_setname_np(pthread_self(), "ServeValidation");
    #endif
    while (true) {
        std::unique_lock<std::mutex> queue_lock(queue_mutex_);
        while (!queue_cv_.wait_for(queue_lock, 100ms,
                                   [this]() { return !queue_.empty() || finished_; })) {
        }
        if (queue_.empty() && finished_) break;
        std::vector<LMDBKvTransaction*> txns;
        while (!queue_.empty()) {
            txns.emplace_back(queue_.front());
            queue_.pop();
        }
        queue_lock.unlock();
        MDB_txn* root_txn;
        int txn_begin_flags = 0;
        if (!durable_) txn_begin_flags = MDB_NOSYNC;
        int ec;
        ec = mdb_txn_begin(env_, nullptr, txn_begin_flags, &root_txn);
        if (ec != MDB_SUCCESS) {
            for (size_t i = 0; i < txns.size(); i++) {
                txns[i]->commit_status_ = -1;
                txns[i]->cv_.notify_one();
            }
            continue;
        }
        size_t write_version = mdb_txn_id(root_txn);
        if (wal_.get())
            wal_->WriteTxnBegin(write_version);
        std::vector<int> results;
        for (auto txn : txns) {
            MDB_txn* child_txn;
            ec = mdb_txn_begin(env_, root_txn, txn_begin_flags, &child_txn);
            if (ec != MDB_SUCCESS) {
                results.emplace_back(ec);
                continue;
            }
            if (wal_.get())
                wal_->WriteTxnBegin(write_version, true);
            bool successful = true;
            for (auto it = txn->deltas_.begin(); it != txn->deltas_.end(); it++) {
                const auto& dbi = it->first;
                const auto& delta = it->second;
                MDB_cursor* cursor;
                ec = mdb_cursor_open(child_txn, dbi, &cursor);
                if (ec != MDB_SUCCESS) {
                    successful = false;
                    break;
                }
                for (auto it = delta.write_set_.begin(); it != delta.write_set_.end(); it++) {
                    const auto& key = it->first;
                    const auto& packed_value = it->second;
                    size_t read_version = *(size_t*)(packed_value.data());
                    int8_t op_type = *(int8_t*)(packed_value.data() + sizeof(size_t));
                    MDB_val mdb_key = {key.size(), (char*)key.data()};
                    MDB_val mdb_value = {0, nullptr};
                    ec = mdb_cursor_get(cursor, &mdb_key, &mdb_value, MDB_SET_KEY);
                    size_t latest_version = 0;
                    if (ec == MDB_SUCCESS) {
                        latest_version = *(size_t*)(mdb_value.mv_data);
                    } else if (ec != MDB_NOTFOUND) {
                        successful = false;
                        break;
                    }
                    if (latest_version != read_version) {
                        // FMA_LOG() << key << " expects " << read_version << " but got " <<
                        // latest_version << " op_type: " << (op_type==1 ? "Put" : ((op_type == 0) ?
                        // "GetForUpdate" : "Delete"));
                        successful = false;
                        ec = MDB_CONFLICTS;
                        break;
                    }
                    if (op_type == 1) {
                        mdb_key = {key.size(), (char*)key.data()};
                        Value tmp(packed_value.size() - sizeof(int8_t));
                        *(size_t*)(tmp.Data()) = write_version;
                        memcpy(tmp.Data() + sizeof(size_t),
                               packed_value.data() + sizeof(size_t) + sizeof(int8_t),
                               packed_value.size() - sizeof(size_t) - sizeof(int8_t));
                        MDB_val mdb_value = tmp.MakeMdbVal();
                        int flags = 0;
                        ec = mdb_cursor_put(cursor, &mdb_key, &mdb_value, flags);
                        if (ec != MDB_SUCCESS) {
                            successful = false;
                            break;
                        }
                        if (wal_.get())
                            wal_->WriteKvPut(dbi,
                                             Value((const char*)mdb_key.mv_data, mdb_key.mv_size),
                                             tmp);
                    } else if (op_type == -1) {
                        ec = mdb_cursor_del(cursor, 0);
                        if (ec != MDB_SUCCESS) {
                            successful = false;
                            break;
                        }
                        if (wal_.get())
                            wal_->WriteKvDel(dbi, Value::ConstRef(key));
                    }
                }
                if (!successful) break;
            }
            if (successful) {
                mdb_txn_set_last_op_id(child_txn, GetLastOpIdOfAllStores());
                ec = mdb_txn_commit(child_txn);
                if (wal_.get() && ec == MDB_SUCCESS)
                    wal_->WriteTxnCommit(write_version, true);
                else if (wal_.get() && ec != MDB_SUCCESS)
                    wal_->WriteTxnAbort(write_version, true);
                results.emplace_back(ec);
            } else {
                mdb_txn_abort(child_txn);
                if (wal_.get())
                    wal_->WriteTxnAbort(write_version, true);
                results.emplace_back(ec);
            }
        }
        // set last op id
        mdb_txn_set_last_op_id(root_txn, GetLastOpIdOfAllStores());
        std::future<void> future;
        if (wal_)
            future = wal_->WriteTxnCommit(write_version, false);
        ec = mdb_txn_commit(root_txn);
        // mdb_txn locks the thread, we need to release the txn before waiting
        if (wal_) wal_->WaitForWalFlush(future);
        if (ec == MDB_SUCCESS) {
            for (size_t i = 0; i < txns.size(); i++) {
                txns[i]->commit_status_ = (results[i] == MDB_SUCCESS) ? 1 : -1;
                txns[i]->commit_ec_ = results[i];
                txns[i]->cv_.notify_one();
            }
        } else {
            for (size_t i = 0; i < txns.size(); i++) {
                txns[i]->commit_status_ = -1;
                txns[i]->commit_ec_ = (results[i] == MDB_SUCCESS) ? ec : results[i];
                txns[i]->cv_.notify_one();
            }
        }
    }
}

}  // namespace lgraph
#endif
