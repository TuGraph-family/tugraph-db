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

//
// Created by botu.wzy
//

#pragma once
#include <unordered_map>
#include <set>
#include <cinttypes>
#include "graph_cf.h"
#include "index.h"
namespace graphdb {
enum class MetaDataType : char {
    // Do not change the order
    VertexPropertyIndex = 0,
    VertexFullTextIndex = 1,
    VertexVectorIndex = 2
};

struct MetaInfo {
    std::unordered_map<uint64_t, VertexPropertyIndex> vertex_property_indexes;
    std::unordered_map<uint64_t, VertexVectorIndex> vertex_vector_indexes;
    std::unordered_map<std::string, std::unique_ptr<VertexFullTextIndex>> vertex_ft_indexes;

    // property index
    void Init(rocksdb::TransactionDB* db,
              boost::asio::io_service &service,
              GraphCF* graph_cf, IdGenerator* id_generator,
              size_t ft_commit_interval);
    VertexPropertyIndex* GetVertexPropertyIndex(uint32_t lid, uint32_t pid);
    VertexPropertyIndex* GetVertexPropertyIndex(const std::string& index_name);
    std::unordered_map<uint64_t, VertexPropertyIndex>& GetVertexPropertyIndex();
    bool AddVertexPropertyIndex(VertexPropertyIndex&& vpi);
    void DeleteVertexPropertyIndex(const std::string& index_name);

    // fulltext index
    std::unordered_map<std::string, std::unique_ptr<VertexFullTextIndex>>&
    GetVertexFullTextIndex();
    VertexFullTextIndex* GetVertexFullTextIndex(const std::string& name);
    void DeleteVertexFullTextIndex(const std::string& name);
    bool AddVertexFullTextIndex(std::unique_ptr<VertexFullTextIndex> ft);

    // vector index
    VertexVectorIndex* GetVertexVectorIndex(uint32_t lid, uint32_t pid);
    void AddVertexVectorIndex(VertexVectorIndex&& vvi);
    std::unordered_map<uint64_t, VertexVectorIndex>&
    GetVertexVectorIndex();
    VertexVectorIndex* GetVertexVectorIndex(const std::string& index_name);
    void DeleteVertexVectorIndex(const std::string& name);
};
}