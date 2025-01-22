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
    bool Start(lgraph::StateMachine* sm, int port, uint64_t node_id,
               const std::string& init_peers, const std::string& log_path);
    void Stop();
    bool Started() {return started_;}
    RaftDriver& raft_driver() {return *raft_driver_;}
    ~BoltRaftServer() { Stop(); }

 private:
    BoltRaftServer() = default;
    std::vector<std::thread> threads_{};
    bool started_ = false;
    boost::asio::io_service listener_{BOOST_ASIO_CONCURRENCY_HINT_UNSAFE};
    std::function<void(raftpb::Message)> protobuf_handler_{};
    std::shared_ptr<RaftDriver> raft_driver_ = nullptr;
    lgraph::StateMachine* sm_ = nullptr;
};
}
