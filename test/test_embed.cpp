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
#include "fma-common/utils.h"

#include "db/galaxy.h"
#include "lgraph/lgraph.h"

#include "gtest/gtest.h"
#include "./ut_utils.h"

class TestEmbed : public TuGraphTest {};

bool Process(lgraph_api::GraphDB& g, const std::string& input, std::string& output) {
    UT_LOG() << "estimated number of vertices: " << g.EstimateNumVertices();
    return true;
}

bool Process2(lgraph_api::GraphDB& g, const std::string& input, std::string& output) {
    fma_common::SleepS(1);
    UT_LOG() << "estimated number of vertices: " << g.EstimateNumVertices();
    return true;
}

TEST_F(TestEmbed, Embed) {
    fma_common::file_system::RemoveDir("./mydb");

    UT_LOG() << "testing CreateGraph and DeleteGraph";
    {
        lgraph_api::Galaxy db("./mydb",
                              lgraph::_detail::DEFAULT_ADMIN_NAME,
                              lgraph::_detail::DEFAULT_ADMIN_PASS,
                              false, true);
        auto graphs = db.ListGraphs();
        UT_EXPECT_EQ(graphs.size(), 1);
        UT_EXPECT_TRUE(!db.CreateGraph("default"));  // default db is created automatically
        lgraph_api::GraphDB g = db.OpenGraph("default");
        {
            g.AddVertexLabel("v",
                             std::vector<lgraph_api::FieldSpec>{
                                 {"id", lgraph_api::FieldType::INT64, false},
                                 {"name", lgraph_api::FieldType::STRING, false},
                             },
                             lgraph_api::VertexOptions("id"));
            auto txn = g.CreateWriteTxn();
            UT_EXPECT_EQ(txn.AddVertex("v", std::vector<std::string>{"id", "name"},
                                       std::vector<std::string>{"001", "s001"}),
                         0);
            txn.Commit();
        }
        std::string output;
        Process(g, "", output);
        UT_EXPECT_TRUE(db.CreateGraph("g2"));
        {
            lgraph_api::GraphDB g2 = db.OpenGraph("g2");
            g2.AddVertexLabel(
                "v",
                std::vector<lgraph_api::FieldSpec>{{"id", lgraph_api::FieldType::INT64, false},
                                                   {"name", lgraph_api::FieldType::STRING, false}},
                lgraph_api::VertexOptions("id"));
            auto txn = g2.CreateWriteTxn();
            UT_EXPECT_EQ(txn.AddVertex("v", std::vector<std::string>{"id", "name"},
                                       std::vector<std::string>{"001", "s001"}),
                         0);
            txn.Commit();
            db.DeleteGraph("g2");
            fma_common::SleepS(0.5);
            Process(g2, "", output);
            // now g2 is deleted
            UT_EXPECT_ANY_THROW(db.OpenGraph("g2"));
        }
    }
    const std::string& admin_user = lgraph::_detail::DEFAULT_ADMIN_NAME;
    const std::string& admin_pass = lgraph::_detail::DEFAULT_ADMIN_PASS;
    const std::string& new_user = "new_user";
    const std::string& new_pass = "new_pass";
    const std::string& new_role = "new_role";
    const std::string& dbdir = "./mydb";
    const std::string& default_graph = "default";
    UT_LOG() << "Testing acl";
    {
        lgraph_api::Galaxy db(dbdir, admin_user, admin_pass, false, true);
        UT_EXPECT_TRUE(db.CreateUser(new_user, new_pass));
    }
    { UT_EXPECT_ANY_THROW(lgraph_api::Galaxy db(dbdir, new_user, "wrong_password",
                                                false, true)); }
    {
        lgraph_api::Galaxy db(dbdir, new_user, new_pass, false, true);
        UT_EXPECT_ANY_THROW(db.OpenGraph(default_graph));  // no access
    }
    {
        lgraph_api::Galaxy db(dbdir, admin_user, admin_pass, false, true);
        UT_EXPECT_TRUE(db.CreateRole(new_role, ""));
        UT_EXPECT_TRUE(
            db.SetRoleAccessRights(new_role, {{default_graph, lgraph::AccessLevel::READ}}));
        UT_EXPECT_TRUE(db.SetUserRoles(new_user, {new_role}));
    }
    {
        lgraph_api::Galaxy db(dbdir, new_user, new_pass, false, true);
        lgraph_api::GraphDB graph = db.OpenGraph(default_graph);
        UT_EXPECT_ANY_THROW(graph.DropAllData());
        UT_EXPECT_ANY_THROW(graph.CreateWriteTxn());
        lgraph_api::Transaction txn = graph.CreateReadTxn();
        size_t nv = 0;
        for (auto it = txn.GetVertexIterator(); it.IsValid(); it.Next()) {
            nv++;
            UT_LOG() << it.ToString();
        }
        UT_EXPECT_ANY_THROW(txn.AddVertex("v", std::vector<std::string>{"id", "name"},
                                          std::vector<std::string>{"002", "s002"}));
    }
}
