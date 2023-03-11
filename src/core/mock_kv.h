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

#include <map>
#include <string>

#include "fma-common/binary_read_write_helper.h"
#include "fma-common/fma_stream.h"
#include "fma-common/type_traits.h"

#include "core/data_type.h"
#include "core/kv_store_exception.h"
#include "core/value.h"

namespace lgraph {
typedef int (*KeySortFunc)(const MDB_val*, const MDB_val*);

class MockKvIterator;

class MockKvTransaction {
    friend class MockKvIterator;
    friend class MockKvStore;
    friend class MockKvTable;

    bool valid_ = true;
    DISABLE_COPY(MockKvTransaction);

 public:
    MockKvTransaction() {}

    MockKvTransaction(MockKvTransaction&& rhs) {
        valid_ = rhs.valid_;
        rhs.valid_ = false;
    }

    MockKvTransaction& operator=(MockKvTransaction&& rhs) {
        valid_ = rhs.valid_;
        rhs.valid_ = false;
        return *this;
    }

    MockKvTransaction Fork() { return MockKvTransaction(); }

    bool IsReadOnly() const { return false; }

    void Commit() { valid_ = false; }

    void Abort() { valid_ = false; }

    bool IsValid() const { return valid_; }

    size_t TxnId() { return 0; }

    int64_t LastOpId() { return 0; }
};

class MockKvTable {
    friend class MockKvIterator;

    std::string name_;
    std::shared_ptr<std::map<std::string, std::string,
                             std::function<bool(const std::string&, const std::string&)>>>
        map_;
    KeySortFunc sort_func_;

    static int CompareAsInt(const MDB_val* a, const MDB_val* b) {
        if (a->mv_size == 4) {
            int ai = (int)_detail::GetNByteIdFromBuf<4>((const char*)(a->mv_data));
            int bi = (int)_detail::GetNByteIdFromBuf<4>((const char*)(b->mv_data));
            return ai < bi ? -1 : (ai > bi ? 1 : 0);
        } else {
            int64_t ai = _detail::GetNByteIdFromBuf<8>((const char*)(a->mv_data));
            int64_t bi = _detail::GetNByteIdFromBuf<8>((const char*)(b->mv_data));
            return ai < bi ? -1 : (ai > bi ? 1 : 0);
        }
    }

 public:
    MockKvTable() {
        map_.reset(new std::map<std::string, std::string,
                                std::function<bool(const std::string&, const std::string&)>>(
            std::less<std::string>()));
    }

    MockKvTable(MockKvTransaction& txn, const std::string& name, bool create_if_not_exist,
                bool key_is_int = false, KeySortFunc key_sort = nullptr) {
        name_ = name;
        if (key_is_int) {
            sort_func_ = CompareAsInt;
            map_ = std::make_shared<
                std::map<std::string, std::string,
                         std::function<bool(const std::string&, const std::string&)>>>(
                [](const std::string& a, const std::string& b) -> bool {
                    MDB_val ak, bk;
                    ak.mv_data = (void*)a.data();
                    ak.mv_size = a.size();
                    bk.mv_data = (void*)b.data();
                    bk.mv_size = b.size();
                    return MockKvTable::CompareAsInt(&ak, &bk) < 0;
                });
        } else if (key_sort) {
            sort_func_ = key_sort;
            map_ = std::make_shared<
                std::map<std::string, std::string,
                         std::function<bool(const std::string&, const std::string&)>>>(
                [key_sort](const std::string& a, const std::string& b) -> bool {
                    MDB_val ak, bk;
                    ak.mv_data = (void*)a.data();
                    ak.mv_size = a.size();
                    bk.mv_data = (void*)b.data();
                    bk.mv_size = b.size();
                    return key_sort(&ak, &bk) < 0;
                });
        } else {
            sort_func_ = nullptr;
            map_ = std::make_shared<
                std::map<std::string, std::string,
                         std::function<bool(const std::string&, const std::string&)>>>(
                std::less<std::string>());
        }
    }

    bool HasKey(MockKvTransaction& txn, const Value& key) {
        return map_->find(key.AsString()) != map_->end();
    }

    Value GetValue(MockKvTransaction& txn, const Value& key) {
        auto it = map_->find(key.AsString());
        if (it == map_->end()) return Value();
        return Value(it->second);
    }

    size_t GetKeyCount(MockKvTransaction& txn) { return map_->size(); }

    bool SetValue(MockKvTransaction& txn, const Value& key, const Value& value,
                  bool overwrite_if_exist = true) {
        auto it = map_->find(key.AsString());
        if (it != map_->end()) {
            if (!overwrite_if_exist) return false;
            it->second = value.AsString();
            return true;
        } else {
            map_->emplace_hint(it, key.AsString(), value.AsString());
            return true;
        }
    }

    bool AddKV(MockKvTransaction& txn, const Value& key, const Value& value) {
        return SetValue(txn, key, value, false);
    }

