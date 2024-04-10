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

#include <map>
#include <string>

#include "core/data_type.h"
#include "fma-common/configuration.h"
#include "fma-common/utils.h"

#include "gtest/gtest.h"

#include "lgraph/lgraph.h"
#include "restful/server/json_convert.h"

#include "./test_tools.h"
#include "./ut_utils.h"
using namespace lgraph_api;

class TestLGraphApi : public TuGraphTest {
 public:
    bool HasVertexIndex(GraphDB& db, const std::string& label,
                        const std::string& field, lgraph::IndexType type) {
        auto txn = db.CreateReadTxn();
        auto indexes = txn.ListVertexIndexes();
        for (auto& i : indexes) {
            if (i.label == label && i.field == field &&
                i.type == type) {
                return true;
            }
        }
        return false;
    }
    bool HasEdgeIndex(GraphDB& db, const std::string& label,
                      const std::string& field, lgraph::IndexType type) {
        auto txn = db.CreateReadTxn();
        auto indexes = txn.ListEdgeIndexes();
        for (auto& i : indexes) {
            if (i.label == label && i.field == field &&
                i.type == type) {
                return true;
            }
        }
        return false;
    }
    size_t GetVertexIndexValueNum(GraphDB& db, const std::string& label,
                                  const std::string& field) {
        auto txn = db.CreateReadTxn();
        size_t count = 0;
        for (auto it = txn.GetVertexIndexIterator(label, field, "", ""); it.IsValid(); it.Next()) {
            ++count;
        }
        return count;
    }
    size_t GetEdgeIndexValueNum(GraphDB& db, const std::string& label,
                                const std::string& field) {
        auto txn = db.CreateReadTxn();
        size_t count = 0;
        for (auto it = txn.GetEdgeIndexIterator(label, field, "", ""); it.IsValid(); it.Next()) {
            ++count;
        }
        return count;
    }
    bool HasVertexIndexKey(GraphDB& db, const std::string& label,
                           const std::string& field, const lgraph::FieldData& key) {
        auto txn = db.CreateReadTxn();
        for (auto it =
                 txn.GetVertexIndexIterator(label, field, key, key); it.IsValid(); it.Next()) {
            if (it.GetIndexValue() == key) return true;
        }
        return false;
    }
    bool HasEdgeIndexKey(GraphDB& db, const std::string& label,
                         const std::string& field, const lgraph::FieldData& key) {
        auto txn = db.CreateReadTxn();
        for (auto it = txn.GetEdgeIndexIterator(label, field, key, key); it.IsValid(); it.Next()) {
            if (it.GetIndexValue() == key) return true;
        }
        return false;
    }
};

TEST_F(TestLGraphApi, ConcurrentVertexAdd) {
    size_t n_threads = 2;
    fma_common::Configuration config;
    config.Add(n_threads, "nt", true).Comment("Number of concurrent threads").SetMin(1);
    config.ParseAndFinalize(_ut_argc, _ut_argv);

    const std::string& dir = "./testdb";
    lgraph::AutoCleanDir cleaner(dir);
    Galaxy galaxy(dir, false, true);
    galaxy.SetCurrentUser(lgraph::_detail::DEFAULT_ADMIN_NAME, lgraph::_detail::DEFAULT_ADMIN_PASS);
    auto graph = galaxy.OpenGraph(lgraph::_detail::DEFAULT_GRAPH_DB_NAME);
    graph.AddVertexLabel(std::string("v"),
                         std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                 {"content", FieldType::STRING, true}}),
                         VertexOptions("id"));

    Barrier bar(n_threads);
    std::atomic<size_t> n_success(0);
    std::atomic<size_t> n_fail(0);
    std::vector<std::thread> threads;
    for (size_t i = 0; i < n_threads; i++) {
        threads.emplace_back([&, i]() {
            auto txn = graph.CreateWriteTxn(true);
            // If two optimistic txns add vertices at the same time, one with small property,
            // the other with large property, then we will create two KVs, one with PackedData
            // type, the other with VertexOnly type, and the conflict resolution will fail to
            // detect this. To solve the problem, we use a kv in meta table to maintain the
            // next vid, so no concurrent vertex add is allowed.
            size_t csize = i % 2 == 0 ? 8 : 2048;
            txn.AddVertex(std::string("v"), std::vector<std::string>{"id", "content"},
                          std::vector<std::string>{std::to_string(i), std::string(csize, 'a')});
            bar.Wait();
            try {
                txn.Commit();
                n_success++;
            } catch (lgraph_api::LgraphException& e) {
                if (e.code() == lgraph_api::ErrorCode::TxnConflict) {
                    n_fail++;
                } else {
                    throw;
                }
            }
        });
    }
    for (auto& t : threads) t.join();
    UT_EXPECT_EQ(n_success.load(), 1);
    UT_EXPECT_EQ(n_fail.load(), n_threads - 1);
    {
        auto txn = graph.CreateReadTxn();
        UT_EXPECT_EQ(txn.GetNumVertices(), 1);
    }
}

