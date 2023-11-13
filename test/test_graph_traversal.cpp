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
#include "fma-common/file_system.h"
#include "fma-common/logging.h"
#include "fma-common/string_formatter.h"

#include "gtest/gtest.h"

#include "core/kv_store.h"
#include "core/graph.h"
#include "core/schema.h"
#include "core/transaction.h"
#include "lgraph/lgraph_traversal.h"
#include "./ut_utils.h"
using namespace std;
using namespace fma_common;
using namespace lgraph_api;
using namespace lgraph;
using namespace lgraph_api::traversal;

class TestGraphTraversal : public TuGraphTest {};

TEST_F(TestGraphTraversal, GraphTraversal) {
    std::string path("./testtmp");
    lgraph_api::Galaxy galaxy("./testtmp",
                              lgraph::_detail::DEFAULT_ADMIN_NAME,
                              lgraph::_detail::DEFAULT_ADMIN_PASS,
                              false, true);
    lgraph_api::GraphDB db = galaxy.OpenGraph("default");
    // database add/delete data vertex must be no tranaction declear
    db.DropAllData();
    db.DropAllVertex();
    UT_EXPECT_TRUE(db.AddVertexLabel(
        "vertex",
        std::vector<FieldSpec>(
            {{"id", STRING, false}, {"type", FieldType::INT8, false}, {"content", STRING, true}}),
        VertexOptions("id")));
    db.Flush();
    UT_EXPECT_EQ(db.AddVertexIndex("vertex", "id", lgraph::IndexType::NonuniqueIndex), false);
    UT_EXPECT_ANY_THROW(db.DeleteVertexIndex("vertex", "id"));
    UT_EXPECT_EQ(db.AddVertexIndex("vertex", "type", lgraph::IndexType::NonuniqueIndex), true);
    UT_EXPECT_EQ(db.DeleteVertexIndex("vertex", "type"), true);
    UT_EXPECT_ANY_THROW(db.DeleteVertexIndex("vertex", "id"));
    UT_EXPECT_EQ(db.AddVertexIndex("vertex", "id", lgraph::IndexType::NonuniqueIndex), false);
    UT_EXPECT_EQ(db.AddVertexIndex("vertex", "content", lgraph::IndexType::NonuniqueIndex), true);
    UT_EXPECT_ANY_THROW(db.AddVertexIndex("vertex_err", "content",
                                          lgraph::IndexType::NonuniqueIndex));
    UT_EXPECT_ANY_THROW(db.AddVertexIndex("vertex", "content_err",
                                          lgraph::IndexType::NonuniqueIndex));
    UT_EXPECT_ANY_THROW(db.DeleteVertexIndex("vertex", "content_err"));
    UT_EXPECT_EQ(db.IsVertexIndexed("vertex", "id"), true);
    UT_EXPECT_ANY_THROW(UT_EXPECT_EQ(db.IsVertexIndexed("vertex", "ssid"), true));

    // db.WaitTillIndexReady("vertex", "id");
    UT_EXPECT_TRUE(db.AddEdgeLabel(
        "edge", std::vector<FieldSpec>({{"type", STRING, false}, {"weight", STRING, false}}), {}));
    UT_EXPECT_TRUE(!db.AddEdgeLabel(
        "edge", std::vector<FieldSpec>({{"type", STRING, false}, {"height", STRING, false}}), {}));
    UT_EXPECT_TRUE(db.AddEdgeLabel(
        "edge_B", std::vector<FieldSpec>({{"type", STRING, false},
                                          {"height", STRING, false}}), {}));

    lgraph_api::Transaction txn_write = db.CreateWriteTxn();
    size_t vlid = txn_write.GetVertexLabelId("vertex");
    size_t elid = txn_write.GetEdgeLabelId("edge");
    size_t v_id_fid = txn_write.GetVertexFieldId(vlid, "id");
    size_t v_type_fid = txn_write.GetVertexFieldId(vlid, "type");
    txn_write.GetVertexFieldId(vlid, "content");
    size_t e_type_fid = txn_write.GetEdgeFieldId(elid, "type");
    size_t e_weight_fid = txn_write.GetEdgeFieldId(elid, "weight");
    // construct condition of database
    std::string vertex_label = "vertex";
    std::vector<std::string> vertex_feild_name = {"id", "type", "content"};
    std::vector<std::string> vertex_feild_nul = {};
    std::vector<std::string> vertex_values_err = {"id_1", "a8", "l_content"};
    std::vector<std::string> vertex_values = {"id_1", "8", "content"};

    // illegal type, must be INT8
    UT_EXPECT_ANY_THROW(txn_write.AddVertex(vertex_label, vertex_feild_name, vertex_values_err));
    // id and type must not be null
    UT_EXPECT_ANY_THROW(txn_write.AddVertex(vertex_label, vertex_feild_nul, vertex_values));
    // add 2 vertexs and 1 edge
    auto ver_id_1 = txn_write.AddVertex(vertex_label, vertex_feild_name, vertex_values);
    UT_EXPECT_EQ(ver_id_1, 0);
    std::vector<std::string> vertex_value2 = {"id_2", "8", "content"};
    std::vector<std::string> vertex_value3 = {"id_3", "8", "content"};
    std::vector<std::string> vertex_value4 = {"id_4", "8", "content"};
    PathTraversal path_traver_wrt(db, txn_write, true);
    auto ver_id_2 = txn_write.AddVertex(vertex_label, vertex_feild_name, vertex_value2);
    UT_EXPECT_EQ(ver_id_2, 1);
    auto ver_id_3 = txn_write.AddVertex(vertex_label, vertex_feild_name, vertex_value3);
    UT_EXPECT_EQ(ver_id_3, 2);
    auto ver_id_4 = txn_write.AddVertex(vertex_label, vertex_feild_name, vertex_value4);
    UT_EXPECT_EQ(ver_id_4, 3);
    std::vector<std::string> vertex_field = {"id", "type", "content"};
    std::vector<FieldData> vertex_field_value = {FieldData("if"), FieldData((int64_t)10),
                                                 FieldData("233")};
    auto ver_id_5 = txn_write.AddVertex(vertex_label, vertex_field, vertex_field_value);
    UT_EXPECT_EQ(ver_id_5, 4);

    PathTraversal path_traver_test(db, txn_write, true, 1ul << 22);
    UT_EXPECT_EQ(path_traver_test.GetFrontier().Capacity(), 1ul << 22);

    UT_LOG() << "now add five vertexes! test is ok!";
    std::vector<std::string> edge_name = {"type", "weight"};
    std::vector<std::string> edge_name_B = {"type", "height"};
    std::vector<std::string> edge_value = {"8", "l_content"};
    std::vector<std::string> edge_value_2 = {"9", "2_content"};
    FieldData field_1("come");
    FieldData field_2("24.5");
    std::vector<FieldData> edge_value_fiel = {field_1, field_2};
    // test iterrator size is 0
    path_traver_wrt.ExpandOutEdges(
        [&](OutEdgeIterator &oeit, Path &path, IteratorHelper &ith) {
            oeit.GetSrc();
            oeit.GetLabelId();
            oeit.GetField("edge");
            oeit.GetAllFields();
            oeit.GetField(0);
            return true;
        },
        [&](VertexIterator &oeit, Path &path, IteratorHelper &ith) { return true; });

    path_traver_wrt.ExpandEdges(
        [&](OutEdgeIterator &oeit, Path &path, IteratorHelper &ith) { return true; },
        [&](InEdgeIterator &oeit, Path &path, IteratorHelper &ith) {
            oeit.Delete();
            return true;
        },
        [&](VertexIterator &oeit, Path &path, IteratorHelper &ith) { return true; },
        [&](VertexIterator &oeit, Path &path, IteratorHelper &ith) { return true; });

    EdgeId e_id = txn_write.AddEdge(ver_id_1, ver_id_2, "edge", edge_name, edge_value).eid;
    UT_LOG() << "add 1 edge! eid:" << e_id;
    UT_EXPECT_EQ(e_id, 0);
    txn_write.AddEdge(ver_id_1, ver_id_2, "edge", edge_name, edge_value);
    auto status_get = txn_write.UpsertEdge(ver_id_1, ver_id_2, "edge", edge_name, edge_value_2);
    UT_EXPECT_EQ(status_get, false);
    e_id = txn_write.AddEdge(ver_id_1, ver_id_2, "edge", edge_name, edge_value_2).eid;
    UT_LOG() << "add 1 edge! eid:" << e_id;
    UT_EXPECT_EQ(e_id, 2);

    e_id = txn_write.AddEdge(ver_id_2, ver_id_3, "edge_B", edge_name_B, edge_value_2).eid;
    UT_LOG() << "add 1 edge_B! eid:" << e_id;
    UT_EXPECT_EQ(e_id, 0);
    status_get = txn_write.UpsertEdge(ver_id_2, ver_id_3, "edge_B", edge_name_B, edge_value_2);
    UT_EXPECT_EQ(status_get, false);

    EdgeId e_id_1 = txn_write.AddEdge(ver_id_2, ver_id_1, "edge", edge_name, edge_value_fiel).eid;

    auto p_get_IndexItrator = txn_write.GetVertexByUniqueIndex(vertex_label, "id", "id_2");
    UT_EXPECT_EQ(p_get_IndexItrator.GetLabel(), "vertex");
    UT_EXPECT_ANY_THROW(txn_write.GetVertexByUniqueIndex(vertex_label, "id", "id_5"));
    txn_write.UpsertEdge(ver_id_2, ver_id_1, "edge", edge_name, edge_value_fiel);
    txn_write.UpsertEdge(ver_id_2, ver_id_1, elid, std::vector<size_t>{e_type_fid, e_weight_fid},
                         edge_value_fiel);
    txn_write.Commit();

    txn_write = db.CreateWriteTxn();
    txn_write.GetVertexByUniqueIndex("vertex", "id", "id_2");
    txn_write.GetVertexByUniqueIndex("vertex", "id", FieldData("id_2"));

    txn_write.GetVertexByUniqueIndex(0, v_id_fid, FieldData("id_2"));
    status_get = txn_write.IsVertexIndexed("vertex", "id");
    UT_EXPECT_EQ(status_get, true);

    status_get = txn_write.IsReadOnly();
    UT_EXPECT_EQ(status_get, false);

    FieldData fail, fail2;
    // index not created for vertex:type
    UT_EXPECT_ANY_THROW(
        txn_write.GetVertexIndexIterator("vertex", "type", FieldData(), FieldData()));
    UT_EXPECT_ANY_THROW(txn_write.GetVertexIndexIterator(0, v_type_fid, FieldData(), FieldData()));
    lgraph_api::VertexIndexIterator in_it =
        txn_write.GetVertexIndexIterator("vertex", "id", FieldData(), FieldData());
    UT_EXPECT_TRUE(in_it.IsValid());
    UT_EXPECT_EQ(in_it.GetIndexValue().AsString(), "id_1");

    auto e_id_weight = txn_write.GetEdgeFieldId(e_id, "weight");
    UT_EXPECT_EQ(e_id_weight, 1);
    // no such field
    UT_EXPECT_ANY_THROW(txn_write.GetEdgeFieldId(e_id, "height"));

    auto pv_write = FindVertices(
        db, txn_write,
        [&](VertexIterator &vit) {
            if (!vit.GetField("content").is_null() && vit.GetField("id").string() == "")
                return false;
            auto ieit = vit.GetInEdgeIterator();
            if (ieit.IsValid()) return false;
            auto oeit = vit.GetOutEdgeIterator();
            return !oeit.IsValid();
        },
        true);
    FieldData field_3("id_2");
    VertexIterator vit = txn_write.GetVertexIterator();
    std::vector<FieldData> vertex_value_data = {FieldData("id_2"), FieldData((int64_t)8),
                                                FieldData("content")};
    //    vit.RecreateVertex(vertex_label, vertex_feild_name, vertex_value_data);
    txn_write.GetVertexByUniqueIndex(vertex_label, "id", field_3);

    FrontierTraversal front_traver_write(db, txn_write, 1);

    FrontierTraversal front_traver_test(db, txn_write, 1 , 1ul << 20);
    UT_EXPECT_EQ(front_traver_test.GetFrontier().Capacity(), 1ul << 20);

    front_traver_write.SetFrontier([&](VertexIterator &vit) {
        UT_LOG() << "read txn set write vertext iterator";
        return true;
    });
    // add 256 vertexs to transaction(if setfrontier has happened，the vector will be reset)
    path_traver_wrt.SetFrontier(pv_write);
    UT_EXPECT_EQ(pv_write.Size(), path_traver_wrt.GetFrontier().Size());
    path_traver_wrt.SetFrontier(0);
    UT_EXPECT_EQ(path_traver_wrt.GetFrontier().Size(), 1);
    path_traver_wrt.SetFrontier([&](VertexIterator &vit) { return true; });
    path_traver_wrt.ExpandInEdges(NULL, NULL);
    path_traver_wrt.ExpandInEdges(
        [&](InEdgeIterator &oeit, Path &path, IteratorHelper &ith) { return true; },
        [&](VertexIterator &oeit, Path &path, IteratorHelper &ith) { return true; });

    path_traver_wrt.ExpandEdges(
        [&](OutEdgeIterator &oeit, Path &path, IteratorHelper &ith) { return true; },
        [&](InEdgeIterator &oeit, Path &path, IteratorHelper &ith) { return true; },
        [&](VertexIterator &oeit, Path &path, IteratorHelper &ith) { return true; },
        [&](VertexIterator &oeit, Path &path, IteratorHelper &ith) { return true; });
    path_traver_wrt.ExpandOutEdges(
        [&](OutEdgeIterator &oeit, Path &path, IteratorHelper &ith) { return true; },
        [&](VertexIterator &oeit, Path &path, IteratorHelper &ith) { return true; });

    auto ver_id_tmp = ver_id_2;
    for (int i = 0; i < 1019; i++) {
        std::vector<std::string> v = {std::to_string(i), "8", "content"};
        auto ver_id_get = txn_write.AddVertex(vertex_label, vertex_feild_name, v);
        txn_write.AddEdge(ver_id_tmp, ver_id_get, "edge", edge_name, edge_value);
        ver_id_tmp = ver_id_get;
    }

    path_traver_wrt.SetFrontier([&](VertexIterator &vit) { return true; });
    path_traver_wrt.ExpandEdges(
        [&](OutEdgeIterator &oeit, Path &path, IteratorHelper &ith) { return true; },
        [&](InEdgeIterator &oeit, Path &path, IteratorHelper &ith) { return true; },
        [&](VertexIterator &oeit, Path &path, IteratorHelper &ith) { return true; },
        [&](VertexIterator &oeit, Path &path, IteratorHelper &ith) { return true; });
    FrontierTraversal front_traver_write_3(db, txn_write, 3);
    front_traver_write_3.SetFrontier(1);
    front_traver_write_3.SetFrontier(pv_write[0]);
    front_traver_write_3.SetFrontier([](VertexIterator &vit) { return true; });
    // txn_write = db.CreateWriteTxn();
    lgraph_api::traversal::Vertex vertex(ver_id_2);
    Edge edge(ver_id_1, elid, 0, ver_id_2, e_id, true);
    Edge edge_2(ver_id_1, elid, 0, ver_id_2, e_id, false);
    Edge edge_3(ver_id_2, elid, 0, ver_id_3, e_id, true);
    Edge edge_4(ver_id_3, elid, 0, ver_id_4, e_id, false);
    IteratorHelper iter_helper_2(txn_write);
    VertexIterator ver_it_2 = iter_helper_2.Cast(vertex);
    std::string name("id");
    FieldData field("wangjun");
    std::vector<std::string> name_vec = {name, name};
    std::vector<FieldData> field_vec = {field, field};
    ver_it_2.SetField(name, field);
    UT_EXPECT_EQ(ver_it_2.GetField(v_id_fid).string(), "wangjun");
    FieldData field_a((int64_t)10);
    ver_it_2.SetField(v_type_fid, field_a);
    UT_EXPECT_EQ(ver_it_2.GetField(v_type_fid).integer(), 10);
    UT_EXPECT_EQ(ver_it_2.GetId(), 1);

    auto ver_get = ver_it_2.GetAllFields();
    UT_EXPECT_EQ(ver_get["id"].ToString(), "wangjun");
    UT_EXPECT_EQ(ver_get["type"].ToString(), "10");
    field_a = ver_it_2.GetField(v_type_fid);
    UT_EXPECT_EQ(field_a.ToString(), "10");
    ver_it_2.GetField("id");
    ver_it_2.SetFields(name_vec, field_vec);
    //    ver_it_2.RecreateVertex(vertex_label, vertex_feild_name, vertex_value2);
    size_t str = ver_it_2.GetNumInEdges(ver_id_1);
    UT_EXPECT_EQ(str, ver_id_1);
    str = ver_it_2.GetNumOutEdges(2);
    UT_EXPECT_EQ(str, 2);
    str = ver_it_2.GetNumOutEdges(4);
    UT_EXPECT_EQ(str, 3);

    OutEdgeIterator oit_2 = iter_helper_2.Cast(edge);

    UT_EXPECT_EQ(oit_2.Goto(EdgeUid(ver_id_1, ver_id_2, elid, 0, e_id)), true);
    UT_EXPECT_EQ(oit_2.GetSrc(), ver_id_1);
    UT_EXPECT_EQ(oit_2.GetLabelId(), elid);
    field_a = oit_2.GetField(e_type_fid);
    UT_EXPECT_EQ(field_a.ToString(), "9");
    UT_EXPECT_EQ(oit_2.GetField("type").ToString(), "9");
    oit_2.GetAllFields();
    oit_2.SetField(e_type_fid, field);
    oit_2.SetField({"type"}, field);
    std::vector<FieldData> ed_vec = {FieldData("type"), FieldData("weight")};
    std::vector<size_t> id_vec = {e_type_fid, e_weight_fid};
    oit_2.SetFields(id_vec, ed_vec);
    UT_LOG() << "edge " << oit_2.ToString();
    std::vector<std::string> field_names = {"type", "weight"};
    std::vector<std::string> field_value = {"type", "weight"};
    std::vector<FieldData> field_eges = {FieldData("type-1"), FieldData("weight-1")};
    oit_2.SetFields(field_names, field_eges);
    field_a = oit_2.GetField(e_type_fid);
    UT_EXPECT_EQ(field_a.ToString(), "type-1");
    field_a = oit_2.GetField(e_weight_fid);
    UT_EXPECT_EQ(field_a.ToString(), "weight-1");

    // test InEdgeIterator

    InEdgeIterator in_ed = ver_it_2.GetInEdgeIterator(EdgeUid(0, ver_id_1, 0, 0, e_id), false);
    status_get = in_ed.Goto(EdgeUid(ver_id_2, ver_id_1, 0, 0, e_id_1));
    UT_EXPECT_EQ(status_get, true);
    size_t label_id = in_ed.GetLabelId();
    UT_EXPECT_EQ(label_id, 0);
    field_a = in_ed.GetField(e_type_fid);
    UT_EXPECT_EQ(field_a.ToString(), "come");
    field_a = in_ed.GetField("type");
    UT_EXPECT_EQ(field_a.ToString(), "come");
    in_ed.GetAllFields();
    in_ed.SetField("type", FieldData("type"));
    in_ed.SetField(e_type_fid, FieldData("type"));
    in_ed.SetFields(id_vec, field_vec);
    in_ed.SetFields(field_names, field_eges);
    in_ed.SetFields(field_names, field_value);
    UT_LOG() << "In edge iterator:" << in_ed.ToString();
    UT_LOG() << "compare InEdgeIterator with OutEdgeIterator:" << (oit_2 == in_ed);
    UT_LOG() << "compare outEdgeIterator with inEdgeIterator:" << (in_ed == oit_2);
    UT_LOG() << "compare outEdgeIterator with outEdgeIterator:" << (oit_2 == oit_2);
    UT_LOG() << "compare inEdgeIterator with inEdgeIterator:" << (in_ed == in_ed);
    UT_LOG() << "compare n InEdgeIterator with OutEdgeIterator:" << (oit_2 != in_ed);
    UT_LOG() << "compare n outEdgeIterator with inEdgeIterator:" << (in_ed != oit_2);
    UT_LOG() << "compare n inEdgeIterator with inEdgeIterator:" << (in_ed != in_ed);
    UT_LOG() << "compare n outEdgeIterator with outEdgeIterator:" << (oit_2 != oit_2);
    txn_write.GetNumVertexLabels();
    txn_write.GetNumEdgeLabels();
    txn_write.Commit();

    // test transaction of read
    lgraph_api::Transaction txn_read = db.CreateReadTxn();
    ASSERT(txn_read.IsReadOnly() == true);
    // txn_read.GetNumLabels(true);
    auto pv = FindVertices(
        db, txn_read,
        [&](VertexIterator &vit) {
            // if (!vit.GetField("content").is_null() && vit.GetField("id").string() == "")
            // return false;
            return true;
        },
        true);
    FrontierTraversal front_traver(db, txn_read, 3);
    front_traver.SetFrontier([&](VertexIterator &vit) { return true; });
    UT_LOG() << "read vertex iterator whether it is valid："
             << txn_read.GetVertexIterator().GetInEdgeIterator().IsValid();
    UT_EXPECT_EQ((front_traver.GetFrontier()).Size(), 1024);
    // context: size<256
    front_traver.ExpandOutEdges([](OutEdgeIterator &oue) { return true; },
                                [](VertexIterator &vit) { return true; });
    front_traver.ExpandOutEdges([](OutEdgeIterator &oue) { return true; }, NULL);
    front_traver.ExpandOutEdges([](OutEdgeIterator &oue) { return true; }, NULL);
    front_traver.SetFrontier([&](VertexIterator &vit) { return true; });
    front_traver.ExpandInEdges(NULL, NULL);
    front_traver.ExpandInEdges([](InEdgeIterator &inv) { return true; }, NULL);
    front_traver.ExpandInEdges([](InEdgeIterator &inv) { return true; },
                               [](VertexIterator &vit) { return true; });
    front_traver.SetFrontier([&](VertexIterator &vit) { return true; });
    front_traver.ExpandInEdges(NULL, NULL);
    front_traver.SetFrontier([&](VertexIterator &vit) { return true; });
    front_traver.ExpandEdges(NULL, NULL, NULL, NULL);
    // curr_frontier_.Size()>256
    front_traver.ExpandEdges([&](OutEdgeIterator &oeit) { return true; }, NULL,
                             [&](VertexIterator &oeit) { return true; },
                             [&](VertexIterator &oeit) { return true; });

    front_traver.SetFrontier(pv);
    front_traver.SetFrontier(1);
    UT_EXPECT_EQ((front_traver.GetFrontier()).Size(), 1);
    UT_LOG() << "reset frontier,the size is ：" << (front_traver.GetFrontier()).Size();

    front_traver.ExpandOutEdges([](OutEdgeIterator &oue) { return true; },
                                [](VertexIterator &vit) { return true; });
    front_traver.ExpandInEdges([](InEdgeIterator &inv) { return true; },
                               [](VertexIterator &vit) { return true; });
    front_traver.ExpandEdges(
        [&](OutEdgeIterator &oeit) { return true; }, [&](InEdgeIterator &ieit) { return true; },
        [&](VertexIterator &oeit) { return true; }, [&](VertexIterator &oeit) { return true; });

    // test PathTraversal
    PathTraversal path_traver(db, txn_read, true);
    path_traver.GetFrontier();
    // rest frontier
    path_traver.ExpandOutEdges(
        [&](OutEdgeIterator &oeit, Path &path, IteratorHelper &ith) { return true; },
        [&](VertexIterator &oeit, Path &path, IteratorHelper &ith) { return true; });
    UT_EXPECT_EQ(path_traver.GetFrontier().Size(), 0);
    UT_LOG() << "check frontier size after reset frontier:" << (path_traver.GetFrontier()).Size();
    path_traver.SetFrontier([](VertexIterator &vit) { return true; });
    path_traver.ExpandInEdges(
        [&](InEdgeIterator &oeit, Path &path, IteratorHelper &ith) { return true; },
        [&](VertexIterator &oeit, Path &path, IteratorHelper &ith) { return true; });

    path_traver.ExpandEdges(
        [&](OutEdgeIterator &oeit, Path &path, IteratorHelper &ith) { return true; },
        [&](InEdgeIterator &oeit, Path &path, IteratorHelper &ith) { return true; },
        [&](VertexIterator &oeit, Path &path, IteratorHelper &ith) { return true; },
        [&](VertexIterator &oeit, Path &path, IteratorHelper &ith) { return true; });
    path_traver.ExpandOutEdges(
        [&](OutEdgeIterator &oeit, Path &path, IteratorHelper &ith) { return true; },
        [&](VertexIterator &oeit, Path &path, IteratorHelper &ith) { return true; });
    path_traver.Reset();
    path_traver.SetFrontier(pv);
    path_traver.SetFrontier(pv[0]);
    UT_EXPECT_EQ(path_traver.GetFrontier().Size(), 1);

    IteratorHelper iter_helper(txn_read);
    UT_EXPECT_EQ(edge.GetEdgeId(), 0);
    UT_EXPECT_EQ(edge.GetSrcVertex().GetId(), ver_id_1);
    UT_EXPECT_EQ(edge.GetDstVertex().GetId(), ver_id_2);
    UT_EXPECT_EQ(edge.IsForward(), true);
    UT_EXPECT_EQ(edge.GetSrcVertex().GetId(), ver_id_1);
    UT_EXPECT_EQ(edge.GetDstVertex().GetId(), ver_id_2);
    if (edge == edge) {
        UT_LOG() << "compare edge ,both edges‘s have same id";
    }
    if (edge != edge) {
    } else {
        UT_LOG() << "compare the same edge ,excute else branch";
    }

    if (edge.GetDstVertex() != edge.GetSrcVertex()) {
        UT_LOG() << "compare edge ,both vertex‘s have same id";
    }

    if (vertex == vertex) {
        UT_LOG() << "YES";
    }
    // fill the path start vertex--ver_id_2
    Path path_1(vertex);
    Path path_2(vertex);
    UT_EXPECT_ANY_THROW(path_2.Append(edge_2));
    UT_EXPECT_ANY_THROW(path_2.GetLastEdge());
    UT_LOG() << "path length:" << path_2.Length();
    // path1 add 2 edges,ids'back de vertex id is the edge start(add)
    path_2.Append(edge_3);
    path_2.Append(edge_4);
    UT_EXPECT_EQ(path_2.GetStartVertex().GetId(), vertex.GetId());
    UT_EXPECT_EQ(path_2.GetEndVertex().GetId(), ver_id_4);
    UT_EXPECT_EQ(path_2.GetLastEdge().GetEdgeId(), edge_4.GetEdgeId());
    UT_EXPECT_EQ(path_2.Length(), 2);
    UT_LOG() << "after add 2 edges,path length:" << path_2.Length();
    // path1 add 2 edges,ids'back de vertex id is the edge start(add)
    UT_EXPECT_EQ(path_2.GetNthEdge(1).GetEdgeId(), edge_4.GetEdgeId());

    UT_EXPECT_ANY_THROW(path_2.GetNthEdge(3));
    path_2.GetNthVertex(1);
    UT_EXPECT_ANY_THROW(path_2.GetNthVertex(4));
    path_1 = path_2;
    vertex.GetId();
    VertexIterator vit_1 = iter_helper.Cast(vertex);
    UT_LOG() << "vector id :" << vit_1.GetLabelId();

    // vit_1.GetAllFields();
    OutEdgeIterator oit_1 = iter_helper.Cast(edge);
    if (oit_1 == oit_1) {
        UT_LOG() << "compare == oit_1 :"
                 << "true";
    }

    iter_helper.Cast(edge_2);
    // test  ParallelBitset class
    UT_EXPECT_NO_THROW(ParallelBitset para_bit(0));
    ParallelBitset para_bit_1(5);

    ParallelBitset para_bit(0x1000001);
    para_bit.Fill();
    UT_EXPECT_EQ(para_bit.Has(0), true);
    para_bit.Swap(para_bit);
    bool lock = false;
    VertexLockGuard ver_lock(&lock);
    UT_LOG() << "lock";
    ver_lock.~VertexLockGuard();
    UT_LOG() << "~lock";
    // clear enviroment
    ParallelVector<size_t> para_vec(10, 0);
    front_traver.SetFrontier(para_vec);
    front_traver.SetFrontier(pv[0]);
    front_traver.Reset();
    front_traver.ResetVisited();
    fma_common::file_system::RemoveDir("./testtmp");
}
