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
#include "lgraph/lgraph_rpc_client.h"
#include "fma-common/configuration.h"
#include "boost/algorithm/string.hpp"
#include "core/data_type.h"
#include "./test_tools.h"
#include "./graph_factory.h"
#include "tiny-process-library/process.hpp"

class TestHAFullImport : public TuGraphTest {
 protected:
    void SetUp() override {
        FILE *fp;
        char buf[100] = {0};
        fp = popen("hostname -I", "r");
        if (fp) {
            fread(buf, 1, sizeof(buf) - 1, fp);
            pclose(fp);
        }
        host = std::string(buf);
        boost::trim(host);
        int len = host.find(' ');
        if (len != std::string::npos) {
            host = host.substr(0, len);
        }
#ifndef __SANITIZE_ADDRESS__
        std::string cmd_f =
            "mkdir {} && cp -r ../../src/server/lgraph_ha.json "
            "./lgraph_server ./resource {} "
            "&& cd {} && ./lgraph_server --host {} --port {} --enable_rpc "
            "true --enable_ha true --ha_snapshot_interval_s -1 "
            "--rpc_port {} --directory ./db --log_dir "
            "./log  --ha_conf {} --verbose 1 -c lgraph_ha.json -d start";
#else
        std::string cmd_f =
            "mkdir {} && cp -r ../../src/server/lgraph_ha.json "
            "./lgraph_server ./resource {} "
            "&& cd {} && ./lgraph_server --host {} --port {} --enable_rpc "
            "true --enable_ha true --ha_snapshot_interval_s -1 "
            "--rpc_port {} --directory ./db --log_dir "
            "./log  --ha_conf {} --use_pthread 1 --verbose 1 -c lgraph_ha.json -d start";
#endif

        int rt;
        std::string cmd = FMA_FMT(cmd_f.c_str(), "ha1", "ha1", "ha1", host, "27072", "29092",
                                  host + ":29092," + host + ":29093," + host + ":29094");
        rt = system(cmd.c_str());
        UT_EXPECT_EQ(rt, 0);
        fma_common::SleepS(5);
        cmd = FMA_FMT(cmd_f.c_str(), "ha2", "ha2", "ha2", host, "27073", "29093",
                      host + ":29092," + host + ":29093," + host + ":29094");
        rt = system(cmd.c_str());
        UT_EXPECT_EQ(rt, 0);
        fma_common::SleepS(5);
        cmd = FMA_FMT(cmd_f.c_str(), "ha3", "ha3", "ha3", host, "27074", "29094",
                      host + ":29092," + host + ":29093," + host + ":29094");
        rt = system(cmd.c_str());
        UT_EXPECT_EQ(rt, 0);
        fma_common::SleepS(15);
    }

    void TearDown() override {
        std::string cmd_f = "cd {} && ./lgraph_server -c lgraph_ha.json -d stop";
        for (int i = 1; i < 4; ++i) {
            std::string dir = "ha" + std::to_string(i);
            std::string cmd = FMA_FMT(cmd_f.c_str(), dir);
            int rt = system(cmd.c_str());
            UT_EXPECT_EQ(rt, 0);
            cmd = FMA_FMT("rm -rf {}", dir);
            rt = system(cmd.c_str());
            UT_EXPECT_EQ(rt, 0);
        }
        int rt = system("rm -rf add_vertex_v.so");
        UT_EXPECT_EQ(rt, 0);
    }

 public:
    std::string host;
};

