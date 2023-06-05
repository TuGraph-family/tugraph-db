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
#include "fma-common/configuration.h"
#include "gtest/gtest.h"

#include "core/kv_store.h"
#include "core/graph.h"
#include "./test_tools.h"
#include "./ut_utils.h"

using namespace fma_common;
using namespace lgraph;
using namespace lgraph::graph;

class TestGraph : public TuGraphTest {
 protected:
    void SetUp() {
        vsize = 1;
        esize = 1;
        number_vertex = 10;
        edge_per_vertex = 1024;
    }

    int vsize;
    int esize;
    int number_vertex;
    int edge_per_vertex;
};

static Value GenProp(size_t s, size_t seed) {
    Value v(s);
    char* str = v.Data();
    for (size_t i = 0; i < s; i++) {
        str[i] = '0' + (char)(i + seed % 10);
    }
    return v;
}

static void DumpTable(KvTransaction& txn, KvTable& t, bool track_incoming) {
    t.Print(
        txn, [](const Value& k) -> std::string { return k.AsString(); },
        [track_incoming](const Value& v) -> std::string { return v.AsString(); });
}

TEST_F(TestGraph, Graph) {
    size_t vpsize = 1024;
    size_t epsize = 1024;
    size_t nv = 10;
    size_t epv = 1024;
    vpsize = vsize;
    epsize = esize;
    nv = number_vertex;
    epv = edge_per_vertex;
    int argc = _ut_argc;
    char** argv = _ut_argv;
    Configuration config;
    config.Add(vpsize, "vpsize", true).Comment("Size of vertex property");
    config.Add(vpsize, "epsize", true).Comment("Size of edge property");
    config.Add(nv, "nv", true).Comment("Number of vertex");
    config.Add(epv, "epv", true).Comment("Edges per vertex");
    config.ParseAndFinalize(argc, argv);

    KvStore store("./testdb", 1 << 30, false);
    // clear old data
    KvTransaction txn = store.CreateWriteTxn();
    store.DropAll(txn);
    txn.Commit();

    // open table
    txn = store.CreateWriteTxn();
    KvTable graph_table = Graph::OpenTable(txn, store, "graph");
    KvTable meta_table =
        store.OpenTable(txn, "meta", true, lgraph::ComparatorDesc::DefaultComparator());
    Graph graph(txn, graph_table, & meta_table);
    txn.Commit();

    UT_LOG() << "Testing iterator refresh";
    {
        UT_LOG() << "\tTesting AddVertex";
        txn = store.CreateWriteTxn();
        KvTable graph_table = Graph::OpenTable(txn, store, "graph");
        Graph graph(txn, graph_table, &meta_table);
        std::vector<VertexIterator> vits;
        for (size_t i = 0; i < nv; i++) {
            graph.AddVertex(txn, GenProp(i * 100, i));
            if (i >= nv / 2) {
                vits.push_back(graph.GetUnmanagedVertexIterator(&txn, i, false));
            }
        }
        VertexId vid = nv / 2;
        for (auto& it : vits) {
            UT_EXPECT_TRUE(it.IsValid());
            it.RefreshContentIfKvIteratorModified();
            UT_EXPECT_TRUE(it.IsValid());
            UT_EXPECT_EQ(it.GetId(), vid);
            UT_EXPECT_EQ(it.GetNumOutEdges(), 0);
            UT_EXPECT_EQ(it.GetNumInEdges(), 0);
            UT_EXPECT_EQ(it.GetProperty().AsString(), GenProp(vid * 100, vid).AsString());
            vid++;
        }
        UT_EXPECT_EQ(vid, nv);
        vits.clear();

        UT_LOG() << "\tTesting AddVertex";
        txn.Abort();
    }

    // create graph
    txn = store.CreateWriteTxn();
    for (size_t i = 0; i < nv; i++) {
        UT_EXPECT_EQ(graph.AddVertex(txn, GenProp(vpsize, i)), i);
    }
    for (size_t i = 0; i < nv; i++) {
        for (size_t j = 0; j < epv; j++) {
            UT_EXPECT_TRUE(graph.AddEdge(txn, i, 0, j % nv, GenProp(epsize, i + j), {}) ==
                           EdgeUid(i, j % nv, 0, 0, j / nv));
        }
    }
    txn.Commit();

    // delete
    {
        txn = store.CreateWriteTxn();
        {
            size_t nd = 0;
            graph::OutEdgeIterator oeit = graph.GetUnmanagedOutEdgeIterator(&txn, EdgeUid(), true);
            EdgeId eid = 0;
            VertexId dstid = 0;
            EdgeId max_eid = epv / nv + ((VertexId)(epv % nv) > dstid ? 1 : 0);
            size_t self_reference = 0;
            while (oeit.IsValid()) {
                UT_EXPECT_EQ(oeit.GetDst(), dstid);
                UT_EXPECT_EQ(oeit.GetEdgeId(), eid);
                if (dstid == 0) self_reference++;
                graph.DeleteEdge(txn, oeit);
                nd++;
                eid++;
                if (eid == max_eid) {
                    eid = 0;
                    dstid++;
                    max_eid = epv / nv + ((VertexId)(epv % nv) > dstid ? 1 : 0);
                }
            }
            UT_EXPECT_EQ(nd, epv);
            size_t ni = 0, no = 0;
            auto on_edge_deleted = [&](bool is_out_edge, const graph::EdgeValue& edge_value){
                if (is_out_edge) {
                    no += edge_value.GetEdgeCount();
                } else {
                    ni += edge_value.GetEdgeCount();
                }
            };
            UT_EXPECT_TRUE(graph.DeleteVertex(txn, 0, on_edge_deleted));
            UT_EXPECT_EQ(ni, (nv - 1) * (epv / nv + (epv % nv != 0)));
            UT_EXPECT_EQ(no, 0);

            OutEdgeIterator oeit2 =
                graph.GetUnmanagedOutEdgeIterator(&txn, EdgeUid(nv - 1, nv - 1, 0, 0, 0), true);
            UT_EXPECT_TRUE(oeit2.IsValid());
            nd = 0;
            while (oeit2.IsValid()) {
                UT_EXPECT_EQ(oeit2.GetDst(), nv - 1);
                UT_EXPECT_EQ(oeit2.GetEdgeId(), nd);
                graph.DeleteEdge(txn, oeit2);
                nd++;
            }
            UT_EXPECT_EQ(nd, epv / nv);
            UT_EXPECT_TRUE(!oeit2.IsValid());
            oeit2.Goto(EdgeUid(nv - 1, 1, 0, 0, 0), false);
            UT_EXPECT_TRUE(oeit2.IsValid());
            graph.DeleteEdge(txn, oeit2);
            if (epv >= nv + 2) {
                UT_EXPECT_TRUE(oeit2.IsValid());
                UT_EXPECT_EQ(oeit2.GetDst(), 1);
                UT_EXPECT_EQ(oeit2.GetEdgeId(), 1);
            } else if (epv >= 2) {
                UT_EXPECT_TRUE(oeit2.IsValid());
                UT_EXPECT_EQ(oeit2.GetDst(), 2);
                UT_EXPECT_EQ(oeit2.GetEdgeId(), 0);
            } else {
                UT_EXPECT_TRUE(!oeit2.IsValid());
            }
        }
        txn.Commit();
    }
}

