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

#include "gtest/gtest.h"
#include "./ut_utils.h"

#include "db/galaxy.h"
#include "lgraph/lgraph.h"
#include "./test_tools.h"

#include "./graph_factory.h"
#include "./antlr4-runtime.h"

#include "cypher/execution_plan/execution_plan.h"
#include "cypher/execution_plan/scheduler.h"

#include "cypher/parser/generated/LcypherLexer.h"
#include "cypher/parser/generated/LcypherParser.h"
#include "cypher/parser/cypher_base_visitor.h"
#include "cypher/parser/cypher_error_listener.h"

class TestEdgeConstraint : public TuGraphTest {};
using namespace lgraph_api;
using namespace parser;
using namespace antlr4;
TEST_F(TestEdgeConstraint, lgraph_api) {
    const std::string& dir = "./testdb";
    lgraph::AutoCleanDir cleaner(dir);
    Galaxy galaxy(dir, false, true);
    galaxy.SetCurrentUser(lgraph::_detail::DEFAULT_ADMIN_NAME,
                          lgraph::_detail::DEFAULT_ADMIN_PASS);
    auto db = galaxy.OpenGraph(lgraph::_detail::DEFAULT_GRAPH_DB_NAME);
    UT_EXPECT_TRUE(db.AddVertexLabel("v1",
                         {{"id", FieldType::STRING, false}},
                         lgraph::VertexOptions("id")));
    UT_EXPECT_TRUE(db.AddVertexLabel("v2",
                         {{"id", FieldType::STRING, false}},
                         lgraph::VertexOptions("id")));
    UT_EXPECT_TRUE(db.AddVertexLabel("v3",
                         {{"id", FieldType::STRING, false}},
                         lgraph::VertexOptions("id")));
    UT_EXPECT_TRUE(db.AddEdgeLabel("v1_v2",
                       {{"weight", FieldType::FLOAT, false}},
                       lgraph::EdgeOptions(lgraph::EdgeConstraints{{"v1","v2"}}))); // NOLINT

    std::string err_msg = "Does not meet the edge constraints";
    Transaction txn = db.CreateWriteTxn();
    auto v1 = txn.AddVertex("v1", {"id"}, {"1"});
    auto v2 = txn.AddVertex("v2", {"id"}, {"2"});
    auto v3 = txn.AddVertex("v3", {"id"}, {"3"});
    txn.AddEdge(v1, v2, "v1_v2", {"weight"}, {"1"});
    txn.Commit();
    {
        Transaction tmp = db.CreateWriteTxn();
        UT_EXPECT_THROW_MSG(tmp.AddEdge(v1, v3, "v1_v2", {"weight"}, {"1"}), err_msg);
    }
    {
        Transaction tmp = db.CreateWriteTxn();
        UT_EXPECT_THROW_MSG(tmp.AddEdge(v3, v1, "v1_v2", {"weight"}, {"1"}), err_msg);
    }
    {
        Transaction tmp = db.CreateWriteTxn();
        UT_EXPECT_THROW_MSG(tmp.AddEdge(v2, v1, "v1_v2", {"weight"}, {"1"}), err_msg);
    }
    {
        Transaction tmp = db.CreateWriteTxn();
        UT_EXPECT_THROW_MSG(tmp.AddEdge(v2, v3, "v1_v2", {"weight"}, {"1"}), err_msg);
    }
    {
        Transaction tmp = db.CreateWriteTxn();
        UT_EXPECT_THROW_MSG(tmp.AddEdge(v3, v2, "v1_v2", {"weight"}, {"1"}), err_msg);
    }
    UT_EXPECT_TRUE(db.DeleteVertexLabel("v1"));
    {
        auto tmp = db.CreateWriteTxn();
        UT_EXPECT_THROW_MSG(tmp.AddEdge(v2, v3, "v1_v2", {"weight"}, {"1"}), err_msg);
    }
    {
        auto tmp = db.CreateWriteTxn();
        UT_EXPECT_THROW_MSG(tmp.AddEdge(v3, v2, "v1_v2", {"weight"}, {"1"}), err_msg);
    }
    UT_EXPECT_TRUE(db.AddVertexLabel("v1",
                                     {{"id", FieldType::STRING, false}},
                                     lgraph::VertexOptions("id")));
    txn = db.CreateWriteTxn();
    auto v1_1 = txn.AddVertex("v1", {"id"}, {"1"});
    txn.AddEdge(v1_1, v2, "v1_v2", {"weight"}, {"1"});
    txn.Commit();
    UT_EXPECT_TRUE(db.AlterVertexLabelAddFields("v1",
                                 {{"name",FieldType::STRING,true}}, // NOLINT
                                 {FieldData("")}));
    txn = db.CreateWriteTxn();
    txn.AddEdge(v1_1, v2, "v1_v2", {"weight"}, {"1"});
    txn.Commit();
}

