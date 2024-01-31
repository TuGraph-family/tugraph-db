﻿/**
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

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "fma-common/timed_task.h"

#include "core/global_config.h"
#include "core/kv_store.h"
#include "core/lightning_graph.h"
#include "core/managed_object.h"

#include "db/acl.h"
#include "db/db.h"

namespace lgraph {
class Galaxy;

class GraphManager {
 public:
    struct Config {
        bool durable = false;
        bool load_plugins = true;
        int plugin_subprocess_max_idle_seconds = 600;
        FullTextIndexOptions ft_index_options;
        bool enable_realtime_count = true;

        Config() {}
        explicit Config(const GlobalConfig& gc)
            : durable(gc.durable),
              load_plugins(true),
              plugin_subprocess_max_idle_seconds(gc.subprocess_max_idle_seconds),
              ft_index_options(gc.ft_index_options),
              enable_realtime_count(gc.enable_realtime_count) {}
    };

    struct ModGraphActions {
        bool mod_desc = false;
        std::string desc;
        bool mod_size = false;
        size_t max_size = 0;
    };

 private:
    typedef GCRefCountedPtr<LightningGraph> GcDb;

    std::shared_ptr<KvTable> table_;
    std::unordered_map<std::string, GcDb> graphs_;
    std::string parent_dir_;
    Config config_;

    std::string GenNewGraphSubDir();

    void StoreConfig(KvTransaction& txn, const std::string& name, const DBConfig& config);

    LightningGraph* OpenGraph();

 public:
    // called at program start
    void Init(KvStore* store, KvTransaction& txn, const std::string& table_name,
              const std::string& parent_dir, const Config& config);

    // create a graph
    bool CreateGraph(KvTransaction& txn, const std::string& name, const DBConfig& config);

    // create a graph with data
    bool CreateGraphWithData(KvTransaction& txn, const std::string& name, const DBConfig& config,
                     const std::string& data_file_path);

    // del graph info from memory and kv
    lgraph::GraphManager::GcDb DelGraph(KvTransaction& txn, const std::string& name);

    bool ModGraph(KvTransaction& txn, const std::string& name, const ModGraphActions& actions);

    std::map<std::string, DBConfig> ListGraphs() const;

    lgraph::ScopedRef<lgraph::LightningGraph> GetGraphRef(const std::string& graph) const;

    bool GraphExists(const std::string& graph) const;

    // reload data from disk
    void ReloadFromDisk(KvStore* store, KvTransaction& txn, const std::string& table_name,
                        const std::string& parent_dir, const Config& config);

    // backup all graphs to dirs under parent_dir
    std::vector<std::string> Backup(const std::string& parent_dir);

    // closes all graphs before destroy
    void CloseAllGraphs();
};
}  // namespace lgraph
