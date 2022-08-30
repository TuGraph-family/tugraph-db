/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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

#ifndef _WIN32
#include "brpc/server.h"
#include "butil/logging.h"
#endif

namespace lgraph {
static Signal _kill_signal_;

void int_handler(int x) {
    FMA_LOG() << "!!!!! Received signal " << x << ", exiting... !!!!!";
    _kill_signal_.Notify();
}

#ifndef _WIN32

#include <cstdio>

static void SetupSignalHandler() {
    struct sigaction sa;
    sa.sa_handler = int_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    int r = sigaction(SIGINT, &sa, nullptr);
    if (r != 0) {
        FMA_ERR() << "Error setting up SIGINT signal handler: " << strerror(errno);
    }
    r = sigaction(SIGQUIT, &sa, nullptr);
    if (r != 0) {
        FMA_ERR() << "Error setting up SIGQUIT signal handler: " << strerror(errno);
    }
}

static void KillAllChildren() { kill(-getpid(), 9); }

#else
static void SetupSignalHandler() { signal(SIGINT, int_handler); }

static void KillAllChildren() {}
#endif

LGraphServer::LGraphServer(std::shared_ptr<lgraph::GlobalConfig> config) : config_(config) {
    auto &fs = fma_common::FileSystem::GetFileSystem(config_->db_dir);
    if (!fs.IsDir(config_->db_dir)) {
        if (!fs.Mkdir(config_->db_dir)) {
            FMA_ERR() << "Failed to create the data dir [" << config_->db_dir << "]";
        }
    }
}

LGraphServer::~LGraphServer() { Stop(false); }

void LGraphServer::AdjustConfig() {
    if (config_->backup_log_dir.empty()) {
        config_->backup_log_dir = config_->db_dir + "/binlog";
    }
    if (config_->snapshot_dir.empty()) {
        config_->snapshot_dir = config_->db_dir + "/snapshot";
    }

    // setup logger
    fma_common::LogLevel level;
    switch (config_->verbose) {
    case 0:
        level = fma_common::LL_ERROR;
        break;
    case 1:
        level = fma_common::LL_INFO;
        break;
    case 2:
    default:
        level = fma_common::LL_DEBUG;
        break;
    }
    fma_common::Logger::Get().SetLevel(level);
    if (!config_->log_dir.empty()) {
        size_t max_log_size = config_->max_log_file_size_mb << 20;
        fma_common::file_system::MkDir(config_->log_dir);
        std::map<fma_common::LogLevel, std::shared_ptr<fma_common::LogDevice>> devices = {
            {fma_common::LogLevel::LL_INFO,
             std::shared_ptr<fma_common::LogDevice>(new fma_common::RotatingFileLogDevice(
                 config_->log_dir, "lgraph.log", max_log_size))},
            {fma_common::LogLevel::LL_WARNING,
             std::shared_ptr<fma_common::LogDevice>(new fma_common::RotatingFileLogDevice(
                 config_->log_dir, "lgraph.err", max_log_size))},
        };
        std::shared_ptr<fma_common::LogDevice> log_device(
            new fma_common::LeveledLogDevice(devices));
        fma_common::Logger::Get().SetDevice(log_device);
    }
    fma_common::Logger::Get().SetFormatter(
        std::shared_ptr<fma_common::LogFormatter>(new fma_common::TimedModuleLogFormatter()));

    if (config_->rpc_port == 0) config_->rpc_port = config_->http_port + 1;

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
        logging::SetLogSink(&blog_sink_);
    }
#endif
    // starting audit log
    if (config_->audit_log_dir.empty())
        lgraph::AuditLogger::GetInstance().Init(config_->db_dir + "/_audit_log_",
                                                config_->audit_log_expire);
    else
        lgraph::AuditLogger::GetInstance().Init(config_->audit_log_dir,
                                                config_->audit_log_expire);
    lgraph::AuditLogger::GetInstance().SetEnable(config_->enable_audit_log);
}

int LGraphServer::MakeStateMachine() {
    // print welcome message
    std::string version;
    version.append(std::to_string(lgraph::_detail::VER_MAJOR))
        .append(".")
        .append(std::to_string(lgraph::_detail::VER_MINOR))
        .append(".")
        .append(std::to_string(lgraph::_detail::VER_PATCH));
    FMA_LOG() << "**********************************************************************";
    FMA_LOG() << "*                  TuGraph Graph Database v" << version
              << std::string(26 - version.size(), ' ') << "*";
    FMA_LOG() << "*                                                                    *";
    FMA_LOG() << "*    Copyright(C) 2018-2021 Ant Group. All rights reserved.          *";
    FMA_LOG() << "*                                                                    *";
    FMA_LOG() << "**********************************************************************";
    FMA_LOG() << "Server is configured with the following parameters:\n"
              << config_->FormatAsString();
    lgraph::StateMachine::Config config(*config_);
    state_machine_.reset(new lgraph::StateMachine(config, config_));
    return 0;
}

