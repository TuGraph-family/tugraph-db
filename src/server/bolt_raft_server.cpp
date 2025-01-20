#include "bolt_raft_server.h"
#include "bolt_raft/raft_driver.h"
#include "bolt_raft/io_service.h"
#include "bolt_raft/connection.h"
#include "tools/json.hpp"
#include "tools/lgraph_log.h"

namespace bolt {
void g_apply(uint64_t index, const std::string& log);
}

namespace bolt_raft {
std::function protobuf_handler = [](raftpb::Message rpc_msg){
    bolt_raft::g_raft_driver->Message(std::move(rpc_msg));
};

static boost::asio::io_service raft_listener(BOOST_ASIO_CONCURRENCY_HINT_UNSAFE);
bool BoltRaftServer::Start(int port, uint64_t node_id, std::string init_peers) {
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
            bolt_raft::g_id_generator = std::make_shared<bolt_raft::Generator>(
                node_id,std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now().time_since_epoch()).count());
            bolt_raft::g_raft_driver = std::make_unique<bolt_raft::RaftDriver> (
                bolt::g_apply,
                0,
                node_id,
                peers,
                "raftlog");
            auto err = bolt_raft::g_raft_driver->Run();
            if (err != nullptr) {
                LOG_ERROR() << "raft driver failed to run, error: " << err.String();
                return;
            }
            bolt_raft::IOService<bolt_raft::ProtobufConnection, decltype(protobuf_handler)> bolt_raft_service(
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
}
