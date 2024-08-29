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
#include "core/Faiss_IVF_FLAT.h"
#include <random>

class FaissIndexTest : public ::testing::Test {
 protected:
    int64_t dim = 128;
    int64_t num_vectors = 10000;
    std::vector<std::vector<float>> vectors;
    std::vector<size_t> vids;
    std::unique_ptr<lgraph::IVFFlat> vector_index;
    std::vector<int> index_spec = {100};
    FaissIndexTest() : vectors(num_vectors, std::vector<float>(dim)),
                                                    vids(num_vectors) {}
    void SetUp() override {
        std::mt19937 rng(47);
        std::uniform_real_distribution<> distrib_real;
        for (int64_t i = 0; i < num_vectors; ++i) {
            vids[i] = i;
            for (int j = 0; j < dim; ++j) {
                vectors[i][j] = distrib_real(rng);
            }
        }
        vector_index = std::make_unique<lgraph::IVFFlat>
                ("label", "name", "L2", "IVF_FLAT", dim, index_spec, nullptr);
        ASSERT_TRUE(vector_index->Build());
    }
    void TearDown() override {}
};

TEST_F(FaissIndexTest, BuildIndex) { ASSERT_TRUE(vector_index->Build()); }

TEST_F(FaissIndexTest, AddVectors) { ASSERT_TRUE(vector_index->Add(vectors,
                                                        vids, num_vectors)); }

TEST_F(FaissIndexTest, SearchIndex) {
    ASSERT_TRUE(vector_index->Add(vectors, vids, num_vectors));
    std::vector<float> query = vectors[0];
    std::vector<float> distances;
    std::vector<int64_t> indices;
    ASSERT_TRUE(vector_index->Search(query, 10, distances, indices));
    ASSERT_EQ(indices[0], vids[0]);
}

TEST_F(FaissIndexTest, SaveAndLoadIndex) {
    ASSERT_TRUE(vector_index->Add(vectors, vids, num_vectors));
    std::vector<uint8_t> serialized_index = vector_index->Save();
    ASSERT_FALSE(serialized_index.empty());
    lgraph::IVFFlat vector_index_loaded("label", "name", "L2", "IVF_FLAT",
                                        dim, index_spec, nullptr);
    ASSERT_TRUE(vector_index_loaded.Build());
    vector_index_loaded.Load(serialized_index);
    std::vector<float> query = vectors[0];
    std::vector<float> distances;
    std::vector<int64_t> indices;
    ASSERT_TRUE(vector_index_loaded.Search(query, 10, distances, indices));
    ASSERT_EQ(indices[0], vids[0]);
}