    void AppendKv(MockKvTransaction& txn, const Value& key, const Value& value) {
        auto it = map_->emplace_hint(map_->end(), key.AsString(), value.AsString());
        if (++it != map_->end()) {
            FMA_LOG() << "map: " << fma_common::ToString(*map_);
            FMA_LOG() << "it: " << fma_common::ToString(*it);
            FMA_ASSERT(false);
        }
    }

    bool DeleteKey(MockKvTransaction& txn, const Value& key) {
        auto it = map_->find(key.AsString());
        if (it == map_->end()) return false;
        map_->erase(it);
        return true;
    }

    MockKvIterator GetIterator(MockKvTransaction& txn);

    MockKvIterator GetIterator(MockKvTransaction& txn, const Value& key);

    MockKvIterator GetClosestIterator(MockKvTransaction& txn, const Value& key);

    int CompareKey(MockKvTransaction& txn, const Value& k1, const Value& k2) const {
        if (sort_func_) {
            MDB_val kk1, kk2;
            kk1.mv_data = k1.Data();
            kk2.mv_data = k2.Data();
            kk1.mv_size = k1.Size();
            kk2.mv_size = k2.Size();
            return sort_func_(&kk1, &kk2);
        } else {
            const std::string& s1 = k1.AsString();
            const std::string& s2 = k2.AsString();
            return s1 == s2 ? 0 : s1 < s2 ? -1 : 1;
        }
    }

    void Dump(MockKvTransaction& txn,
              const std::function<void(const Value& key, void* context)>& begin_key,
              const std::function<void(const Value& value, void* context)>& dump_value,
              const std::function<void(const Value& key, void* context)>& finish_key,
              void* context) {}

    void Print(MockKvTransaction& txn,
               const std::function<std::string(const Value& key)>& print_key,
               const std::function<std::string(const Value& key)>& print_value) {}

    void Drop(MockKvTransaction& txn) { map_->clear(); }

    /**
     * Gets the name of the table.
     *
     * \return  A reference to a std::string.
     */
    const std::string& Name() const { return name_; }

    template <typename StreamT>
    size_t Serialize(StreamT& stream) const {
        return fma_common::BinaryWrite(stream, *map_);
    }

    template <typename StreamT>
    size_t Deserialize(StreamT& stream) {
        return fma_common::BinaryRead(stream, *map_);
    }
};

class MockKvIterator {
    std::map<std::string, std::string>::iterator it_;
    MockKvTable* table_;

    MockKvIterator() : table_(nullptr) {}

 public:
    void Close() {}

    bool RefreshAfterModify() { return true; }
    bool UnderlyingPointerModified() { return false; }

    MockKvIterator(const MockKvIterator& rhs) = default;
    MockKvIterator& operator=(const MockKvIterator& rhs) = default;

    MockKvIterator(MockKvIterator&& rhs) : it_(std::move(rhs.it_)), table_(rhs.table_) {
        rhs.table_ = nullptr;
    }

    MockKvIterator& operator=(MockKvIterator&& rhs) {
        if (this == &rhs) return *this;
        table_ = rhs.table_;
        it_ = std::move(rhs.it_);
        return *this;
    }

    MockKvIterator(MockKvTransaction& txn, MockKvTable& table, const Value& key, bool closest) {
        table_ = &table;
        if (key.Empty()) {
            it_ = table_->map_->begin();
        } else {
            if (closest) {
                it_ = table_->map_->lower_bound(key.AsString());
            } else {
                it_ = table_->map_->find(key.AsString());
            }
        }
    }

    MockKvIterator(MockKvTransaction& txn, MockKvTable& table) {
        table_ = &table;
        it_ = table.map_->end();
    }

    bool Next() {
        it_++;
        return IsValid();
    }

    bool Prev() {
        if (table_->map_->empty()) return false;
        it_--;
        return IsValid();
    }

    bool GotoKey(const Value& key) {
        it_ = table_->map_->find(key.AsString());
        return IsValid();
    }

    bool GotoClosestKey(const Value& key) {
        it_ = table_->map_->lower_bound(key.AsString());
        return IsValid();
    }

    bool GotoLastKey() {
        it_ = (--table_->map_->end());
        return IsValid();
    }

    bool GotoFirstKey() {
        it_ = table_->map_->begin();
        return IsValid();
    }

    Value GetKey() const { return Value::ConstRef(it_->first); }

    Value GetValue() const { return Value::ConstRef(it_->second); }

    void SetValue(const Value& value) { it_->second = value.AsString(); }

    bool AddKeyValue(const Value& key, const Value& value, bool overwrite = false) {
        it_ = table_->map_->find(key.AsString());
        if (it_ == table_->map_->end()) {
            it_ = table_->map_->emplace_hint(it_, key.AsString(), value.AsString());
            return true;
        } else {
            if (!overwrite) return false;
            it_->second = value.AsString();
            return true;
        }
    }

    bool IsValid() const { return table_ && it_ != table_->map_->end(); }

    void DeleteKey() { it_ = table_->map_->erase(it_); }

