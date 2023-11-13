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

#include "fma-common/configuration.h"
#include "fma-common/logger.h"
#include "fma-common/utils.h"
#include "gtest/gtest.h"

#include "core/lightning_graph.h"
#include "./graph_factory.h"
#include "./test_tools.h"
#include "./random_port.h"
#include "./ut_utils.h"

std::map<std::string, lgraph::FieldSpec> GetCurrSchema(const std::string& dir, bool is_vertex) {
    using namespace lgraph;
    lgraph::DBConfig conf;
    conf.dir = dir;
    lgraph::LightningGraph lg(conf);
    auto txn = lg.CreateReadTxn();
    return is_vertex ? txn.GetSchemaAsMap(true, "person") : txn.GetSchemaAsMap(false, "knows");
}

std::map<std::string, lgraph::FieldData> GetVertexFields(lgraph::Transaction& txn,
                                                         lgraph::VertexId vid) {
    auto d = txn.GetVertexFields(txn.GetVertexIterator(vid));
    return std::map<std::string, lgraph::FieldData>(d.begin(), d.end());
}

RandomSeed sc_seed(0);

static std::string RandomString(size_t n) {
    std::string str(n, 0);
    for (size_t i = 0; i < n; i++) str[i] = (rand_r(&sc_seed) % 26) + 'a';
    return str;
}

static void CreateSampleDB(const std::string& dir, bool detach_property) {
    using namespace lgraph;
    lgraph::DBConfig conf;
    conf.dir = dir;
    lgraph::LightningGraph lg(conf);
    VertexOptions vo;
    vo.primary_field = "id";
    vo.detach_property = detach_property;
    UT_EXPECT_TRUE(lg.AddLabel(
        "person",
        std::vector<FieldSpec>(
            {FieldSpec("id", FieldType::INT32, false), FieldSpec("name", FieldType::STRING, false),
             FieldSpec("age", FieldType::FLOAT, true), FieldSpec("img", FieldType::BLOB, true),
             FieldSpec("desc", FieldType::STRING, true), FieldSpec("img2", FieldType::BLOB, true)}),
        true, vo));
    lg.BlockingAddIndex("person", "name", lgraph::IndexType::NonuniqueIndex, true);
    lg.BlockingAddIndex("person", "age", lgraph::IndexType::NonuniqueIndex, true);
    EdgeOptions options;
    options.temporal_field = "ts";
    options.temporal_field_order = lgraph::TemporalFieldOrder::ASC;
    options.detach_property = detach_property;
    UT_EXPECT_TRUE(lg.AddLabel("knows",
                               std::vector<FieldSpec>({FieldSpec("weight", FieldType::FLOAT, true),
                                                       FieldSpec("ts", FieldType::INT64, true)}),
                               false, options));
    lg.BlockingAddIndex("knows", "weight", lgraph::IndexType::NonuniqueIndex, false);
    auto txn = lg.CreateWriteTxn();
    VertexId v0 =
        txn.AddVertex(std::string("person"),
                      std::vector<std::string>({"id", "name", "age", "img", "desc", "img2"}),
                      std::vector<std::string>({"1", "p1", "11.5", "", "desc for p1",
                                                lgraph_api::base64::Encode("img2")}));
    VertexId v1 = txn.AddVertex(
        std::string("person"),
        std::vector<std::string>({"id", "name", "age", "img", "desc", "img2"}),
        std::vector<std::string>(
            {"2", "p2", "12", lgraph_api::base64::Encode(std::string(8192, 'a')),
             std::string(4096, 'b'), lgraph_api::base64::Encode(std::string(8192, 'c'))}));

    txn.AddEdge(v0, v1, std::string("knows"), std::vector<std::string>({"weight"}),
                std::vector<std::string>({"1.25"}));
    txn.AddEdge(v0, v1, std::string("knows"), std::vector<std::string>({"weight"}),
                std::vector<std::string>({"1.5"}));
    txn.AddEdge(v1, v0, std::string("knows"), std::vector<std::string>({"weight"}),
                std::vector<std::string>({"10.25"}));
    txn.AddEdge(v1, v0, std::string("knows"), std::vector<std::string>({"weight"}),
                std::vector<std::string>({"10.5"}));
    txn.Commit();
}

