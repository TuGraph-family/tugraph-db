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

#include <atomic>
#include <string>

#include "core/data_type.h"
#include "fma-common/configuration.h"

namespace lgraph {

struct BrowserOptions {
    bool retain_connection_credentials = true;
    int credential_timeout = 3600;
};

struct BasicConfigs {
    std::string db_dir = "./lgraph_db";  // db
    int thread_limit = 2;                // number of threads, for http
    int subprocess_max_idle_seconds = 600;
    // address and ssl
    std::string bind_host = "0.0.0.0";
    bool enable_ssl = false;
    std::string server_key_file;
    std::string server_cert_file;
    // http
    uint16_t http_port = 7071;
    std::string http_web_dir = "./resource";
    bool http_disable_auth = false;
    std::string jwt_secret = "fma.ai";  // salt for jwt
    // rpc & ha
    bool enable_rpc = false;
    uint16_t rpc_port = 9091;
    bool use_pthread = false;
    bool enable_ha = false;
    std::string ha_conf;
    std::string ha_log_dir;
    int ha_election_timeout_ms = 500;           // election time out in 0.5s
    int ha_snapshot_interval_s = 7 * 24 * 3600;     // snapshot every 24 hours
    int ha_heartbeat_interval_ms = 1000;           // send heartbeat every 1 sec
    int ha_node_offline_ms = 1200000;   // node will be marked as offline after 20 min
    int ha_node_remove_ms = 600000;  // node will be removed from node list after 10 min
    int ha_node_join_group_s = 10;  // node will join group in 10 s
    int ha_bootstrap_role = 0;
    bool ha_is_witness = false;    // node is witness or not
    bool ha_enable_witness_to_leader = false;  // enable witness to leader or not
    std::string ha_first_snapshot_start_time;  // first snapshot start time
                                                    // whose format is "HH:MM:SS",
                                                    // and the default value is ""
                                                    // indicating a random time.
    bool is_cypher_v2 = true;

    // log
    int verbose = 1;
    std::string log_dir;
    size_t max_log_file_size_mb = 256;
    size_t max_n_log_files = 16;
    size_t audit_log_expire = 0;
    std::string audit_log_dir = "./audit_log";
    std::string backup_log_dir = "./binlog";
    std::string snapshot_dir = "./snapshot_log";
    size_t max_backup_log_file_size = (size_t)1 << 30;
    // fulltext index
    FullTextIndexOptions ft_index_options;
    // token time
    bool unlimited_token = false;
    // reset admin password
    bool reset_admin_password = false;
    // vertex and edge count
    bool enable_realtime_count = true;
    // bolt
    int bolt_port = 0;
    int bolt_io_thread_num = 1;
    // bolt raft
    int bolt_raft_port = 0;
    uint64_t bolt_raft_node_id = 0;
    std::string bolt_raft_init_peers;
    uint64_t bolt_raft_tick_interval = 100;
    uint64_t bolt_raft_election_tick = 10;
    uint64_t bolt_raft_heartbeat_tick = 1;

    std::string bolt_raft_logstore_path;
    uint64_t bolt_raft_logstore_cache = 1024;
    uint64_t bolt_raft_logstore_threads = 4;
    uint64_t bolt_raft_logstore_keep_logs = 0;
    uint64_t bolt_raft_logstore_gc_interval = 0;

    // default disable plugin load/delete
    bool enable_plugin = false;
    BrowserOptions browser_options;
};

template <typename T>
inline void AddOption(std::map<std::string, std::string>& m, const std::string& name, const T& d) {
    m[name] = fma_common::ToString(d);
}

inline void AddOption(std::map<std::string, std::string>& m, const std::string& name,
                      const std::string& d) {
    m[name] = d.empty() ? "\"\"" : d;
}

// config for a TuGraph instance
struct GlobalConfig : public BasicConfigs {
    // modifiable
    std::atomic<bool> durable{false};
    std::atomic<bool> txn_optimistic{false};
    std::atomic<bool> enable_audit_log{false};
    std::atomic<bool> enable_ip_check{false};
    std::atomic<bool> enable_backup_log{false};

    [[nodiscard]] virtual std::map<std::string, std::string> FormatAsOptions() const;
    [[nodiscard]] std::string FormatAsString(size_t heading_space = 2) const;
    [[nodiscard]] virtual std::map<std::string, FieldData> ToFieldDataMap() const;
    static int PrintVersion(std::string &config_file, std::string &cmd, int *argc, char ***argv);
    virtual fma_common::Configuration InitConfig(std::string &cmd);
};
}  // namespace lgraph
