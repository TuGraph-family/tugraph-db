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
#include "core/type_convert.h"
#include "core/value.h"
#include "core/vertex_index.h"

int TestEdgeIndexImpl();
int TestEdgeIndexValue();
int TestEdgeIndexCRUD();
int CURDEdgeWithTooLongKey();
int TestERefreshContentIfKvIteratorModified();

namespace lgraph {
class Transaction;
class EdgeIndex;
class EdgeIndexIterator;

/**
 * An EdgeIndexValue packs multiple src_vids, dst_vids, lids, tids and eids into a single value.
 */
class EdgeIndexValue {
    /**
     * uint8_t count
     * [24 bytes euid] * count
     */

    friend class EdgeIndex;
    friend class EdgeIndexIterator;
    friend int ::TestEdgeIndexValue();

    Value v_;
    EdgeIndexValue();
    explicit EdgeIndexValue(const Value& v);
    explicit EdgeIndexValue(Value&& v);

    template <typename ET>
    EdgeIndexValue(const ET& euid_beg, const ET& euid_end) {
        size_t n = euid_end - euid_beg;
        FMA_DBG_ASSERT(n < std::numeric_limits<uint8_t>::max());
        v_.Resize(1 + (_detail::EUID_SIZE)*n);
        char* p = v_.Data();
        *(uint8_t*)p = (uint8_t)n;
        p++;
        // TODO(jzj) will be reconfigured soon，compress the saved euid
        for (auto et = euid_beg; et < euid_end; et++) {
            auto src_vid = (*et).src;
            auto dst_vid = (*et).dst;
            auto lid = (*et).lid;
            auto tid = (*et).tid;
            auto eid = (*et).eid;
            _detail::WriteVid(p, src_vid);
            _detail::WriteVid(p + _detail::VID_SIZE, dst_vid);
            _detail::WriteLabelId(p + _detail::LID_BEGIN, lid);
            _detail::WriteTemporalId(p + _detail::TID_BEGIN, tid);
            _detail::WriteEid(p + _detail::EID_BEGIN, eid);
            p += _detail::EUID_SIZE;
        }
    }

    int GetEUidCount() const { return static_cast<int>(*(uint8_t*)v_.Data()); }

    VertexId GetNthSrcVid(int n) const {
        return _detail::GetVid(v_.Data() + 1 + (_detail::EUID_SIZE)*n);
    }

    VertexId GetNthDstVid(int n) const {
        return _detail::GetVid(v_.Data() + 1 + (_detail::EUID_SIZE)*n + _detail::VID_SIZE);
    }

    LabelId GetNthLid(int n) const {
        return _detail::GetLabelId(v_.Data() + 1 + (_detail::EUID_SIZE)*n + _detail::LID_BEGIN);
    }

    TemporalId GetNthTid(int n) const {
        return _detail::GetTemporalId(v_.Data() + 1 + (_detail::EUID_SIZE)*n + _detail::TID_BEGIN);
    }

    EdgeId GetNthEid(int n) const {
        return _detail::GetEid(v_.Data() + 1 + (_detail::EUID_SIZE)*n + _detail::EID_BEGIN);
    }

    EdgeUid GetNthEuid(int n) const {
        return EdgeUid(GetNthSrcVid(n), GetNthDstVid(n), GetNthLid(n), GetNthTid(n), GetNthEid(n));
    }

    /**
     * Search for the specified edgeuid
     *
     * \param           euid        The Edge unique id.
     * \param [in,out]  found       Is this euid found?
     *
     * \return  Position where the euid is found.
     * list.
     */
    int SearchEUid(const EdgeUid& euid, bool& found) const;

    /**
     * Insert the specified euid.
     *
     * \param           euid        The Edge unique id.
     * \return  The status of the operation.
     *
     * return value:
     *   0, euid already exists
     *   1, the key does not change
     *   2, the key changes
     */
    uint8_t InsertEUid(const EdgeUid& euid);

    void InsertEUidAtNthPos(int pos, const EdgeUid& euid);

    /**
     * Delete the specified euid.
     *
     * \param           euid        The Edge unique id.
     * \return  The status of the operation.
     *
     * return value:
     *   0, euid does not exist
     *   1, the key does not change
     *   2, the key changes
     */
    uint8_t DeleteEUidIfExist(const EdgeUid& euid);

