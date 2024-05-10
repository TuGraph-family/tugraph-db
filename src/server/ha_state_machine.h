/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once
#include <chrono>
#include "tools/lgraph_log.h"
#include "core/global_config.h"
#include "server/state_machine.h"

#ifndef _WIN32
#include "braft/raft.h"
#include "brpc/closure_guard.h"
#include "brpc/server.h"

#include "core/lightning_graph.h"
#include "protobuf/ha.pb.h"

namespace lgraph {

class HaStateMachine : public StateMachine, public braft::StateMachine {
 public:
    struct Config : public ::lgraph::StateMachine::Config {
        std::string ha_conf;
        std::string ha_dir;
        int ha_election_timeout_ms = 500;
        int ha_snapshot_interval_s = 7 * 24 * 3600;
        int ha_heartbeat_interval_ms = 1000;     // send heartbeat every 1 sec
        int ha_node_offline_ms = 600 * 1000;  // node will be made offline after this period
        int ha_node_remove_ms = 1200 * 1000;  // node will be removed from peer list after 20 min
        int ha_bootstrap_role;
        int ha_node_join_group_s;
        bool ha_is_witness;
        bool ha_enable_witness_to_leader;
        std::string ha_first_snapshot_start_time = "";

        Config() {}
        explicit Config(const GlobalConfig& c) : ::lgraph::StateMachine::Config(c) {
            ha_conf = c.ha_conf;
            ha_dir = c.ha_log_dir;
            ha_election_timeout_ms = c.ha_election_timeout_ms;
            ha_snapshot_interval_s = c.ha_snapshot_interval_s;
            ha_heartbeat_interval_ms = c.ha_heartbeat_interval_ms;
            ha_node_offline_ms = c.ha_node_offline_ms;
            ha_node_remove_ms = c.ha_node_remove_ms;
            ha_bootstrap_role = c.ha_bootstrap_role;
            ha_node_join_group_s = c.ha_node_join_group_s;
            ha_is_witness = c.ha_is_witness;
            ha_enable_witness_to_leader = c.ha_enable_witness_to_leader;
            ha_first_snapshot_start_time = c.ha_first_snapshot_start_time;
        }
    };

 protected:
    braft::Node* volatile node_;
    std::atomic<int64_t> leader_term_;
    std::atomic<bool> joined_group_;
    Config config_;
    std::string my_rpc_addr_;
    std::string my_rest_addr_;

    mutable std::mutex hb_mutex_;
    std::condition_variable hb_cond_;
    bool exit_flag_ = false;  // should heartbeat thread exit?
    struct HeartbeatStatus {
        std::string rpc_addr;
        std::string rest_addr;
        NodeState state;
        NodeRole role;
        double last_heartbeat;
    };

    std::unordered_map<std::string, HeartbeatStatus>
        heartbeat_states_;          // rpc_addr -> last_heartbeat_time, valid only if this is master
    std::string master_rest_addr_;  // master rest address, valid only for follow
    std::string master_rpc_addr_;   // master rpc address, valid only for follow
    std::shared_ptr<LGraphRPCService_Stub>
        rpc_stub_;  // rpc stub to send heartbeat, valid only for follow

    std::thread heartbeat_thread_;

 public:
    HaStateMachine(const Config& config, const std::shared_ptr<GlobalConfig>& global_config)
        : ::lgraph::StateMachine(config, global_config),
          node_(nullptr),
          leader_term_(-1),
          joined_group_(false),
          config_(config) {
        my_rest_addr_ =
            fma_common::StringFormatter::Format("{}:{}", config.host, global_config->http_port);
        my_rpc_addr_ = fma_common::StringFormatter::Format("{}:{}", config.host, config_.rpc_port);
        heartbeat_thread_ = std::thread([this]() { HeartbeatThread(); });
    }
    ~HaStateMachine() override { HaStateMachine::Stop(); }

    void Start() override;

    void Stop() override;

