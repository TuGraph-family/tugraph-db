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
#include "core/Vsag_HNSW.h"

class VSAGTest : public ::testing::Test {
 protected:
    int64_t dim = 4;
    int64_t num_vectors = 10000;
    std::vector<std::vector<float>> vectors;
    std::vector<size_t> vids;
    std::unique_ptr<lgraph::HNSW> vector_index;
    std::vector<int> index_spec = {24, 100};
    VSAGTest() : vectors(num_vectors, std::vector<float>(dim)), vids(num_vectors) {}
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
            std::make_unique<lgraph::HNSW>("label", "name",
                            "L2", "HNSW", dim, index_spec, nullptr);
    }
    void TearDown() override {}
};

TEST_F(VSAGTest, BuildIndex) { ASSERT_TRUE(vector_index->Build()); }

TEST_F(VSAGTest, AddVectors) {
    ASSERT_TRUE(vector_index->Build());
    ASSERT_TRUE(vector_index->Add(vectors, vids, num_vectors));
}

TEST_F(VSAGTest, SearchIndex) {
    ASSERT_TRUE(vector_index->Build());
    ASSERT_TRUE(vector_index->Add(vectors, vids, num_vectors));
    std::vector<float> query(vectors[0].begin(), vectors[0].end());
    std::vector<float> distances;
    std::vector<int64_t> indices;
    ASSERT_TRUE(vector_index->Search(query, 10, distances, indices));
    ASSERT_EQ(indices[0], vids[0]);
}

TEST_F(VSAGTest, SaveAndLoadIndex) {
    ASSERT_TRUE(vector_index->Build());
    ASSERT_TRUE(vector_index->Add(vectors, vids, num_vectors));
    std::vector<uint8_t> serialized_index = vector_index->Save();
    ASSERT_FALSE(serialized_index.empty());
    lgraph::HNSW vector_index_loaded("label", "name",
                            "L2", "HNSW", dim, index_spec, nullptr);
    ASSERT_TRUE(vector_index_loaded.Build());
    vector_index_loaded.Load(serialized_index);
    std::vector<float> query(vectors[0].begin(), vectors[0].end());
    std::vector<float> distances;
    std::vector<int64_t> indices;
    ASSERT_TRUE(vector_index_loaded.Search(query, 10, distances, indices));
    ASSERT_EQ(indices[0], vids[0]);
}

TEST_F(VSAGTest, DeleteVectors) {
    ASSERT_TRUE(vector_index->Build());
    ASSERT_TRUE(vector_index->Add(vectors, vids, num_vectors));
    std::vector<size_t> delete_vids = {vids[0], vids[1]};
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

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
