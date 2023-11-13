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

#include "fma-common/logging.h"
#include "fma-common/string_formatter.h"
#include "gtest/gtest.h"

#include "core/kv_store.h"
#include "core/mock_kv.h"
#include "core/graph.h"
#include "./ut_utils.h"
using namespace fma_common;
using namespace lgraph;
using namespace lgraph::graph;

class TestGraphSimple : public TuGraphTest {};

struct VertexProperty {
    VertexProperty() {}
    VertexProperty(uint64_t u, int t) : uid(u), type(t) {}
    explicit VertexProperty(const Value& v) {
        memcpy(&uid, v.Data(), sizeof(uid));
        memcpy(&type, v.Data() + sizeof(uid), sizeof(type));
    }

    uint64_t uid{};
    int type{};
};

typedef double EdgeProperty;

void DumpGraph(KvTransaction& txn, Graph& g) {
    VertexIterator it = g.GetUnmanagedVertexIterator(&txn);
    while (it.IsValid()) {
        std::string line;
        VertexId vid = it.GetId();
        auto vv = it.GetProperty();
        VertexProperty vp(vv);
        line.append(StringFormatter::Format("[{} : ({}, {})] \n\t--> ", vid, vp.uid, vp.type));
        OutEdgeIterator eit = it.GetOutEdgeIterator();
        while (eit.IsValid()) {
            VertexId dst = eit.GetDst();
            EdgeProperty ep = eit.GetProperty().AsType<EdgeProperty>();
            line.append(StringFormatter::Format(" [{} : {}]", dst, ep));
            eit.Next();
        }
        eit.Close();
        line.append("\n\t<-- ");
        InEdgeIterator iit = it.GetInEdgeIterator();
        while (iit.IsValid()) {
            VertexId src = iit.GetSrc();
            EdgeProperty ep = iit.GetProperty().AsType<EdgeProperty>();
            line.append(StringFormatter::Format(" [{} : {}]", src, ep));
            iit.Next();
        }
        iit.Close();
        UT_LOG() << line;
        it.Next();
    }
}

