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

#include <random>
#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "fma-common/string_formatter.h"
#include "gtest/gtest.h"

#include "core/graph.h"
#include "core/mock_kv.h"
#include "./ut_utils.h"
using namespace fma_common;
using namespace lgraph;
using namespace lgraph::graph;

struct ParamGraph {
    ParamGraph(int _tc, bool _du, size_t _p, int _vps = 8, int _eps = 8) {
        tc = _tc;
        du = _du;
        p = _p;
        vps = _vps;
        eps = _eps;
    }
    size_t tc;
    bool du;
    size_t p;
    size_t vps;
    size_t eps;
};

class TestPerfGraph : public TuGraphTestWithParam<struct ParamGraph> {};

static size_t power = 12;           // power of nv
static int64_t nv = 1LL << power;   // number of vertex
static size_t ne = 8;               // number of edge per vertex
static size_t v_prop_size = 8;      // vertex property size
static size_t e_prop_size = 8;      // edge property size
static size_t txn_batch = 1 << 10;  // txn batch

static char prop[] = {/* 64 * 8 */
                      "0A0B0C0D0E0F0G0H0I0J0K0L0M0N0O0P0Q0R0S0T0U0V0W0X0Y0Z0a0b0c0d0e0f"
                      "1A1B1C1D1E1F1G1H1I1J1K1L1M1N1O1P1Q1R1S1T1U1V1W1X1Y1Z1a1b1c1d1e1f"
                      "2A2B2C2D2E2F2G2H2I2J2K2L2M2N2O2P2Q2R2S2T2U2V2W2X2Y2Z2a2b2c2d2e2f"
                      "3A3B3C3D3E3F3G3H3I3J3K3L3M3N3O3P3Q3R3S3T3U3V3W3X3Y3Z3a3b3c3d3e3f"
                      "4A4B4C4D4E4F4G4H4I4J4K4L4M4N4O4P4Q4R4S4T4U4V4W4X4Y4Z4a4b4c4d4e4f"
                      "5A5B5C5D5E5F5G5H5I5J5K5L5M5N5O5P5Q5R5S5T5U5V5W5X5Y5Z5a5b5c5d5e5f"
                      "6A6B6C6D6E6F6G6H6I6J6K6L6M6N6O6P6Q6R6S6T6U6V6W6X6Y6Z6a6b6c6d6e6f"
                      "7A7B7C7D7E7F7G7H7I7J7K7L7M7N7O7P7Q7R7S7T7U7V7W7X7Y7Z7a7b7c7d7e7f"};

static void DumpGraph(KvTransaction& txn, graph::Graph& g, size_t b, size_t e) {
    auto it = g.GetUnmanagedVertexIterator(&txn, b);
    size_t count = b;
    while (it.IsValid() && count++ <= e) {
        std::string line;
        VertexId vid = it.GetId();
        auto vp = it.GetProperty();
        auto dsts = it.ListDstVids();
        auto srcs = it.ListSrcVids();
        line.append(StringFormatter::Format("[{} : ({}@{}), in={}, out={}] --> ", vid,
                                            vp.AsString().substr(0, 2), vp.Size(), srcs.size(),
                                            dsts.size()));
        auto eit = it.GetOutEdgeIterator();
        while (eit.IsValid()) {
            auto dst = eit.GetDst();
            auto eid = eit.GetEdgeId();
            auto ep = eit.GetProperty();
            line.append(StringFormatter::Format(" [{}({}) : {}@{}]", dst, eid,
                                                ep.AsString().substr(0, 2), ep.Size()));
            eit.Next();
        }
        UT_LOG() << line;
        it.Next();
    }
}

static void PeekGraph(KvTransaction& txn, Graph& g) { DumpGraph(txn, g, nv / 2 - 1, nv / 2 + 1); }

static bool track_incoming = true;
static bool durable = false;
static double t1, t_add_v, t_add_e, t_read_v, t_read_ve, t_update_v, t_update_ve, t_delete_qtr;

static void PrintReport() {
    std::string report;
    double logical_space = (double)(5 /*vid*/ + v_prop_size) * nv;
    logical_space += (5 /*dst*/ + 4 /*eid*/ + e_prop_size + 5 /*src*/) * ne * nv;
    report.append(
        StringFormatter::Format("(continuous, track_in, durable, nv, ne(-1: variable), txn_batch, "
                                "fixed_size, v_prop_size, e_prop_size, logical_space(MB)) = "
                                "\n\t({}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
                                0, track_incoming, durable, nv, -1, txn_batch, 1, v_prop_size,
                                e_prop_size, logical_space / 1024 / 1024));
    report.append(StringFormatter::Format(
        "\n\t(t_add_v, t_add_e, t_read_v, t_read_ve, "
        "t_update_v, t_update_ve, t_delete_qtr) = "
        "\n\t({}, {}, {}, {}, {}, {}, {})s\n",
        t_add_v, t_add_e, t_read_v, t_read_ve, t_update_v, t_update_ve, t_delete_qtr));
    UT_LOG() << report;
}