static void CreateLargeSampleDB(const std::string& dir, bool detach_property) {
    using namespace lgraph;
    lgraph::DBConfig conf;
    conf.dir = dir;
    lgraph::LightningGraph lg(conf);
    VertexOptions vo;
    vo.primary_field = "name";
    vo.detach_property = detach_property;
    UT_EXPECT_TRUE(
        lg.AddLabel("large",
                    std::vector<FieldSpec>({FieldSpec("name", FieldType::STRING, false),
                                            FieldSpec("number", FieldType::INT32, false)}),
                    true, vo));

    auto txn = lg.CreateWriteTxn();
    const size_t commit_size = 110000;  // commit size when moding field is 100000
    for (int vertices_num = 0; vertices_num < commit_size; vertices_num++) {
        std::string field_value = RandomString(4);
        txn.AddVertex(std::string("large"), std::vector<std::string>({"name", "number"}),
                      std::vector<std::string>({std::to_string(vertices_num) + field_value,
                                                std::to_string(vertices_num)}));
    }
    txn.Commit();
}

class TestSchemaChange : public TuGraphTestWithParam<bool> {};

INSTANTIATE_TEST_CASE_P(TestSchemaChange, TestSchemaChange, testing::Values(false, true));

TEST_P(TestSchemaChange, ModifyFields) {
    using namespace lgraph;
    std::string dir = "./testdb";
    AutoCleanDir cleaner(dir);

    UT_LOG() << "Test Schema::AddFields, DelFields and ModFields";
    Schema s1;
    s1.SetSchema(
        true,
        std::vector<FieldSpec>(
            {FieldSpec("id", FieldType::INT32, false), FieldSpec("id2", FieldType::INT32, false),
             FieldSpec("name1", FieldType::STRING, true),
             FieldSpec("name2", FieldType::STRING, true), FieldSpec("blob", FieldType::BLOB, true),
             FieldSpec("age", FieldType::FLOAT, false)}),
        "id", "", {}, {});
    std::map<std::string, FieldSpec> fields = s1.GetFieldSpecsAsMap();
    {
        Schema s2(s1);
        s2.AddFields(std::vector<FieldSpec>({FieldSpec("id3", FieldType::INT32, false)}));
        UT_EXPECT_TRUE(s2.GetFieldExtractor("id3")->GetFieldSpec() ==
                       FieldSpec("id3", FieldType::INT32, false));
        auto fmap = s2.GetFieldSpecsAsMap();
        UT_EXPECT_EQ(fmap.size(), fields.size() + 1);
        fmap.erase("id3");
        UT_EXPECT_TRUE(fmap == fields);
        UT_EXPECT_TRUE(s2.HasBlob());
        {
            Schema s3(s2);
            UT_EXPECT_THROW(
                s2.AddFields(std::vector<FieldSpec>({FieldSpec("id", FieldType::INT32, false)})),
                FieldAlreadyExistsException);
        }
        {
            Schema s3(s2);
            std::vector<FieldSpec> to_add;
            for (size_t i = 0; i < _detail::MAX_NUM_FIELDS - s3.GetNumFields(); i++) {
                to_add.emplace_back(UT_FMT("a{}", i), FieldType::INT64, false);
            }
            s3.AddFields(to_add);
            UT_EXPECT_EQ(s3.GetNumFields(), _detail::MAX_NUM_FIELDS);
        }
        {
            Schema s3(s2);
            std::vector<FieldSpec> to_add;
            for (size_t i = 0; i < _detail::MAX_NUM_FIELDS - s3.GetNumFields() + 1; i++) {
                to_add.emplace_back(UT_FMT("a{}", i), FieldType::INT64, false);
            }
            UT_EXPECT_THROW(s3.AddFields(to_add), TooManyFieldsException);
        }
    }
    {
        Schema s2(s1);
        std::vector<std::string> to_del = {"blob", "name2"};
        s2.DelFields(to_del);
        UT_EXPECT_TRUE(!s2.HasBlob());
        auto fmap = s2.GetFieldSpecsAsMap();
        auto old_fields = fields;
        for (auto& f : to_del) old_fields.erase(f);
        UT_EXPECT_TRUE(fmap == old_fields);
        UT_EXPECT_THROW(s2.DelFields(std::vector<std::string>({"no_such_field"})),
                        FieldNotFoundException);
    }
    {
        Schema s2(s1);
        std::vector<FieldSpec> mod = {FieldSpec("id", FieldType::INT64, false),
                                      FieldSpec("name1", FieldType::STRING, false),
                                      FieldSpec("blob", FieldType::STRING, false)};
        s2.ModFields(mod);
        UT_EXPECT_TRUE(!s2.HasBlob());
        auto fmap = s2.GetFieldSpecsAsMap();
        auto old_fields = fields;
        for (auto& f : mod) old_fields[f.name] = f;
        UT_EXPECT_TRUE(fmap == old_fields);
        UT_EXPECT_THROW(s2.ModFields(std::vector<FieldSpec>(
                            {FieldSpec("no_such_field", FieldType::BLOB, true)})),
                        FieldNotFoundException);
        UT_EXPECT_THROW(
            s2.ModFields(std::vector<FieldSpec>({FieldSpec("blob", FieldType::NUL, true)})),
            FieldCannotBeNullTypeException);
    }
}

