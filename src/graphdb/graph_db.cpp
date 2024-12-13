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
#include "graph_db.h"

#include <filesystem>

#include "common/logger.h"
#include "meta_info.h"
#include "proto/meta.pb.h"
#include "transaction/transaction.h"
namespace fs = std::filesystem;
using namespace boost::endian;
namespace graphdb {
std::unique_ptr<GraphDB> GraphDB::Open(const std::string& path,
                                       const GraphDBOptions& graph_options) {
    std::string rocksdb_path = path + "/data";
    std::filesystem::create_directories(rocksdb_path);
    rocksdb::Options options;
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    options.enable_pipelined_write = true;
    rocksdb::BlockBasedTableOptions table_options;
    table_options.cache_index_and_filter_blocks = true;
    if (graph_options.block_cache) {
        table_options.block_cache = graph_options.block_cache;
    } else {
        table_options.block_cache =
            rocksdb::NewLRUCache(1 * 1024 * 1024 * 1024L);
    }
    table_options.data_block_index_type =
        rocksdb::BlockBasedTableOptions::kDataBlockBinaryAndHash;
    table_options.partition_filters = true;
    table_options.index_type =
        rocksdb::BlockBasedTableOptions::IndexType::kTwoLevelIndexSearch;
    options.table_factory.reset(
        rocksdb::NewBlockBasedTableFactory(table_options));
    if (graph_options.row_cache) {
        options.row_cache = graph_options.row_cache;
    } else {
        rocksdb::LRUCacheOptions cache_options;
        cache_options.capacity = 1 * 1024 * 1024 * 1024L;
        options.row_cache = cache_options.MakeSharedRowCache();
    }
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    std::vector<std::string> built_in_cfs = {rocksdb::kDefaultColumnFamilyName,
                                             "graph_topology",
                                             "vertex_property",
                                             "edge_property",
                                             "vertex_label_vid",
                                             "edge_type_eid",
                                             "name_id",
                                             "meta_info",
                                             "index",
                                             "wal",
                                             "vector_index_manifest",
                                             "vector_index_delta"};
    std::vector<rocksdb::ColumnFamilyDescriptor> cfs;
    cfs.reserve(built_in_cfs.size());
    for (const auto& name : built_in_cfs) {
        cfs.emplace_back(name, options);
    }
    std::vector<rocksdb::ColumnFamilyHandle*> cf_handles;
    rocksdb::TransactionDBOptions txn_db_options;
    rocksdb::TransactionDB* db;
    auto s = rocksdb::TransactionDB::Open(options, txn_db_options, rocksdb_path,
                                          cfs, &cf_handles, &db);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    auto graph_db = std::make_unique<GraphDB>();
    graph_db->db_ = db;
    graph_db->path_ = path;
    graph_db->graph_cf_.graph_topology = cf_handles[1];
    graph_db->graph_cf_.vertex_property = cf_handles[2];
    graph_db->graph_cf_.edge_property = cf_handles[3];
    graph_db->graph_cf_.vertex_label_vid = cf_handles[4];
    graph_db->graph_cf_.edge_type_eid = cf_handles[5];
    graph_db->graph_cf_.name_id = cf_handles[6];
    graph_db->graph_cf_.meta_info = cf_handles[7];
    graph_db->graph_cf_.index = cf_handles[8];
    graph_db->graph_cf_.wal = cf_handles[9];
    graph_db->graph_cf_.vector_index_manifest = cf_handles[10];
    graph_db->graph_cf_.vector_index_delta = cf_handles[11];
    graph_db->cf_handles_ = std::move(cf_handles);
    graph_db->options_ = graph_options;
    graph_db->service_threads_.emplace_back([&graph_db]() {
        pthread_setname_np(pthread_self(), "assistant");
        boost::asio::io_service::work holder(graph_db->assistant_);
        graph_db->assistant_.run();
    });
    graph_db->meta_info_.Init(graph_db->db_, graph_db->assistant_,
                              &graph_db->graph_cf_, &graph_db->id_generator_,
                              graph_db->options_.ft_commit_interval_);
    graph_db->id_generator_.Init(graph_db->db_, &graph_db->graph_cf_);

    return graph_db;
}

GraphDB::~GraphDB() {
    LOG_INFO("Close graph: {}", db_meta_.graph_name());
    meta_info_.GetVertexVectorIndex().clear();
    meta_info_.GetVertexFullTextIndex().clear();
    for (auto handle : cf_handles_) {
        auto s = db_->DestroyColumnFamilyHandle(handle);
        assert(s.ok());
    }
    auto s = db_->Close();
    if (!s.ok()) {
        LOG_WARN("graph db close error : {}", s.ToString());
    }
    delete db_;
    db_ = nullptr;

    assistant_.stop();
    for (auto& t : service_threads_) {
        t.join();
    }
    if (drop_on_close_) {
        std::filesystem::remove_all(path_);
        LOG_INFO("filesystem remove_all {}", path_);
    }
}

std::unique_ptr<txn::Transaction> GraphDB::BeginTransaction() {
    rocksdb::WriteOptions wo;
    rocksdb::TransactionOptions to;
    rocksdb::Transaction* txn = db_->BeginTransaction(wo, to);
    return std::make_unique<txn::Transaction>(txn, this);
}

void GraphDB::AddVertexPropertyIndex(const std::string& index_name,
                                     bool /*unique*/, const std::string& label,
                                     const std::string& property) {
    if (index_name.empty() || label.empty() || property.empty()) {
        THROW_CODE(InvalidParameter);
    }
    if (meta_info_.GetVertexPropertyIndex(index_name)) {
        THROW_CODE(VertexIndexAlreadyExist,
                   "Vertex index name {} already exists", index_name);
    }
    auto lid = id_generator_.GetOrCreateLid(label);
    auto pid = id_generator_.GetOrCreatePid(property);
    if (meta_info_.GetVertexPropertyIndex(lid, pid)) {
        THROW_CODE(VertexIndexAlreadyExist,
                   "Vertex index [label:{}, property:{}] already exists",
                   big_to_native(lid), big_to_native(pid));
    }
    busy_index_.Mark({pid}, {lid});
    auto index_id = id_generator_.GetNextIndexId();
    rocksdb::ReadOptions ro;
    rocksdb::WriteOptions wo;
    std::unique_ptr<rocksdb::Iterator> iter(
        db_->NewIterator(ro, graph_cf_.vertex_label_vid));
    rocksdb::Slice prefix((const char*)&lid, sizeof(lid));
    for (iter->Seek(prefix); iter->Valid() && iter->key().starts_with(prefix);
         iter->Next()) {
        auto key = iter->key();
        key.remove_prefix(sizeof(uint32_t));

        std::string property_key = key.ToString();
        property_key.append((const char*)&pid, sizeof(pid));
        std::string property_val;
        auto s = db_->Get(ro, graph_cf_.vertex_property, property_key,
                          &property_val);
        if (s.IsNotFound()) {
            continue;
        } else if (!s.ok()) {
            THROW_CODE(StorageEngineError, s.ToString());
        }
        std::string index_key, tmp;
        index_key.append((const char*)(&index_id), sizeof(index_id));
        index_key.append(property_val);
        s = db_->Get(ro, graph_cf_.index, index_key, &tmp);
        if (s.ok()) {
            THROW_CODE(IndexValueAlreadyExist);
        } else if (!s.IsNotFound()) {
            THROW_CODE(StorageEngineError, s.ToString());
        }
        rocksdb::Slice vid = key;
        s = db_->Put(wo, graph_cf_.index, index_key, vid);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }

    // write meta info
    std::string meta_key;
    meta_key.append(1, static_cast<char>(MetaDataType::VertexPropertyIndex));
    meta_key.append(index_name);
    meta::VertexPropertyIndex meta_val;
    meta_val.set_name(index_name);
    meta_val.set_is_unique(true);
    meta_val.set_label(label);
    meta_val.set_property(property);
    meta_val.set_label_id(big_to_native(lid));
    meta_val.set_property_id(big_to_native(pid));
    meta_val.set_index_id(big_to_native(index_id));
    auto s = db_->Put(wo, graph_cf_.meta_info, meta_key,
                      meta_val.SerializeAsString());
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    LOG_INFO("Add vertex index: [lid:{}, pid:{}, is_unique:{}]",
             big_to_native(lid), big_to_native(pid), true);
    VertexPropertyIndex vpi(meta_val, graph_cf_.index, index_id, lid, pid);
    auto ret = meta_info_.AddVertexPropertyIndex(std::move(vpi));
    assert(ret);
    busy_index_.Clear();
}

void GraphDB::DeleteVertexPropertyIndex(const std::string& index_name) {
    if (!meta_info_.GetVertexPropertyIndex(index_name)) {
        THROW_CODE(VertexUniqueIndexNotFound,
                   "No such vertex unique index [{}]", index_name);
    }
    meta_info_.DeleteVertexPropertyIndex(index_name);
    std::string meta_key;
    meta_key.append(1, static_cast<char>(MetaDataType::VertexPropertyIndex));
    meta_key.append(index_name);
    rocksdb::WriteOptions wo;
    auto s = db_->Delete(wo, graph_cf_.meta_info, meta_key);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    LOG_INFO("Delete vertex unique index: {}", index_name);
}

void GraphDB::AddVertexFullTextIndex(
    const std::string& index_name, const std::vector<std::string>& labels,
    const std::vector<std::string>& properties) {
    if (index_name.empty() || labels.empty() || properties.empty()) {
        THROW_CODE(InvalidParameter);
    }
    if (meta_info_.GetVertexFullTextIndex(index_name)) {
        THROW_CODE(VertexFullTextIndexAlreadyExist,
                   "Vertex fulltext index [{}] already exists", index_name);
    }
    std::string ft_index_pth = path_ + "/ft/" + index_name;
    std::filesystem::create_directories(ft_index_pth);
    std::unordered_set<uint32_t> lids, native_lids;
    std::unordered_set<uint32_t> pids, native_pids;
    for (const auto& label : labels) {
        auto lid = id_generator_.GetOrCreateLid(label);
        lids.insert(lid);
        native_lids.insert(big_to_native(lid));
    }
    for (const auto& prop : properties) {
        auto pid = id_generator_.GetOrCreatePid(prop);
        pids.insert(pid);
        native_pids.insert(big_to_native(pid));
    }
    uint32_t index_id = id_generator_.GetNextIndexId();
    // write meta info
    std::string meta_key;
    meta_key.append(1, static_cast<char>(MetaDataType::VertexFullTextIndex));
    meta_key.append(index_name);
    meta::VertexFullTextIndex meta;
    meta.set_name(index_name);
    meta.set_path(ft_index_pth);
    *meta.mutable_labels() = {labels.begin(), labels.end()};
    *meta.mutable_properties() = {properties.begin(), properties.end()};
    *meta.mutable_label_ids() = {native_lids.begin(), native_lids.end()};
    *meta.mutable_property_ids() = {native_pids.begin(), native_pids.end()};
    auto v_ft_index = std::make_unique<VertexFullTextIndex>(
        db_, assistant_, &graph_cf_, &id_generator_, meta, index_id, lids, pids,
        options_.ft_commit_interval_);
    v_ft_index->Load();
    rocksdb::WriteOptions wo;
    auto s =
        db_->Put(wo, graph_cf_.meta_info, meta_key, meta.SerializeAsString());
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    LOG_INFO("Add vertex full text index: [lids:{}, pids:{}]", native_lids,
             native_pids);
    meta_info_.AddVertexFullTextIndex(std::move(v_ft_index));
}

void GraphDB::DeleteVertexFullTextIndex(const std::string& index_name) {
    if (!meta_info_.GetVertexFullTextIndex(index_name)) {
        THROW_CODE(FullTextIndexNotFound, "No such vertex fulltext index [{}]",
                   index_name);
    }
    auto path = meta_info_.GetVertexFullTextIndex(index_name)->meta().path();
    std::string meta_key;
    meta_key.append(1, static_cast<char>(MetaDataType::VertexFullTextIndex));
    meta_key.append(index_name);
    rocksdb::WriteOptions wo;
    auto s = db_->Delete(wo, graph_cf_.meta_info, meta_key);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    meta_info_.DeleteVertexFullTextIndex(index_name);
    try {
        std::filesystem::remove_all(path);
        LOG_INFO("remove fulltext index data {}", path);
    } catch (const std::exception& e) {
        LOG_ERROR("[DeleteVertexFullTextIndex] remove_all {} meet error: {}",
                  path, e.what());
    }
}

void GraphDB::AddVertexVectorIndex(const std::string& index_name,
                                   const std::string& label,
                                   const std::string& property,
                                   int sharding_num, int dimension,
                                   std::string distance_type, int hnsw_m,
                                   int hnsw_ef_construction) {
    if (index_name.empty() || label.empty() || property.empty()) {
        THROW_CODE(InvalidParameter);
    }
    if (meta_info_.GetVertexVectorIndex(index_name)) {
        THROW_CODE(VertexVectorIndexAlreadyExist,
                   "Vertex vector index [{}] already exists", index_name);
    }
    auto lid = id_generator_.GetOrCreateLid(label);
    auto pid = id_generator_.GetOrCreatePid(property);
    if (meta_info_.GetVertexVectorIndex(lid, pid)) {
        THROW_CODE(VertexVectorIndexAlreadyExist,
                   "Vertex vector index [label:{}, property:{}] already exists",
                   big_to_native(lid), big_to_native(pid));
    }
    meta::VectorIndexType index_type = meta::VectorIndexType::HNSW;
    meta::VectorDistanceType dist_type;
    if (distance_type == "l2") {
        dist_type = meta::VectorDistanceType::L2;
    } else if (distance_type == "ip") {
        dist_type = meta::VectorDistanceType::IP;
    } else {
        THROW_CODE(InvalidParameter, "Distance Type {} not supported",
                   distance_type);
    }
    auto index_dir = uuid_generator_.Next();
    std::filesystem::create_directories(fmt::format("{}/{}", path_, index_dir));
    meta::VertexVectorIndex meta;
    meta.set_name(index_name);
    meta.set_label(label);
    meta.set_property(property);
    meta.set_sharding_num(sharding_num);
    meta.set_label_id(big_to_native(lid));
    meta.set_property_id(big_to_native(pid));
    meta.set_dimensions(dimension);
    meta.set_index_type(index_type);
    meta.set_distance_type(dist_type);
    meta.set_hnsw_m(hnsw_m);
    meta.set_hnsw_ef_construction(hnsw_ef_construction);
    meta.set_index_dir(index_dir);
    VertexVectorIndex vvi(db_, &graph_cf_, lid, pid, meta);
    vvi.Load();
    // write meta info
    std::string meta_key;
    meta_key.append(1, static_cast<char>(MetaDataType::VertexVectorIndex));
    meta_key.append(index_name);
    auto s =
        db_->Put({}, graph_cf_.meta_info, meta_key, meta.SerializeAsString());
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    LOG_INFO("Add vertex vector index: [lid:{}, pid:{}]", big_to_native(lid),
             big_to_native(pid));
    meta_info_.AddVertexVectorIndex(std::move(vvi));
}

void GraphDB::DeleteVertexVectorIndex(const std::string& index_name) {
    if (!meta_info_.GetVertexVectorIndex(index_name)) {
        THROW_CODE(VectorIndexNotFound, "No such vertex vector index [{}]",
                   index_name);
    }
    std::string meta_key;
    meta_key.append(1, static_cast<char>(MetaDataType::VertexVectorIndex));
    meta_key.append(index_name);
    rocksdb::WriteOptions wo;
    auto s = db_->Delete(wo, graph_cf_.meta_info, meta_key);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    meta_info_.DeleteVertexVectorIndex(index_name);
}

}  // namespace graphdb
