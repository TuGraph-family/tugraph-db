/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include <map>
#include <string>

#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "fma-common/utils.h"
#include "fma-common/unit_test_utils.h"

#include "gtest/gtest.h"

#include "lgraph/lgraph.h"
#include "restful/server/json_convert.h"

#include "./test_tools.h"
#include "./ut_utils.h"
using namespace lgraph_api;

class TestLGraphApi : public TuGraphTest {};

TEST_F(TestLGraphApi, ConcurrentVertexAdd) {
    size_t n_threads = 2;
    fma_common::Configuration config;
    config.Add(n_threads, "nt", true).Comment("Number of concurrent threads").SetMin(1);
    config.ParseAndFinalize(_ut_argc, _ut_argv);

    const std::string& dir = "./testdb";
    lgraph::AutoCleanDir cleaner(dir);
    Galaxy galaxy(dir, false, true);
    galaxy.SetCurrentUser(lgraph::_detail::DEFAULT_ADMIN_NAME,
        lgraph::_detail::DEFAULT_ADMIN_PASS);
    auto graph = galaxy.OpenGraph(lgraph::_detail::DEFAULT_GRAPH_DB_NAME);
    graph.AddVertexLabel(std::string("v"),
                          std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                  {"content", FieldType::STRING, true}}),
                          "id");

    Barrier bar(n_threads);
    std::atomic<size_t> n_success(0);
    std::atomic<size_t> n_fail(0);
    std::vector<std::thread> threads;
    for (size_t i = 0; i < n_threads; i++) {
        threads.emplace_back([&, i]() {
            auto txn = graph.CreateWriteTxn(true, false);
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
            } catch (lgraph_api::TxnConflictError&) {
                n_fail++;
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
    std::string path = "testdb";
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
        // add vertex label
        std::string vertex_label("vertex");
        std::vector<std::string> vertex_feild_name = {"id", "type", "content"};
        std::vector<std::string> vertex_values = {"id_1", "8", "content"};

        db.AddVertexLabel(vertex_label,
                          std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                  {"type", FieldType::INT8, false},
                                                  {"content", FieldType::STRING, true}}),
                          "id");
        UT_EXPECT_TRUE(db.IsVertexIndexed(vertex_label, "id"));
        // UT_EXPECT_TRUE(db.DeleteVertexIndex(vertex_label, "id"));
        Transaction txn_write = db.CreateWriteTxn();

        for (int i = 0; i < 4; i++) {
            std::vector<std::string> v = {std::to_string(i), "8", "content"};
            auto vid = txn_write.AddVertex(vertex_label, vertex_feild_name, v);
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
        db.DropAllData();
        // add vertex label
        std::string vlabel = "v";
        db.AddVertexLabel(vlabel,
                          std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                  {"type", FieldType::INT8, false},
                                                  {"content", FieldType::STRING, true}}),
                          "id");
        std::string elabel = "e";
        db.AddEdgeLabel(elabel, std::vector<FieldSpec>({{"weight", FieldType::FLOAT, false}}), {});
        UT_EXPECT_TRUE(db.IsVertexIndexed(vlabel, "id"));
        Transaction txn_write = db.CreateWriteTxn();
        for (int i = 0; i < 4; i++) {
            auto vid = txn_write.AddVertex(vlabel, {"id", "type"},
                                           {FieldData(std::to_string(i)), FieldData((int64_t)0)});
        }
        txn_write.Commit();

        UT_LOG() << "\tTesting EdgeIndex";
        {
            db.AddEdgeLabel("esw", std::vector<FieldSpec>({{"weight", FieldType::INT64, false}}),
                            {});
            db.AddVertexLabel("esk", std::vector<FieldSpec>({{"fs", FieldType::INT64, false}}),
                              "fs");
            db.AddEdgeLabel("esp", std::vector<FieldSpec>({{"weight", FieldType::INT64, false}}),
                            {});
            db.AddEdgeIndex("esw", "weight", false);
            db.AddVertexIndex("esk", "fs", false);
            db.AddEdgeIndex("esp", "weight", false);
            UT_EXPECT_TRUE(db.IsEdgeIndexed("esw", "weight"));
            UT_EXPECT_TRUE(db.IsEdgeIndexed("esp", "weight"));
            auto txn1 = db.CreateWriteTxn();
            for (int i = 0; i < 4; i++) {
                UT_EXPECT_TRUE(
                    txn1.AddEdge(i, (i + 1) % 4, "esw", {"weight"}, {FieldData(int64_t(i))}) ==
                    EdgeUid(i, (i + 1) % 4, 1, 0, 0));
            }
            for (int i = 0; i < 4; i++) {
                UT_EXPECT_TRUE(
                    txn1.AddEdge(i, (i + 1) % 4, "esp", {"weight"}, {FieldData(int64_t(i))}) ==
                    EdgeUid(i, (i + 1) % 4, 2, 0, 0));
            }
            auto eit1 = txn1.GetEdgeIndexIterator("esw", "weight", "1", "2");
            auto eit2 = txn1.GetEdgeIndexIterator("esw", "weight", "2", "3");
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
        UT_EXPECT_TRUE(false);
        ERR() << "An error occurred: " << e.what();
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
                          "id");
        UT_EXPECT_THROW_MSG(
            db.AddVertexLabel("v3", std::vector<FieldSpec>({{"f", FieldType::NUL, false}}), "f"),
            "NUL type");
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
                          "id");
        db.AddVertexIndex("v2", "name", false);
        UT_EXPECT_ANY_THROW(db.AddVertexIndex("v2", "img", true));  // blob cannot be indexed
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
        // key size too large
        UT_EXPECT_ANY_THROW(
            AddVertexWithString(id++, std::string(lgraph::_detail::MAX_KEY_SIZE + 1, 'a'), ""));
        // string too long
        UT_EXPECT_ANY_THROW(
            AddVertexWithString(id++, std::string(lgraph::_detail::MAX_STRING_SIZE + 1, 'a'), ""));
        int64_t vid2 = AddVertexWithString(
            id++, "bob", std::string(lgraph::_detail::MAX_IN_PLACE_BLOB_SIZE, (char)255));
        int64_t vid3 = AddVertexWithString(id++, "charly", std::string(1 << 20, (char)255));
        db.DeleteVertexIndex("v2", "name");
        // now that "name" index has been removed, we can add larger strings
        int64_t vid4 =
            AddVertexWithString(id++, std::string(lgraph::_detail::MAX_STRING_SIZE, 'a'), "");
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
                         std::string(lgraph::_detail::MAX_STRING_SIZE, 'a'));
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
            FMA_LOG() << "Testing edges with tid";
            db.AddEdgeLabel("et",
                            std::vector<lgraph_api::FieldSpec>{
                                lgraph_api::FieldSpec("ts", FieldType::INT64, false),
                                lgraph_api::FieldSpec("tt", FieldType::INT64, false)}, "ts",
                            {});
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
                                         "id"));
        UT_EXPECT_TRUE(db.AddVertexLabel("v2",
                                         std::vector<FieldSpec>({{"id2", FieldType::INT32, false},
                                                                 {"valid", FieldType::BOOL, true}}),
                                         "id2"));
        UT_EXPECT_TRUE(db.AddEdgeLabel(
            "e1", std::vector<FieldSpec>({{"weight", FieldType::FLOAT, false}}), {}));
        UT_EXPECT_TRUE(db.AddEdgeLabel(
            "e2", std::vector<FieldSpec>({{"weight2", FieldType::DOUBLE, false}}), {}));
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
                                         "id"));
        {
            auto txn = db.CreateWriteTxn();
            auto vid1 = txn.AddVertex(
                "v1", std::vector<std::string>{"id", "id1"},
                std::vector<std::string>{std::string("2222"),
                                         std::string(lgraph::_detail::MAX_KEY_SIZE + 1, '1')});
            txn.Commit();
            UT_EXPECT_THROW_MSG(db.AddVertexIndex("v1", "id1", true), "too long");
            UT_EXPECT_THROW_MSG(db.AddVertexIndex("v1", "id1", true), "too long");
            txn = db.CreateWriteTxn();
            txn.GetVertexIterator(vid1).Delete();
            txn.Commit();
        }
        {
            // UT_EXPECT_TRUE(db.AddVertexIndex("v1", "id", true));
            auto txn = db.CreateWriteTxn();
            UT_EXPECT_THROW_MSG(
                txn.AddVertex(
                    "v1", std::vector<std::string>{"id", "id1"},
                    std::vector<std::string>{std::string(lgraph::_detail::MAX_KEY_SIZE + 1, '1'),
                                             std::string("333")}),
                "key size");
        }
    }
    // #endif
}
