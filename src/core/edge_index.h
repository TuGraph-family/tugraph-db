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

namespace lgraph {
class Transaction;

namespace _detail {
/**
 * Patch the key with the specified vid.
 *
 * \param           key     The key.
 * \param           src_vid     The src_vid.
 * \param           src_vid     The dst_vid.
 * \param           lid         The label id.
 * \param           tid         The temporal id.
 * \param           eid     The eid.
 * \return  The patched key.
 */
static Value PatchKeyWithEUid(const Value& key, VertexId src_vid, VertexId end_vid, LabelId lid,
                              TemporalId tid, EdgeId eid) {
    Value key_euid(key.Size() + EUID_SIZE);
    memcpy(key_euid.Data(), key.Data(), key.Size());
    WriteVid(key_euid.Data() + key.Size(), src_vid);
    WriteVid(key_euid.Data() + key.Size() + VID_SIZE, end_vid);
    WriteLabelId(key_euid.Data() + key.Size() + LID_BEGIN, lid);
    WriteTemporalId(key_euid.Data() + key.Size() + TID_BEGIN, tid);
    WriteEid(key_euid.Data() + key.Size() + EID_BEGIN, eid);
    return key_euid;
}

}  // namespace _detail

class EdgeIndex;
class EdgeIndexIterator;

/**
 * An EdgeIndexValue packs multiple src_vids ,dst_vids, lids, tids and eids into a single value.
 */
class EdgeIndexValue {
    /**
     * uint8_t count
     * [uint40_t vid] * count
     */

    friend class EdgeIndex;
    friend class EdgeIndexIterator;

    Value v_;

    EdgeIndexValue() : v_(1) { *(uint8_t*)v_.Data() = 0; }

    explicit EdgeIndexValue(const Value& v) : v_(v) {}

    explicit EdgeIndexValue(Value&& v) : v_(std::move(v)) {}

