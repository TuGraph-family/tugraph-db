/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "core/kv_store.h"
#include "core/graph_vertex_iterator.h"
#include "core/transaction.h"

namespace lgraph {
namespace graph {
VertexIterator::VertexIterator(::lgraph::Transaction* txn, KvTable& tbl, VertexId vid, bool closest)
    : IteratorBase(txn), it_(txn_->GetTxn(), tbl), impl_(it_) {
    impl_.Goto(vid, closest);
}

VertexIterator::VertexIterator(KvTransaction* txn, KvTable& tbl, VertexId vid, bool closest)
    : IteratorBase(nullptr), it_(*txn, tbl), impl_(it_) {
    impl_.Goto(vid, closest);
}

VertexIterator::VertexIterator(VertexIterator&& rhs)
    : IteratorBase(std::move(rhs)), it_(std::move(rhs.it_)), impl_(std::move(rhs.impl_)) {
    impl_.SetItPtr(&it_);
}

void VertexIterator::CloseImpl() { impl_.Close(); }
}  // namespace graph
}  // namespace lgraph
