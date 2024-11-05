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

#include "galaxy.h"

#include "common/logger.h"

#include <boost/endian/conversion.hpp>
#include <filesystem>

#include "common/exceptions.h"
using namespace graphdb;
namespace server {
std::unique_ptr<server::Galaxy> g_galaxy;
Galaxy::~Galaxy() {
    graphs_.clear();
    auto s = meta_db_->Close();
    if (!s.ok()) {
        LOG_WARN("meta db close error : {}", s.ToString());
    }
    delete meta_db_;
    meta_db_ = nullptr;
    LOG_INFO("Close galaxy");
}

std::unique_ptr<Galaxy> Galaxy::Open(const std::string &path, const GalaxyOptions& galaxy_options) {
    LOG_INFO("Open galaxy: {}", path);
    rocksdb::Options options;
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    rocksdb::TransactionDBOptions txn_db_options;

    std::string meta_path = path + "/meta";
    std::filesystem::create_directories(meta_path);
    rocksdb::TransactionDB* db = nullptr;
    auto s = rocksdb::TransactionDB::Open(options, txn_db_options, meta_path,
                                          &db);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    auto galaxy = std::make_unique<Galaxy>();
    galaxy->path_ = path;
    galaxy->block_cache_ = rocksdb::NewLRUCache(galaxy_options.block_cache_size);
    galaxy->row_cache_ = rocksdb::NewLRUCache(galaxy_options.row_cache_size);
    galaxy->options_ = galaxy_options;
    galaxy->meta_db_ = db;

    rocksdb::ReadOptions ro;
    {
        std::string next_graph_id_key(
            1, static_cast<char>(GalaxyMetaDataType::NextGraphID));
        std::string val;
        s = galaxy->meta_db_->Get(ro, next_graph_id_key, &val);
        if (s.ok()) {
            galaxy->next_graph_id_ = *(uint64_t*)val.data();
        } else if (s.IsNotFound()) {
            galaxy->next_graph_id_ = 1;
        } else {
            THROW_CODE(StorageEngineError, s.ToString());
        }
    }
    std::unique_ptr<rocksdb::Iterator> iter(galaxy->meta_db_->NewIterator(ro));
    std::string prefix;
    prefix.append(1, static_cast<char>(GalaxyMetaDataType::GraphDB));
    for (iter->Seek(prefix); iter->Valid() && iter->key().starts_with(prefix);
         iter->Next()) {
        auto val = iter->value();
        meta::GraphDBMetaInfo meta;
        bool f = meta.ParseFromString(val.ToString());
        assert(f);
        LOG_INFO("Load GraphDB: [{}]", meta.ShortDebugString());
        std::string graph_path =
            galaxy->path_ + "/graph" + std::to_string(meta.graph_id());
        auto graph_db = GraphDB::Open(
            graph_path, {.block_cache = galaxy->block_cache_,
                         .row_cache = galaxy->row_cache_,
                         .ft_commit_interval_ = galaxy->options_.ft_commit_interval});
        graph_db->db_meta() = meta;
        galaxy->graphs_.emplace(meta.graph_name(), std::move(graph_db));
    }
    if (galaxy->graphs_.empty()) {
        galaxy->CreateGraph("default");
    }
    return galaxy;
}

std::shared_ptr<GraphDB> Galaxy::OpenGraph(const std::string &name) {
    std::shared_lock<std::shared_mutex> read_lock(graphs_mutex_);
    auto iter = graphs_.find(name);
    if (iter != graphs_.end()) {
        return iter->second;
    } else {
        THROW_CODE(NoSuchGraph, "No such graph: {}", name);
    }
}

GraphDB *Galaxy::CreateGraph(const std::string &name) {
    std::unique_lock<std::shared_mutex> write_lock(graphs_mutex_);
    auto iter = graphs_.find(name);
    if (iter != graphs_.end()) {
        THROW_CODE(GraphAlreadyExists, "The graph already exists: {}", name);
    }
    meta::GraphDBMetaInfo meta;
    uint64_t graph_id = next_graph_id_++;
    meta.set_graph_id(graph_id);
    meta.set_graph_name(name);
    std::string graph_path = path_ + "/graph" + std::to_string(meta.graph_id());
    auto graph_db = GraphDB::Open(
        graph_path, {.block_cache = block_cache_,
                     .row_cache = row_cache_,
                     .ft_commit_interval_ = options_.ft_commit_interval});
    rocksdb::WriteOptions wo;
    rocksdb::WriteBatch wb;
    std::string key;
    key.append(1, static_cast<char>(GalaxyMetaDataType::GraphDB));
    boost::endian::native_to_big_inplace(graph_id);
    key.append((const char *)&graph_id, sizeof(graph_id));
    wb.Put(key, meta.SerializeAsString());
    uint64_t next = next_graph_id_;
    wb.Put(std::string(1, static_cast<char>(GalaxyMetaDataType::NextGraphID)), std::string((const char*)&next, sizeof(next)));
    auto s = meta_db_->Write(wo, {}, &wb);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    LOG_INFO("Create graph:{}, path:{}", name, graph_path);
    graph_db->db_meta() = meta;
    graphs_.emplace(meta.graph_name(), std::move(graph_db));
    return graphs_[name].get();
}

void Galaxy::DeleteGraph(const std::string &name) {
    std::unique_lock<std::shared_mutex> write_lock(graphs_mutex_);
    auto iter = graphs_.find(name);
    if (iter == graphs_.end()) {
        THROW_CODE(NoSuchGraph, "No such graph: {}", name);
    }

    rocksdb::WriteOptions wo;
    std::string key;
    uint64_t graph_id = iter->second->db_meta().graph_id();
    key.append(1, static_cast<char>(GalaxyMetaDataType::GraphDB));
    boost::endian::native_to_big_inplace(graph_id);
    key.append((const char *)&graph_id, sizeof(graph_id));
    auto s = meta_db_->Delete(wo, key);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    iter->second->drop_on_close() = true;
    std::string graph_path = iter->second->path();
    LOG_INFO("Erase graph:{}, path:{}", name, graph_path);
    graphs_.erase(iter);
}

}