    void DeleteNthEUid(int pos);

    bool TooLarge() {
        return v_.Size() > _detail::NODE_SPLIT_THRESHOLD;
    }

    EdgeIndexValue SplitRightHalf();

    const Value& GetBuf() const { return v_; }

    Value& GetBuf() { return v_; }
    /**
     * Patch the key with the last euid in this EdgeIndexValue.
     */
    Value CreateKey(const Value& key) const;
};

/**
 * The iterator for indices
 */
class EdgeIndexIterator : public ::lgraph::IteratorBase {
    friend class EdgeIndex;
    EdgeIndex* index_;
    std::unique_ptr<KvIterator> it_;
    Value key_end_;
    Value curr_key_;     // current indexed key, excluding vid
    EdgeIndexValue iv_;  // EdgeIndexValue, if this is non-unique index
    bool valid_;
    int pos_;
    VertexId src_vid_;  // src vid
    VertexId dst_vid_;  // dst vid
    LabelId lid_;
    TemporalId tid_;  // TOOD(heng):
    EdgeId eid_;
    IndexType type_;

    /**
     * The constructor for EdgeIndexIterator
     *
     * \param [in,out]  idx         The EdgeIndex object this iterator will point to.
     * \param [in,out]  txn         The transaction.
     * \param [in,out]  table       The index table.
     * \param           key_start   The start key.
     * \param           key_end     The end key.
     * \param           src_vid     The src vid from which to start searching.
     * \param           dst_vid     The dst vid from which to start searching.
     * \param           lid         The label id form which to start searching.
     * \param           tid         The Temporal id form which to start searching.
     * \param           eid         The eid from which to start searching.
     * \param           type        Index type.
     */
    EdgeIndexIterator(EdgeIndex* idx, Transaction* txn, KvTable& table, const Value& key_start,
                      const Value& key_end, VertexId src_vid, VertexId dst_vid, LabelId lid,
                      TemporalId tid, EdgeId eid, IndexType type);

    EdgeIndexIterator(EdgeIndex* idx, KvTransaction* txn, KvTable& table, const Value& key_start,
                      const Value& key_end, VertexId src_vid, VertexId dst_vid, LabelId lid,
                      TemporalId tid, EdgeId eid, IndexType type);
    bool KeyOutOfRange();


    /**
     * Traverse to the previous key/value pair. Used when an inserted euid is
     * greater than all existing euids. Should be used together with KeyEquals.
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
    bool KeyEquals(Value& key);

    /** Loads content from iterator, assuming iterator is already at the right
     * position. */
    void LoadContentFromIt();

 protected:
    void CloseImpl() override;
    DISABLE_COPY(EdgeIndexIterator);
    EdgeIndexIterator& operator=(EdgeIndexIterator&&) = delete;

 public:
    EdgeIndexIterator(EdgeIndexIterator&& rhs);

    /**
     * Query if this iterator is valid, i.e. the Key and euid can be queried.
     *
     * \return  True if valid, false if not.
     */
    bool IsValid() const { return valid_; }

    /**
     * Move to the next edge unique id in the list, which consists of all the valid
     * edge unique ids of the iterator and is sorted from small to large.
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

    VertexId GetPairUniqueSrcVertexId();

    VertexId GetPairUniqueDstVertexId();

    FieldData GetKeyData() const {
        return field_data_helper::ValueToFieldData(GetKey(), KeyType());
    }

    /**
     * Gets the current edge unique id.
     *
     * \return  The current edge unique id.
     */
    VertexId GetSrcVid() const { return src_vid_; }

    VertexId GetDstVid() const { return dst_vid_; }

    LabelId GetLabelId() const { return lid_; }

    TemporalId GetTemporalId() const { return tid_; }

    EdgeId GetEid() const { return eid_; }

    EdgeUid GetUid() const { return EdgeUid(src_vid_, dst_vid_, lid_, tid_, eid_); }

    FieldType KeyType() const;

    /**
     * Determines if we can refresh content if kv iterator modified
     *
     * \return  True if KvIterator was modified.
     */
    void RefreshContentIfKvIteratorModified() override;
};

class Schema;

/**
 * The indices
 */
class EdgeIndex {
    friend class Schema;
    friend class IndexManager;
    friend class LightningGraph;
    friend class Transaction;
    friend int ::TestEdgeIndexImpl();
    friend int ::TestEdgeIndexCRUD();
    friend int ::CURDEdgeWithTooLongKey();
    friend int ::TestERefreshContentIfKvIteratorModified();

