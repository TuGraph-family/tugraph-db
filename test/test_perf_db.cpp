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

#include "core/lightning_graph.h"
#include "core/mock_kv.h"
#include "./ut_utils.h"
using namespace fma_common;
using namespace lgraph;

struct ParamPerfdb {
    ParamPerfdb(size_t _tc, bool _ti, bool _du, size_t _p, bool _ec) {
        tc = _tc;
        ti = _ti;
        du = _du;
        p = _p;
        ec = _ec;
    }
    size_t tc;
    bool ti;
    bool du;
    size_t p;
    bool ec;
};

class TestPerfdb : public TuGraphTestWithParam<struct ParamPerfdb> {};

static size_t power = 12;           // power of nv
static int64_t nv = 1LL << power;   // number of vertex
static size_t ne = 8;               // number of edge per vertex
static size_t v_prop_size = 32;     // vertex property size
static size_t e_prop_size = 16;     // edge property size
static size_t txn_batch = 1 << 10;  // txn batch
static bool enable_check = false;

static char prop[] = {/* 64 * 8 */
                      "0A0B0C0D0E0F0G0H0I0J0K0L0M0N0O0P0Q0R0S0T0U0V0W0X0Y0Z0a0b0c0d0e0f"
                      "1A1B1C1D1E1F1G1H1I1J1K1L1M1N1O1P1Q1R1S1T1U1V1W1X1Y1Z1a1b1c1d1e1f"
                      "2A2B2C2D2E2F2G2H2I2J2K2L2M2N2O2P2Q2R2S2T2U2V2W2X2Y2Z2a2b2c2d2e2f"
                      "3A3B3C3D3E3F3G3H3I3J3K3L3M3N3O3P3Q3R3S3T3U3V3W3X3Y3Z3a3b3c3d3e3f"
                      "4A4B4C4D4E4F4G4H4I4J4K4L4M4N4O4P4Q4R4S4T4U4V4W4X4Y4Z4a4b4c4d4e4f"
                      "5A5B5C5D5E5F5G5H5I5J5K5L5M5N5O5P5Q5R5S5T5U5V5W5X5Y5Z5a5b5c5d5e5f"
                      "6A6B6C6D6E6F6G6H6I6J6K6L6M6N6O6P6Q6R6S6T6U6V6W6X6Y6Z6a6b6c6d6e6f"
                      "7A7B7C7D7E7F7G7H7I7J7K7L7M7N7O7P7Q7R7S7T7U7V7W7X7Y7Z7a7b7c7d7e7f"};

static void PeekGraph(Transaction& txn) {}

