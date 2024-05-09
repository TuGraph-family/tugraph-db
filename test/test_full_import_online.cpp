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
#include "./ut_utils.h"
#include "./ut_config.h"
#include "cpprest/json.h"
#include "import/import_client.h"
#include "lgraph/lgraph_rpc_client.h"

class TestFullImportOnline : public TuGraphTest {
 public:
    std::string db_dir = "./test_full_import_db";
    uint16_t port = 17772;
    uint16_t rpc_port = 19992;

 protected:
    void SetUp() override {
        using namespace lgraph;
#ifndef __SANITIZE_ADDRESS__
        std::string server_cmd = FMA_FMT(
            "./lgraph_server -c lgraph_standalone.json --port {} --rpc_port {}"
            " --host 127.0.0.1 --verbose 1 --directory {} -d start",
            port, rpc_port, db_dir);
#else
        std::string server_cmd = FMA_FMT(
            "./lgraph_server -c lgraph_standalone.json --port {} --rpc_port {}"
            " --host 127.0.0.1 --verbose 1 --directory {} --use_pthread true -d start",
            port, rpc_port, db_dir);
#endif
        UT_EXPECT_EQ(system(server_cmd.c_str()), 0);
        fma_common::SleepS(5);
    }

    void TearDown() override {
        using namespace lgraph;
#ifndef __SANITIZE_ADDRESS__
        std::string server_cmd = FMA_FMT(
            "./lgraph_server -c lgraph_standalone.json --port {} --rpc_port {}"
            " --host 127.0.0.1 --verbose 1 --directory {} -d stop",
            port, rpc_port, db_dir);
#else
        std::string server_cmd = FMA_FMT(
            "./lgraph_server -c lgraph_standalone.json --port {} --rpc_port {}"
            " --host 127.0.0.1 --verbose 1 --directory {} --use_pthread true -d stop",
            port, rpc_port, db_dir);
#endif
        int rt;
        rt = system(server_cmd.c_str());
        UT_EXPECT_EQ(rt, 0);
        fma_common::SleepS(5);
        std::string cmd = FMA_FMT("rm -rf {}", db_dir);
        rt = system(cmd.c_str());
        UT_EXPECT_EQ(rt, 0);
    }
};

TEST_F(TestFullImportOnline, MFByOtherProcess) {
    using namespace lgraph;
    std::string import_cmd = FMA_FMT("./lgraph_import --online true --online_type 1 "
        "-c {} -r http://127.0.0.1:{} -u {} -p {} --overwrite true --delimiter \"|\"",
        lgraph::ut::TEST_RESOURCE_DIRECTORY + "/data/mini_finbench/mini_finbench.json",
        port, lgraph::_detail::DEFAULT_ADMIN_NAME, lgraph::_detail::DEFAULT_ADMIN_PASS);
    UT_LOG() << import_cmd;
    SubProcess import_client(import_cmd);
    import_client.Wait();
    UT_EXPECT_EQ(import_client.GetExitCode(), 0);
    lgraph::RpcClient rpc_client(FMA_FMT("127.0.0.1:{}", rpc_port),
                                 lgraph::_detail::DEFAULT_ADMIN_NAME,
                                 lgraph::_detail::DEFAULT_ADMIN_PASS);
    std::string res;
    bool succeed = rpc_client.CallCypher(res, "match (n) return count(n)");
    UT_EXPECT_EQ(succeed, true);
    web::json::value v = web::json::value::parse(res);
    UT_EXPECT_EQ(v[0]["count(n)"].as_integer(), 100);
}

TEST_F(TestFullImportOnline, MFByCurrentProcess) {
    lgraph::import_v2::OnlineImportClient::Config config;
    config.url = "http://127.0.0.1:17772/";
    config.config_file =
        lgraph::ut::TEST_RESOURCE_DIRECTORY + "/data/mini_finbench/mini_finbench.json";
    config.username = lgraph::_detail::DEFAULT_ADMIN_NAME;
    config.password = lgraph::_detail::DEFAULT_ADMIN_PASS;
    config.delete_if_exists = true;
    config.delimiter = "|";
    lgraph::import_v2::OnlineImportClient client(config);
    client.DoFullImport();
    lgraph::RpcClient rpc_client(FMA_FMT("127.0.0.1:{}", rpc_port),
                                 config.username, config.password);
    std::string res;
    bool succeed = rpc_client.CallCypher(res, "match (n) return count(n)");
    UT_EXPECT_EQ(succeed, true);
    web::json::value v = web::json::value::parse(res);
    UT_EXPECT_EQ(v[0]["count(n)"].as_integer(), 100);
}

TEST_F(TestFullImportOnline, FBByOtherProcess) {
    using namespace lgraph;
    std::string import_cmd = FMA_FMT("./lgraph_import --online true --online_type 1 "
        "-c {} -r http://127.0.0.1:{} -u {} -p {} -g test --delimiter ,",
        lgraph::ut::TEST_RESOURCE_DIRECTORY + "/../integration/data/algo/fb.conf",
        port, lgraph::_detail::DEFAULT_ADMIN_NAME, lgraph::_detail::DEFAULT_ADMIN_PASS);
    UT_LOG() << import_cmd;
    SubProcess import_client(import_cmd);
    import_client.Wait();
    UT_EXPECT_EQ(import_client.GetExitCode(), 0);
    lgraph::RpcClient rpc_client(FMA_FMT("127.0.0.1:{}", rpc_port),
                                 lgraph::_detail::DEFAULT_ADMIN_NAME,
                                 lgraph::_detail::DEFAULT_ADMIN_PASS);
    std::string res;
    bool succeed = rpc_client.CallCypher(res, "match (n) return count(n)", "test");
    UT_EXPECT_EQ(succeed, true);
    web::json::value v = web::json::value::parse(res);
    UT_EXPECT_EQ(v[0]["count(n)"].as_integer(), 4039);
}

TEST_F(TestFullImportOnline, FBByCurrentProcess) {
    lgraph::import_v2::OnlineImportClient::Config config;
    config.url = "http://127.0.0.1:17772/";
    config.config_file =
        lgraph::ut::TEST_RESOURCE_DIRECTORY + "/../integration/data/algo/fb.conf";
    config.username = lgraph::_detail::DEFAULT_ADMIN_NAME;
    config.password = lgraph::_detail::DEFAULT_ADMIN_PASS;
    config.graph_name = "test";
    config.delete_if_exists = false;
    config.delimiter = ",";
    lgraph::import_v2::OnlineImportClient client(config);
    client.DoFullImport();
    lgraph::RpcClient rpc_client(FMA_FMT("127.0.0.1:{}", rpc_port),
                                 config.username, config.password);
    std::string res;
    bool succeed = rpc_client.CallCypher(res, "match (n) return count(n)", "test");
    UT_EXPECT_EQ(succeed, true);
    web::json::value v = web::json::value::parse(res);
    UT_EXPECT_EQ(v[0]["count(n)"].as_integer(), 4039);
}
