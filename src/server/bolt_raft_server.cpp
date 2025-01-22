#include "bolt_raft_server.h"
#include "bolt_raft/raft_driver.h"
#include "bolt_raft/io_service.h"
#include "bolt_raft/connection.h"
#include "tools/json.hpp"
#include "tools/lgraph_log.h"
namespace bolt {
void ApplyRaftRequest(uint64_t index, const bolt_raft::RaftRequest& request);
}

namespace bolt_raft {
bool BoltRaftServer::Start(int port, uint64_t node_id, std::string init_peers) {
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();
    threads_.emplace_back([this, port, node_id, init_peers, &promise]() {
        bool promise_done = false;
        try {
            std::vector<eraft::Peer> peers;
            auto cluster = nlohmann::json::parse(init_peers);
            for (const auto& item : cluster) {
                eraft::Peer peer;
                NodeInfo node_info;
                node_info.set_node_id(item["bolt_raft_node_id"].get<uint64_t>());
                node_info.set_ip(item["ip"].get<std::string>());
                node_info.set_bolt_port(item["bolt_port"].get<int32_t>());
                node_info.set_bolt_raft_port(item["bolt_raft_port"].get<int32_t>());
                node_info.set_is_leader(false);
                peer.id_ = node_info.node_id();
                peer.context_ = node_info.SerializeAsString();
                peers.emplace_back(std::move(peer));
            }
            raft_driver_ = std::make_unique<RaftDriver> (
                bolt::ApplyRaftRequest,
                0,
                node_id,
                peers,
                "raftlog");
            auto err = raft_driver_->Run();
            if (err != nullptr) {
                LOG_ERROR() << "raft driver failed to run, error: " << err.String();
                return;
            }
            protobuf_handler_ = [this](raftpb::Message rpc_msg) {
                raft_driver_->Message(std::move(rpc_msg));
            };
            bolt_raft::IOService<bolt_raft::ProtobufConnection, decltype(protobuf_handler_)> bolt_raft_service(
                listener_, port, 1, protobuf_handler_);
            boost::asio::io_service::work holder(listener_);
            LOG_INFO() << "bolt raft server run";
            promise.set_value(true);
            promise_done = true;
            started_ = true;
            listener_.run();
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
    if (!started_) {
        return;
    }
    listener_.stop();
    for (auto& t : threads_) {
        t.join();
    }
    started_ = false;
    LOG_INFO() << "bolt raft server stopped.";
}
}
