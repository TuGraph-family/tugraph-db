/**
* Copyright 2024 AntGroup CO., Ltd.
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

class TestHALGraphPeer : public TuGraphTest {
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
            "true --enable_ha true --ha_node_offline_ms 3000 "
            "--ha_node_remove_ms 5000 --ha_snapshot_interval_s -1 "
            "--rpc_port {} --directory ./db --log_dir "
            "./log  --ha_conf {} --verbose 1 -c lgraph_ha.json -d start";
#else
        std::string cmd_f =
            "mkdir {} && cp -r ../../src/server/lgraph_ha.json "
            "./lgraph_server ./resource {} "
            "&& cd {} && ./lgraph_server --host {} --port {} --enable_rpc "
            "true --enable_ha true --ha_node_offline_ms 3000 "
            "--ha_node_remove_ms 5000 --ha_snapshot_interval_s -1 "
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
        fma_common::SleepS(5);
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

TEST_F(TestHALGraphPeer, LGraphPeer) {
    std::string ha_all_conf = host + ":29092," + host + ":29093" + host + ":29094";
    fma_common::SleepS(10);

    // test transfer_leader
    lgraph::SubProcess transfer_process(
        UT_FMT("./lgraph_peer --command transfer_leader --peer {} --conf {}",
               host + ":29093",
               ha_all_conf));
    UT_EXPECT_TRUE(transfer_process.Wait(10000));
    fma_common::SleepS(10);
    lgraph::RpcClient rpcClient(host + ":29092", "admin", "73@TuGraph");
    std::string result;
    rpcClient.CallCypher(result, "CALL dbms.ha.clusterInfo()", "default", true, 10);
    UT_LOG() << result;
    nlohmann::json cluster_info = nlohmann::json::parse(result.c_str());
    for (auto &node : cluster_info[0]["cluster_info"]) {
        if (node["state"] == "MASTER") {
            UT_EXPECT_TRUE(node["rpc_address"] == host + ":29093");
        }
    }
    rpcClient.Logout();

    // test remove peer
    lgraph::SubProcess remove_process(
        UT_FMT("./lgraph_peer --command remove_peer --peer {} --conf {}",
               host + ":29094",
               ha_all_conf));
    UT_EXPECT_TRUE(remove_process.Wait(10000));
    fma_common::SleepS(10);
    lgraph::RpcClient rpcClient2(host + ":29092", "admin", "73@TuGraph");
    rpcClient2.CallCypher(result, "CALL dbms.ha.clusterInfo()", "default", true, 10);
    cluster_info = nlohmann::json::parse(result.c_str());
    rpcClient2.Logout();
    UT_LOG() << cluster_info;
    UT_EXPECT_TRUE(cluster_info[0]["cluster_info"].size() == 2);
    for (auto &node : cluster_info[0]["cluster_info"]) {
        UT_EXPECT_TRUE(node["rpc_address"] != host + ":29094");
    }

    // test snapshot
    lgraph::RpcClient rpcClient3(host + ":29092", "admin", "73@TuGraph");
    rpcClient3.CallCypher(result, "CALL dbms.graph.createGraph('ha', 'description', 2045)");
    lgraph::SubProcess snapshot_process(
        UT_FMT("./lgraph_peer --command snapshot --peer {}",
               host + ":29093"));
    UT_EXPECT_TRUE(snapshot_process.Wait(10000));
    rpcClient3.Logout();
}
