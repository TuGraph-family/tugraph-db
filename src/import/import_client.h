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

#include <signal.h>
#include <string>

#include "fma-common/fma_stream.h"

#include "import/import_config_parser.h"

namespace lgraph {
namespace import_v2 {

class OnlineImportClient {
 public:
    struct Config {
        std::string config_file;
        std::string url = "http://127.0.0.1:7071/";
        std::string username = lgraph::_detail::DEFAULT_ADMIN_NAME;
        std::string password = lgraph::_detail::DEFAULT_ADMIN_PASS;
        bool continue_on_error = false;
        size_t skip_packages = 0;
        std::string graph_name = "default";
        std::string delimiter = ",";
        std::string progress_log_file;
        bool breakpoint_continue = false;
        bool delete_if_exists = false;
        size_t parse_block_size =
            8 << 20;  // block size used in parser, blocks will be parsed in parallel
        uint16_t parse_block_threads = 5;
        uint16_t parse_file_threads = 5;
        uint16_t generate_sst_threads = 15;
        uint16_t read_rocksdb_threads = 15;
        size_t vid_num_per_reading = 10000;
        size_t max_size_per_reading = 32*1024*1024;
        bool compact = false;
        bool keep_vid_in_memory = true;
        bool enable_fulltext_index = false;
        std::string fulltext_index_analyzer = "StandardAnalyzer";
        std::string path;
        bool remote = false;
    };
    explicit OnlineImportClient(const Config& config);
    void DoImport();
    void DoFullImport() const;
    void DoFullImportFile() const;

 protected:
    Config config_;

    static std::atomic<bool> exit_flag_;
    static void SignalHandler(int signum);

    static std::string FormatPercent(double p);
    static std::string FormatBytes(double n_bytes, int nums = 3);

 private:
    size_t ReadProcessedPackages(std::ifstream& is);
    void WriteProcessedLinePackages(std::ofstream& os, size_t packages);
};

}  // namespace import_v2
}  // namespace lgraph
