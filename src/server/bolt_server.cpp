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

/*
* written by botu.wzy
*/
#include "server/bolt_server.h"
#include "bolt_ha/connection.h"
#include "bolt_ha/io_service.h"
#include "bolt_ha/raft_driver.h"

namespace bolt {
static boost::asio::io_service listener(BOOST_ASIO_CONCURRENCY_HINT_UNSAFE);
extern std::function<void(bolt::BoltConnection &conn, bolt::BoltMsg msg,
                          std::vector<std::any> fields, std::vector<uint8_t> raw_data)> BoltHandler;
bool BoltServer::Start(lgraph::StateMachine* sm, int port, int io_thread_num) {
    sm_ = sm;
    bolt::MarkersInit();
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();
    threads_.emplace_back([port, io_thread_num, &promise](){
        bool promise_done = false;
        try {
            bolt::IOService<bolt::BoltConnection, decltype(bolt::BoltHandler)> bolt_service(
                listener, port, io_thread_num, bolt::BoltHandler);
            boost::asio::io_service::work holder(listener);
            LOG_INFO() << "bolt server run";
            promise.set_value(true);
            promise_done = true;
            listener.run();
        } catch (const std::exception& e) {
            LOG_WARN() << "bolt server expection: " << e.what();
            if (!promise_done) {
                promise.set_value(false);
            }
        }
    });
    return future.get();
}

void BoltServer::Stop() {
    if (stopped_) {
        return;
    }
    listener.stop();
    for (auto& t : threads_) {
        t.join();
    }
    stopped_ = true;
    LOG_INFO() << "bolt server stopped.";
}

std::function protobuf_handler = [](raftpb::Message rpc_msg){
    bolt_ha::g_raft_driver->Message(std::move(rpc_msg));
};

void g_apply(uint64_t index, const std::string& log);

static boost::asio::io_service raft_listener(BOOST_ASIO_CONCURRENCY_HINT_UNSAFE);
bool BoltRaftServer::Start(lgraph::StateMachine* sm, int port, uint64_t node_id, std::string init_peers) {
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();
    threads_.emplace_back([port, node_id, init_peers, &promise](){
        bool promise_done = false;
        try {
            std::vector<eraft::Peer> peers;
            auto cluster = nlohmann::json::parse(init_peers);
            for (const auto& item : cluster) {
                eraft::Peer peer;
                peer.id_ = item["node_id"].get<int64_t>();
                peer.context_ = item.dump();
                peers.emplace_back(std::move(peer));
            }
            bolt_ha::g_id_generator = std::make_shared<bolt_ha::Generator>(
                node_id,std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now().time_since_epoch()).count());
            bolt_ha::g_raft_driver = std::make_unique<bolt_ha::RaftDriver> (
                bolt::g_apply,
                0,
                node_id,
                peers,
                "raftlog");
            auto err = bolt_ha::g_raft_driver->Run();
            if (err != nullptr) {
                LOG_ERROR() << "raft driver failed to run, error: " << err.String();
                return;
            }
            bolt_ha::IOService<bolt_ha::ProtobufConnection, decltype(protobuf_handler)> bolt_raft_service(
                raft_listener, port, 1, protobuf_handler);
            boost::asio::io_service::work holder(raft_listener);
            LOG_INFO() << "bolt raft server run";
            promise.set_value(true);
            promise_done = true;
            raft_listener.run();
        } catch (const std::exception& e) {
            LOG_WARN() << "bolt raft server expection: " << e.what();
            if (!promise_done) {
                promise.set_value(false);
            }
        }
    });
    return future.get();
}

void BoltRaftServer::Stop() {
    if (stopped_) {
        return;
    }
    raft_listener.stop();
    for (auto& t : threads_) {
        t.join();
    }
    stopped_ = true;
    LOG_INFO() << "bolt raft server stopped.";
}

}  // namespace bolt
