/* Copyright (c) 2022 AntGroup. All Rights Reserved. */
#include "gtest/gtest.h"
#include "./ut_utils.h"
#include "lgraph/lgraph_rpc_client.h"
#include "fma-common/configuration.h"
#include "boost/algorithm/string.hpp"
#include "client/cpp/rpc/rpc_exception.h"

const char ha_mkdir[] = "mkdir {} && cp -r ../../src/server/lgraph_ha.json "
    "./lgraph_server ./resource {}";
#ifndef __SANITIZE_ADDRESS__
const char server_cmd_f[] =
    "cd {} && ./lgraph_server --host {} --port {} --enable_rpc "
    "true --enable_ha true --ha_snapshot_interval_s -1 --ha_node_join_group_s 60 "
    "--rpc_port {} --enable_plugin 1 --directory ./db --log_dir "
    "./log --ha_conf {} --verbose 1 -c lgraph_ha.json -d start";
const char witness_cmd_f[] =
    "cd {} && ./lgraph_server --host {} --port {} --enable_rpc "
    "true --enable_ha true --ha_is_witness 1 "
    "--ha_snapshot_interval_s -1 --ha_node_join_group_s 60 "
    "--rpc_port {} --enable_plugin 1 --directory ./db --log_dir "
    "./log --ha_conf {} --verbose 1 "
    "-c lgraph_ha.json --ha_enable_witness_to_leader {} -d start";
#else
const char server_cmd_f[] =
    "cd {} && ./lgraph_server --host {} --port {} --enable_rpc "
    "true --enable_ha true --ha_snapshot_interval_s -1 "
    "--ha_node_join_group_s 60 --rpc_port {} --directory ./db --log_dir "
    "./log --enable_plugin 1  --ha_conf {} --use_pthread 1 --verbose 1 -c lgraph_ha.json -d start";
const char witness_cmd_f[] =
    "cd {} && ./lgraph_server --host {} --port {} --enable_rpc "
    "true --enable_ha true --enable_plugin 1 --ha_is_witness 1 --ha_snapshot_interval_s -1 "
    "--ha_node_join_group_s 60 --rpc_port {} --directory ./db --log_dir "
    "./log --ha_conf {} --use_pthread 1 --verbose 1 -c lgraph_ha.json "
    "--ha_enable_witness_to_leader {} -d start";
#endif

void start_server(const std::string &host, bool enable_witness_leader = false) {
    int rt;
    std::string cmd = FMA_FMT(ha_mkdir, "ha1", "ha1");
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    cmd = FMA_FMT(server_cmd_f, "ha1", host, "27072", "29092",
                              host + ":29092," + host + ":29093," + host + ":29094");
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    fma_common::SleepS(5);

    cmd = FMA_FMT(ha_mkdir, "ha2", "ha2");
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    cmd = FMA_FMT(server_cmd_f, "ha2", host, "27073", "29093",
                  host + ":29092," + host + ":29093," + host + ":29094");
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    fma_common::SleepS(5);

    cmd = FMA_FMT(ha_mkdir, "ha3", "ha3");
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    cmd = FMA_FMT(witness_cmd_f, "ha3", host, "27074", "29094",
                  host + ":29092," + host + ":29093," + host + ":29094",
                  enable_witness_leader);
    UT_EXPECT_EQ(system(cmd.c_str()), 0);
    fma_common::SleepS(15);
}

class TestHAWitness : public TuGraphTest {
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
    }

    void TearDown() override {
        std::string cmd_f = "cd {} && ./lgraph_server -c lgraph_ha.json -d stop";
        for (int i = 1; i < 4; ++i) {
            std::string dir = "ha" + std::to_string(i);
            std::string cmd = FMA_FMT(cmd_f.c_str(), dir);
            system(cmd.c_str());
            cmd = FMA_FMT("rm -rf {}", dir);
            int rt = system(cmd.c_str());
            UT_EXPECT_EQ(rt, 0);
        }
        int rt = system("rm -rf sortstr.so");
        UT_EXPECT_EQ(rt, 0);
    }

 public:
    std::string host;
    const std::string schema =
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
        "                    }        \n"
        "]\n"
        "}";
    const std::string data_desc =
        "{\"files\": [    \n"
        "   {        \n"
        "       \"columns\": [\"name\", \"birthyear\", \"phone\"],\n"
        "       \"format\": \"CSV\",        \n"
        "       \"header\": 0,        \n"
        "       \"label\": \"Person\"    \n"
        "   }]\n"
        "}";
    const std::string data_person =
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
};

