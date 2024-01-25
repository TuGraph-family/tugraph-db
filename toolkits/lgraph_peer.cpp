
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

#include <braft/cli.h>
#include <gflags/gflags.h>
#include "fma-common/configuration.h"

DEFINE_int32(timeout_ms, -1, "Timeout (in milliseconds) of the operation");
DEFINE_int32(max_retry, 3, "Max retry times of each operation");

int main(int argc, char** argv) {
    std::string command;
    std::string group = "lgraph";
    std::string peer;
    std::string conf;

    fma_common::Configuration config;
    config.Add(command, "command", false).Comment("HA command").
        SetPossibleValues({"remove_peer", "transfer_leader", "snapshot"});
    config.Add(peer, "peer", false).Comment("HA peer node");
    config.Add(conf, "conf", true).Comment("HA configuration");
    try {
        config.ExitAfterHelp(true);
        config.ParseAndFinalize(argc, argv);
    } catch (std::exception& e) {
        LOG_ERROR() << e.what();
        return -1;
    }
    braft::cli::CliOptions opt;
    opt.timeout_ms = FLAGS_timeout_ms;
    opt.max_retry = FLAGS_max_retry;
    if (command == "snapshot") {
        butil::Status status = braft::cli::snapshot(group, peer, opt);
        if (!status.ok()) {
            LOG_ERROR() << "Fail to make snapshot : " << status;
            return -1;
        }
    } else {
        braft::Configuration raft_conf;
        if (raft_conf.parse_from(conf) != 0) {
            LOG_ERROR() << "Fail to parse --conf=`" << conf << '\'';
            return -1;
        }
        if (command == "remove_peer") {
            butil::Status status = braft::cli::remove_peer(group, raft_conf, peer, opt);
            if (!status.ok()) {
                LOG_ERROR() << "Fail to remove_peer : " << status;
                return -1;
            }
        } else if (command == "transfer_leader") {
            butil::Status status = braft::cli::transfer_leader(group, raft_conf, peer, opt);
            if (!status.ok()) {
                LOG_ERROR() << "Fail to transfer_leader: " << status;
                return -1;
            }
        }
    }
    return 0;
}
