/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <boost/endian/conversion.hpp>
#include "rocksdb/db.h"
#include "rocksdb/table.h"
#include "core/lightning_graph.h"
#include "core/type_convert.h"
#include "import/column_parser.h"
#include "import/import_config_parser.h"
#include "import/dense_string.h"

namespace lgraph {
namespace import_v3 {

template<class T>
void encodeNumToStr(T num, std::string& ret) {
    static_assert(std::is_same_v<T, int8_t> ||
                  std::is_same_v<T, int16_t> ||
                  std::is_same_v<T, int32_t> ||
                  std::is_same_v<T, int64_t> ||
                  std::is_same_v<T, float> ||
                  std::is_same_v<T, double>);

    if constexpr(std::is_same_v<T, int8_t>) {
        uint8_t val = uint8_t(num ^ (uint8_t)1<<7);
        ret.append((const char*)&val, sizeof(val));
    } else if constexpr(std::is_same_v<T, int16_t>) {
        uint16_t val = uint16_t(num ^ (uint16_t)1<<15);
        boost::endian::native_to_big_inplace(val);
        ret.append((const char*)&val, sizeof(val));
    } else if constexpr(std::is_same_v<T, int32_t>) {
        uint32_t val = uint32_t(num ^ (uint32_t)1<<31);
        boost::endian::native_to_big_inplace(val);
        ret.append((const char*)&val, sizeof(val));
    } else if constexpr(std::is_same_v<T, int64_t>) {
        uint64_t val = uint64_t(num ^ (uint64_t)1<<63);
        boost::endian::native_to_big_inplace(val);
        ret.append((const char*)&val, sizeof(val));
    } else if constexpr(std::is_same_v<T, float>) {
        uint32_t val = *(uint32_t*)&num;
        if (num >= 0) {
            val |= (uint32_t)1<<31;
        } else {
            val = ~val;
        }
        boost::endian::native_to_big_inplace(val);
        ret.append((const char*)&val, sizeof(val));
    } else if constexpr(std::is_same_v<T, double>) {
        uint64_t val = *(uint64_t*)&num;
        if (num >= 0) {
            val |= (uint64_t)1<<63;
        } else {
            val = ~val;
        }
        boost::endian::native_to_big_inplace(val);
        ret.append((const char*)&val, sizeof(val));
    }
}

template<class T>
T decodeStrToNum(const char* str) {
    static_assert(std::is_same_v<T, int8_t> ||
                  std::is_same_v<T, int16_t> ||
                  std::is_same_v<T, int32_t> ||
                  std::is_same_v<T, int64_t> ||
                  std::is_same_v<T, float> ||
                  std::is_same_v<T, double>);

    if constexpr(std::is_same_v<T, int8_t>) {
        uint8_t newVal = *(uint8_t*)str;
        return int8_t(newVal ^ (uint8_t)1<<7);
    } else if constexpr(std::is_same_v<T, int16_t>) {
        uint16_t newVal = *(uint16_t*)str;
        boost::endian::big_to_native_inplace(newVal);
        return int16_t(newVal ^ (uint16_t)1<<15);
    } else if constexpr(std::is_same_v<T, int32_t>) {
        uint32_t newVal = *(uint32_t*)str;
        boost::endian::big_to_native_inplace(newVal);
        return int32_t(newVal ^ (uint32_t)1<<31);
    } else if constexpr(std::is_same_v<T, int64_t>) {
        uint64_t newVal = *(uint64_t*)str;
        boost::endian::big_to_native_inplace(newVal);
        return int64_t(newVal ^ (uint64_t)1<<63);
    } else if constexpr(std::is_same_v<T, float>) {
        uint32_t newVal = *(uint32_t*)str;
        boost::endian::big_to_native_inplace(newVal);
        if ((newVal & (uint32_t)1<<31) > 0) {
            newVal &= ~((uint32_t)1<<31);
        } else {
            newVal = ~newVal;
        }
        return *(float*)(&newVal);
    } else if constexpr(std::is_same_v<T, double>) {
        uint64_t newVal = *(uint64_t*)str;
        boost::endian::big_to_native_inplace(newVal);
        if ((newVal & (uint64_t)1<<63) > 0) {
            newVal &= ~((uint64_t)1<<63);
        } else {
            newVal = ~newVal;
        }
        return *(double*)(&newVal);
    }
}

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
    static void AppendFieldData(std::string& ret, const FieldData& data);
    void OnErrorOffline(const std::string& msg);
    inline Value GetConstRef(const import_v2::DenseString& d) { return Value(d.data(), d.size()); }
    void VertexDataToSST();
    void EdgeDataToSST();
    void VertexPrimaryIndexToLmdb();
    void RocksdbToLmdb();
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
    std::mutex exceptions_lock_;
    std::queue<std::exception_ptr> exceptions_;
    std::unique_ptr<rocksdb::DB> rocksdb_vids_;
};

}  // namespace import_v3
}  // namespace lgraph
