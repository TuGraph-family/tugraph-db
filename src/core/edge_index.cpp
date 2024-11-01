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

#include "core/edge_index.h"
#include "core/transaction.h"

namespace lgraph {

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
static Value PatchNonuniqueIndexKey(const Value& key, VertexId src_vid, VertexId end_vid,
                                    LabelId lid, TemporalId tid, EdgeId eid) {
    Value key_euid(key.Size() + EUID_SIZE);
    memcpy(key_euid.Data(), key.Data(), key.Size());
    WriteVid(key_euid.Data() + key.Size(), src_vid);
    WriteVid(key_euid.Data() + key.Size() + VID_SIZE, end_vid);
    WriteLabelId(key_euid.Data() + key.Size() + LID_BEGIN, lid);
    WriteTemporalId(key_euid.Data() + key.Size() + TID_BEGIN, tid);
    WriteEid(key_euid.Data() + key.Size() + EID_BEGIN, eid);
    return key_euid;
}

static Value PatchPairUniqueIndexKey(const Value& key, VertexId src_vid, VertexId end_vid) {
    Value key_euid(key.Size() + VID_SIZE * 2);
    memcpy(key_euid.Data(), key.Data(), key.Size());
    WriteVid(key_euid.Data() + key.Size(), src_vid);
    WriteVid(key_euid.Data() + key.Size() + VID_SIZE, end_vid);
    return key_euid;
}

std::unique_ptr<KvIterator> InitEdgeIndexIterator(KvTransaction& txn, KvTable& table,
                                                  const Value& key, VertexId vid, VertexId vid2,
                                                  LabelId lid, TemporalId tid, EdgeId eid,
                                                  IndexType type) {
    switch (type) {
    case IndexType::GlobalUniqueIndex:
        return table.GetClosestIterator(txn, key);
    case IndexType::PairUniqueIndex:
        return table.GetClosestIterator(txn, _detail::PatchPairUniqueIndexKey(key, vid, vid2));
    case IndexType::NonuniqueIndex:
        return table.GetClosestIterator(
            txn, _detail::PatchNonuniqueIndexKey(key, vid, vid2, lid, tid, eid));
    }
    return std::unique_ptr<KvIterator>();
}

Value InitKeyEndValue(const Value& key, IndexType type) {
    switch (type) {
    case IndexType::GlobalUniqueIndex:
        return Value::MakeCopy(key);
    case IndexType::PairUniqueIndex:
        return _detail::PatchPairUniqueIndexKey(key, -1, -1);
    case IndexType::NonuniqueIndex:
        return _detail::PatchNonuniqueIndexKey(key, -1, -1, -1, -1, -1);
    }
    return Value();
}

inline Value ReturnKeyEvenIfLong(Value&& v) { return std::move(v); }

}  // namespace _detail

//  begin of class EdgeIndexValue

EdgeIndexValue::EdgeIndexValue()
    : v_(1) { *(uint8_t*)v_.Data() = 0; }

EdgeIndexValue::EdgeIndexValue(const Value& v)
    : v_(v) {}

EdgeIndexValue::EdgeIndexValue(Value&& v)
    : v_(std::move(v)) {}

int EdgeIndexValue::SearchEUid(const EdgeUid& euid, bool& found) const {
    if (GetEUidCount() == 0) {
        found = false;
        return 0;
    }
    int beg = 0;
    int end = (int)GetEUidCount();
    while (beg < end) {
        int p = (beg + end) / 2;
        EdgeUid peuid = GetNthEuid(p);
        if (euid == peuid) {
            found = true;
            return p;
        }
        if (euid < peuid) {
            end = p;
        } else if (euid > peuid) {
            beg = p + 1;
        }
    }
    found = false;
    return end;
}

uint8_t EdgeIndexValue::InsertEUid(const EdgeUid& euid) {
    bool exists;
    int pos = SearchEUid(euid, exists);
    if (exists) {
        return 0;
    }
    bool tail = (pos == GetEUidCount());
    InsertEUidAtNthPos(pos, euid);
    uint8_t ret = !tail ? 1 : 2;
    return ret;
}

