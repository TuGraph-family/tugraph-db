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

int TestVertexIndexImpl();
int CURDVertexWithTooLongKey();
int TestVRefreshContentIfKvIteratorModified();

namespace lgraph {
class Transaction;
class VertexIndex;
class VertexIndexIterator;
class Schema;
class CompositeIndex;
class CompositeIndexIterator;

/**
 * An VertexIndexValue packs multiple vids into a single value.
 */
class VertexIndexValue {
    /**
     * uint8_t count
     * [5 bytes vid] * count
     */

    friend class VertexIndex;
    friend class VertexIndexIterator;
    friend class CompositeIndex;
    friend class CompositeIndexIterator;

    Value v_;

    VertexIndexValue();

    explicit VertexIndexValue(const Value& v);

    explicit VertexIndexValue(Value&& v);

    template <typename IT>
    VertexIndexValue(const IT& beg, const IT& end) {
        size_t n = end - beg;
        FMA_DBG_ASSERT(n < std::numeric_limits<uint8_t>::max());
        v_.Resize(1 + _detail::VID_SIZE * n);
        char* p = v_.Data();
        *(uint8_t*)p = (uint8_t)n;
        p++;
        for (auto it = beg; it < end; it++) {
            _detail::WriteVid(p, *it);
            p += _detail::VID_SIZE;
        }
    }

    int GetVidCount() const { return static_cast<int>(*(uint8_t*)v_.Data()); }

    int64_t GetNthVid(int n) const {
        return _detail::GetVid(v_.Data() + 1 + _detail::VID_SIZE * n);
    }

    /**
     * Search for the specified vertex id
     *
     * \param           vid     The vid.
     * \param [in,out]  found   Is this vid found?
     *
     * \return  Position where the vid is found.
     * list.
     */
    int SearchVid(int64_t vid, bool& found) const;

    /**
     * Insert the specified vid.
     *
     * \param           vid     The vid.
     * \return  The status of the operation.
     *
     * return value:
     *   0, vid already exists
     *   1, the key does not change
     *   2, the key changes
     */
    uint8_t InsertVid(int64_t vid);

    void InsertVidAtNthPos(int pos, int64_t vid);

    /**
     * Delete the specified vid.
     *
     * \param           vid     The vid.
     * \return  The status of the operation.
     *
     * return value:
     *   0, vid does not exist
     *   1, the key does not change
     *   2, the key changes
     */
    uint8_t DeleteVidIfExist(int64_t vid);

    void DeleteNthVid(int pos);

    bool TooLarge() { return v_.Size() > _detail::NODE_SPLIT_THRESHOLD; }

    VertexIndexValue SplitRightHalf();

    const Value& GetBuf() const { return v_; }

    Value& GetBuf() { return v_; }

    /**
     * Patch the key with the last vid in this VertexIndexValue.
     */
    Value CreateKey(const Value& key) const;
};

/**
 * The iterator for indices
 */
class VertexIndexIterator : public ::lgraph::IteratorBase {
    friend class VertexIndex;

    VertexIndex* index_;
    std::unique_ptr<KvIterator> it_;
    Value key_end_;
    Value curr_key_;  // current indexed key, excluding vid
    VertexIndexValue iv_;   // VertexIndexValue, if this is non-unique index
    bool valid_;
    int pos_;
    VertexId vid_;  // current vid
    IndexType type_;

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
    VertexIndexIterator(VertexIndex* idx, Transaction* txn, KvTable& table,
                        const Value& key_start,
                        const Value& key_end,
                        VertexId vid, IndexType type);

    VertexIndexIterator(VertexIndex* idx, KvTransaction* txn, KvTable& table,
                        const Value& key_start,
                        const Value& key_end,
                        VertexId vid, IndexType type);

    bool KeyOutOfRange();

    /**
     * Traverse to the previous key/value pair. Used when an inserted vid is
     * greater than all existing vids. Should be used together with KeyEquals.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool PrevKV();

    /**
     * Check whether the iterator's current key starts with key.
     *
     * \param        key       The key.
     * \return  Whether the iterator's current key starts with key.
     */
    bool KeyEquals(const Value& key);

    /** Loads content from iterator, assuming iterator is already at the right
     * position. */
    void LoadContentFromIt();

    DISABLE_COPY(VertexIndexIterator);
    VertexIndexIterator& operator=(VertexIndexIterator&&) = delete;

 protected:
    void CloseImpl() override;

 public:
    VertexIndexIterator(VertexIndexIterator&& rhs);

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

    bool Goto(VertexId vid);


    /**
     * Gets the current key.
     *
     * \return  The current key.
     */
    Value GetKey() const;


    FieldData GetKeyData() const {
        return field_data_helper::ValueToFieldData(GetKey(), KeyType());
    }

