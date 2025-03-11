/**
 * Copyright 2025 AntGroup CO., Ltd.
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

// written by botu.wzy

#pragma once
#include <thread>
#include <shared_mutex>
#include "fma-common/type_traits.h"
#include "bolt_raft/raft_driver.h"
#include "bolt_raft/bolt_raft.pb.h"
#include "server/state_machine.h"

namespace bolt_raft {
class BoltRaftServer final {
 public:
    static BoltRaftServer& Instance() {
        static BoltRaftServer server;
        return server;
    }
    DISABLE_COPY(BoltRaftServer);
    DISABLE_MOVE(BoltRaftServer);
    bool Start(lgraph::StateMachine* sm, int port, uint64_t node_id, const std::string& init_peers,
               const RaftLogStoreConfig& store_config, const RaftConfig& config);
    void Stop();
    bool Started() const { return started_; }
    RaftDriver& raft_driver() { return *raft_driver_; }
    ~BoltRaftServer() { Stop(); }
    lgraph::StateMachine* StateMachine() { return sm_; }

 private:
    BoltRaftServer() = default;
    std::vector<std::thread> threads_{};
    bool started_ = false;
    boost::asio::io_service listener_{BOOST_ASIO_CONCURRENCY_HINT_UNSAFE};
    std::function<void(raftpb::Message)> protobuf_handler_{};
    std::shared_ptr<RaftDriver> raft_driver_ = nullptr;
    lgraph::StateMachine* sm_ = nullptr;
};
}  // namespace bolt_raft