TEST_F(TestLGraphApi, LGraphApi) {
    std::string path = "./testdb";
    auto ADMIN = lgraph::_detail::DEFAULT_ADMIN_NAME;
    auto ADMIN_PASS = lgraph::_detail::DEFAULT_ADMIN_PASS;
    int argc = _ut_argc;
    char** argv = _ut_argv;
    fma_common::Configuration config;
    config.Add(path, "dir", true).Comment("DB directory");
    config.ParseAndFinalize(argc, argv);
    try {
        lgraph::AutoCleanDir cleaner(path);
        std::unique_ptr<Galaxy> g(new Galaxy(path));
        for (size_t i = 0; i < 10; i++) {
            g.reset();
            g.reset(new Galaxy(path));
            g->SetCurrentUser(ADMIN, ADMIN_PASS);
            GraphDB db = g->OpenGraph("default");
        }
    } catch (std::exception& e) {
        UT_EXPECT_TRUE(false);
        UT_ERR() << "Unexpected error: " << e.what();
    }

    UT_LOG() << "Testing user names, passwords and graph names";
    {
        lgraph::AutoCleanDir cleaner(path);
        Galaxy g(path);
        g.SetCurrentUser(ADMIN, ADMIN_PASS);

        auto TestWithDifferentStrings =
            [&](const std::vector<std::pair<std::string, bool>>& str_success,
                const std::function<bool(const std::string&)>& do_op,
                const std::function<bool(const std::string&)>& undo_op) {
                for (auto& p : str_success) {
                    const std::string& name = p.first;
                    if (p.second) {
                        // success, do and then undo
                        UT_EXPECT_TRUE(do_op(name));
                        UT_EXPECT_TRUE(undo_op(name));
                    } else {
                        // expect fail
                        UT_EXPECT_ANY_THROW(do_op(name));
                    }
                }
            };

        std::vector<std::pair<std::string, bool>> names_and_success = {
            {std::string(64, 'a'), true},  // ok
#ifndef _MSC_VER
            {std::string("中文"), true},  // invalid string // NOLINT
#endif
            {std::string("_"), true},           // ok
            {std::string("abc_1234_"), true},   // ok
            {std::string(""), false},           // empty name, fail
            {std::string(65, 'a'), false},      // too long
            {std::string("01823_bbb"), false},  // cannot start with digits
            {std::string("`"), false},          // invalid character
            {std::string("#"), false},          // invalid character
            {std::string("$"), false},          // invalid character
            {std::string("^"), false},          // invalid character
            {std::string("-"), false},          // invalid character
        };

        UT_LOG() << "Testing graph name";
        TestWithDifferentStrings(
            names_and_success, [&](const std::string& name) -> bool { return g.CreateGraph(name); },
            [&](const std::string& name) -> bool { return g.DeleteGraph(name); });

        UT_LOG() << "Testing user name";
        TestWithDifferentStrings(
            names_and_success,
            [&](const std::string& name) -> bool { return g.CreateUser(name, "pass"); },
            [&](const std::string& name) -> bool { return g.DeleteUser(name); });

        UT_LOG() << "Testing password";
        std::vector<std::pair<std::string, bool>> passwords_and_success = {
            {std::string(64, 'a'), true},      // ok
            {std::string(6, '\x03'), true},    // ok
            {std::string("任意密码"), true},   // ok  // NOLINT
            {std::string("_"), true},          // ok
            {std::string("abc_1234_"), true},  // ok
            {std::string(""), false},          // empty password, fail
            {std::string(65, 'a'), false},     // too long
            {std::string("01823_bbb"), true},  // ok
            {std::string("`"), true},          // ok
            {std::string("#"), true},          // ok
            {std::string("$"), true},          // ok
            {std::string("^"), true},          // ok
            {std::string("-"), true},          // ok
        };
        TestWithDifferentStrings(
            passwords_and_success,
            [&](const std::string& pass) -> bool { return g.CreateUser("valid_user", pass); },
            [&](const std::string& name) -> bool { return g.DeleteUser("valid_user"); });
    }
    // #if 0
    {
#ifdef _WIN32
        fma_common::SleepS(1);
#endif
        lgraph::AutoCleanDir cleaner(path);
        Galaxy galaxy(path);
        galaxy.SetCurrentUser(ADMIN, ADMIN_PASS);
        GraphDB db = galaxy.OpenGraph("default");
        UT_EXPECT_EQ(db.GetDescription(), "");
        UT_EXPECT_EQ(db.GetMaxSize(), lgraph::_detail::DEFAULT_GRAPH_SIZE);
        // add vertex label
        std::string vertex_label("vertex");
        std::vector<std::string> vertex_feild_name = {"id", "type", "content"};
        std::vector<std::string> vertex_values = {"id_1", "8", "content"};

        db.AddVertexLabel(vertex_label,
                          std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                  {"type", FieldType::INT8, false},
                                                  {"content", FieldType::STRING, true}}),
                          VertexOptions("id"));
        UT_EXPECT_TRUE(db.IsVertexIndexed(vertex_label, "id"));
        // UT_EXPECT_TRUE(db.DeleteVertexIndex(vertex_label, "id"));
        Transaction txn_write = db.CreateWriteTxn();

        for (int i = 0; i < 4; i++) {
            std::vector<std::string> v = {std::to_string(i), "8", "content"};
            txn_write.AddVertex(vertex_label, vertex_feild_name, v);
        }
        txn_write.Commit();
        // txn_write.AddVertex();
        Transaction txn_read = db.CreateReadTxn();
        std::vector<int64_t> vids = {33164890, 33166571, 33442318, 34094368};
        for (auto vid : vids) {
            auto it = txn_read.GetVertexIterator(vid, false);
            UT_LOG() << vid << " : " << it.IsValid();
        }
        txn_read.IsValid();
    }

    try {
        UT_LOG() << "Testing automatic iterator refresh";
        lgraph::AutoCleanDir cleaner(path);
        Galaxy galaxy(path);
        galaxy.SetCurrentUser(ADMIN, ADMIN_PASS);
        GraphDB db = galaxy.OpenGraph("default");
        db = galaxy.OpenGraph("default");
        UT_EXPECT_FALSE(ShouldKillThisTask());
        db.DropAllData();
        // add vertex label
        std::string vlabel = "v";
        db.AddVertexLabel(vlabel,
                          std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                  {"type", FieldType::INT8, false},
                                                  {"content", FieldType::STRING, true}}),
                          VertexOptions("id"));
        std::string elabel = "e";
        db.AddEdgeLabel(elabel, std::vector<FieldSpec>({{"weight", FieldType::FLOAT, false}}), {});
        UT_EXPECT_TRUE(db.IsVertexIndexed(vlabel, "id"));
        Transaction txn_write = db.CreateWriteTxn();
        for (int i = 0; i < 4; i++) {
            txn_write.AddVertex(vlabel, {"id", "type"},
                                {FieldData(std::to_string(i)), FieldData((int64_t)0)});
        }
        txn_write.Commit();

        UT_LOG() << "\tTesting EdgeIndex";
        {
            db.AddEdgeLabel("esw", std::vector<FieldSpec>({{"weight", FieldType::INT64, false}}),
                            {});
            db.AddVertexLabel("esk", std::vector<FieldSpec>({{"fs", FieldType::INT64, false}}),
                              VertexOptions("fs"));
            db.AddEdgeLabel("esp", std::vector<FieldSpec>({{"weight", FieldType::INT64, false}}),
                            {});
            db.AddEdgeLabel("unique", std::vector<FieldSpec>({{"weight", FieldType::INT64, false}}),
                            {});
            db.AddEdgeIndex("esw", "weight", lgraph::IndexType::NonuniqueIndex);
            db.AddVertexIndex("esk", "fs", lgraph::IndexType::NonuniqueIndex);
            db.AddEdgeIndex("esp", "weight", lgraph::IndexType::NonuniqueIndex);
            db.AddEdgeIndex("unique", "weight", lgraph::IndexType::GlobalUniqueIndex);
            UT_EXPECT_TRUE(db.IsEdgeIndexed("esw", "weight"));
            UT_EXPECT_TRUE(db.IsEdgeIndexed("esp", "weight"));
            UT_EXPECT_TRUE(db.IsEdgeIndexed("unique", "weight"));
            auto txn1 = db.CreateWriteTxn();
            UT_EXPECT_TRUE(txn1.IsEdgeIndexed("esw", "weight"));
            UT_EXPECT_TRUE(txn1.IsEdgeIndexed("unique", "weight"));
            std::vector<size_t> edge_field_ids =
                txn1.GetEdgeFieldIds(1, std::vector<std::string>{"weight"});
            UT_EXPECT_EQ(edge_field_ids[0], 0);
            for (int i = 0; i < 4; i++) {
                EdgeUid e = txn1.AddEdge(i, (i + 1) % 4, "esw",
                                         {"weight"}, {FieldData(int64_t(i))});
                EdgeUid expect(i, (i + 1) % 4, 1, 0, 0);
                UT_EXPECT_TRUE(e == expect);
            }
            for (int i = 0; i < 4; i++) {
                UT_EXPECT_TRUE(
                    txn1.AddEdge(i, (i + 1) % 4, "esp", {"weight"}, {FieldData(int64_t(i))}) ==
                    EdgeUid(i, (i + 1) % 4, 2, 0, 0));
            }
            for (int i = 0; i < 4; i++) {
                auto s = txn1.AddEdge(i, (i + 1) % 4, "unique",
                                      {"weight"}, {FieldData(int64_t(i))});
                UT_EXPECT_TRUE(s == EdgeUid(i, (i + 1) % 4, 3, 0, 0));
            }
            auto uet = txn1.GetEdgeByUniqueIndex("unique", "weight", "1");
            UT_EXPECT_TRUE(uet.IsValid());
            UT_EXPECT_EQ(uet.GetSrc(), 1);
            UT_EXPECT_EQ(uet.GetDst(), 2);
            UT_EXPECT_EQ(uet.GetLabelId(), 3);
            UT_EXPECT_EQ(uet.GetTemporalId(), 0);
            UT_EXPECT_EQ(uet.GetEdgeId(), 0);
            UT_EXPECT_TRUE(txn1.GetEdgeByUniqueIndex("unique", "weight", FieldData(1)).IsValid());
            UT_EXPECT_TRUE(txn1.GetEdgeByUniqueIndex(3, 0, FieldData(1)).IsValid());
            auto eit1 = txn1.GetEdgeIndexIterator("esw", "weight", "1", "2");
            auto eit2 = txn1.GetEdgeIndexIterator("esw", "weight", "2", "3");
            auto test_edge_iter1 =
                txn1.GetEdgeIndexIterator("esw", "weight", FieldData(1), FieldData(2));
            UT_EXPECT_TRUE(test_edge_iter1.IsValid());
            UT_EXPECT_TRUE(txn1.GetOutEdgeIterator(0, 1, 1).IsValid());
            UT_EXPECT_FALSE(txn1.GetOutEdgeIterator(0, 1, 0).IsValid());
            UT_EXPECT_TRUE(txn1.GetInEdgeIterator(0, 1, 1).IsValid());
            UT_EXPECT_FALSE(txn1.GetInEdgeIterator(1, 0, 0).IsValid());
            auto out_eit = txn1.GetOutEdgeIterator(0, 1, 1);
            out_eit.SetFields(std::vector<std::string>{"weight"}, std::vector<std::string>{"0"});
            UT_EXPECT_EQ(out_eit.GetFields(std::vector<std::string>{"weight"})[0].AsInt64(), 0);
            auto in_eit = txn1.GetInEdgeIterator(0, 1, 1);
            UT_EXPECT_EQ(in_eit.GetFields(std::vector<std::string>{"weight"})[0].AsInt64(), 0);
            UT_EXPECT_EQ(in_eit.GetFields(std::vector<std::size_t>{0})[0].AsInt64(), 0);
            EdgeIndexIterator test_eit1 = txn1.GetEdgeIndexIterator("esw", "weight", "1", "2");
            UT_EXPECT_TRUE(test_eit1.IsValid());
            auto test_eit2 = txn1.GetEdgeIndexIterator("esw", "weight", "2", "3");
            UT_EXPECT_TRUE(test_eit2.IsValid());
            test_eit2.Next();
            UT_EXPECT_TRUE(test_eit2.IsValid());
            test_eit2.Close();
            UT_EXPECT_FALSE(test_eit2.IsValid());
            UT_EXPECT_EQ(test_eit1.GetSrc(), 1);
            UT_EXPECT_EQ(test_eit1.GetDst(), 2);
            UT_EXPECT_EQ(test_eit1.GetEdgeId(), 0);
            UT_EXPECT_EQ(test_eit1.GetLabelId(), 1);
            UT_EXPECT_EQ(test_eit1.GetUid().src, 1);

            UT_EXPECT_TRUE(eit1.IsValid());
            UT_EXPECT_TRUE(eit2.IsValid());
            UT_EXPECT_EQ(eit1.GetIndexValue().ToString(), "1");
            UT_EXPECT_EQ(eit2.GetIndexValue().ToString(), "2");

            auto eit3 = txn1.GetEdgeIndexIterator("esp", "weight", "1", "2");
            auto eit4 = txn1.GetEdgeIndexIterator("esp", "weight", "2", "3");
            UT_EXPECT_TRUE(eit3.IsValid());
            UT_EXPECT_TRUE(eit4.IsValid());
            UT_EXPECT_EQ(eit3.GetIndexValue().ToString(), "1");
            UT_EXPECT_EQ(eit4.GetIndexValue().ToString(), "2");
            txn1.Commit();
            db.DeleteEdgeIndex("esw", "weight");
            db.DeleteEdgeIndex("esp", "weight");
            UT_EXPECT_TRUE(!db.IsEdgeIndexed("esw", "weight"));
            UT_EXPECT_TRUE(db.DeleteEdgeLabel("esw"));
            UT_EXPECT_TRUE(db.DeleteEdgeLabel("esp"));
            UT_EXPECT_TRUE(db.DeleteVertexLabel("esk"));
        }
        UT_LOG() << "\tTesting FullTextIndex";
        {
            UT_EXPECT_ANY_THROW(db.AddVertexFullTextIndex("esw", "esp"));
            UT_EXPECT_ANY_THROW(db.AddEdgeFullTextIndex("esw", "esp"));
            UT_EXPECT_ANY_THROW(db.DeleteVertexFullTextIndex("esw", "esp"));
            UT_EXPECT_ANY_THROW(db.DeleteEdgeFullTextIndex("esw", "esp"));
            std::set<std::string> vertex_labels;
            vertex_labels.insert("esw");
            std::set<std::string> edge_labels;
            edge_labels.insert("esp");
            db.RebuildFullTextIndex(vertex_labels, edge_labels);
            db.ListFullTextIndexes();
            UT_EXPECT_ANY_THROW(db.QueryVertexByFullTextIndex("esw", "name", 1));
            UT_EXPECT_ANY_THROW(db.QueryEdgeByFullTextIndex("esp", "id", 1));
        }
        UT_LOG() << "\tTesting delete";
        {
            auto txn = db.CreateWriteTxn();
            auto iit = txn.GetVertexIndexIterator(vlabel, "id", "1", "2");
            UT_EXPECT_TRUE(iit.IsValid());
            auto vit = txn.GetVertexIterator(iit.GetVid(), false);
            UT_EXPECT_TRUE(vit.IsValid());
            vit.Delete();
            UT_EXPECT_TRUE(vit.IsValid());
            UT_EXPECT_EQ(vit.GetId(), 2);
            UT_EXPECT_EQ(vit.GetField("id").ToString(), "2");
            UT_EXPECT_TRUE(iit.IsValid());
            UT_EXPECT_EQ(iit.GetIndexValue().ToString(), "2");
            UT_EXPECT_EQ(iit.GetVid(), 2);
            vit.Delete();
            // now iit goes out of bound
            UT_EXPECT_TRUE(!iit.IsValid());
            UT_EXPECT_TRUE(vit.IsValid());
            txn.Abort();
            // vit gets closed when txn is aborted
            UT_EXPECT_TRUE(!vit.IsValid());
        }

        UT_LOG() << "\tTesting add";
        {
            auto txn = db.CreateWriteTxn();
            auto vit = txn.GetVertexIterator(3);
            auto iit = txn.GetVertexIndexIterator(vlabel, "id", "3", "3");
            UT_EXPECT_TRUE(iit.IsValid());
            UT_EXPECT_EQ(iit.GetIndexValue().ToString(), "3");
            UT_EXPECT_EQ(iit.GetVid(), 3);
            for (int i = 0; i < 4; i++) {
                UT_EXPECT_TRUE(
                    txn.AddEdge(i, (i + 1) % 4, "e", {"weight"}, {FieldData(float(i))}) ==
                    EdgeUid(i, (i + 1) % 4, 0, 0, 0));
            }
            auto oeit = vit.GetOutEdgeIterator();
            auto ieit = vit.GetInEdgeIterator();
            UT_EXPECT_EQ(oeit.GetDst(), 0);
            UT_EXPECT_EQ(ieit.GetSrc(), 2);
            UT_EXPECT_TRUE(vit.IsValid());
            UT_EXPECT_EQ(vit.GetId(), 3);
            UT_EXPECT_EQ(vit.GetField("id").ToString(), "3");
            // add more edges to cause a page split
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 1000; j++) {
                    txn.AddEdge(i, (i + 1) % 4, "e", {"weight"}, {FieldData(float(j))});
                }
            }
            UT_EXPECT_TRUE(vit.IsValid());
            UT_EXPECT_EQ(vit.GetId(), 3);
            UT_EXPECT_EQ(vit.GetField("id").ToString(), "3");
            UT_EXPECT_TRUE(iit.IsValid());
            UT_EXPECT_EQ(iit.GetIndexValue().ToString(), "3");
            UT_EXPECT_EQ(iit.GetVid(), 3);
            UT_EXPECT_TRUE(!iit.Next());
            UT_EXPECT_TRUE(oeit.IsValid());
            UT_EXPECT_EQ(oeit.GetDst(), 0);
            oeit.Next();
            oeit.Next();
            UT_EXPECT_EQ(oeit.GetField("weight").AsFloat(), 1);
            UT_EXPECT_TRUE(ieit.IsValid());
            UT_EXPECT_EQ(ieit.GetSrc(), 2);
            ieit.Next();
            ieit.Next();
            UT_EXPECT_EQ(oeit.GetField("weight").AsFloat(), 1);
        }
    } catch (std::exception& e) {
        UT_LOG() << "An error occurred: " << e.what();
        UT_EXPECT_TRUE(false);
    }
    {
        lgraph::AutoCleanDir cleaner(path);
        Galaxy galaxy(path);
        galaxy.SetCurrentUser(ADMIN, ADMIN_PASS);
        GraphDB db = galaxy.OpenGraph("default");
        db.DropAllData();
        UT_LOG() << "Testing date, datetime and bin types";
        db.AddVertexLabel("v2",
                          std::vector<FieldSpec>({{"id", FieldType::INT32, false},
                                                  {"date", FieldType::DATE, false},
                                                  {"datetime", FieldType::DATETIME, true},
                                                  {"img", FieldType::BLOB, true},
                                                  {"valid", FieldType::BOOL, true}}),
                          VertexOptions("id"));
        UT_EXPECT_THROW_MSG(
            db.AddVertexLabel("v3", std::vector<FieldSpec>({{"f", FieldType::NUL, false}}),
                              VertexOptions("f")), "NUL type");
        auto txn = db.CreateWriteTxn();
        size_t label_id = txn.GetVertexLabelId("v2");
        std::vector<size_t> field_ids = txn.GetVertexFieldIds(
            label_id, std::vector<std::string>{"id", "date", "datetime", "img", "valid"});
        txn.AddVertex(label_id, field_ids,
                      std::vector<FieldData>({FieldData::Int32(0), FieldData::Date("2019-01-02"),
                                              FieldData::DateTime("2019-01-02 00:00:01"),
                                              FieldData::Blob(std::string(30, -127)),
                                              FieldData::Bool(true)}));
        std::vector<unsigned char> binary_blob = std::vector<unsigned char>(22, 244);
        txn.AddVertex(
            std::string("v2"), std::vector<std::string>({"id", "date", "datetime", "img", "valid"}),
            std::vector<std::string>({"100", "2019-02-02", "2019-02-02 00:02:02",
                                      _TS(utility::conversions::to_base64(binary_blob)), "false"}));
        auto vit = txn.GetVertexIterator(0);
        auto m = vit.GetAllFields();
        std::map<std::string, FieldData> v0 = {
            {"id", FieldData::Int32(0)},
            {"date", FieldData::Date("2019-01-02")},
            {"datetime", FieldData::DateTime("2019-01-02 00:00:01")},
            {"img", FieldData::Blob(std::string(30, -127))},
            {"valid", FieldData::Bool(true)}};
        UT_EXPECT_EQ(m, v0);
        std::map<std::string, FieldData> v1 = {
            {"id", FieldData::Int32(100)},
            {"date", FieldData::Date("2019-02-02")},
            {"datetime", FieldData::DateTime("2019-02-02 00:02:02")},
            {"img", FieldData::Blob(binary_blob)},
            {"valid", FieldData::Bool(false)}};
        auto m1 = txn.GetVertexIterator(1).GetAllFields();
        UT_EXPECT_TRUE(m1 == v1);
    }
    {
        UT_LOG() << "Testing string and blob data";
        lgraph::AutoCleanDir cleaner(path);
        Galaxy galaxy(path);
        galaxy.SetCurrentUser(ADMIN, ADMIN_PASS);
        GraphDB db = galaxy.OpenGraph("default");
        db.AddVertexLabel("v2",
                          std::vector<FieldSpec>({{"id", FieldType::INT32, false},
                                                  {"name", FieldType::STRING, false},
                                                  {"img", FieldType::BLOB, true}}),
                          VertexOptions("id"));
        db.AddVertexIndex("v2", "name", lgraph::IndexType::NonuniqueIndex);
        UT_EXPECT_ANY_THROW(db.AddVertexIndex(
            "v2", "img", lgraph::IndexType::GlobalUniqueIndex));  // blob cannot be indexed
        auto AddVertexWithString = [&](int32_t id, const std::string& name,
                                       const std::string& blob) {
            auto txn = db.CreateWriteTxn();
            int64_t vid =
                txn.AddVertex(std::string("v2"), std::vector<std::string>({"id", "name", "img"}),
                              std::vector<std::string>(
                                  {std::to_string(id), name, lgraph_api::base64::Encode(blob)}));
            txn.Commit();
            return vid;
        };
        int32_t id = 0;
        AddVertexWithString(id++, "alex", "");
        int64_t vid1 =
            AddVertexWithString(id++, std::string(lgraph::_detail::MAX_KEY_SIZE, 'a'), "");
        AddVertexWithString(id++, std::string(lgraph::_detail::MAX_KEY_SIZE + 1, 'a'), "");
        // string too long
        UT_EXPECT_ANY_THROW(
            AddVertexWithString(id++, std::string(lgraph::_detail::MAX_STRING_SIZE + 1, 'a'), ""));
        int64_t vid2 = AddVertexWithString(
            id++, "bob", std::string(lgraph::_detail::MAX_IN_PLACE_BLOB_SIZE, (char)255));
        int64_t vid3 = AddVertexWithString(id++, "charly", std::string(1 << 20, (char)255));
        db.DeleteVertexIndex("v2", "name");
        // now that "name" index has been removed, we can add larger strings
        int64_t vid4 = AddVertexWithString(
            id++, std::string(std::min<size_t>(lgraph::_detail::MAX_STRING_SIZE, 64<<20), 'a'), "");
        // check data
        {
            auto txn = db.CreateReadTxn();
            auto it1 = txn.GetVertexIterator(vid1);
            UT_EXPECT_EQ(it1.GetField("name").AsString(),
                         std::string(lgraph::_detail::MAX_KEY_SIZE, 'a'));
            UT_EXPECT_EQ(it1.GetField("img").AsBlob(), "");
            UT_EXPECT_EQ(txn.GetVertexIterator(vid2).GetField("img").AsBlob(),
                         std::string(lgraph::_detail::MAX_IN_PLACE_BLOB_SIZE, (char)255));
            UT_EXPECT_EQ(txn.GetVertexIterator(vid3).GetField("img").AsBlob(),
                         std::string(1 << 20, (char)255));
            UT_EXPECT_EQ(txn.GetVertexIterator(vid4).GetField("name").AsString(),
                         std::string(
                            std::min<size_t>(lgraph::_detail::MAX_STRING_SIZE, 64<<20),
                            'a'));
        }
        // test update
        {
            auto txn = db.CreateWriteTxn();
            auto vit1 = txn.GetVertexIterator(vid1);
            vit1.SetField("img", FieldData());
            auto vit2 = txn.GetVertexIterator(vid2);
            vit2.SetField("img", FieldData::Blob(std::string(1 << 20, (char)255)));
            auto vit3 = txn.GetVertexIterator(vid3);
            vit3.SetField("img", FieldData::Blob(std::string(100, 'a')));
            txn.Commit();

            txn = db.CreateReadTxn();
            vit1 = txn.GetVertexIterator(vid1);
            UT_EXPECT_EQ(vit1.GetField("img").AsBlob(), "");
            vit2 = txn.GetVertexIterator(vid2);
            UT_EXPECT_TRUE(vit2.GetField("img") ==
                           FieldData::Blob(std::string(1 << 20, (char)255)));
            vit3 = txn.GetVertexIterator(vid3);
            UT_EXPECT_TRUE(vit3.GetField("img") == FieldData::Blob(std::string(100, 'a')));
        }
        {
            // testing edges with tid
            LOG_INFO() << "Testing edges with tid";
            EdgeOptions options;
            options.temporal_field = "ts";
            options.temporal_field_order = lgraph::TemporalFieldOrder::ASC;
            db.AddEdgeLabel("et",
                            std::vector<lgraph_api::FieldSpec>{
                                lgraph_api::FieldSpec("ts", FieldType::INT64, false),
                                lgraph_api::FieldSpec("tt", FieldType::INT64, false)},
                            options);
            auto AddEdgeWithInt = [&](int64_t src, int64_t dst, const int64_t i) {
                auto txn = db.CreateWriteTxn();
                auto eid = txn.AddEdge(src, dst, "et", std::vector<std::string>{"ts", "tt"},
                                       std::vector<FieldData>{FieldData(i), FieldData(i)});
                txn.Commit();
                return eid;
            };
            auto eid1 = AddEdgeWithInt(vid1, vid2, 12);
            auto txn = db.CreateWriteTxn();
            auto eit1 = txn.GetOutEdgeIterator(eid1);
            FMA_ASSERT(eit1.IsValid());
            FMA_CHECK_EQ(eit1.GetField("ts").AsInt64(), 12);
            FMA_CHECK_EQ(eit1.GetField("tt").AsInt64(), 12);
#if __APPLE__
            eit1.SetField("tt", FieldData(20ll));
#else
            eit1.SetField("tt", FieldData(20l));
#endif
            auto eit2 = txn.GetInEdgeIterator(eid1);
            FMA_ASSERT(eit2.IsValid());
            FMA_CHECK_EQ(eit2.GetField("ts").AsInt64(), 12);
            FMA_CHECK_EQ(eit2.GetField("tt").AsInt64(), 20);
            txn.Abort();
        }
        // testing with edges
        db.AddEdgeLabel("e",
                        std::vector<lgraph_api::FieldSpec>{
                            lgraph_api::FieldSpec("blob", FieldType::BLOB, false)},
                        {});
        auto AddEdgeWithString = [&](int64_t src, int64_t dst, const std::string& blob) {
            auto txn = db.CreateWriteTxn();
            auto eid = txn.AddEdge(vid1, vid2, "e", std::vector<std::string>{"blob"},
                                   std::vector<std::string>{lgraph_api::base64::Encode(blob)});
            txn.Commit();
            return eid;
        };
        auto eid1 = AddEdgeWithString(vid1, vid2, "");
        auto eid2 = AddEdgeWithString(vid2, vid3, std::string(100, (char)122));
        auto eid3 = AddEdgeWithString(vid3, vid2, std::string(1 << 19, 123));
        {
            auto txn = db.CreateReadTxn();
            auto eit1 = txn.GetOutEdgeIterator(eid1, false);
            UT_EXPECT_EQ(eit1.GetField("blob").AsBlob(), "");
            auto eit2 = txn.GetOutEdgeIterator(eid2, false);
            UT_EXPECT_EQ(eit2.GetField("blob").AsBlob(), std::string(100, 122));
            auto eit3 = txn.GetOutEdgeIterator(eid3, false);
            UT_EXPECT_EQ(eit3.GetField("blob").AsBlob(), std::string(1 << 19, 123));
            txn.Abort();

            txn = db.CreateWriteTxn();
            eit1 = txn.GetOutEdgeIterator(eid1, false);
            // field cannot be null
            UT_EXPECT_ANY_THROW(eit1.SetField("blob", FieldData()));
            eit2 = txn.GetOutEdgeIterator(eid2, false);
            eit2.SetField("blob",
                          FieldData::String(lgraph_api::base64::Encode(std::string(1 << 16, 123))));
            eit3 = txn.GetOutEdgeIterator(eid3, false);
            eit3.SetField("blob", FieldData::Blob(std::string("1234")));
            txn.Commit();

            txn = db.CreateReadTxn();
            eit2 = txn.GetOutEdgeIterator(eid2, false);
            UT_EXPECT_EQ(eit2.GetField("blob").AsBlob(), std::string(1 << 16, 123));
            eit3 = txn.GetOutEdgeIterator(eid3, false);
            UT_EXPECT_EQ(eit3.GetField("blob").AsBlob(), std::string("1234"));
            txn.Abort();
        }

        // test count
        std::pair<uint64_t, uint64_t> count_befor, count_after;
        {
            auto txn = db.CreateReadTxn();
            count_befor = txn.Count();
        }
        db.RefreshCount();
        {
            auto txn = db.CreateReadTxn();
            count_after = txn.Count();
        }
        UT_EXPECT_EQ(count_befor, count_after);
    }
    {
        UT_LOG() << "Testing label modifications.";
        lgraph::AutoCleanDir cleaner(path);
        Galaxy galaxy(path);
        galaxy.SetCurrentUser(ADMIN, ADMIN_PASS);
        GraphDB db = galaxy.OpenGraph("default");
        UT_EXPECT_TRUE(db.AddVertexLabel("v1",
                                         std::vector<FieldSpec>({{"id", FieldType::INT32, false},
                                                                 {"img", FieldType::BLOB, true}}),
                                         VertexOptions("id")));

        UT_EXPECT_TRUE(db.AddVertexLabel("v2",
                                         std::vector<FieldSpec>({{"id2", FieldType::INT32, false},
                                                                 {"valid", FieldType::BOOL, true}}),
                                         VertexOptions("id2")));
        UT_EXPECT_TRUE(db.AddVertexLabel("test_v",
                                         std::vector<FieldSpec>({{"id3", FieldType::INT32, false},
                                                                 {"img3", FieldType::BOOL, true}}),
                                         VertexOptions("id3")));
        UT_EXPECT_TRUE(db.AlterVertexLabelAddFields(
            "test_v", std::vector<FieldSpec>({{"test3", FieldType::STRING, false}}),
            std::vector<FieldData>{FieldData("test_value")}));

        UT_EXPECT_TRUE(db.AlterVertexLabelModFields(
            "test_v", std::vector<FieldSpec>({{"test3", FieldType::STRING, true}})));

        UT_EXPECT_TRUE(db.AlterVertexLabelDelFields("test_v", std::vector<std::string>({"test3"})));

        UT_EXPECT_TRUE(db.AddEdgeLabel(
            "e1", std::vector<FieldSpec>({{"weight", FieldType::FLOAT, false}}), {}));
        UT_EXPECT_TRUE(db.AddEdgeLabel(
            "e2", std::vector<FieldSpec>({{"weight2", FieldType::DOUBLE, false}}), {}));
        UT_EXPECT_TRUE(db.AddEdgeLabel(
            "test_edge", std::vector<FieldSpec>({{"test_weight", FieldType::STRING, false}}), {}));

        UT_EXPECT_TRUE(db.AlterEdgeLabelAddFields(
            "test_edge", std::vector<FieldSpec>({{"test_edge", FieldType::STRING, false}}),
            std::vector<FieldData>{FieldData("test_value")}));

        UT_EXPECT_TRUE(db.AlterEdgeLabelModFields(
            "test_edge", std::vector<FieldSpec>({{"test_edge", FieldType::STRING, true}})));

        UT_EXPECT_TRUE(db.AlterLabelModEdgeConstraints(
            "test_edge", std::vector<std::pair<std::string, std::string>>{{"v1", "v2"}}));

        UT_EXPECT_TRUE(
            db.AlterEdgeLabelDelFields("test_edge", std::vector<std::string>({"test_edge"})));

        UT_EXPECT_TRUE(db.DeleteVertexLabel("v2"));
        UT_EXPECT_TRUE(db.DeleteEdgeLabel("e2"));
        auto txn = db.CreateWriteTxn();
        UT_EXPECT_THROW_MSG(txn.AddVertex("v2", std::vector<std::string>({"id2", "valid"}),
                                          std::vector<std::string>({"123", "true"})),
                            "Vertex label v2 does not exist");
        auto src = txn.AddVertex("v1", std::vector<std::string>({"id", "img"}),
                                 std::vector<std::string>({"1", "aaaa"}));
        auto dst = txn.AddVertex("v1", std::vector<std::string>({"id", "img"}),
                                 std::vector<std::string>({"2", "bbbb"}));
        UT_EXPECT_THROW_MSG(txn.AddEdge(src, dst, "e2", std::vector<std::string>({"weight2"}),
                                        std::vector<std::string>({"123"})),
                            "Edge label e2 does not exist");
    }
    {
        UT_LOG() << "Testing building index on large string id";
        lgraph::AutoCleanDir cleaner(path);
        Galaxy galaxy(path);
        galaxy.SetCurrentUser(ADMIN, ADMIN_PASS);
        GraphDB db = galaxy.OpenGraph("default");
        UT_EXPECT_TRUE(db.AddVertexLabel("v1",
                                         std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                                 {"id1", FieldType::STRING, false},
                                                                 {"img", FieldType::BLOB, true}}),
                                         VertexOptions("id")));
        {
            auto txn = db.CreateWriteTxn();
            auto vid1 = txn.AddVertex(
                "v1", std::vector<std::string>{"id", "id1"},
                std::vector<std::string>{std::string("2222"),
                                         std::string(lgraph::_detail::MAX_KEY_SIZE + 1, '1')});
            txn.Commit();
            db.AddVertexIndex("v1", "id1", lgraph::IndexType::GlobalUniqueIndex);
            txn = db.CreateWriteTxn();
            txn.GetVertexIterator(vid1).Delete();
            txn.Commit();
        }
        {
            // UT_EXPECT_TRUE(db.AddVertexIndex("v1", "id", true));
            auto txn = db.CreateWriteTxn();
            txn.AddVertex("v1", std::vector<std::string>{"id", "id1"},
                          std::vector<std::string>{std::string(
                                                       lgraph::_detail::MAX_KEY_SIZE + 1, '1'),
                                                   std::string("333")});
        }
        {
            UT_LOG() << "Test Exception";
            UT_EXPECT_THROW_MSG(THROW_CODE(InvalidGalaxy), "[InvalidGalaxy] Invalid Galaxy.");
            UT_EXPECT_THROW_MSG(THROW_CODE(InvalidGraphDB), "[InvalidGraphDB] Invalid GraphDB.");
            UT_EXPECT_THROW_MSG(THROW_CODE(InvalidTxn), "[InvalidTxn] Invalid transaction.");
            UT_EXPECT_THROW_MSG(THROW_CODE(InvalidIterator),
                                "[InvalidIterator] Invalid iterator.");
            UT_EXPECT_THROW_MSG(THROW_CODE(InvalidFork),
                                "[InvalidFork] Write transactions cannot be forked.");
            UT_EXPECT_THROW_MSG(THROW_CODE(TaskKilled), "[TaskKilled] Task is killed.");
            UT_EXPECT_THROW_MSG(THROW_CODE(IOError), "[IOError] IO Error.");
        }
        {
            UT_LOG() << "Test Lgraph Types";
            UT_EXPECT_EQ(to_string(FieldAccessLevel::NONE), "NONE");
            UT_EXPECT_EQ(to_string(FieldAccessLevel::READ), "READ");
            UT_EXPECT_EQ(to_string(FieldAccessLevel::WRITE), "WRITE");
            UT_EXPECT_EQ(to_string(FieldType::NUL), "NUL");
            UT_EXPECT_EQ(to_string(FieldType::INT32), "INT32");
            UT_EXPECT_EQ(to_string(FieldType::INT64), "INT64");
            UT_EXPECT_EQ(to_string(FieldType::FLOAT), "FLOAT");
            UT_EXPECT_EQ(to_string(FieldType::DOUBLE), "DOUBLE");
            UT_EXPECT_EQ(to_string(FieldType::DATE), "DATE");
            UT_EXPECT_EQ(to_string(FieldType::DATETIME), "DATETIME");
            UT_EXPECT_EQ(to_string(FieldType::BLOB), "BLOB");
            UT_EXPECT_EQ(PluginCodeTypeStr(PluginCodeType::CPP), "cpp");
            UT_EXPECT_EQ(PluginCodeTypeStr(PluginCodeType::ZIP), "zip");
            UT_EXPECT_TRUE(FieldData::Bool(true).AsBool());
            UT_EXPECT_EQ(FieldData::Int16(32767).AsInt16(), 32767);
            UT_EXPECT_EQ(FieldData::Int32(32767).AsInt32(), 32767);
            UT_EXPECT_EQ(FieldData::Int64(32767).AsInt64(), 32767);
            UT_EXPECT_EQ(FieldData::Float(1.2).AsFloat(), (float)(1.2));
            UT_EXPECT_EQ(FieldData::Double(1.2).AsDouble(), (double)(1.2));
            UT_EXPECT_THROW(FieldData::Bool(true).real(), std::bad_cast);
            UT_EXPECT_THROW(FieldData::Bool(true).integer(), std::bad_cast);
            UT_EXPECT_THROW(FieldData::Bool(true).string(), std::bad_cast);
            UT_EXPECT_THROW(FieldData::Bool(true).AsBase64Blob(), std::bad_cast);
            UT_EXPECT_FALSE(FieldData::Bool(true) > FieldData::Bool(true));
            UT_EXPECT_TRUE(FieldData::Bool(true) >= FieldData::Bool(true));
            UT_EXPECT_TRUE(FieldData::Float(1.2) >= FieldData::Float(1.19));
            UT_EXPECT_TRUE(FieldData::Int32(2) > FieldData::Float(1.2));
            UT_EXPECT_THROW((FieldData::String("2") > FieldData::Float(1.2)), std::runtime_error);
        }
    }