    /**
     * Gets the current vertex id.
     *
     * \return  The current vertex id.
     */
    int64_t GetVid() const { return vid_; }

    FieldType KeyType() const;

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
class VertexIndex {
    friend class Schema;
    friend class IndexManager;
    friend class LightningGraph;
    friend class Transaction;
    friend int ::TestVertexIndexImpl();
    friend int ::CURDVertexWithTooLongKey();
    friend int ::TestVRefreshContentIfKvIteratorModified();

    std::shared_ptr<KvTable> table_;
    FieldType key_type_;
    std::atomic<bool> ready_;
    std::atomic<bool> disabled_;
    IndexType type_;

 public:
    VertexIndex(std::shared_ptr<KvTable> table, FieldType key_type, IndexType type);

    VertexIndex(const VertexIndex& rhs);


    VertexIndex(VertexIndex&& rhs) = delete;

    VertexIndex& operator=(const VertexIndex& rhs) = delete;

    VertexIndex& operator=(VertexIndex&& rhs) = delete;

    static std::unique_ptr<KvTable> OpenTable(KvTransaction& txn, KvStore& store,
                                   const std::string& name, FieldType dt, IndexType type);

    void _AppendVertexIndexEntry(KvTransaction& txn, const Value& k, VertexId vid);


    /**
     * Append an entry to non-unique vertex index.
     *
     * \param   txn     The transaction.
     * \param   k       The key, raw format.
     * \param   vids    All the vids corresponding to k. The vids must be sorted.
     */
    void _AppendNonUniqueVertexIndexEntry(KvTransaction& txn, const Value& k,
                                          const std::vector<VertexId>& vids);


#define _GET_UNM_INDEX_ITER_WITH_EMPTY_START_KEY(ft)                         \
    do {                                                                     \
        auto d = field_data_helper::MinStoreValue<FieldType::ft>();          \
        return VertexIndexIterator(this, &txn, *table_, Value::ConstRef(d),   \
                                   std::forward<V2>(key_end),                \
                                   vid, type_);                            \
    } while (0)

    /**
     * Get an Vertex index iterator.
     *
     * \param [in,out]  txn         The transaction.
     * \param           key_start   The start key.
     * \param           key_end     The end key.
     * \param           vid         The vid to start from.
     *
     * \return  The index iterator.
     */
    template <typename V1, typename V2>
    VertexIndexIterator GetUnmanagedIterator(KvTransaction& txn, V1&& key_start, V2&& key_end,
                                             VertexId vid = 0) {
        if (key_start.Empty()) {
            switch (key_type_) {
            case FieldType::BOOL:
                _GET_UNM_INDEX_ITER_WITH_EMPTY_START_KEY(BOOL);
            case FieldType::INT8:
                _GET_UNM_INDEX_ITER_WITH_EMPTY_START_KEY(INT8);
            case FieldType::INT16:
                _GET_UNM_INDEX_ITER_WITH_EMPTY_START_KEY(INT16);
            case FieldType::INT32:
                _GET_UNM_INDEX_ITER_WITH_EMPTY_START_KEY(INT32);
            case FieldType::INT64:
                _GET_UNM_INDEX_ITER_WITH_EMPTY_START_KEY(INT64);
            case FieldType::DATE:
                _GET_UNM_INDEX_ITER_WITH_EMPTY_START_KEY(DATE);
            case FieldType::DATETIME:
                _GET_UNM_INDEX_ITER_WITH_EMPTY_START_KEY(DATETIME);
            case FieldType::FLOAT:
                _GET_UNM_INDEX_ITER_WITH_EMPTY_START_KEY(FLOAT);
            case FieldType::DOUBLE:
                _GET_UNM_INDEX_ITER_WITH_EMPTY_START_KEY(DOUBLE);
            case FieldType::STRING:
                {
                    return VertexIndexIterator(this, &txn, *table_, std::forward<V1>(key_start),
                        CutKeyIfLongOnlyForNonUniqueIndex(std::forward<V2>(key_end)), vid, type_);
                }
            case FieldType::BLOB:
                FMA_ASSERT(false) << "Blob fields must not be indexed.";
            default:
                FMA_ASSERT(false);
                return VertexIndexIterator(this, &txn, *table_, std::forward<V1>(key_start),
                                     std::forward<V2>(key_end), vid, type_);
            }
        } else {
            return VertexIndexIterator(this, &txn, *table_,
                   CutKeyIfLongOnlyForNonUniqueIndex(std::forward<V1>(key_start)),
                   CutKeyIfLongOnlyForNonUniqueIndex(std::forward<V2>(key_end)),
                   vid, type_);
        }
    }

#define _GET_INDEX_ITER_WITH_EMPTY_START_KEY(ft)                            \
    do {                                                                    \
        auto d = field_data_helper::MinStoreValue<FieldType::ft>();         \
        return VertexIndexIterator(this, txn, *table_, Value::ConstRef(d),   \
                                   std::forward<V2>(key_end),               \
                                   vid, type_);                           \
    } while (0)