    template <typename ET>
    EdgeIndexValue(const ET& euid_beg, const ET& euid_end) {
        size_t n = euid_end - euid_beg;
        FMA_DBG_ASSERT(n < std::numeric_limits<uint8_t>::max());
        v_.Resize(1 + (_detail::EUID_SIZE)*n);
        char* p = v_.Data();
        *(uint8_t*)p = (uint8_t)n;

        p++;
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

    /**
     * Search for the specified edgeuid
     *
     * \param           src_vid     The src_vid.
     * \param           dst_vid     The dst_vid.
     * \param           lid         The label id.
     * \param           tid         The temporal id.
     * \param           eid         The eid.
     * \param [in,out]  found       Is this vid found?
     *
     * \return  Position where the euid is found.
     * list.
     */
    int SearchEUid(VertexId src_vid, VertexId dst_vid, LabelId lid, TemporalId tid, EdgeId eid,
                   bool& found) const {
        if (GetEUidCount() == 0) {
            found = false;
            return 0;
        }
        int beg = 0;
        int end = (int)GetEUidCount();
        while (beg < end) {
            int p = (beg + end) / 2;
            VertexId src_pivot = GetNthSrcVid(p);
            VertexId dst_pivot = GetNthDstVid(p);
            LabelId lid_pivot = GetNthLid(p);
            TemporalId tid_pivot = GetNthTid(p);
            EdgeId eid_pivot = GetNthEid(p);
            if (src_pivot == src_vid) {
                if (dst_pivot < dst_vid) {
                    end = p;
                } else if (dst_pivot > dst_vid) {
                    beg = p + 1;
                } else {
                    if (lid_pivot < lid) {
                        end = p;
                    } else if (lid_pivot > lid) {
                        beg = p + 1;
                    } else {
                        if (tid_pivot < tid) {
                            end = p;
                        } else if (tid_pivot > tid) {
                            beg = p + 1;
                        } else {
                            if (eid < eid_pivot) {
                                end = p;
                            } else if (eid > eid_pivot) {
                                beg = p + 1;
                            } else {
                                found = true;
                                return p;
                            }
                        }
                    }
                }
            }
            if (src_vid < src_pivot) {
                end = p;
            } else if (src_vid > src_pivot) {
                beg = p + 1;
            }
        }
        found = false;
        return end;
    }

    /**
     * Insert the specified euid.
     *
     * \param           src_vid     The src_vid.
     * \param           dst_vid     The dst_vid.
     * \param           lid         The label id.
     * \param           tid         The temporal id.
     * \param           eid         The edge id.
     * \return  The status of the operation.
     *
     * return value:
     *   0, euid already exists
     *   1, the key does not change
     *   2, the key changes
     */
    uint8_t InsertEUid(VertexId src_vid, VertexId dst_vid, LabelId lid, TemporalId tid,
                       EdgeId eid) {
        bool exists;
        int pos = SearchEUid(src_vid, dst_vid, lid, tid, eid, exists);
        if (exists) {
            return 0;
        }
        bool tail = (pos == GetEUidCount());
        InsertEUidAtNthPos(pos, src_vid, dst_vid, lid, tid, eid);
        return !tail ? 1 : 2;
    }

    void InsertEUidAtNthPos(int pos, VertexId src_vid, VertexId dst_vid, LabelId lid,
                            TemporalId tid, EdgeId eid) {
        // insert the vid
        size_t bytes_after_p = v_.Size() - (1 + (_detail::EUID_SIZE)*pos);
        v_.Resize(v_.Size() + _detail::EUID_SIZE);
        char* ptr = v_.Data() + 1 + (_detail::EUID_SIZE)*pos;
        memmove(ptr + (_detail::EUID_SIZE), ptr, bytes_after_p);
        _detail::WriteVid(ptr, src_vid);
        _detail::WriteVid(ptr + _detail::VID_SIZE, dst_vid);
        _detail::WriteLabelId(ptr + _detail::LID_BEGIN, lid);
        _detail::WriteTemporalId(ptr + _detail::TID_BEGIN, tid);
        _detail::WriteEid(ptr + _detail::EID_BEGIN, eid);
        (*(uint8_t*)v_.Data())++;
    }

    /**
     * Delete the specified euid.
     *
     * \param           src_vid     The vid.
     * \param           dst_vid     The dst_vid.
     * \param           lid         The label id.
     * \param           tid         The temporal id.
     * \param           eid         The edge id.
     * \return  The status of the operation.
     *
     * return value:
     *   0, euid does not exist
     *   1, the key does not change
     *   2, the key changes
     */
    uint8_t DeleteEUidIfExist(VertexId src_vid, VertexId dst_vid, LabelId lid, TemporalId tid,
                              EdgeId eid) {
        bool exists;
        int pos = SearchEUid(src_vid, dst_vid, lid, tid, eid, exists);
        if (!exists) {
            return 0;
        }
        bool tail = (pos == GetEUidCount() - 1);
        DeleteNthEUid(pos);
        return !tail ? 1 : 2;
    }

    void DeleteNthEUid(int pos) {
        v_.Resize(v_.Size());
        char* ptr = v_.Data() + 1 + (_detail::EUID_SIZE)*pos;
        uint8_t n = GetEUidCount();
        memmove(ptr, ptr + (_detail::EUID_SIZE), (n - pos - 1) * (_detail::EUID_SIZE));
        v_.Resize(v_.Size() - (_detail::EUID_SIZE));
        *(uint8_t*)v_.Data() = n - 1;
    }

    bool TooLarge() { return v_.Size() > _detail::NODE_SPLIT_THRESHOLD; }

    EdgeIndexValue SplitRightHalf() {
        int n = GetEUidCount();
        int next_half_size = n / 2;
        int this_half_size = n - next_half_size;
        EdgeIndexValue newnode;
        newnode.v_.Resize(1 + next_half_size * (_detail::EUID_SIZE));
        char* p = newnode.v_.Data();
        *(uint8_t*)p = next_half_size;
        memcpy(p + 1, v_.Data() + 1 + this_half_size * (_detail::EUID_SIZE),
               next_half_size * (_detail::EUID_SIZE));
        v_.Resize(v_.Size() - next_half_size * (_detail::EUID_SIZE));
        *(uint8_t*)v_.Data() = this_half_size;
        return newnode;
    }

    const Value& GetBuf() const { return v_; }

    Value& GetBuf() { return v_; }

    /**
     * Patch the key with the last euid in this EdgeIndexValue.
     */
    Value CreateKey(const Value& key) const {
        int pos = GetEUidCount() - 1;
        Value v(key.Size() + (_detail::EUID_SIZE));
        memcpy(v.Data(), key.Data(), key.Size());
        memcpy(v.Data() + key.Size(), v_.Data() + 1 + pos * (_detail::EUID_SIZE),
               (_detail::EUID_SIZE));
        return v;
    }
};

/**
 * The iterator for indices
 */
class EdgeIndexIterator : public ::lgraph::IteratorBase {
    friend class EdgeIndex;
    EdgeIndex* index_;
    KvIterator it_;
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
    bool unique_;

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
     * \param           unique      Whether the index is a unique index.
     */
    EdgeIndexIterator(EdgeIndex* idx, Transaction* txn, KvTable& table, const Value& key_start,
                      const Value& key_end, VertexId src_vid, VertexId dst_vid, LabelId lid,
                      TemporalId tid, EdgeId eid, bool unique);