TEST_F(TestGraphSimple, GraphSimple) {
    AutoCleanDir _dir("./testdb");
    auto store = std::make_unique<LMDBKvStore>("./testdb", 1 << 30, false);

    // clear old data
    auto txn = store->CreateWriteTxn();
    store->DeleteTable(*txn, "graph");
    txn->Commit();

    // open table
    txn = store->CreateWriteTxn();
    auto graph_table = Graph::OpenTable(*txn, *store, "graph");
    auto meta_table =
        store->OpenTable(*txn, "meta", true, lgraph::ComparatorDesc::DefaultComparator());
    txn->Commit();

    // create graph
    txn = store->CreateWriteTxn();
    Graph graph(*txn, std::move(graph_table), std::move(meta_table));

    VertexProperty vp(0, 0);
    VertexId vid = 0;
    Value prop(&vp, sizeof(vp));
    vid = graph.AddVertex(*txn, prop);
    UT_EXPECT_EQ(int64_t(vid), 0);
    vp.uid = 1;
    vp.type = 1;
    vid = graph.AddVertex(*txn, prop);
    UT_EXPECT_EQ(int64_t(vid), 1);
    vp.uid = 22;
    vp.type = 2;
    vid = graph.AddVertex(*txn, prop);
    UT_EXPECT_EQ(int64_t(vid), 2);
    vp.uid = 333;
    vp.type = 3;
    vid = graph.AddVertex(*txn, prop);
    UT_EXPECT_EQ(int64_t(vid), 3);
    vp.uid = 4444;
    vp.type = 4;
    vid = graph.AddVertex(*txn, prop);
    UT_EXPECT_EQ(int64_t(vid), 4);
    UT_LOG() << "Vertex added:";
    DumpGraph(*txn, graph);

    EdgeProperty ep = 1.2;
    prop = Value(&ep, sizeof(ep));

    EdgeUid eid = graph.AddEdge(*txn, 1, 0, 2, prop);

    DumpGraph(*txn, graph);

    UT_EXPECT_EQ(eid.eid, 0);
    ep = 2.2;
    eid = graph.AddEdge(*txn, 2, 0, 2, prop);
    UT_EXPECT_EQ(eid.eid, 0);
    ep = 2.32;
    eid = graph.AddEdge(*txn, 2, 2, 3, prop);
    UT_EXPECT_EQ(eid.eid, 0);
    ep = 2.31;
    eid = graph.AddEdge(*txn, 2, 1, 3, prop);
    ep = 2.311;
    UT_EXPECT_EQ(eid.eid, 0);
    eid = graph.AddEdge(*txn, 2, 1, 3, prop);
    UT_EXPECT_EQ(eid.eid, 1);
    ep = 2.33;
    eid = graph.AddEdge(*txn, 2, 3, 3, prop);
    UT_EXPECT_EQ(eid.eid, 0);
    ep = 2.30;
    eid = graph.AddEdge(*txn, 2, 0, 3, prop);
    UT_EXPECT_EQ(eid.eid, 0);
    ep = 3.4;
    eid = graph.AddEdge(*txn, 3, 0, 4, prop);
    UT_EXPECT_EQ(eid.eid, 0);
    {
        auto vit = graph.GetUnmanagedVertexIterator(txn.get(), 1);
        UT_EXPECT_TRUE(vit.IsValid());
        UT_EXPECT_EQ(vit.GetId(), 1);
    }
    UT_LOG() << "Graph created:";
    DumpGraph(*txn, graph);
    txn->Commit();

    txn = store->CreateReadTxn();
    {
        std::vector<VertexId> dsts;
        dsts = graph.GetUnmanagedVertexIterator(txn.get(), 0).ListDstVids();
        UT_EXPECT_EQ(dsts.size(), 0);
        dsts = graph.GetUnmanagedVertexIterator(txn.get(), 1).ListDstVids();
        UT_EXPECT_EQ(dsts.size(), 1);
        size_t ne = 0;
        dsts = graph.GetUnmanagedVertexIterator(txn.get(), 2).ListDstVids(
            nullptr, nullptr, std::numeric_limits<size_t>::max(), nullptr, &ne);
        UT_EXPECT_EQ(dsts.size(), 2);
        UT_EXPECT_EQ(ne, 6);

        std::vector<VertexId> srcs;
        srcs = graph.GetUnmanagedVertexIterator(txn.get(), 0).ListSrcVids();
        UT_EXPECT_EQ(srcs.size(), 0);
        srcs = graph.GetUnmanagedVertexIterator(txn.get(), 1).ListSrcVids();
        UT_EXPECT_EQ(srcs.size(), 0);
        UT_LOG() << "1 <- " << fma_common::ToString(srcs);
        srcs = graph.GetUnmanagedVertexIterator(txn.get(), 2).ListSrcVids();
        UT_EXPECT_EQ(srcs.size(), 2);
        UT_LOG() << "2 <- " << fma_common::ToString(srcs);
        srcs = graph.GetUnmanagedVertexIterator(txn.get(), 3).ListSrcVids();
        UT_EXPECT_EQ(srcs.size(), 1);
        UT_LOG() << "3 <- " << fma_common::ToString(srcs);
        auto vit = graph.GetUnmanagedVertexIterator(txn.get());
        while (vit.IsValid()) {
            std::string line;
            fma_common::StringFormatter::Append(line, "Incomings of [{}]:", vit.GetId());
            auto eit = vit.GetInEdgeIterator();
            while (eit.IsValid()) {
                fma_common::StringFormatter::Append(line, " [{} : ({})]", eit.GetSrc(),
                                                    eit.GetProperty().AsType<EdgeProperty>());
                eit.Next();
            }
            UT_LOG() << line;
            vit.Next();
        }
    }
    txn->Abort();

    // test SetProperty
    txn = store->CreateWriteTxn();
    {
        std::vector<std::pair<VertexId, VertexProperty>> vertexes;
        std::vector<std::pair<VertexId, std::vector<std::pair<EdgeUid, EdgeProperty>>>> adj_list;
        auto vit = graph.GetUnmanagedVertexIterator(txn.get());
        while (vit.IsValid()) {
            VertexId vid = vit.GetId();
            vertexes.emplace_back(vid, vit.GetProperty().AsType<VertexProperty>());
            std::vector<std::pair<EdgeUid, EdgeProperty>> edges;
            auto eit = vit.GetOutEdgeIterator();
            while (eit.IsValid()) {
                const Value& p = eit.GetProperty();
                edges.emplace_back(eit.GetUid(), p.AsType<EdgeProperty>());
                eit.Next();
            }
            adj_list.emplace_back(vid, std::move(edges));
            vit.Next();
        }
        for (auto& v : vertexes) {
            v.second.uid *= 10;
            v.second.type *= 10;
            graph.SetVertexProperty(*txn, v.first, Value::ConstRef(v.second));
        }
        DumpGraph(*txn, graph);
        for (auto& l : adj_list) {
            VertexId src = l.first;
            for (auto& e : l.second) {
                LabelId lid = e.first.lid;
                VertexId dst = e.first.dst;
                EdgeId eid = e.first.eid;
                EdgeProperty np = e.second * 10;
                graph.SetEdgeProperty(*txn, EdgeUid(src, dst, lid, 0, eid), Value::ConstRef(np));
            }
        }
        UT_LOG() << "Set property";
        DumpGraph(*txn, graph);
    }
    txn->Commit();

    // test remove edge and remove vertex
    txn = store->CreateWriteTxn();
    size_t n_in = 0, n_out = 0;
    auto on_edge_deleted = [&](bool is_out_edge, const graph::EdgeValue& edge_value){
        if (is_out_edge) {
            n_out += edge_value.GetEdgeCount();
        } else {
            n_in += edge_value.GetEdgeCount();
        }
    };
    UT_EXPECT_TRUE(graph.DeleteVertex(*txn, 4, on_edge_deleted));
    UT_EXPECT_EQ(n_in, 1);
    UT_EXPECT_EQ(n_out, 0);
    UT_EXPECT_TRUE(graph.DeleteEdge(*txn, EdgeUid(1, 2, 0, 0, 0)));

    auto vit = graph.GetUnmanagedVertexIterator(txn.get(), 3);
    auto eit = vit.GetInEdgeIterator();
    eit.Next();
    graph.DeleteEdge(*txn, eit);
    vit.RefreshContentIfKvIteratorModified();

    UT_EXPECT_EQ(vit.ListSrcVids().size(), 1);
    UT_EXPECT_TRUE(graph.DeleteEdge(*txn, EdgeUid(2, 3, 2, 0, 0)));
    UT_EXPECT_TRUE(!graph.DeleteEdge(*txn, EdgeUid(2, 3, 2, 0, 1)));  // removed earlier
    UT_LOG() << "After removing edges and vertexes";
    DumpGraph(*txn, graph);

    {
        auto it = graph.GetUnmanagedVertexIterator(txn.get());
        // now there are three vertexes and one edge 2->2
        UT_EXPECT_TRUE(it.IsValid());
        UT_EXPECT_EQ(it.GetId(), VertexId(0));
        UT_EXPECT_TRUE(it.Next());

        UT_EXPECT_TRUE(it.IsValid());
        UT_EXPECT_EQ(it.GetId(), VertexId(1));
        UT_EXPECT_TRUE(it.Next());

        UT_EXPECT_TRUE(it.IsValid());
        auto eit = it.GetOutEdgeIterator();
        UT_EXPECT_EQ(eit.GetSrc(), VertexId(2));
        UT_EXPECT_EQ(eit.GetProperty().AsType<EdgeProperty>(), 2.2 * 10);
        UT_EXPECT_TRUE(eit.Next());
        UT_EXPECT_EQ(it.GetId(), VertexId(2));
        UT_EXPECT_TRUE(it.Next());

        UT_EXPECT_TRUE(it.IsValid());
        UT_EXPECT_EQ(it.GetId(), VertexId(3));
        UT_EXPECT_TRUE(!it.Next());
    }
    UT_EXPECT_ANY_THROW(graph.AddEdge(*txn, -1, 1, 1, prop));
    UT_EXPECT_ANY_THROW(graph.AddEdge(*txn, 1, 1, 5, prop));
}