    int64_t GetVersion() override;

    std::string GetMasterRestAddr() override {
        std::lock_guard<std::mutex> l(hb_mutex_);
        if (node_->is_leader())
            return my_rest_addr_;
        else
            return master_rest_addr_;
    }

    std::string GetMasterRpcAddr() override {
        std::lock_guard<std::mutex> l(hb_mutex_);
        if (node_->is_leader())
            return my_rpc_addr_;
        else
            return master_rpc_addr_;
    }

    bool IsCurrentMaster() override { return IsLeader(); }

    bool DoRequest(bool is_write, const LGraphRequest* req, LGraphResponse* resp,
                   google::protobuf::Closure* on_done) override;

    bool IsLeader() const { return leader_term_.load(std::memory_order_acquire) > 0; }

    void LeaveGroup();

    // override from braft::StateMachine
    void on_leader_start(int64_t term) override;

    // override from braft::StateMachine
    void on_leader_stop(const butil::Status& status) override;

    // override from braft::StateMachine
    void on_shutdown() override;

    // override from braft::StateMachine
    void on_error(const ::braft::Error& e) override;

    // override from braft::StateMachine
    void on_configuration_committed(const ::braft::Configuration& conf) override;

    // override from braft::StateMachine
    void on_stop_following(const ::braft::LeaderChangeContext& ctx) override;

    // override from braft::StateMachine
    void on_start_following(const ::braft::LeaderChangeContext& ctx) override;

    bool IsInHaMode() const override { return true; }

    std::vector<Peer> ListPeers() const override;

    // override from braft::StateMachine
    void on_snapshot_save(braft::SnapshotWriter* writer, braft::Closure* done) override;

    // override from braft::StateMachine
    int on_snapshot_load(braft::SnapshotReader* reader) override;

    // override from braft::StateMachine
    void on_apply(braft::Iterator& iter) override;

 private:
    struct ReqRespClosure : public braft::Closure {
        ReqRespClosure(HaStateMachine* sm, const LGraphRequest* req, LGraphResponse* resp,
                       google::protobuf::Closure* done)
            : sm_(sm), req_(req), resp_(resp), done_(done) {}

        ~ReqRespClosure() override = default;

        [[nodiscard]] const LGraphRequest* Request() const { return req_; }
        [[nodiscard]] LGraphResponse* Response() const { return resp_; }
        void Run() override {
            // Auto delete this after Run()
            std::unique_ptr<ReqRespClosure> self_guard(this);
            // Repspond this RPC.
            brpc::ClosureGuard done_guard(done_);
        }

     private:
        HaStateMachine* sm_;
        const LGraphRequest* req_;
        LGraphResponse* resp_;
        google::protobuf::Closure* done_;
    };

    struct SnapshotArg {
        braft::SnapshotWriter* writer;
        braft::Closure* done;
        lgraph::HaStateMachine* haStateMachine;
    };

    static void *save_snapshot(void* arg);

    bool ReplicateAndApplyRequest(const LGraphRequest* req, LGraphResponse* resp,
                                  google::protobuf::Closure* on_done);

    // apply a HA request, specifically Heartbeat request
    bool ApplyHaRequest(const LGraphRequest* req, LGraphResponse* resp);

    void HeartbeatThread();

    void SendHeartbeatToMasterLocked();
    void ScanHeartbeatStatusLocked();
};
}  // namespace lgraph
#else
namespace lgraph {
class HaStateMachine : public StateMachine {
 public:
    struct Config : public ::lgraph::StateMachine::Config {
        Config() {}
        explicit Config(const GlobalConfig& c) : ::lgraph::StateMachine::Config(c) {}
    };

    HaStateMachine(const Config& config, GlobalConfig* gc) : ::lgraph::StateMachine(config, gc) {
        LOG_ERROR() << "Replication is not implemented for Windows yet.";
    }
    virtual ~HaStateMachine() {}
};
}  // namespace lgraph
#endif