int TestPerfGraphContinuous(bool durable) {
    UT_LOG() << "Begining test " << __func__;

    // open store
    auto store = std::make_unique<LMDBKvStore>("./testdb", (size_t)1 << 40, durable);
    // clear old data
    auto txn = store->CreateWriteTxn();
    store->DropAll(*txn);
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
    txn->Commit();

    // add vertex
    VertexId vid;
    Value v_prop(prop, v_prop_size);
    t1 = fma_common::GetTime();
    for (size_t k = 0; k < nv / txn_batch; k++) {
        txn = store->CreateWriteTxn();
        for (size_t i = 0; i < txn_batch; i++) {
            vid = graph.AddVertex(*txn, v_prop);
            UT_EXPECT_EQ(int64_t(vid), txn_batch * k + i);
        }
        txn->Commit();
    }
    t_add_v = fma_common::GetTime() - t1;

    // add edge
    EdgeUid eid;
    Value e_prop(prop + 256, e_prop_size);
    t1 = fma_common::GetTime();
    for (size_t k = 0; k < nv / txn_batch; k++) {
        txn = store->CreateWriteTxn();
        for (size_t i = 0; i < txn_batch; i++) {
            for (size_t j = 0; j < ne; j++) {
                int64_t src = txn_batch * k + i;
                int64_t dst = (src + j + 1) % nv;
                eid = graph.AddEdge(*txn, src, 0, dst, e_prop);
                UT_EXPECT_EQ(eid.eid, 0);  // id of multi-edge
            }
        }
        txn->Commit();
    }
    t_add_e = fma_common::GetTime() - t1;

    UT_LOG() << "graph created";
    txn = store->CreateReadTxn();
    PeekGraph(*txn, graph);
    txn->Abort();

    PrintReport();
    return 0;
}

