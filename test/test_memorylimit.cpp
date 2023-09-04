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
#include "fma-common/logging.h"
#include "fma-common/string_formatter.h"
#include "gtest/gtest.h"

// The 'U' macro can be used to create a string or character literal of the platform type, i.e.
// utility::char_t. If you are using a library causing conflicts with 'U' macro, it can be turned
// off by defining the macro '_TURN_OFF_PLATFORM_STRING' before including the C++ REST SDK header
// files, and e.g. use '_XPLATSTR' instead.
#define _TURN_OFF_PLATFORM_STRING
#include "cpprest/http_client.h"

#include "restful/server/stdafx.h"
#include "client/cpp/restful/rest_client.h"
#include "server/lgraph_server.h"

#include "./graph_factory.h"
#include "./test_tools.h"
#include "./ut_utils.h"
using namespace utility;               // Common utilities like string conversions
using namespace web;                   // Common features like URIs.
using namespace web::http;             // Common HTTP functionality
using namespace web::http::client;     // HTTP client features
using namespace concurrency::streams;  // Asynchronous streams

class TestMemoryLimit : public TuGraphTest {};

TEST_F(TestMemoryLimit, MemoryLimit) {
    using namespace fma_common;
    using namespace lgraph;
    bool enable_ssl = true;
    std::string host = "127.0.0.1";
    uint16_t port = 6888;
    std::string db_name = "default";
    std::string test_db_name = "memory_graph";
    const std::string key_path = "./key";
    const std::string cert_path = "./cert";
    std::string db_dir = "./testdb";

    std::shared_ptr<lgraph::GlobalConfig> gconfig = std::make_shared<lgraph::GlobalConfig>();
#ifdef _WIN32
    enable_ssl = false;
#endif

    if (enable_ssl) {
        lgraph::WriteCertFiles(cert_path, key_path);
    }

    // -----------------------------------
    // start server
    // build a test db
    AutoCleanDir _data_cleaner(db_dir);
    // Build listener's URI from the configured address and the hard-coded path "RestServer/Action"
    gconfig->bind_host = host;
    gconfig->http_port = port;
    gconfig->enable_ssl = enable_ssl;
    gconfig->enable_audit_log = true;
    gconfig->db_dir = db_dir;
    gconfig->server_key_file = key_path;
    gconfig->server_cert_file = cert_path;
    LGraphServer server(gconfig);
    server.Start();

    // -----------------------------------
    // server started, now start test
    std::string url;
    if (enable_ssl)
        url = fma_common::StringFormatter::Format("https://{}:{}/", host, port);
    else
        url = fma_common::StringFormatter::Format("http://{}:{}/", host, port);
    UT_LOG() << "  url:  " << url;

    UT_LOG() << "Testing login";
    RestClient client(url, cert_path);
    client.Login("admin", "73@TuGraph");
    client.EvalCypher(db_name,
                      "CALL dbms.graph.createGraph('memory_graph','memory limit test',2045)");
    client.AddUser("memory_test1", true, "pd111");
    client.EvalCypher(
        test_db_name,
        "CALL dbms.security.modRoleAccessLevel('memory_test1',{memory_graph : 'FULL'})");
    client.EvalCypher(test_db_name,
                      "CALL dbms.security.setUserMemoryLimit('memory_test1', 786433)");

    // make sure set memory_limit success and default memory_limit.
    auto uiret = client.EvalCypher(test_db_name, "CALL dbms.security.getUserInfo('memory_test1')");
    auto uinfo = uiret[_TU("result")].serialize();
    UT_EXPECT_GT(uinfo.find("786433"), 0);
    uiret = client.EvalCypher(test_db_name, "CALL dbms.security.getUserInfo('admin')");
    uinfo = uiret[_TU("result")].serialize();
    UT_EXPECT_GT(uinfo.find("2199023255552"), 0);

    client.AddUser("memory_test2", true, "pd111");
    client.EvalCypher(
        test_db_name,
        "CALL dbms.security.modRoleAccessLevel('memory_test2',{memory_graph : 'FULL'})");
    client.EvalCypher(test_db_name,
                      "CALL dbms.security.setUserMemoryLimit('memory_test2', 786433)");
    client.AddUser("memory_test3", true, "pd111");
    client.EvalCypher(
        test_db_name,
        "CALL dbms.security.modRoleAccessLevel('memory_test2',{memory_graph : 'FULL'})");
    client.EvalCypher(test_db_name,
                      "CALL dbms.security.setUserMemoryLimit('memory_test3', 786433)");
    client.Login("memory_test1", "pd111");
    std::vector<FieldSpec> fds;
    fds.emplace_back(FieldSpec("name", FieldType::INT64, false));
    client.AddVertexLabel(test_db_name, "node", fds, "name");

#define VERTEXES_NUM 500000
#define CLIENT_NUM 4

    UT_LOG() << "Testing MemoryLimit";
    {
        std::vector<std::vector<FieldData>> field_values_vector;
        for (int64_t i = 1; i <= VERTEXES_NUM; i++) {
            field_values_vector.push_back(std::vector<FieldData>{FieldData((int64_t)i)});
        }

        auto ids = client.AddVertexes(test_db_name, "node", std::vector<std::string>{"name"},
                                      field_values_vector);
        UT_EXPECT_EQ(ids.size(), VERTEXES_NUM);
        UT_LOG() << "Finish Import";

        json::value ret;
        std::vector<std::unique_ptr<RestClient>> clients;
        for (int i = 0; i < CLIENT_NUM; i++) {
            clients.emplace_back(new RestClient(url, cert_path));
            clients[i]->Login("memory_test1", "pd111");
        }
        int thread_num = 4;

        std::vector<std::thread> thrds;

        ret = client.EvalCypher(test_db_name, "MATCH (n:node) RETURN n.name ORDER BY n.name");

        UT_LOG() << "Single thread success";

        int cnt = 0;
        int id = 0;
        std::mutex lck;
        for (int i = 0; i < thread_num; i++) {
            thrds.emplace_back([&]() {
                int seq;
                lck.lock();
                seq = id++;
                lck.unlock();
                try {
                    clients[seq]->EvalCypher(test_db_name,
                                             "MATCH (n:node) RETURN n.name ORDER BY n.name");
                } catch (std::exception& e) {
                    cnt++;
                    std::string store = e.what();
                    uint64_t result = store.find("excess user memory limit");
                    if (result > 0) cnt++;
                }
            });
        }
        for (int i = 0; i < thread_num; i++) {
            thrds[i].join();
        }
        UT_EXPECT_GT(cnt, 0);
        ret = client.EvalCypher(test_db_name,
                                "CALL dbms.security.getUserMemoryUsage('memory_test1')");
        auto memory_usage = ret[_TU("result")].serialize();
        UT_EXPECT_EQ(memory_usage, "[[0]]");
        UT_LOG() << "Multi-thread success";

        clients[1]->Login("admin", "73@TuGraph");
        clients[2]->Login("memory_test2", "pd111");
        clients[3]->Login("memory_test3", "pd111");
        std::vector<std::thread> thrd;

        id = 0;

        for (int i = 0; i < thread_num; i++) {
            thrd.emplace_back([&]() {
                int seq;
                lck.lock();
                seq = id++;
                lck.unlock();
                try {
                    clients[seq]->EvalCypher(test_db_name,
                                             "MATCH (n:node) RETURN n.name ORDER BY n.name");
                } catch (std::exception& e) {
                    cnt++;
                    UT_LOG() << e.what();
                }
            });
        }
        for (int i = 0; i < thread_num; i++) {
            thrd[i].join();
        }
    }
    client.Login("admin", "73@TuGraph");
    client.DeleteUser("memory_test1");
    client.DeleteUser("memory_test2");
    client.DeleteUser("memory_test3");
    client.EvalCypher(test_db_name, "CALL dbms.graph.deleteGraph('memory_graph')");
    server.Stop();
    UT_LOG() << "Testing MemoryLimit succeeded";
}
