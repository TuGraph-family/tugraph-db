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

#include <string>
#include "fma-common/logging.h"
#include "fma-common/configuration.h"

#include "core/graph_edge_iterator.h"
#include "core/kv_store.h"
#include "./random_port.h"
#include "gtest/gtest.h"
#include "./ut_utils.h"
using namespace fma_common;
using namespace lgraph;
using namespace lgraph::graph;

class TestGraphEdgeIterator : public TuGraphTest {};

static Value GenProp(size_t size) {
    Value v(size);
    memset(v.Data(), size % 128, v.Size());
    return v;
}

TEST_F(TestGraphEdgeIterator, GraphEdgeIterator) {
    int n_edges = 1000;
    RandomSeed seed(0);
    int argc = _ut_argc;
    char** argv = _ut_argv;
    fma_common::Configuration config;
    config.Add(n_edges, "n_edges", true).Comment("Number of edges to add in the test");
    config.ParseAndFinalize(argc, argv);
    UT_LOG() << "Testing OutEdgeIterator";
    auto store = std::make_unique<LMDBKvStore>("./testdb", 1 << 30, false);
    auto txn = store->CreateWriteTxn();
    store->DropAll(*txn);
    auto table = store->OpenTable(*txn, "graph", true, ComparatorDesc::DefaultComparator());

    // first, we create a packed data node
    VertexId vid = 10000LL;
    PackedDataValue pdv;
    std::string vidstr = std::to_string(vid);
    VertexValue vov(Value::ConstRef(vidstr));
    EdgeValue iev;
    EdgeValue oev;
    pdv = PackedDataValue::PackData(vov, oev, iev);
    table->AddKV(*txn, KeyPacker::CreatePackedDataKey(vid), pdv.GetBuf());
    txn->Commit();
    {
        // empty database
        auto txn = store->CreateReadTxn();
        OutEdgeIterator oeit(txn.get(), *table, EdgeUid(vid, 0, 0, 0, 0), true);
        OutEdgeIterator out_edge_it = std::move(oeit);
        UT_EXPECT_TRUE(!out_edge_it.IsValid());
        UT_EXPECT_TRUE(!InEdgeIterator(txn.get(), *table,
                                       EdgeUid(0, vid, 0, 0, 0), true).IsValid());
        out_edge_it.Close();
        txn->Abort();
    }
    {
        // single huge edge
        auto txn = store->CreateWriteTxn();
        auto it = table->GetIterator(*txn);
        EdgeIteratorImpl<PackType::OUT_EDGE>::InsertEdge(EdgeSid(vid, 200, 100, 0), GenProp(31000),
                                                         *it);
        EdgeIteratorImpl<PackType::OUT_EDGE>::InsertEdge(EdgeSid(vid, 300, 0, 0), GenProp(6553),
                                                         *it);
        EdgeIteratorImpl<PackType::IN_EDGE>::InsertEdge(
            EdgeSid(vid, 300, 0, 0), GenProp(6553), *it);
        EdgeIteratorImpl<PackType::IN_EDGE>::InsertEdge(
            EdgeSid(vid, 200, 100, 0), GenProp(31000), *it);
        InEdgeIterator ieit(txn.get(), *table, EdgeUid(200, vid, 100, 0, 0), false);
        UT_EXPECT_TRUE(ieit.IsValid());
        UT_EXPECT_EQ(ieit.GetSrc(), 200);
        UT_EXPECT_EQ(ieit.GetDst(), vid);
        UT_EXPECT_EQ(ieit.GetLabelId(), 100);
        UT_EXPECT_EQ(ieit.GetProperty().AsString(), GenProp(31000).AsString());
        UT_EXPECT_TRUE(ieit.Prev());
        UT_EXPECT_EQ(ieit.GetSrc(), 300);
        UT_EXPECT_EQ(ieit.GetDst(), vid);
        UT_EXPECT_EQ(ieit.GetLabelId(), 0);
        UT_EXPECT_EQ(ieit.GetProperty().AsString(), GenProp(6553).AsString());
        UT_EXPECT_NO_THROW(EdgeIteratorImpl<PackType::OUT_EDGE>::InsertEdge(
            EdgeSid(vid, 200, 100, 0), GenProp(32767), *it));
    }
    {
        // edge with TemporalId
        auto txn = store->CreateWriteTxn();
        auto it = table->GetIterator(*txn);
        PackedDataValue pdv;
        table->AddKV(*txn, KeyPacker::CreatePackedDataKey(vid + 10), pdv.GetBuf());
        std::vector<std::pair<EdgeUid, size_t>> edges;
        EdgeId eid;
        std::vector<std::pair<EdgeUid, size_t>> ins;
        for (int i = 0; i < n_edges; i++) {
            TemporalId tid = rand_r(&seed) % 20;
            VertexId src = rand_r(&seed) % 20;
            size_t psize = rand_r(&seed) % 16;
            eid = EdgeIteratorImpl<PackType::IN_EDGE>::InsertEdge(
                EdgeSid(vid + 10, src, 12, tid), GenProp(psize),
                *it);
            ins.emplace_back(EdgeUid(src, vid + 10, 12, tid, eid), psize);
        }
        typedef std::pair<EdgeUid, size_t> EdgeSizePair;
        std::sort(ins.begin(), ins.end(), [](const EdgeSizePair& lhs, const EdgeSizePair& rhs) {
            return EdgeUid::InEdgeSortOrder()(lhs.first, rhs.first);
        });
        InEdgeIterator ieit(txn.get(), *table, EdgeUid(0, vid + 10, 0, 0, 0), true);
        auto iit = ins.begin();
        while (ieit.IsValid()) {
            // ieit.GetPrimaryId(), ieit.GetSrc(), ieit.GetEdgeId());
            UT_EXPECT_EQ(ieit.GetDst(), vid + 10);
            UT_EXPECT_EQ(ieit.GetLabelId(), iit->first.lid);
            UT_EXPECT_EQ(ieit.GetTemporalId(), iit->first.tid);
            UT_EXPECT_EQ(ieit.GetSrc(), iit->first.src);
            UT_EXPECT_EQ(ieit.GetEdgeId(), iit->first.eid);
            UT_EXPECT_EQ(ieit.GetProperty().AsString(), GenProp(iit->second).AsString());
            ieit.Next();
            iit++;
        }
    }

    {
        // a lot of small edges
        auto txn = store->CreateWriteTxn();
        auto it = table->GetIterator(*txn);
        PackedDataValue pdv;
        table->AddKV(*txn, KeyPacker::CreatePackedDataKey(vid + 1), pdv.GetBuf());
        std::vector<std::pair<EdgeUid, size_t>> edges;
        EdgeId eid = EdgeIteratorImpl<PackType::OUT_EDGE>::InsertEdge(
            EdgeSid(vid + 1, 200, 100, 0), GenProp(1),
            *it);
        edges.emplace_back(EdgeUid(vid + 1, 200, 100, 0, eid), 1);
        // insert from front
        eid = EdgeIteratorImpl<PackType::OUT_EDGE>::InsertEdge(
            EdgeSid(vid + 1, 300, 99, 0), GenProp(2),
            *it);
        edges.emplace_back(EdgeUid(vid + 1, 300, 99, 0, eid), 2);
        // insert from back
        eid = EdgeIteratorImpl<PackType::OUT_EDGE>::InsertEdge(
            EdgeSid(vid + 1, 100, 200, 0), GenProp(3),
            *it);
        edges.emplace_back(EdgeUid(vid + 1, 100, 200, 0, eid), 3);
        // insert into middle
        eid = EdgeIteratorImpl<PackType::OUT_EDGE>::InsertEdge(
            EdgeSid(vid + 1, 400, 99, 0), GenProp(4),
            *it);
        edges.emplace_back(EdgeUid(vid + 1, 400, 99, 0, eid), 4);
        // randomly insert a lot
        std::vector<std::pair<EdgeUid, size_t>> ins;
        for (size_t i = 0; i < n_edges; i++) {
            LabelId lid = rand_r(&seed);
            VertexId src = rand_r(&seed);
            size_t psize = rand_r(&seed) % 16;
            eid = EdgeIteratorImpl<PackType::IN_EDGE>::InsertEdge(
                EdgeSid(vid + 1, src, lid, 0), GenProp(psize),
                *it);
            ins.emplace_back(EdgeUid(src, vid + 1, lid, 0, eid), psize);
        }
        // validate
        typedef std::pair<EdgeUid, size_t> EdgeSizePair;
        std::sort(edges.begin(), edges.end(), [](const EdgeSizePair& lhs, const EdgeSizePair& rhs) {
            return EdgeUid::OutEdgeSortOrder()(lhs.first, rhs.first);
        });
        std::sort(ins.begin(), ins.end(), [](const EdgeSizePair& lhs, const EdgeSizePair& rhs) {
            return EdgeUid::InEdgeSortOrder()(lhs.first, rhs.first);
        });
        OutEdgeIterator oeit(txn.get(), *table, EdgeUid(vid + 1, 0, 0, 0, 0), true);
        auto oit = edges.begin();
        while (oeit.IsValid()) {
            UT_EXPECT_EQ(oeit.GetSrc(), vid + 1);
            UT_EXPECT_EQ(oeit.GetLabelId(), oit->first.lid);
            UT_EXPECT_EQ(oeit.GetDst(), oit->first.dst);
            UT_EXPECT_EQ(oeit.GetEdgeId(), oit->first.eid);
            UT_EXPECT_EQ(oeit.GetProperty().AsString(), GenProp(oit->second).AsString());
            oeit.Next();
            oit++;
        }
        UT_EXPECT_TRUE(oit == edges.end());
        InEdgeIterator ieit(txn.get(), *table, EdgeUid(0, vid + 1, 0, 0, 0), true);
        auto iit = ins.begin();
        while (ieit.IsValid()) {
            UT_EXPECT_EQ(ieit.GetDst(), vid + 1);
            UT_EXPECT_EQ(ieit.GetLabelId(), iit->first.lid);
            UT_EXPECT_EQ(ieit.GetSrc(), iit->first.src);
            UT_EXPECT_EQ(ieit.GetEdgeId(), iit->first.eid);
            UT_EXPECT_EQ(ieit.GetProperty().AsString(), GenProp(iit->second).AsString());
            ieit.Next();
            iit++;
        }
        UT_EXPECT_TRUE(iit == ins.end());
    }

    txn = store->CreateWriteTxn();
    UT_LOG() << "\tSequentail insert";
    int64_t dst = 2000LL;
    LabelId lid = 21;
    try {
        std::vector<OutEdgeIterator> oeits;
        auto it = table->GetIterator(*txn);
        for (int i = 0; i < n_edges; i++) {
            EdgeIteratorImpl<PackType::OUT_EDGE>::InsertEdge(EdgeSid(vid, dst, lid, 0),
                                                             GenProp(i), *it);
            oeits.push_back(OutEdgeIterator(txn.get(), *table,
                                            EdgeUid(vid, dst, lid, 0, i), false));
            UT_EXPECT_TRUE((oeits.end() - 1)->IsValid());
        }
        EdgeId eid = 0;
        for (auto& oeit : oeits) {
            oeit.RefreshContentIfKvIteratorModified();
            UT_EXPECT_TRUE(oeit.IsValid());
            UT_EXPECT_EQ(oeit.GetSrc(), vid);
            UT_EXPECT_EQ(oeit.GetDst(), dst);
            UT_EXPECT_EQ(oeit.GetEdgeId(), eid);
            UT_EXPECT_EQ(oeit.GetProperty().AsString(), GenProp(eid).AsString());
            eid++;
        }
    } catch (std::exception& e) {
        ERR() << e.what();
    }

    UT_LOG() << "Checking Next()";
    int eid = 0;
    OutEdgeIterator out_edge_it(txn.get(), *table, EdgeUid(vid, 0, 0, 0, 0), true);
    out_edge_it.Goto(EdgeUid(vid, dst, lid, 0, 0), true);
    while (out_edge_it.IsValid()) {
        UT_EXPECT_EQ(out_edge_it.GetSrc(), vid);
        UT_EXPECT_EQ(out_edge_it.GetDst(), dst);
        UT_EXPECT_EQ(out_edge_it.GetEdgeId(), eid);
        UT_EXPECT_EQ(out_edge_it.GetProperty().AsString(), GenProp(eid).AsString());
        out_edge_it.Next();
        eid++;
    }
    UT_EXPECT_EQ(eid, n_edges);

    UT_LOG() << "Checking Prev()";
    eid = n_edges - 1;
    out_edge_it.Goto(EdgeUid(vid, dst, lid, 0, eid), false);
    while (eid >= 0) {
        UT_EXPECT_EQ(out_edge_it.GetSrc(), vid);
        UT_EXPECT_EQ(out_edge_it.GetDst(), dst);
        UT_EXPECT_EQ(out_edge_it.GetEdgeId(), eid);
        UT_EXPECT_EQ(out_edge_it.GetProperty().AsString(), GenProp(eid).AsString());
        if (eid == 0)
            out_edge_it.TryPrev();
        else
            out_edge_it.Prev();
        eid--;
    }
    UT_EXPECT_EQ(eid, -1);
    UT_EXPECT_TRUE(out_edge_it.IsValid());
    UT_EXPECT_EQ(out_edge_it.GetSrc(), vid);
    UT_EXPECT_EQ(out_edge_it.GetDst(), dst);
    UT_EXPECT_EQ(out_edge_it.GetEdgeId(), 0);
    UT_EXPECT_EQ(out_edge_it.GetProperty().AsString(), std::string());

    UT_LOG() << "\tGoto";
    out_edge_it.Goto(EdgeUid(vid + 1, 0, lid, 0, 0), true);
    UT_EXPECT_TRUE(!out_edge_it.IsValid());
    out_edge_it.Goto(EdgeUid(vid - 1, 0, lid, 0, 0), true);
    UT_EXPECT_TRUE(!out_edge_it.IsValid());

    out_edge_it.Goto(EdgeUid(vid, 0, lid, 0, 0), true);
    UT_EXPECT_EQ(out_edge_it.GetSrc(), vid);
    UT_EXPECT_EQ(out_edge_it.GetDst(), dst);
    UT_EXPECT_EQ(out_edge_it.GetEdgeId(), 0);

    out_edge_it.Goto(EdgeUid(vid, dst, lid, 0, 30), false);
    UT_EXPECT_EQ(out_edge_it.GetSrc(), vid);
    UT_EXPECT_EQ(out_edge_it.GetDst(), dst);
    UT_EXPECT_EQ(out_edge_it.GetEdgeId(), 30);

    out_edge_it.Goto(EdgeUid(vid, dst, lid, 0, 100000), false);
    UT_EXPECT_TRUE(!out_edge_it.IsValid());

    UT_EXPECT_TRUE(out_edge_it.Goto(EdgeUid(vid, dst, lid, 0, 0), true));
    // test next and prev
    eid = 0;
    while (out_edge_it.IsValid()) {
        UT_EXPECT_EQ(out_edge_it.GetSrc(), vid);
        UT_EXPECT_EQ(out_edge_it.GetDst(), dst);
        UT_EXPECT_EQ(out_edge_it.GetEdgeId(), eid);
        eid++;
        out_edge_it.Next();
    }
    UT_EXPECT_EQ(eid, n_edges);
    eid--;
    UT_EXPECT_TRUE(out_edge_it.Prev());
    while (out_edge_it.IsValid()) {
        UT_EXPECT_EQ(out_edge_it.GetSrc(), vid);
        UT_EXPECT_EQ(out_edge_it.GetDst(), dst);
        UT_EXPECT_EQ(out_edge_it.GetEdgeId(), eid);
        eid--;
        out_edge_it.Prev();
    }
    UT_EXPECT_EQ(eid, -1);

    UT_LOG() << "\tSetProperty";
    out_edge_it.Goto(EdgeUid(vid, dst, lid, 0, 30), false);
    OutEdgeIterator oeit2(txn.get(), *table, EdgeUid(vid, dst, lid, 0, 30), false);
    std::string large_prop(10000, 1);
    out_edge_it.SetProperty(Value::ConstRef(large_prop));
    UT_EXPECT_EQ(out_edge_it.GetSrc(), vid);
    UT_EXPECT_EQ(out_edge_it.GetDst(), dst);
    UT_EXPECT_EQ(out_edge_it.GetEdgeId(), 30);
    UT_EXPECT_EQ(out_edge_it.GetProperty().AsString(), large_prop);
    oeit2.RefreshContentIfKvIteratorModified();
    UT_EXPECT_EQ(oeit2.GetSrc(), vid);
    UT_EXPECT_EQ(oeit2.GetDst(), dst);
    UT_EXPECT_EQ(oeit2.GetEdgeId(), 30);
    UT_EXPECT_EQ(oeit2.GetProperty().AsString(), large_prop);

    out_edge_it.SetProperty(GenProp(30));
    oeit2.RefreshContentIfKvIteratorModified();
    UT_EXPECT_EQ(oeit2.GetSrc(), vid);
    UT_EXPECT_EQ(oeit2.GetDst(), dst);
    UT_EXPECT_EQ(oeit2.GetEdgeId(), 30);
    UT_EXPECT_EQ(oeit2.GetProperty().AsString(), GenProp(30).AsString());

    out_edge_it.Goto(EdgeUid(vid, 0, lid, 0, 0), true);
    eid = 0;
    while (out_edge_it.IsValid()) {
        UT_EXPECT_EQ(out_edge_it.GetSrc(), vid);
        UT_EXPECT_EQ(out_edge_it.GetDst(), dst);
        UT_EXPECT_EQ(out_edge_it.GetEdgeId(), eid);
        UT_EXPECT_EQ(out_edge_it.GetProperty().AsString(), GenProp(eid).AsString());
        out_edge_it.Next();
        eid++;
    }
    UT_EXPECT_EQ(eid, n_edges);

    UT_LOG() << "\tDelete";
    out_edge_it.Goto(EdgeUid(vid, dst, lid, 0, 0), true);
    oeit2.Goto(EdgeUid(vid, dst, lid, 0, 0), true);
    out_edge_it.Delete();
    out_edge_it.Delete();
    UT_EXPECT_EQ(out_edge_it.GetSrc(), vid);
    UT_EXPECT_EQ(out_edge_it.GetDst(), dst);
    UT_EXPECT_EQ(out_edge_it.GetEdgeId(), 2);
    oeit2.RefreshContentIfKvIteratorModified();
    UT_EXPECT_EQ(oeit2.GetSrc(), vid);
    UT_EXPECT_EQ(oeit2.GetDst(), dst);
    UT_EXPECT_EQ(oeit2.GetEdgeId(), 2);
    UT_EXPECT_EQ(oeit2.GetProperty().AsString(), GenProp(2).AsString());

    out_edge_it.Goto(EdgeUid(vid, dst, lid, 0, 30), false);
    oeit2.Goto(EdgeUid(vid, dst, lid, 0, 30), false);
    out_edge_it.Delete();
    UT_EXPECT_EQ(out_edge_it.GetSrc(), vid);
    UT_EXPECT_EQ(out_edge_it.GetDst(), dst);
    UT_EXPECT_EQ(out_edge_it.GetEdgeId(), 31);
    oeit2.RefreshContentIfKvIteratorModified();
    UT_EXPECT_EQ(oeit2.GetSrc(), vid);
    UT_EXPECT_EQ(oeit2.GetDst(), dst);
    UT_EXPECT_EQ(oeit2.GetEdgeId(), 31);
    UT_EXPECT_EQ(oeit2.GetProperty().AsString(), GenProp(31).AsString());

    // random test
    UT_LOG() << "\tRandom";
    std::map<std::pair<VertexId, EdgeId>, size_t> all_pairs;
    out_edge_it.Goto(EdgeUid(vid, 0, 0, 0, 0), true);
    while (out_edge_it.IsValid()) {
        VertexId dst = out_edge_it.GetDst();
        EdgeId eid = out_edge_it.GetEdgeId();
        size_t s = out_edge_it.GetProperty().Size();
        all_pairs.emplace(std::make_pair(dst, eid), s);
        out_edge_it.Next();
    }

    srand(0);
    auto kvit = table->GetIterator(*txn);
    for (int i = 0; i < 10000; i++) {
        VertexId dst = rand_r(&seed);
        EdgeId eid = rand_r(&seed);
        size_t s = rand_r(&seed) % 2000;
        auto it = all_pairs.find(std::make_pair(dst, eid));
        if (it == all_pairs.end()) {
            int insert = (rand_r(&seed) % 2) == 0;
            if (insert) {
                eid = EdgeIteratorImpl<PackType::OUT_EDGE>::InsertEdge(
                    EdgeSid(vid, dst, lid, 0), GenProp(s),
                    *kvit);
                all_pairs.emplace(std::make_pair(dst, eid), s);
            }
        } else {
            out_edge_it.Goto(EdgeUid(vid, dst, lid, 0, eid), false);
            UT_EXPECT_TRUE(out_edge_it.IsValid());
            UT_EXPECT_EQ(out_edge_it.GetSrc(), vid);
            UT_EXPECT_EQ(out_edge_it.GetDst(), dst);
            UT_EXPECT_EQ(out_edge_it.GetEdgeId(), eid);
            UT_EXPECT_EQ(out_edge_it.GetProperty().AsString(), GenProp(it->second).AsString());
            int op = rand_r(&seed) % 3;
            if (op == 0) {
                // delete
                out_edge_it.Delete();
                all_pairs.erase(it);
            } else if (op == 1) {
                // update
                out_edge_it.SetProperty(GenProp(it->second));
                it->second = s;
                UT_EXPECT_TRUE(out_edge_it.IsValid());
            }
        }
    }

    auto it = all_pairs.begin();
    out_edge_it.Goto(EdgeUid(vid, 0, 0, 0, 0), true);
    while (out_edge_it.IsValid()) {
        UT_EXPECT_TRUE(out_edge_it.IsValid());
        UT_EXPECT_EQ(out_edge_it.GetSrc(), vid);
        UT_EXPECT_EQ(out_edge_it.GetDst(), it->first.first);
        UT_EXPECT_EQ(out_edge_it.GetEdgeId(), it->first.second);
        UT_EXPECT_EQ(out_edge_it.GetProperty().AsString(), GenProp(it->second).AsString());
        it++;
        out_edge_it.Next();
    }
    UT_EXPECT_EQ(it, all_pairs.end());
}