int TestPerfGraphNoncontinuous(bool durable) {
    UT_LOG() << "Begining test " << __func__;

    // open store
    auto store = std::make_unique<LMDBKvStore>("./testdb", (size_t)1 << 40, durable);
    // clear old data
    auto txn = store->CreateWriteTxn();
    store->DropAll(*txn);
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
    txn->Commit();

    // add vertex
    VertexId vid;
    Value v_prop(prop, v_prop_size);
    t1 = fma_common::GetTime();
    for (size_t k = 0; k < nv / txn_batch; k++) {
        txn = store->CreateWriteTxn();
        for (size_t i = 0; i < txn_batch; i++) {
            vid = graph.AddVertex(*txn, v_prop);
            UT_EXPECT_EQ(int64_t(vid), txn_batch * k + i);
        }
        txn->Commit();
    }
    t_add_v = fma_common::GetTime() - t1;

    // add edge
    // set random, Note: do not use std::default_random_engine, not good enough
    std::mt19937 re(0);
    std::uniform_int_distribution<int64_t> rand_id(0, nv - 1);
    EdgeUid eid;
    Value e_prop(prop + 256, e_prop_size);
    t1 = fma_common::GetTime();
    for (size_t k = 0; k < nv / txn_batch; k++) {
        txn = store->CreateWriteTxn();
        for (size_t i = 0; i < txn_batch; i++) {
            for (size_t j = 0; j < ne; j++) {
                int64_t src = rand_id(re);
                int64_t dst = rand_id(re);
                eid = graph.AddEdge(*txn, src, 0, dst, e_prop);

                UT_EXPECT_EQ(eid.src, src);
                UT_EXPECT_EQ(eid.dst, dst);
                UT_EXPECT_EQ(eid.lid, 0);
                UT_EXPECT_GE(eid.eid, 0);
            }
        }
        txn->Commit();
    }
    t_add_e = fma_common::GetTime() - t1;
    UT_LOG() << "add vertex & edge done";
    txn = store->CreateReadTxn();
    PeekGraph(*txn, graph);
    txn->Abort();

    // read vertex
    vid = 0;
    t1 = fma_common::GetTime();
    for (size_t k = 0; k < nv / txn_batch; k++) {
        txn = store->CreateReadTxn();
        for (size_t i = 0; i < txn_batch; i++) {
            vid = rand_id(re);
            auto it = graph.GetUnmanagedVertexIterator(txn.get(), vid);
            auto vp = it.GetProperty();
        }
        txn->Commit();
    }
    t_read_v = fma_common::GetTime() - t1;

    // read vertex & edge
    t1 = fma_common::GetTime();
    for (size_t k = 0; k < nv / txn_batch; k++) {
        txn = store->CreateReadTxn();
        for (size_t i = 0; i < txn_batch; i++) {
            vid = rand_id(re);
            auto it = graph.GetUnmanagedVertexIterator(txn.get(), vid);
            auto vp = it.GetProperty();
            auto eit = it.GetOutEdgeIterator();
            while (eit.IsValid()) {
                eit.GetDst();
                eit.GetProperty();
                eit.Next();
            }
        }
        txn->Commit();
    }
    t_read_ve = fma_common::GetTime() - t1;
    UT_LOG() << "read vertex & edge done";

    // update vertex
    t1 = fma_common::GetTime();
    for (size_t k = 0; k < nv / txn_batch; k++) {
        txn = store->CreateWriteTxn();
        for (size_t i = 0; i < txn_batch; i++) {
            vid = rand_id(re);
            auto it = graph.GetUnmanagedVertexIterator(txn.get(), vid);
            auto vp = it.GetProperty();
            Value vv = Value::MakeCopy(vp);
            (*(char*)vv.Data())++;
            graph.SetVertexProperty(*txn, vid, vv);
        }
        txn->Commit();
    }
    t_update_v = fma_common::GetTime() - t1;
    UT_LOG() << "graph update vertex done";

    // update vertex & edge
    // Note: Do not use Graph::SetProperty, otherwise the vertex/edge iterator
    // will become illegal afterwards!
    t1 = fma_common::GetTime();
    for (size_t k = 0; k < nv / txn_batch; k++) {
        txn = store->CreateWriteTxn();
        for (size_t i = 0; i < txn_batch; i++) {
            vid = rand_id(re);
            auto it = graph.GetUnmanagedVertexIterator(txn.get(), vid);
            auto vp = it.GetProperty();
            Value vv = Value::MakeCopy(vp);
            (*(char*)vv.Data())++;
            it.SetProperty(vv);
            auto eit = it.GetOutEdgeIterator();
            while (eit.IsValid()) {
                eit.GetDst();
                eit.GetEdgeId();
                auto ep = eit.GetProperty();
                DBG_ASSERT(ep.Size() == e_prop_size);
                Value vv = Value::MakeCopy(ep);
                (*(char*)vv.Data())++;
                eit.SetProperty(vv);
                eit.Next();
            }
        }
        txn->Commit();
    }
    t_update_ve = fma_common::GetTime() - t1;
    UT_LOG() << "graph update vertex & edge done";
    txn = store->CreateReadTxn();
    PeekGraph(*txn, graph);
    txn->Abort();

    // delete a quarter vertex
    t1 = fma_common::GetTime();
    for (size_t k = 0; k < nv / txn_batch / 4; k++) {
        txn = store->CreateWriteTxn();
        for (size_t i = 0; i < txn_batch; i++) {
            vid = rand_id(re);
            graph.DeleteVertex(*txn, vid);
        }
        txn->Commit();
    }
    t_delete_qtr = fma_common::GetTime() - t1;

    PrintReport();
    return 0;
}

TEST_P(TestPerfGraph, PerfGraph) {
    int test_case = 0x1;
    int argc = _ut_argc;
    char ** argv = _ut_argv;
    Configuration config;
    config.Add(test_case, "tc", true).Comment("test case mask: [xxxx]");
    config.Add(durable, "du", true).Comment("Whether the db is durable");
    config.Add(power, "p", true).Comment("power of nv (nv = 2^p)");
    config.Add(v_prop_size, "vps", true).Comment("vertex property size");
    config.Add(e_prop_size, "eps", true).Comment("edge property size");
    config.ParseAndRemove(&argc, &argv);
    config.Finalize();
    test_case = GetParam().tc;
    durable = GetParam().du;
    power = GetParam().p;
    v_prop_size = GetParam().vps;
    e_prop_size = GetParam().eps;
    power = std::min<size_t>(power, 32);
    nv = 1LL << power;
    v_prop_size = std::min<size_t>(v_prop_size, 512);
    e_prop_size = std::min<size_t>(e_prop_size, 256);
    txn_batch = std::min<size_t>(txn_batch, (size_t)1 << power);

    int ret;
    if (test_case & 0x1) {
        ret = TestPerfGraphContinuous(durable);
        if (ret != 0) return;
    }
    if (test_case & 0x2) {
        ret = TestPerfGraphNoncontinuous(durable);
        if (ret != 0) return;
    }
    if (test_case & 0x4) {
        // ret = TestPerfGraphNoncontinuousVar(track_incoming, durable, power);
        // if (ret != 0) return ret;
    }
}

INSTANTIATE_TEST_CASE_P(TestPerfGraph, TestPerfGraph,
                        testing::Values(ParamGraph(1, false, 10), ParamGraph(1, true, 10),
                                        ParamGraph(1, false, 10), ParamGraph(2, true, 10),
                                        ParamGraph(1, false, 10, 1024, 1024),
                                        ParamGraph(1, true, 10, 1024, 1024)));
