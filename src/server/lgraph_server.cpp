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

#include <csignal>
#include <thread>
#include <boost/log/core.hpp>

#ifndef _WIN32
// disable thread limit on windows
#include <pplx/threadpool.h>
#endif

#include "fma-common/fma_stream.h"
#include "fma-common/hardware_info.h"
#include "server/lgraph_server.h"
#include "core/audit_logger.h"
#include "core/global_config.h"
#include "core/full_text_index.h"
#include "restful/server/rest_server.h"
#include "server/state_machine.h"
#include "server/ha_state_machine.h"
#include "server/db_management_client.h"
#include "server/bolt_server.h"
#include "server/bolt_raft_server.h"

#ifndef _WIN32
#include "brpc/server.h"
#include "butil/logging.h"
namespace brpc {
DECLARE_bool(usercode_in_pthread);
}
#endif
#include <sys/resource.h>

namespace lgraph {
static Signal _kill_signal_;

void shutdown_handler(int sig) {
    LOG_WARN() << FMA_FMT("Received signal {}, shutdown", std::string(strsignal(sig)));
    lgraph_log::LoggerManager::GetInstance().FlushAllSinks();
    _kill_signal_.Notify();
}

void crash_handler(int sig) {
    LOG_ERROR() << FMA_FMT("Received signal {}, crash", std::string(strsignal(sig)));
    lgraph_log::LoggerManager::GetInstance().FlushAllSinks();

    struct sigaction sa{};
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SIG_DFL;
    sigaction(sig, &sa, nullptr);
    kill(getpid(), sig);
}

#ifndef _WIN32

#include <cstdio>

static void SetupSignalHandler() {
    {
        // shutdown
        struct sigaction sa{};
        sa.sa_handler = shutdown_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, nullptr);
        sigaction(SIGTERM, &sa, nullptr);
        sigaction(SIGUSR1, &sa, nullptr);
    }
    {
        // crash
        struct sigaction sa{};
        sa.sa_handler = crash_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_NODEFER;
        sigaction(SIGSEGV, &sa, nullptr);
        sigaction(SIGBUS, &sa, nullptr);
        sigaction(SIGFPE, &sa, nullptr);
        sigaction(SIGILL, &sa, nullptr);
        sigaction(SIGABRT, &sa, nullptr);
    }
}

static void KillAllChildren() { kill(-getpid(), 9); }

#else
static void SetupSignalHandler() { signal(SIGINT, int_handler); }

static void KillAllChildren() {}
#endif

LGraphServer::LGraphServer(std::shared_ptr<lgraph::GlobalConfig> config)
    : config_(std::move(config)) {
    auto &fs = fma_common::FileSystem::GetFileSystem(config_->db_dir);
    if (!fs.IsDir(config_->db_dir)) {
        if (!fs.Mkdir(config_->db_dir)) {
            LOG_ERROR() << "Failed to create the data dir [" << config_->db_dir << "]";
        }
    }
}

LGraphServer::~LGraphServer() { Stop(false); }

