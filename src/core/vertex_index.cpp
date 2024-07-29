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

#include "core/vertex_index.h"
#include "core/transaction.h"

namespace lgraph {

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

inline Value ReturnKeyEvenIfLong(Value&& v) { return std::move(v); }

}  // namespace _detail

//  begin of VertexIndexValue

VertexIndexValue::VertexIndexValue() : v_(1) { *(uint8_t*)v_.Data() = 0; }

VertexIndexValue::VertexIndexValue(const Value& v) : v_(v) {}

VertexIndexValue::VertexIndexValue(Value&& v) : v_(std::move(v)) {}

int VertexIndexValue::SearchVid(int64_t vid, bool& found) const {
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

uint8_t VertexIndexValue::InsertVid(int64_t vid) {
    bool exists;
    int pos = SearchVid(vid, exists);
    if (exists) {
        return 0;
    }
    bool tail = (pos == GetVidCount());
    InsertVidAtNthPos(pos, vid);
    return !tail ? 1 : 2;
}

void VertexIndexValue::InsertVidAtNthPos(int pos, int64_t vid) {
    // insert the vid
    size_t bytes_after_p = v_.Size() - (1 + _detail::VID_SIZE * pos);
    v_.Resize(v_.Size() + _detail::VID_SIZE);
    char* ptr = v_.Data() + 1 + _detail::VID_SIZE * pos;
    memmove(ptr + _detail::VID_SIZE, ptr, bytes_after_p);
    _detail::WriteVid(ptr, vid);
    (*(uint8_t*)v_.Data())++;
}

uint8_t VertexIndexValue::DeleteVidIfExist(int64_t vid) {
    bool exists;
    int pos = SearchVid(vid, exists);
    if (!exists) {
        return 0;
    }
    bool tail = (pos == GetVidCount() - 1);
    DeleteNthVid(pos);
    return !tail ? 1 : 2;
}

void VertexIndexValue::DeleteNthVid(int pos) {
    v_.Resize(v_.Size());
    char* ptr = v_.Data() + 1 + _detail::VID_SIZE * pos;
    uint8_t n = GetVidCount();
    memmove(ptr, ptr + _detail::VID_SIZE, (n - pos - 1) * _detail::VID_SIZE);
    v_.Resize(v_.Size() - _detail::VID_SIZE);
    *(uint8_t*)v_.Data() = n - 1;
}

VertexIndexValue VertexIndexValue::SplitRightHalf() {
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

Value VertexIndexValue::CreateKey(const Value& key) const {
    int pos = GetVidCount() - 1;
    Value v(key.Size() + _detail::VID_SIZE);
    memcpy(v.Data(), key.Data(), key.Size());
    memcpy(v.Data() + key.Size(), v_.Data() + 1 + pos * _detail::VID_SIZE, _detail::VID_SIZE);
    return v;
}

//  begin of VertexIndexIterator

VertexIndexIterator::VertexIndexIterator(VertexIndex* idx, Transaction* txn, KvTable& table,
                                         const Value& key_start, const Value& key_end, VertexId vid,
                                         IndexType type)
    : IteratorBase(txn),
      index_(idx),
      it_(table.GetClosestIterator(txn->GetTxn(), type == IndexType::GlobalUniqueIndex
                                                      ? key_start
                                                      : _detail::PatchKeyWithVid(key_start, vid))),
      key_end_(type == IndexType::GlobalUniqueIndex ? Value::MakeCopy(key_end)
                                                    : _detail::PatchKeyWithVid(key_end, -1)),
      iv_(),
      valid_(false),
      pos_(0),
      type_(type) {
    if (type == IndexType::PairUniqueIndex) {
        return;
    }
    if (!it_->IsValid() || KeyOutOfRange()) {
        return;
    }
    LoadContentFromIt();
}

VertexIndexIterator::VertexIndexIterator(VertexIndex* idx, KvTransaction* txn, KvTable& table,
                                         const Value& key_start, const Value& key_end, VertexId vid,
                                         IndexType type)
    : IteratorBase(nullptr),
      index_(idx),
      it_(table.GetClosestIterator(*txn, type == IndexType::GlobalUniqueIndex
                                             ? key_start
                                             : _detail::PatchKeyWithVid(key_start, vid))),
      key_end_(type == IndexType::GlobalUniqueIndex ? Value::MakeCopy(key_end)
                                                    : _detail::PatchKeyWithVid(key_end, -1)),
      iv_(),
      valid_(false),
      pos_(0),
      type_(type) {
    if (type == IndexType::PairUniqueIndex) {
        return;
    }
    if (!it_->IsValid() || KeyOutOfRange()) {
        return;
    }

    LoadContentFromIt();
}

VertexIndexIterator::VertexIndexIterator(VertexIndexIterator&& rhs)
    : IteratorBase(std::move(rhs)),
      index_(rhs.index_),
      it_(std::move(rhs.it_)),
      key_end_(std::move(rhs.key_end_)),
      curr_key_(std::move(rhs.curr_key_)),
      iv_(std::move(rhs.iv_)),
      valid_(rhs.valid_),
      pos_(rhs.pos_),
      vid_(rhs.vid_),
      type_(rhs.type_) {
    rhs.valid_ = false;
}

bool VertexIndexIterator::KeyOutOfRange() {
    if (key_end_.Empty() ||
        (type_ != IndexType::GlobalUniqueIndex && key_end_.Size() == _detail::VID_SIZE))
        return false;
    return it_->GetTable().CompareKey(it_->GetTxn(), it_->GetKey(), key_end_) > 0;
}

bool VertexIndexIterator::PrevKV() {
    if (!it_->Prev()) {
        return false;
    }
    LoadContentFromIt();
    return true;
}

bool VertexIndexIterator::KeyEquals(const Value& key) {
    FMA_DBG_ASSERT(type_ != IndexType::PairUniqueIndex);
    // just need to deal with NonuniqueIndex，because a key has a set of values
    // in this kind index, and other types are guaranteed by lmdb
    if (type_ == IndexType::NonuniqueIndex) {
        auto key_vid = it_->GetKey();
        if (key_vid.Size() - _detail::VID_SIZE != key.Size()) {
            return false;
        }
        return memcmp(key.Data(), key_vid.Data(), key.Size()) == 0;
    }
    return false;
}

void VertexIndexIterator::LoadContentFromIt() {
    valid_ = true;
    pos_ = 0;
    curr_key_.Copy(GetKey());
    switch (type_) {
    case IndexType::GlobalUniqueIndex:
        {
            vid_ = _detail::GetVid(it_->GetValue().Data());
            break;
        }
    case IndexType::NonuniqueIndex:
        {
            iv_ = VertexIndexValue(it_->GetValue());
            vid_ = iv_.GetNthVid(pos_);
            break;
        }
    case IndexType::PairUniqueIndex:
        {
            THROW_CODE(InputError, "vertex index do not support pair-unique attributes");
        }
    }
}

void VertexIndexIterator::CloseImpl() {
    it_->Close();
    valid_ = false;
}

bool VertexIndexIterator::Next() {
    // if we haven't reach the end of the current VertexIndexValue,
    // just move forward
    if (type_ != IndexType::GlobalUniqueIndex && pos_ < iv_.GetVidCount() - 1) {
        pos_ += 1;
        vid_ = iv_.GetNthVid(pos_);
        return true;
    }
    // otherwise try moving to the next VertexIndex Value,
    // and check if it is still within the range
    valid_ = false;
    if (!it_->Next() || KeyOutOfRange()) {
        return false;
    }
    LoadContentFromIt();
    return true;
}

bool VertexIndexIterator::Goto(lgraph::VertexId vid) {
    if (!it_->GotoClosestKey(curr_key_)) return false;
    if (KeyOutOfRange()) return false;
    LoadContentFromIt();
    return it_->IsValid() && vid == vid_;
}

Value VertexIndexIterator::GetKey() const {
    switch (type_) {
    case IndexType::GlobalUniqueIndex:
        {
            return _detail::ReturnKeyEvenIfLong(it_->GetKey());
        }
    case IndexType::NonuniqueIndex:
        {
            Value key_vid = it_->GetKey();
            return _detail::ReturnKeyEvenIfLong(
                Value(key_vid.Data(), key_vid.Size() - _detail::VID_SIZE));
        }
    case IndexType::PairUniqueIndex:
        {
            THROW_CODE(InputError, "vertex index do not support pair-unique attributes");
        }
    }
    return Value();
}

FieldType VertexIndexIterator::KeyType() const { return index_->KeyType(); }

void VertexIndexIterator::RefreshContentIfKvIteratorModified() {
    if (IsValid() && it_->IsValid() && it_->UnderlyingPointerModified()) {
        valid_ = false;
        switch (type_) {
        case IndexType::GlobalUniqueIndex:
            {
                if (!it_->GotoClosestKey(curr_key_)) return;
                if (KeyOutOfRange()) return;
                LoadContentFromIt();
                return;
            }
        case IndexType::NonuniqueIndex:
            {
                if (!it_->GotoClosestKey(_detail::PatchKeyWithVid(curr_key_, vid_))) return;
                if (KeyOutOfRange()) return;
                // non-unique, need to find correct pos_
                iv_ = VertexIndexValue(it_->GetValue());
                pos_ = iv_.SearchVid(vid_, valid_);
                if (pos_ >= iv_.GetVidCount()) return;
                valid_ = true;
                curr_key_.Copy(GetKey());
                vid_ = iv_.GetNthVid(pos_);
                return;
            }
        case IndexType::PairUniqueIndex:
            THROW_CODE(InputError, "vertex index do not support pair-unique attributes");
        }
        // now it_ points to a valid position, but not necessary the right one
    }
}

//  begin of VertexIndex

VertexIndex::VertexIndex(std::shared_ptr<KvTable> table, FieldType key_type, IndexType type)
    : table_(std::move(table)), key_type_(key_type), ready_(false), disabled_(false), type_(type) {}

VertexIndex::VertexIndex(const VertexIndex& rhs)
    : table_(rhs.table_),
      key_type_(rhs.key_type_),
      ready_(rhs.ready_.load()),
      disabled_(rhs.disabled_.load()),
      type_(rhs.type_) {}

std::unique_ptr<KvTable> VertexIndex::OpenTable(KvTransaction& txn, KvStore& store,
                                                const std::string& name, FieldType dt,
                                                IndexType type) {
    ComparatorDesc desc;
    switch (type) {
    case IndexType::GlobalUniqueIndex:
        {
            desc.comp_type = ComparatorDesc::SINGLE_TYPE;
            break;
        }
    case IndexType::NonuniqueIndex:
        {
            desc.comp_type = ComparatorDesc::TYPE_AND_VID;
            break;
        }
    case IndexType::PairUniqueIndex:
        {
            THROW_CODE(InputError, "vertex index do not support pair-unique attributes");
        }
    }
    desc.data_type = dt;
    return store.OpenTable(txn, name, true, desc);
}

void VertexIndex::_AppendVertexIndexEntry(KvTransaction& txn, const Value& k, VertexId vid) {
    FMA_DBG_ASSERT(type_ == IndexType::GlobalUniqueIndex);
    if (k.Size() > GetMaxVertexIndexKeySize())
        THROW_CODE(InputError, "Vertex unique index value [{}] is too long.", k.AsString());
    table_->AppendKv(txn, Value::ConstRef(k), Value::ConstRef(vid));
}

void VertexIndex::_AppendNonUniqueVertexIndexEntry(KvTransaction& txn, const Value& k,
                                                   const std::vector<VertexId>& vids) {
    FMA_DBG_ASSERT(type_ == IndexType::NonuniqueIndex);
    FMA_DBG_ASSERT(!vids.empty());
    Value key = CutKeyIfLongOnlyForNonUniqueIndex(k);
    size_t vid_per_idv = _detail::NODE_SPLIT_THRESHOLD / _detail::VID_SIZE;
    for (size_t i = 0; i < vids.size(); i += vid_per_idv) {
        size_t end = i + vid_per_idv;
        end = end <= vids.size() ? end : vids.size();
        VertexIndexValue idv(vids.begin() + i, vids.begin() + end);
        Value real_key = idv.CreateKey(key);
        table_->AppendKv(txn, real_key, idv.GetBuf());
    }
}

void VertexIndex::Dump(KvTransaction& txn,
                       const std::function<std::string(const char* p, size_t s)>& key_to_string) {
    auto it = table_->GetIterator(txn);
    for (it->GotoFirstKey(); it->IsValid(); it->Next()) {
        const Value& k = it->GetKey();
        VertexIndexValue iv(it->GetValue());
        std::string line = key_to_string(k.Data(), k.Size());
        line.append(" -> ");
        switch (type_) {
        case IndexType::GlobalUniqueIndex:
            {
                line.append(std::to_string(_detail::GetVid(iv.GetBuf().Data())));
                break;
            }
        case IndexType::NonuniqueIndex:
            {
                for (int i = 0; i < iv.GetVidCount(); i++) {
                    line.append(std::to_string(iv.GetNthVid(i)));
                    line.append(",");
                }
                break;
            }
        case IndexType::PairUniqueIndex:
            {
                THROW_CODE(InputError, "vertex index do not support pair-unique attributes");
            }
        }
        LOG_INFO() << line;
    }
}

bool VertexIndex::Delete(KvTransaction& txn, const Value& k, int64_t vid) {
    Value key = type_ == IndexType::GlobalUniqueIndex ? k : CutKeyIfLongOnlyForNonUniqueIndex(k);
    VertexIndexIterator it = GetUnmanagedIterator(txn, key, key, vid);
    if (!it.IsValid() || it.KeyOutOfRange()) {
        // no such key_vid
        return false;
    }
    switch (type_) {
    case IndexType::GlobalUniqueIndex:
        {
            it.it_->DeleteKey();
            return true;
        }
    case IndexType::NonuniqueIndex:
        {
            uint8_t ret = it.iv_.DeleteVidIfExist(vid);
            if (ret == 1) {
                // the key does not change after deletion
                it.it_->SetValue(it.iv_.GetBuf());
            } else if (ret == 2) {
                // the key changes after deletion
                it.it_->DeleteKey();
                if (it.iv_.GetVidCount()) {
                    bool r = it.it_->AddKeyValue(it.iv_.CreateKey(key), it.iv_.GetBuf());
                    FMA_DBG_ASSERT(r);
                }
            } else {
                // no such vid
                return false;
            }
            return true;
        }
    case IndexType::PairUniqueIndex:
        {
            THROW_CODE(InputError, "vertex index do not support pair-unique attributes");
        }
    }
    return false;
}

bool VertexIndex::Update(KvTransaction& txn, const Value& old_key, const Value& new_key,
                         int64_t vid) {
    if (!Delete(txn, old_key, vid)) {
        return false;
    }
    return Add(txn, new_key, vid);
}

bool VertexIndex::UniqueIndexConflict(KvTransaction& txn, const Value& k) {
    if (type_ != IndexType::GlobalUniqueIndex) {
        THROW_CODE(InputError, "vertex UniqueIndexConflict only can be used in unique index.");
    }
    if (k.Size() > GetMaxVertexIndexKeySize()) {
        return true;
    }
    Value v;
    return table_->GetValue(txn, k, v);
}

bool VertexIndex::Add(KvTransaction& txn, const Value& k, int64_t vid) {
    switch (type_) {
    case IndexType::GlobalUniqueIndex:
        {
            if (k.Size() > GetMaxVertexIndexKeySize())
                THROW_CODE(InputError, "Vertex unique index value [{}] is too long.", k.AsString());
            return table_->AddKV(txn, Value::ConstRef(k), Value::ConstRef(vid));
        }
    case IndexType::NonuniqueIndex:
        {
            Value key = CutKeyIfLongOnlyForNonUniqueIndex(k);
            VertexIndexIterator it = GetUnmanagedIterator(txn, key, key, vid);
            if (!it.IsValid() || it.KeyOutOfRange()) {
                if (!it.PrevKV() || !it.KeyEquals(key)) {
                    // create a new VertexIndexValue
                    VertexIndexValue iv;
                    uint8_t r = iv.InsertVid(vid);
                    FMA_DBG_CHECK_NEQ(r, 0);
                    bool r2 = table_->AddKV(txn, iv.CreateKey(key), iv.GetBuf());
                    FMA_DBG_ASSERT(r2);
                    return true;
                }
            }
            // found an existing VertexIndexValue
            uint8_t ret = it.iv_.InsertVid(vid);
            if (it.iv_.TooLarge()) {
                VertexIndexValue right = it.iv_.SplitRightHalf();
                // remove the original VertexIndexValue
                it.it_->DeleteKey();
                // insert the left half VertexIndexValue
                bool r = table_->AddKV(txn, it.iv_.CreateKey(key), it.iv_.GetBuf());
                FMA_DBG_ASSERT(r);
                // insert the right half VertexIndexValue
                r = table_->AddKV(txn, right.CreateKey(key), right.GetBuf());
                FMA_DBG_ASSERT(r);
            } else {
                if (ret == 1) {
                    // the key does not change, so just update the VertexIndexValue
                    it.it_->SetValue(it.iv_.GetBuf());
                } else if (ret == 2) {
                    // the key changes so we need to remove the original VertexIndexValue
                    it.it_->DeleteKey();
                    // then insert the VertexIndexValue with an updated key
                    bool r = table_->AddKV(txn, it.iv_.CreateKey(key), it.iv_.GetBuf());
                    FMA_DBG_ASSERT(r);
                } else {
                    // vid already exists
                    return false;
                }
            }
            return true;
        }
    case IndexType::PairUniqueIndex:
        {
            THROW_CODE(InputError, "vertex index do not support pair-unique attributes");
        }
    }
    return false;
}

size_t VertexIndex::GetMaxVertexIndexKeySize() {
    size_t key_size = 0;
    switch (type_) {
    case IndexType::GlobalUniqueIndex:
        {
            key_size = _detail::MAX_KEY_SIZE;
            break;
        }
    case IndexType::NonuniqueIndex:
        {
            key_size = _detail::MAX_KEY_SIZE - _detail::VID_SIZE;
            break;
        }
    case IndexType::PairUniqueIndex:
        {
            THROW_CODE(InputError, "vertex index do not support pair-unique type");
        }
    }
    return key_size;
}

Value VertexIndex::CutKeyIfLongOnlyForNonUniqueIndex(const lgraph::Value& k) {
    if (type_ != IndexType::NonuniqueIndex) return Value::ConstRef(k);
    size_t key_size = GetMaxVertexIndexKeySize();
    if (k.Size() < key_size) return Value::ConstRef(k);
    return Value(k.Data(), key_size);
}

}  // namespace lgraph
