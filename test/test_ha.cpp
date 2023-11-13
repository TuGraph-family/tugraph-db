/* Copyright (c) 2022 AntGroup. All Rights Reserved. */
#include "gtest/gtest.h"
#include "./ut_utils.h"
#include "lgraph/lgraph.h"
#include "lgraph/lgraph_rpc_client.h"
#include "fma-common/configuration.h"
#include <boost/algorithm/string.hpp>

void build_so(const std::string& so_name, const std::string& so_path) {
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
    cmd = FMA_FMT(cmd_f.c_str(), INCLUDE_DIR, DEPS_INCLUDE_DIR, so_name,
                  so_path, LIBLGRAPH);
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
}

bool HasElement(const nlohmann::json& val, const std::string& value, const std::string& field) {
    if (val.is_array()) {
        for (const auto & i : val) {
            if (i.contains(field)) {
                if (i.at(field).get<std::string>() == value) {
                    return true;
                }
            }
        }
    } else if (val.is_object()) {
        if (!val.contains(field)) return false;
        if (val.at(field).get<std::string>() == value) {
            return true;
        }
    }
    return false;
}

class TestHA : public TuGraphTest {
 protected:
    void SetUp() override {
        build_so("./add_vertex_v.so", "../../test/test_procedures/add_vertex_v.cpp");
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
            "true --enable_ha true --ha_node_offline_ms 5000 "
            "--ha_node_remove_ms 10000 --ha_snapshot_interval_s -1 "
            "--rpc_port {} --directory ./db --log_dir "
            "./log  --ha_conf {} --verbose 1 -c lgraph_ha.json -d start";
#else
        std::string cmd_f =
            "mkdir {} && cp -r ../../src/server/lgraph_ha.json "
            "./lgraph_server ./resource {} "
            "&& cd {} && ./lgraph_server --host {} --port {} --enable_rpc "
            "true --enable_ha true --ha_node_offline_ms 5000 "
            "--ha_node_remove_ms 10000 --ha_snapshot_interval_s -1 "
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

TEST_F(TestHA, HAClient) {
    std::string schema =
        "{\"schema\" :\n"
        "[  \n"
        "{ \n"
        "\"label\" : \"Person\",             \n"
        "\"type\" : \"VERTEX\",             \n"
        "\"primary\" : \"name\",             \n"
        "\"properties\" : [                 \n"
        "    {\"name\" : \"name\", \"type\":\"STRING\"},                 \n"
        "    {\"name\" : \"birthyear\", \"type\":\"INT16\", "
        "\"optional\":true},                 \n"
        "    {\"name\" : \"phone\", \"type\":\"INT16\",\"unique\":true, "
        "\"index\":true}             \n"
        "]         \n"
        "                    },        \n"
        "                    {            \n"
        "\"label\" : \"Film\",            \n"
        "\"type\" : \"VERTEX\",            \n"
        "\"primary\" : \"title\",            \n"
        "\"properties\" : [                \n"
        "    {\"name\" : \"title\", \"type\":\"STRING\"}             \n"
        "]        \n"
        "                    },       \n"
        "                    {\t        \n"
        "\"label\": \"PLAY_IN\",\t        \n"
        "\"type\": \"EDGE\",\t        \n"
        "\"properties\": [\n"
        "    {\"name\": \"role\", \"type\": \"STRING\", \"optional\": "
        "true}\n"
        "],\t        \n"
        "\"constraints\": [ [\"Person\", \"Film\"] ]       \n"
        "}    \n"
        "]\n"
        "}";
    std::string data_desc =
        "{\"files\": [    \n"
        "   {        \n"
        "       \"columns\": [\"name\", \"birthyear\", \"phone\"],\n"
        "       \"format\": \"CSV\",        \n"
        "       \"header\": 0,        \n"
        "       \"label\": \"Person\"    \n"
        "   }]\n"
        "}";
    std::string data_person =
        "Rachel Kempson,1910,10086\n"
        "Michael Redgrave,1908,10087\n"
        "Vanessa Redgrave,1937,10088\n"
        "Corin Redgrave,1939,10089\n"
        "Liam Neeson,1952,10090\n"
        "Natasha Richardson,1963,10091\n"
        "Richard Harris,1930,10092\n"
        "Dennis Quaid,1954,10093\n"
        "Lindsay Lohan,1986,10094\n"
        "Jemma Redgrave,1965,10095\n"
        "Roy Redgrave,1873,10096\n"
        "John Williams,1932,10097\n"
        "Christopher Nolan,1970,10098";
    build_so("./sortstr.so", "../../test/test_procedures/sortstr.cpp");
    lgraph::RpcClient client(this->host + ":29092", "admin", "73@TuGraph");
    std::string result;
    bool ret = client.CallCypher(result, "CALL db.dropDB()");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(result, "CALL dbms.procedures()");
    UT_EXPECT_TRUE(ret);

    ret = client.ListProcedures(result, "CPP");
    UT_EXPECT_TRUE(ret);

    ret = client.ImportSchemaFromContent(result, schema) &&
          client.ImportDataFromContent(result, data_desc, data_person, ",");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypherToLeader(result, "MATCH (n) RETURN COUNT(n)");
    UT_EXPECT_TRUE(ret);
    nlohmann::json res = nlohmann::json::parse(result);
    UT_EXPECT_EQ(res[0]["COUNT(n)"], 13);

    ret = client.CallGqlToLeader(result, "MATCH (n) RETURN COUNT(n)");
    UT_EXPECT_TRUE(ret);
    res = nlohmann::json::parse(result);
    UT_EXPECT_EQ(res[0]["COUNT(n)"], 13);

    std::string code_so_path = "./sortstr.so";
    ret = client.LoadProcedure(result, code_so_path, "CPP", "test_plugin1", "SO",
                               "this is a test plugin", true, "v1");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypherToLeader(result,
                                    "CALL db.plugin.getPluginInfo('CPP','test_plugin1', false)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallProcedureToLeader(result, "CPP", "test_plugin1", "gecfb");
    UT_EXPECT_TRUE(ret);
    nlohmann::json json_val = nlohmann::json::parse(result);
    UT_EXPECT_TRUE(HasElement(json_val, "bcefg", "result"));

    client.CallCypher(result, "CALL db.dropDB()");
    client.Logout();
}

TEST_F(TestHA, HAConsistency) {
    std::string cmd_f = "cd {} && ./lgraph_server -c lgraph_ha.json -d stop";
    std::string cmd = FMA_FMT(cmd_f.c_str(), "ha3");
    int rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    fma_common::SleepS(5);
    lgraph::RpcClient rpcClient(this->host + ":29092", "admin", "73@TuGraph");
    std::string result;
    rpcClient.LoadProcedure(result, "add_vertex_v.so", "CPP", "add_vertex_v", "SO", "", false);
    rpcClient.CallProcedure(result, "CPP", "add_vertex_v", "{}");
#ifndef __SANITIZE_ADDRESS__
    cmd_f =
        "cd {} && ./lgraph_server --host {} --port {} --enable_rpc "
        "true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
        "--ha_snapshot_interval_s -1 --rpc_port {} --directory ./db --log_dir "
        "./log  --ha_conf {} -c lgraph_ha.json -d start";
#else
    cmd_f =
        "cd {} && ./lgraph_server --host {} --port {} --enable_rpc "
        "true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
        "--ha_snapshot_interval_s -1 --rpc_port {} --directory ./db --log_dir "
        "./log  --ha_conf {} --use_pthread 1 --verbose 1 -c lgraph_ha.json -d start";
#endif
    cmd = FMA_FMT(cmd_f.c_str(), "ha3", host, "27074", "29094",
                  host + ":29092," + host + ":29093," + host + ":29094");
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    fma_common::SleepS(30);
    rpcClient.CallCypher(result, "MATCH (n) RETURN COUNT(n)", "default", true, 0, host + ":29094");
    nlohmann::json res = nlohmann::json::parse(result);
    UT_EXPECT_EQ(res[0]["COUNT(n)"], 300000);
    rpcClient.Logout();
}
