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

#include "gtest/gtest.h"
#include "client/cpp/restful/rest_client.h"
#include "server/lgraph_server.h"

#include "./graph_factory.h"
#include "./test_tools.h"
#include "./ut_utils.h"

class TestRestClient : public TuGraphTestWithParam<bool> {};

TEST_P(TestRestClient, RestClient) {
    _ParseTests(&_ut_argc, &_ut_argv);
    const std::string& admin_user = lgraph::_detail::DEFAULT_ADMIN_NAME;
    const std::string& admin_pass = lgraph::_detail::DEFAULT_ADMIN_PASS;
    const std::string& admin_role = lgraph::_detail::ADMIN_ROLE;

    const std::string new_user = "new_user";
    const std::string new_pass = "new_pass";
    const std::string new_role = "new_role";
    const std::string new_desc1 = "description for user";
    const std::string new_desc2 = "modified description";

    using namespace fma_common;
    using namespace lgraph;
    bool enable_ssl = true;
    std::string host = "127.0.0.1";
    uint16_t port = 6464;
    std::string db_name = "default";
    std::shared_ptr<lgraph::GlobalConfig> gconfig = std::make_shared<lgraph::GlobalConfig>();
    enable_ssl = GetParam();
    {
        int argc = _ut_argc;
        char** argv = _ut_argv;
        Configuration config;
        config.Add(enable_ssl, "ssl", true).Comment("Enable SSL");
        config.Add(host, "host", true).Comment("Host address");
        config.Add(port, "port", true).Comment("HTTP port");
        config.Add(gconfig->enable_ip_check, "enable_ip_check", true).Comment("Enable IP check.");
        config.ParseAndFinalize(argc, argv);
    }
#ifdef _WIN32
    enable_ssl = false;
#endif

    const std::string key_path = "./key";
    const std::string cert_path = "./cert";
    std::string url = UT_FMT("http://{}:{}", host, port);
    if (enable_ssl) {
        lgraph::WriteCertFiles(cert_path, key_path);
        url = UT_FMT("https://{}:{}", host, port);
    }
    std::string db_dir = "./restdb";
    gconfig->bind_host = host;
    gconfig->http_port = port;
    gconfig->enable_ssl = enable_ssl;
    gconfig->enable_audit_log = true;
    gconfig->db_dir = db_dir;
    gconfig->server_key_file = key_path;
    gconfig->server_cert_file = cert_path;

    auto StartEmptyServer = [&]() {
        std::unique_ptr<LGraphServer> server(new LGraphServer(gconfig));
        server->Start();
        return server;
    };

    auto GetRestClient = [&]() {
        std::unique_ptr<RestClient> client(new RestClient(url, gconfig->server_cert_file));
        return client;
    };

    std::unique_ptr<LGraphServer> server;
    std::unique_ptr<RestClient> client;
    auto Setup = [&](bool login_as_admin = true) {
        client.reset();
        if (server) server->Stop();
        server.reset();
        fma_common::file_system::RemoveDir(gconfig->db_dir);
        server = StartEmptyServer();
        client = GetRestClient();
        for (int retry_times = 5; retry_times > 0; --retry_times) {
            try {
                if (client->Login(admin_user, admin_pass)) break;
            } catch (std::exception& e) {
                UT_LOG() << FMA_FMT("error on connect rest server : {} ", e.what());
            }
            fma_common::SleepS(2);
        }
    };

    DefineTest("DeleteAdminRole") {
        Setup();
        UT_EXPECT_THROW(
            {
                try {
                    client->DeleteRole(admin_role);
                } catch (std::exception& e) {
                    // and this tests that it has the correct message
                    std::string result = e.what();
                    UT_EXPECT_NE(result.find("cannot"), std::string::npos);
                    throw;
                }
            },
            std::exception);
    }

    DefineTest("CreateUser") {
        Setup();
        client->AddUser(new_user, false, new_pass, new_desc1);
        client->DisableUser(new_user);
        client->EnableUser(new_user);
        auto uinfo = client->GetUserInfo(new_user);
        UT_EXPECT_EQ(uinfo.desc, new_desc1);
        UT_EXPECT_TRUE(uinfo.roles == std::set<std::string>({new_user}));
        UT_EXPECT_TRUE(!uinfo.disabled);
    }
    DefineTest("CreateRole") {
        Setup();
        client->CreateRole("test_role", "role for test");
        client->DisableRole("test_role");
        client->EnableRole("test_role");
        client->SetRoleDesc("test_role", "role for set role desc");
        std::map<std::string, lgraph_api::AccessLevel> graph_level;
        graph_level.insert(std::make_pair(db_name, lgraph_api::AccessLevel::FULL));
        client->SetRoleGraphAccess("test_role", graph_level);
        std::map<std::string, FieldData> configs;
        configs.insert(std::make_pair("durable", FieldData(false)));
        client->SetConfig(configs);
        UT_EXPECT_THROW_MSG(
            client->SetMisc(),
            "is_write_op must be set for non-Cypher/non-Gql and non-plugin request.");
        client->GetLabelNum("default");
        client->ListUserLabel("default");
        client->ListUserGraph("admin");
        client->ListGraphs("default");
        client->ListGraphs("");
        client->ListRoles("test_role");
        client->ListRoles("");
    }
    DefineTest("AddLabel") {
        Setup();
        EdgeConstraints ec;
        ec.push_back(std::make_pair("vert", "vert"));
        UT_LOG() << "AddVertexLabel";
        UT_EXPECT_TRUE(client->AddVertexLabel(
            db_name, "vert",
            std::vector<FieldSpec>({FieldSpec("vids", FieldType::STRING, false),
                                    FieldSpec("namez", FieldType::STRING, false)}),
            "vids"));
        UT_LOG() << "AddEdgeLabel";
        UT_EXPECT_TRUE(client->AddEdgeLabel(
            db_name, "edg",
            std::vector<FieldSpec>({FieldSpec("eids", FieldType::STRING, false),
                                    FieldSpec("namee", FieldType::STRING, false)}),
            ec));
        std::vector<std::string> res1;
        std::vector<std::string> res2;
        res1 = client->ListVertexLabels(db_name);
        res2 = client->ListEdgeLabels(db_name);
        UT_EXPECT_EQ(res1.size(), 1);
        UT_EXPECT_EQ(res2.size(), 1);
    }
    DefineTest("AddVertexAndEdge") {
        int64_t v1 = client->AddVertex(db_name, "vert", std::vector<std::string>({"vids", "namez"}),
                                       std::vector<std::string>({"1", "vertex1"}));
        UT_EXPECT_NE(v1, -1);
        UT_EXPECT_EQ(client->AddVertex(db_name, "vert", std::vector<std::string>({"vids", "namez"}),
                                       std::vector<std::string>({"1"})),
                     -1);
        int64_t v2 = client->AddVertex(db_name, "vert", std::vector<std::string>({"vids", "namez"}),
                                       std::vector<std::string>({"2", "vertex2"}));
        UT_EXPECT_NE(v2, -1);
        int64_t v3 = client->AddVertex(db_name, "vert", std::vector<std::string>({"vids", "namez"}),
                                       std::vector<FieldData>({FieldData("2")}));

        UT_EXPECT_EQ(v3, -1);
        auto v4 = client->AddVertexes(db_name, "vert", std::vector<std::string>({"vids", "namez"}),
                                      std::vector<std::vector<FieldData>>({{FieldData("2")}}));
        UT_EXPECT_EQ(v4.size(), 0);
        client->AddEdge(db_name, v1, v2, "edg", std::vector<std::string>({"eids", "namee"}),
                        std::vector<std::string>({"1", "edges"}));
        UT_EXPECT_EQ(
            client
                ->AddEdge(db_name, v1, v2, "edg", std::vector<std::string>({"eids", "namee"}),
                          std::vector<std::string>({"1"}))
                .src,
            0);
        auto e1 =
            client->AddEdge(db_name, v1, v2, "ert", std::vector<std::string>({"vids", "namez"}),
                            std::vector<FieldData>({FieldData("2")}));
        UT_EXPECT_EQ(e1.src, 0);
        UT_EXPECT_THROW(
            client->SetVertexProperty(db_name, 1, std::vector<std::string>({"vids", "namez"}),
                                      std::vector<FieldData>({FieldData("2")})),
            std::runtime_error);
    }

    DefineTest("ModUser") {
        Setup();
        client->AddUser(new_user, false, new_pass, new_desc1);
        UT_EXPECT_EQ(client->GetUserInfo(new_user).desc, new_desc1);
        client->SetUserDesc(new_user, new_desc2);
        UT_EXPECT_EQ(client->GetUserInfo(new_user).desc, new_desc2);
    }
    fma_common::file_system::RemoveDir(db_dir);
}

INSTANTIATE_TEST_CASE_P(TestRestClient, TestRestClient, testing::Values(true, false));
