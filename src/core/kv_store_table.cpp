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
#include <cstring>

#include "fma-common/string_formatter.h"

#include "core/kv_store_iterator.h"
#include "core/kv_store_table.h"
#include "core/kv_store_transaction.h"
#include "core/wal.h"

namespace lgraph {
size_t KvTable::GetVersion(KvTransaction& txn, const Value& key) {
    MDB_val k = key.MakeMdbVal();
    MDB_val v;
    int ec = mdb_get(txn.GetTxn(), dbi_, &k, &v);
    return ec == 0 ? *(size_t*)(v.mv_data) : 0;
}

static int DefaultCompareKey(const MDB_val* a, const MDB_val* b) {
    bool b_longer = false;
    size_t s = a->mv_size;
    if (s > b->mv_size) {
        b_longer = true;
        s = b->mv_size;
    }
    int r = memcmp(a->mv_data, b->mv_data, s);
    if (r) return r;
    return b_longer ? -1 : (a->mv_size > b->mv_size);
}

KvTable::KvTable(KvTransaction& txn,
    const std::string& name,
    bool create_if_not_exist,
    const ComparatorDesc& desc) {
    name_ = name;
    int flags = 0;
    if (create_if_not_exist) flags |= MDB_CREATE;
    THROW_ON_ERR(mdb_dbi_open(txn.GetTxn(), name.empty() ? nullptr : name.c_str(), flags, &dbi_));
    // may need to set customized comparator
    auto comp = GetKeyComparator(desc);
    if (comp) {
        THROW_ON_ERR(mdb_set_compare(txn.GetTxn(), dbi_, comp));
        compare_key_ = comp;
    } else {
        compare_key_ = DefaultCompareKey;
    }
    // write wal
    if (!txn.IsReadOnly() && txn.GetWal()) {
        txn.GetWal()->WriteTableOpen(dbi_, name, desc);
    }
}

bool KvTable::HasKey(KvTransaction& txn, const Value& key) {
    ThrowIfTaskKilled();
    if (!txn.read_only_ && txn.optimistic_) {
        // treat empty key as we would in lmdb
        if (key.Empty()) throw KvException(MDB_INVALID);
        DeltaStore& delta = txn.GetDelta(*this);
        auto status_value = delta.Get(key);
        if (status_value.first != 0) {
            return (status_value.first == 1);
        }
    }
    MDB_val val;
    MDB_val k = key.MakeMdbVal();
    int ec = mdb_get(txn.GetTxn(), dbi_, &k, &val);
    if (ec == MDB_SUCCESS) return true;
    if (ec == MDB_NOTFOUND) return false;
    THROW_ERR(ec);
    return false;
}

Value KvTable::GetValue(KvTransaction& txn, const Value& key, bool for_update) {
    ThrowIfTaskKilled();
    if (!txn.read_only_ && txn.optimistic_) {
        DeltaStore& delta = txn.GetDelta(*this);
        auto status_value = delta.Get(key);
        if (status_value.first != 0) {
            return status_value.second;
        }
    }
    MDB_val val;
    MDB_val k = key.MakeMdbVal();
    int ec = mdb_get(txn.GetTxn(), dbi_, &k, &val);
    if (ec == MDB_SUCCESS) {
        if (for_update) {
            DeltaStore& delta = txn.GetDelta(*this);
            size_t version = *(size_t*)(val.mv_data);
            delta.GetForUpdate(key, version);
        }
        return Value((char*)val.mv_data + sizeof(size_t), val.mv_size - sizeof(size_t));
    }
    if (ec == MDB_NOTFOUND) return Value();
    THROW_ERR(ec);
    return Value();
}

size_t KvTable::GetKeyCount(KvTransaction& txn) {
    MDB_stat stat;
    THROW_ON_ERR(mdb_stat(txn.GetTxn(), dbi_, &stat));
    if (txn.read_only_ || !txn.optimistic_) return stat.ms_entries;
    DeltaStore& delta = txn.GetDelta(*this);
    size_t count = stat.ms_entries;
    for (auto it = delta.write_set_.begin(); it != delta.write_set_.end(); it++) {
        const auto& packed_value = it->second;
        int8_t op_type = *(int8_t*)(packed_value.data() + sizeof(size_t));
        count += op_type;
    }
    return count;
}

bool KvTable::SetValue(KvTransaction& txn, const Value& key, const Value& value,
                       bool overwrite_if_exist) {
    ThrowIfTaskKilled();
    if (!txn.read_only_ && txn.optimistic_) {
        if (!overwrite_if_exist && HasKey(txn, key)) return false;
        DeltaStore& delta = txn.GetDelta(*this);
        size_t version = GetVersion(txn, key);
        delta.Put(key, version, value);
        return true;
    }
    MDB_val k = key.MakeMdbVal();
    Value tmp(sizeof(size_t) + value.Size());
    *(size_t*)(tmp.Data()) = txn.version_;
    memcpy(tmp.Data() + sizeof(size_t), value.Data(), value.Size());
    MDB_val v = tmp.MakeMdbVal();
    int flags = 0;
    if (!overwrite_if_exist) flags |= MDB_NOOVERWRITE;
    int ec = mdb_put(txn.GetTxn(), dbi_, &k, &v, flags);
    if (ec == MDB_SUCCESS) {
        // write wal
        if (txn.GetWal())
            txn.GetWal()->WriteKvPut(dbi_, key, tmp);
        return true;
    } else if (ec == MDB_KEYEXIST) {
        return false;
    }
    THROW_ERR(ec);
}

bool KvTable::AddKV(KvTransaction& txn, const Value& key, const Value& value) {
    ThrowIfTaskKilled();
    return SetValue(txn, key, value, false);
}

void KvTable::AppendKv(KvTransaction& txn, const Value& key, const Value& value) {
    ThrowIfTaskKilled();
    if (!txn.read_only_ && txn.optimistic_) {
        AddKV(txn, key, value);
        return;
    }
    MDB_val k = key.MakeMdbVal();
    Value tmp(sizeof(size_t) + value.Size());
    *(size_t*)(tmp.Data()) = txn.version_;
    memcpy(tmp.Data() + sizeof(size_t), value.Data(), value.Size());
    MDB_val v = tmp.MakeMdbVal();
    THROW_ON_ERR_WITH_KV(mdb_put(txn.GetTxn(), dbi_, &k, &v, MDB_APPEND), k, v);
    // write wal
    if (txn.GetWal())
        txn.GetWal()->WriteKvPut(dbi_, key, tmp);
}

bool KvTable::DeleteKey(KvTransaction& txn, const Value& key) {
    ThrowIfTaskKilled();
    if (!txn.read_only_ && txn.optimistic_) {
        if (key.Empty()) throw KvException(MDB_INVALID);
        DeltaStore& delta = txn.GetDelta(*this);
        auto it = delta.write_set_.find(key);
        if (it != delta.write_set_.end()) {
            size_t version = *(size_t*)(it->second.data());
            delta.Delete(key, version);
            return true;
        }
        size_t version = GetVersion(txn, key);
        if (version) {
            delta.Delete(key, version);
            return true;
        }
        return false;
    }
    MDB_val k = key.MakeMdbVal();
    int ec = mdb_del(txn.GetTxn(), dbi_, &k, nullptr);
    if (ec == MDB_SUCCESS) {
        // write wal
        if (txn.GetWal())
            txn.GetWal()->WriteKvDel(dbi_, key);
        return true;
    }
    if (ec == MDB_NOTFOUND) return false;
    THROW_ERR(ec);
    return false;
}

void KvTable::Dump(KvTransaction& txn,
                   const std::function<void(const Value& key, void* context)>& begin_key,
                   const std::function<void(const Value& value, void* context)>& dump_value,
                   const std::function<void(const Value& key, void* context)>& finish_key,
                   void* context) {
    KvIterator it = GetIterator(txn);
    while (it.IsValid()) {
        Value key = it.GetKey();
        begin_key(key, context);
        dump_value(it.GetValue(), context);
        finish_key(key, context);
        it.Next();
    }
}

void KvTable::Print(KvTransaction& txn,
                    const std::function<std::string(const Value& key)>& print_key,
                    const std::function<std::string(const Value& key)>& print_value) {
    std::string line;
    Dump(
        txn,
        [print_key](const Value& k, void* l) {
            std::string& line = *(std::string*)l;
            line.clear();
            fma_common::StringFormatter::Append(line, "[{}] : ", print_key(k));
        },
        [print_value](const Value& v, void* l) {
            std::string& line = *(std::string*)l;
            fma_common::StringFormatter::Append(line, " [{}]", print_value(v));
        },
        [](const Value& k, void* l) {
            std::string& line = *(std::string*)l;
            std::cout << line;
        },
        &line);
}

KvIterator KvTable::GetIterator(KvTransaction& txn) {
    return KvIterator(txn, *this, Value(), false);
}

KvIterator KvTable::GetIterator(KvTransaction& txn, const Value& key) {
    return KvIterator(txn, *this, key, false);
}

KvIterator KvTable::GetClosestIterator(KvTransaction& txn, const Value& key) {
    return KvIterator(txn, *this, key, true);
}

int KvTable::CompareKey(KvTransaction& txn, const Value& k1, const Value& k2) const {
    MDB_val a = k1.MakeMdbVal();
    MDB_val b = k2.MakeMdbVal();
    return mdb_cmp(txn.GetTxn(), dbi_, &a, &b);
}

void KvTable::Drop(KvTransaction& txn) {
    THROW_ON_ERR(mdb_drop(txn.GetTxn(), dbi_, 0));
    // write wal
    if (txn.GetWal())
        txn.GetWal()->WriteTableDrop(dbi_);
}

}  // namespace lgraph
#endif
