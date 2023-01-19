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

namespace lgraph {
class Transaction;

namespace _detail {
/**
 * Patch the key with the specified vid.
 *
 * \param           key     The key.
 * \param           vid     The vid.
 * \return  The patched key.
 */
static Value PatchKeyWithVid(const Value& key, int64_t vid) {
    Value key_vid(key.Size() + VID_SIZE);
    memcpy(key_vid.Data(), key.Data(), key.Size());
    WriteVid(key_vid.Data() + key.Size(), vid);
    return key_vid;
}

inline size_t hash_c_string(const char* p, size_t s) {
    size_t result = 0;
    const size_t prime = 31;
    for (size_t i = 0; i < s; ++i) {
        result = p[i] + (result * prime);
    }
    return result;
}

inline Value CutKeyIfLong(const Value& k) {
    if (k.Size() < MAX_KEY_SIZE) return Value::ConstRef(k);
    return Value(k.Data(), MAX_KEY_SIZE);
}

inline Value ReturnKeyEvenIfLong(Value&& v) { return std::move(v); }

/**
 * The comparators for fixed-length types.
 * Compare first by key, then by vid.
 */
template <FieldType DT>
struct KeyVidCompareFunc {
    static int func(const MDB_val* a, const MDB_val* b) {
        typedef typename ::lgraph::field_data_helper::FieldType2StorageType<DT>::type T;
        FMA_DBG_ASSERT(a->mv_size == sizeof(T) + VID_SIZE && b->mv_size == sizeof(T) + VID_SIZE);
        int r = field_data_helper::ValueCompare<DT>(a->mv_data, sizeof(T), b->mv_data, sizeof(T));
        if (r != 0) return r;
        int64_t a_vid = GetVid((char*)a->mv_data + sizeof(T));
        int64_t b_vid = GetVid((char*)b->mv_data + sizeof(T));
        return a_vid < b_vid ? -1 : a_vid > b_vid ? 1 : 0;
    }
};

template <FieldType DT>
struct KeyCompareFunc {
    static int func(const MDB_val* a, const MDB_val* b) {
        typedef typename ::lgraph::field_data_helper::FieldType2StorageType<DT>::type T;
        return field_data_helper::ValueCompare<DT>(a->mv_data, a->mv_size, b->mv_data, b->mv_size);
    }
};

/**
 * The comparator for bytes or strings.
 */
static int LexicalKeyVidCompareFunc(const MDB_val* a, const MDB_val* b) {
    int diff;
    int len_diff;
    unsigned int len;

    len = static_cast<int>(a->mv_size) - VID_SIZE;
    len_diff = static_cast<int>(a->mv_size) - static_cast<int>(b->mv_size);
    if (len_diff > 0) {
        len = static_cast<int>(b->mv_size) - VID_SIZE;
        len_diff = 1;
    }

    diff = memcmp(a->mv_data, b->mv_data, len);
    if (diff == 0 && len_diff == 0) {
        int64_t a_vid = GetVid((char*)a->mv_data + a->mv_size - VID_SIZE);
        int64_t b_vid = GetVid((char*)b->mv_data + b->mv_size - VID_SIZE);
        return a_vid < b_vid ? -1 : a_vid > b_vid ? 1 : 0;
    }
    return static_cast<int>(diff ? diff : len_diff < 0 ? -1 : len_diff);
}

}  // namespace _detail

class VertexIndex;
class VertexIndexIterator;

/**
 * An VertexIndexValue packs multiple vids into a single value.
 */
class VertexIndexValue {
    /**
     * uint8_t count
     * [uint40_t vid] * count
     */

    friend class VertexIndex;
    friend class VertexIndexIterator;

    Value v_;

    VertexIndexValue() : v_(1) { *(uint8_t*)v_.Data() = 0; }

    explicit VertexIndexValue(const Value& v) : v_(v) {}

    explicit VertexIndexValue(Value&& v) : v_(std::move(v)) {}

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
    int SearchVid(int64_t vid, bool& found) const {
        if (GetVidCount() == 0) {
            found = false;
            return 0;
        }
        int beg = 0;
        int end = (int)GetVidCount();
        while (beg < end) {
            int p = (beg + end) / 2;
            int64_t pivot = GetNthVid(p);
            if (pivot == vid) {
                found = true;
                return p;
            }
            if (vid < pivot) {
                end = p;
            } else if (vid > pivot) {
                beg = p + 1;
            }
        }
        found = false;
        return end;
    }

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
    uint8_t InsertVid(int64_t vid) {
        bool exists;
        int pos = SearchVid(vid, exists);
        if (exists) {
            return 0;
        }
        bool tail = (pos == GetVidCount());
        InsertVidAtNthPos(pos, vid);
        return !tail ? 1 : 2;
    }