//     #endif
}

TEST_F(TestLGraphApi, CURDWithTooLongKey) {
    std::string path = "./testdb";
    auto ADMIN = lgraph::_detail::DEFAULT_ADMIN_NAME;
    auto ADMIN_PASS = lgraph::_detail::DEFAULT_ADMIN_PASS;
    {
        lgraph::AutoCleanDir cleaner(path);
        Galaxy galaxy(path);
        galaxy.SetCurrentUser(ADMIN, ADMIN_PASS);
        GraphDB db = galaxy.OpenGraph("default");
        UT_EXPECT_TRUE(db.AddVertexLabel("Person",
                                         std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                                 {"name", FieldType::STRING, false},
                                                                 {"tel", FieldType::STRING, true}}),
                                         VertexOptions("id")));

        UT_EXPECT_TRUE(db.AddEdgeLabel("Relation",
                                         std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                        {"name", FieldType::STRING, false},
                                                        {"instructions", FieldType::STRING, true}}),
                                       EdgeOptions()));

        std::vector<std::string> vp({"id", "name", "tel"});
        std::vector<std::string> ep({"id", "name", "instructions"});
        auto txn = db.CreateWriteTxn();
        auto vid1 = txn.AddVertex(std::string("Person"), vp,
                                  {std::string(lgraph::_detail::MAX_KEY_SIZE + 1, 'a'),
                                   std::string(lgraph::_detail::MAX_KEY_SIZE + 1, 'a'),
                                   std::string(lgraph::_detail::MAX_KEY_SIZE + 1, 'a')});

        auto vid2 = txn.AddVertex(std::string("Person"), vp,
                                  {std::string(lgraph::_detail::MAX_KEY_SIZE + 10, 'b'),
                                   std::string(lgraph::_detail::MAX_KEY_SIZE + 10, 'b'),
                                   std::string(lgraph::_detail::MAX_KEY_SIZE + 10, 'b')});

        auto vid3 = txn.AddVertex(std::string("Person"), vp,
                                  {std::string(lgraph::_detail::MAX_KEY_SIZE + 20, 'c'),
                                   std::string(lgraph::_detail::MAX_KEY_SIZE + 20, 'c'),
                                   std::string(lgraph::_detail::MAX_KEY_SIZE + 20, 'c')});

        auto vid4 = txn.AddVertex(std::string("Person"), vp,
                                  {std::string(lgraph::_detail::MAX_KEY_SIZE + 50, 'd'),
                                   std::string(lgraph::_detail::MAX_KEY_SIZE + 50, 'd'),
                                   std::string(lgraph::_detail::MAX_KEY_SIZE + 50, 'd')});

        txn.AddEdge(vid1, vid2, std::string("Relation"), ep,
                    {std::string(lgraph::_detail::MAX_KEY_SIZE + 50, 'z'),
                     std::string(lgraph::_detail::MAX_KEY_SIZE + 50, 'z'),
                     std::string(lgraph::_detail::MAX_KEY_SIZE + 50, 'z')});
        txn.AddEdge(vid2, vid3, std::string("Relation"), ep,
                    {std::string(lgraph::_detail::MAX_KEY_SIZE + 50, 'y'),
                     std::string(lgraph::_detail::MAX_KEY_SIZE + 50, 'y'),
                     std::string(lgraph::_detail::MAX_KEY_SIZE + 50, 'y')});
        txn.AddEdge(vid3, vid4, std::string("Relation"), ep,
                    {std::string(lgraph::_detail::MAX_KEY_SIZE + 50, 'x'),
                     std::string(lgraph::_detail::MAX_KEY_SIZE + 50, 'x'),
                     std::string(lgraph::_detail::MAX_KEY_SIZE + 50, 'x')});
        txn.AddEdge(vid4, vid1, std::string("Relation"), ep,
                    {std::string(lgraph::_detail::MAX_KEY_SIZE + 50, 'w'),
                     std::string(lgraph::_detail::MAX_KEY_SIZE + 50, 'w'),
                     std::string(lgraph::_detail::MAX_KEY_SIZE + 50, 'w')});
        txn.Commit();

        db.AddVertexIndex("Person", "name", lgraph::IndexType::GlobalUniqueIndex);
        db.AddVertexIndex("Person", "tel", lgraph::IndexType::NonuniqueIndex);
        db.AddEdgeIndex("Relation", "id", lgraph::IndexType::GlobalUniqueIndex);
        db.AddEdgeIndex("Relation", "name", lgraph::IndexType::PairUniqueIndex);
        db.AddEdgeIndex("Relation", "instructions", lgraph::IndexType::NonuniqueIndex);

        UT_EXPECT_TRUE(HasVertexIndex(db, "Person", "id", lgraph::IndexType::GlobalUniqueIndex));
        UT_EXPECT_EQ(GetVertexIndexValueNum(db, "Person", "id"), 4);
        UT_EXPECT_TRUE(HasVertexIndex(db, "Person", "name", lgraph::IndexType::GlobalUniqueIndex));
        UT_EXPECT_EQ(GetVertexIndexValueNum(db, "Person", "name"), 4);
        UT_EXPECT_TRUE(HasVertexIndex(db, "Person", "tel", lgraph::IndexType::NonuniqueIndex));
        UT_EXPECT_EQ(GetVertexIndexValueNum(db, "Person", "tel"), 4);
        UT_EXPECT_TRUE(HasEdgeIndex(db, "Relation", "id", lgraph::IndexType::GlobalUniqueIndex));
        UT_EXPECT_EQ(GetEdgeIndexValueNum(db, "Relation", "id"), 4);
        UT_EXPECT_TRUE(HasEdgeIndex(db, "Relation", "name", lgraph::IndexType::PairUniqueIndex));
        UT_EXPECT_EQ(GetEdgeIndexValueNum(db, "Relation", "name"), 4);
        UT_EXPECT_TRUE(HasEdgeIndex(db, "Relation", "instructions",
                                    lgraph::IndexType::NonuniqueIndex));
        UT_EXPECT_EQ(GetEdgeIndexValueNum(db, "Relation", "instructions"), 4);

        UT_EXPECT_TRUE(HasVertexIndexKey(
            db, "Person", "id", lgraph::FieldData(
                                    std::string(lgraph::_detail::MAX_KEY_SIZE, 'a'))));
        UT_EXPECT_TRUE(HasVertexIndexKey(
            db, "Person", "name", lgraph::FieldData(
                                    std::string(lgraph::_detail::MAX_KEY_SIZE, 'a'))));
        // since vertex NonuniqueIndex append vid in key, max key size is
        // lgraph::_detail::MAX_KEY_SIZE - VID_SIZE(5)
        UT_EXPECT_TRUE(HasVertexIndexKey(
            db, "Person", "tel", lgraph::FieldData(
                                     std::string(lgraph::_detail::MAX_KEY_SIZE - 5, 'a'))));
        UT_EXPECT_TRUE(HasEdgeIndexKey(
            db, "Relation", "id", lgraph::FieldData(
                                     std::string(lgraph::_detail::MAX_KEY_SIZE, 'z'))));
        // since edge PairUniqueIndex append src_vid and dst_vid in key, max key size is
        // lgraph::_detail::MAX_KEY_SIZE - 2 * VID_SIZE(5)
        UT_EXPECT_TRUE(HasEdgeIndexKey(
            db, "Relation", "name", lgraph::FieldData(
                                      std::string(lgraph::_detail::MAX_KEY_SIZE - 10, 'z'))));
        // since edge NonuniqueIndex append Euid in key, max key size is
        // lgraph::_detail::MAX_KEY_SIZE - EUID_SIZE(24)
        UT_EXPECT_TRUE(HasEdgeIndexKey(
            db, "Relation", "instructions", lgraph::FieldData(
                                        std::string(lgraph::_detail::MAX_KEY_SIZE - 24, 'z'))));
    }
}

