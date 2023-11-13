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

EdgeIndexIterator::EdgeIndexIterator(EdgeIndex* idx, Transaction* txn, KvTable& table,
                                     const Value& key_start, const Value& key_end, VertexId vid,
                                     VertexId vid2, LabelId lid, TemporalId tid, EdgeId eid,
                                     IndexType type)
    : IteratorBase(txn),
      index_(idx),
      it_(InitEdgeIndexIterator(txn->GetTxn(), table, key_start, vid, vid2, lid, tid, eid, type)),
      key_end_(InitKeyEndValue(key_end, type)),
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
      it_(InitEdgeIndexIterator(*txn, table, key_start, vid, vid2, lid, tid, eid, type)),
      key_end_(InitKeyEndValue(key_end, type)),
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

void EdgeIndexIterator::CloseImpl() {
    it_->Close();
    valid_ = false;
}
}  // namespace lgraph