    void InsertVidAtNthPos(int pos, int64_t vid) {
        // insert the vid
        size_t bytes_after_p = v_.Size() - (1 + _detail::VID_SIZE * pos);
        v_.Resize(v_.Size() + _detail::VID_SIZE);
        char* ptr = v_.Data() + 1 + _detail::VID_SIZE * pos;
        memmove(ptr + _detail::VID_SIZE, ptr, bytes_after_p);
        _detail::WriteVid(ptr, vid);
        (*(uint8_t*)v_.Data())++;
    }

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
    uint8_t DeleteVidIfExist(int64_t vid) {
        bool exists;
        int pos = SearchVid(vid, exists);
        if (!exists) {
            return 0;
        }
        bool tail = (pos == GetVidCount() - 1);
        DeleteNthVid(pos);
        return !tail ? 1 : 2;
    }

    void DeleteNthVid(int pos) {
        v_.Resize(v_.Size());
        char* ptr = v_.Data() + 1 + _detail::VID_SIZE * pos;
        uint8_t n = GetVidCount();
        memmove(ptr, ptr + _detail::VID_SIZE, (n - pos - 1) * _detail::VID_SIZE);
        v_.Resize(v_.Size() - _detail::VID_SIZE);
        *(uint8_t*)v_.Data() = n - 1;
    }

    bool TooLarge() { return v_.Size() > _detail::NODE_SPLIT_THRESHOLD; }

    VertexIndexValue SplitRightHalf() {
        int n = GetVidCount();
        int next_half_size = n / 2;
        int this_half_size = n - next_half_size;
        VertexIndexValue newnode;
        newnode.v_.Resize(1 + next_half_size * _detail::VID_SIZE);
        char* p = newnode.v_.Data();
        *(uint8_t*)p = next_half_size;
        memcpy(p + 1, v_.Data() + 1 + this_half_size * _detail::VID_SIZE,
               next_half_size * _detail::VID_SIZE);
        v_.Resize(v_.Size() - next_half_size * _detail::VID_SIZE);
        *(uint8_t*)v_.Data() = this_half_size;
        return newnode;
    }

    const Value& GetBuf() const { return v_; }

    Value& GetBuf() { return v_; }

    /**
     * Patch the key with the last vid in this VertexIndexValue.
     */
    Value CreateKey(const Value& key) const {
        int pos = GetVidCount() - 1;
        Value v(key.Size() + _detail::VID_SIZE);
        memcpy(v.Data(), key.Data(), key.Size());
        memcpy(v.Data() + key.Size(), v_.Data() + 1 + pos * _detail::VID_SIZE, _detail::VID_SIZE);
        return v;
    }
};

/**
 * The iterator for indices
 */
class VertexIndexIterator : public ::lgraph::IteratorBase {
    friend class VertexIndex;

    VertexIndex* index_;
    KvIterator it_;
    Value key_end_;
    Value curr_key_;  // current indexed key, excluding vid
    VertexIndexValue iv_;   // VertexIndexValue, if this is non-unique index
    bool valid_;
    int pos_;
    VertexId vid_;  // current vid
    bool unique_;

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
                        VertexId vid, bool unique);

    VertexIndexIterator(VertexIndex* idx, KvTransaction* txn, KvTable& table,
                        const Value& key_start,
                        const Value& key_end,
                        VertexId vid, bool unique);

    bool KeyOutOfRange() {
        if (key_end_.Empty() || (!unique_ && key_end_.Size() == _detail::VID_SIZE)) return false;
        return it_.GetTable().CompareKey(it_.GetTxn(), it_.GetKey(), key_end_) > 0;
    }