    std::shared_ptr<KvTable> table_;
    FieldType key_type_;
    std::atomic<bool> ready_;
    std::atomic<bool> disabled_;
    IndexType type_;

 public:
    EdgeIndex(std::shared_ptr<KvTable> table, FieldType key_type, IndexType type);

    EdgeIndex(const EdgeIndex& rhs);

    EdgeIndex(EdgeIndex&& rhs) = delete;

    EdgeIndex& operator=(const EdgeIndex& rhs) = delete;

    EdgeIndex& operator=(EdgeIndex&& rhs) = delete;

    static std::unique_ptr<KvTable> OpenTable(KvTransaction& txn, KvStore& store,
                                              const std::string& name, FieldType dt,
                                              IndexType type);

    void _AppendIndexEntry(KvTransaction& txn, const Value& k, EdgeUid euid);

    /**
     * Append an entry to non-unique index.
     *
     * \param   txn     The transaction.
     * \param   k       The key, raw format.
     * \param   euids    All the euids corresponding to k. The euids must be sorted.
     */
    void _AppendNonUniqueIndexEntry(KvTransaction& txn, const Value& k,
                                    const std::vector<EdgeUid>& euids);

#define _GET_UNM_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(ft)                                     \
    do {                                                                                      \
        auto d = field_data_helper::MinStoreValue<FieldType::ft>();                           \
        return EdgeIndexIterator(this, &txn, *table_, Value::ConstRef(d),                     \
                                 std::forward<V2>(key_end), vid, vid2, lid, tid, eid, type_); \
    } while (0)

    /**
     * Get an index iterator.
     *
     * \param [in,out]  txn         The transaction.
     * \param           key_start   The start key.
     * \param           key_end     The end key.
     * \param           vid         The vid to start from.
     * \param           vid2         The dst_vid to start from.
     * \param           lid         The label id to start from.
     * \param           tid         The temporal id to start from.
     * \param           eid         The eid to start from.
     * \return  The index iterator.
     */
    template <typename V1, typename V2>
    EdgeIndexIterator GetUnmanagedIterator(KvTransaction& txn, V1&& key_start, V2&& key_end,
                                           VertexId vid = 0, VertexId vid2 = 0, LabelId lid = 0,
                                           TemporalId tid = 0, EdgeId eid = 0) {
        if (key_start.Empty()) {
            switch (key_type_) {
            case FieldType::BOOL:
                _GET_UNM_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(BOOL);
            case FieldType::INT8:
                _GET_UNM_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(INT8);
            case FieldType::INT16:
                _GET_UNM_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(INT16);
            case FieldType::INT32:
                _GET_UNM_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(INT32);
            case FieldType::INT64:
                _GET_UNM_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(INT64);
            case FieldType::DATE:
                _GET_UNM_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(DATE);
            case FieldType::DATETIME:
                _GET_UNM_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(DATETIME);
            case FieldType::FLOAT:
                _GET_UNM_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(FLOAT);
            case FieldType::DOUBLE:
                _GET_UNM_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(DOUBLE);
            case FieldType::STRING:
                {
                    return EdgeIndexIterator(this, &txn, *table_, std::forward<V1>(key_start),
                           CutKeyIfLongOnlyForNonUniqueIndex(std::forward<V2>(key_end)), vid,
                           vid2, lid, tid, eid, type_);
                }
            case FieldType::BLOB:
                FMA_ASSERT(false) << "Blob fields must not be indexed.";
            default:
                FMA_ASSERT(false);
                return EdgeIndexIterator(this, &txn, *table_, std::forward<V1>(key_start),
                                         std::forward<V2>(key_end), vid, vid2, lid, tid, eid,
                                         type_);
            }
        } else {
            return EdgeIndexIterator(
                this, &txn, *table_, CutKeyIfLongOnlyForNonUniqueIndex(std::forward<V1>(key_start)),
                CutKeyIfLongOnlyForNonUniqueIndex(std::forward<V2>(key_end)),
                vid, vid2, lid, tid, eid, type_);
        }
    }

#define _GET_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(ft)                                         \
    do {                                                                                      \
        auto d = field_data_helper::MinStoreValue<FieldType::ft>();                           \
        return EdgeIndexIterator(this, txn, *table_, Value::ConstRef(d),                      \
                                 std::forward<V2>(key_end), vid, vid2, lid, tid, eid, type_); \
    } while (0)

