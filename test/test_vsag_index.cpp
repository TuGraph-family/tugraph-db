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
#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include <boost/lexical_cast.hpp>

#include "./ut_utils.h"
#include "fma-common/configuration.h"
#include "restful/server/stdafx.h"
#include "restful/server/rest_server.h"
#include "./graph_factory.h"
#include "fma-common/string_formatter.h"
#include "brpc/closure_guard.h"
#include "brpc/server.h"
#include "lgraph/lgraph_rpc_client.h"
#include "./test_tools.h"

#include "gtest/gtest.h"
#include "core/vsag_hnsw.h"

using namespace utility;               // Common utilities like string conversions
using namespace concurrency::streams;  // Asynchronous streams

class TestVsag : public TuGraphTest {
 protected:
    int64_t dim = 4;
    int64_t num_vectors = 10000;
    std::vector<std::vector<float>> vectors;
    std::vector<int64_t> vids;
    std::unique_ptr<lgraph::HNSW> vector_index;
    std::vector<int> index_spec = {24, 100};
    TestVsag() : vectors(num_vectors, std::vector<float>(dim)), vids(num_vectors) {}
    void SetUp() override {
        std::mt19937 rng(47);
        std::uniform_real_distribution<> distrib_real;
        for (int64_t i = 0; i < num_vectors; ++i) {
            vids[i] = i;
            for (int j = 0; j < dim; ++j) {
                vectors[i][j] = distrib_real(rng);
            }
        }
        vector_index =
            std::make_unique<lgraph::HNSW>("label", "name", "L2", "HNSW", dim, index_spec);
    }
    void TearDown() override {}
};

TEST_F(TestVsag, BuildIndex) { ASSERT_TRUE(vector_index->Build()); }

TEST_F(TestVsag, AddVectors) {
    ASSERT_TRUE(vector_index->Build());
    ASSERT_TRUE(vector_index->Add(vectors, vids, num_vectors));
}

TEST_F(TestVsag, SearchIndex) {
    ASSERT_TRUE(vector_index->Build());
    ASSERT_TRUE(vector_index->Add(vectors, vids, num_vectors));
    std::vector<float> query(vectors[0].begin(), vectors[0].end());
    std::vector<float> distances;
    std::vector<int64_t> indices;
    ASSERT_TRUE(vector_index->Search(query, 10, distances, indices));
    ASSERT_EQ(indices[0], vids[0]);
}

TEST_F(TestVsag, SaveAndLoadIndex) {
    ASSERT_TRUE(vector_index->Build());
    ASSERT_TRUE(vector_index->Add(vectors, vids, num_vectors));
    std::vector<uint8_t> serialized_index = vector_index->Save();
    ASSERT_FALSE(serialized_index.empty());
    lgraph::HNSW vector_index_loaded("label", "name", "L2", "HNSW", dim, index_spec);
    ASSERT_TRUE(vector_index_loaded.Build());
    vector_index_loaded.Load(serialized_index);
    std::vector<float> query(vectors[0].begin(), vectors[0].end());
    std::vector<float> distances;
    std::vector<int64_t> indices;
    ASSERT_TRUE(vector_index_loaded.Search(query, 10, distances, indices));
    ASSERT_EQ(indices[0], vids[0]);
}

TEST_F(TestVsag, DeleteVectors) {
    ASSERT_TRUE(vector_index->Build());
    ASSERT_TRUE(vector_index->Add(vectors, vids, num_vectors));
    std::vector<int64_t> delete_vids = {vids[0], vids[1]};
    ASSERT_TRUE(vector_index->Add({}, delete_vids, 0));
    std::vector<float> query(vectors[0].begin(), vectors[0].end());
    std::vector<float> distances;
    std::vector<int64_t> indices;
    ASSERT_TRUE(vector_index->Search(query, 10, distances, indices));
    for (const auto& idx : indices) {
        ASSERT_TRUE(std::find(delete_vids.begin(),
                        delete_vids.end(), idx) == delete_vids.end());
    }
}

