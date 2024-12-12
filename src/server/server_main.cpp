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

//
// Created by botu.wzy
//

#include <sys/resource.h>
#include <tabulate/table.hpp>
#include "common/logger.h"
#include "server/galaxy.h"
#include "bolt/bolt_server.h"
#include "service.h"
#include "common/version.h"
#include "common/flags.h"

std::unordered_set<std::string> inner_flags = {
    "flagfile",
    "fromenv",
    "tryfromenv",
    "undefok",
    "tab_completion_columns",
    "tab_completion_word",
    "help",
    "helpfull",
    "helpmatch",
    "helpon",
    "helppackage",
    "helpshort",
    "helpxml",
    "version"
};

using namespace bolt;
namespace server {
std::string Version() {
    std::ostringstream info;
    std::string version;
    version.append(std::to_string(LGRAPH_VERSION_MAJOR))
        .append(".")
        .append(std::to_string(LGRAPH_VERSION_MINOR))
        .append(".")
        .append(std::to_string(LGRAPH_VERSION_PATCH));
    info << "\nTuGraph v" << version << "\nCompiled from " << GIT_BRANCH
               << " branch\nCommit " << GIT_COMMIT_HASH;
    info << "\nCPP compiler version: " << CXX_COMPILER_ID << " " << CXX_COMPILER_VERSION
               << ".";
    return info.str();
}
void PrintWelcome() {
    std::string version;
    version.append(std::to_string(5))
        .append(".")
        .append(std::to_string(0))
        .append(".")
        .append(std::to_string(0));
    std::ostringstream info;
    info << "\n"
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
         << "\n";
    {
        tabulate::Table table;
        table.format().trim_mode(tabulate::Format::TrimMode::kNone);
        info << "Compile Information:\n";
        table.add_row({"Branch", GIT_BRANCH});
        table.add_row({"Commit", GIT_COMMIT_HASH});
        table.add_row({"BuildType", BUILD_TYPE});
        info << table << "\n";
    }
    {
        struct rlimit rlim{};
        getrlimit(RLIMIT_CORE, &rlim);
        std::ifstream file("/proc/sys/kernel/core_pattern");
        std::string path;
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                                (std::istreambuf_iterator<char>()));
            path = content;
            file.close();
        } else {
            LOG_ERROR("Failed to read /proc/sys/kernel/core_pattern");
        }
        tabulate::Table table;
        table.add_row({"coredump file limit size", std::to_string(rlim.rlim_cur)});
        table.add_row({"coredump file path", path});
        info << "System environment Information:\n";
        info << table << "\n";
    }
    {
        tabulate::Table table;
        table.format().trim_mode(tabulate::Format::TrimMode::kNone);
        std::vector<gflags::CommandLineFlagInfo> flags;
        GetAllFlags(&flags);
        for (auto& flag : flags) {
            if (inner_flags.count(flag.name)) {
                continue;
            }
            table.add_row({flag.name, flag.current_value});
        }
        info << "Config Information:\n";
        info << table;
    }
    LOG_INFO(info.str());
    spdlog::default_logger()->flush();
}
void ShutDownHandler(int sig) {
    LOG_INFO("Received signal {}, shutdown",strsignal(sig));
    spdlog::default_logger()->flush();
    BoltServer::Instance().Stop();
    spdlog::default_logger()->flush();
}
void CrashHandler(int sig) {
    LOG_ERROR("Received signal {}, crash", strsignal(sig));
    spdlog::default_logger()->flush();
    struct sigaction sa{};
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SIG_DFL;
    sigaction(sig, &sa, nullptr);
    kill(getpid(), sig);
}
void SetupSignalHandler() {
    {
        // shutdown
        struct sigaction sa{};
        sa.sa_handler = ShutDownHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, nullptr);
        sigaction(SIGTERM, &sa, nullptr);
        sigaction(SIGUSR1, &sa, nullptr);
    }
    {
        // crash
        struct sigaction sa{};
        sa.sa_handler = CrashHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_NODEFER;
        sigaction(SIGSEGV, &sa, nullptr);
        sigaction(SIGBUS, &sa, nullptr);
        sigaction(SIGFPE, &sa, nullptr);
        sigaction(SIGILL, &sa, nullptr);
        sigaction(SIGABRT, &sa, nullptr);
    }
}

extern std::function<void(bolt::BoltConnection& conn, bolt::BoltMsg msg,
                          std::vector<std::any> fields)> g_bolt_handler;
class LGraphDaemon : public Service {
   public:
    LGraphDaemon()
        : Service("lgraph_server", FLAGS_pid_file) {}
    int Run() override {
        if (!SetupLogger()) return -1;
        SetupSignalHandler();
        PrintWelcome();
        try {
            g_galaxy = Galaxy::Open(FLAGS_data_path,
                                    {.block_cache_size = FLAGS_block_cache,
                                     .row_cache_size = FLAGS_row_cache,
                                     .ft_apply_interval = FLAGS_ft_apply_interval,
                                     .vt_apply_interval = FLAGS_vt_apply_interval});
            BoltServer::Instance().Start(
                FLAGS_bolt_port, FLAGS_bolt_io_thread_num, g_bolt_handler);
            g_galaxy.reset();
            spdlog::shutdown();
            return 0;
        } catch (const std::exception& e) {
            LOG_ERROR(e.what());
            return -1;
        }
    }
};
}
using namespace server;
int main(int argc, char* argv[]) {
    gflags::SetVersionString(Version());
    gflags::SetUsageMessage("Usage: " + std::string(argv[0]) + " [options]");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    server::LGraphDaemon daemon;
    spdlog::set_pattern("%v");
    if (FLAGS_mode == "run") {
        return daemon.Run();
    } else if (FLAGS_mode == "start") {
        return daemon.Start();
    } else if (FLAGS_mode == "restart") {
        return daemon.Restart();
    } else if (FLAGS_mode == "stop") {
        return daemon.Stop();
    }
}