TEST_P(TestSchemaChange, DelFields) {
    using namespace lgraph;
    std::string dir = "./testdb";
    AutoCleanDir cleaner(dir);

    auto DumpGraph = [](LightningGraph& g, Transaction& txn) {
        std::string str;
        for (auto it = txn.GetVertexIterator(); it.IsValid(); it.Next()) {
            str += FMA_FMT("{}: {}\n", it.GetId(), txn.GetVertexFields(it));
            for (auto eit = it.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                str += FMA_FMT("    -[{}]->[{}]: {}\n", eit.GetUid(), eit.GetDst(),
                               txn.GetEdgeFields(eit));
            }
            for (auto eit = it.GetInEdgeIterator(); eit.IsValid(); eit.Next()) {
                str += FMA_FMT("    <-[{}]-[{}]: {}\n", eit.GetUid(), eit.GetSrc(),
                               txn.GetEdgeFields(eit));
            }
        }
        FMA_LOG() << "Current Graph: \n" << str;
    };

    DBConfig conf;
    conf.dir = dir;
    UT_LOG() << "Testing del field";
    CreateSampleDB(dir, GetParam());
    auto orig_v_schema = GetCurrSchema(dir, true);
    auto orig_e_schema = GetCurrSchema(dir, false);
    {
        LightningGraph graph(conf);
        size_t n_changed = 0;
        UT_EXPECT_TRUE(graph.AlterLabelDelFields("person", std::vector<std::string>({"age", "img"}),
                                                 true, &n_changed));
        UT_EXPECT_EQ(n_changed, 2);
    }
    {
        LightningGraph graph(conf);
        auto txn = graph.CreateReadTxn();
        DumpGraph(graph, txn);
        auto labels = txn.GetAllLabels(true);
        UT_EXPECT_EQ(labels.size(), 1);
        UT_EXPECT_EQ(labels.front(), "person");
        auto schema = txn.GetSchemaAsMap(true, std::string("person"));
        orig_v_schema.erase("age");
        orig_v_schema.erase("img");
        UT_EXPECT_TRUE(orig_v_schema == schema);
        auto indexes = txn.ListVertexIndexByLabel("person");
        auto edge_indexes = txn.ListEdgeIndexByLabel("knows");
        UT_EXPECT_EQ(indexes.size(), 2);
        UT_EXPECT_EQ(edge_indexes.size(), 1);
        auto iit = txn.GetVertexIndexIterator("person", "name", "p1", "p1");
        UT_EXPECT_TRUE(iit.IsValid());
        auto eit = txn.GetEdgeIndexIterator("knows", "weight", "1.25", "1.25");
        eit.RefreshContentIfKvIteratorModified();
        UT_EXPECT_TRUE(eit.IsValid());
        auto fvs = txn.GetVertexFields(txn.GetVertexIterator(iit.GetVid()));
        std::map<std::string, FieldData> fvmap(fvs.begin(), fvs.end());
        UT_EXPECT_TRUE(fvmap["id"] == FieldData(1));
        UT_EXPECT_TRUE(fvmap["name"] == FieldData("p1"));
        UT_EXPECT_TRUE(fvmap["desc"] == FieldData("desc for p1"));
        UT_EXPECT_TRUE(fvmap["img2"] == FieldData::Blob("img2"));
        auto iit2 = txn.GetVertexIndexIterator("person", "id", "2", "2");
        UT_EXPECT_TRUE(iit2.IsValid());
        fvs = txn.GetVertexFields(txn.GetVertexIterator(iit2.GetVid()));
        fvmap = std::map<std::string, FieldData>(fvs.begin(), fvs.end());
        UT_EXPECT_TRUE(fvmap["id"] == FieldData(2));
        UT_EXPECT_TRUE(fvmap["name"] == FieldData("p2"));
        UT_EXPECT_TRUE(fvmap["desc"] == FieldData(std::string(4096, 'b')));
        UT_EXPECT_TRUE(fvmap["img2"] == FieldData::Blob(std::string(8192, 'c')));
    }
    {
        LightningGraph graph(conf);
        size_t n_changed = 0;
        UT_EXPECT_TRUE(graph.AlterLabelDelFields("knows", std::vector<std::string>({"weight"}),
                                                 false, &n_changed));
        UT_EXPECT_EQ(n_changed, 4);
    }
    {
        LightningGraph graph(conf);
        size_t n_changed = 0;
        UT_EXPECT_THROW(
            graph.AlterLabelDelFields("knows", std::vector<std::string>({"ts"}), false, &n_changed),
            lgraph::FieldCannotBeDeletedException);
        UT_EXPECT_EQ(n_changed, 0);
    }
    {
        auto schema = GetCurrSchema(dir, false);
        UT_EXPECT_TRUE(!schema.empty());
        LightningGraph graph(conf);
        auto txn = graph.CreateReadTxn();
        auto vit0 = txn.GetVertexIterator(0);
        auto vit1 = txn.GetVertexIterator(1);
        UT_EXPECT_EQ(vit0.GetNumOutEdges(), 2);
        UT_EXPECT_EQ(vit0.GetNumInEdges(), 2);
        UT_EXPECT_FALSE(txn.GetEdgeFields(vit0.GetOutEdgeIterator()).empty());
        UT_EXPECT_FALSE(txn.GetEdgeFields(vit0.GetInEdgeIterator()).empty());
    }
}

