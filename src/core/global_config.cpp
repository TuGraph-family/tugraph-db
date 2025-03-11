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

#include "fma-common/string_formatter.h"

#include "core/global_config.h"

std::map<std::string, std::string> lgraph::GlobalConfig::FormatAsOptions() const {
    std::map<std::string, std::string> options;
    AddOption(options, "DB directory", db_dir);
    AddOption(options, "thread limit", thread_limit);
    AddOption(options, "subprocess idle limit", subprocess_max_idle_seconds);
    AddOption(options, "bind host", bind_host);
    AddOption(options, "SSL enable", enable_ssl);
    if (enable_ssl) {
        AddOption(options, "SSL key file", server_key_file);
        AddOption(options, "SSL cert file", server_cert_file);
    }
    AddOption(options, "HTTP port", http_port);
    AddOption(options, "HTTP web dir", http_web_dir);
    AddOption(options, "disable auth", http_disable_auth);
    AddOption(options, "RPC enable", enable_rpc);
    if (enable_rpc) {
        AddOption(options, "RPC port", rpc_port);
    }
    AddOption(options, "log verbose", verbose);
    AddOption(options, "log dir", log_dir);
    AddOption(options, "audit log enable", enable_audit_log);
    if (enable_audit_log) {
        AddOption(options, "audit log dir", audit_log_dir);
        AddOption(options, "audit log expire(hours)", audit_log_expire);
    }
    AddOption(options, "durable", durable);
    AddOption(options, "optimistic transaction", txn_optimistic);
    AddOption(options, "Backup log enable", enable_backup_log);
    AddOption(options, "Whether the token is unlimited", unlimited_token);
    AddOption(options, "reset admin password if you forget", reset_admin_password);
    AddOption(options, "HA enable", enable_ha);
    if (enable_ha) {
        AddOption(options, "HA init group", ha_conf);
        AddOption(options, "HA log dir", ha_log_dir);
        AddOption(options, "HA election timeout(ms)", ha_election_timeout_ms);
        AddOption(options, "HA snapshot interval(s)", ha_snapshot_interval_s);
        AddOption(options, "HA heartbeat interval(ms)", ha_heartbeat_interval_ms);
        AddOption(options, "HA wait unreachable(ms)", ha_node_offline_ms);
        AddOption(options, "HA wait dead(ms)", ha_node_remove_ms);
        AddOption(options, "HA node join(s)", ha_node_join_group_s);
        AddOption(options, "Bootstrap Role", ha_bootstrap_role);
    }
    AddOption(options, "bolt port", bolt_port);
    AddOption(options, "number of bolt io threads", bolt_io_thread_num);
    AddOption(options, "bolt raft port", bolt_raft_port);
    AddOption(options, "bolt raft node id", bolt_raft_node_id);
    return options;
}

std::string lgraph::GlobalConfig::FormatAsString(size_t heading_space) const {
    std::map<std::string, std::string> options = FormatAsOptions();
    size_t max_name_len = 0;
    for (auto& kv : options) max_name_len = std::max(kv.first.size(), max_name_len);
    std::string ret;
    size_t n = 0;
    for (auto& kv : options) {
        if (n != 0) ret.append("\n");
        n++;
        ret.append(heading_space, ' ')
            .append(kv.first)
            .append(":")
            .append(max_name_len - kv.first.size() + 2, ' ')
            .append(kv.second);
    }
    return ret;
}