TEST_F(TestLGraphApi, deleteLable) {
    std::string path = "./testdb";
    auto ADMIN = lgraph::_detail::DEFAULT_ADMIN_NAME;
    auto ADMIN_PASS = lgraph::_detail::DEFAULT_ADMIN_PASS;
    lgraph::AutoCleanDir cleaner(path);
    Galaxy galaxy(path);
    std::string db_path;
    namespace fs = std::filesystem;
    for (const auto& entry : fs::directory_iterator("testdb")) {
        if (fs::is_directory(entry.status())) {
            auto name = entry.path().filename().string();
            if (name == ".meta") {
                continue;
            }
            db_path = FMA_FMT("testdb/{}", name);
            break;
        }
    }
    auto check_dbs = [&db_path](int num) {
        std::string cmd = FMA_FMT("./mdb_stat {} | grep Entries", db_path);
        lgraph::SubProcess mdb_stat(cmd);
        mdb_stat.Wait();
        UT_EXPECT_EQ(mdb_stat.Stdout(), FMA_FMT("  Entries: {}\n", num));
    };
    check_dbs(6);
    galaxy.SetCurrentUser(ADMIN, ADMIN_PASS);
    GraphDB db = galaxy.OpenGraph("default");
    UT_EXPECT_TRUE(db.AddVertexLabel("Person",
                                     std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                             {"name", FieldType::STRING, false},
                                                             {"tel", FieldType::STRING, true}}),
                                     VertexOptions("id")));
    UT_EXPECT_TRUE(db.AddEdgeLabel("Relation",
                                   std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                       {"name", FieldType::STRING, false},
                                                       {"instructions", FieldType::STRING, true}}),
                                   EdgeOptions()));
    UT_EXPECT_TRUE(db.AddVertexIndex("Person", "name", IndexType::NonuniqueIndex));
    UT_EXPECT_TRUE(db.AddEdgeIndex("Relation", "name", IndexType::NonuniqueIndex));
    check_dbs(9);
    UT_EXPECT_TRUE(db.DeleteEdgeLabel("Relation"));
    UT_EXPECT_TRUE(db.DeleteVertexLabel("Person"));
    check_dbs(6);
    UT_EXPECT_TRUE(db.AddVertexLabel("Person1",
                                     std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                             {"name", FieldType::STRING, false},
                                                             {"tel", FieldType::STRING, true}}),
                                     VertexOptions("id")));
    UT_EXPECT_TRUE(db.AddEdgeLabel("Relation1",
                                   std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                       {"name", FieldType::STRING, false},
                                                       {"instructions", FieldType::STRING, true}}),
                                   EdgeOptions()));
    UT_EXPECT_TRUE(db.AddVertexIndex("Person1", "name", IndexType::NonuniqueIndex));
    UT_EXPECT_TRUE(db.AddEdgeIndex("Relation1", "name", IndexType::NonuniqueIndex));
    check_dbs(9);
    UT_EXPECT_TRUE(db.DeleteEdgeLabel("Relation1"));
    UT_EXPECT_TRUE(db.DeleteVertexLabel("Person1"));
    check_dbs(6);
}