TEST_F(TestHAFullImport, FullImport) {
    // create yago files
    GraphFactory::WriteYagoFiles();
    lgraph::SubProcess import_client(
        FMA_FMT("./lgraph_import -c yago.conf -d ./test_ha_import_db --overwrite 1"));
    import_client.Wait();
    UT_EXPECT_EQ(import_client.GetExitCode(), 0);
    fma_common::SleepS(5);

    // get lmdb data file
    std::string data_file_path;
    for (const auto& entry : std::filesystem::directory_iterator("./test_ha_import_db")) {
        if (entry.is_directory() && entry.path().filename().string() != ".meta" &&
            std::filesystem::exists(entry.path().string() + "/data.mdb")) {
            data_file_path = entry.path().string() + "/data.mdb";
            break;
        }
    }
    data_file_path = std::filesystem::absolute(data_file_path).string();

    // ok, now check imported data
    std::unique_ptr<lgraph::RpcClient> rpc_client = std::make_unique<lgraph::RpcClient>(
        this->host + ":29092", "admin", "73@TuGraph");
    std::string res;
    bool succeed = rpc_client->CallCypher(res, FMA_FMT(R"(CALL db.importor.fullFileImportor("{}","{}"))",
                                                       "ha", data_file_path));
    UT_EXPECT_TRUE(succeed);
    succeed = rpc_client->CallCypherToLeader(res, "match (n) return count(n)", "ha");
    UT_LOG() << res;
    UT_EXPECT_TRUE(succeed);
    web::json::value v = web::json::value::parse(res);
    UT_EXPECT_EQ(v[0]["count(n)"].as_integer(), 21);
    bool ret = rpc_client->CallCypher(res, "CALL dbms.ha.clusterInfo()");
    UT_EXPECT_TRUE(ret);
    std::string master_rest;
    nlohmann::json cluster_info = nlohmann::json::parse(res.c_str());
    for (auto &node : cluster_info[0]["cluster_info"]) {
        if (node["state"] == "MASTER") {
            master_rest = to_string(node["rest_address"]);
            std::vector<std::string> output;
            boost::split(output, master_rest, boost::is_any_of("\\\""));
            master_rest = output[1];
        }
    }
    std::string import_cmd = FMA_FMT("./lgraph_import --online true --online_type 2 "
        "-r http://{} -u {} -p {} -g test --path {}",
        master_rest, lgraph::_detail::DEFAULT_ADMIN_NAME,
        lgraph::_detail::DEFAULT_ADMIN_PASS,
        data_file_path);
    UT_LOG() << import_cmd;
    lgraph::SubProcess online_import_client(import_cmd);
    online_import_client.Wait();
    UT_EXPECT_EQ(online_import_client.GetExitCode(), 0);
    fma_common::SleepS(20);
    succeed = rpc_client->CallCypherToLeader(res, "match (n) return count(n)", "test");
    UT_EXPECT_TRUE(succeed);
    v = web::json::value::parse(res);
    UT_EXPECT_EQ(v[0]["count(n)"].as_integer(), 21);
    rpc_client->Logout();
}

TEST_F(TestHAFullImport, FullImportRemote) {
    // ok, now check imported data
    GraphFactory::WriteYagoFiles();
    lgraph::SubProcess import_client(
        FMA_FMT("./lgraph_import -c yago.conf -d ./test_ha_import_db --overwrite 1"));
    import_client.Wait();
    UT_EXPECT_EQ(import_client.GetExitCode(), 0);
    fma_common::SleepS(5);

    // get lmdb data file
    std::string data_file_dir;
    for (const auto& entry : std::filesystem::directory_iterator("./test_ha_import_db")) {
        if (entry.is_directory() && entry.path().filename().string() != ".meta" &&
            std::filesystem::exists(entry.path().string() + "/data.mdb")) {
            data_file_dir = entry.path().string();
            break;
        }
    }
    data_file_dir = std::filesystem::absolute(data_file_dir).string();
    lgraph::SubProcess server_for_mdb(
        FMA_FMT("cd {} && python3 -m http.server 28082", data_file_dir));
    UT_LOG() << FMA_FMT("cd {} && python3 -m http.server 28082", data_file_dir);
    fma_common::SleepS(10);

    std::unique_ptr<lgraph::RpcClient> rpc_client = std::make_unique<lgraph::RpcClient>(
        this->host + ":29092", "admin", "73@TuGraph");
    std::string res;
    bool succeed = rpc_client->CallCypher(res, FMA_FMT(R"(CALL db.importor.fullFileImportor("{}","{}",true))",
                                                       "ha", "http://localhost:28082/data.mdb"));
    UT_EXPECT_TRUE(succeed);
    succeed = rpc_client->CallCypherToLeader(res, "match (n) return count(n)", "ha");
    UT_LOG() << res;
    UT_EXPECT_TRUE(succeed);
    web::json::value v = web::json::value::parse(res);
    UT_EXPECT_EQ(v[0]["count(n)"].as_integer(), 21);
    bool ret = rpc_client->CallCypher(res, "CALL dbms.ha.clusterInfo()");
    UT_EXPECT_TRUE(ret);
    std::string master_rest;
    nlohmann::json cluster_info = nlohmann::json::parse(res.c_str());
    for (auto &node : cluster_info[0]["cluster_info"]) {
        if (node["state"] == "MASTER") {
            master_rest = to_string(node["rest_address"]);
            std::vector<std::string> output;
            boost::split(output, master_rest, boost::is_any_of("\\\""));
            master_rest = output[1];
        }
    }
    std::string import_cmd = FMA_FMT("./lgraph_import --online true --online_type 2 "
        "-r http://{} -u {} -p {} -g test --path {} --remote true",
        master_rest, lgraph::_detail::DEFAULT_ADMIN_NAME,
        lgraph::_detail::DEFAULT_ADMIN_PASS,
        "http://localhost:28082/data.mdb");
    UT_LOG() << import_cmd;
    lgraph::SubProcess online_import_client(import_cmd);
    online_import_client.Wait();
    UT_EXPECT_EQ(online_import_client.GetExitCode(), 0);
    fma_common::SleepS(40);
    succeed = rpc_client->CallCypherToLeader(res, "match (n) return count(n)", "test");
    UT_EXPECT_TRUE(succeed);
    v = web::json::value::parse(res);
    UT_EXPECT_EQ(v[0]["count(n)"].as_integer(), 21);
    rpc_client->Logout();
    server_for_mdb.Kill();
}