std::map<std::string, lgraph::FieldData> lgraph::GlobalConfig::ToFieldDataMap() const {
    std::map<std::string, lgraph::FieldData> v;
    // v["db_dir"] = FieldData(fs.db_dir);
    v["thread_limit"] = FieldData(thread_limit);
    v["subprocess_max_idle_seconds"] = FieldData(subprocess_max_idle_seconds);
    v["bind_host"] = FieldData(bind_host);
    v["enable_ssl"] = FieldData(enable_ssl);
    if (enable_ssl) {
        // v["server_cert"] = FieldData(server_cert_file);
        // v["server_key"] = FieldData(server_key_file);
    }
    v["port"] = FieldData(http_port);
    // v["resource_dir"] = FieldData(http_web_dir);
    v["disable_auth"] = FieldData(http_disable_auth);
    v["enable_rpc"] = FieldData(enable_rpc);
    if (enable_rpc) {
        v["rpc_port"] = FieldData(rpc_port);
    }
    v["verbose"] = FieldData(verbose);
    // v["log_dir"] = FieldData(log_dir);
    if (enable_audit_log) {
        // v["audit_log_dir"] = FieldData(audit_log_dir);
        v["audit_log_expire"] = FieldData((int64_t)audit_log_expire);
    }
    v["enable_backup_log"] = FieldData(enable_backup_log);
    v[lgraph::_detail::OPT_DB_DURABLE] = FieldData(durable);
    v[lgraph::_detail::OPT_TXN_OPTIMISTIC] = FieldData(txn_optimistic);
    v[lgraph::_detail::OPT_IP_CHECK_ENABLE] = FieldData(enable_ip_check);
    v[lgraph::_detail::OPT_AUDIT_LOG_ENABLE] = FieldData(enable_audit_log);
    v["enable_fulltext_index"] = FieldData(ft_index_options.enable_fulltext_index);
    if (ft_index_options.enable_fulltext_index) {
        v["fulltext_analyzer"] = FieldData(ft_index_options.fulltext_analyzer);
        v["fulltext_commit_interval"] = FieldData(ft_index_options.fulltext_commit_interval);
        v["fulltext_refresh_interval"] = FieldData(ft_index_options.fulltext_refresh_interval);
    }
    v["enable_ha"] = FieldData(enable_ha);
    if (enable_ha) {
        v["ha_conf"] = FieldData(ha_conf);
        v["ha_log_dir"] = FieldData(ha_log_dir);
        v["ha_election_timeout_ms"] = FieldData(ha_election_timeout_ms);
        v["ha_snapshot_interval_s"] = FieldData(ha_snapshot_interval_s);
        v["ha_heartbeat_interval_ms"] = FieldData(ha_heartbeat_interval_ms);
        v["ha_node_offline_ms"] = FieldData(ha_node_offline_ms);
        v["ha_node_remove_ms"] = FieldData(ha_node_remove_ms);
        v["ha_node_join_group_s"] = FieldData(ha_node_join_group_s);
        v["ha_bootstrap_role"] = FieldData(ha_bootstrap_role);
    }
    v["browser.credential_timeout"] = FieldData(browser_options.credential_timeout);
    v["browser.retain_connection_credentials"] =
        FieldData(browser_options.retain_connection_credentials);
    return v;
}
int lgraph::GlobalConfig::PrintVersion(std::string &config_file, std::string &cmd,
                                       int *argc, char ***argv) {
    bool print_version = false;
    {
        fma_common::Configuration conf;
        conf.Add(config_file, "c,config", true).Comment("Path to the config file");
        conf.Add(cmd, "d,mode", true)
            .Comment(
                "Mode to run the server in. 'run' - run the server directly. "
                "'start/restart/stop' - run the server in daemon mode")
            .SetPossibleValues({"run", "start", "stop", "restart"});
        conf.AddBinary(print_version, "version", true)
            .Comment("Print the version of this software.");
        conf.ExitAfterHelp(false);
        conf.ParseAndRemove(argc, argv);
        conf.Finalize();
    }
    // check if dumping version
    if (print_version) {
        std::string version;
        version.append(std::to_string(lgraph::_detail::VER_MAJOR))
            .append(".")
            .append(std::to_string(lgraph::_detail::VER_MINOR))
            .append(".")
            .append(std::to_string(lgraph::_detail::VER_PATCH));
        LOG_INFO() << "TuGraph v" << version << ", compiled from " << GIT_BRANCH
                  << " branch, commit " << GIT_COMMIT_HASH << " (web commit " << WEB_GIT_COMMIT_HASH
                  << ").";
        LOG_INFO() << "  CPP compiler version: " << CXX_COMPILER_ID << " " << CXX_COMPILER_VERSION
                  << ".";
        LOG_INFO() << "  Python version : " << PYTHON_LIB_VERSION << ".";
        return 0;
    }
    return 1;
}

