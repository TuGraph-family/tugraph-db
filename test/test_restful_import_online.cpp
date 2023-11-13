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
#include "fma-common/utils.h"
#include "gtest/gtest.h"
#include "restful/server/stdafx.h"
#include "restful/server/rest_server.h"
#include "client/cpp/restful/rest_client.h"
#include "fma-common/string_formatter.h"

#include "./graph_factory.h"
#include "./test_tools.h"
#include "./ut_utils.h"
using namespace utility;               // Common utilities like string conversions
using namespace web;                   // Common features like URIs.
using namespace web::http;             // Common HTTP functionality
using namespace web::http::client;     // HTTP client features
using namespace concurrency::streams;  // Asynchronous streams

void test_client_other_branch(RestClient& client);

inline void WriteFiles(const std::map<std::string, std::string>& name_contents) {
    for (auto& kv : name_contents) {
        fma_common::OutputFmaStream stream(kv.first);
        stream.Write(kv.second.data(), kv.second.size());
    }
}

class TestRestfulImportOnline : public TuGraphTestWithParam<bool> {};

TEST_P(TestRestfulImportOnline, RestfulImportOnline) {
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

    if (enable_ssl) {
        lgraph::WriteCertFiles(cert_path, key_path);
    }

    // -----------------------------------
    // start server
    // build a test db
    { GraphFactory::create_modern(); }
    // Build listener's URI from the configured address and the hard-coded path
    // "RestServer/Action"
    lgraph::StateMachine::Config sm_config;
    sm_config.db_dir = "./testdb";
    lgraph::StateMachine state_machine(sm_config, gconfig);
    auto res = state_machine.GetMasterRestAddr();
    UT_EXPECT_EQ(res, "");
    state_machine.Start();
    lgraph::RestServer::Config rest_config;
    rest_config.host = host;
    rest_config.port = port;
    rest_config.use_ssl = enable_ssl;
    if (enable_ssl) {
        UT_EXPECT_ANY_THROW(lgraph::RestServer(&state_machine, rest_config, gconfig));
        rest_config.server_key = key_path;
        UT_EXPECT_ANY_THROW(lgraph::RestServer(&state_machine, rest_config, gconfig));
        rest_config.server_cert = cert_path;
    }
    // open the db_config api to set
    lgraph::RestServer rest_server(&state_machine, rest_config, gconfig);
    // -----------------------------------
    // server started, now start test
    std::string url;
    if (enable_ssl)
        url = fma_common::StringFormatter::Format("https://{}:{}/", host, port);
    else
        url = fma_common::StringFormatter::Format("http://{}:{}/", host, port);
    UT_LOG() << "  url:  " << url;
    // now start client
    RestClient client(url, cert_path);
    client.Login("admin", "73@TuGraph");

    // schema
    UT_LOG() << "add company schema";
    {
        std::vector<FieldSpec> fds;
        fds.emplace_back(FieldSpec("name", FieldType::STRING, false));
        fds.emplace_back(FieldSpec("address", FieldType::STRING, false));
        fds.emplace_back(FieldSpec("scale", FieldType::INT32, false));
        UT_EXPECT_EQ(client.AddVertexLabel(db_name, "company", fds, "name"), true);
        UT_EXPECT_ANY_THROW(client.AddVertexLabel(db_name, "company", fds, "name"));
    }
    UT_LOG() << "Testing schema succeeded";

#ifndef _WIN32
// In visual studio, when we use multi-byte chars, the encoding of Chinese characters
// causes errors. So disable this test for windows.
    UT_LOG() << "Testing import";
    {
        // std::string desc = "LABEL=company\nscale,address,name";
        std::string desc = R"(
{
    "files" : [
        {
            "label" : "company",
            "format" : "CSV",
            "columns" : ["scale","address","name"]
        }
     ]
})";
        std::string data = "1,清华西路,费马1\n3,清华西路,费马2\n";
        bool res_get = client.AddIndex(db_name, "company", "scale", 1);
        UT_EXPECT_EQ(res_get, true);
        // client.AddIndex(db_name, "person", "uid", true);
        auto res = client.SendImportData(db_name, desc, data, true, ",");
        UT_LOG() << res;
        UT_EXPECT_EQ(res, "");
        // 加入check，导入之后再查询

        std::string label_name("company");
        auto v_map = client.GetVertex(db_name, 6, label_name);
        UT_LOG() << "1111";
        for (auto& kv : v_map) {
            UT_LOG() << kv.first << " has value " << kv.second.ToString();
        }
        UT_EXPECT_EQ(v_map["scale"].AsInt64(), 1);
        UT_LOG() << "3333333333";
        std::string tmp("清华西路");
        UT_EXPECT_EQ(v_map["address"].AsString(), tmp);
        tmp = "费马1";
        UT_EXPECT_EQ(v_map["name"].AsString(), tmp);
        v_map = client.GetVertex(db_name, 7, label_name);
        UT_EXPECT_EQ(v_map["scale"].AsInt64(), 3);
        tmp = "清华西路";
        UT_EXPECT_EQ(v_map["address"].AsString(), tmp);
        tmp = "费马2";
        UT_EXPECT_EQ(v_map["name"].AsString(), tmp);

        data = "1,清华西路\n";
        UT_EXPECT_ANY_THROW(client.SendImportData(db_name, desc, data, false, ","));

        // desc = "LABEL=company,HEADER=2\nscale,address,name";
        desc = R"(
{
    "files" : [
        {   
            "header" : 2,
            "label" : "company",
            "format" : "CSV",
            "columns" : ["scale","address","name"]
        }
     ]
})";

        data = "sda\nad\n2,清华西路,费马\n";
        res = client.SendImportData(db_name, desc, data, true, ",");
        UT_EXPECT_EQ(res, "");
        // check the last vertex 2
        v_map = client.GetVertex(db_name, 8, label_name);
        UT_EXPECT_EQ(v_map["scale"].AsInt64(), 2);
        tmp = "清华西路";
        UT_EXPECT_EQ(v_map["address"].AsString(), tmp);
        tmp = "费马";
        UT_EXPECT_EQ(v_map["name"].AsString(), tmp);

        // import edge data
        // desc =
        // "LABEL=knows,SRC_ID=person:uid,DST_ID=person:uid\n@SRC_ID,@DST_ID,weight,since";
        desc = R"(
{
    "files" : [
        {   
            "label" : "knows",
            "SRC_ID" : "person",
            "DST_ID" : "person",
            "format" : "CSV",
            "columns" : ["SRC_ID","DST_ID","weight","since"]
        }
     ]
})";
        data = "6,6,0.9,1927\n";
        res = client.SendImportData(db_name, desc, data, true, ",");
        UT_EXPECT_EQ(res, "");
    }
    UT_LOG() << "Testing import succeeded";
#endif

    UT_LOG() << "\n" << __func__ << " succeeded";
    UT_LOG() << "\n" << __func__ << " exit";
}

INSTANTIATE_TEST_CASE_P(TestRestfulImportOnline, TestRestfulImportOnline,
                        testing::Values(true, false));
