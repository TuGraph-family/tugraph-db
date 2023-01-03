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

#include "core/graph.h"

void lgraph::graph::Graph::_ScanAndDelete(
    KvStore& store, KvTransaction& txn,
    const std::function<bool(VertexIterator&)>& should_delete_node,
    const std::function<bool(InEdgeIterator&)>& should_delete_in_edge,
    const std::function<bool(OutEdgeIterator&)>& should_delete_out_edge, size_t& n_modified,
    size_t batch_size) {
    FMA_DBG() << "_ScanAndDelete(batch_size=" << batch_size << ")";
    bool is_vertex = (should_delete_node != nullptr);
    size_t n_node = 0;
    size_t n_edge = 0;
    size_t n_last_commit = 0;
    std::unique_ptr<VertexIterator> vit(new VertexIterator(GetUnmanagedVertexIterator(&txn)));
    while (vit->IsValid()) {
        if (should_delete_node && should_delete_node(*vit)) {
            VertexId vid = vit->GetId();
            // deleting vertex
            n_node++;
            // delete vertex pointed to by vit
            KvIterator& kvit = vit->GetIt();
            while (kvit.IsValid() && KeyPacker::GetFirstVid(kvit.GetKey()) == vid) {
                kvit.DeleteKey();
            }
            vit->GotoClosestVertex(vid + 1);
        } else {
            if (should_delete_out_edge) {
                for (auto eit = vit->GetOutEdgeIterator(); eit.IsValid();) {
                    if (should_delete_out_edge(eit)) {
                        n_edge++;
                        eit.Delete();
                    } else {
                        eit.Next();
                    }
                }
            }
            vit->RefreshContentIfKvIteratorModified();
            if (should_delete_in_edge) {
                for (auto eit = vit->GetInEdgeIterator(); eit.IsValid();) {
                    if (should_delete_in_edge(eit)) {
                        n_edge++;
                        eit.Delete();
                    } else {
                        eit.Next();
                    }
                }
            }
            vit->RefreshContentIfKvIteratorModified();
            vit->Next();
        }
        if (n_node + n_edge - n_last_commit >= batch_size) {
#if 0
            VertexId vid = vit->GetId();
            vit.reset();
            txn.Commit();
            txn = store.CreateWriteTxn(false, false);
            n_last_commit = n_node + n_edge;
            FMA_LOG() << "Committed "
                << (is_vertex ? n_last_commit : n_last_commit / 2)
                << " changes.";
            vit.reset(new VertexIterator(GetUnmanagedVertexIterator(&txn, vid, true)));
#else
            n_last_commit = n_node + n_edge;
            FMA_LOG() << "Made " << (is_vertex ? n_last_commit : n_last_commit / 2) << " changes.";
#endif
        }
    }
    if (should_delete_node) {
        n_modified = n_node;
    } else {
        FMA_DBG_CHECK_EQ(n_node, 0);
        n_modified = n_edge / 2;
    }
}
