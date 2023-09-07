/* Copyright (c) 2022 AntGroup. All Rights Reserved. */
#include "gtest/gtest.h"
#include "./ut_utils.h"
#include "lgraph/lgraph.h"
#include "lgraph/lgraph_rpc_client.h"
#include "fma-common/configuration.h"
#include <boost/algorithm/string.hpp>

void build_add_vertex_so() {
    const std::string INCLUDE_DIR = "../../include";
    const std::string DEPS_INCLUDE_DIR = "../../deps/install/include";
    const std::string LIBLGRAPH = "./liblgraph.so";
    int rt;
#ifndef __clang__
    std::string cmd_f =
        "g++ -fno-gnu-unique -fPIC -g --std=c++17 -I {} -I {} -rdynamic -O3 -fopenmp -DNDEBUG "
        "-o {} {} {} -shared";
#elif __APPLE__
    std::string cmd_f =
        "clang++ -stdlib=libc++ -fPIC -g --std=c++17 -I {} -I {} -rdynamic -O3 -Xpreprocessor "
        "-fopenmp -DNDEBUG "
        "-o {} {} {} -shared";
#else
    std::string cmd_f =
        "clang++ -stdlib=libc++ -fPIC -g --std=c++17 -I {} -I {} -rdynamic -O3 -fopenmp -DNDEBUG "
        "-o {} {} {} -shared";
#endif
    std::string cmd;
    cmd = FMA_FMT(cmd_f.c_str(), INCLUDE_DIR, DEPS_INCLUDE_DIR, "./add_vertex_v.so",
                  "../../test/test_procedures/add_vertex_v.cpp", LIBLGRAPH);
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
}

class TestHAConsistency : public TuGraphTest {
 protected:
    void SetUp() override {
        build_add_vertex_so();
        FILE *fp;
        char buf[100]={0};
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
            "true --enable_ha true --ha_node_offline_ms 5000 "
            "--ha_node_remove_ms 10000 "
            "--rpc_port {} --directory ./db --log_dir "
            "./log  --ha_conf {} --verbose 1 -c lgraph_ha.json -d start";
#else
        std::string cmd_f =
            "mkdir {} && cp -r ../../src/server/lgraph_ha.json "
            "./lgraph_server ./resource {} "
            "&& cd {} && ./lgraph_server --host {} --port {} --enable_rpc "
            "true --enable_ha true --ha_node_offline_ms 5000 "
            "--ha_node_remove_ms 10000 "
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

TEST_F(TestHAConsistency, HAConsistency) {
    std::thread thread = std::thread([this]{
        lgraph::RpcClient rpcClient(this->host + ":29092", "admin", "73@TuGraph");
        std::string result;
        rpcClient.LoadProcedure(result, "add_vertex_v.so", "CPP",
                                  "add_vertex_v", "SO", "", false);
        rpcClient.CallProcedure(result, "CPP", "add_vertex_v", "{}");
        rpcClient.Logout();
    });
    fma_common::SleepS(5);
    std::string cmd_f = "cd {} && ./lgraph_server -c lgraph_ha.json -d stop";
    std::string cmd = FMA_FMT(cmd_f.c_str(), "ha3");
    int rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    fma_common::SleepS(5);
#ifndef __SANITIZE_ADDRESS__
    cmd_f = "cd {} && ./lgraph_server --host {} --port {} --enable_rpc "
        "true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
        "--rpc_port {} --directory ./db --log_dir "
        "./log  --ha_conf {} -c lgraph_ha.json -d start";
#else
    cmd_f = "cd {} && ./lgraph_server --host {} --port {} --enable_rpc "
        "true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
        "--rpc_port {} --directory ./db --log_dir "
        "./log  --ha_conf {} --use_pthread 1 --verbose 1 -c lgraph_ha.json -d start";
#endif
    cmd = FMA_FMT(cmd_f.c_str(), "ha3", host, "27074", "29094",
                  host + ":29092," + host + ":29093," + host + ":29094");
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    fma_common::SleepS(10);
    if (thread.joinable())
        thread.join();
    lgraph::RpcClient rpcClient(this->host + ":29092", "admin", "73@TuGraph");
    fma_common::SleepS(30);
    std::string result;
    rpcClient.CallCypher(result, "MATCH (n) RETURN COUNT(n)", "default", true, 0,
                           host + ":29094");
    nlohmann::json res = nlohmann::json::parse(result);
    UT_EXPECT_EQ(res[0]["COUNT(n)"], 3000000);
}