    EdgeIndexIterator(EdgeIndex* idx, KvTransaction* txn, KvTable& table, const Value& key_start,
                      const Value& key_end, VertexId src_vid, VertexId dst_vid, LabelId lid,
                      TemporalId tid, EdgeId eid, bool unique);

    bool KeyOutOfRange() {
        if (key_end_.Empty() || (!unique_ && key_end_.Size() == _detail::EUID_SIZE)) return false;
        return it_.GetTable().CompareKey(it_.GetTxn(), it_.GetKey(), key_end_) > 0;
    }

    /**
     * Traverse to the previous key/value pair. Used when an inserted euid is
     * greater than all existing euids. Should be used together with KeyEquals.
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
            auto key_euid = it_.GetKey();
            if (key_euid.Size() - _detail::EUID_SIZE != key.Size()) {
                return false;
            }
            return memcmp(key.Data(), key_euid.Data(), key.Size()) == 0;
        }
    }

    /** Loads content from iterator, assuming iterator is already at the right
     * position. */
    void LoadContentFromIt() {
        valid_ = true;
        pos_ = 0;
        curr_key_.Copy(GetKey());
        if (!unique_) {
            iv_ = EdgeIndexValue(it_.GetValue());
            src_vid_ = iv_.GetNthSrcVid(pos_);
            dst_vid_ = iv_.GetNthDstVid(pos_);
            lid_ = iv_.GetNthLid(pos_);
            tid_ = iv_.GetNthTid(pos_);
            eid_ = iv_.GetNthEid(pos_);
        } else {
            src_vid_ = _detail::GetVid(it_.GetValue().Data());
            dst_vid_ = _detail::GetVid(it_.GetValue().Data() + _detail::VID_SIZE);
            lid_ = _detail::GetLabelId(it_.GetValue().Data() + _detail::LID_BEGIN);
            tid_ = _detail::GetTemporalId(it_.GetValue().Data() + _detail::TID_BEGIN);
            eid_ = _detail::GetEid(it_.GetValue().Data() + _detail::EID_BEGIN);
        }
    }

    DISABLE_COPY(EdgeIndexIterator);
    EdgeIndexIterator& operator=(EdgeIndexIterator&&) = delete;

 protected:
    void CloseImpl() override;

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
    bool Next() {
        // if we haven't reach the end of the current EdgeIndexValue,
        // just move forward
        if (!unique_ && pos_ < iv_.GetEUidCount() - 1) {
            pos_ += 1;
            src_vid_ = iv_.GetNthSrcVid(pos_);
            dst_vid_ = iv_.GetNthDstVid(pos_);
            lid_ = iv_.GetNthLid(pos_);
            tid_ = iv_.GetNthTid(pos_);
            eid_ = iv_.GetNthEid(pos_);
            return true;
        }
        // otherwise try moving to the next EdgeIndex Value,
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
            Value key_euid = it_.GetKey();
            return _detail::ReturnKeyEvenIfLong(
                Value(key_euid.Data(), key_euid.Size() - _detail::EUID_SIZE));
        }
    }

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
    void RefreshContentIfKvIteratorModified() override {
        if (IsValid() && it_.IsValid() && it_.UnderlyingPointerModified()) {
            valid_ = false;
            // now it_ points to a valid position, but not necessary the right one
            it_.GotoClosestKey(unique_ ? curr_key_
                                       : _detail::PatchKeyWithEUid(curr_key_, src_vid_, dst_vid_,
                                                                   lid_, tid_, eid_));
            FMA_DBG_ASSERT(it_.IsValid());
            if (KeyOutOfRange()) return;
            if (unique_) {
                // found the right key
                LoadContentFromIt();
                return;
            } else {
                // non-unique, need to find correct pos_
                iv_ = EdgeIndexValue(it_.GetValue());
                pos_ = iv_.SearchEUid(src_vid_, dst_vid_, lid_, tid_, eid_, valid_);
                if (pos_ >= iv_.GetEUidCount()) return;
                valid_ = true;
                curr_key_.Copy(GetKey());
                src_vid_ = iv_.GetNthSrcVid(pos_);
                dst_vid_ = iv_.GetNthDstVid(pos_);
                lid_ = iv_.GetNthLid(pos_);
                tid_ = iv_.GetNthTid(pos_);
                eid_ = iv_.GetNthEid(pos_);
                return;
            }
        }
    }
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

