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
#include "fma-common/configuration.h"
#include "gtest/gtest.h"

#include "core/graph.h"
#include "core/graph_vertex_iterator.h"
#include "./ut_utils.h"

using namespace fma_common;
using namespace lgraph;
using namespace lgraph::graph;

class TestGraphVertexIterator : public TuGraphTest {};

static Value GenProp(size_t s, char c) {
    Value v(s);
    memset(v.Data(), c, s);
    return v;
}

TEST_F(TestGraphVertexIterator, GraphVertexIterator) {
    size_t vpsize = 1024;
    size_t nv = 10;
    size_t epv = 1024;
    int argc = _ut_argc;
    char** argv = _ut_argv;
    Configuration config;
    config.Add(vpsize, "vpsize", true).Comment("Size of vertex property");
    config.Add(vpsize, "epsize", true).Comment("Size of edge property");
    config.Add(nv, "nv", true).Comment("Number of vertex");
    config.Add(epv, "epv", true).Comment("Edges per vertex");
    config.ParseAndFinalize(argc, argv);

    auto store = std::make_unique<LMDBKvStore>("./testdb", 1 << 30, false);
    // clear old data
    auto txn = store->CreateWriteTxn();
    store->DropAll(*txn);
    txn->Commit();

    // open table
    txn = store->CreateWriteTxn();
    auto graph_table = Graph::OpenTable(*txn, *store, "graph");
    auto meta_table =
        store->OpenTable(*txn, "meta", true, lgraph::ComparatorDesc::DefaultComparator());
    Graph graph(*txn, std::move(graph_table), std::move(meta_table));
    txn->Commit();

    // create graph
    {
        UT_LOG() << "Test vertex iterator";
        txn = store->CreateWriteTxn();
        {
            UT_EXPECT_EQ(graph.AddVertex(*txn, GenProp(100, 'a')), 0);
            auto it_a = graph.GetUnmanagedVertexIterator(txn.get());
            UT_EXPECT_TRUE(it_a.IsValid());
            UT_EXPECT_EQ(it_a.GetId(), 0);
            UT_EXPECT_TRUE(graph.SetVertexProperty(*txn, 0, GenProp(5000, 'a')));
            it_a.RefreshContentIfKvIteratorModified();
            // after refreshing, the iterator should become unmodified
            it_a.RefreshContentIfKvIteratorModified();
            UT_EXPECT_EQ(it_a.GetProperty().AsString(), std::string(5000, 'a'));
            UT_EXPECT_EQ(graph.AddVertex(*txn, GenProp(1000, 'b')), 1);
            auto it_b = graph.GetUnmanagedVertexIterator(txn.get(), 1);
            UT_EXPECT_EQ(graph.AddVertex(*txn, GenProp(2096, 'c')), 2);
            it_b.RefreshContentIfKvIteratorModified();
            UT_EXPECT_TRUE(it_b.IsValid());
            UT_EXPECT_EQ(it_b.GetId(), 1);
            UT_EXPECT_EQ(it_b.GetProperty().AsString(), std::string(1000, 'b'));
            graph.DeleteVertex(*txn, it_b);
            UT_EXPECT_TRUE(it_b.IsValid());
            UT_EXPECT_EQ(it_b.GetId(), 2);
            UT_EXPECT_EQ(it_b.GetProperty().AsString(), std::string(2096, 'c'));
            UT_EXPECT_TRUE(graph.SetVertexProperty(*txn, 0, GenProp(10, 'a')));
            UT_EXPECT_TRUE(graph.SetVertexProperty(*txn, 2, GenProp(10, 'c')));
            UT_EXPECT_TRUE(graph.SetVertexProperty(*txn, 2, GenProp(0, 'c')));
            UT_EXPECT_TRUE(graph.SetVertexProperty(*txn, 2, GenProp(10, 'c')));
            it_b.RefreshContentIfKvIteratorModified();
            UT_EXPECT_EQ(it_b.GetId(), 2);
            UT_EXPECT_EQ(it_b.GetProperty().AsString(), std::string(10, 'c'));
        }
        txn->Abort();
    }
}
