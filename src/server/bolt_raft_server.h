#include <thread>
#include "fma-common/type_traits.h"

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
    ~BoltRaftServer() { Stop(); }
 private:
    BoltRaftServer() = default;
    std::vector<std::thread> threads_{};
    bool stopped_ = false;
};
}