    KvTable table_;
    FieldType key_type_;
    std::atomic<bool> ready_;
    std::atomic<bool> disabled_;
    bool unique_;

 public:
    EdgeIndex(const KvTable& table, FieldType key_type, bool unique)
        : table_(table), key_type_(key_type), ready_(false), disabled_(false), unique_(unique) {}

    EdgeIndex(const EdgeIndex& rhs)
        : table_(rhs.table_),
          key_type_(rhs.key_type_),
          ready_(rhs.ready_.load()),
          disabled_(rhs.disabled_.load()),
          unique_(rhs.unique_) {}

    EdgeIndex(EdgeIndex&& rhs) = delete;

    EdgeIndex& operator=(const EdgeIndex& rhs) = delete;

    EdgeIndex& operator=(EdgeIndex&& rhs) = delete;

    static KvTable OpenTable(KvTransaction& txn, KvStore& store, const std::string& name,
                             FieldType dt, bool unique) {
        ComparatorDesc desc;
        desc.comp_type = unique ? ComparatorDesc::SINGLE_TYPE : ComparatorDesc::TYPE_AND_EUID;
        desc.data_type = dt;
        return store.OpenTable(txn, name, true, desc);
    }

    void _AppendIndexEntry(KvTransaction& txn, const Value& k, EdgeUid euid) {
        FMA_DBG_ASSERT(unique_);
        FMA_DBG_CHECK_LE(k.Size(), _detail::MAX_KEY_SIZE);
        Value v(_detail::EUID_SIZE);
        _detail::WriteVid(v.Data(), euid.src);
        _detail::WriteVid(v.Data() + _detail::VID_SIZE, euid.dst);
        _detail::WriteLabelId(v.Data() + _detail::LID_BEGIN, euid.lid);
        _detail::WriteTemporalId(v.Data() + _detail::TID_BEGIN, euid.tid);
        _detail::WriteEid(v.Data() + _detail::EID_BEGIN, euid.eid);
        table_.AppendKv(txn, k, v);
    }

    /**
     * Append an entry to non-unique index.
     *
     * \param   txn     The transaction.
     * \param   k       The key, raw format.
     * \param   euids    All the euids corresponding to k. The euids must be sorted.
     */
    void _AppendNonUniqueIndexEntry(KvTransaction& txn, const Value& k,
                                    const std::vector<EdgeUid>& euids) {
        FMA_DBG_ASSERT(!unique_);
        FMA_DBG_ASSERT(!euids.empty());
        FMA_DBG_CHECK_LE(k.Size(), _detail::MAX_KEY_SIZE);
        size_t euid_per_idv = _detail::NODE_SPLIT_THRESHOLD / _detail::EUID_SIZE;
        for (size_t i = 0; i < euids.size(); i += euid_per_idv) {
            size_t end = i + euid_per_idv;
            end = end <= euids.size() ? end : euids.size();
            EdgeIndexValue idv(euids.begin() + i, euids.begin() + end);
            Value key = idv.CreateKey(k);
            table_.AppendKv(txn, key, idv.GetBuf());
        }
    }

#define _GET_UNM_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(ft)                                       \
    do {                                                                                        \
        auto d = field_data_helper::MinStoreValue<FieldType::ft>();                             \
        return EdgeIndexIterator(this, &txn, table_, Value::ConstRef(d),                        \
                                 std::forward<V2>(key_end), vid, vid2, lid, tid, eid, unique_); \
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
                    return EdgeIndexIterator(this, &txn, table_, std::forward<V1>(key_start),
                                             _detail::CutKeyIfLong(std::forward<V2>(key_end)), vid,
                                             vid2, lid, tid, eid, unique_);
                    break;
                }
            case FieldType::BLOB:
                FMA_ASSERT(false) << "Blob fields must not be indexed.";
            default:
                FMA_ASSERT(false);
                return EdgeIndexIterator(this, &txn, table_, std::forward<V1>(key_start),
                                         std::forward<V2>(key_end), vid, vid2, lid, tid, eid,
                                         unique_);
            }
        } else {
            return EdgeIndexIterator(this, &txn, table_,
                                     _detail::CutKeyIfLong(std::forward<V1>(key_start)),
                                     _detail::CutKeyIfLong(std::forward<V2>(key_end)), vid, vid2,
                                     lid, tid, eid, unique_);
        }
    }

