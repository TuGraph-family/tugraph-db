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
#include "fma-common/leveled_log_device.h"
#include "fma-common/rotating_file_log_device.h"

#include "server/lgraph_server.h"
#include "core/audit_logger.h"
#include "core/global_config.h"
#include "core/full_text_index.h"
#include "restful/server/rest_server.h"
#include "server/state_machine.h"
#include "server/ha_state_machine.h"
#include "server/db_management_client.h"

#ifndef _WIN32
#include "brpc/server.h"
#include "butil/logging.h"
namespace brpc {
DECLARE_bool(usercode_in_pthread);
}
#endif

namespace lgraph {
static Signal _kill_signal_;

void int_handler(int x) {
    GENERAL_LOG(INFO) << "!!!!! Received signal " << x << ", exiting... !!!!!";
    _kill_signal_.Notify();
}

#ifndef _WIN32

#include <cstdio>

static void SetupSignalHandler() {
    struct sigaction sa{};
    sa.sa_handler = int_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    int r = sigaction(SIGINT, &sa, nullptr);
    if (r != 0) {
        GENERAL_LOG(ERROR) << "Error setting up SIGINT signal handler: " << strerror(errno);
    }
    r = sigaction(SIGQUIT, &sa, nullptr);
    if (r != 0) {
        GENERAL_LOG(ERROR) << "Error setting up SIGQUIT signal handler: " << strerror(errno);
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
            GENERAL_LOG(ERROR) << "Failed to create the data dir [" << config_->db_dir << "]";
        }
    }
}

LGraphServer::~LGraphServer() { Stop(false); }

int LGraphServer::Start() {
    // adjust config
    if (config_->enable_ha && config_->ha_log_dir.empty()) {
#if LGRAPH_SHARE_DIR
        GENERAL_LOG(ERROR) << "HA is enabled, but ha_log_dir is not specified.";
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
            GENERAL_LOG(INFO) << "Server starting in HA mode.";
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
               << "**********************************************************************" << "\n"
               << "*                  TuGraph Graph Database v" << version
               << std::string(26 - version.size(), ' ') << "*" << "\n"
               << "*                                                                    *" << "\n"
               << "*    Copyright(C) 2018-2023 Ant Group. All rights reserved.          *" << "\n"
               << "*                                                                    *" << "\n"
               << "**********************************************************************" << "\n"
               << "Server is configured with the following parameters:\n"
               << config_->FormatAsString();
        GENERAL_LOG(INFO) << header.str();

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
        if (config_->thread_limit != 0)
            crossplat::threadpool::initialize_with_threads(config_->thread_limit);
        rpc_service_ = std::make_unique<RPCService>(state_machine_.get());
        // start RPC service
        if (config_->enable_rpc) {
            if (config_->use_pthread) {
                brpc::FLAGS_usercode_in_pthread = true;
            }
            rpc_server_ = std::make_unique<brpc::Server>();
            if (rpc_server_->AddService(rpc_service_.get(), brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
                GENERAL_LOG(WARNING) << "Failed to add service to RPC server";
                return -1;
            }
            // start http server
            http_service_ = std::make_unique<http::HttpService>(state_machine_.get());
            if (rpc_server_->AddService(http_service_.get(),
                                        brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
                GENERAL_LOG(WARNING) << "Failed to add http service to http server";
                return -1;
            }

            std::string rpc_addr =
                fma_common::StringFormatter::Format("{}:{}", config_->bind_host, config_->rpc_port);
            if (config_->enable_ha) {
                if (braft::add_service(rpc_server_.get(), rpc_addr.c_str()) != 0) {
                    GENERAL_LOG(ERROR) << "Failed to add service to RAFT";
                    return -1;
                }
            }
            brpc::ServerOptions brpc_options;
            brpc_options.has_builtin_services = false;
            if (config_->thread_limit != 0) brpc_options.max_concurrency = config_->thread_limit;
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
                    GENERAL_LOG(WARNING) << "RPC server returns -1, try again after 1s";
                } else {
                    break;
                }
                fma_common::SleepS(1);
                retries++;
            }
            if (retries >= max_retries) {
                GENERAL_LOG(ERROR) << "Failed to start RPC server";
                return -1;
            }
            GENERAL_LOG(INFO) << "Listening for RPC on port " << config_->rpc_port;
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
        if (config_->enable_rpc) {
            http_service_->Start(config_.get());
        }
        GENERAL_LOG(INFO) << "Server started.";

#ifndef __SANITIZE_ADDRESS__
        heartbeat_detect = std::thread([](){
            // start db management service
            try {
                DBManagementClient::GetInstance().InitChannel("localhost:6091");
            } catch(std::exception& e) {
                GENERAL_LOG(WARNING) << "Failed to init db management channel";
            }
            DBManagementClient::DetectHeartbeat();
        });
#endif
    } catch (std::exception &e) {
        _kill_signal_.Notify();
        GENERAL_LOG(WARNING) << "Server hit an exception and shuts down abnormally: " << e.what();
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
        GENERAL_LOG(WARNING) << "Server hit an exception and shuts down abnormally: " << e.what();
        return -1;
    }
    return Stop(true);
}

int LGraphServer::Stop(bool force_exit) {
    // if already stopped, just return
    if (!state_machine_) return 0;
    // otherwise, try to stop the services, exit forcefully if necessary
    try {
        GENERAL_LOG(INFO) << "Stopping TuGraph...";
        DBManagementClient::exit_flag = true;
        DBManagementClient::hb_cond_.notify_all();
        if (heartbeat_detect.joinable())
            heartbeat_detect.join();
        // the kaishaku watches the server, if exit flag is set and the server cannot be shutdown
        // normally after three seconds, it kills the process
        std::thread kaishaku;
        if (force_exit) {
            kaishaku = std::thread([&]() {
                _kill_signal_.Wait(-1);
                if (!server_exit_.Wait(3)) {
                    GENERAL_LOG(INFO) << "Killing server...";
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

#endif
        if (state_machine_) state_machine_->Stop();
        state_machine_.reset();
        GENERAL_LOG(INFO) << "Server shutdown.";
        server_exit_.Notify();
        if (kaishaku.joinable()) kaishaku.join();
        return 0;
    } catch (std::exception &e) {
        GENERAL_LOG(WARNING) << "Server hit an exception and shuts down abnormally: " << e.what();
        return -1;
    }
}
}  // namespace lgraph
