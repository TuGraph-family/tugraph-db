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
            std::make_unique<lgraph::HNSW>("label", "name", "l2", "hnsw", dim, index_spec);
    }
    void TearDown() override {}
};

TEST_F(TestVsag, AddVectors) {
    EXPECT_NO_THROW(vector_index->Add(vectors, vids));
}

TEST_F(TestVsag, AddVectorsException) {
    EXPECT_NO_THROW(vector_index->Add(vectors, vids));
    UT_EXPECT_THROW_CODE(vector_index->Add(vectors, vids), VectorIndexException);
}

TEST_F(TestVsag, SearchIndex) {
    EXPECT_NO_THROW(vector_index->Add(vectors, vids));
    std::vector<float> query(vectors[0].begin(), vectors[0].end());
    std::vector<std::pair<int64_t, float>> ret;
    ret = vector_index->KnnSearch(query, 10, 10);
    ASSERT_TRUE(!ret.empty());
    ASSERT_EQ(ret[0].first, vids[0]);
}

/*
TEST_F(TestVsag, SaveAndLoadIndex) {
    EXPECT_NO_THROW(vector_index->Add(vectors, vids));
    std::vector<uint8_t> serialized_index = vector_index->Save();
    ASSERT_FALSE(serialized_index.empty());
    lgraph::HNSW vector_index_loaded("label", "name", "l2", "hnsw", dim, index_spec);
    vector_index_loaded.Load(serialized_index);
    std::vector<float> query(vectors[0].begin(), vectors[0].end());
    auto ret = vector_index_loaded.KnnSearch(query, 10, 10);
    ASSERT_TRUE(!ret.empty());
    ASSERT_EQ(ret[0].first, vids[0]);
}
*/

TEST_F(TestVsag, DeleteVectors) {
    EXPECT_NO_THROW(vector_index->Add(vectors, vids));
    std::vector<int64_t> delete_vids = {vids[0], vids[1]};
    EXPECT_NO_THROW(vector_index->Remove(delete_vids));
    std::vector<float> query(vectors[0].begin(), vectors[0].end());
    auto ret = vector_index->KnnSearch(query, 10, 10);
    for (const auto& pair : ret) {
        ASSERT_TRUE(std::find(delete_vids.begin(),
                        delete_vids.end(), pair.first) == delete_vids.end());
    }
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

TEST_F(TestVsag, restart) {
    using namespace lgraph;
    lgraph::GlobalConfig conf;
    conf.db_dir = "./testdb";
    conf.http_port = 7774;
    conf.enable_rpc = true;
    conf.rpc_port = 9394;
    conf.bind_host = "127.0.0.1";
#ifdef __SANITIZE_ADDRESS__
    conf.use_pthread = true;
#endif
    AutoCleanDir cleaner(conf.db_dir);
    {
        auto server = StartLGraphServer(conf);
        // create graphs
        RpcClient client(UT_FMT("{}:{}", conf.bind_host, conf.rpc_port),
                         _detail::DEFAULT_ADMIN_NAME, _detail::DEFAULT_ADMIN_PASS);
        UT_LOG() << "test AddVectorIndex , DeleteVectorIndex , ShowVectorIndex , VectorIndexQuery";
        std::string str;
        // vector.AddVectorIndex test
        bool ret =
            client.CallCypher(str,
                              "CALL db.createVertexLabel("
                              "'person','id','id','int64',false,'vector','float_vector',true)");
        UT_EXPECT_TRUE(ret);
        ret = client.CallCypher(
            str, "CALL db.addVertexVectorIndex('person','vector', {dimension:4})");
        UT_EXPECT_TRUE(ret);
        ret = client.CallCypher(str, "CREATE (n:person {id:1, vector: [1.0,1.0,1.0,1.0]})");
        UT_EXPECT_TRUE(ret);
        ret = client.CallCypher(str, "CREATE (n:person {id:2, vector: [2.0,2.0,2.0,2.0]})");
        UT_EXPECT_TRUE(ret);
        ret = client.CallCypher(str,
                                "CALL db.upsertVertex('person', [{id:3, vector: [3.0,3.0,3.0,3.0]},"
                                "{id:4, vector: [4.0,4.0,4.0,4.0]}])");
        UT_EXPECT_TRUE(ret);
        ret = client.CallCypher(str,"CALL db.vertexVectorKnnSearch"  //NOLINT
        "('person','vector',[1,2,3,4], {top_k:4, hnsw_ef_search:10}) YIELD node RETURN node.id");
        UT_EXPECT_EQ(str, R"([{"node.id":2},{"node.id":3},{"node.id":1},{"node.id":4}])");
        UT_EXPECT_TRUE(ret);
        server->Kill();
        server->Wait();
    }
    {
        auto server = StartLGraphServer(conf);
        // create graphs
        RpcClient client(UT_FMT("{}:{}", conf.bind_host, conf.rpc_port),
                         _detail::DEFAULT_ADMIN_NAME, _detail::DEFAULT_ADMIN_PASS);
        std::string str;
        auto ret = client.CallCypher(str, "CALL db.vertexVectorKnnSearch"
                                     "('person','vector',[1,2,3,4], {top_k:4, hnsw_ef_search:10}) "
                                     "YIELD node RETURN node.id");
        UT_EXPECT_EQ(str, R"([{"node.id":2},{"node.id":3},{"node.id":1},{"node.id":4}])");
        UT_EXPECT_TRUE(ret);
        ret = client.CallCypher(str, "CALL db.alterLabelDelFields('vertex', 'person', ['vector'])");
        UT_EXPECT_TRUE(ret);
        server->Kill();
        server->Wait();
    }
    {
        auto server = StartLGraphServer(conf);
        // create graphs
        RpcClient client(UT_FMT("{}:{}", conf.bind_host, conf.rpc_port),
                         _detail::DEFAULT_ADMIN_NAME, _detail::DEFAULT_ADMIN_PASS);
        std::string str;
        auto ret = client.CallCypher(str, "CALL db.vertexVectorKnnSearch"
                                     "('person','vector',[1,2,3,4], 4, 10) "
                                     "YIELD node RETURN node.id");
        UT_EXPECT_FALSE(ret);
        server->Kill();
        server->Wait();
    }
}

TEST_F(TestVsag, error) {
    using namespace lgraph;
    lgraph::GlobalConfig conf;
    conf.db_dir = "./testdb";
    conf.http_port = 7774;
    conf.enable_rpc = true;
    conf.rpc_port = 9394;
    conf.bind_host = "127.0.0.1";
#ifdef __SANITIZE_ADDRESS__
    conf.use_pthread = true;
#endif

    AutoCleanDir cleaner(conf.db_dir);
    auto server = StartLGraphServer(conf);
    // create graphs
    RpcClient client(UT_FMT("{}:{}", conf.bind_host, conf.rpc_port),
                     _detail::DEFAULT_ADMIN_NAME, _detail::DEFAULT_ADMIN_PASS);
    UT_LOG() << "test AddVectorIndex , DeleteVectorIndex , ShowVectorIndex , VectorIndexQuery";
    std::string str;
    // vector.AddVectorIndex test
    bool ret = client.CallCypher(str,
                                 "CALL db.createVertexLabel("
                                 "'person','id','id','int64',false,'vector','float_vector',true)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL db.addVertexVectorIndex('person','vector', {dimension:4})");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CREATE (n:person {id:1, vector: [1.0,1.0,1.0,1.0]})");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CREATE (n:person {id:2, vector: [1.0,1.0,1.0]})");
    UT_EXPECT_FALSE(ret);
    UT_EXPECT_TRUE(str.find("dimension mismatch") != std::string::npos);
    ret = client.CallCypher(str, R"(CALL db.upsertVertex('person', [{id:3, vector: [4.0,4.0,4.0]}]))");
    UT_EXPECT_FALSE(ret);
    UT_EXPECT_TRUE(str.find("dimension mismatch") != std::string::npos);
    server->Kill();
    server->Wait();
}

