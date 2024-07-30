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

#include "core/lightning_graph.h"
#include "core/type_convert.h"
#include "import/column_parser.h"
#include "import/import_config_parser.h"
#include "import/import_data_file.h"
#include "import/import_planner.h"
#include "import/vid_table.h"

namespace lgraph {
class HaStateMachine;

namespace import_v2 {

class ImportOnline;
using lgraph::import::PlanExecutor;

class Importer {
    friend class ImportOnline;  // for using GetSchemaDef
 public:
    struct Config {
        std::string config_file = "";        // the config file specifying both the scheam & files
        std::string db_dir = "./lgraph_db";  // db data dir to use
        std::string user = "admin";
        std::string password = "73@TuGraph";
        std::string graph = "default";   // graph name
        bool delete_if_exists = false;   // force import, delete data if already exists
        bool dry_run = false;            // whether to do dry run, i.e. calculating steps
                                         // without actually committing changes
        bool continue_on_error = false;  // whether to continue when there are data errors

        size_t parse_block_size =
            8 << 20;  // block size used in parser, blocks will be parsed in parallel
        size_t n_parser_threads = 10;                    // number of parser threads
        std::string intermediate_dir = "./.import_tmp";  // intermediate data dir
        size_t bucket_size_mb = 1024;                    // memory size of each bucket
        size_t intermediate_buf_size = 4 << 20;          // buffer size for intermediate files
        size_t n_stichers = 10;
        size_t n_ireaders = 2;
        size_t n_sorters = 4;
        size_t n_packers = 8;
        // no id mapping, simply use uid as vid. user is responsible to make sure
        // uid are unique and of int types
        // bool no_id_mapping = false;
        bool keep_intermediate_files = false;
        bool quiet = false;  // do not print error messages when continue_on_error==true
        std::string delimiter = ",";
        bool enable_fulltext_index = false;
        std::string fulltext_index_analyzer = "StandardAnalyzer";
    };

    typedef std::vector<std::pair<Value, Value>> DataForOneVertex;
    typedef std::vector<DataForOneVertex> PackedKVs;

 protected:
    Config config_;
    std::map<std::string, size_t> n_buckets_;
    ImportDataFile intermediate_file_;
    AllVidTables vid_tables_;
    std::atomic<VidType> next_vid_;
    bool imported_ = false;
    std::atomic<BlobManager::BlobKey> next_blob_key;
    AccessControlledDB OpenGraph(Galaxy& galaxy, bool empty_db);

 public:
    explicit Importer(const Config& config);

    void DoImportOffline();

 protected:
    /**
     * Loads vertex file located in path and write into intermediate file.
     *
     * @param path  Full pathname of the file.
     * @param desc  File description
     */
    void LoadVertexFiles(LightningGraph* db, const std::vector<CsvDesc*>& files,
                         const LabelDesc& ld);

    void LoadEdgeFiles(LightningGraph* db, std::string src_label, std::string dst_label,
                       const LabelDesc ld, const std::vector<CsvDesc*>& files);

    template <typename T>
    struct KeyVidValid {
        T key;
        VidType vid;
        bool is_valid;

        KeyVidValid(const T& k, VidType v) : key(k), vid(v), is_valid(true) {}
        KeyVidValid() : key(T()), vid(0), is_valid(false) {}

        bool operator<(const KeyVidValid& rhs) const { return key < rhs.key; }

        std::string ToString() const { return FMA_FMT("{} : {} : {}\n", key, vid, is_valid); }
    };

    template <class Map>
    class cuckoohash_map_foreach_helper : public Map::locked_table::const_iterator {
     public:
        typedef typename Map::locked_table::const_iterator parent_t;
        explicit cuckoohash_map_foreach_helper(const parent_t& it) : parent_t(it) {}

        template <class F>
        void foreach(F fn){
// TODO(jiazhenjiang): mutil-thread memory issues need to be fixed
// #pragma omp parallel for schedule(dynamic)
            for (decltype(parent_t::index_) index = 0; index < parent_t::buckets_->size();
                 ++index) {
                for (decltype(parent_t::slot_) slot = 0; slot < Map::slot_per_bucket(); ++slot) {
                    if ((*parent_t::buckets_)[index].occupied(slot)) {
                        fn((*parent_t::buckets_)[index].kvpair(slot));
                    }
                }
            }
        }
    };

#if !defined(LGRAPH_CUCKOO_DEBUG) || (!LGRAPH_CUCKOO_DEBUG)
    template <class Map, class F>
    void cuckoohash_map_foreach(Map& map, F fn) {
        const auto& lt = map.lock_table();
        cuckoohash_map_foreach_helper<Map> it(lt.cbegin());
        it.foreach(fn);
    }
#else
    template <class Map, class F>
    void cuckoohash_map_foreach(Map& map, F fn) {
        for (auto& kv : map) fn(kv);
    }
#endif

    template <typename T>
    Value GetConstRef(const T& d) {
        return Value::ConstRef(d);
    }

    inline Value GetConstRef(const DenseString& d) { return Value(d.data(), d.size()); }

    template <FieldType FT>
    void DumpIndexOutOfCore(LightningGraph* db, const std::string& label, const std::string& field);

    void WriteVertex(LightningGraph* db, const std::string& label, const std::string& key_field,
                     FieldType key_type);

 protected:
    void OnErrorOffline(const std::string& msg, bool continue_on_error);
    void OnMissingUidOffline(const std::string& file, const std::string& uid,
                             const std::vector<FieldSpec>& fs,
                             const std::vector<lgraph::FieldData>& fd, bool continue_on_error);
    void OnMissingVertexOffline(const std::string& label, VidType vid, bool continue_on_error);

    DataForOneVertex MakeDataKvs(
        VertexId vid, const DenseString& vdata,
        std::deque<std::tuple<LabelId, TemporalId, VertexId, DenseString>>&& outs,
        std::deque<std::tuple<LabelId, TemporalId, VertexId, DenseString>>&& ins);

    std::vector<PlanExecutor::Action> MakePlan(
        const std::map<std::string, std::vector<CsvDesc*>>& v_label_files,
        const std::map<std::pair<std::string, std::string>,
                       std::map<std::string, std::vector<CsvDesc*>>>& src_dst_files,
        std::vector<std::string>& vid_label);
};
}  // namespace import_v2
}  // namespace lgraph