int LGraphServer::Start() {
    // assign AccessControllerDB enable_plugin
    AccessControlledDB::SetEnablePlugin(config_->enable_plugin);
    // adjust config
    if (config_->enable_ha && config_->ha_log_dir.empty()) {
#if LGRAPH_SHARE_DIR
        LOG_ERROR() << "HA is enabled, but ha_log_dir is not specified.";
        return -1;
#else
        config_->ha_log_dir = config_->db_dir + "/ha";
#endif
    }
    if (config_->backup_log_dir.empty()) {
        config_->backup_log_dir = config_->db_dir + "/binlog";
    }
    if (config_->snapshot_dir.empty()) {
        config_->snapshot_dir = config_->db_dir + "/snapshot";
    }
    if (config_->ha_snapshot_interval_s > 21 * 24 * 3600) {
        throw std::runtime_error(
            "The maximum value of ha_snapshot_interval_s is 1814400s (21*24*3600s)");
    }

    if (config_->rpc_port == 0) config_->rpc_port = config_->http_port + 1;

    // Setup lgraph log
    lgraph_log::severity_level verbose_level;
    switch (config_->verbose) {
    case 0:
        verbose_level = lgraph_log::ERROR;
        break;
    case 1:
        verbose_level = lgraph_log::INFO;
        break;
    case 2:
    default:
        verbose_level = lgraph_log::DEBUG;
        break;
    }
    size_t max_log_size = config_->max_log_file_size_mb << 20;
    lgraph_log::LoggerManager::GetInstance().Init(config_->log_dir, verbose_level, max_log_size);
    if (config_->rpc_port == 0) config_->rpc_port = config_->http_port + 1;
    // start
    int ret = 0;

    try {
        // If ha is enabled, we need to enable rpc
        if (config_->enable_ha) {
            config_->enable_rpc = true;
            LOG_INFO() << "Server starting in HA mode.";
        }

        // print welcome message
        std::string version;
        version.append(std::to_string(lgraph::_detail::VER_MAJOR))
            .append(".")
            .append(std::to_string(lgraph::_detail::VER_MINOR))
            .append(".")
            .append(std::to_string(lgraph::_detail::VER_PATCH));
        std::ostringstream header;
        header << "\n"
               << "**********************************************************************"
               << "\n"
               << "*                  TuGraph Graph Database v" << version
               << std::string(26 - version.size(), ' ') << "*"
               << "\n"
               << "*                                                                    *"
               << "\n"
               << "*    Copyright(C) 2018-2023 Ant Group. All rights reserved.          *"
               << "\n"
               << "*                                                                    *"
               << "\n"
               << "**********************************************************************"
               << "\n"
               << "Server is configured with the following parameters:\n"
               << config_->FormatAsString();
        LOG_INFO() << header.str();
        struct rlimit rlim{};
        getrlimit(RLIMIT_CORE, &rlim);
        LOG_INFO() << FMA_FMT("Core dump file limit size, soft limit: {}, hard limit: {}",
                              rlim.rlim_cur, rlim.rlim_max);
        std::ifstream file("/proc/sys/kernel/core_pattern");
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                                (std::istreambuf_iterator<char>()));
            LOG_INFO() << "Core dump file path: " << content;
            file.close();
        }

        // starting audit log
        if (config_->audit_log_dir.empty())
            lgraph::AuditLogger::GetInstance().Init(config_->db_dir + "/_audit_log_",
                                                    config_->audit_log_expire);
        else
            lgraph::AuditLogger::GetInstance().Init(config_->audit_log_dir,
                                                    config_->audit_log_expire);
        lgraph::AuditLogger::SetEnable(config_->enable_audit_log);

#ifndef _WIN32
        // setup brpc and braft logger
        if (config_->enable_rpc) {
            int blog_level;
            switch (config_->verbose) {
            case 0:
                blog_level = logging::BLOG_ERROR;
                break;
            case 1:
            case 2:
                blog_level = logging::BLOG_WARNING;
                break;
            case 3:
            default:
                blog_level = logging::BLOG_DEBUG;
                break;
            }
            logging::LoggingSettings blog_setting;
            // blog_setting.logging_dest = logging::LoggingDestination::LOG_NONE;
            logging::InitLogging(blog_setting);
            logging::SetMinLogLevel(blog_level);
            blog_sink_ = std::make_unique<FMALogSink>();
            logging::SetLogSink(blog_sink_.get());
        }
#endif
        // starting services
        lgraph::HaStateMachine::Config config(*config_);
        if (config_->enable_ha) {
            // config_ gets updated when UpdateConfig is called
            state_machine_ = std::make_unique<lgraph::HaStateMachine>(config, config_);
        } else {
            state_machine_ = std::make_unique<lgraph::StateMachine>(config, config_);
        }