fma_common::Configuration lgraph::GlobalConfig::InitConfig
    (std::string &cmd) {
    // configure parameters
    bool standalone = (cmd == "run");
    std::string cwd = fma_common::FileSystem::GetWorkingDirectory();
    durable = false;
    db_dir = cwd + "/lgraph_db";
    bind_host = "0.0.0.0";
    log_dir = standalone ? "" : "/var/log/lgraph/";
    http_port = 7070;
    http_web_dir = cwd + "/resource";
    server_cert_file = cwd + "/server-cert.pem";
    server_key_file = cwd + "/server-key.pem";
    enable_ssl = false;
    verbose = 1;
    enable_audit_log = false;
    audit_log_expire = 0;
    audit_log_dir = "";
    backup_log_dir = "";
    snapshot_dir = "";
    thread_limit = 0;
    unlimited_token = false;
    enable_realtime_count = true;
    reset_admin_password = false;
    // fulltext index
    ft_index_options.enable_fulltext_index = false;
    ft_index_options.fulltext_commit_interval = 0;
    ft_index_options.fulltext_refresh_interval = 0;
    ft_index_options.fulltext_analyzer = "StandardAnalyzer";

    // HA
    enable_ha = false;
    ha_bootstrap_role = 0;
    ha_log_dir = "";
    ha_election_timeout_ms = 500;
    ha_snapshot_interval_s = 3600 * 24 * 7;
    ha_heartbeat_interval_ms = 1000;
    ha_node_offline_ms = 600 * 1000;
    ha_node_remove_ms = 1200 * 1000;
    ha_node_join_group_s = 10;
    ha_is_witness = false;
    ha_enable_witness_to_leader = false;
    ha_first_snapshot_start_time = "";

    // bolt
    bolt_port = 0;
    bolt_io_thread_num = 1;
    // default disable plugin load/delete
    enable_plugin = false;
    bolt_raft_port = 0;
    bolt_raft_node_id = 0;

    bolt_raft_tick_interval = 100;
    bolt_raft_election_tick = 10;
    bolt_raft_heartbeat_tick = 1;

    bolt_raft_logstore_cache = 1024;  // MB
    bolt_raft_logstore_threads = 4;
    bolt_raft_logstore_keep_logs = 1000000;
    bolt_raft_logstore_gc_interval = 10;  // Minute

    // parse options
    fma_common::Configuration argparser;
    argparser.Add(durable, lgraph::_detail::OPT_DB_DURABLE, true)
        .Comment(
            "Whether to run the server in durable mode."
            " When durable is set to true, write transactions are flushed to disk immediately"
            " after commit."
            " This will cause a lot of IOs and cause the write operation to slow down, especially"
            " when you have a slow disk. On the other hand, if durable is set to false, newly"
            " updated data may be lost since they are not flushed to disk yet.");
    argparser.Add(db_dir, "directory", true)
        .Comment("Directory where the db files are stored.");
    argparser.Add(bind_host, "host", true).Comment("Host on which the restful server runs.");
    argparser.Add(log_dir, "log_dir", true)
        .Comment("Location where the log file will be stored. Empty string means log to stderr.");
    argparser.Add(max_log_file_size_mb, "log_file_size", true)
        .Comment("Max size in MB for each log file before splitting.")
        .SetMin(1)
        .SetMax(4096);
    argparser.Add(max_n_log_files, "max_n_log_files", true)
        .Comment("Max number of log files to keep.")
        .SetMin(1)
        .SetMax(std::numeric_limits<size_t>::max());
    argparser.Add(enable_audit_log, lgraph::_detail::OPT_AUDIT_LOG_ENABLE, true)
        .Comment("Whether to enable audit log.");
    argparser.Add(audit_log_expire, "audit_log_expire", true)
        .Comment("When the audit log expire (in hour).");
    argparser.Add(audit_log_dir, "audit_log_dir", true)
        .Comment("Directory where the audit log files are stored..");
    argparser.Add(http_port, "port", true).Comment("Port on which the restful server runs.");
    argparser.Add(http_web_dir, "web", true)
        .Comment("Directory for the web resource files.");
    argparser.Add(server_cert_file, "server_cert", true).Comment("SSL server certificate.");
    argparser.Add(server_key_file, "server_key", true).Comment("SSL server private key.");
    argparser.Add(enable_ssl, "ssl_auth", true).Comment("Whether to authenticate by ssl.");
    argparser.Add(verbose, "verbose,v", true)
        .Comment("How verbose the output should be. 0-only warnings and errors; 1-normal; 2-debug");
    argparser.Add(enable_rpc, "enable_rpc", true).Comment("Whether to enable RPC server.");
    argparser.Add(rpc_port, "rpc_port", true).Comment("The RPC port used in HA mode.");
    argparser.Add(use_pthread, "use_pthread", true)
        .Comment("Whether to use pthread mode in brpc, default is bthread.");
    argparser.Add(txn_optimistic, lgraph::_detail::OPT_TXN_OPTIMISTIC, true)
        .Comment("Enable optimistic multi-writer transaction for Cypher.");
    argparser.Add(http_disable_auth, "disable_auth", true)
        .Comment("Disable authentication for REST.");
    argparser.Add(enable_ip_check, lgraph::_detail::OPT_IP_CHECK_ENABLE, true)
        .Comment("Enable IP whitelist check.");
    argparser.Add(subprocess_max_idle_seconds, "idle_seconds", true)
        .SetMin(0)
        .Comment("Maximum number of seconds during which a subprocess can be idle.");
    argparser.Add(enable_backup_log, "enable_backup_log", true)
        .Comment("Whether to enable backup logging.");
    argparser.Add(backup_log_dir, "backup_log_dir", true)
        .Comment("Directory to store the backup files.");
    argparser.Add(snapshot_dir, "snapshot_dir", true)
        .Comment("Directory to store the snapshot files.");
    argparser.Add(thread_limit, "thread_limit", true)
        .Comment("Maximum number of threads to use in http web server");
    argparser.Add(unlimited_token, "unlimited_token", true)
        .Comment("Unlimited token for TuGraph.");
    argparser.Add(reset_admin_password, "reset_admin_password", true)
        .Comment("Reset admin password if you forget.");
    argparser.Add(enable_realtime_count, "realtime vertex and edge count", true)
        .Comment("Whether to enable realtime vertex and edge count.");

#if LGRAPH_ENABLE_FULLTEXT_INDEX
    argparser.Add(ft_index_options.enable_fulltext_index, "enable_fulltext_index", true)
        .Comment("Whether to enable fulltext index.");
    argparser.Add(ft_index_options.fulltext_analyzer, "fulltext_analyzer", true)
        .SetPossibleValues({"SmartChineseAnalyzer", "StandardAnalyzer"})
        .Comment("FullText index text analyzer");
    argparser
        .Add(ft_index_options.fulltext_commit_interval, "fulltext_commit_interval", true)
        .Comment("FullText index writer commit interval");
    argparser
        .Add(ft_index_options.fulltext_refresh_interval, "fulltext_refresh_interval", true)
        .Comment("FullText index reader refresh interval");
#endif
    argparser.Add(ha_log_dir, "ha_log_dir", true)
        .Comment("Directory where the HA log is stored, required in HA mode.");
    argparser.Add(enable_ha, "enable_ha", true).Comment("Whether to enable HA mode.");
    argparser.Add(ha_bootstrap_role, "ha_bootstrap_role", true)
        .Comment("Bootstrap role, 0 for no bootstrap, 1 for bootstrap_leader, "
            "2 for bootstrap_follower")
        .SetMin(0)
        .SetMax(2);
    argparser.Add(ha_conf, "ha_conf", true)
        .Comment("Initial list of machines in the form of host1:port1,host2:port2");
    argparser.Add(ha_snapshot_interval_s, "ha_snapshot_interval_s", true)
        .Comment("Snapshot interval in seconds.");
    argparser.Add(ha_election_timeout_ms, "ha_election_timeout_ms", true)
        .Comment("leader step down timeout in ms.");
    argparser.Add(ha_heartbeat_interval_ms, "ha_heartbeat_interval_ms", true)
        .Comment("Interval for heartbeat in ms.");
    argparser.Add(ha_node_offline_ms, "ha_node_offline_ms", true)
        .Comment("Peer considered failed and marked OFFLINE after this duration.");
    argparser.Add(ha_node_remove_ms, "ha_node_remove_ms", true)
        .Comment("Node considered completly dead and removed from list after this duration.");
    argparser.Add(ha_node_join_group_s, "ha_node_join_group_s", true)
        .Comment("Node joined replicated group during this duration.");
    argparser.Add(ha_is_witness, "ha_is_witness", true)
        .Comment("Node is witness (donot have data & can not apply request) or not.");
    argparser.Add(ha_enable_witness_to_leader, "ha_enable_witness_to_leader", true)
        .Comment("Node is witness (donot have data & can not apply request) or not.");
    argparser.Add(ha_first_snapshot_start_time, "ha_first_snapshot_start_time", true)
        .Comment(R"("First snapshot start time whose format is "HH:MM:SS", and the default value is "
            "" indicating a random time.)");
    argparser.Add(bolt_port, "bolt_port", true)
        .Comment("Bolt protocol port.");
    argparser.Add(bolt_io_thread_num, "bolt_io_thread_num", true)
        .Comment("Number of bolt io threads.");
    argparser.Add(enable_plugin, "enable_plugin", true)
        .Comment("Enable load/delete procedure.");
    argparser.Add(browser_options.credential_timeout, "browser.credential_timeout", true)
        .Comment("Config the timeout of browser credentials stored in local storage.");
    argparser.Add(browser_options.retain_connection_credentials,
                  "browser.retain_connection_credentials", true)
        .Comment("Config browser whether to store user credentials in local storage.");
    argparser.Add(is_cypher_v2,
                  "is_cypher_v2", true)
        .Comment("Config browser whether to store user credentials in local storage.");
    argparser.Add(bolt_raft_port, "bolt_raft_port", true)
    .Comment("Bolt raft port.");
    argparser.Add(bolt_raft_node_id, "bolt_raft_node_id", true)
    .Comment("Bolt raft node id.");
    argparser.Add(bolt_raft_init_peers, "bolt_raft_init_peers", true)
    .Comment("Bolt raft initial member information.");

    argparser.Add(bolt_raft_tick_interval, "bolt_raft_tick_interval", true)
        .Comment("Bolt raft tick interval.");
    argparser.Add(bolt_raft_heartbeat_tick, "bolt_raft_heartbeat_tick", true)
        .Comment("Bolt raft heartbeat tick.");
    argparser.Add(bolt_raft_election_tick, "bolt_raft_election_tick", true)
        .Comment("Bolt raft election tick.");

    argparser.Add(bolt_raft_logstore_cache, "bolt_raft_logstore_cache", true)
        .Comment("Bolt raft logstore cache in MB.");
    argparser.Add(bolt_raft_logstore_threads, "bolt_raft_logstore_threads", true)
        .Comment("Bolt raft logstore threads.");
    argparser.Add(bolt_raft_logstore_keep_logs, "bolt_raft_logstore_keep_logs", true)
        .Comment("Maximum number of raft logs to keep.");
    argparser.Add(bolt_raft_logstore_gc_interval, "bolt_raft_logstore_gc_interval", true)
        .Comment("Interval of checking and GC raft log.");

    return argparser;
}