TEST_F(TestHAWitness, HAWitness) {
    // 1 leader + 1 follower + 1 witness (disable to leader)
    start_server(this->host);
    build_so("./sortstr.so", "../../test/test_procedures/sortstr.cpp");
    lgraph::RpcClient client(this->host + ":29092", "admin", "73@TuGraph");
    std::string result;
    bool ret = client.CallCypher(result, "CALL db.dropDB()");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(result, "CALL dbms.ha.clusterInfo()");
    UT_EXPECT_TRUE(ret);
    auto check_witness = [&result, this] () {
        nlohmann::json cluster_info = nlohmann::json::parse(result.c_str());
        for (auto &node : cluster_info[0]["cluster_info"]) {
            if (node["ha_role"] == "WITNESS" && node["rpc_address"] == this->host + ":29094") {
                return true;
            }
        }
        return false;
    };

    // check witness url
    UT_EXPECT_TRUE(check_witness());
    // check client interface & server ability
    UT_EXPECT_FALSE(client.CallCypher(result, "CALL dbms.ha.clusterInfo()",
                                          "default", true, 0, this->host + ":29094"));

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

TEST_F(TestHAWitness, HAWitnessDisableLeader) {
    start_server(this->host, true);
    build_so("./sortstr.so", "../../test/test_procedures/sortstr.cpp");
    std::unique_ptr<lgraph::RpcClient> client = std::make_unique<lgraph::RpcClient>(
        this->host + ":29094", "admin", "73@TuGraph");
    std::string result, code_so_path = "./sortstr.so";
    bool ret = client->LoadProcedure(result, code_so_path, "CPP", "test_plugin1", "SO",
                               "this is a test plugin", true, "v1");
    UT_EXPECT_TRUE(ret);
    ret = client->CallCypher(result, "CALL dbms.ha.clusterInfo()");
    UT_EXPECT_TRUE(ret);
    std::string master_rpc, follower_rpc;
    nlohmann::json cluster_info = nlohmann::json::parse(result.c_str());
    for (auto &node : cluster_info[0]["cluster_info"]) {
        if (node["state"] == "MASTER") {
            master_rpc = to_string(node["rpc_address"]);
            std::vector<std::string> output;
            boost::split(output, master_rpc, boost::is_any_of("\\\""));
            master_rpc = output[1];
        } else if (node["state"] == "FOLLOW" && node["ha_role"] != "WITNESS") {
            follower_rpc = to_string(node["rpc_address"]);
            std::vector<std::string> output;
            boost::split(output, follower_rpc, boost::is_any_of("\\\""));
            follower_rpc = output[1];
        }
    }
    // stop follower
    std::string cmd_f = "cd {} && ./lgraph_server -c lgraph_ha.json -d stop";
    std::vector<std::string> output;
    boost::split(output, follower_rpc, boost::is_any_of(":"));
    std::string cmd = FMA_FMT(cmd_f.c_str(), "ha" + std::to_string(stoi(output[1]) - 29091));
    ret = system(cmd.c_str());
    UT_EXPECT_EQ(ret, 0);
    fma_common::SleepS(15);

    // apply something
    client = std::make_unique<lgraph::RpcClient>(this->host + ":29094", "admin", "73@TuGraph");
    ret = client->ImportSchemaFromContent(result, schema) &&
          client->ImportDataFromContent(result, data_desc, data_person, ",");
    UT_EXPECT_TRUE(ret);
    client->Logout();

    // stop leader
    boost::split(output, master_rpc, boost::is_any_of(":"));
    cmd = FMA_FMT(cmd_f.c_str(), "ha" + std::to_string(stoi(output[1]) - 29091));
    ret = system(cmd.c_str());
    UT_EXPECT_EQ(ret, 0);
    fma_common::SleepS(15);

    // start follower, witness which has newer log will be leader temporary
    boost::split(output, follower_rpc, boost::is_any_of(":"));
    std::string ha_dir = "ha" + std::to_string(stoi(output[1]) - 29091);
    cmd = FMA_FMT(server_cmd_f, ha_dir, host, stoi(output[1]) - 2020,
                  output[1], host + ":29092," + host + ":29093," + host + ":29094");
    ret = system(cmd.c_str());
    UT_EXPECT_EQ(ret, 0);
    fma_common::SleepS(60);
    master_rpc.clear();
    int times = 0;
    do {
        try {
            client = std::make_unique<lgraph::RpcClient>(
                follower_rpc, "admin", "73@TuGraph");
            client->CallCypher(result, "CALL dbms.ha.clusterInfo()");
            cluster_info = nlohmann::json::parse(result.c_str());
            if (cluster_info == nullptr || cluster_info[0]["cluster_info"] == nullptr)
                throw lgraph::RpcException("no cluster info");
            for (auto &node : cluster_info[0]["cluster_info"]) {
                if (node["state"] == "MASTER" && node["ha_role"] != "WITNESS") {
                    master_rpc = to_string(node["rpc_address"]);
                    break;
                }
            }
            if (!master_rpc.empty()) {
                break;
            } else {
                client->Logout();
                fma_common::SleepS(1);
            }
        } catch (std::exception &e) {
            LOG_WARN() << e.what();
            fma_common::SleepS(1);
            continue;
        }
    } while (++times < 120);
    ret = client->CallCypherToLeader(result, "MATCH (n) RETURN COUNT(n)");
    UT_EXPECT_TRUE(ret);
    nlohmann::json res = nlohmann::json::parse(result);
    UT_EXPECT_EQ(res[0]["COUNT(n)"], 13);
    client->Logout();
}
