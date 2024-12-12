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
#include "graphdb/graph_db.h"

namespace server {
enum class GalaxyMetaDataType : char {
    GraphDB = 0,
    NextGraphID = 1,
};

struct GalaxyOptions {
    size_t block_cache_size = 64 * 1024 * 1024L;
    size_t row_cache_size = 32 * 1024 * 1024L;
    size_t ft_apply_interval = 3;
    size_t vt_apply_interval = 3;
};

class Galaxy {
   public:
    Galaxy() = default;
    ~Galaxy();
    // No copying allowed
    Galaxy(const Galaxy&) = delete;
    void operator=(const Galaxy&) = delete;

    static std::unique_ptr<Galaxy> Open(const std::string& path, const GalaxyOptions& galaxy_options);
    std::shared_ptr<graphdb::GraphDB> OpenGraph(const std::string& name);
    graphdb::GraphDB* CreateGraph(const std::string& name);
    void DeleteGraph(const std::string& name);
   const std::unordered_map<std::string, std::shared_ptr<graphdb::GraphDB>>& Graphs() {
       return graphs_;
   }

   private:
    rocksdb::TransactionDB* meta_db_ = nullptr;
    std::unordered_map<std::string, std::shared_ptr<graphdb::GraphDB>> graphs_;
    std::shared_ptr<rocksdb::Cache> block_cache_;
    std::shared_ptr<rocksdb::RowCache> row_cache_;
    std::shared_mutex graphs_mutex_;
    std::atomic<uint64_t> next_graph_id_ = 1;
    std::string path_;
    GalaxyOptions options_;
};
extern std::unique_ptr<server::Galaxy> g_galaxy;
}