    /**
     * Traverse to the previous key/value pair. Used when an inserted vid is
     * greater than all existing vids. Should be used together with KeyEquals.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool PrevKV() {
        if (!it_.Prev()) {
            return false;
        }
        LoadContentFromIt();
        return true;
    }

    /**
     * Check whether the iterator's current key starts with key.
     *
     * \param        key       The key.
     * \return  Whether the iterator's current key starts with key.
     */
    bool KeyEquals(const Value& key) {
        if (unique_) {
            const Value& k = it_.GetKey();
            return key.Size() == k.Size() && (memcmp(k.Data(), key.Data(), k.Size()) == 0);
        } else {
            auto key_vid = it_.GetKey();
            if (key_vid.Size() - _detail::VID_SIZE != key.Size()) {
                return false;
            }
            return memcmp(key.Data(), key_vid.Data(), key.Size()) == 0;
        }
    }

    /** Loads content from iterator, assuming iterator is already at the right
     * position. */
    void LoadContentFromIt() {
        valid_ = true;
        pos_ = 0;
        curr_key_.Copy(GetKey());
        if (!unique_) {
            iv_ = VertexIndexValue(it_.GetValue());
            vid_ = iv_.GetNthVid(pos_);
        } else {
            vid_ = _detail::GetVid(it_.GetValue().Data());
        }
    }

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
    bool Next() {
        // if we haven't reach the end of the current VertexIndexValue,
        // just move forward
        if (!unique_ && pos_ < iv_.GetVidCount() - 1) {
            pos_ += 1;
            vid_ = iv_.GetNthVid(pos_);
            return true;
        }
        // otherwise try moving to the next VertexIndex Value,
        // and check if it is still within the range
        valid_ = false;
        if (!it_.Next() || KeyOutOfRange()) {
            return false;
        }
        LoadContentFromIt();
        return true;
    }

    /**
     * Gets the current key.
     *
     * \return  The current key.
     */
    Value GetKey() const {
        if (unique_) {
            return _detail::ReturnKeyEvenIfLong(it_.GetKey());
        } else {
            Value key_vid = it_.GetKey();
            return _detail::ReturnKeyEvenIfLong(
                Value(key_vid.Data(), key_vid.Size() - _detail::VID_SIZE));
        }
    }

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
    void RefreshContentIfKvIteratorModified() override {
        if (IsValid() && it_.IsValid() && it_.UnderlyingPointerModified()) {
            valid_ = false;
            // now it_ points to a valid position, but not necessary the right one
            it_.GotoClosestKey(unique_ ? curr_key_ : _detail::PatchKeyWithVid(curr_key_, vid_));
            FMA_DBG_ASSERT(it_.IsValid());
            if (KeyOutOfRange()) return;
            if (unique_) {
                // found the right key
                LoadContentFromIt();
                return;
            } else {
                // non-unique, need to find correct pos_
                iv_ = VertexIndexValue(it_.GetValue());
                pos_ = iv_.SearchVid(vid_, valid_);
                if (pos_ >= iv_.GetVidCount()) return;
                valid_ = true;
                curr_key_.Copy(GetKey());
                vid_ = iv_.GetNthVid(pos_);
                return;
            }
        }
    }
};

class Schema;

/**
 * The indices
 */
class VertexIndex {
    friend class Schema;
    friend class IndexManager;
    friend class LightningGraph;
    friend class Transaction;
    friend int ::TestVertexIndexImpl();

    KvTable table_;
    FieldType key_type_;
    std::atomic<bool> ready_;
    std::atomic<bool> disabled_;
    bool unique_;

 public:
    VertexIndex(const KvTable& table, FieldType key_type, bool unique)
        : table_(table), key_type_(key_type), ready_(false), disabled_(false), unique_(unique) {}

    VertexIndex(const VertexIndex& rhs)
        : table_(rhs.table_),
          key_type_(rhs.key_type_),
          ready_(rhs.ready_.load()),
          disabled_(rhs.disabled_.load()),
          unique_(rhs.unique_) {}

    VertexIndex(VertexIndex&& rhs) = delete;

    VertexIndex& operator=(const VertexIndex& rhs) = delete;

    VertexIndex& operator=(VertexIndex&& rhs) = delete;

    static KvTable OpenTable(KvTransaction& txn, KvStore& store, const std::string& name,
                             FieldType dt, bool unique) {
        ComparatorDesc desc;
        desc.comp_type = unique ? ComparatorDesc::SINGLE_TYPE : ComparatorDesc::TYPE_AND_VID;
        desc.data_type = dt;
        return store.OpenTable(txn, name, true, desc);
    }

