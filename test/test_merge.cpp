/**
 * Copyright 2024 AntGroup CO., Ltd.
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

#include "fma-common/configuration.h"
#include "fma-common/utils.h"
#include "gtest/gtest.h"

#include "core/lightning_graph.h"
#include "../procedures/demo/merge.h"
#include "../procedures/demo/merge_reduced.h"
#include "./ut_utils.h"
using namespace lgraph_api;
using namespace std;

class TestMerge : public TuGraphTestWithParam<int> {};

void test_merge_vertex(GraphDB &db, string &err) {
    auto txn = db.CreateWriteTxn();
    string error_msg;
    bool r;
    // match
    r = MergeVertex(
        txn,
        MatchIterator(txn, "person", vector<string>{"name"}, vector<FieldData>{FieldData("cc")}),
        vector<string>{"age", "gender"},
        vector<FieldData>{FieldData((int64_t)15), FieldData((int64_t)0)}, vector<string>{"age"},
        vector<FieldData>{FieldData((int64_t)10)}, error_msg);
    UT_EXPECT_TRUE(r);
    r = MergeVertex(txn,
                    MatchIterator(txn, "person", vector<string>{"gender"},
                                  vector<FieldData>{FieldData((int64_t)1)}),
                    vector<string>{"age", "name"},
                    vector<FieldData>{FieldData((int64_t)25), FieldData("25s")},
                    vector<string>{"age"}, vector<FieldData>{FieldData((int64_t)20)}, error_msg);
    UT_EXPECT_TRUE(r);
    // create
    r = MergeVertex(
        txn,
        MatchIterator(txn, "person", vector<string>{"name"}, vector<FieldData>{FieldData("ee")}),
        vector<string>{"age", "gender"},
        vector<FieldData>{FieldData((int64_t)35), FieldData((int64_t)0)}, vector<string>{"age"},
        vector<FieldData>{FieldData((int64_t)30)}, error_msg);
    UT_EXPECT_TRUE(r);
    r = MergeVertex(txn,
                    MatchIterator(txn, "person", vector<string>{"name", "age"},
                                  vector<FieldData>{FieldData("ff"), FieldData((int64_t)40)}),
                    vector<string>{"gender"}, vector<FieldData>{FieldData((int64_t)1)},
                    vector<string>{"gender"}, vector<FieldData>{FieldData((int64_t)1)}, error_msg);
    UT_EXPECT_TRUE(r);
    txn.Commit();
    UT_LOG() << __func__ << " done.";
}

void test_merge_edge(GraphDB &db, string &err) {
    auto txn = db.CreateWriteTxn();
    bool r;
    // create
    r = MergeEdge(
        txn,
        MatchIterator(txn, "person", vector<string>{"name"}, vector<FieldData>{FieldData("cc")}),
        MatchIterator(txn, "person", vector<string>{"name"}, vector<FieldData>{FieldData("dd")}),
        "knows", vector<string>{}, vector<FieldData>{}, vector<string>{"since", "weight"},
        vector<FieldData>{FieldData((int64_t)2021), FieldData(0.2021)}, vector<string>{"weight"},
        vector<FieldData>{FieldData(0.77)}, err);
    UT_EXPECT_TRUE(r);
    r = MergeEdge(
        txn,
        MatchIterator(txn, "person", vector<string>{"name"}, vector<FieldData>{FieldData("cc")}),
        MatchIterator(txn, "person", vector<string>{"name"}, vector<FieldData>{FieldData("dd")}),
        "knows", vector<string>{"weight"}, vector<FieldData>{FieldData(1.0)},
        vector<string>{"since", "weight"},
        vector<FieldData>{FieldData((int64_t)2022), FieldData(0.2022)}, vector<string>{"since"},
        vector<FieldData>{FieldData((int64_t)1977)}, err);
    UT_EXPECT_TRUE(r);
    // match
    r = MergeEdge(
        txn,
        MatchIterator(txn, "person", vector<string>{"name"}, vector<FieldData>{FieldData("aa")}),
        MatchIterator(txn, "person", vector<string>{"name"}, vector<FieldData>{FieldData("bb")}),
        "knows", vector<string>{}, vector<FieldData>{}, vector<string>{"since", "weight"},
        vector<FieldData>{FieldData((int64_t)2023), FieldData(0.2023)}, vector<string>{"weight"},
        vector<FieldData>{FieldData(0.77)}, err);
    UT_EXPECT_TRUE(r);
    r = MergeEdge(
        txn,
        MatchIterator(txn, "person", vector<string>{"name"}, vector<FieldData>{FieldData("cc")}),
        MatchIterator(txn, "person", vector<string>{"name"}, vector<FieldData>{FieldData("dd")}),
        "knows", vector<string>{"since"}, vector<FieldData>{FieldData((int64_t)2021)},
        vector<string>{"since", "weight"},
        vector<FieldData>{FieldData((int64_t)2024), FieldData(0.2024)}, vector<string>{"weight"},
        vector<FieldData>{FieldData(0.77)}, err);
    UT_EXPECT_TRUE(r);
    txn.Commit();
    UT_LOG() << __func__ << " done.";
}

void test_merge_vertex_r(GraphDB &db, string &err) {
    auto txn = db.CreateWriteTxn();
    std::pair<bool, int64_t> r;
    // match
    r = MergeVertex(txn, "person", vector<string>{"name"}, vector<FieldData>{FieldData("cc")},
                    vector<string>{"age", "gender"},
                    vector<FieldData>{FieldData((int64_t)15), FieldData((int64_t)0)},
                    vector<string>{"age"}, vector<FieldData>{FieldData((int64_t)10)}, err);
    UT_EXPECT_TRUE(r.first && r.second == 2);
    // create
    r = MergeVertex(txn, "person", vector<string>{"name"}, vector<FieldData>{FieldData("ee")},
                    vector<string>{"age", "gender"},
                    vector<FieldData>{FieldData((int64_t)35), FieldData((int64_t)0)},
                    vector<string>{"age"}, vector<FieldData>{FieldData((int64_t)30)}, err);
    UT_EXPECT_TRUE(r.first && r.second == 4);
    txn.Commit();
    UT_LOG() << __func__ << " done.";
}

void test_merge_edge_r(GraphDB &db, string &err) {
    auto txn = db.CreateWriteTxn();
    std::pair<bool, lgraph_api::EdgeUid> r;
    // create
    r = MergeEdge(txn, 2, 3, "knows", vector<string>{}, vector<FieldData>{},
                  vector<string>{"since", "weight"},
                  vector<FieldData>{FieldData((int64_t)2021), FieldData(0.2021)},
                  vector<string>{"weight"}, vector<FieldData>{FieldData(0.77)}, err);
    UT_EXPECT_TRUE(r.first && r.second.eid == 0);
    r = MergeEdge(txn, "person", vector<string>{"name"}, vector<FieldData>{FieldData("cc")},
                  "person", vector<string>{"name"}, vector<FieldData>{FieldData("dd")}, "knows",
                  vector<string>{"weight"}, vector<FieldData>{FieldData(1.0)},
                  vector<string>{"since", "weight"},
                  vector<FieldData>{FieldData((int64_t)2022), FieldData(0.2022)},
                  vector<string>{"since"}, vector<FieldData>{FieldData((int64_t)1977)}, err);
    UT_EXPECT_TRUE(r.first && r.second.eid == 1);
    // match
    r = MergeEdge(txn, 0, 1, "knows", vector<string>{}, vector<FieldData>{},
                  vector<string>{"since", "weight"},
                  vector<FieldData>{FieldData((int64_t)2023), FieldData(0.2023)},
                  vector<string>{"weight"}, vector<FieldData>{FieldData(0.77)}, err);
    UT_EXPECT_TRUE(r.first && r.second.eid == 0);
    r = MergeEdge(txn, "person", vector<string>{"name"}, vector<FieldData>{FieldData("cc")},
                  "person", vector<string>{"name"}, vector<FieldData>{FieldData("dd")}, "knows",
                  vector<string>{"since"}, vector<FieldData>{FieldData((int64_t)2021)},
                  vector<string>{"since", "weight"},
                  vector<FieldData>{FieldData((int64_t)2024), FieldData(0.2024)},
                  vector<string>{"weight"}, vector<FieldData>{FieldData(0.77)}, err);
    UT_EXPECT_TRUE(r.first && r.second.eid == 0);
    txn.Commit();
    UT_LOG() << __func__ << " done.";
}

TEST_P(TestMerge, Merge) {
    int test_case = 0;
    test_case = GetParam();
    int argc = _ut_argc;
    char ** argv = _ut_argv;
    fma_common::Configuration config;
    config.Add(test_case, "tc", true).Comment("Test case");
    config.ParseAndFinalize(argc, argv);
    Galaxy galaxy("./testdb",
                  lgraph::_detail::DEFAULT_ADMIN_NAME,
                  lgraph::_detail::DEFAULT_ADMIN_PASS, false, true);
    GraphDB db = galaxy.OpenGraph("default");
    db.DropAllData();
    string label_v = "person", label_e = "knows";
    db.AddVertexLabel(label_v,
                      vector<FieldSpec>{{"name", FieldType::STRING, false},
                                        {"age", FieldType::INT32, true},
                                        {"gender", FieldType::INT8, false}},
                      VertexOptions("name"));
    db.AddEdgeLabel(
        label_e,
        vector<FieldSpec>{{"since", FieldType::INT32, false}, {"weight", FieldType::DOUBLE, false}},
        {});
    UT_ASSERT(!db.AddVertexIndex(label_v, "name", lgraph::IndexType::NonuniqueIndex));
    db.AddVertexIndex(label_v, "gender", lgraph::IndexType::NonuniqueIndex);
    auto txn = db.CreateWriteTxn();
    int64_t vid[8];
    vid[0] = txn.AddVertex(label_v, vector<string>{"name", "age", "gender"},
                           vector<string>{"aa", "5", "1"});
    vid[1] = txn.AddVertex(label_v, vector<string>{"name", "age", "gender"},
                           vector<string>{"bb", "5", "0"});
    vid[2] = txn.AddVertex(label_v, vector<string>{"name", "age", "gender"},
                           vector<string>{"cc", "5", "0"});
    vid[3] = txn.AddVertex(label_v, vector<string>{"name", "age", "gender"},
                           vector<string>{"dd", "5", "1"});
    txn.AddEdge(vid[0], vid[1], label_e, vector<string>{"since", "weight"},
                vector<string>{"2019", "0.5"});
    txn.AddEdge(vid[2], vid[1], label_e, vector<string>{"since", "weight"},
                vector<string>{"2019", "0.5"});
    txn.Commit();

    std::string err;
    switch (test_case) {
    case 1:
        test_merge_vertex(db, err);
        break;
    case 2:
        test_merge_edge(db, err);
        break;
    case 3:
        test_merge_vertex_r(db, err);
        break;
    case 4:
        test_merge_edge_r(db, err);
        break;
    default:
        break;
    }
}

INSTANTIATE_TEST_CASE_P(TestMerge, TestMerge, testing::Values(0, 1, 2, 3, 4));
