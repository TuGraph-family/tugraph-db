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
#include <rocksdb/convenience.h>
#include <rocksdb/db.h>
#include <rocksdb/utilities/transaction_db.h>
#include <string>
#include <boost/asio.hpp>
#include "common/value.h"
#include "graph_cf.h"
#include "id_generator.h"
#include "meta_info.h"
#include "proto/meta.pb.h"

namespace txn {
class Transaction;
}
namespace graphdb {

struct GraphDBOptions {
    std::shared_ptr<rocksdb::Cache> block_cache;
    std::shared_ptr<rocksdb::RowCache> row_cache;
    size_t ft_apply_interval_ = 3;
    size_t vt_apply_interval_ = 3;
};

class GraphDB {
   public:
    GraphDB() = default;
    ~GraphDB();
    // No copying allowed
    GraphDB(const GraphDB&) = delete;
    void operator=(const GraphDB&) = delete;

    static std::unique_ptr<GraphDB> Open(const std::string& path, const GraphDBOptions& options);
    std::unique_ptr<txn::Transaction> BeginTransaction();
    void AddVertexPropertyIndex(const std::string& index_name, bool,
                                const std::string& label,
                                const std::string& property);
    void DeleteVertexPropertyIndex(const std::string& index_name);
    void AddVertexFullTextIndex(const std::string& index_name,
                                const std::vector<std::string>& labels,
                                const std::vector<std::string>& properties);
    void DeleteVertexFullTextIndex(const std::string& index_name);
    void AddVertexVectorIndex(const std::string& index_name,
                                const std::string& label,
                                const std::string& property,
                              int dimension, std::string distance_type,
                              int hnsw_m, int hnsw_ef_construction);
    void DeleteVertexVectorIndex(const std::string& index_name);
    std::vector<rocksdb::ColumnFamilyHandle*>& cf_handles() {
        return cf_handles_;
    }
    GraphCF& graph_cf() { return graph_cf_; }
    IdGenerator& id_generator() { return id_generator_; }
    MetaInfo& meta_info() { return meta_info_; }
    BusyIndex& busy_index() { return busy_index_; }
    meta::GraphDBMetaInfo& db_meta() { return db_meta_; }
    const std::string& path() { return path_; }
    bool& drop_on_close() {return drop_on_close_;}

   private:
    std::string path_;
    rocksdb::TransactionDB* db_ = nullptr;
    std::vector<rocksdb::ColumnFamilyHandle*> cf_handles_;
    boost::asio::io_service assistant_;
    GraphCF graph_cf_;
    IdGenerator id_generator_;
    MetaInfo meta_info_;
    BusyIndex busy_index_;
    meta::GraphDBMetaInfo db_meta_;
    std::vector<std::thread> service_threads_;
    GraphDBOptions options_;
    bool drop_on_close_ = false;
};
}