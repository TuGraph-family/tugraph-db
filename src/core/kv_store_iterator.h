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
#include "core/kv_store_table.h"
#include "core/lmdb/lmdb.h"
#include "core/task_tracker.h"

namespace lgraph {
class KvTable;

class DeltaStore;

/**
 * An KvIterator is a pointer to a key-value pair in a table. It is associated with a
 * transaction and a table. Iterators should be closed before a transaction is committed or
 * aborted. An iterator is invalidated if:
 *      1. the key is not found during construction, or
 *      2. any of the non-const members throws exception, or
 *      3. Next() and Goto() returns false
 * Trying to call Next() or GetKey() or GetValue() on an invalid iterator is an error and can
 * lead to segmentation fault or reading arbitrary bytes.
 */
class KvIterator {
    KvTransaction* txn_;
    KvTable* table_;
    MDB_cursor* cursor_ = nullptr;
    bool valid_ = false;
    MDB_val key_;
    MDB_val value_;
    friend class KvTransaction;

    KvIterator& operator=(const KvIterator&) = delete;

    DeltaStore* delta_ = nullptr;
    std::map<std::string, std::string, KvTable>::iterator iter_;
    int8_t main_status_;
    int8_t delta_status_;
    int8_t current_mode_;
    int8_t current_cursor_;

    int Compare();

    void Fix();

    void MoveForwardMain();

    void MoveForwardDelta();

    void MoveBackwardMain();

    void MoveBackwardDelta();

 public:
    ~KvIterator();

    /** Closes an iterator. Must be called BEFORE closing the enclosing transaction. The iterator
     * MUST NOT be used after being closed. Doing so may lead to segmentation fault. */
    void Close();

    /**
     * Place the iterator at the key. If the key is empty, iterator is placed at the first key. If
     * key is specified, and closest is false, and invalid iterator is returned if key does not
     * exist.
     * @param [in,out] txn              The transaction.
     * @param [in,out] table            The table.
     * @param          refresh_callback Callback function when iterator was refreshed.
     * @param          key              The key.
     * @param          closest          True to closest.
     */
    KvIterator(KvTransaction& txn, KvTable& table, const Value& key, bool closest);

    KvIterator(KvTransaction& txn, KvTable& table);

    KvIterator(const KvIterator& rhs);

    KvIterator(KvIterator&& rhs);

    KvIterator& operator=(KvIterator&& rhs);

    /**
     * Determines if the underlying cursor was modified by other write
     * operations, in which case we need to refresh the content of this
     * iterator and all upper classes.
     *
     * @return  True if the cursor was valid and had been modified.
     */
    bool UnderlyingPointerModified();

    /**
     * Reload the data from underlying cursor. This is usually used when
     * the cursor was modified by other write operations, which can be
     * determined by calling UnderlyingPointerModified.
     *
     * @return  True if it succeeds, false if it fails.
     */
    bool RefreshAfterModify();

    /**
     * Point the iterator to the first value of next key. It will invalidate the iterator if it
     * already points to the last key.
     *
     * \return  true if success, false if there is no next key.
     */
    bool Next();

    /**
     * Move to the previous item in the collection. If the iterator is
     * invalid, return to the last key.
     *
     * \return  True if it succeeds, false if there is no previous key.
     */
    bool Prev();

    /**
     * Goto key
     *
     * \param   key The key.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool GotoKey(const Value& key);

    /**
     * Goto closest key
     *
     * \param   key The key.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool GotoClosestKey(const Value& key);

    bool GotoLastKey();

    bool GotoFirstKey();

    /**
     * Gets the current key. Call only when iterator is valid.
     *
     * \return  The key.
     */
    Value GetKey() const;

    /**
     * Gets the current value. Call only when iterator is valid.
     *
     * \return  The value.
     */
    Value GetValue(bool for_update = false);

    /**
     * Replace the value to which the iterator points. The iterator will point to the new key-value
     * pair if the operation succeeded.
     *
     * \param   value   The new value.
     */
    void SetValue(const Value& value);

    /**
     * Adds a key value to the table. The iterator will point to the new pair after this call. Any
     * error will invalidate the iterator. Invalidates the iterator if call fails.
     *
     * @param key       The key.
     * @param value     The value.
     * @param overwrite (Optional) True to overwrite if the key already exists.
     *
     * @return  True if it succeeds, false if key exists and overwrite==false.
     */
    bool AddKeyValue(const Value& key, const Value& value, bool overwrite = false);

    /**
     * Query if this iterator is valid. Methods except for GotoXXX() will fail on invalid iterator.
     *
     * \return  True if valid, false if not.
     */
    bool IsValid() const;

    /**
     * Deletes the key and all associated values. If operation succeeds, iterator points to the next
     * key. If it fails, the iterator becomes invalid.
     */
    void DeleteKey();

    KvTransaction& GetTxn() const { return *txn_; }

    KvTable& GetTable() const { return *table_; }
};
}  // namespace lgraph
