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

#include "core/graph_edge_iterator.h"
#include "core/transaction.h"

namespace lgraph {
namespace graph {

template <PackType ET>
EdgeIterator<ET>::EdgeIterator(::lgraph::Transaction* txn, KvTable& table, const EdgeUid& euid,
                               bool closest)
    : IteratorBase(txn), it_(table.GetIterator(txn->GetTxn())), impl_(*it_) {
    impl_.Goto(euid, closest);
}

template EdgeIterator<PackType::IN_EDGE>::EdgeIterator(::lgraph::Transaction* txn, KvTable& table,
                                                       const EdgeUid& euid, bool closest);

template EdgeIterator<PackType::OUT_EDGE>::EdgeIterator(::lgraph::Transaction* txn, KvTable& table,
                                                        const EdgeUid& euid, bool closest);

}  // namespace graph
}  // namespace lgraph