    void _AppendVertexIndexEntry(KvTransaction& txn, const Value& k, VertexId vid) {
        FMA_DBG_ASSERT(unique_);
        FMA_DBG_CHECK_LE(k.Size(), _detail::MAX_KEY_SIZE);
        table_.AppendKv(txn, k, Value::ConstRef(vid));
    }

    /**
     * Append an entry to non-unique vertex index.
     *
     * \param   txn     The transaction.
     * \param   k       The key, raw format.
     * \param   vids    All the vids corresponding to k. The vids must be sorted.
     */
    void _AppendNonUniqueVertexIndexEntry(KvTransaction& txn, const Value& k,
                                          const std::vector<VertexId>& vids) {
        FMA_DBG_ASSERT(!unique_);
        FMA_DBG_ASSERT(!vids.empty());
        FMA_DBG_CHECK_LE(k.Size(), _detail::MAX_KEY_SIZE);
        size_t vid_per_idv = _detail::NODE_SPLIT_THRESHOLD / _detail::VID_SIZE;
        for (size_t i = 0; i < vids.size(); i += vid_per_idv) {
            size_t end = i + vid_per_idv;
            end = end <= vids.size() ? end : vids.size();
            VertexIndexValue idv(vids.begin() + i, vids.begin() + end);
            Value key = idv.CreateKey(k);
            table_.AppendKv(txn, key, idv.GetBuf());
        }
    }

#define _GET_UNM_INDEX_ITER_WITH_EMPTY_START_KEY(ft)                         \
    do {                                                                     \
        auto d = field_data_helper::MinStoreValue<FieldType::ft>();          \
        return VertexIndexIterator(this, &txn, table_, Value::ConstRef(d),   \
                                   std::forward<V2>(key_end),                \
                                   vid, unique_);                            \
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
                    return VertexIndexIterator(this, &txn, table_, std::forward<V1>(key_start),
                                         _detail::CutKeyIfLong(std::forward<V2>(key_end)), vid,
                                         unique_);
                    break;
                }
            case FieldType::BLOB:
                FMA_ASSERT(false) << "Blob fields must not be indexed.";
            default:
                FMA_ASSERT(false);
                return VertexIndexIterator(this, &txn, table_, std::forward<V1>(key_start),
                                     std::forward<V2>(key_end), vid, unique_);
            }
        } else {
            return VertexIndexIterator(this, &txn, table_,
                                       _detail::CutKeyIfLong(std::forward<V1>(key_start)),
                                       _detail::CutKeyIfLong(std::forward<V2>(key_end)),
                                       vid, unique_);
        }
    }

