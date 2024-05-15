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
    }
    desc.data_types = dt;
    return store.OpenTable(txn, name, true, desc);
}

void CompositeIndex::_AppendCompositeIndexEntry(KvTransaction& txn, const Value& k, VertexId vid) {
    FMA_DBG_ASSERT(type_ == CompositeIndexType::UniqueIndex);
    if (k.Size() >= _detail::MAX_KEY_SIZE) {
        THROW_CODE(ReachMaximumCompositeIndexField, "The key of the composite index is "
                   "too long and exceeds the limit.");
    }
    table_->AppendKv(txn, k, Value::ConstRef(vid));
}

bool CompositeIndex::Add(KvTransaction& txn, const Value& k, int64_t vid) {
    if (k.Size() >= _detail::MAX_KEY_SIZE) {
        THROW_CODE(ReachMaximumCompositeIndexField, "The key of the composite index is "
                   "too long and exceeds the limit.");
    }
    switch (type_) {
    case CompositeIndexType::UniqueIndex:
        {
            return table_->AddKV(txn, k, Value::ConstRef(vid));
        }
    }
    return false;
}

CompositeIndexIterator::CompositeIndexIterator(lgraph::CompositeIndex *idx,
                                               lgraph::Transaction *txn,
                                               lgraph::KvTable &table,
                                               const lgraph::Value &key_start,
                                               const lgraph::Value &key_end,
                                               lgraph::VertexId vid, CompositeIndexType type)
    : IteratorBase(txn),
      index_(idx),
      it_(table.GetClosestIterator(txn->GetTxn(),
                                   type == CompositeIndexType::UniqueIndex ? key_start
                                       : Value())),
      key_end_(type == CompositeIndexType::UniqueIndex ? Value::MakeCopy(key_end)
                                                    : Value()),
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
                                                                           : Value())),
      key_end_(type == CompositeIndexType::UniqueIndex ? Value::MakeCopy(key_end)
                                                       : Value()),
      valid_(false),
      pos_(0),
      type_(type) {
    if (!it_->IsValid() || KeyOutOfRange()) {
        return;
    }
    LoadContentFromIt();
}

CompositeIndexIterator::CompositeIndexIterator(lgraph::CompositeIndexIterator &&rhs)
    : IteratorBase(std::move(rhs)),
    index_(rhs.index_),
    it_(std::move(rhs.it_)),
    key_end_(std::move(rhs.key_end_)),
    curr_key_(std::move(rhs.curr_key_)),
    valid_(rhs.valid_),
    pos_(rhs.pos_),
    vid_(rhs.vid_),
    type_(rhs.type_) {
    rhs.valid_ = false;
}

Value CompositeIndexIterator::GetKey() const {
    switch (type_) {
    case CompositeIndexType::UniqueIndex:
        {
            return it_->GetKey();
        }
    }
    return {};
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
    }
}

bool CompositeIndexIterator::KeyOutOfRange() {
    if (key_end_.Empty()) return false;
    return it_->GetTable().CompareKey(it_->GetTxn(), it_->GetKey(), key_end_) > 0;
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
        }
        // now it_ points to a valid position, but not necessary the right one
    }
}

bool CompositeIndexIterator::Next() {
    valid_ = false;
    if (!it_->Next() || KeyOutOfRange()) {
        return false;
    }
    LoadContentFromIt();
    return true;
}

}  // namespace lgraph