std::mutex lock_rpc_vector;
std::condition_variable cond_vector;
int stage_3_vector = 0;
lgraph::StateMachine::Config sm_config_vector;
std::shared_ptr<lgraph::GlobalConfig> gconfig_vector = std::make_shared<lgraph::GlobalConfig>();
lgraph::StateMachine* ptr_state_machine_vector;
extern void build_so();

brpc::Server rpc_server_vector;
struct Config : public lgraph::StateMachine::Config, public lgraph::RestServer::Config {
    int verbose = 1;
    std::string log_file;
    bool enable_ha = false;
    bool enable_rpc = true;
    int thread_limit = 0;
};
class RPCService : public lgraph::LGraphRPCService {
 public:
    explicit RPCService(lgraph::StateMachine* sm) : sm_(sm) {}

    void HandleRequest(::google::protobuf::RpcController* controller,
                       const ::lgraph::LGraphRequest* request, ::lgraph::LGraphResponse* response,
                       ::google::protobuf::Closure* done) {
        sm_->HandleRequest(controller, request, response, done);
    }

 private:
    lgraph::StateMachine* sm_;
};
RPCService* ptr_rpc_service_vector;

void on_initialize_rpc_server_vector() {
    using namespace fma_common;
    using namespace lgraph;
    bool enable_ssl = true;
    std::string host = "127.0.0.1";
    uint16_t port = 6464;
    std::string db_name = "default";
    {
        Configuration config;
        config.Add(enable_ssl, "ssl", true).Comment("Enable SSL");
        config.Add(host, "host", true).Comment("Host address");
        config.Add(port, "port", true).Comment("HTTP port");
        config.Add(gconfig_vector->enable_ip_check,
                    "enable_ip_check", true).Comment("Enable IP check.");
    }

    { GraphFactory::create_modern(); }
    // Build listener's URI from the configured address and the hard-coded path "RestServer/Action"
    sm_config_vector.db_dir = "./testdb_vector";
    sm_config_vector.rpc_port = 19089;

    gconfig_vector->ft_index_options.enable_fulltext_index = true;
    lgraph::AccessControlledDB::SetEnablePlugin(true);
    ptr_state_machine_vector = new lgraph::StateMachine(sm_config_vector, gconfig_vector);
    ptr_rpc_service_vector = new RPCService(ptr_state_machine_vector);

    rpc_server_vector.AddService(ptr_rpc_service_vector, brpc::SERVER_DOESNT_OWN_SERVICE);
    rpc_server_vector.Start(sm_config_vector.rpc_port, nullptr);

    ptr_state_machine_vector->Start();
}

void on_shutdown_rpc_server_vector() {
    try {
        ptr_state_machine_vector->Stop();
    } catch (std::exception& e) {
        LOG_ERROR() << "Rest server shutdown failed: " << e.what();
    }
    rpc_server_vector.Stop(0);
}

void* test_vector_rpc_server(void*) {
    std::unique_lock<std::mutex> l(lock_rpc_vector);
    if (stage_3_vector == 0) {
        on_initialize_rpc_server_vector();
        UT_LOG() << "rpc server is running";
        stage_3_vector++;
        cond_vector.notify_one();
    }
    if (stage_3_vector != 2) cond_vector.wait(l);
    on_shutdown_rpc_server_vector();
    UT_LOG() << __func__ << " thread exit";
    delete ptr_state_machine_vector;
    delete ptr_rpc_service_vector;
    return nullptr;
}

int ElementCount_vector(const web::json::value& val,
                        const std::string& value, const std::string& field) {
    int count = 0;
    if (val.is_array()) {
        for (int i = 0; i < val.size(); ++i) {
            if (val.at(i).has_field(field)) {
                if (val.at(i).at(field).as_string() == value) {
                    ++count;
                }
            }
        }
    } else if (val.is_object()) {
        if (val.has_field(field)) {
            if (val.at(field).as_string() == value) {
                ++count;
            }
        }
    }
    return count;
}