static void eval_scripts(cypher::RTContext *ctx, const std::vector<std::string> &scripts) {
    for (int i = 0; i < (int)scripts.size(); i++) {
        auto s = scripts[i];
        UT_LOG() << s;
        ANTLRInputStream input(s);
        LcypherLexer lexer(&input);
        CommonTokenStream tokens(&lexer);
        LcypherParser parser(&tokens);
        parser.addErrorListener(&CypherErrorListener::INSTANCE);
        CypherBaseVisitor visitor(ctx, parser.oC_Cypher());
        cypher::ExecutionPlan execution_plan;
        execution_plan.PreValidate(ctx, visitor.GetNodeProperty(), visitor.GetRelProperty());
        execution_plan.Build(visitor.GetQuery(), visitor.CommandType(), ctx);
        execution_plan.Validate(ctx);
        execution_plan.DumpGraph();
        execution_plan.DumpPlan(0, false);
        execution_plan.Execute(ctx);
        UT_LOG() << "Result:\n" << ctx->result_->Dump(false);
    }
}

TEST_F(TestEdgeConstraint, cypher) {
    const std::string& dir = "./testdb";
    lgraph::AutoCleanDir cleaner(dir);
    lgraph::Galaxy::Config gconf;
    gconf.dir = dir;
    lgraph::Galaxy galaxy(gconf, true, nullptr);
    cypher::RTContext db(
        nullptr,
        &galaxy,
        lgraph::_detail::DEFAULT_ADMIN_NAME,
        "default");

    eval_scripts(&db, {"CALL db.createVertexLabel('v1', 'id', 'id', 'STRING', false)"});
    eval_scripts(&db, {"CALL db.createVertexLabel('v2', 'id', 'id', 'STRING', false)"});
    eval_scripts(&db, {"CALL db.createVertexLabel('v3', 'id', 'id', 'STRING', false)"});
    UT_EXPECT_THROW_MSG(
        eval_scripts(&db, {R"(CALL db.createEdgeLabel('v1_v2', '[["v4", "v2"]]', 'weight', 'FLOAT', false))"}),
        "No such vertex label");
    eval_scripts(&db, {R"(CALL db.createEdgeLabel('v1_v2', '[["v1", "v2"]]', 'weight', 'FLOAT', false))"});
    eval_scripts(&db, {"CREATE (:v1 {id:'1'}), (:v2 {id:'2'}), (:v3 {id:'3'})"});
    eval_scripts(&db, {"MATCH (a:v1 {id:'1'}),(b:v2 {id:'2'}) CREATE (a)-[:v1_v2{weight:1}]->(b)"});
    std::string constraints_err_msg = "Does not meet the edge constraints";
    std::string cmd = "MATCH (a:v1 {id:'1'}),(b:v3 {id:'3'}) CREATE (a)-[:v1_v2{weight:1}]->(b)";
    UT_EXPECT_THROW_MSG(eval_scripts(&db, {cmd}), constraints_err_msg);
    cmd = "MATCH (a:v2 {id:'2'}),(b:v1 {id:'1'}) CREATE (a)-[:v1_v2{weight:1}]->(b)";
    UT_EXPECT_THROW_MSG(eval_scripts(&db, {cmd}), constraints_err_msg);
    cmd = "MATCH (a:v2 {id:'2'}),(b:v3 {id:'3'}) CREATE (a)-[:v1_v2{weight:1}]->(b)";
    UT_EXPECT_THROW_MSG(eval_scripts(&db, {cmd}), constraints_err_msg);

    eval_scripts(&db, {R"(CALL db.addEdgeConstraints('v1_v2', '[["v2", "v3"]]'))"});
    eval_scripts(&db, {"MATCH (a:v2 {id:'2'}),(b:v3 {id:'3'}) CREATE (a)-[:v1_v2{weight:1}]->(b)"});

    eval_scripts(&db, {"CALL db.clearEdgeConstraints('v1_v2')"});
    eval_scripts(&db, {"MATCH (a:v1 {id:'1'}),(b:v3 {id:'3'}) CREATE (a)-[:v1_v2{weight:1}]->(b)"});

    // test exception
    UT_EXPECT_THROW_MSG(
        eval_scripts(&db, {R"(CALL db.createEdgeLabel('v2_v3', '[["v4", "v2"],["v4", "v2"]]', 'weight', 'FLOAT', false))"}),
        "Duplicate constraints");
    eval_scripts(&db, {R"(CALL db.createEdgeLabel('v2_v3', '[["v2", "v3"]]', 'weight', 'FLOAT', false))"});
    UT_EXPECT_THROW_MSG(
        eval_scripts(&db, {R"(CALL db.addEdgeConstraints('v2_v3', '[["v2", "v3"]]'))"}),
        "already exist");
    UT_EXPECT_THROW_MSG(
        eval_scripts(&db, {R"(CALL db.addEdgeConstraints('v2_v3', '[]'))"}),
        "Constraints are empty");
    UT_EXPECT_THROW_MSG(
        eval_scripts(&db, {R"(CALL db.addEdgeConstraints('v2_v3', '["ddddd"]'))"}),
        "json.exception.type_error");
    UT_EXPECT_THROW_MSG(
        eval_scripts(&db, {R"(CALL db.addEdgeConstraints('v2_v3', '[["v4","v2"]]'))"}),
        "No such vertex label");
}