void EdgeIndexValue::InsertEUidAtNthPos(int pos, const EdgeUid& euid) {
    // insert the vid
    size_t bytes_after_p = v_.Size() - (1 + (_detail::EUID_SIZE)*pos);
    v_.Resize(v_.Size() + _detail::EUID_SIZE);
    char* ptr = v_.Data() + 1 + (_detail::EUID_SIZE)*pos;
    memmove(ptr + (_detail::EUID_SIZE), ptr, bytes_after_p);
    _detail::WriteVid(ptr, euid.src);
    _detail::WriteVid(ptr + _detail::VID_SIZE, euid.dst);
    _detail::WriteLabelId(ptr + _detail::LID_BEGIN, euid.lid);
    _detail::WriteTemporalId(ptr + _detail::TID_BEGIN, euid.tid);
    _detail::WriteEid(ptr + _detail::EID_BEGIN, euid.eid);
    (*(uint8_t*)v_.Data())++;
}

uint8_t EdgeIndexValue::DeleteEUidIfExist(const EdgeUid& euid) {
    bool exists;
    int pos = SearchEUid(euid, exists);
    if (!exists) {
        return 0;
    }
    bool tail = (pos == GetEUidCount() - 1);
    DeleteNthEUid(pos);
    return !tail ? 1 : 2;
}

void EdgeIndexValue::DeleteNthEUid(int pos) {
    v_.Resize(v_.Size());
    char* ptr = v_.Data() + 1 + (_detail::EUID_SIZE)*pos;
    uint8_t n = GetEUidCount();
    memmove(ptr, ptr + (_detail::EUID_SIZE), (n - pos - 1) * (_detail::EUID_SIZE));
    v_.Resize(v_.Size() - (_detail::EUID_SIZE));
    *(uint8_t*)v_.Data() = n - 1;
}