    template <typename V1, typename V2>
    EdgeIndexIterator GetIterator(Transaction* txn, V1&& key_start, V2&& key_end, VertexId vid = 0,
                                  VertexId vid2 = 0, LabelId lid = 0, TemporalId tid = 0,
                                  EdgeId eid = 0) {
        if (key_start.Empty()) {
            switch (key_type_) {
            case FieldType::BOOL:
                _GET_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(BOOL);
            case FieldType::INT8:
                _GET_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(INT8);
            case FieldType::INT16:
                _GET_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(INT16);
            case FieldType::INT32:
                _GET_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(INT32);
            case FieldType::INT64:
                _GET_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(INT64);
            case FieldType::DATE:
                _GET_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(DATE);
            case FieldType::DATETIME:
                _GET_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(DATETIME);
            case FieldType::FLOAT:
                _GET_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(FLOAT);
            case FieldType::DOUBLE:
                _GET_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(DOUBLE);
            case FieldType::STRING:
                {
                    return EdgeIndexIterator(this, txn, *table_, std::forward<V1>(key_start),
                           CutKeyIfLongOnlyForNonUniqueIndex(std::forward<V2>(key_end)), vid,
                           vid2, lid, tid, eid, type_);
                    break;
                }
            case FieldType::BLOB:
                FMA_ASSERT(false) << "Blob fields must not be indexed.";
            default:
                FMA_ASSERT(false);
                return EdgeIndexIterator(this, txn, *table_, std::forward<V1>(key_start),
                                         std::forward<V2>(key_end), vid, vid2, lid, tid, eid,
                                         type_);
            }
        } else {
            return EdgeIndexIterator(
                this, txn, *table_, CutKeyIfLongOnlyForNonUniqueIndex(std::forward<V1>(key_start)),
                CutKeyIfLongOnlyForNonUniqueIndex(std::forward<V2>(key_end)),
                vid, vid2, lid, tid, eid, type_);
        }
    }

    bool IsReady() const {
        return ready_.load(std::memory_order_acquire) && !disabled_.load(std::memory_order_acquire);
    }

    bool IsUnique() const {
        return type_ == IndexType::GlobalUniqueIndex;
    }
    bool IsPairUnique() const {
        return type_ == IndexType::PairUniqueIndex;
    }
    IndexType GetType() const { return type_; }

    bool IsGlobal() const { return type_ == IndexType::GlobalUniqueIndex; }

    FieldType KeyType() { return key_type_; }

    void Dump(KvTransaction& txn,
              const std::function<std::string(const char* p, size_t s)>& key_to_string,
              std::string& str);

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
     * \param [in,out]  txn  The transaction.
     * \param           k    The key.
     * \param           euid The Edge unique id.
     * \return  Whether the operation succeeds or not.
     */
    bool Delete(KvTransaction& txn, const Value& k, const EdgeUid& euid);

    /**
     * Move a specified vertex from an old key to a new key.
     *
     * \exception   IndexException  Thrown when an EdgeIndex error condition occurs.
     *
     * \param [in,out]  txn     The transaction.
     * \param           old_key The old key.
     * \param           new_key The new key.
     * \param           euid    The Edge unique id.
     * \return  Whether the operation succeeds or not.
     */
    bool Update(KvTransaction& txn, const Value& old_key, const Value& new_key,
                const EdgeUid& euid);

    /**
     * Add a specified vertex under a specified key.
     *
     * \exception   IndexException  Thrown when an EdgeIndex error condition occurs.
     *
     * \param [in,out]  txn  The transaction.
     * \param           k    The key.
     * \param           euid The Edge unique id.
     * \return  Whether the operation succeeds or not.
     */
    bool Add(KvTransaction& txn, const Value& k, const EdgeUid& euid);

    bool UniqueIndexConflict(KvTransaction& txn, const Value& k);

    size_t GetMaxEdgeIndexKeySize();

    Value CutKeyIfLongOnlyForNonUniqueIndex(const Value& k);
};

}  // namespace lgraph