TEST_F(TestVsag, VectorProcedure) {
    using namespace lgraph;
    lgraph::GlobalConfig conf;
    conf.db_dir = "./testdb";
    conf.http_port = 7774;
    conf.enable_rpc = true;
    conf.rpc_port = 9394;
    conf.bind_host = "127.0.0.1";
#ifdef __SANITIZE_ADDRESS__
    conf.use_pthread = true;
#endif

    AutoCleanDir cleaner(conf.db_dir);
    auto server = StartLGraphServer(conf);
    // create graphs
    RpcClient client(UT_FMT("{}:{}", conf.bind_host, conf.rpc_port),
                     _detail::DEFAULT_ADMIN_NAME, _detail::DEFAULT_ADMIN_PASS);
    UT_LOG() << "test AddVectorIndex , DeleteVectorIndex , ShowVectorIndex , VectorIndexQuery";
    std::string str;
    // vector.AddVectorIndex test
    bool ret = client.CallCypher(str,
                                 "CALL db.createVertexLabel("
                                 "'person','id','id','int64',false,'vector','float_vector',true)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL db.addVertexVectorIndex('person','vector', {dimension:4})");
    UT_EXPECT_TRUE(ret);

    // vector.ShowVectorIndex test
    ret = client.CallCypher(str,
                            "CALL db.showVertexVectorIndex()");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_TRUE(ret);
    UT_EXPECT_EQ(ElementCount_vector(json_val, "person", "label_name"), 1);

    // vector.DeleteVectorIndex test
    ret = client.CallCypher(str,
                            "CALL db.deleteVertexVectorIndex('person','vector')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL db.addVertexVectorIndex('person','vector',{dimension:4})");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL db.showVertexVectorIndex()");
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
    ret = client.CallCypher(
        str,"CALL db.vertexVectorKnnSearch('person','vector',[1,2,3,4], " // NOLINT
        "{top_k:2, hnsw_ef_search:10})");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val.size(), 2);
    ret = client.CallCypher(str,
    "CALL db.vertexVectorKnnSearch('person','vector',[1,2,3,4], "
                            "{top_k:2, hnsw_ef_search:10}) YIELD node");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["node"]["identity"], 1);
    UT_EXPECT_EQ(json_val[1]["node"]["identity"], 2);
    ret = client.CallCypher(str,
    "CALL db.vertexVectorKnnSearch('person','vector',[1,2,3,4], "
                            "{top_k:2, hnsw_ef_search:10}) YIELD node RETURN node.id");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_TRUE(ret);
    UT_EXPECT_EQ(json_val[0]["node.id"], 2);
    UT_EXPECT_EQ(json_val[1]["node.id"], 3);
    ret = client.CallCypher(str, "CALL db.dropDB");
    UT_EXPECT_TRUE(ret);
    server->Kill();
    server->Wait();
}