#ifndef _WIN32
        // set REST thread limit
        if (config_->thread_limit != 0) {
            try {
                crossplat::threadpool::initialize_with_threads(config_->thread_limit);
            } catch (const std::exception& e) {
                LOG_WARN() << "failed to init cpprest threadpool, exception:" << e.what();
            }
        }
        rpc_service_ = std::make_unique<RPCService>(state_machine_.get());
        // start RPC service
        if (config_->enable_rpc) {
            if (config_->use_pthread) {
                brpc::FLAGS_usercode_in_pthread = true;
            }
            rpc_server_ = std::make_unique<brpc::Server>();
            if (rpc_server_->AddService(rpc_service_.get(), brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
                LOG_WARN() << "Failed to add service to RPC server";
                return -1;
            }
            // start http server
            http_service_ = std::make_unique<http::HttpService>(state_machine_.get());
            if (rpc_server_->AddService(http_service_.get(), brpc::SERVER_DOESNT_OWN_SERVICE) !=
                0) {
                LOG_WARN() << "Failed to add http service to http server";
                return -1;
            }

            std::string rpc_addr =
                fma_common::StringFormatter::Format("{}:{}", config_->bind_host, config_->rpc_port);
            if (config_->enable_ha) {
                if (braft::add_service(rpc_server_.get(), rpc_addr.c_str()) != 0) {
                    LOG_ERROR() << "Failed to add service to RAFT";
                    return -1;
                }
            }
            brpc::ServerOptions brpc_options;
            brpc_options.has_builtin_services = false;
            // if (config_->thread_limit != 0) brpc_options.max_concurrency = config_->thread_limit;
            // brpc_options.num_threads = brpc_options.max_concurrency = 1;
            if (config_->enable_ssl) {
                brpc_options.mutable_ssl_options()->default_cert.certificate =
                    config_->server_cert_file;
                brpc_options.mutable_ssl_options()->default_cert.private_key =
                    config_->server_key_file;
            }

            int max_retries = 5;
            int retries = 0;
            while (retries < max_retries) {
                if (rpc_server_->Start(rpc_addr.c_str(), &brpc_options) != 0) {
                    LOG_WARN() << "RPC server returns -1, try again after 1s";
                } else {
                    break;
                }
                fma_common::SleepS(1);
                retries++;
            }
            if (retries >= max_retries) {
                LOG_ERROR() << "Failed to start RPC server";
                return -1;
            }
            LOG_INFO() << "Listening for RPC on port " << config_->rpc_port;
        }
#endif
        state_machine_->Start();
        if (config_->unlimited_token == 1) {
            state_machine_->SetTokenTimeUnlimited();
        }
        // start rest
        lgraph::RestServer::Config rest_config(*config_);
        rest_server_ = std::make_unique<lgraph::RestServer>(state_machine_.get(),
                                                            rest_config, config_);
        if (config_->bolt_port > 0) {
            if (!bolt::BoltServer::Instance().Start(state_machine_.get(),
                                               config_->bolt_port,
                                                    config_->bolt_io_thread_num)) {
                return -1;
            }
            if (config_->bolt_raft_port > 0) {
                std::string log_path = config_->bolt_raft_logstore_path;
                if (log_path.empty()) {
                    log_path = config_->db_dir + "/raftlog";
                }
                if (config_->bolt_raft_node_id  == 0) {
                    LOG_ERROR() << "bolt_raft_node_id should be greater than 0";
                    return -1;
                }
                if (config_->bolt_raft_init_peers.empty()) {
                    LOG_ERROR() << "bolt_raft_init_peers is empty";
                    return -1;
                }
                bolt_raft::RaftConfig rc;
                rc.tick_interval = config_->bolt_raft_tick_interval;
                rc.heartbeat_tick = config_->bolt_raft_heartbeat_tick;
                rc.election_tick = config_->bolt_raft_election_tick;
                if (!rc.Check()) {
                    return -1;
                }
                bolt_raft::RaftLogStoreConfig rsc;
                rsc.path = log_path;
                rsc.block_cache = config_->bolt_raft_logstore_cache;
                rsc.total_threads = config_->bolt_raft_logstore_threads;
                rsc.keep_logs = config_->bolt_raft_logstore_keep_logs;
                rsc.gc_interval = config_->bolt_raft_logstore_gc_interval;
                if (!rsc.Check()) {
                    return -1;
                }
                if (!bolt_raft::BoltRaftServer::Instance().Start(
                        state_machine_.get(), config_->bolt_raft_port,
                        config_->bolt_raft_node_id, config_->bolt_raft_init_peers,
                        rsc, rc)) {
                    return -1;
                }
            }
        }

        if (config_->enable_rpc) {
            http_service_->Start(config_.get());
        }
        if (config_->reset_admin_password == 1) {
            if (state_machine_->ResetAdminPassword(
                lgraph::_detail::DEFAULT_ADMIN_NAME,
                lgraph::_detail::DEFAULT_ADMIN_PASS, true)) {
                // kill the server
                LOG_INFO() << "Reset admin password successfully, server will exit now";
            } else {
                LOG_ERROR() << "Failed to reset admin password, server will exit now";
            }
            return Stop();
        }
        LOG_INFO() << "Server started.";
    } catch (std::exception &e) {
        _kill_signal_.Notify();
        LOG_WARN() << "Server hit an exception and shuts down abnormally: " << e.what();
        ret = -2;
    }
    return ret;
}

