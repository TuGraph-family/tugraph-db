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

#include "core/composite_index.h"
#include "core/transaction.h"

namespace lgraph {

namespace _detail {
static Value PatchKeyWithVid(const Value& key, int64_t vid) {
    Value key_vid(key.Size() + VID_SIZE);
    memcpy(key_vid.Data(), key.Data(), key.Size());
    WriteVid(key_vid.Data() + key.Size(), vid);
    return key_vid;
}
}  // namespace _detail

CompositeIndex::CompositeIndex(std::shared_ptr<KvTable> table, std::vector<FieldType> key_types,
    CompositeIndexType type) : table_(std::move(table)), key_types(std::move(key_types)),
    ready_(false), disabled_(false), type_(type) {}

CompositeIndex::CompositeIndex(const CompositeIndex& rhs)
    : table_(rhs.table_),
      key_types(rhs.key_types),
      ready_(rhs.ready_.load()),
      disabled_(rhs.disabled_.load()),
      type_(rhs.type_) {}

std::unique_ptr<KvTable> CompositeIndex::OpenTable(KvTransaction &txn, KvStore &store,
                                                   const std::string &name,
                                                   const std::vector<FieldType> &dt,
                                                   CompositeIndexType type) {
    ComparatorDesc desc;
    switch (type) {
    case CompositeIndexType::UniqueIndex:
        {
            desc.comp_type = ComparatorDesc::COMPOSITE_KEY;
            break;
        }
    case CompositeIndexType::NonUniqueIndex:
        {
            desc.comp_type = ComparatorDesc::COMPOSITE_KEY_AND_VID;
            break;
        }
    }
    desc.data_types = dt;
    return store.OpenTable(txn, name, true, desc);
}

void CompositeIndex::_AppendCompositeIndexEntry(KvTransaction& txn, const Value& k, VertexId vid) {
    FMA_DBG_ASSERT(type_ == CompositeIndexType::UniqueIndex);
    if (k.Size() > _detail::MAX_KEY_SIZE) {
        THROW_CODE(ReachMaximumCompositeIndexField, "The key of the composite index is "
                   "too long and exceeds the limit.");
    }
    table_->AppendKv(txn, k, Value::ConstRef(vid));
}

void CompositeIndex::_AppendNonUniqueCompositeIndexEntry(lgraph::KvTransaction &txn,
                                                         const lgraph::Value &k,
                                                         const std::vector<VertexId> &vids) {
    FMA_DBG_ASSERT(type_ == CompositeIndexType::NonUniqueIndex);
    FMA_DBG_ASSERT(!vids.empty());
    if (k.Size() > _detail::MAX_KEY_SIZE - _detail::VID_SIZE) {
        THROW_CODE(ReachMaximumCompositeIndexField, "The key of the composite index is "
                   "too long and exceeds the limit.");
    }
    size_t vid_per_idv = _detail::NODE_SPLIT_THRESHOLD / _detail::VID_SIZE;
    for (size_t i = 0; i < vids.size(); i += vid_per_idv) {
        size_t end = i + vid_per_idv;
        end = end <= vids.size() ? end : vids.size();
        VertexIndexValue idv(vids.begin() + i, vids.begin() + end);
        Value real_key = idv.CreateKey(k);
        table_->AppendKv(txn, real_key, idv.GetBuf());
    }
}

bool CompositeIndex::Add(KvTransaction& txn, const Value& k, int64_t vid) {
    switch (type_) {
    case CompositeIndexType::UniqueIndex:
        {
            if (k.Size() > _detail::MAX_KEY_SIZE) {
                THROW_CODE(ReachMaximumCompositeIndexField, "The key of the composite index is "
                           "too long and exceeds the limit.");
            }
            return table_->AddKV(txn, k, Value::ConstRef(vid));
        }
    case CompositeIndexType::NonUniqueIndex:
        {
            if (k.Size() > _detail::MAX_KEY_SIZE - _detail::VID_SIZE) {
                THROW_CODE(ReachMaximumCompositeIndexField, "The key of the composite index is "
                           "too long and exceeds the limit.");
            }
            CompositeIndexIterator it = GetUnmanagedIterator(txn, k, k, vid);
            if (!it.IsValid() || it.KeyOutOfRange()) {
                if (!it.PrevKV() || !it.KeyEquals(k)) {
                    // create a new VertexIndexValue
                    VertexIndexValue iv;
                    uint8_t r = iv.InsertVid(vid);
                    FMA_DBG_CHECK_NEQ(r, 0);
                    bool r2 = table_->AddKV(txn, iv.CreateKey(k), iv.GetBuf());
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
                bool r = table_->AddKV(txn, it.iv_.CreateKey(k), it.iv_.GetBuf());
                FMA_DBG_ASSERT(r);
                // insert the right half VertexIndexValue
                r = table_->AddKV(txn, right.CreateKey(k), right.GetBuf());
                FMA_DBG_ASSERT(r);
            } else {
                if (ret == 1) {
                    // the key does not change, so just update the VertexIndexValue
                    it.it_->SetValue(it.iv_.GetBuf());
                } else if (ret == 2) {
                    // the key changes so we need to remove the original VertexIndexValue
                    it.it_->DeleteKey();
                    // then insert the VertexIndexValue with an updated key
                    bool r = table_->AddKV(txn, it.iv_.CreateKey(k), it.iv_.GetBuf());
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

bool CompositeIndex::Delete(lgraph::KvTransaction &txn, const lgraph::Value &k, int64_t vid) {
    CompositeIndexIterator it = GetUnmanagedIterator(txn, Value::ConstRef(k),
                                                     Value::ConstRef(k), vid);
    if (!it.IsValid() || it.KeyOutOfRange()) {
        // no such key_vid
        return false;
    }
    switch (type_) {
    case CompositeIndexType::UniqueIndex:
        {
            it.it_->DeleteKey();
            return true;
        }
    case CompositeIndexType::NonUniqueIndex:
        {
            uint8_t ret = it.iv_.DeleteVidIfExist(vid);
            if (ret == 1) {
                // the key does not change after deletion
                it.it_->SetValue(it.iv_.GetBuf());
            } else if (ret == 2) {
                // the key changes after deletion
                it.it_->DeleteKey();
                if (it.iv_.GetVidCount()) {
                    bool r = it.it_->AddKeyValue(it.iv_.CreateKey(k), it.iv_.GetBuf());
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

CompositeIndexIterator::CompositeIndexIterator(CompositeIndex *idx,
                                               lgraph::Transaction *txn,
                                               lgraph::KvTable &table,
                                               const lgraph::Value &key_start,
                                               const lgraph::Value &key_end,
                                               lgraph::VertexId vid, CompositeIndexType type)
    : IteratorBase(txn),
      index_(idx),
      it_(table.GetClosestIterator(txn->GetTxn(),
                                   type == CompositeIndexType::UniqueIndex ? key_start
                                       : _detail::PatchKeyWithVid(key_start, vid))),
      key_end_(type == CompositeIndexType::UniqueIndex ? Value::MakeCopy(key_end)
                                       : _detail::PatchKeyWithVid(key_start, -1)),
      iv_(),
      valid_(false),
      pos_(0),
      type_(type) {
    if (!it_->IsValid() || KeyOutOfRange()) {
        return;
    }
    LoadContentFromIt();
}

CompositeIndexIterator::CompositeIndexIterator(lgraph::CompositeIndex *idx,
                                               lgraph::KvTransaction *txn,
                                               lgraph::KvTable &table,
                                               const lgraph::Value &key_start,
                                               const lgraph::Value &key_end,
                                               lgraph::VertexId vid,
                                               lgraph::CompositeIndexType type)
    : IteratorBase(nullptr),
      index_(idx),
      it_(table.GetClosestIterator(*txn,
                                   type == CompositeIndexType::UniqueIndex ? key_start
                                   : _detail::PatchKeyWithVid(key_start, vid))),
      key_end_(type == CompositeIndexType::UniqueIndex ? Value::MakeCopy(key_end)
                                   : _detail::PatchKeyWithVid(key_start, -1)),
      iv_(),
      valid_(false),
      pos_(0),
      type_(type) {
    if (!it_->IsValid() || KeyOutOfRange()) {
        return;
    }
    LoadContentFromIt();
}

CompositeIndexIterator::CompositeIndexIterator(CompositeIndexIterator &&rhs) noexcept
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

void CompositeIndexIterator::CloseImpl() {
    it_->Close();
    valid_ = false;
}

Value CompositeIndexIterator::GetKey() const {
    switch (type_) {
    case CompositeIndexType::UniqueIndex:
        {
            return it_->GetKey();
        }
    case CompositeIndexType::NonUniqueIndex:
        {
            Value key_vid = it_->GetKey();
            return Value(key_vid.Data(), key_vid.Size() - _detail::VID_SIZE);
        }
    }
    return {};
}

std::vector<FieldType> CompositeIndexIterator::KeyType() const {
    return index_->key_types;
}

std::vector<FieldData> CompositeIndexIterator::GetKeyData() const {
    return composite_index_helper::CompositeIndexKeyToFieldData(GetKey(), KeyType());
}

void CompositeIndexIterator::LoadContentFromIt() {
    valid_ = true;
    pos_ = 0;
    curr_key_.Copy(GetKey());
    switch (type_) {
    case CompositeIndexType::UniqueIndex:
        {
            vid_ = _detail::GetVid(it_->GetValue().Data());
            break;
        }
    case CompositeIndexType::NonUniqueIndex:
        {
            iv_ = VertexIndexValue(it_->GetValue());
            vid_ = iv_.GetNthVid(pos_);
            break;
        }
    }
}

bool CompositeIndexIterator::KeyOutOfRange() {
    if (key_end_.Empty() || (type_ != CompositeIndexType::UniqueIndex
                             && key_end_.Size() == _detail::VID_SIZE)) return false;
    return it_->GetTable().CompareKey(it_->GetTxn(), it_->GetKey(), key_end_) > 0;
}

bool CompositeIndexIterator::PrevKV() {
    if (!it_->Prev()) {
        return false;
    }
    LoadContentFromIt();
    return true;
}

bool CompositeIndexIterator::KeyEquals(const Value& key) {
    // just need to deal with NonuniqueIndex，because a key has a set of values
    // in this kind index, and other types are guaranteed by lmdb
    if (type_ == CompositeIndexType::NonUniqueIndex) {
        auto key_vid = it_->GetKey();
        if (key_vid.Size() - _detail::VID_SIZE != key.Size()) {
            return false;
        }
        return memcmp(key.Data(), key_vid.Data(), key.Size()) == 0;
    }
    return false;
}

void CompositeIndexIterator::RefreshContentIfKvIteratorModified() {
    if (IsValid() && it_->IsValid() && it_->UnderlyingPointerModified()) {
        valid_ = false;
        switch (type_) {
        case CompositeIndexType::UniqueIndex:
            {
                if (!it_->GotoClosestKey(curr_key_)) return;
                if (KeyOutOfRange()) return;
                LoadContentFromIt();
                return;
            }
        case CompositeIndexType::NonUniqueIndex:
            {
                if (!it_->GotoClosestKey(_detail::PatchKeyWithVid(curr_key_, vid_)))
                    return;
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
        }
        // now it_ points to a valid position, but not necessary the right one
    }
}

bool CompositeIndexIterator::Next() {
    // if we haven't reach the end of the current VertexIndexValue,
    // just move forward
    if (type_ != CompositeIndexType::UniqueIndex && pos_ < iv_.GetVidCount() - 1) {
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

}  // namespace lgraph
