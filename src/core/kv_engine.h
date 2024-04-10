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
#pragma once
#include <iostream>
#include <memory>
#include "core/value.h"
#include "core/kv_table_comparators.h"

namespace lgraph {

class KvTransaction {
 public:
    KvTransaction() = default;
    virtual ~KvTransaction() = default;
    DISABLE_COPY(KvTransaction);
    virtual std::unique_ptr<KvTransaction> Fork() = 0;
    virtual bool IsOptimistic() const = 0;
    virtual void Commit() = 0;
    virtual void Abort() = 0;
    virtual bool IsValid() const = 0;
    virtual size_t TxnId()  = 0;
    virtual int64_t LastOpId() const  = 0;
};

class KvTable;
class KvIterator {
 public:
    KvIterator() = default;
    virtual ~KvIterator() = default;
    DISABLE_COPY(KvIterator);
    virtual void Close() = 0;
    virtual std::unique_ptr<KvIterator> Fork() = 0;
    virtual bool UnderlyingPointerModified() = 0;
    virtual bool RefreshAfterModify() = 0;
    virtual bool Next() = 0;
    virtual bool Prev() = 0;
    virtual bool GotoKey(const Value& key) = 0;
    virtual bool GotoClosestKey(const Value& key) = 0;
    virtual bool GotoLastKey() = 0;
    virtual bool GotoFirstKey() = 0;
    virtual Value GetKey() const = 0;
    virtual Value GetValue(bool for_update = false) = 0;
    virtual void SetValue(const Value& value) = 0;
    virtual bool AddKeyValue(const Value& key, const Value& value, bool overwrite = false) = 0;
    virtual bool IsValid() const = 0;
    virtual void DeleteKey() = 0;
    virtual KvTransaction& GetTxn() const = 0;
    virtual KvTable& GetTable() const = 0;
};

class KvTable {
 public:
    KvTable() = default;
    virtual ~KvTable() = default;
    virtual bool HasKey(KvTransaction& txn, const Value& key) = 0;
    virtual Value GetValue(KvTransaction& txn, const Value& key, bool for_update = false) = 0;
    virtual size_t GetKeyCount(KvTransaction& txn) = 0;
    virtual bool SetValue(KvTransaction& txn, const Value& key, const Value& value,
                  bool overwrite_if_exist = true) = 0;
    virtual bool AddKV(KvTransaction& txn, const Value& key, const Value& value) = 0;
    virtual void AppendKv(KvTransaction& txn, const Value& key, const Value& value) = 0;
    virtual bool DeleteKey(KvTransaction& txn, const Value& key) = 0;
    virtual std::unique_ptr<KvIterator> GetIterator(KvTransaction& txn) = 0;
    virtual std::unique_ptr<KvIterator> GetIterator(KvTransaction& txn, const Value& key) = 0;
    virtual std::unique_ptr<KvIterator>
    GetClosestIterator(KvTransaction& txn, const Value& key) = 0;
    virtual int CompareKey(KvTransaction& txn, const Value& k1, const Value& k2) const = 0;
    // empty the table
    virtual void Drop(KvTransaction& txn) = 0;
    // delete the table
    virtual void Delete(KvTransaction& txn) = 0;
    virtual const std::string& Name() const = 0;
};

class KvStore {
 public:
    KvStore() = default;
    virtual ~KvStore() = default;
    DISABLE_COPY(KvStore);
    virtual std::unique_ptr<KvTransaction> CreateReadTxn() = 0;
    virtual std::unique_ptr<KvTransaction> CreateWriteTxn(bool optimistic = false) = 0;
    virtual std::unique_ptr<KvTable> OpenTable(KvTransaction& txn,
                                       const std::string& table_name,
                                       bool create_if_not_exist,
                                       const ComparatorDesc& desc) = 0;
    virtual std::unique_ptr<KvTable> _OpenTable_(KvTransaction& txn,
                                         const std::string& table_name,
                                         bool create_if_not_exist,
                                         const KeySortFunc& func) = 0;
    virtual bool DeleteTable(KvTransaction& txn, const std::string& table_name) = 0;
    virtual std::vector<std::string> ListAllTables(KvTransaction& txn) = 0;
    virtual void Flush() = 0;
    virtual void DropAll(KvTransaction& txn) = 0;
    virtual void DumpStat(KvTransaction& txn, size_t& memory_size, size_t& height) = 0;
    virtual size_t Backup(const std::string& path, bool compact = false) = 0;
    virtual void Snapshot(KvTransaction& txn, const std::string& path, bool compaction = false) = 0;
    virtual void LoadSnapshot(const std::string& snapshot_path) = 0;
    virtual void WarmUp(size_t* size) = 0;
};
}  // namespace lgraph