TEST_P(TestSchemaChange, ModData) {
    using namespace lgraph;
    std::string dir = "./testdb";
    AutoCleanDir cleaner(dir);

    DBConfig conf;
    conf.dir = dir;
    UT_LOG() << "Testing mod with data";
    CreateSampleDB(dir, GetParam());
    auto orig_v_schema = GetCurrSchema(dir, true);
    auto orig_e_schema = GetCurrSchema(dir, false);
    {
        LightningGraph graph(conf);
        size_t n_changed = 0;
        UT_EXPECT_TRUE(!graph.AlterLabelModFields(
            "no_such_label", std::vector<FieldSpec>({FieldSpec("img", FieldType::STRING, true)}),
            true, &n_changed));
        UT_EXPECT_THROW(
            graph.AlterLabelModFields(
                "person",
                std::vector<FieldSpec>({FieldSpec("no_such_field", FieldType::STRING, true)}), true,
                &n_changed),
            lgraph::FieldNotFoundException);
        UT_EXPECT_THROW(
            graph.AlterLabelModFields(
                "person", std::vector<FieldSpec>({FieldSpec("img", FieldType::STRING, true)}), true,
                &n_changed),
            lgraph::InputError);  // blob cannot be converted to other types
        UT_EXPECT_THROW(
            graph.AlterLabelModFields(
                "person", std::vector<FieldSpec>({FieldSpec("age", FieldType::STRING, true)}), true,
                &n_changed),
            lgraph::ParseIncompatibleTypeException);  // cannot convert float to string
    }
    {
        LightningGraph graph(conf);
        size_t n_changed = 0;
        UT_EXPECT_TRUE(graph.AlterLabelModFields(
            "person",
            std::vector<FieldSpec>({FieldSpec("age", FieldType::INT16, false),
                                    FieldSpec("desc", FieldType::BLOB, true)}),
            true, &n_changed));
        UT_EXPECT_EQ(n_changed, 2);
        UT_EXPECT_TRUE(graph.AlterLabelModFields(
            "knows", std::vector<FieldSpec>({FieldSpec("weight", FieldType::DOUBLE, true)}), false,
            &n_changed));
        UT_EXPECT_EQ(n_changed, 4);
    }
    orig_v_schema["age"] = FieldSpec("age", FieldType::INT16, false);
    orig_v_schema["desc"] = FieldSpec("desc", FieldType::BLOB, true);
    orig_e_schema["weight"].type = FieldType::DOUBLE;
    {
        auto curr_v_schema = GetCurrSchema(dir, true);
        UT_EXPECT_TRUE(curr_v_schema == orig_v_schema);
        auto curr_e_schema = GetCurrSchema(dir, false);
        UT_EXPECT_TRUE(curr_e_schema == orig_e_schema);
        LightningGraph graph(conf);
        auto txn = graph.CreateReadTxn();
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("age")) == FieldData::Int16(11));
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("desc")) ==
                       FieldData::Blob("desc for p1"));
        UT_EXPECT_TRUE(txn.GetVertexField(1, std::string("desc")) ==
                       FieldData::Blob(std::string(4096, 'b')));
        UT_EXPECT_TRUE(txn.GetVertexField(1, std::string("img2")) ==
                       FieldData::Blob(std::string(8192, 'c')));
        auto vit0 = txn.GetVertexIterator(0);
        auto eit = vit0.GetOutEdgeIterator();
        UT_EXPECT_TRUE(txn.GetEdgeField(eit, std::string("weight")) == FieldData::Double(1.25));
        eit.Next();
        UT_EXPECT_TRUE(txn.GetEdgeField(eit, std::string("weight")) == FieldData::Double(1.5));
        auto ieit = vit0.GetInEdgeIterator();
        UT_EXPECT_TRUE(txn.GetEdgeField(ieit, std::string("weight")) == FieldData::Double(10.25));
        ieit.Next();
        UT_EXPECT_TRUE(txn.GetEdgeField(ieit, std::string("weight")) == FieldData::Double(10.5));
        UT_EXPECT_TRUE(!txn.IsIndexed("person", "age"));  // index has been removed
    }
}