int TestPerfDbNoncontinuous(bool durable, bool track_in) {
    UT_LOG() << "Begining test " << __func__;
    double t1, t_add_v, t_add_e, t_update_v, t_update_ve, t_read_v, t_read_ve, t_del_qtr;
    nv = (int64_t)1 << power;
    std::mt19937 re(0);
    std::uniform_int_distribution<int64_t> rand_id(0, nv - 1);

    // create lgraph, cleaning
    {
        auto store = std::make_unique<LMDBKvStore>("./testdb", (size_t)1 << 40, durable);
        auto txn = store->CreateWriteTxn();
        store->DropAll(*txn);
        txn->Commit();
    }
    DBConfig config;
    config.dir = "./testdb";
    LightningGraph db(config);
    auto txn = db.CreateReadTxn();
    // db.DumpGraph(txn);
    txn.Abort();

    // add schemas
    UT_EXPECT_TRUE(db.AddLabel("person",
                       std::vector<FieldSpec>(
                           {{"uid", FieldType::INT64, false}, {"name", FieldType::STRING, false}}),
                       true /*is vertex*/, VertexOptions("uid")));
    UT_EXPECT_TRUE(db.AddLabel("knows",
                       std::vector<FieldSpec>(
                           {{"uid", FieldType::INT64, false}, {"name", FieldType::STRING, false}}),
                       false /*is edge*/, EdgeOptions()));

    // add vertex
    VertexId vid = 0;
    int64_t uid = 0;
    t1 = fma_common::GetTime();
    for (size_t k = 0; k < nv / txn_batch; k++) {
        txn = db.CreateWriteTxn();
        for (size_t i = 0; i < txn_batch; i++) {
            uid++;
            vid = txn.AddVertex(
                std::string("person"), std::vector<std::string>{"uid", "name"},
                std::vector<std::string>{ToString(uid), std::string(prop, v_prop_size - 8)});
            UT_EXPECT_EQ(int64_t(vid), txn_batch * k + i);
        }
        txn.Commit();
    }
    t_add_v = fma_common::GetTime() - t1;

    // add edge
    uid = 0;
    t1 = fma_common::GetTime();
    for (size_t k = 0; k < nv / txn_batch; k++) {
        txn = db.CreateWriteTxn();
        for (size_t i = 0; i < txn_batch; i++) {
            for (size_t j = 0; j < ne; j++) {
                int64_t src = rand_id(re);
                int64_t dst = rand_id(re);
                uid++;
                auto eid = txn.AddEdge(
                    src, dst, std::string("knows"), std::vector<std::string>{"uid", "name"},
                    std::vector<std::string>{ToString(uid),
                                             std::string(prop + 256, e_prop_size - 8)});
                DBG_ASSERT(eid.eid >= 0);
            }
        }
        txn.Commit();
    }
    t_add_e = fma_common::GetTime() - t1;
    UT_LOG() << "add vertex & edge done";

    // read vertex
    std::vector<size_t> field_ids{0, 1};
    std::vector<FieldData> fields;
    FieldData field;
    t1 = fma_common::GetTime();
    for (size_t k = 0; k < nv / txn_batch; k++) {
        txn = db.CreateReadTxn();
        for (size_t i = 0; i < txn_batch; i++) {
            vid = rand_id(re);
            field = txn.GetVertexField(vid, field_ids[0]);
            if (enable_check) UT_EXPECT_EQ(field.AsInt64(), vid + 1);
        }
        txn.Abort();
    }
    t_read_v = fma_common::GetTime() - t1;

    // read vertex & edge
    t1 = fma_common::GetTime();
    for (size_t k = 0; k < nv / txn_batch; k++) {
        txn = db.CreateReadTxn();
        for (size_t i = 0; i < txn_batch; i++) {
            vid = rand_id(re);
            field = txn.GetVertexField(vid, field_ids[0]);
            if (enable_check) UT_EXPECT_EQ(field.AsInt64(), vid + 1);
            auto it = txn.GetVertexIterator(vid);
            auto eit = it.GetOutEdgeIterator();
            while (eit.IsValid()) {
                fields = txn.GetEdgeFields(eit, field_ids);
                eit.Next();
            }
        }
        txn.Abort();
    }
    t_read_ve = fma_common::GetTime() - t1;
    UT_LOG() << "read vertex & edge done";
    txn = db.CreateReadTxn();
    PeekGraph(txn);
    txn.Abort();

    // update vertex
    t1 = fma_common::GetTime();
    for (size_t k = 0; k < nv / txn_batch; k++) {
        txn = db.CreateWriteTxn();
        for (size_t i = 0; i < txn_batch; i++) {
            vid = rand_id(re);
            fields = txn.GetVertexFields(vid, field_ids);
            fields[1].AsString()[0]++;
            std::string expected = fields[1].AsString();
            txn.SetVertexProperty(vid, field_ids, std::vector<FieldData>{fields[0], fields[1]});
            if (enable_check) {
                UT_EXPECT_EQ(fields[0].AsInt64(), vid + 1);
                auto f = txn.GetVertexField(vid, (size_t)1);
                UT_EXPECT_TRUE(!f.is_null());
                UT_EXPECT_EQ(f.AsString(), expected);
            }
        }
        txn.Commit();
    }
    t_update_v = fma_common::GetTime() - t1;

    // update vertex & edge
    t1 = fma_common::GetTime();
    for (size_t k = 0; k < nv / txn_batch; k++) {
        txn = db.CreateWriteTxn();
        for (size_t i = 0; i < txn_batch; i++) {
            vid = rand_id(re);
            fields = txn.GetVertexFields(vid, field_ids);
            fields[1].AsString()[0]++;
            txn.SetVertexProperty(vid, field_ids, std::vector<FieldData>{fields[0], fields[1]});
            auto it = txn.GetVertexIterator(vid);
            auto eit = it.GetOutEdgeIterator();
            while (eit.IsValid()) {
                fields = txn.GetEdgeFields(eit, field_ids);
                fields[1].AsString()[0]++;
                txn.SetEdgeProperty(eit, field_ids, std::vector<FieldData>{fields[0], fields[1]});
                eit.Next();
            }
        }
        txn.Commit();
    }
    t_update_ve = fma_common::GetTime() - t1;
    UT_LOG() << "update vertex & edge done";
    txn = db.CreateReadTxn();
    PeekGraph(txn);
    txn.Abort();

    // delete a quarter vertex
    t1 = fma_common::GetTime();
    for (size_t k = 0; k < nv / txn_batch / 4; k++) {
        txn = db.CreateWriteTxn();
        for (size_t i = 0; i < txn_batch; i++) {
            vid = rand_id(re);
            txn.DeleteVertex(vid);
        }
        txn.Commit();
    }
    t_del_qtr = fma_common::GetTime() - t1;

    std::string report;
    report.append(StringFormatter::Format(
        "(continuous, track_in, durable, enable_check, nv, ne, txn_batch, "
        "fixed_size, v_prop_size, e_prop_size) = "
        "({}, {}, {}, {}, {}, {}, {}, {}, {}, {})\n",
        0, track_in, durable, enable_check, nv, ne * nv, txn_batch, 1, v_prop_size, e_prop_size));
    report.append(StringFormatter::Format(
        "(t_add_v, t_add_e, t_read_v, t_read_ve, "
        "t_update_v, t_update_ve, t_del_qtr) = \n"
        "({}, {}, {}, {}, {}, {}, {})s\n",
        t_add_v, t_add_e, t_read_v, t_read_ve, t_update_v, t_update_ve, t_del_qtr));
    UT_LOG() << report;
    return 0;
}

