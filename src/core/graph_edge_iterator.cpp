/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "core/graph_edge_iterator.h"
#include "core/transaction.h"

namespace lgraph {
namespace graph {

template <PackType ET>
EdgeIterator<ET>::EdgeIterator(::lgraph::Transaction* txn, KvTable& table, const EdgeUid& euid,
                               bool closest)
    : IteratorBase(txn), it_(txn->GetTxn(), table), impl_(it_) {
    impl_.Goto(euid, closest);
}

template EdgeIterator<PackType::IN_EDGE>::EdgeIterator(::lgraph::Transaction* txn, KvTable& table,
                                                       const EdgeUid& euid, bool closest);

template EdgeIterator<PackType::OUT_EDGE>::EdgeIterator(::lgraph::Transaction* txn, KvTable& table,
                                                        const EdgeUid& euid, bool closest);

}  // namespace graph
}  // namespace lgraph
