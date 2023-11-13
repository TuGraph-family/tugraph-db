/**
 * Copyright 2022 AntGroup CO., Ltd.
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

#include <random>

#include "fma-common/binary_buffer.h"
#include "fma-common/binary_read_write_helper.h"
#include "fma-common/encrypt.h"

#include "db/graph_manager.h"

// generate a subdir name for new graph
// the dir name is generated as Base16(timestamp) + 16-byte random string
std::string lgraph::GraphManager::GenNewGraphSubDir() {
    // random string
    std::string subdir;
    size_t s = lgraph::_detail::GRAPH_SUBDIR_NAME_LEN;
    std::random_device r;
    std::default_random_engine e1(r());
    std::uniform_int_distribution<int> uniform_dist(0, 256);
    std::string buf(s, 0);
    for (size_t i = 0; i < buf.size(); i++) buf[i] = (char)uniform_dist(e1);
    subdir.append(fma_common::encrypt::Base16::Encode(buf));
    // append timestamp
    auto t = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::string tmps;
    tmps.assign((const char*)&t, sizeof(t));
    subdir.append(fma_common::encrypt::Base16::Encode(tmps));
    return subdir;
}

static inline std::string GetGraphActualDir(const std::string& parent_dir,
                                            const std::string& sub_dir) {
    return fma_common::StringFormatter::Format("{}/{}", parent_dir, sub_dir);
}

// utility function, stores graph config to KvStore
void lgraph::GraphManager::StoreConfig(lgraph::KvTransaction& txn, const std::string& name,
                                       const lgraph::DBConfig& config) {
    fma_common::BinaryBuffer buf;
    fma_common::BinaryWrite(buf, config);
    table_->SetValue(txn, lgraph::Value::ConstRef(name),
                     lgraph::Value(buf.GetBuf(), buf.GetSize()));
}

void lgraph::GraphManager::CloseAllGraphs() {
    std::mutex m;
    std::condition_variable cv;
    size_t n_destroyed = 0;
    size_t n_opened = 0;
    for (auto& kv : graphs_) {
        if (kv.second) {
            n_opened++;
            kv.second.Assign(nullptr, nullptr, [&]() {
                std::lock_guard<std::mutex> l(m);
                n_destroyed++;
                cv.notify_all();
            });
        }
    }
    std::unique_lock<std::mutex> l(m);
    while (n_opened != n_destroyed) cv.wait(l);
    graphs_.clear();
}

void lgraph::GraphManager::Init(KvStore* store, KvTransaction& txn, const std::string& table_name,
                                const std::string& parent_dir, const Config& config) {
    ReloadFromDisk(store, txn, table_name, parent_dir, config);
}

inline void UpdateDBConfigWithGMConfig(lgraph::DBConfig& dbc,
                                       const lgraph::GraphManager::Config& gmc) {
    dbc.load_plugins = gmc.load_plugins;
    dbc.durable = gmc.durable;
    dbc.subprocess_max_idle_seconds = gmc.plugin_subprocess_max_idle_seconds;
    dbc.ft_index_options = gmc.ft_index_options;
    dbc.enable_realtime_count = gmc.enable_realtime_count;
}

bool lgraph::GraphManager::CreateGraph(KvTransaction& txn, const std::string& name,
                                       const DBConfig& config) {
    // check desc length
    if (config.desc.size() > _detail::MAX_DESC_LEN)
        throw InputError("Graph description is too long.");
    auto it = graphs_.find(name);
    if (it != graphs_.end()) return false;
    if (graphs_.size() >= _detail::MAX_NUM_GRAPHS)
        throw std::runtime_error("Maximum number of graphs reached: " +
                                 std::to_string(_detail::MAX_NUM_GRAPHS));
    DBConfig real_config = config;
    UpdateDBConfigWithGMConfig(real_config, config_);
    real_config.name = name;
    if (real_config.db_size == 0)
        real_config.db_size = _detail::DEFAULT_GRAPH_SIZE;
    else if (real_config.db_size > _detail::MAX_GRAPH_SIZE)
        throw InputError("Graph max size is too big.");
    real_config.dir = GenNewGraphSubDir();
    StoreConfig(txn, name, real_config);
    // update graphs_
    real_config.create_if_not_exist = true;
    std::string secret = real_config.dir;
    real_config.dir = GetGraphActualDir(parent_dir_, real_config.dir);
    std::unique_ptr<LightningGraph> graph(new LightningGraph(real_config));
    graph->CheckDbSecret(secret);
    graphs_.emplace_hint(it, name, GcDb(graph.release()));
    return true;
}

lgraph::GraphManager::GcDb lgraph::GraphManager::DelGraph(KvTransaction& txn,
                                                          const std::string& name) {
    auto it = graphs_.find(name);
    if (it == graphs_.end()) return GcDb();
    // delete from table_
    table_->DeleteKey(txn, Value::ConstRef(name));
    // delete DB after all references are released
    GcDb gcobj = it->second;
    graphs_.erase(it);
    return gcobj;
}

bool lgraph::GraphManager::ModGraph(KvTransaction& txn, const std::string& name,
                                    const ModGraphActions& actions) {
    auto it = graphs_.find(name);
    if (it == graphs_.end()) return false;
    auto db = it->second.GetScopedRef();
    DBConfig old_config = db->GetConfig();
    // close graph
    db->Close();
    // setup rollback actions in case of exception
    CleanupActions rollback;
    rollback.Emplace([&](){
        UpdateDBConfigWithGMConfig(old_config, config_);
        db->ReloadFromDisk(old_config);
    });
    // now update config
    DBConfig config = old_config;
    config.dir = fma_common::FilePath(config.dir).Name();
    if (actions.mod_desc) {
        if (actions.desc.size() > _detail::MAX_DESC_LEN)
            throw InputError("Graph description is too long.");
        config.desc = actions.desc;
    }
    if (actions.mod_size) {
        if (actions.max_size > _detail::MAX_GRAPH_SIZE)
            throw InputError("Graph max size is too big.");
        config.db_size = std::min(actions.max_size, _detail::MAX_GRAPH_SIZE);
    }
    // write config to db
    StoreConfig(txn, name, config);
    // re-open
    UpdateDBConfigWithGMConfig(config, config_);
    config.dir = GetGraphActualDir(parent_dir_, config.dir);
    FMA_DBG() << "Re-openning graph with config {" << fma_common::ToString(config) << "}";
    db->ReloadFromDisk(config);
    // cancel rollback
    rollback.CancelAll();
    return true;
}

std::map<std::string, lgraph::DBConfig> lgraph::GraphManager::ListGraphs() const {
    std::map<std::string, DBConfig> ret;
    for (auto& kv : graphs_) ret.emplace(kv.first, kv.second.GetScopedRef()->GetConfig());
    return ret;
}

lgraph::ScopedRef<lgraph::LightningGraph> lgraph::GraphManager::GetGraphRef(
    const std::string& graph) const {
    if (graph.empty()) throw InputError("Graph name cannot be empty.");
    auto it = graphs_.find(graph);
    if (it == graphs_.end())
        throw InputError(fma_common::StringFormatter::Format("No such graph: {}", graph));
    return it->second.GetScopedRef();
}

bool lgraph::GraphManager::GraphExists(const std::string& graph) const {
    if (!IsValidLGraphName(graph)) throw InputError("Invalid graph name: " + graph);
    return graphs_.find(graph) != graphs_.end();
}

void lgraph::GraphManager::ReloadFromDisk(KvStore* store, KvTransaction& txn,
                                          const std::string& table_name,
                                          const std::string& parent_dir, const Config& config) {
    std::unordered_set<std::string> existing_graphs;
    for (auto& kv : graphs_) existing_graphs.emplace(kv.first);

    config_ = config;
    table_ = store->OpenTable(txn, table_name, true, ComparatorDesc::DefaultComparator());
    parent_dir_ = parent_dir;
    if (table_->GetKeyCount(txn) == 0) {
        // create default DB
        DBConfig config;
        config.name = _detail::DEFAULT_GRAPH_DB_NAME;
        config.dir = GenNewGraphSubDir();
        UpdateDBConfigWithGMConfig(config, config_);
        StoreConfig(txn, _detail::DEFAULT_GRAPH_DB_NAME, config);
        // create the graph so later we can open it succesfully
        config.create_if_not_exist = true;
        std::string secret = config.dir;
        config.dir = GetGraphActualDir(parent_dir_, config.dir);
        LightningGraph l(config);
        l.CheckDbSecret(secret);
    }
    auto it = table_->GetIterator(txn);
    for (it->GotoFirstKey(); it->IsValid(); it->Next()) {
        const std::string& graph_name = it->GetKey().AsString();
        const Value& value = it->GetValue();
        if (existing_graphs.erase(graph_name) == 1) {
            // existing graph, just reload
            ScopedRef<LightningGraph> g = graphs_[graph_name].GetScopedRef();
            g->ReloadFromDisk(DBConfig());
        } else {
            // new graph, add
            DBConfig conf;
            UpdateDBConfigWithGMConfig(conf, config_);
            conf.name = graph_name;
            fma_common::BinaryBuffer buf(value.Data(), value.Size());
            if (fma_common::BinaryRead(buf, conf) != value.Size()) {
                throw std::runtime_error("Failed to read DB config for graph " + graph_name);
            }
            std::string secret = conf.dir;
            conf.dir = GetGraphActualDir(parent_dir_, conf.dir);
            // now open db, set create_if_not_exist to false
            conf.create_if_not_exist = false;
            conf.durable = config_.durable;
            conf.load_plugins = config_.load_plugins;
            // FMA_DBG() << "Openning graph with config {" << fma_common::ToString(conf) << "}";
            std::unique_ptr<LightningGraph> graph(new LightningGraph(conf));
            if (!graph->CheckDbSecret(secret)) throw std::runtime_error("DB corruptted.");
            graphs_.emplace(graph_name, GcDb(graph.release()));
        }
    }
    // clear up deleted graphs
    for (const std::string& g : existing_graphs) {
        GcDb& db = graphs_[g];
        db.Assign(nullptr, [](LightningGraph* db) {
            std::string dir = db->GetConfig().dir;
            // db->Close();
            if (fma_common::FileSystem::GetFileSystem(dir).RemoveDir(dir))
                FMA_WARN() << "GraphDB " << dir << " deleted.";
        });
        graphs_.erase(g);
    }
}

std::vector<std::string> lgraph::GraphManager::Backup(const std::string& backup_parent_dir) {
    std::vector<std::string> ret;
    // copy graphs one by one
    for (auto& kv : graphs_) {
        const std::string& name = kv.first;
        FMA_LOG() << "Backup subgraph " << name;
        ScopedRef<LightningGraph> g = kv.second.GetScopedRef();
        std::string sub_dir = fma_common::FilePath(g->GetConfig().dir).Name();
        std::string graph_dir = GetGraphActualDir(backup_parent_dir, sub_dir);
        ret.push_back(graph_dir + "/data.mdb");
        if (!fma_common::file_system::MkDir(graph_dir)) {
            FMA_WARN() << "Error backing up graph " << name << ": cannot create dir " << graph_dir;
            throw std::runtime_error("Error backing up graph [" + name + "]: cannot create dir " +
                                     graph_dir);
        }
        g->Backup(graph_dir);
    }
    return ret;
}
