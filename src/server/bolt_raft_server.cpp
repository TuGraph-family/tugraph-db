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
std::shared_ptr<ApplyContext> BoltRaftServer::Propose(const RaftRequest& request) {
    return raft_driver_->Propose(request.SerializeAsString());
}

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
                peer.id_ = item["node_id"].get<int64_t>();
                peer.context_ = item.dump();
                peers.emplace_back(std::move(peer));
            }
            id_generator_ = std::make_shared<Generator>(
                node_id,std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now().time_since_epoch()).count());
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
    if (stopped_) {
        return;
    }
    listener_.stop();
    for (auto& t : threads_) {
        t.join();
    }
    stopped_ = true;
    LOG_INFO() << "bolt raft server stopped.";
}
}
