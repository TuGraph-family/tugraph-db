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
#include "gtest/gtest.h"

// The 'U' macro can be used to create a string or character literal of the platform type, i.e.
// utility::char_t. If you are using a library causing conflicts with 'U' macro, it can be turned
// off by defining the macro '_TURN_OFF_PLATFORM_STRING' before including the C++ REST SDK header
// files, and e.g. use '_XPLATSTR' instead.
#define _TURN_OFF_PLATFORM_STRING
#include "cpprest/json.h"

#include "core/global_config.h"
#include "lgraph/lgraph_rpc_client.h"
#include "restful/server/json_convert.h"

#include "./test_tools.h"
#include "./ut_utils.h"

class TestLGraphServer : public TuGraphTest {};

struct GraphInfo {
    GraphInfo() {}
    GraphInfo(const std::string& d, size_t s) : desc(d), max_size_GB(s) {}

    bool operator==(const GraphInfo& rhs) const {
        return desc == rhs.desc && max_size_GB == rhs.max_size_GB;
    }

    std::string desc;
    size_t max_size_GB;
};

std::ostringstream& operator<<(std::ostringstream& os, const GraphInfo& g) {
    os << "GraphInfo(" << g.desc << ", "
       << ")";
    return os;
}

TEST_F(TestLGraphServer, LGraphServer) {
    using namespace lgraph;
    lgraph::GlobalConfig conf;
    conf.db_dir = "./testdb";
    conf.http_port = 7774;
    conf.rpc_port = 9394;
    conf.bind_host = "127.0.0.1";
#ifdef __SANITIZE_ADDRESS__
    conf.use_pthread = true;
#endif
    auto ListGraphs = [](RpcClient& client) {
        std::string graphs;
        bool succeed = client.CallCypher(graphs, "call dbms.graph.listGraphs()");
        UT_EXPECT_EQ(succeed, true);
        web::json::value v = web::json::value::parse(graphs);
        return v;
    };

    UT_LOG() << "Testing create graph, modify graph, delete graph";
    {
        AutoCleanDir cleaner(conf.db_dir);
        {
            auto server = StartLGraphServer(conf);
            // create graphs
            RpcClient client(UT_FMT("{}:{}", conf.bind_host, conf.rpc_port),
                            _detail::DEFAULT_ADMIN_NAME, _detail::DEFAULT_ADMIN_PASS);
            auto obj = ListGraphs(client)[0];
            UT_EXPECT_EQ(obj.is_object(), true);
            UT_EXPECT_EQ(obj["graph_name"].as_string(), "default");
            UT_EXPECT_EQ(obj["configuration"]["max_size_GB"].as_integer(), 4096);

            std::string str;
            bool ret = client.CallCypher(
                str, "call dbms.graph.createGraph('g1', 'this is description', 2048)");
            UT_EXPECT_EQ(ret, true);
            obj = ListGraphs(client);
            UT_EXPECT_EQ(obj.size(), 2);
            UT_EXPECT_EQ(obj[0]["graph_name"].as_string(), "default");
            UT_EXPECT_EQ(obj[1]["graph_name"].as_string(), "g1");
            UT_EXPECT_EQ(obj[1]["configuration"]["max_size_GB"].as_integer(), 2048);

            ret = client.CallCypher(str,
                                    "call dbms.graph.modGraph('default', {description:'default "
                                    "graph', max_size_GB:3})");
            UT_EXPECT_EQ(ret, true);
            obj = ListGraphs(client);
            UT_EXPECT_EQ(obj.size(), 2);
            UT_EXPECT_EQ(obj[0]["graph_name"].as_string(), "default");
            UT_EXPECT_EQ(obj[1]["graph_name"].as_string(), "g1");
            UT_EXPECT_EQ(obj[0]["configuration"]["description"].as_string(), "default graph");
            UT_EXPECT_EQ(obj[0]["configuration"]["max_size_GB"].as_integer(), 3);

            server->Kill();
            server->Wait();
        }
        std::string str;
        auto server = StartLGraphServer(conf);
        RpcClient client(UT_FMT("{}:{}", conf.bind_host, conf.rpc_port),
                         _detail::DEFAULT_ADMIN_NAME, _detail::DEFAULT_ADMIN_PASS);
        auto obj = ListGraphs(client);
        bool ret = client.CallCypher(str,
                                "call dbms.graph.modGraph('default', {description:'default "
                                "graph', max_size_GB:3})");
        UT_EXPECT_EQ(ret, true);
        obj = ListGraphs(client);
        UT_EXPECT_EQ(obj[0]["graph_name"].as_string(), "default");
        UT_EXPECT_EQ(obj[0]["configuration"]["description"].as_string(), "default graph");
        UT_EXPECT_EQ(obj[0]["configuration"]["max_size_GB"].as_integer(), 3);
        UT_EXPECT_EQ(obj[1]["graph_name"].as_string(), "g1");
        UT_EXPECT_EQ(obj[1]["configuration"]["description"].as_string(), "this is description");
        UT_EXPECT_EQ(obj[1]["configuration"]["max_size_GB"].as_integer(), 2048);
    }
}