void test_vector_index(lgraph::RpcClient& client) {
    UT_LOG() << "test AddVectorIndex , DeleteVectorIndex , ShowVectorIndex , VectorIndexQuery";
    std::string str;
    // vector.AddVectorIndex test
    bool ret = client.CallCypher(str,
    "CALL db.createVertexLabel('person','id','id','int64',false,'vector','float_vector',true)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
    "CALL vector.AddVectorIndex('person','vector','HNSW',4,'L2',24,100)");
    UT_EXPECT_TRUE(ret);

    // vector.ShowVectorIndex test
    ret = client.CallCypher(str,
    "CALL vector.ShowVectorIndex()");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_TRUE(ret);
    UT_EXPECT_EQ(ElementCount_vector(json_val, "person", "label_name"), 1);

    // vector.DeleteVectorIndex test
    ret = client.CallCypher(str,
    "CALL vector.DeleteVectorIndex('person','vector','HNSW',4,'L2')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
    "CALL vector.AddVectorIndex('person','vector','HNSW',4,'L2',24,100)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
    "CALL vector.ShowVectorIndex()");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_TRUE(ret);
    UT_EXPECT_EQ(ElementCount_vector(json_val, "person", "label_name"), 1);

    // vector.VectorIndexQuery test
    ret = client.CallCypher(str,
    "CREATE (n:person {id:1, vector: [1.0,1.0,1.0,1.0]})");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
    "CREATE (n:person {id:2, vector: [2.0,2.0,2.0,2.0]})");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
    R"(CALL db.upsertVertexByJson('person', '[{"id":3, "vector": [3,3,3,3]},
            {"id":4, "vector": [4,4,4,4]}]'))");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
    "CALL db.upsertVertex('person', [{id:5, vector: [5.0,5.0,5.0,5.0]},"
                            "{id:6, vector: [6.0,6.0,6.0,6.0]}])");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
    "CALL vector.VectorIndexQuery('person','vector',[1,2,3,4], 2, 10)");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_TRUE(ret);
    UT_EXPECT_EQ(ElementCount_vector(json_val,
    "3.000000,3.000000,3.000000,3.000000", "vec"), 1);
    UT_EXPECT_EQ(ElementCount_vector(json_val,
    "2.000000,2.000000,2.000000,2.000000", "vec"), 1);
    ret = client.CallCypher(str,
         "CALL vector.VectorIndexQuery('person','vector',[1,2,3,4], 2, 10) YIELD vid");
    UT_EXPECT_TRUE(ret);
    UT_EXPECT_EQ(json_val[0]["vid"], 1);
    UT_EXPECT_EQ(json_val[1]["vid"], 2);
    ret = client.CallCypher(str, "CALL db.dropDB");
    UT_EXPECT_TRUE(ret);
}

void* test_vector_rpc_client(void*) {
    using namespace lgraph;
    std::unique_lock<std::mutex> l(lock_rpc_vector);
    if (stage_3_vector == 0) cond_vector.wait(l);
    // start test user login
    UT_LOG() << "admin user login";
    {
        RpcClient client3("0.0.0.0:19089", "admin", "73@TuGraph");
        test_vector_index(client3);
    }

    stage_3_vector++;
    cond_vector.notify_one();
    UT_LOG() << __func__ << " thread exit";
    return nullptr;
}

TEST_F(TestVsag, VectorProcedure) {
    // fma_common::Logger::Get().SetLevel(fma_common::LogLevel::LL_DEBUG);
    std::thread tid_https[2] = {std::thread(test_vector_rpc_server, nullptr),
                                std::thread(test_vector_rpc_client, nullptr)};
    tid_https[0].join();
    tid_https[1].join();
}
