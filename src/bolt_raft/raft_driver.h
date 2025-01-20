#pragma once
#include <deque>
#include <boost/asio.hpp>
#include <utility>
#include "raft_log_store.h"
namespace bolt_raft {
class NodeClient : public std::enable_shared_from_this<NodeClient> {
public:
    NodeClient(boost::asio::io_service& io_service, const std::string &ip, int port)
            : io_service_(io_service),
              socket_(io_service_),
              endpoint_(boost::asio::ip::address::from_string(ip), port),
              interval_(1000),
              timer_(io_service_) {
    }
    void Connect();
    void Send(std::string str);
    void Close();
    std::string ip() { return endpoint_.address().to_string(); }
    int port() { return endpoint_.port(); }
private:
    void do_send();
    void do_read_some();
    void reconnect();
    void send_magic_code();

    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::endpoint endpoint_;
    std::deque<std::string> msg_queue_;
    std::vector<boost::asio::const_buffer> send_buffers_;
    boost::posix_time::millisec interval_;
    boost::asio::deadline_timer timer_;
    std::atomic<bool> has_closed_{false};
    bool connected_ = false;
    const uint8_t magic_code[4] = {0x17, 0xB0, 0x60, 0x60};
    uint8_t buffer4_[4] = {0};
};

class RaftDriver {
public:
    RaftDriver(std::function<void (uint64_t index, const std::string&)> apply,
               uint64_t apply_id,
               int64_t node_id,
               std::vector<eraft::Peer> init_peers,
               std::string log_path);
    eraft::Error Run();
    void Message(raftpb::Message msg);
    eraft::Error Propose(std::string data);
    eraft::Error ProposeConfChange(const raftpb::ConfChange& cc);

private:
    eraft::Error PostMessage(raftpb::Message msg);
    void tick() {
        raft_service_.post([this]() mutable {
            rn_->Tick();
            check_ready();
        });
        tick_timer_.expires_at(tick_timer_.expires_at() + tick_interval_);
        tick_timer_.async_wait(std::bind(&RaftDriver::tick, this));
    }
    void advance() {
        raft_service_.post([this]() mutable {
            rn_->Advance({});
            advance_ = false;
        });
    }
    void check_ready();
    void on_ready(eraft::Ready ready);
    std::string nodes_info();

    std::vector<std::thread> threads_;
    boost::asio::io_service raft_service_;
    boost::asio::io_service tick_service_;
    boost::asio::io_service ready_service_;
    boost::asio::io_service client_service_;
    std::function<void (uint64_t, const std::string&)> apply_;
    uint64_t apply_id_;
    uint64_t node_id_;
    std::vector<eraft::Peer> init_peers_;
    std::string log_path_;
    boost::posix_time::millisec tick_interval_;
    boost::asio::deadline_timer tick_timer_;
    std::shared_ptr<eraft::RawNode> rn_;
    std::shared_ptr<RaftLogStorage> storage_;
    uint64_t lead_ = eraft::None;
    bool advance_ = false;
    std::unordered_map<uint64_t, std::shared_ptr<NodeClient>> node_clients_;
};
extern std::shared_ptr<RaftDriver> g_raft_driver;

struct Generator {
    static const int tsLen  = 5 * 8;
    static const int cntLen  = 8;
    static const int suffixLen = tsLen + cntLen;

    Generator(uint64_t id, uint64_t time) {
        prefix = id << suffixLen;
        suffix = lowbit(time, tsLen) << cntLen;
    }
    uint64_t prefix = 0;
    std::atomic<uint64_t> suffix{0};
    uint64_t lowbit(uint64_t x, int n) {
        return x & (std::numeric_limits<uint64_t>::max() >> (64 - n));
    }
    uint64_t Next() {
        auto suf = suffix.fetch_add(1) + 1;
        auto id = prefix | lowbit(suf, suffixLen);
        return id;
    }
};
extern std::shared_ptr<Generator> g_id_generator;

struct ApplyContext {
    uint64_t index = 0;
    std::promise<void> start;
    std::promise<void> end;
};

}