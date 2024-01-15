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
#include <mutex>
#include <unordered_map>
#include "tools/lgraph_log.h"
#include "fma-common/string_formatter.h"
#include "core/lmdb/lmdb.h"

#define MDB_PROFILE 0

#if MDB_PROFILE
class MDBTxnManager {
    std::mutex mu_;
    std::unordered_map<MDB_txn*, bool> txns_;

    std::string DumpAllTxnsNoLock() {
        std::string buf;
        for (auto& t : txns_) {
            fma_common::StringFormatter::Append(buf, "{}:{}:{}, ", (uint64_t)t.first,
                                                mdb_txn_id(t.first), t.second);
        }
        return buf;
    }

 public:
    void NewTxn(MDB_txn* txn, bool read_only) {
        std::lock_guard<std::mutex> l(mu_);
        if (!read_only && !txns_.empty()) {
            FMA_LOG() << "Creating write txn " << mdb_txn_id(txn)
                      << " when there is read: " << DumpAllTxnsNoLock();
        }
        txns_.emplace(txn, read_only);
    }

    void DeleteTxn(MDB_txn* txn) {
        std::lock_guard<std::mutex> l(mu_);
        auto txnid = mdb_txn_id(txn);
        auto it = txns_.find(txn);
        FMA_ASSERT(it != txns_.end());
        bool read_only = it->second;
        txns_.erase(it);
        if (!read_only && !txns_.empty()) {
            FMA_LOG() << "Write txn " << txnid << " exit, remaining txns: " << DumpAllTxnsNoLock();
        }
    }
};

inline MDBTxnManager& GetTxnManager() {
    static MDBTxnManager mgr;
    return mgr;
}

inline int MdbTxnBegin(MDB_env* env, MDB_txn* parent, int flags, MDB_txn** txn) {
    int r = mdb_txn_begin(env, parent, flags, txn);
    bool read_only = (flags | MDB_RDONLY);
    GetTxnManager().NewTxn(*txn, read_only);
    return r;
}

inline void MdbTxnAbort(MDB_txn* txn) {
    GetTxnManager().DeleteTxn(txn);
    mdb_txn_abort(txn);
}

inline int MdbTxnCommit(MDB_txn* txn) {
    GetTxnManager().DeleteTxn(txn);
    return mdb_txn_commit(txn);
}

inline int MdbTxnFork(MDB_txn* txn, MDB_txn** out) {
    int r = mdb_txn_fork(txn, out);
    GetTxnManager().NewTxn(*out, true);
    return r;
}
#else
#define MdbTxnBegin(e, p, f, t) mdb_txn_begin(e, p, f, t)
#define MdbTxnFork(t, o) mdb_txn_fork(t, o)
#define MdbTxnAbort(t) mdb_txn_abort(t)
#define MdbTxnCommit(t) mdb_txn_commit(t)
#endif