int TestPerfDbNonbatch(bool durable, bool track_in) {
    UT_LOG() << "Begining test " << __func__;
    double t1, t_add_v, t_add_e;
    nv = (int64_t)1 << power;
    std::mt19937 re(0);
    std::uniform_int_distribution<int64_t> rand_id(0, nv - 1);

    // create lgraph, cleaning
    DBConfig config;
    config.dir = "./testdb";
    LightningGraph db(config);
    db.DropAllData();
    // add schemas
    UT_EXPECT_TRUE(db.AddLabel("person",
                               std::vector<FieldSpec>({{"uid", FieldType::INT64, false},
                                                       {"name", FieldType::STRING, false}}),
                               true /*is vertex*/, VertexOptions("uid")));
    UT_EXPECT_TRUE(db.AddLabel("knows",
                               std::vector<FieldSpec>({{"uid", FieldType::INT64, false},
                                                       {"name", FieldType::STRING, false}}),
                               false /*is vertex*/, EdgeOptions()));

    // add vertex
    VertexId vid = 0;
    int64_t uid = 0;
    t1 = fma_common::GetTime();
    for (int64_t i = 0; i < nv; i++) {
        auto txn = db.CreateWriteTxn();
        uid++;
        vid = txn.AddVertex(
            std::string("person"), std::vector<std::string>{"uid", "name"},
            std::vector<std::string>{ToString(uid), std::string(prop, v_prop_size - 8)});
        UT_EXPECT_EQ(int64_t(vid), i);
        txn.Commit();
    }
    t_add_v = fma_common::GetTime() - t1;

    // add edge
    uid = 0;
    t1 = fma_common::GetTime();
    for (int64_t i = 0; i < nv; i++) {
        auto txn = db.CreateWriteTxn();
        for (size_t j = 0; j < ne; j++) {
            int64_t src = rand_id(re);
            int64_t dst = rand_id(re);
            uid++;
            auto eid = txn.AddEdge(
                src, dst, std::string("knows"), std::vector<std::string>{"uid", "name"},
                std::vector<std::string>{ToString(uid), std::string(prop + 256, e_prop_size - 8)});
            DBG_ASSERT(eid.eid >= 0);
        }
        txn.Commit();
    }
    t_add_e = fma_common::GetTime() - t1;
    UT_LOG() << "add vertex & edge done";

    std::string report;
    report.append(StringFormatter::Format(
        "non-batch (continuous, track_in, durable, "
        "enable_check, nv, ne, txn_batch, "
        "fixed_size, v_prop_size, e_prop_size) = "
        "({}, {}, {}, {}, {}, {}, {}, {}, {}, {})\n",
        0, track_in, durable, enable_check, nv, ne * nv, txn_batch, 1, v_prop_size, e_prop_size));
    report.append(
        StringFormatter::Format("(t_add_v, t_add_e, t_read_v, t_read_ve, "
                                "t_update_v, t_update_ve, t_del_qtr) = \n"
                                "({}, {}, {}, {}, {}, {}, {})s\n",
                                t_add_v, t_add_e, -1, -1, -1, -1, -1));
    UT_LOG() << report;
    return 0;
}

TEST_P(TestPerfdb, PerfDb) {
    int test_case = 0x2;
    bool durable = false;
    bool track_in = true;
    int argc = _ut_argc;
    char ** argv = _ut_argv;
    Configuration config;
    config.Add(test_case, "tc", true).Comment("test case mask: [xxxx]");
    config.Add(durable, "du", true).Comment("Whether the db is durable");
    config.Add(track_in, "ti", true).Comment("track incoming edge");
    config.Add(power, "p", true).Comment("power of nv (nv = 2^p)");
    config.Add(v_prop_size, "vps", true).Comment("vertex property size");
    config.Add(e_prop_size, "eps", true).Comment("edge property size");
    config.Add(enable_check, "ec", true).Comment("enable correctness check");
    config.ParseAndRemove(&argc, &argv);
    config.Finalize();
    test_case = GetParam().tc;
    durable = GetParam().du;
    track_in = GetParam().ti;
    power = GetParam().p;
    enable_check = GetParam().ec;
    power = std::min<size_t>(power, 32);
    nv = 1LL << power;
    v_prop_size = std::min<size_t>(v_prop_size, 512);
    e_prop_size = std::min<size_t>(e_prop_size, 256);
    txn_batch = std::min<size_t>(txn_batch, (size_t)1 << power);

    if (test_case & 0x2) {
        TestPerfDbNoncontinuous(durable, track_in);
    }
    if (test_case & 0x8) {
        TestPerfDbNonbatch(durable, track_in);
    }
}

INSTANTIATE_TEST_CASE_P(TestPerfdb, TestPerfdb,
                        testing::Values(ParamPerfdb(2, true, true, 10, true),
                                        ParamPerfdb(2, true, false, 10, true)));