int LGraphServer::WaitTillKilled() {
    try {
        SetupSignalHandler();
        while (!_kill_signal_.Wait(3600)) {
        }
    } catch (std::exception &e) {
        LOG_WARN() << "Server hit an exception and shuts down abnormally: " << e.what();
        return -1;
    }
    return Stop(true);
}

int LGraphServer::Stop(bool force_exit) {
    // if already stopped, just return
    if (!state_machine_) return 0;
    // otherwise, try to stop the services, exit forcefully if necessary
    try {
        LOG_INFO() << "Stopping TuGraph...";
        DBManagementClient::GetInstance().StopHeartbeat();
        if (heartbeat_detect.joinable()) heartbeat_detect.join();
        // the kaishaku watches the server, if exit flag is set and the server cannot be shutdown
        // normally after three seconds, it kills the process
        std::thread kaishaku;
        if (force_exit) {
            kaishaku = std::thread([&]() {
                _kill_signal_.Wait(-1);
                if (!server_exit_.Wait(3)) {
                    LOG_INFO() << "Killing server...";
#ifdef _WIN32
                    exit(1);
#else
                    KillAllChildren();
                    raise(SIGKILL);
#endif
                }
            });
        }
        // server killed
        lgraph::TaskTracker::GetInstance().KillAllTasks();
        if (rest_server_) rest_server_->Stop();
#ifndef _WIN32
        if (config_->enable_ha && state_machine_)
            dynamic_cast<HaStateMachine *>(state_machine_.get())->LeaveGroup();
        if (rpc_server_) rpc_server_->Stop(0);
        rpc_server_.reset();
        rpc_service_.reset();
        if (config_->bolt_port > 0) {
            bolt::BoltServer::Instance().Stop();
            if (config_->bolt_raft_port > 0) {
                bolt_raft::BoltRaftServer::Instance().Stop();
            }
        }
#endif
        if (state_machine_) state_machine_->Stop();
        state_machine_.reset();
        LOG_INFO() << "Server shutdown.";
        server_exit_.Notify();
        if (kaishaku.joinable()) kaishaku.join();
        return 0;
    } catch (std::exception &e) {
        LOG_WARN() << "Server hit an exception and shuts down abnormally: " << e.what();
        return -1;
    }
}
}  // namespace lgraph
