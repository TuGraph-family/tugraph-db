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

#include "core/kv_store.h"
#include "core/graph_vertex_iterator.h"
#include "core/transaction.h"

namespace lgraph {
namespace graph {
VertexIterator::VertexIterator(::lgraph::Transaction* txn, KvTable& tbl, VertexId vid, bool closest)
    : IteratorBase(txn), it_(tbl.GetIterator(txn_->GetTxn())), impl_(*it_) {
    impl_.Goto(vid, closest);
}

VertexIterator::VertexIterator(KvTransaction* txn, KvTable& tbl, VertexId vid, bool closest)
    : IteratorBase(nullptr), it_(tbl.GetIterator(*txn)), impl_(*it_) {
    impl_.Goto(vid, closest);
}

VertexIterator::VertexIterator(VertexIterator&& rhs)
    : IteratorBase(std::move(rhs)), it_(std::move(rhs.it_)), impl_(std::move(rhs.impl_)) {
    impl_.SetItPtr(it_.get());
}

void VertexIterator::CloseImpl() { impl_.Close(); }
}  // namespace graph
}  // namespace lgraph