#define _GET_EDGE_INDEX_ITER_WITH_EMPTY_START_KEY(ft)                                              \
    do {                                                                                           \
        auto d = field_data_helper::MinStoreValue<FieldType::ft>();                                \
        return EdgeIndexIterator(this, txn, table_, Value::ConstRef(d), std::forward<V2>(key_end), \
                                 vid, vid2, lid, tid, eid, unique_);                               \
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
                    return EdgeIndexIterator(this, txn, table_, std::forward<V1>(key_start),
                                             _detail::CutKeyIfLong(std::forward<V2>(key_end)), vid,
                                             vid2, lid, tid, eid, unique_);
                    break;
                }
            case FieldType::BLOB:
                FMA_ASSERT(false) << "Blob fields must not be indexed.";
            default:
                FMA_ASSERT(false);
                return EdgeIndexIterator(this, txn, table_, std::forward<V1>(key_start),
                                         std::forward<V2>(key_end), vid, vid2, lid, tid, eid,
                                         unique_);
            }
        } else {
            return EdgeIndexIterator(this, txn, table_,
                                     _detail::CutKeyIfLong(std::forward<V1>(key_start)),
                                     _detail::CutKeyIfLong(std::forward<V2>(key_end)), vid, vid2,
                                     lid, tid, eid, unique_);
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
            EdgeIndexValue iv(it.GetValue());
            std::string line = key_to_string(k.Data(), k.Size() - _detail::EUID_SIZE);
            line.append(" -> ");
            if (unique_) {
                line.append(std::to_string(_detail::GetVid(iv.GetBuf().Data())));
                line.append(
                    std::to_string(_detail::GetVid(iv.GetBuf().Data() + _detail::VID_SIZE)));
                line.append(
                    std::to_string(_detail::GetLabelId(iv.GetBuf().Data() + _detail::LID_BEGIN)));
                line.append(std::to_string(
                    _detail::GetTemporalId(iv.GetBuf().Data() + _detail::TID_BEGIN)));
                line.append(
                    std::to_string(_detail::GetEid(iv.GetBuf().Data() + _detail::EID_BEGIN)));
            } else {
                for (int i = 0; i < iv.GetEUidCount(); i++) {
                    line.append(std::to_string(iv.GetNthSrcVid(i)));
                    line.append(std::to_string(iv.GetNthDstVid(i)));
                    line.append(std::to_string(iv.GetNthLid(i)));
                    line.append(std::to_string(iv.GetNthTid(i)));
                    line.append(std::to_string(iv.GetNthEid(i)));
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
     * \param           src_vid The src vid.
     * \param           dst_vid The dst vid.
     * \param           lid The label id.
     * \param           tid the temporal id.
     * \param           eid The eid.
     * \return  Whether the operation succeeds or not.
     */
    bool Delete(KvTransaction& txn, const Value& k, VertexId src_vid, VertexId dst_vid, LabelId lid,
                TemporalId tid, EdgeId eid) {
        Value key = _detail::CutKeyIfLong(k);
        EdgeIndexIterator it = GetUnmanagedIterator(txn, key, key, src_vid, dst_vid, lid, tid, eid);
        if (!it.IsValid() || it.KeyOutOfRange()) {
            // no such key_vid
            return false;
        }
        if (unique_) {
            it.it_.DeleteKey();
            return true;
        } else {
            uint8_t ret = it.iv_.DeleteEUidIfExist(src_vid, dst_vid, lid, tid, eid);
            if (ret == 1) {
                // the key does not change after deletion
                it.it_.SetValue(it.iv_.GetBuf());
            } else if (ret == 2) {
                // the key changes after deletion
                it.it_.DeleteKey();
                if (it.iv_.GetEUidCount()) {
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
     * \exception   IndexException  Thrown when an EdgeIndex error condition occurs.
     *
     * \param [in,out]  txn     The transaction.
     * \param           old_key The old key.
     * \param           new_key The new key.
     * \param           src_vid The src vid.
     * \param           dst_vid The dst vid.
     * \param           lid     the label id.
     * \param           tid     the temporal id.
     * \param           eid     The eid.
     * \return  Whether the operation succeeds or not.
     */
    bool Update(KvTransaction& txn, const Value& old_key, const Value& new_key, VertexId src_vid,
                VertexId dst_vid, LabelId lid, TemporalId tid, EdgeId eid) {
        if (!Delete(txn, old_key, src_vid, dst_vid, lid, tid, eid)) {
            return false;
        }
        return Add(txn, new_key, src_vid, dst_vid, lid, tid, eid);
    }

    /**
     * Add a specified vertex under a specified key.
     *
     * \exception   IndexException  Thrown when an EdgeIndex error condition occurs.
     *
     * \param [in,out]  txn The transaction.
     * \param           key The key.
     * \param           src_vid The vid.
     * \param           dst_vid     The dst_vid.
     * \param           lid     The label id.
     * \param           tid     The temporal id.
     * \param           eid     The eid.
     * \return  Whether the operation succeeds or not.
     */
    bool Add(KvTransaction& txn, const Value& k, VertexId src_vid, VertexId dst_vid, LabelId lid,
             TemporalId tid, EdgeId eid, bool throw_on_long_key = true) {
        if (k.Size() > _detail::MAX_KEY_SIZE && throw_on_long_key) {
            throw InputError("EdgeIndex key size too large, must be less than 480.");
        }
        Value key = _detail::CutKeyIfLong(k);
        if (unique_) {
            Value v(_detail::EUID_SIZE);
            _detail::WriteVid(v.Data(), src_vid);
            _detail::WriteVid(v.Data() + _detail::VID_SIZE, dst_vid);
            _detail::WriteLabelId(v.Data() + _detail::LID_BEGIN, lid);
            _detail::WriteTemporalId(v.Data() + _detail::TID_BEGIN, tid);
            _detail::WriteEid(v.Data() + _detail::EID_BEGIN, eid);
            return table_.AddKV(txn, key, v);
        }
        EdgeIndexIterator it = GetUnmanagedIterator(txn, key, key, src_vid, dst_vid, lid, tid, eid);
        if (!it.IsValid() || it.KeyOutOfRange()) {
            if (!it.PrevKV() || !it.KeyEquals(key)) {
                // create a new EdgeIndexValue
                EdgeIndexValue iv;
                uint8_t r = iv.InsertEUid(src_vid, dst_vid, lid, tid, eid);
                FMA_DBG_CHECK_NEQ(r, 0);
                bool r2 = table_.AddKV(txn, iv.CreateKey(key), iv.GetBuf());
                FMA_DBG_ASSERT(r2);
                return true;
            }
        }
        uint8_t ret = it.iv_.InsertEUid(src_vid, dst_vid, lid, tid, eid);
        if (it.iv_.TooLarge()) {
            EdgeIndexValue right = it.iv_.SplitRightHalf();
            // remove the original EdgeIndexValue
            it.it_.DeleteKey();
            // insert the left half EdgeIndexValue
            bool r = table_.AddKV(txn, it.iv_.CreateKey(key), it.iv_.GetBuf());
            FMA_DBG_ASSERT(r);
            // insert the right half EdgeIndexValue
            r = table_.AddKV(txn, right.CreateKey(key), right.GetBuf());
            FMA_DBG_ASSERT(r);
        } else {
            if (ret == 1) {
                // the key does not change, so just update the EdgeIndexValue
                it.it_.SetValue(it.iv_.GetBuf());
            } else if (ret == 2) {
                // the key changes so we need to remove the original EdgeIndexValue
                it.it_.DeleteKey();
                // then insert the EdgeIndexValue with an updated key
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

inline FieldType EdgeIndexIterator::KeyType() const { return index_->KeyType(); }
}  // namespace lgraph