TEST_F(TestGraph, DeleteVertexRemoveEdges) {
    // deleting a vertex should remove all edges
    // that have the vertex as source or destination
    AutoCleanDir _dir("./testdb");
    KvStore store("./testdb", 1 << 30, false);
    
    // open table
    auto txn = store.CreateWriteTxn();
    KvTable graph_table = Graph::OpenTable(txn, store, "graph");
    KvTable meta_table =
        store.OpenTable(txn, "meta", true, lgraph::ComparatorDesc::DefaultComparator());
    Graph graph(txn, graph_table, &meta_table);
    txn.Commit();

    // create graph
    txn = store.CreateWriteTxn();
    {
        size_t vpsize = 10;
        UT_EXPECT_EQ(graph.AddVertex(txn, GenProp(vpsize, 0)), 0);
        UT_EXPECT_EQ(graph.AddVertex(txn, GenProp(vpsize, 1)), 1);
        UT_EXPECT_EQ(graph.AddVertex(txn, GenProp(vpsize, 2)), 2);
        Value prop = GenProp(vpsize, 2);
        // add edges from 0 to 1
        // add edge with different tids
        UT_EXPECT_TRUE(graph.AddEdge(txn, EdgeSid(0, 1, 0, 1000), prop, {}) ==
                       EdgeUid(0, 1, 0, 1000, 0));
        UT_EXPECT_TRUE(graph.AddEdge(txn, EdgeSid(0, 2, 0, 2000), prop, {}) ==
                       EdgeUid(0, 2, 0, 2000, 0));
        UT_EXPECT_TRUE(graph.AddEdge(txn, EdgeSid(0, 1, 0, 3000), prop, {}) ==
                       EdgeUid(0, 1, 0, 3000, 0));
        // delete vertex 0
        size_t ni, no;
        UT_EXPECT_TRUE(graph.DeleteVertex(txn, 0, &ni, &no));
        UT_EXPECT_EQ(ni, 0);
        UT_EXPECT_EQ(no, 3);
        // check that edges are gone
        UT_EXPECT_TRUE(
            !graph.GetUnmanagedInEdgeIterator(&txn, EdgeUid(0, 1, 0, 0, 0), true).IsValid());
        UT_EXPECT_TRUE(
            !graph.GetUnmanagedInEdgeIterator(&txn, EdgeUid(0, 2, 0, 0, 0), true).IsValid());
        auto vit = graph.GetUnmanagedVertexIterator(&txn, 1);
        auto eit = vit.GetInEdgeIterator();
        UT_EXPECT_FALSE(eit.IsValid());
        auto vit2 = graph.GetUnmanagedVertexIterator(&txn, 2);
        auto eit2 = vit2.GetInEdgeIterator();
        UT_EXPECT_FALSE(eit2.IsValid());
    }
    txn.Commit();
}