﻿/**
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

#include "core/kv_store_exception.h"
#include "core/kv_store_transaction.h"
#include "core/kv_table_comparators.h"
#include "core/task_tracker.h"
#include "core/value.h"

namespace lgraph {

class KvTransaction;

class KvIterator;

class KvStore;

class KvTable {
    MDB_dbi dbi_;
    std::string name_;

    friend class KvStore;
    friend class KvIterator;
    friend class KvTransaction;
    MDB_dbi& GetDbi() { return dbi_; }

    std::function<int(const MDB_val*, const MDB_val*)> compare_key_;

    size_t GetVersion(KvTransaction& txn, const Value& key);

 public:
    KvTable() {}

    /**
     * Constructor
     *
     * \param [in,out]  txn                 The transaction.
     * \param           name                The name of the table.
     * \param           create_if_not_exist True to create if table does not exist.
     * \param           key_is_uint         Is the key a uint32 or uint64?
     * \param           comparator_desc     Describes the comparator. Ignored if key_is_uint==True
     */
    KvTable(KvTransaction& txn,
        const std::string& name,
        bool create_if_not_exist,
        const ComparatorDesc& comparator_desc);

    /**
     * Query if the key exists in the table
     *
     * \param [in,out]  txn The transaction.
     * \param           key The key.
     *
     * \return  True if key exists, false if not.
     */
    bool HasKey(KvTransaction& txn, const Value& key);

    /**
     * Gets a value corresponding to the key
     *
     * \param [in,out]  txn The transaction.
     * \param           key The key.
     * \param           for_update Whether to check conflicts upon commit.
     *
     * \return  The value. An empty value is returned if the key does not exist.
     */
    Value GetValue(KvTransaction& txn, const Value& key, bool for_update = false);

    /**
     * Gets number of k-v pairs in the table.
     *
     * \param [in,out]  txn The transaction.
     *
     * \return  The kv pair count.
     */
    size_t GetKeyCount(KvTransaction& txn);

    /**
     * Sets the value corresponding to the key. If the key does not exist, then it is created.
     *
     * \param [in,out]  txn                 The transaction.
     * \param           key                 The key.
     * \param           value               The value.
     * \param           overwrite_if_exist  (Optional) True to overwrite, false to preserve if
     * exist.
     *
     * \return  true if success, false if key already exist and overwrite_if_exist=false
     */
    bool SetValue(KvTransaction& txn, const Value& key, const Value& value,
                  bool overwrite_if_exist = true);

    /**
     * Adds a key-value pair to the table.
     *
     * \param [in,out]  txn     The transaction.
     * \param           key     The key.
     * \param           value   The value.
     *
     * \return  true if success, false if key already exists
     */
    bool AddKV(KvTransaction& txn, const Value& key, const Value& value);

    /**
     * Appends a kv to the end of the table. The key MUST sort behind the last key in table.
     *
     * \param [in,out] txn      The transaction.
     * \param          key      The key.
     * \param          value    The value.
     */
    void AppendKv(KvTransaction& txn, const Value& key, const Value& value);

    /**
     * Deletes the key and corresponding value.
     *
     * \param [in,out]  txn The transaction.
     * \param           key The key.
     *
     * \return  True if it succeeds, false if the key does not exist.
     */
    bool DeleteKey(KvTransaction& txn, const Value& key);

    /**
     * Gets an iterator place at the first key of the table. If there is no key, returns an invalid
     * iterator.
     *
     * \param [in,out]  txn The transaction.
     *
     * \return  The iterator.
     */
    KvIterator GetIterator(KvTransaction& txn);

    /**
     * Gets an iterator placed at key. If key does not exist, returns an invalid iterator.
     *
     * \param [in,out]  txn The transaction.
     * \param           key The key.
     *
     * \return  The iterator.
     */
    KvIterator GetIterator(KvTransaction& txn, const Value& key);

    KvIterator GetClosestIterator(KvTransaction& txn, const Value& key);

    int CompareKey(KvTransaction& txn, const Value& k1, const Value& k2) const;

    void Dump(KvTransaction& txn,
              const std::function<void(const Value& key, void* context)>& begin_key,
              const std::function<void(const Value& value, void* context)>& dump_value,
              const std::function<void(const Value& key, void* context)>& finish_key,
              void* context);

    void Print(KvTransaction& txn, const std::function<std::string(const Value& key)>& print_key,
               const std::function<std::string(const Value& key)>& print_value);

    /**
     * Drops this table. Please make sure no other thread will access this table after this call.
     *
     * \param [in,out]  txn             The transaction.
     */
    void Drop(KvTransaction& txn);

    /**
     * Gets the name of the table.
     *
     * \return  A reference to a std::string.
     */
    const std::string& Name() const { return name_; }

    /**
     * Heterogeneous key comparators
     */

    using is_transparent = void;

    bool operator()(const std::string& a, const std::string& b) const {
        MDB_val v_a = {a.size(), (void*)a.data()};
        MDB_val v_b = {b.size(), (void*)b.data()};
        return compare_key_(&v_a, &v_b) < 0;
    }

    bool operator()(const std::string& a, const Value& b) const {
        MDB_val v_a = {a.size(), (void*)a.data()};
        MDB_val v_b = b.MakeMdbVal();
        return compare_key_(&v_a, &v_b) < 0;
    }

    bool operator()(const Value& a, const std::string& b) const {
        MDB_val v_a = a.MakeMdbVal();
        MDB_val v_b = {b.size(), (void*)b.data()};
        return compare_key_(&v_a, &v_b) < 0;
    }

    bool operator()(const Value& a, const Value& b) const {
        MDB_val v_a = a.MakeMdbVal();
        MDB_val v_b = b.MakeMdbVal();
        return compare_key_(&v_a, &v_b) < 0;
    }
};
}  // namespace lgraph