    template <typename V1, typename V2>
    VertexIndexIterator GetIterator(Transaction* txn, V1&& key_start,
                                    V2&& key_end, VertexId vid = 0) {
        if (key_start.Empty()) {
            switch (key_type_) {
            case FieldType::BOOL:
                _GET_INDEX_ITER_WITH_EMPTY_START_KEY(BOOL);
            case FieldType::INT8:
                _GET_INDEX_ITER_WITH_EMPTY_START_KEY(INT8);
            case FieldType::INT16:
                _GET_INDEX_ITER_WITH_EMPTY_START_KEY(INT16);
            case FieldType::INT32:
                _GET_INDEX_ITER_WITH_EMPTY_START_KEY(INT32);
            case FieldType::INT64:
                _GET_INDEX_ITER_WITH_EMPTY_START_KEY(INT64);
            case FieldType::DATE:
                _GET_INDEX_ITER_WITH_EMPTY_START_KEY(DATE);
            case FieldType::DATETIME:
                _GET_INDEX_ITER_WITH_EMPTY_START_KEY(DATETIME);
            case FieldType::FLOAT:
                _GET_INDEX_ITER_WITH_EMPTY_START_KEY(FLOAT);
            case FieldType::DOUBLE:
                _GET_INDEX_ITER_WITH_EMPTY_START_KEY(DOUBLE);
            case FieldType::STRING:
                {
                    return VertexIndexIterator(this, txn, *table_, std::forward<V1>(key_start),
                           CutKeyIfLongOnlyForNonUniqueIndex(std::forward<V2>(key_end)),
                                               vid, type_);
                }
            case FieldType::BLOB:
                FMA_ASSERT(false) << "Blob fields must not be indexed.";
            default:
                FMA_ASSERT(false);
                return VertexIndexIterator(this, txn, *table_, std::forward<V1>(key_start),
                                     std::forward<V2>(key_end), vid, type_);
            }
        } else {
            return VertexIndexIterator(this, txn, *table_,
                   CutKeyIfLongOnlyForNonUniqueIndex(std::forward<V1>(key_start)),
                   CutKeyIfLongOnlyForNonUniqueIndex(std::forward<V2>(key_end)),
                   vid, type_);
        }
    }

    bool IsReady() const {
        return ready_.load(std::memory_order_acquire) && !disabled_.load(std::memory_order_acquire);
    }

    bool IsUnique() const { return type_ == IndexType::GlobalUniqueIndex; }

    IndexType GetType() const { return type_; }

    FieldType KeyType() { return key_type_; }

    void Dump(KvTransaction& txn,
              const std::function<std::string(const char* p, size_t s)>& key_to_string);

    size_t GetNumKeys(KvTransaction& txn) { return table_->GetKeyCount(txn); }

 private:
    void Clear(KvTransaction& txn) { table_->Drop(txn); }

    void SetReady() { ready_.store(true, std::memory_order_release); }

    void Disable() { disabled_.store(true, std::memory_order_release); }

    void Enable() { disabled_.store(false, std::memory_order_release); }

    bool IsDisabled() const { return disabled_.load(std::memory_order_acquire); }

    /**
     * Delete a specified vertex under a specified key.
     *
     * \param [in,out]  txn The transaction.
     * \param           key The key.
     * \param           vid The vid.
     *
     * \return  Whether the operation succeeds or not.
     */
    bool Delete(KvTransaction& txn, const Value& k, int64_t vid);

    /**
     * Move a specified vertex from an old key to a new key.
     *
     * \exception   IndexException  Thrown when an VertexIndex error condition occurs.
     *
     * \param [in,out]  txn     The transaction.
     * \param           old_key The old key.
     * \param           new_key The new key.
     * \param           vid     The vid.
     *
     * \return  Whether the operation succeeds or not.
     */
    bool Update(KvTransaction& txn, const Value& old_key, const Value& new_key, int64_t vid);

    /**
     * Add a specified vertex under a specified key.
     *
     * \exception   IndexException  Thrown when an VertexIndex error condition occurs.
     *
     * \param [in,out]  txn The transaction.
     * \param           key The key.
     * \param           vid The vid.
     *
     * \return  Whether the operation succeeds or not.
     */
    bool Add(KvTransaction& txn, const Value& k, int64_t vid);

    bool UniqueIndexConflict(KvTransaction& txn, const Value& k);

    size_t GetMaxVertexIndexKeySize();

    Value CutKeyIfLongOnlyForNonUniqueIndex(const Value& k);
};
}  // namespace lgraph
