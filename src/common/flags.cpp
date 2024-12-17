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

/*
 * written by botu.wzy
 */

#include <set>
#include <iostream>
#include <spdlog/fmt/fmt.h>
#include "flags.h"

DEFINE_string(mode, "run", "Mode to run the server in. "
              "'run' - run the server directly. "
              "'start/restart/stop' - run the server in daemon mode");
DEFINE_string(data_path, "data", "Directory where the graph data is stored");
DEFINE_string(pid_file, "lgraph.pid", "Pid file");

DEFINE_string(log_path, "log", "Log file path");
DEFINE_string(log_level, "info", "Log level");
DEFINE_uint64(log_max_size, (uint64_t)64*1024*1024, "Maximum size of a single log file");
DEFINE_uint32(log_max_files, 10, "Maximum number of log files");

DEFINE_bool(enable_query_log, false, "Whether to enable query log.");
DEFINE_string(query_log_path, "log", "Query log file path.");
DEFINE_uint64(query_log_max_size, (uint64_t)64*1024*1024, "Maximum size of a single query log file");
DEFINE_uint32(query_log_max_files, 10, "Maximum number of query log files");

DEFINE_uint32(log_flush_interval, 3, "The interval to write the log to file, in seconds.");

DEFINE_string(host, "0.0.0.0", "Host ip");
DEFINE_uint32(bolt_port, 7687, "Bolt port");
DEFINE_uint32(bolt_io_thread_num, 2, "Number of Bolt io thread");

DEFINE_uint64(block_cache, (uint64_t)8*1024*1024*1024, "Block data cache size, in bytes.");
DEFINE_uint64(row_cache, (uint64_t)8*1024*1024*1024, "Row data cache size, in bytes.");
DEFINE_uint64(ft_apply_interval, (uint64_t)1, "Fulltext index WAL auto apply interval, in seconds.");
DEFINE_uint64(vt_apply_interval, (uint64_t)1, "Vector index WAL auto apply interval, in seconds.");
DEFINE_uint64(vt_serialize_interval, (uint64_t)10000, "Vector index serialize interval.");

bool validate_mode(const char* flagname, const std::string& mode) {
    std::set<std::string> vals = {"run", "start", "restart", "stop"};
    if (!vals.count(mode)) {
        std::cerr << fmt::format("Invalid value for --{}: {}", flagname, mode) << std::endl;
        return false;
    }
    return true;
}
DEFINE_validator(mode, &validate_mode);