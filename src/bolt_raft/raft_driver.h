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

// written by botu.wzy

#pragma once
#include <utility>
#include <deque>
#include <boost/asio.hpp>
#include "bolt_raft/raft_log_store.h"
#include "bolt_raft/bolt_raft.pb.h"

namespace bolt_raft {
using namespace boost::asio::ip;
class NodeClient : public std::enable_shared_from_this<NodeClient> {
 public:
    NodeClient(boost::asio::io_service& io_service, const std::string& ip, int port)
        : io_service_(io_service),
          socket_(io_service_),
          endpoint_(address::from_string(ip), port),
          interval_(1000),
          timer_(io_service_) {}
    void Connect();
    void Send(std::string str);
    void Close();
    std::string ip() { return endpoint_.address().to_string(); }
    int port() { return endpoint_.port(); }
    bool connected() { return connected_; }

 private:
    void do_send();
    void do_read_some();
    void reconnect();
    void send_magic_code();

    boost::asio::io_service& io_service_;
    tcp::socket socket_;
    tcp::endpoint endpoint_;
    std::deque<std::string> msg_queue_;
    std::vector<boost::asio::const_buffer> send_buffers_;
    boost::posix_time::millisec interval_;
    boost::asio::deadline_timer timer_;
    std::atomic<bool> has_closed_{false};
    std::atomic<bool> connected_ = false;
    const uint8_t magic_code[4] = {0x17, 0xB0, 0x60, 0x60};
    uint8_t buffer4_[4] = {0};
};

struct Generator {
    static const int tsLen = 5 * 8;
    static const int cntLen = 8;
    static const int suffixLen = tsLen + cntLen;

    Generator(uint64_t id, uint64_t time) {
        prefix = id << suffixLen;
        suffix = lowbit(time, tsLen) << cntLen;
    }
    uint64_t prefix = 0;
    std::atomic<uint64_t> suffix{0};
    static uint64_t lowbit(uint64_t x, int n) {
        return x & (std::numeric_limits<uint64_t>::max() >> (64 - n));
    }
    uint64_t Next() {
        auto suf = suffix.fetch_add(1) + 1;
        auto id = prefix | lowbit(suf, suffixLen);
        return id;
    }
};

struct PromiseContext {
    uint64_t index = 0;
    std::promise<eraft::Error> proposed;
    std::promise<void> commited;
    std::promise<void> applied;
};

struct RaftStatus {
    eraft::Status s;
    uint64_t first_log = 0;
    uint64_t last_log = 0;
};

struct RaftConfig {
    int64_t tick_interval = 0;
    int64_t election_tick = 0;
    int64_t heartbeat_tick = 0;
    bool Check();
};

struct RaftLogStoreConfig {
    std::string path;
    uint64_t block_cache = 0;
    uint64_t total_threads = 0;
    uint64_t keep_logs = 0;
    uint64_t gc_interval = 0;
    bool Check();
};

class RaftDriver {
 public:
    RaftDriver(std::function<void(uint64_t index, const RaftRequest&)> apply, uint64_t apply_id,
               int64_t node_id, std::vector<eraft::Peer> init_peers,
               const RaftLogStoreConfig& store_config, const RaftConfig& config);
    eraft::Error Run();
    void Stop();
    void Step(raftpb::Message msg);
    std::shared_ptr<PromiseContext> ProposeRaftRequest(bolt_raft::RaftRequest request);
    std::shared_ptr<PromiseContext> ProposeConfChange(raftpb::ConfChange& cc);
    NodeInfos GetNodeInfosWithLeader();
    RaftStatus GetRaftStatus();

 private:
    std::shared_ptr<PromiseContext> Propose(uint64_t uuid, raftpb::Message msg);
    void Tick();
    void CheckAndCompactLog();
    void CheckReady();
    void Apply(const std::vector<raftpb::Entry>& entries);

    std::vector<std::thread> threads_;
    boost::asio::io_service raft_service_;
    boost::asio::io_service timer_service_;
    boost::asio::io_service apply_service_;
    boost::asio::io_service client_service_;
    std::function<void(uint64_t, const RaftRequest&)> apply_;
    uint64_t apply_id_;
    uint64_t node_id_;
    std::vector<eraft::Peer> init_peers_;
    boost::posix_time::millisec tick_interval_;
    boost::asio::deadline_timer tick_timer_;
    boost::posix_time::millisec compact_interval_;
    boost::asio::deadline_timer compact_timer_;
    std::shared_ptr<eraft::RawNode> rn_;
    std::shared_ptr<RaftLogStorage> storage_;
    std::shared_mutex nodes_mutex_;
    NodeInfos node_infos_;
    std::unordered_map<uint64_t, std::shared_ptr<NodeClient>> node_clients_;
    Generator id_generator_;
    std::mutex promise_mutex_;
    std::unordered_map<uint64_t, std::shared_ptr<PromiseContext>> pending_promise_;
    std::unordered_set<uint64_t> mark_unreachable_;
    RaftLogStoreConfig store_config_;
    RaftConfig raft_config_;
};
}  // namespace bolt_raft