int LGraphServer::StartRpcService() {
#ifndef _WIN32
    rpc_service_.reset(new RPCService(state_machine_.get()));
    rpc_server_.reset(new brpc::Server());
    if (rpc_server_->AddService(rpc_service_.get(), brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        FMA_ERR_STREAM(logger_) << "Failed to add service to RPC server";
        return -1;
    }
    return 0;
#else
    return -1;
#endif
}

void LGraphServer::KillServer() {
    lgraph::TaskTracker::GetInstance().KillAllTasks();
    if (rest_server_) rest_server_->Stop();
#ifndef _WIN32
    if (rpc_server_) rpc_server_->Stop(0);
    rpc_server_.reset();
    rpc_service_.reset();
#endif
    if (state_machine_) state_machine_->Stop();
    state_machine_.reset();
    FMA_INFO_STREAM(logger_) << "Server shutdown.";
    server_exit_.Notify();
}

int LGraphServer::Start() {
    // adjust config
    AdjustConfig();
    // start
    int ret = 0;
    try {
        if (MakeStateMachine() == -1)
            return -1;

#ifndef _WIN32
        // set REST thread limit
        if (config_->thread_limit != 0)
            crossplat::threadpool::num_threads() = config_->thread_limit;

        if (config_->enable_rpc) {
            // start RPC service
            if (StartRpcService() == -1)
                return -1;
            // start RPC server
            std::string rpc_addr =
                fma_common::StringFormatter::Format("{}:{}", config_->bind_host, config_->rpc_port);

            brpc::ServerOptions brpc_options;
            brpc_options.has_builtin_services = false;
            if (config_->thread_limit != 0) brpc_options.max_concurrency = config_->thread_limit;
            if (rpc_server_->Start(rpc_addr.c_str(), &brpc_options) != 0) {
                FMA_ERR_STREAM(logger_) << "Failed to start RPC server";
                return -1;
            }
            FMA_INFO_STREAM(logger_) << "Listening for RPC on port " << config_->rpc_port;
        }
#endif
        state_machine_->Start();
        // start rest
        lgraph::RestServer::Config rest_config(*config_);
        rest_server_.reset(new lgraph::RestServer(state_machine_.get(), rest_config, config_));
        FMA_INFO_STREAM(logger_) << "Server started.";
    } catch (std::exception &e) {
        _kill_signal_.Notify();
        FMA_WARN_STREAM(logger_) << "Server hit an exception and shuts down abnormally:";
        FMA_WARN_STREAM(logger_) << e.what();
        ret = -2;
    }
    return ret;
}

int LGraphServer::WaitTillKilled() {
    try {
        SetupSignalHandler();
        while (!_kill_signal_.Wait(3600)) {
            WaitSignal();
        }
    } catch (std::exception &e) {
        FMA_WARN_STREAM(logger_) << "Server hit an exception and shuts down abnormally:";
        FMA_WARN_STREAM(logger_) << e.what();
        return -1;
    }
    return Stop(true);
}

int LGraphServer::Stop(bool force_exit) {
    // if already stopped, just return
    if (!state_machine_) return 0;
    // otherwise, try to stop the services, exit forcefully if necessary
    try {
        FMA_DBG_STREAM(logger_) << "Stopping TuGraph...";
        // the kaishaku watches the server, if exit flag is set and the server cannot be shutdown
        // normally after three seconds, it kills the process
        std::thread kaishaku;
        if (force_exit) {
            kaishaku = std::thread([&]() {
                _kill_signal_.Wait(-1);
                if (!server_exit_.Wait(3)) {
                    FMA_WARN_STREAM(logger_) << "Killing server...";
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
        KillServer();
        if (kaishaku.joinable()) kaishaku.join();
        return 0;
    } catch (std::exception &e) {
        FMA_WARN_STREAM(logger_) << "Server hit an exception and shuts down abnormally:";
        FMA_WARN_STREAM(logger_) << e.what();
        return -1;
    }
}
void LGraphServer::WaitSignal() {}
}  // namespace lgraph