TEST_P(TestSchemaChange, UpdateConstraints) {
    using namespace lgraph;
    std::string dir = "./testdb";
    AutoCleanDir cleaner(dir);

    DBConfig conf;
    conf.dir = dir;

    UT_LOG() << "Testing upate edge constraints";
    CreateSampleDB(dir, GetParam());
    EdgeConstraints new_ec = {{"ver1", "ver2"}, {"ver3", "ver4"}};
    {
        LightningGraph graph(conf);
        auto txn = graph.CreateReadTxn();
        auto ec = txn.GetEdgeConstraints("knows");
        txn.Abort();
        UT_EXPECT_TRUE(ec.empty());
        UT_EXPECT_TRUE(graph.AlterLabelModEdgeConstraints("knows", new_ec));
    }
    {
        LightningGraph graph(conf);
        auto txn = graph.CreateReadTxn();
        auto ec = txn.GetEdgeConstraints("knows");
        UT_EXPECT_TRUE(ec == new_ec);
    }
}

TEST_P(TestSchemaChange, ModAndAddfieldWithData) {
    using namespace lgraph;
    std::string dir = "./testdb";
    AutoCleanDir cleaner(dir);

    DBConfig conf;
    conf.dir = dir;

    UT_LOG() << "Testing mod with large data";
    {
        AutoCleanDir cleaner(dir);
        CreateLargeSampleDB(dir, GetParam());
        {
            LightningGraph graph(conf);
            size_t n_changed = 0;
            // Test issue 85, Error: Nested transaction is forbidden
            UT_EXPECT_TRUE(graph.AlterLabelModFields(
                "large",
                std::vector<FieldSpec>({FieldSpec("name", FieldType::STRING, false),
                                        FieldSpec("number", FieldType::INT32, true)}),
                true, &n_changed));
        }
    }

    UT_LOG() << "Testing add field with data";
    {
        AutoCleanDir cleaner(dir);
        CreateSampleDB(dir, GetParam());
        auto orig_v_schema = GetCurrSchema(dir, true);
        auto orig_e_schema = GetCurrSchema(dir, false);
        {
            LightningGraph graph(conf);
            size_t n_changed = 0;
            UT_EXPECT_TRUE(!graph.AlterLabelAddFields(
                "no_such_label",
                std::vector<FieldSpec>({FieldSpec("img3", FieldType::STRING, true)}),
                std::vector<FieldData>({FieldData()}), true, &n_changed));
            UT_EXPECT_THROW(
                graph.AlterLabelAddFields(
                    "person",
                    std::vector<FieldSpec>({FieldSpec("some_field", FieldType::STRING, true)}),
                    std::vector<FieldData>(), true, &n_changed),
                lgraph::InputError);  // # field and default_values does not match
            UT_EXPECT_THROW(
                graph.AlterLabelAddFields(
                    "person", std::vector<FieldSpec>({FieldSpec("img", FieldType::STRING, true)}),
                    std::vector<FieldData>({FieldData(FieldData())}), true, &n_changed),
                lgraph::FieldAlreadyExistsException);  // field already exists
            UT_EXPECT_THROW(
                graph.AlterLabelAddFields(
                    "person",
                    std::vector<FieldSpec>({FieldSpec("string_field", FieldType::STRING, true)}),
                    std::vector<FieldData>({FieldData(123)}), true, &n_changed),
                lgraph::ParseIncompatibleTypeException);  // cannot convert 123 to string
        }
        {
            LightningGraph graph(conf);
            size_t n_changed = 0;
            UT_EXPECT_TRUE(graph.AlterLabelAddFields(
                "person",
                std::vector<FieldSpec>({FieldSpec("income", FieldType::INT64, true),
                                        FieldSpec("addr", FieldType::STRING, true)}),
                std::vector<FieldData>({FieldData::Int64(100), FieldData()}), true, &n_changed));
            UT_EXPECT_EQ(n_changed, 2);
            UT_EXPECT_TRUE(graph.AlterLabelAddFields(
                "knows", std::vector<FieldSpec>({FieldSpec("since", FieldType::DATE, true)}),
                std::vector<FieldData>({FieldData::DateTime("2020-01-01 00:00:01")}), false,
                &n_changed));
            UT_EXPECT_EQ(n_changed, 4);
        }
        orig_v_schema["income"] = FieldSpec("income", FieldType::INT64, true);
        orig_v_schema["addr"] = FieldSpec("addr", FieldType::STRING, true);
        orig_e_schema["since"] = FieldSpec("since", FieldType::DATE, true);
        {
            auto curr_v_schema = GetCurrSchema(dir, true);
            UT_EXPECT_TRUE(curr_v_schema == orig_v_schema);
            auto curr_e_schema = GetCurrSchema(dir, false);
            UT_EXPECT_TRUE(curr_e_schema == orig_e_schema);
            LightningGraph graph(conf);
            auto txn = graph.CreateReadTxn();
            UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("income")) == FieldData::Int64(100));
            UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("addr")).IsNull());
            UT_EXPECT_TRUE(txn.GetVertexField(1, std::string("desc")) ==
                           FieldData::String(std::string(4096, 'b')));
            UT_EXPECT_TRUE(txn.GetVertexField(1, std::string("img2")) ==
                           FieldData::Blob(std::string(8192, 'c')));
            auto vit0 = txn.GetVertexIterator(0);
            auto eit = vit0.GetOutEdgeIterator();
            UT_EXPECT_TRUE(txn.GetEdgeField(eit, std::string("weight")) == FieldData::Double(1.25));
            eit.Next();
            UT_EXPECT_TRUE(txn.GetEdgeField(eit, std::string("since")) ==
                           FieldData::Date("2020-01-01"));
            auto ieit = vit0.GetInEdgeIterator();
            UT_EXPECT_TRUE(txn.GetEdgeField(ieit, std::string("weight")) ==
                           FieldData::Double(10.25));
            ieit.Next();
            UT_EXPECT_TRUE(txn.GetEdgeField(ieit, std::string("since")) ==
                           FieldData::Date("2020-01-01"));
            UT_EXPECT_EQ(txn.ListVertexIndexByLabel("person").size(), 3);
        }
    }
}