#define _GET_INDEX_ITER_WITH_EMPTY_START_KEY(ft)                            \
    do {                                                                    \
        auto d = field_data_helper::MinStoreValue<FieldType::ft>();         \
        return VertexIndexIterator(this, txn, table_, Value::ConstRef(d),   \
                                   std::forward<V2>(key_end),               \
                                   vid, unique_);                           \
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
                    return VertexIndexIterator(this, txn, table_, std::forward<V1>(key_start),
                                         _detail::CutKeyIfLong(std::forward<V2>(key_end)), vid,
                                         unique_);
                    break;
                }
            case FieldType::BLOB:
                FMA_ASSERT(false) << "Blob fields must not be indexed.";
            default:
                FMA_ASSERT(false);
                return VertexIndexIterator(this, txn, table_, std::forward<V1>(key_start),
                                     std::forward<V2>(key_end), vid, unique_);
            }
        } else {
            return VertexIndexIterator(this, txn, table_,
                                       _detail::CutKeyIfLong(std::forward<V1>(key_start)),
                                       _detail::CutKeyIfLong(std::forward<V2>(key_end)),
                                                             vid, unique_);
        }
    }

    bool IsReady() const {
        return ready_.load(std::memory_order_acquire) && !disabled_.load(std::memory_order_acquire);
    }

    bool IsUnique() const { return unique_; }

    FieldType KeyType() { return key_type_; }

    void Dump(KvTransaction& txn,
              const std::function<std::string(const char* p, size_t s)>& key_to_string) {
        for (auto it = table_.GetIterator(txn); it.IsValid(); it.Next()) {
            const Value& k = it.GetKey();
            VertexIndexValue iv(it.GetValue());
            std::string line = key_to_string(k.Data(), k.Size() - _detail::VID_SIZE);
            line.append(" -> ");
            if (unique_) {
                line.append(std::to_string(_detail::GetVid(iv.GetBuf().Data())));
            } else {
                for (int i = 0; i < iv.GetVidCount(); i++) {
                    line.append(std::to_string(iv.GetNthVid(i)));
                    line.append(",");
                }
            }
            FMA_LOG() << line;
        }
    }

    size_t GetNumKeys(KvTransaction& txn) { return table_.GetKeyCount(txn); }

 private:
    void Clear(KvTransaction& txn) { table_.Drop(txn); }

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
    bool Delete(KvTransaction& txn, const Value& k, int64_t vid) {
        Value key = _detail::CutKeyIfLong(k);
        VertexIndexIterator it = GetUnmanagedIterator(txn, key, key, vid);
        if (!it.IsValid() || it.KeyOutOfRange()) {
            // no such key_vid
            return false;
        }
        if (unique_) {
            it.it_.DeleteKey();
            return true;
        } else {
            uint8_t ret = it.iv_.DeleteVidIfExist(vid);
            if (ret == 1) {
                // the key does not change after deletion
                it.it_.SetValue(it.iv_.GetBuf());
            } else if (ret == 2) {
                // the key changes after deletion
                it.it_.DeleteKey();
                if (it.iv_.GetVidCount()) {
                    bool r = it.it_.AddKeyValue(it.iv_.CreateKey(key), it.iv_.GetBuf());
                    FMA_DBG_ASSERT(r);
                }
            } else {
                // no such vid
                return false;
            }
            return true;
        }
    }

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
    bool Update(KvTransaction& txn, const Value& old_key, const Value& new_key, int64_t vid) {
        if (!Delete(txn, old_key, vid)) {
            return false;
        }
        return Add(txn, new_key, vid);
    }

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
    bool Add(KvTransaction& txn, const Value& k, int64_t vid, bool throw_on_long_key = true) {
        if (k.Size() > _detail::MAX_KEY_SIZE && throw_on_long_key) {
            throw InputError("VertexIndex key size too large, must be less than 480.");
        }
        Value key = _detail::CutKeyIfLong(k);
        if (unique_) {
            return table_.AddKV(txn, key, Value::ConstRef(vid));
        }
        VertexIndexIterator it = GetUnmanagedIterator(txn, key, key, vid);
        if (!it.IsValid() || it.KeyOutOfRange()) {
            if (!it.PrevKV() || !it.KeyEquals(key)) {
                // create a new VertexIndexValue
                VertexIndexValue iv;
                uint8_t r = iv.InsertVid(vid);
                FMA_DBG_CHECK_NEQ(r, 0);
                bool r2 = table_.AddKV(txn, iv.CreateKey(key), iv.GetBuf());
                FMA_DBG_ASSERT(r2);
                return true;
            }
        }
        // found an existing VertexIndexValue
        uint8_t ret = it.iv_.InsertVid(vid);
        if (it.iv_.TooLarge()) {
            VertexIndexValue right = it.iv_.SplitRightHalf();
            // remove the original VertexIndexValue
            it.it_.DeleteKey();
            // insert the left half VertexIndexValue
            bool r = table_.AddKV(txn, it.iv_.CreateKey(key), it.iv_.GetBuf());
            FMA_DBG_ASSERT(r);
            // insert the right half VertexIndexValue
            r = table_.AddKV(txn, right.CreateKey(key), right.GetBuf());
            FMA_DBG_ASSERT(r);
        } else {
            if (ret == 1) {
                // the key does not change, so just update the VertexIndexValue
                it.it_.SetValue(it.iv_.GetBuf());
            } else if (ret == 2) {
                // the key changes so we need to remove the original VertexIndexValue
                it.it_.DeleteKey();
                // then insert the VertexIndexValue with an updated key
                bool r = table_.AddKV(txn, it.iv_.CreateKey(key), it.iv_.GetBuf());
                FMA_DBG_ASSERT(r);
            } else {
                // vid already exists
                return false;
            }
        }
        return true;
    }
};

inline FieldType VertexIndexIterator::KeyType() const { return index_->KeyType(); }
}  // namespace lgraph
