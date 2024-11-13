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
#include <atomic>
#include <set>
#include <shared_mutex>
#include <rocksdb/utilities/transaction_db.h>
#include "graph_cf.h"
namespace graphdb {
enum class TokenNameType : char { VertexLabel = 0, EdgeType = 1, Property = 2 };

class IdGenerator {
   public:
    IdGenerator() = default;
    // No copying allowed
    IdGenerator(const IdGenerator&) = delete;
    void operator=(const IdGenerator&) = delete;

    void Init(rocksdb::TransactionDB* db, GraphCF* graph_cf);
    int64_t GetNextVid();
    int64_t GetNextEid();
    uint32_t GetNextIndexId();
    std::optional<uint32_t> GetLid(const std::string& name);
    std::optional<uint32_t> GetPid(const std::string& name);
    std::optional<uint32_t> GetTid(const std::string& name);
    std::optional<std::string> GetPropertyName(uint32_t pid);
    std::optional<std::string> GetVertexLabelName(uint32_t lid);
    std::optional<std::string> GetEdgeTypeName(uint32_t tid);
    uint32_t GetOrCreateLid(const std::string& name);
    uint32_t GetOrCreatePid(const std::string& name);
    uint32_t GetOrCreateTid(const std::string& name);
    std::unordered_set<std::string> GetProperties();
    std::unordered_set<std::string> GetVertexLabels();
    std::unordered_set<std::string> GetEdgeTypes();

   private:
    std::atomic<int64_t> vertex_next_vid_;
    std::atomic<int64_t> edge_next_eid_;
    std::atomic<uint32_t> label_next_lid_;
    std::atomic<uint32_t> label_next_pid_;
    std::atomic<uint32_t> label_next_tid_;
    std::atomic<uint32_t> index_next_id_;
    std::unordered_map<std::string, uint32_t> vertex_labels_name_to_id_;
    std::unordered_map<uint32_t, std::string> vertex_labels_id_to_name_;
    std::unordered_map<std::string, uint32_t> edge_types_name_to_id_;
    std::unordered_map<uint32_t, std::string> edge_types_id_to_name_;
    std::unordered_map<std::string, uint32_t> properties_name_to_id_;
    std::unordered_map<uint32_t, std::string> properties_id_to_name_;
    rocksdb::TransactionDB* db_ = nullptr;
    GraphCF* graph_cf_;
    std::shared_mutex vertex_labels_mutex_;
    std::shared_mutex edge_types_mutex_;
    std::shared_mutex properties_mutex_;
};
}