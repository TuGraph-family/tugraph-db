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

#pragma once
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include "libcuckoo/cuckoohash_map.hh"
#include "rocksdb/db.h"
#include "rocksdb/table.h"
#include "core/lightning_graph.h"
#include "core/type_convert.h"
#include "import/column_parser.h"
#include "import/jsonlines_parser.h"
#include "import/graphar_parser.h"
#include "import/import_config_parser.h"
#include "import/dense_string.h"

namespace lgraph {
namespace import_v3 {



class Importer {
 public:
    struct Config {
        Config() {
            auto cpu_core = std::thread::hardware_concurrency();
            if (cpu_core > 0) {
                generate_sst_threads = cpu_core;
                read_rocksdb_threads = cpu_core;
                parse_file_threads = std::max((uint16_t)1, (uint16_t)(cpu_core/3));
                parse_block_threads = std::max((uint16_t)1, (uint16_t)(cpu_core/3));
            }
        }
        std::string config_file;        // the config file specifying both the scheam & files
        std::string db_dir = "./lgraph_db";  // db data dir to use
        std::string user = "admin";
        std::string password = "73@TuGraph";
        bool is_graphar = false;         // import Graphar file, config file must be absolute path
        std::string graph = "default";   // graph name
        bool delete_if_exists = false;   // force import, delete data if already exists
        bool continue_on_error = false;  // whether to continue when there are data errors
        size_t parse_block_size =
            8 << 20;  // block size used in parser, blocks will be parsed in parallel
        uint16_t parse_block_threads = 5;
        std::string intermediate_dir = "./.import_tmp";  // intermediate data dir
        uint16_t parse_file_threads = 5;
        uint16_t generate_sst_threads = 15;
        uint16_t read_rocksdb_threads = 15;
        size_t vid_num_per_reading = 10000;
        size_t max_size_per_reading = 32*1024*1024;
        bool keep_vid_in_memory = true;
        bool compact = false;
        std::string delimiter = ",";
        bool quiet = false;  // do not print error messages when continue_on_error==true
        bool enable_fulltext_index = false;
        std::string fulltext_index_analyzer = "StandardAnalyzer";
    };

    explicit Importer(Config config);
    void DoImportOffline();

 private:
    AccessControlledDB OpenGraph(Galaxy& galaxy, bool empty_db);
    void OnErrorOffline(const std::string& msg);
    inline Value GetConstRef(const import_v2::DenseString& d) { return Value(d.data(), d.size()); }
    void VertexDataToSST();
    void EdgeDataToSST();
    void VertexPrimaryIndexToLmdb();
    void RocksdbToLmdb();
    void WriteCount();
    cuckoohash_map<std::string, VertexId> key_vid_maps_;  // vertex primary key => vid
    Config config_;
    std::mutex next_vid_lock_;
    std::atomic<VertexId> next_vid_;
    std::mutex next_eid_lock_;
    std::atomic<uint64_t> next_eid_;
    // parse vertex and edge data files
    std::unique_ptr<boost::asio::thread_pool> parse_file_threads_;
    // generate vertex and edge sst files
    std::unique_ptr<boost::asio::thread_pool> generate_sst_threads_;
    std::vector<import_v2::CsvDesc> data_files_;
    import_v2::SchemaDesc schemaDesc_;
    AccessControlledDB* db_;
    std::string sst_files_path_;
    std::string rocksdb_path_;
    std::string vid_path_;
    std::string dirty_data_path_;
    std::mutex exceptions_lock_;
    std::queue<std::exception_ptr> exceptions_;
    std::unique_ptr<rocksdb::DB> rocksdb_vids_;
    std::unordered_map<LabelId, std::atomic<int64_t>> vertex_count_;
    std::unordered_map<LabelId, std::atomic<int64_t>> edge_count_;
    std::unordered_map<LabelId, bool> vlid_detach_;
    std::unordered_map<LabelId, bool> elid_detach_;
};

}  // namespace import_v3
}  // namespace lgraph
