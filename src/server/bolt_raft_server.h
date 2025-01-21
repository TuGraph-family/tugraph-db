#include <thread>
#include <shared_mutex>
#include "fma-common/type_traits.h"
#include "bolt_raft/raft_driver.h"
#include "bolt_raft/raft_request.pb.h"

namespace bolt_raft {
class BoltRaftServer final {
 public:
    static BoltRaftServer& Instance() {
        static BoltRaftServer server;
        return server;
    }
    DISABLE_COPY(BoltRaftServer);
    DISABLE_MOVE(BoltRaftServer);
    bool Start(int port, uint64_t node_id, std::string init_peers);
    void Stop();
    bool Started() {return started_;}
    std::shared_ptr<PromiseContext> Propose(const RaftRequest& request);
    ~BoltRaftServer() { Stop(); }
 private:
    BoltRaftServer() = default;
    std::vector<std::thread> threads_{};
    bool started_ = false;
    boost::asio::io_service listener_{BOOST_ASIO_CONCURRENCY_HINT_UNSAFE};
    std::function<void(raftpb::Message)> protobuf_handler_{};
    std::shared_ptr<RaftDriver> raft_driver_ = nullptr;
    std::shared_ptr<Generator> id_generator_ = nullptr;
    std::unordered_map<uint64_t, std::shared_ptr<bolt_raft::PromiseContext>> pending_promise_;
};
}