TEST_P(TestSchemaChange, DelLabel) {
    using namespace lgraph;
    std::string dir = "./testdb";
    AutoCleanDir cleaner(dir);

    DBConfig conf;
    conf.dir = dir;

    auto ScanGraph = [](lgraph_api::GraphDB& db, size_t& nv, size_t& ne) {
        auto txn = db.CreateReadTxn();
        nv = 0;
        ne = 0;
        for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
            nv++;
            for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) ne++;
        }
    };
    UT_LOG() << "Testing del label";
    {
        AutoCleanDir cleaner(dir);
        GraphFactory::create_yago(dir);
        lgraph_api::Galaxy galaxy(dir);
        galaxy.SetCurrentUser(lgraph::_detail::DEFAULT_ADMIN_NAME,
                              lgraph::_detail::DEFAULT_ADMIN_PASS);
        auto db = galaxy.OpenGraph("default");
        size_t nv, ne;
        ScanGraph(db, nv, ne);
        size_t n_changed = 0;
        UT_EXPECT_TRUE(db.DeleteVertexLabel("Person", &n_changed));
        UT_EXPECT_EQ(n_changed, 13);
        size_t nv_now, ne_now;
        ScanGraph(db, nv_now, ne_now);
        UT_EXPECT_EQ(nv - n_changed, nv_now);
    }

    {
        AutoCleanDir cleaner(dir);
        GraphFactory::create_yago(dir);
        lgraph_api::Galaxy galaxy(dir);
        galaxy.SetCurrentUser(lgraph::_detail::DEFAULT_ADMIN_NAME,
                              lgraph::_detail::DEFAULT_ADMIN_PASS);
        auto db = galaxy.OpenGraph("default");
        size_t nv, ne;
        ScanGraph(db, nv, ne);
        size_t n_changed = 0;
        UT_EXPECT_TRUE(db.DeleteEdgeLabel("ACTED_IN", &n_changed));
        UT_LOG() << "Number of edges deleted: " << n_changed;
        size_t nv_now, ne_now;
        ScanGraph(db, nv_now, ne_now);
        UT_EXPECT_EQ(ne - n_changed, ne_now);
        UT_EXPECT_EQ(nv, nv_now);
    }
    UT_LOG() << "Testing illegal field name";
    {
        Schema s1;
        s1.SetSchema(true,
                     std::vector<FieldSpec>({FieldSpec("id", FieldType::INT32, false),
                                             FieldSpec("id2", FieldType::INT32, false),
                                             FieldSpec("name1", FieldType::STRING, true),
                                             FieldSpec("name2", FieldType::STRING, true),
                                             FieldSpec("blob", FieldType::BLOB, true),
                                             FieldSpec("age", FieldType::FLOAT, false)}),
                     "id", "", {}, {});
        UT_EXPECT_THROW(
            s1.AddFields(std::vector<FieldSpec>({FieldSpec("SKIP", FieldType::STRING, false)})),
            lgraph::InputError);
        UT_EXPECT_THROW(
            s1.AddFields(std::vector<FieldSpec>({FieldSpec("SRC_ID", FieldType::STRING, false)})),
            lgraph::InputError);
        UT_EXPECT_THROW(
            s1.AddFields(std::vector<FieldSpec>({FieldSpec("DST_ID", FieldType::STRING, false)})),
            lgraph::InputError);
    }

    UT_LOG() << "Testing delete field name";
    {
        Schema s;
        s.SetSchema(false,
                    std::vector<FieldSpec>({FieldSpec("id", FieldType::INT32, false),
                                            FieldSpec("id2", FieldType::INT32, false),
                                            FieldSpec("name1", FieldType::STRING, true),
                                            FieldSpec("name2", FieldType::STRING, true),
                                            FieldSpec("blob", FieldType::BLOB, true),
                                            FieldSpec("age", FieldType::FLOAT, false)}),
                    "", "id", {}, {});
        UT_EXPECT_THROW(s.DelFields(std::vector<std::string>{"id"}), FieldCannotBeDeletedException);
        s.AddFields({FieldSpec("telphone", FieldType::STRING, false)});
        UT_EXPECT_EQ(s.HasTemporalField(), true);
        UT_EXPECT_EQ(s.GetTemporalField(), "id");
        UT_EXPECT_EQ(s.GetTemporalFieldId(), s.GetFieldId("id"));
    }
    fma_common::SleepS(1);  // waiting for memory reclaiming by async task
}
