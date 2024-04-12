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
struct BasicConfigs {
    BasicConfigs()
        : db_dir("./lgraph_db")
        , thread_limit(0)
        , subprocess_max_idle_seconds(600)
        , bind_host("0.0.0.0")
        , enable_ssl(false)
        , server_key_file()
        , server_cert_file()
        , http_port(7071)
        , http_web_dir("./resource")
        , http_disable_auth(false)
        , jwt_secret("fma.ai")
        , enable_rpc(false)
        , rpc_port(9091)
        , use_pthread(false)
        , verbose(1)
        , log_dir()
        , max_log_file_size_mb(256)
        , max_n_log_files(16)
        , audit_log_expire(0)
        , audit_log_dir("./audit_log")
        , backup_log_dir("./binlog")
        , snapshot_dir("./snapshot_log")
        , max_backup_log_file_size((size_t)1 << 30)
        , unlimited_token(false)
        , reset_admin_password(false)
        , enable_realtime_count(true) {}

    BasicConfigs(const BasicConfigs &basicConfigs)
        : db_dir(basicConfigs.db_dir)
          , thread_limit(basicConfigs.thread_limit)
          , subprocess_max_idle_seconds(basicConfigs.subprocess_max_idle_seconds)
          , bind_host(basicConfigs.bind_host)
          , enable_ssl(basicConfigs.enable_ssl)
          , server_key_file(basicConfigs.server_key_file)
          , server_cert_file(basicConfigs.server_cert_file)
          , http_port(basicConfigs.http_port)
          , http_web_dir(basicConfigs.http_web_dir)
          , http_disable_auth(basicConfigs.http_disable_auth)
          , jwt_secret(basicConfigs.jwt_secret)
          , enable_rpc(basicConfigs.enable_rpc)
          , rpc_port(basicConfigs.rpc_port)
          , use_pthread(basicConfigs.use_pthread)
          , verbose(basicConfigs.verbose)
          , log_dir(basicConfigs.log_dir)
          , max_log_file_size_mb(basicConfigs.max_log_file_size_mb)
          , max_n_log_files(basicConfigs.max_n_log_files)
          , audit_log_expire(basicConfigs.audit_log_expire)
          , audit_log_dir(basicConfigs.audit_log_dir)
          , backup_log_dir(basicConfigs.backup_log_dir)
          , snapshot_dir(basicConfigs.snapshot_dir)
          , max_backup_log_file_size(basicConfigs.max_backup_log_file_size)
          , ft_index_options(basicConfigs.ft_index_options)
          , unlimited_token(basicConfigs.unlimited_token)
          , reset_admin_password(basicConfigs.reset_admin_password) {}

    std::string db_dir;  // db
    int thread_limit;                // number of threads, for both rpc and http
    int subprocess_max_idle_seconds;
    // address and ssl
    std::string bind_host;
    bool enable_ssl;
    std::string server_key_file;
    std::string server_cert_file;
    // http
    uint16_t http_port;
    std::string http_web_dir;
    bool http_disable_auth;
    std::string jwt_secret;  // salt for jwt
    // rpc & ha
    bool enable_rpc;
    uint16_t rpc_port;
    bool use_pthread;
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

    // log
    int verbose;
    std::string log_dir;
    size_t max_log_file_size_mb;
    size_t max_n_log_files;
    size_t audit_log_expire;
    std::string audit_log_dir;
    std::string backup_log_dir;
    std::string snapshot_dir;
    size_t max_backup_log_file_size;
    // fulltext index
    FullTextIndexOptions ft_index_options;
    // token time
    bool unlimited_token;
    // reset admin password
    bool reset_admin_password;
    // vertex and edge count
    bool enable_realtime_count{};
    // bolt
    int bolt_port = 0;
    int bolt_io_thread_num = 1;
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
    GlobalConfig()
        : durable(false),
          txn_optimistic(false),
          enable_audit_log(false),
          enable_ip_check(false),
          enable_backup_log(false) {}
    GlobalConfig(const GlobalConfig& rhs)
        : BasicConfigs(rhs),
          durable(rhs.durable.load()),
          txn_optimistic(rhs.txn_optimistic.load()),
          enable_audit_log(rhs.enable_audit_log.load()),
          enable_ip_check(rhs.enable_ip_check.load()),
          enable_backup_log(rhs.enable_backup_log.load()) {}

    // modifiable
    std::atomic<bool> durable;
    std::atomic<bool> txn_optimistic;
    std::atomic<bool> enable_audit_log;
    std::atomic<bool> enable_ip_check;
    std::atomic<bool> enable_backup_log;

    [[nodiscard]] virtual std::map<std::string, std::string> FormatAsOptions() const;
    [[nodiscard]] std::string FormatAsString(size_t heading_space = 2) const;
    [[nodiscard]] virtual std::map<std::string, FieldData> ToFieldDataMap() const;
    static int PrintVersion(std::string &config_file, std::string &cmd, int *argc, char ***argv);
    virtual fma_common::Configuration InitConfig(std::string &cmd);
};
}  // namespace lgraph
