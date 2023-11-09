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
VertexIndexIterator::VertexIndexIterator(VertexIndex* idx, Transaction* txn, KvTable& table,
                                         const Value& key_start,
                                         const Value& key_end, VertexId vid, bool unique)
    : IteratorBase(txn),
      index_(idx),
      it_(table.GetClosestIterator(
          txn->GetTxn(), unique ? key_start : _detail::PatchKeyWithVid(key_start, vid))),
      key_end_(unique ? Value::MakeCopy(key_end)
               : _detail::PatchKeyWithVid(key_end, -1)),
      iv_(),
      valid_(false),
      pos_(0),
      unique_(unique) {
    if (!it_->IsValid() || KeyOutOfRange()) {
        return;
    }
    LoadContentFromIt();
}

VertexIndexIterator::VertexIndexIterator(VertexIndex* idx, KvTransaction* txn, KvTable& table,
                                         const Value& key_start,
                                         const Value& key_end, VertexId vid, bool unique)
    : IteratorBase(nullptr),
      index_(idx),
      it_(table.GetClosestIterator(
          *txn, unique ? key_start : _detail::PatchKeyWithVid(key_start, vid))),
      key_end_(unique ? Value::MakeCopy(key_end) : _detail::PatchKeyWithVid(key_end, -1)),
      iv_(),
      valid_(false),
      pos_(0),
      unique_(unique) {
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
      unique_(rhs.unique_) {
    rhs.valid_ = false;
}

void VertexIndexIterator::CloseImpl() {
    it_->Close();
    valid_ = false;
}
}  // namespace lgraph
