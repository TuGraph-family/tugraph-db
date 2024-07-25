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

#include <atomic>
#include <exception>
#include <unordered_map>

#include "core/field_data_helper.h"
#include "core/data_type.h"
#include "core/iterator_base.h"
#include "core/kv_store.h"
#include "core/kv_table_comparators.h"
#include "core/type_convert.h"
#include "core/value.h"
#include "core/vertex_index.h"

namespace lgraph {
class CompositeIndex;
class CompositeIndexIterator;

class CompositeIndexIterator : public ::lgraph::IteratorBase {
    friend class CompositeIndex;

    CompositeIndex* index_;
    std::unique_ptr<KvIterator> it_;
    Value key_end_;
    Value curr_key_;  // current indexed key, excluding vid
    VertexIndexValue iv_;   // VertexIndexValue, if this is non-unique index
    bool valid_;
    int pos_;
    VertexId vid_;  // current vid
    CompositeIndexType type_;

    /**
     * The constructor for VertexIndexIterator
     *
     * \param [in,out]  idx         The VertexIndex object this iterator will point to.
     * \param [in,out]  txn         The transaction.
     * \param [in,out]  table       The index table.
     * \param           key_start   The start key.
     * \param           key_end     The end key.
     * \param           vid         The vid from which to start searching.
     * \param           unique      Whether the index is a unique index.
     */
    CompositeIndexIterator(CompositeIndex* idx, Transaction* txn, KvTable& table,
                        const Value& key_start,
                        const Value& key_end,
                        VertexId vid, CompositeIndexType type);

    CompositeIndexIterator(CompositeIndex* idx, KvTransaction* txn, KvTable& table,
                        const Value& key_start,
                        const Value& key_end,
                        VertexId vid, CompositeIndexType type);

    bool KeyOutOfRange();

    bool PrevKV();

    bool KeyEquals(const Value& key);

    /** Loads content from iterator, assuming iterator is already at the right
     * position. */
    void LoadContentFromIt();

    DISABLE_COPY(CompositeIndexIterator);
    CompositeIndexIterator& operator=(CompositeIndexIterator&&) = delete;

 protected:
    void CloseImpl() override;

 public:
    CompositeIndexIterator(CompositeIndexIterator&& rhs) noexcept;

    /**
     * Query if this iterator is valid, i.e. the Key and Vid can be queried.
     *
     * \return  True if valid, false if not.
     */
    bool IsValid() const { return valid_; }

    /**
     * Move to the next vertex id in the list, which consists of all the valid
     * vertex ids of the iterator and is sorted from small to large.
     *
     * \return  True if it succeeds, otherwise false.
     */
    bool Next();

    /**
     * Gets the current key.
     *
     * \return  The current key.
     */
    Value GetKey() const;

    /**
     * Gets the current vertex id.
     *
     * \return  The current vertex id.
     */
    int64_t GetVid() const { return vid_; }

    std::vector<FieldData> GetKeyData() const;

    std::vector<FieldType> KeyType() const;


    /**
     * Determines if we can refresh content if kv iterator modified
     *
     * @return  True if KvIterator was modified.
     */
    void RefreshContentIfKvIteratorModified() override;
};

/**
 * The indices
 */
class CompositeIndex {
    friend class Schema;
    friend class LightningGraph;
    friend class Transaction;
    friend class CompositeIndexIterator;
    friend class IndexManager;

    std::shared_ptr<KvTable> table_;
    std::vector<FieldType> key_types;
    std::atomic<bool> ready_;
    std::atomic<bool> disabled_;
    CompositeIndexType type_;

 public:
    CompositeIndex(std::shared_ptr<KvTable> table, std::vector<FieldType> key_types,
                   CompositeIndexType type);

    CompositeIndex(const CompositeIndex& rhs);

    CompositeIndex(CompositeIndex&& rhs) = delete;

    CompositeIndex& operator=(const CompositeIndex& rhs) = delete;

    CompositeIndex& operator=(CompositeIndex&& rhs) = delete;

    static std::unique_ptr<KvTable> OpenTable(KvTransaction& txn, KvStore& store,
                                              const std::string& name,
                                              const std::vector<FieldType> &dt,
                                              CompositeIndexType type);

    void _AppendCompositeIndexEntry(KvTransaction& txn, const Value& k, VertexId vid);

    /**
     * Append an entry to non-unique vertex composite index.
     *
     * \param   txn     The transaction.
     * \param   k       The key, raw format.
     * \param   vids    All the vids corresponding to k. The vids must be sorted.
     */
    void _AppendNonUniqueCompositeIndexEntry(KvTransaction& txn, const Value& k,
                                          const std::vector<VertexId>& vids);

    bool Add(KvTransaction& txn, const Value& k, int64_t vid);

    bool IsReady() const {
        return ready_.load(std::memory_order_acquire) && !disabled_.load(std::memory_order_acquire);
    }

    CompositeIndexIterator GetUnmanagedIterator(KvTransaction& txn,
                                                const Value& key_start, const Value& key_end,
                                                VertexId vid = 0) {
        return CompositeIndexIterator(this, &txn, *table_, key_start,
            key_end, vid, type_);
    }

    CompositeIndexIterator GetIterator(Transaction* txn, const Value& key_start,
                                       const Value& key_end, VertexId vid = 0) {
        return CompositeIndexIterator(this, txn, *table_, key_start,
                                      key_end, vid, type_);
    }

 private:
    void Clear(KvTransaction& txn) { table_->Drop(txn); }

    void SetReady() { ready_.store(true, std::memory_order_release); }

    void Disable() { disabled_.store(true, std::memory_order_release); }

    void Enable() { disabled_.store(false, std::memory_order_release); }

    [[nodiscard]] bool IsDisabled() const { return disabled_.load(std::memory_order_acquire); }

    /**
     * Delete a specified composite vertex under a specified key.
     *
     * \param [in,out]  txn The transaction.
     * \param           key The key.
     * \param           vid The vid.
     *
     * \return  Whether the operation succeeds or not.
     */
    bool Delete(KvTransaction& txn, const Value& k, int64_t vid);
};

namespace composite_index_helper {
static inline Value GenerateCompositeIndexKey(std::vector<Value> keys) {
    int n = keys.size(), len = (n - 1) * 2;
    for (int i = 0; i < n; ++i) {
        len += keys[i].Size();
    }
    Value res(len);
    int16_t off = 0;
    for (int i = 0; i < n - 1; ++i) {
        off += keys[i].Size();
        memcpy(res.Data() + i * 2, &off, sizeof(int16_t));
    }
    off = 0;
    for (int i = 0; i < n; ++i) {
        memcpy(res.Data() + (n - 1) * 2 + off, keys[i].Data(), keys[i].Size());
        off += keys[i].Size();
    }
    return res;
}

static inline std::vector<FieldData> CompositeIndexKeyToFieldData(const Value& key,
                                     std::vector<FieldType> types) {
    std::vector<FieldData> res;
    int len = types.size();
    std::vector<int16_t> offset(len + 1);
    for (int i = 1; i < len; ++i) {
        memcpy(&offset[i], (char*)key.Data() + (i - 1) * 2, 2);
    }
    offset[len] = key.Size() - (len - 1) * 2;
    for (int i = 0; i < len; ++i) {
        res.push_back(field_data_helper::ValueToFieldData(Value((char*)key.Data() +
                      (len - 1) * 2 + offset[i], offset[i + 1]-offset[i]), types[i]));
    }
    return res;
}

}  // namespace composite_index_helper
}  // namespace lgraph