EdgeIndexValue EdgeIndexValue::SplitRightHalf() {
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

Value EdgeIndexValue::CreateKey(const Value& key) const {
    int pos = GetEUidCount() - 1;
    Value v(key.Size() + (_detail::EUID_SIZE));
    memcpy(v.Data(), key.Data(), key.Size());
    memcpy(v.Data() + key.Size(),
           v_.Data() + 1 + pos * (_detail::EUID_SIZE), _detail::EUID_SIZE);
    return v;
}

//  begin of class EdgeIndexIterator

EdgeIndexIterator::EdgeIndexIterator(EdgeIndex* idx, Transaction* txn, KvTable& table,
                                     const Value& key_start, const Value& key_end, VertexId vid,
                                     VertexId vid2, LabelId lid, TemporalId tid, EdgeId eid,
                                     IndexType type)
    : IteratorBase(txn),
      index_(idx),
      it_(_detail::InitEdgeIndexIterator(
          txn->GetTxn(), table, key_start, vid, vid2, lid, tid, eid, type)),
      key_end_(_detail::InitKeyEndValue(key_end, type)),
      iv_(),
      valid_(false),
      pos_(0),
      type_(type) {
    if (!it_->IsValid() || KeyOutOfRange()) {
        return;
    }
    LoadContentFromIt();
}

EdgeIndexIterator::EdgeIndexIterator(EdgeIndex* idx, KvTransaction* txn, KvTable& table,
                                     const Value& key_start, const Value& key_end, VertexId vid,
                                     VertexId vid2, LabelId lid, TemporalId tid, EdgeId eid,
                                     IndexType type)
    : IteratorBase(nullptr),
      index_(idx),
      it_(_detail::InitEdgeIndexIterator(
          *txn, table, key_start, vid, vid2, lid, tid, eid, type)),
      key_end_(_detail::InitKeyEndValue(key_end, type)),
      iv_(),
      valid_(false),
      pos_(0),
      type_(type) {
    if (!it_->IsValid() || KeyOutOfRange()) {
        return;
    }
    LoadContentFromIt();
}

EdgeIndexIterator::EdgeIndexIterator(EdgeIndexIterator&& rhs)
    : IteratorBase(std::move(rhs)),
      index_(rhs.index_),
      it_(std::move(rhs.it_)),
      key_end_(std::move(rhs.key_end_)),
      curr_key_(std::move(rhs.curr_key_)),
      iv_(std::move(rhs.iv_)),
      valid_(rhs.valid_),
      pos_(rhs.pos_),
      src_vid_(rhs.src_vid_),
      dst_vid_(rhs.dst_vid_),
      lid_(rhs.lid_),
      tid_(rhs.tid_),
      eid_(rhs.eid_),
      type_(rhs.type_) {
    rhs.valid_ = false;
}

bool EdgeIndexIterator::KeyOutOfRange() {
    if (key_end_.Empty() ||
        (type_ == IndexType::NonuniqueIndex && key_end_.Size() == _detail::EUID_SIZE) ||
        (type_ == IndexType::PairUniqueIndex && key_end_.Size() == _detail::VID_SIZE * 2))
        return false;
    return it_->GetTable().CompareKey(it_->GetTxn(), it_->GetKey(), key_end_) > 0;
}

bool EdgeIndexIterator::PrevKV() {
    if (!it_->Prev()) {
        return false;
    }
    LoadContentFromIt();
    return true;
}

bool EdgeIndexIterator::KeyEquals(Value& key) {
    // just need to deal with NonuniqueIndex，because a key has a set of values in
    // this index type, and other types are guaranteed by lmdb
    if (type_ == IndexType::NonuniqueIndex) {
        auto key_euid = it_->GetKey();
        if (key_euid.Size() - _detail::EUID_SIZE != key.Size()) {
            return false;
        }
        return memcmp(key.Data(), key_euid.Data(), key.Size()) == 0;
    }
    return false;
}

void EdgeIndexIterator::LoadContentFromIt() {
    valid_ = true;
    pos_ = 0;
    curr_key_.Copy(GetKey());
    switch (type_) {
    case IndexType::GlobalUniqueIndex:
        {
            src_vid_ = _detail::GetVid(it_->GetValue().Data());
            dst_vid_ = _detail::GetVid(it_->GetValue().Data() + _detail::VID_SIZE);
            lid_ = _detail::GetLabelId(it_->GetValue().Data() + _detail::LID_BEGIN);
            tid_ = _detail::GetTemporalId(it_->GetValue().Data() + _detail::TID_BEGIN);
            eid_ = _detail::GetEid(it_->GetValue().Data() + _detail::EID_BEGIN);
            break;
        }
    case IndexType::PairUniqueIndex:
        {
            src_vid_ = GetPairUniqueSrcVertexId();
            dst_vid_ = GetPairUniqueDstVertexId();
            lid_ = _detail::GetLabelId(it_->GetValue().Data());
            tid_ = _detail::GetTemporalId(it_->GetValue().Data() + _detail::LID_SIZE);
            eid_ =
                _detail::GetEid(it_->GetValue().Data() + _detail::LID_SIZE + _detail::TID_SIZE);
            break;
        }
    case IndexType::NonuniqueIndex:
        {
            iv_ = EdgeIndexValue(it_->GetValue());
            src_vid_ = iv_.GetNthSrcVid(pos_);
            dst_vid_ = iv_.GetNthDstVid(pos_);
            lid_ = iv_.GetNthLid(pos_);
            tid_ = iv_.GetNthTid(pos_);
            eid_ = iv_.GetNthEid(pos_);
            break;
        }
    }
}

void EdgeIndexIterator::CloseImpl() {
    it_->Close();
    valid_ = false;
}

bool EdgeIndexIterator::Next() {
    // if we haven't reach the end of the current EdgeIndexValue,
    // just move forward
    switch (type_) {
    case IndexType::NonuniqueIndex:
        {
            if (pos_ < iv_.GetEUidCount() - 1) {
                pos_ += 1;
                src_vid_ = iv_.GetNthSrcVid(pos_);
                dst_vid_ = iv_.GetNthDstVid(pos_);
                lid_ = iv_.GetNthLid(pos_);
                tid_ = iv_.GetNthTid(pos_);
                eid_ = iv_.GetNthEid(pos_);
                return true;
            }
        }
    case IndexType::GlobalUniqueIndex:
    case IndexType::PairUniqueIndex:
        {
            valid_ = false;
            if (!it_->Next() || KeyOutOfRange()) {
                return false;
            }
            LoadContentFromIt();
            return true;
        }
    }
    return false;
}

Value EdgeIndexIterator::GetKey() const {
    switch (type_) {
    case IndexType::GlobalUniqueIndex:
        {
            return _detail::ReturnKeyEvenIfLong(it_->GetKey());
        }
    case IndexType::PairUniqueIndex:
        {
            Value key_both_vid = it_->GetKey();
            return _detail::ReturnKeyEvenIfLong(
                Value(key_both_vid.Data(), key_both_vid.Size() - _detail::VID_SIZE * 2));
        }
    case IndexType::NonuniqueIndex:
        {
            Value key_euid = it_->GetKey();
            return _detail::ReturnKeyEvenIfLong(
                Value(key_euid.Data(), key_euid.Size() - _detail::EUID_SIZE));
        }
    }
    return Value();
}

VertexId EdgeIndexIterator::GetPairUniqueSrcVertexId() {
    VertexId vid = 0;
    Value key_vid = _detail::ReturnKeyEvenIfLong(it_->GetKey());
    memcpy((char*)&vid, key_vid.Data() + key_vid.Size() - 2 * _detail::VID_SIZE,
           _detail::VID_SIZE);
    return vid;
}

VertexId EdgeIndexIterator::GetPairUniqueDstVertexId() {
    VertexId vid = 0;
    Value key_vid = _detail::ReturnKeyEvenIfLong(it_->GetKey());
    memcpy((char*)&vid, key_vid.Data() + key_vid.Size() - _detail::VID_SIZE, _detail::VID_SIZE);
    return vid;
}

FieldType EdgeIndexIterator::KeyType() const { return index_->KeyType(); }

void EdgeIndexIterator::RefreshContentIfKvIteratorModified() {
    if (IsValid() && it_->IsValid() && it_->UnderlyingPointerModified()) {
        valid_ = false;
        // now it_ points to a valid position, but not necessary the right one
        switch (type_) {
        case IndexType::GlobalUniqueIndex:
            {
                if (!it_->GotoClosestKey(curr_key_)) return;
                if (KeyOutOfRange()) return;
                LoadContentFromIt();
                return;
            }
        case IndexType::PairUniqueIndex:
            {
                if (!it_->GotoClosestKey(
                    _detail::PatchPairUniqueIndexKey(curr_key_, src_vid_, dst_vid_)))
                    return;
                if (KeyOutOfRange()) return;
                LoadContentFromIt();
                return;
            }
        case IndexType::NonuniqueIndex:
            {
                if (!it_->GotoClosestKey(_detail::PatchNonuniqueIndexKey(
                    curr_key_, src_vid_, dst_vid_, lid_, tid_, eid_)))
                    return;
                if (KeyOutOfRange()) return;
                iv_ = EdgeIndexValue(it_->GetValue());
                pos_ = iv_.SearchEUid({src_vid_, dst_vid_, lid_, tid_, eid_}, valid_);
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
}

//  begin of class EdgeIndex

EdgeIndex::EdgeIndex(std::shared_ptr<KvTable> table, FieldType key_type, IndexType type)
    : table_(std::move(table)),
      key_type_(key_type),
      ready_(false),
      disabled_(false),
      type_(type) {}

EdgeIndex::EdgeIndex(const EdgeIndex& rhs)
    : table_(rhs.table_),
      key_type_(rhs.key_type_),
      ready_(rhs.ready_.load()),
      disabled_(rhs.disabled_.load()),
      type_(rhs.type_) {}

std::unique_ptr<KvTable> EdgeIndex::OpenTable(KvTransaction& txn, KvStore& store,
                                   const std::string& name, FieldType dt,
                                   IndexType type) {
    ComparatorDesc desc;
    switch (type) {
    case IndexType::NonuniqueIndex:
        desc.comp_type = ComparatorDesc::TYPE_AND_EUID;
        desc.data_type = dt;
        break;
    case IndexType::GlobalUniqueIndex:
        desc.comp_type = ComparatorDesc::SINGLE_TYPE;
        desc.data_type = dt;
        break;
    case IndexType::PairUniqueIndex:
        desc.comp_type = ComparatorDesc::BOTH_SIDE_VID;
        desc.data_type = dt;
        break;
    }
    return store.OpenTable(txn, name, true, desc);
}

void EdgeIndex::_AppendIndexEntry(KvTransaction& txn, const Value& k, EdgeUid euid) {
    FMA_DBG_ASSERT(type_ == IndexType::GlobalUniqueIndex ||
                   type_ == IndexType::PairUniqueIndex);
    if (k.Size() > GetMaxEdgeIndexKeySize())
        THROW_CODE(InputError, "Edge index value [{}] is too long.", k.AsString());
    Value key = Value::ConstRef(k);
    if (type_ == IndexType::PairUniqueIndex) {
        size_t key_size = key.Size();
        key.Resize(key_size + _detail::VID_SIZE * 2);
        _detail::WriteVid(key.Data() + key_size, euid.src);
        _detail::WriteVid(key.Data() + key_size + _detail::VID_SIZE, euid.dst);
        Value v(_detail::LID_SIZE + _detail::TID_SIZE + _detail::EID_SIZE);
        _detail::WriteLabelId(v.Data(), euid.lid);
        _detail::WriteTemporalId(v.Data() + _detail::LID_SIZE, euid.tid);
        _detail::WriteEid(v.Data() + _detail::LID_SIZE + _detail::TID_SIZE, euid.eid);
        table_->AppendKv(txn, key, v);
    } else if (type_ == IndexType::GlobalUniqueIndex) {
        Value v(_detail::EUID_SIZE);
        _detail::WriteVid(v.Data(), euid.src);
        _detail::WriteVid(v.Data() + _detail::VID_SIZE, euid.dst);
        _detail::WriteLabelId(v.Data() + _detail::LID_BEGIN, euid.lid);
        _detail::WriteTemporalId(v.Data() + _detail::TID_BEGIN, euid.tid);
        _detail::WriteEid(v.Data() + _detail::EID_BEGIN, euid.eid);
        table_->AppendKv(txn, key, v);
    }
}

void EdgeIndex::_AppendNonUniqueIndexEntry(KvTransaction& txn, const Value& k,
                                const std::vector<EdgeUid>& euids) {
    FMA_DBG_ASSERT(type_ == IndexType::NonuniqueIndex);
    FMA_DBG_ASSERT(!euids.empty());
    Value key = CutKeyIfLongOnlyForNonUniqueIndex(k);
    size_t euid_per_idv = _detail::NODE_SPLIT_THRESHOLD / _detail::EUID_SIZE;
    for (size_t i = 0; i < euids.size(); i += euid_per_idv) {
        size_t end = i + euid_per_idv;
        end = end <= euids.size() ? end : euids.size();
        EdgeIndexValue idv(euids.begin() + i, euids.begin() + end);
        Value real_key = idv.CreateKey(key);
        table_->AppendKv(txn, real_key, idv.GetBuf());
    }
}

void EdgeIndex::Dump(KvTransaction& txn,
          const std::function<std::string(const char* p, size_t s)>& key_to_string,
          std::string& str) {
    auto it = table_->GetIterator(txn);
    for (it->GotoFirstKey(); it->IsValid(); it->Next()) {
        const Value& k = it->GetKey();
        EdgeIndexValue iv(it->GetValue());
        std::string line;
        line = key_to_string(k.Data(), k.Size());
        line.append(" -> ");
        switch (type_) {
        case IndexType::GlobalUniqueIndex:
            {
                line.append(std::to_string(_detail::GetVid(iv.GetBuf().Data())));
                line.append(
                    std::to_string(_detail::GetVid(iv.GetBuf().Data() + _detail::VID_SIZE)));
                line.append(std::to_string(
                    _detail::GetLabelId(iv.GetBuf().Data() + _detail::LID_BEGIN)));
                line.append(std::to_string(
                    _detail::GetTemporalId(iv.GetBuf().Data() + _detail::TID_BEGIN)));
                line.append(
                    std::to_string(_detail::GetEid(iv.GetBuf().Data() + _detail::EID_BEGIN)));
                break;
            }
        case IndexType::PairUniqueIndex:
            {
                line.append(std::to_string(_detail::GetLabelId(iv.GetBuf().Data())));
                line.append(std::to_string(
                    _detail::GetTemporalId(iv.GetBuf().Data() + _detail::LID_SIZE)));
                line.append(std::to_string(_detail::GetEid(
                    iv.GetBuf().Data() + _detail::LID_SIZE + _detail::TID_SIZE)));
                break;
            }
        case IndexType::NonuniqueIndex:
            {
                for (int i = 0; i < iv.GetEUidCount(); i++) {
                    line.append(std::to_string(iv.GetNthSrcVid(i)));
                    line.append(std::to_string(iv.GetNthDstVid(i)));
                    line.append(std::to_string(iv.GetNthLid(i)));
                    line.append(std::to_string(iv.GetNthTid(i)));
                    line.append(std::to_string(iv.GetNthEid(i)));
                    line.append(",");
                }
                break;
            }
        }
        str.swap(line);
    }
}

bool EdgeIndex::Delete(KvTransaction& txn, const Value& k, const EdgeUid& euid) {
    Value key = type_ == IndexType::NonuniqueIndex ? CutKeyIfLongOnlyForNonUniqueIndex(k) : k;
    EdgeIndexIterator it =
        GetUnmanagedIterator(txn, k, k, euid.src, euid.dst, euid.lid, euid.tid, euid.eid);
    if (!it.IsValid() || it.KeyOutOfRange()) {
        // no such key_vid
        return false;
    }
    switch (type_) {
    case IndexType::GlobalUniqueIndex:
    case IndexType::PairUniqueIndex:
        it.it_->DeleteKey();
        return true;
    case IndexType::NonuniqueIndex:
        {
            uint8_t ret =
                it.iv_.DeleteEUidIfExist({euid.src, euid.dst, euid.lid, euid.tid, euid.eid});
            if (ret == 1) {
                // the key does not change after deletion
                it.it_->SetValue(it.iv_.GetBuf());
            } else if (ret == 2) {
                // the key changes after deletion
                it.it_->DeleteKey();
                if (it.iv_.GetEUidCount()) {
                    bool r = it.it_->AddKeyValue(it.iv_.CreateKey(key), it.iv_.GetBuf());
                    FMA_DBG_ASSERT(r);
                }
            } else {
                // no such vid
                return false;
            }
            return true;
        }
    }
    return false;
}

bool EdgeIndex::Update(KvTransaction& txn, const Value& old_key, const Value& new_key,
            const EdgeUid& euid) {
    if (!Delete(txn, old_key, euid)) {
        return false;
    }
    return Add(txn, new_key, euid);
}

bool EdgeIndex::UniqueIndexConflict(KvTransaction& txn, const Value& k) {
    if (type_ != IndexType::GlobalUniqueIndex) {
        THROW_CODE(InputError, "edge UniqueIndexConflict only can be used in unique index.");
    }
    if (k.Size() > GetMaxEdgeIndexKeySize()) {
        return true;
    }
    Value v;
    return table_->GetValue(txn, k, v);
}

bool EdgeIndex::Add(KvTransaction& txn, const Value& k, const EdgeUid& euid) {
    switch (type_) {
    case IndexType::GlobalUniqueIndex:
        {
            if (k.Size() > GetMaxEdgeIndexKeySize())
                THROW_CODE(InputError, "Edge global unique index value [{}] is too long.",
                           k.AsString());
            Value v(_detail::EUID_SIZE);
            _detail::WriteVid(v.Data(), euid.src);
            _detail::WriteVid(v.Data() + _detail::VID_SIZE, euid.dst);
            _detail::WriteLabelId(v.Data() + _detail::LID_BEGIN, euid.lid);
            _detail::WriteTemporalId(v.Data() + _detail::TID_BEGIN, euid.tid);
            _detail::WriteEid(v.Data() + _detail::EID_BEGIN, euid.eid);
            return table_->AddKV(txn, Value::ConstRef(k), v);
        }
    case IndexType::PairUniqueIndex:
        {
            if (k.Size() > GetMaxEdgeIndexKeySize())
                THROW_CODE(InputError, "Edge pair unique index value [{}] is too long.",
                           k.AsString());
            Value real_key = _detail::PatchPairUniqueIndexKey(Value::ConstRef(k),
                                                              euid.src, euid.dst);
            Value val(_detail::LID_SIZE + _detail::TID_SIZE + _detail::EID_SIZE);
            _detail::WriteLabelId(val.Data(), euid.lid);
            _detail::WriteTemporalId(val.Data() + _detail::LID_SIZE, euid.tid);
            _detail::WriteEid(val.Data() + _detail::LID_SIZE + _detail::TID_SIZE, euid.eid);
            return table_->AddKV(txn, real_key, val);
        }
    case IndexType::NonuniqueIndex:
        {
            Value key = CutKeyIfLongOnlyForNonUniqueIndex(k);
            EdgeIndexIterator it = GetUnmanagedIterator(txn, key, key, euid.src, euid.dst,
                                                        euid.lid, euid.tid, euid.eid);
            if (!it.IsValid() || it.KeyOutOfRange()) {
                if (!it.PrevKV() || !it.KeyEquals(key)) {
                    // create a new VertexIndexValue
                    EdgeIndexValue iv;
                    uint8_t r = iv.InsertEUid({euid.src, euid.dst, euid.lid, euid.tid, euid.eid});
                    FMA_DBG_CHECK_NEQ(r, 0);
                    bool r2 = table_->AddKV(txn, iv.CreateKey(key), iv.GetBuf());
                    FMA_DBG_ASSERT(r2);
                    return true;
                }
            }
            uint8_t ret = it.iv_.InsertEUid({euid.src, euid.dst, euid.lid, euid.tid, euid.eid});
            if (it.iv_.TooLarge()) {
                EdgeIndexValue right = it.iv_.SplitRightHalf();
                // remove the original EdgeIndexValue
                it.it_->DeleteKey();
                // insert the left half EdgeIndexValue
                bool r = table_->AddKV(txn, it.iv_.CreateKey(key), it.iv_.GetBuf());
                FMA_DBG_ASSERT(r);
                // insert the right half EdgeIndexValue
                r = table_->AddKV(txn, right.CreateKey(key), right.GetBuf());
                FMA_DBG_ASSERT(r);
            } else {
                if (ret == 1) {
                    // the key does not change, so just update the EdgeIndexValue
                    it.it_->SetValue(it.iv_.GetBuf());
                } else if (ret == 2) {
                    // the key changes so we need to remove the original EdgeIndexValue
                    it.it_->DeleteKey();
                    // then insert the EdgeIndexValue with an updated key
                    bool r = table_->AddKV(txn, it.iv_.CreateKey(key), it.iv_.GetBuf());
                    FMA_DBG_ASSERT(r);
                } else {
                    // vid already exists
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

size_t EdgeIndex::GetMaxEdgeIndexKeySize() {
    size_t key_size = 0;
    switch (type_) {
    case IndexType::GlobalUniqueIndex:
        {
            key_size = _detail::MAX_KEY_SIZE;
            break;
        }
    case IndexType::PairUniqueIndex:
        {
            key_size = _detail::MAX_KEY_SIZE - _detail::VID_SIZE * 2;
            break;
        }
    case IndexType::NonuniqueIndex:
        {
            key_size = _detail::MAX_KEY_SIZE - _detail::EUID_SIZE;
        }
    }
    return key_size;
}

Value EdgeIndex::CutKeyIfLongOnlyForNonUniqueIndex(const Value& k) {
    if (type_ != IndexType::NonuniqueIndex)
        return Value::ConstRef(k);
    size_t key_size = GetMaxEdgeIndexKeySize();
    if (k.Size() < key_size) return Value::ConstRef(k);
    return Value(k.Data(), key_size);
}

}  // namespace lgraph