    static MockKvIterator InvalidIterator() { return MockKvIterator(); }

    MockKvTransaction& GetTxn() const {
        static MockKvTransaction txn;
        return txn;
    }

    MockKvTable& GetTable() const { return *table_; }

    void OnIteratorChange() {}
};

class MockKvStore {
    std::map<std::string, MockKvTable> tables_;
    std::string data_file_;

 public:
    DISABLE_COPY(MockKvStore);
    DISABLE_MOVE(MockKvStore);

    MockKvStore() {}
    MockKvStore(const std::string& path, size_t s = 1, bool durable = false,
                bool create_if_not_exist = true)
        : data_file_(path + "/data.mem") {
        if (!fma_common::file_system::DirExists(path)) {
            if (!create_if_not_exist)
                throw KvException("Data directory " + path + " does not exist.");
            if (!fma_common::file_system::MkDir(path))
                throw KvException("Failed to create data directoy " + path);
        }
        if (fma_common::file_system::FileExists(data_file_)) {
            fma_common::InputFmaStream in(data_file_);
            if (fma_common::BinaryRead(in, tables_) != in.Size()) {
                throw KvException("KvException: failed to read file " + data_file_);
            }
        }
    }

    ~MockKvStore() {
        // write everything to disk
        fma_common::OutputFmaStream out(data_file_);
        if (out.Good()) {
            size_t n_bytes = fma_common::BinaryWrite(out, tables_);
            FMA_DBG() << "KvStore " << data_file_ << " is of size " << n_bytes;
        }
    }

    MockKvTransaction CreateReadTxn() { return MockKvTransaction(); }

    MockKvTransaction CreateWriteTxn(bool optimisitc = true, bool flush = true) {
        return MockKvTransaction();
    }

    MockKvTable OpenTable(MockKvTransaction& txn, const std::string& table_name,
                          bool create_if_not_exist, bool key_is_integer = false,
                          KeySortFunc key_cmp = nullptr) {
        auto it = tables_.find(table_name);
        if (it != tables_.end()) {
            return it->second;
        }
        MockKvTable t = MockKvTable(txn, table_name, create_if_not_exist, key_is_integer, key_cmp);
        tables_.emplace_hint(it, table_name, t);
        return t;
    }

    MockKvTable CreateTable(MockKvTransaction& txn, const std::string& table_name,
                            bool key_is_integer = false, KeySortFunc key_cmp = nullptr) {
        return OpenTable(txn, table_name, true, key_is_integer, key_cmp);
    }

    MockKvTable GetOpenedTable(const std::string& table_name) {
        auto it = tables_.find(table_name);
        if (it != tables_.end()) {
            return it->second;
        } else {
            throw KvException("Table not found");
        }
    }

    bool DeleteTable(MockKvTransaction& txn, const std::string& table_name) {
        auto it = tables_.find(table_name);
        if (it == tables_.end()) {
            try {
                MockKvTable t = MockKvTable(txn, table_name, false);
                it = tables_.emplace_hint(it, table_name, t);
            } catch (KvException&) {
                // no such table
                return false;
            }
        }
        it->second.Drop(txn);
        tables_.erase(it);
        return true;
    }

    std::vector<std::string> ListAllTables(MockKvTransaction& txn) {
        std::vector<std::string> tables;
        for (auto& kv : tables_) tables.emplace_back(kv.first);
        return tables;
    }

    std::map<std::string, MockKvTable> ListAllOpenedTables() { return tables_; }

    void Flush() {}

    void DropAll(MockKvTransaction& txn) { tables_.clear(); }

    void DumpStat(MockKvTransaction& txn, size_t& memory_size, size_t& height) {
        memory_size = 0;
        height = 0;
    }

    size_t Backup(const std::string& path, bool compact = false) { return 0; }

    void Snapshot(MockKvTransaction& txn, const std::string& path) {}

    void LoadSnapshot(const std::string& snapshot_path) {}

    void WarmUp(size_t* size) {
        if (size) {
            *size = 0;
            MockKvTransaction txn = CreateReadTxn();
            for (auto& kv : tables_) *size += kv.second.GetKeyCount(txn);
        }
    }

    static int64_t GetLastOpIdOfAllStores() { return 0; }
    static void SetLastOpIdOfAllStores(int64_t) {}
};

inline MockKvIterator MockKvTable::GetIterator(MockKvTransaction& txn) {
    return MockKvIterator(txn, *this, Value(), false);
}

inline MockKvIterator MockKvTable::GetIterator(MockKvTransaction& txn, const Value& key) {
    return MockKvIterator(txn, *this, key, false);
}

inline MockKvIterator MockKvTable::GetClosestIterator(MockKvTransaction& txn, const Value& key) {
    return MockKvIterator(txn, *this, key, true);
}

struct MockKvTypes {
    typedef MockKvStore Store;
    typedef MockKvTransaction Transaction;
    typedef MockKvTable Table;
    typedef MockKvIterator Iterator;
};

}  // namespace lgraph
