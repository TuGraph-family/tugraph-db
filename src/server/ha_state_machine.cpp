/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "braft/protobuf_file.h"
#include "braft/raft.h"
#include "braft/util.h"

#include "brpc/channel.h"
#include "braft/cli.h"
#include "brpc/closure_guard.h"

#include "server/ha_state_machine.h"
#include "restful/server/rest_server.h"

namespace braft {
    DECLARE_bool(raft_enable_witness_to_leader);
    DECLARE_bool(enable_first_snapshot_config);
    DECLARE_string(first_snapshot_start_time);
}

void lgraph::HaStateMachine::Start() {
    // check ha node can be started
    butil::EndPoint addr;
    butil::str2endpoint(config_.host.c_str(), config_.rpc_port, &addr);
    if (butil::IP_ANY == addr.ip) {
        throw std::runtime_error("TuGraph can't be started from IP_ANY (0.0.0.0) in HA mode.");
    }
    if (node_) {
        LOG_WARN() << "HaStateMachine already started.";
        return;
    }
    ::lgraph::StateMachine::Start();

    // bootstrap
    if (config_.ha_bootstrap_role == 1) {
        const int64_t BOOTSTRAP_LOG_INDEX = 1024;
        if (config_.ha_is_witness) {
            ::lgraph::StateMachine::Stop();
            throw std::runtime_error("Can not bootstrap on witness node");
        }
#if LGRAPH_SHARE_DIR
        LOG_WARN() << "Bootstrapping is not necessary in this version, ignored";
#else
        if (galaxy_->GetRaftLogIndex() != -1) {
            LOG_INFO() << "Bootstrapping from existing data...";
        }

        LOG_DEBUG() << "Bootstrapping...";
        int64_t new_log_index = galaxy_->GetRaftLogIndex() + BOOTSTRAP_LOG_INDEX;
        // change the log index in DB
        galaxy_->BootstrapRaftLogIndex(new_log_index);
        // now bootstrap
        braft::BootstrapOptions options;
        options.group_conf.add_peer(braft::PeerId(addr));
        options.fsm = this;
        options.node_owns_fsm = false;
        std::string prefix = "local://" + config_.ha_dir;
        options.log_uri = prefix + "/log";
        options.raft_meta_uri = prefix + "/raft_meta";
        options.snapshot_uri = prefix + "/snapshot";
        options.last_log_index = new_log_index;
        // delete old snapshot to force creating new snapshot
        auto& fs = fma_common::FileSystem::GetFileSystem("./");
        fs.RemoveDir(config_.ha_dir + "/snapshot");
        // now do bootstrap
        int r = braft::bootstrap(options);
        if (r != 0) {
            ::lgraph::StateMachine::Stop();
            throw std::runtime_error(
                fma_common::StringFormatter::Format("Failed to bootstrap node: ec={}", r));
        }
        LOG_DEBUG() << "Bootstrap succeed. Log index set to " << galaxy_->GetRaftLogIndex();
#endif
    }
    if (config_.ha_first_snapshot_start_time != "") {
        braft::FLAGS_enable_first_snapshot_config = true;
        braft::FLAGS_first_snapshot_start_time =
            config_.ha_first_snapshot_start_time;
    }
    // start braft::Node
    braft::Node* node = new braft::Node("lgraph", braft::PeerId(addr, 0,
                                                                config_.ha_is_witness));
    braft::NodeOptions node_options;
    if (config_.ha_bootstrap_role == 1) {
        node_options.initial_conf.add_peer(braft::PeerId(addr));
    } else if (config_.ha_bootstrap_role == 0) {
        if (node_options.initial_conf.parse_from(config_.ha_conf) != 0) {
            ::lgraph::StateMachine::Stop();
            throw std::runtime_error("Fail to parse configuration " + config_.ha_conf);
        }
    }
    braft::FLAGS_raft_enable_witness_to_leader = config_.ha_enable_witness_to_leader;
    node_options.election_timeout_ms = config_.ha_election_timeout_ms;
    node_options.fsm = this;
    node_options.node_owns_fsm = false;
    node_options.snapshot_interval_s = config_.ha_snapshot_interval_s;
    std::string prefix = "local://" + config_.ha_dir;
    node_options.log_uri = prefix + "/log";
    node_options.raft_meta_uri = prefix + "/raft_meta";
    node_options.snapshot_uri = prefix + "/snapshot";
    node_options.disable_cli = false;
    node_options.witness = config_.ha_is_witness;
    int r = node->init(node_options);
    if (r != 0) {
        ::lgraph::StateMachine::Stop();
        throw std::runtime_error(
            fma_common::StringFormatter::Format("Failed to init raft node: ec={}", r));
    }
    node_ = node;

    // start HA by initial_conf
    if (config_.ha_bootstrap_role != 2) {
        LOG_INFO() << "Start HA by initial_conf";
        for (int t = 0; t < config_.ha_node_join_group_s; t++) {
            LOG_INFO() << "Waiting to join replication group...";
            fma_common::SleepS(1);
            if (joined_group_.load(std::memory_order_acquire)) return;
        }
    }

    // start HA by add_peer
    if (config_.ha_bootstrap_role != 1) {
        LOG_INFO() << "Start HA by add_peer";
        braft::Configuration init_conf;
        if (init_conf.parse_from(config_.ha_conf) != 0) {
            ::lgraph::StateMachine::Stop();
            throw std::runtime_error("Fail to parse configuration " + config_.ha_conf);
        }
        for (int t = 0; t < config_.ha_node_join_group_s; t++) {
            fma_common::SleepS(1);
            LOG_INFO() << "Waiting to join replication group...";
            braft::PeerId my_id(addr);
            butil::Status status = braft::cli::add_peer(braft::GroupId("lgraph"), init_conf,
                                                        my_id, braft::cli::CliOptions());
            if (status.ok())
                return;
            else
                LOG_WARN() << "Failed to join group: " << status.error_str();
        }
    }
    throw std::runtime_error("Failed to start node && join group!");
}

void lgraph::HaStateMachine::Stop() {
    {
        std::lock_guard<std::mutex> l(hb_mutex_);
        exit_flag_ = true;
        hb_cond_.notify_all();
    }
    if (heartbeat_thread_.joinable()) heartbeat_thread_.join();
    if (node_) {
        node_->shutdown(nullptr);
        node_->join();
        delete node_;
        node_ = nullptr;
    }
    ::lgraph::StateMachine::Stop();
}

int64_t lgraph::HaStateMachine::GetVersion() { return galaxy_->GetRaftLogIndex(); }

bool lgraph::HaStateMachine::DoRequest(bool is_write, const LGraphRequest* req,
                                       LGraphResponse* resp, google::protobuf::Closure* on_done) {
    brpc::ClosureGuard done_guard(on_done);
    if (req->Req_case() == LGraphRequest::kHaRequest) {
        return ApplyHaRequest(req, resp);
    } else {
        if (is_write && !IsCurrentMaster()) return RespondRedirect(resp, master_rpc_addr_);
#if LGRAPH_SHARE_DIR
        return ApplyRequestDirectly(req, resp);
#else
        if (is_write) {
            // return value of this function is used to decide whether to write backup log
            // It is ok to return true on failure, since re-execution of this same request
            // will fail and thus has no effect.
            std::string user = galaxy_->ParseAndValidateToken(req->token());
            (const_cast<LGraphRequest*>(req))->set_user(user);
            return ReplicateAndApplyRequest(req, resp, done_guard.release());
        } else {
            return ApplyRequestDirectly(req, resp);
        }
#endif
    }
}

void lgraph::HaStateMachine::on_leader_start(int64_t term) {
    leader_term_.store(term, std::memory_order_release);
    joined_group_.store(true, std::memory_order_release);
#if LGRAPH_SHARE_DIR
    _HoldWriteLock(galaxy_->GetRWLock());
    galaxy_->ReloadFromDisk();
#endif
    LOG_INFO() << "Node becomes leader";
}

void lgraph::HaStateMachine::on_leader_stop(const butil::Status& status) {
    leader_term_.store(-1, std::memory_order_release);
    LOG_DEBUG() << "Node stepped down : " << status;
}

void lgraph::HaStateMachine::on_shutdown() { LOG_DEBUG() << "This node is down";
}

void lgraph::HaStateMachine::on_error(const ::braft::Error& e) {
    // LeaveGroup();
    LOG_INFO() << "Met raft error " << e;
}

void lgraph::HaStateMachine::on_configuration_committed(const ::braft::Configuration& conf) {
    LOG_DEBUG() << "Configuration of this group is " << conf;
    if (node_) {
        LOG_INFO() << "Master becomes: " << node_->leader_id().to_string();
    }
}

void lgraph::HaStateMachine::on_stop_following(const ::braft::LeaderChangeContext& ctx) {
    LOG_INFO() << "Node stops following " << ctx;
}

void lgraph::HaStateMachine::on_start_following(const ::braft::LeaderChangeContext& ctx) {
    joined_group_.store(true, std::memory_order_release);
    LOG_INFO() << "Node joined the group as follower.";
}

bool lgraph::HaStateMachine::ReplicateAndApplyRequest(const LGraphRequest* req,
                                                      LGraphResponse* resp,
                                                      google::protobuf::Closure* on_done) {
    brpc::ClosureGuard done_guard(on_done);
    // Serialize request to the replicated write-ahead-log so that all the
    // peers in the group receive this request as well.
    // Notice that _value can't be modified in this routine otherwise it
    // will be inconsistent with others in this group.
    butil::IOBuf log;
    butil::IOBufAsZeroCopyOutputStream wrapper(&log);
    if (!req->SerializeToZeroCopyStream(&wrapper)) {
        LOG_ERROR() << "Fail to serialize request";
        return RespondException(resp, "Internal error: failed to serialize request.");
    }
    braft::Task task;
    task.data = &log;
    task.expected_term = leader_term_.load(std::memory_order_acquire);
    task.done = new ReqRespClosure(this, req, resp, done_guard.release());
    // Now the task is applied to the group, waiting for the result.
    node_->apply(task);
    return true;
}

void* lgraph::HaStateMachine::save_snapshot(void* arg) {
    auto* sa = (SnapshotArg*) arg;
    std::unique_ptr<SnapshotArg> arg_guard(sa);
    brpc::ClosureGuard closure_guard(sa->done);
    std::string path = sa->writer->get_path() + "/_snapshot_";
    try {
        LOG_DEBUG() << "Saving snapshot to " << path;
        _HoldReadLock(sa->haStateMachine->galaxy_->GetReloadLock());
        // bthread_usleep(30 * 1000000);
        auto files = sa->haStateMachine->galaxy_->SaveSnapshot(path);
        LOG_DEBUG() << "Snapshot files: " << fma_common::ToString(files);
        for (auto& f : files) {
            // normalize file path
            auto pos = f.find("_snapshot_");
            if (sa->writer->add_file(f.substr(pos)) != 0) {
                sa->done->status().set_error(EIO, "Fail to add file to writer");
                return NULL;
            }
        }
        sa->writer->list_files(&files);
        LOG_DEBUG() << "Actual files in snapshot: " << fma_common::ToString(files);
    } catch (std::exception& e) {
        LOG_ERROR() << "Failed to save snapshot to " << path << ": " << e.what();
        sa->done->status().set_error(EIO, "Failed to save snapshot: %s", e.what());
    }
    return NULL;
}

void lgraph::HaStateMachine::on_snapshot_save(braft::SnapshotWriter* writer, braft::Closure* done) {
#if LGRAPH_SHARE_DIR
    std::string path = writer->get_path() + "/snapshot.dummy";
    LOG_DEBUG() << "Saving snapshot to " << path;
    braft::AsyncClosureGuard closure_guard(done);
    std::ofstream dummy_file(path);
    _HoldReadLock(galaxy_->GetReloadLock());
    dummy_file << galaxy_->GetRaftLogIndex();
    dummy_file.close();
    writer->add_file("snapshot.dummy");
    LOG_DEBUG() << "Snapshot saved";
#else
    auto* arg = new SnapshotArg;
    arg->writer = writer;
    arg->done = done;
    arg->haStateMachine = this;
    bthread_t tid;
    bthread_start_urgent(&tid, NULL, save_snapshot, arg);
#endif
}

int lgraph::HaStateMachine::on_snapshot_load(braft::SnapshotReader* reader) {
#if LGRAPH_SHARE_DIR
    return 0;
#else
    // Load snapshot from reader, replacing the running StateMachine
    FMA_ASSERT(!IsLeader()) << "Leader is not supposed to load snapshot";
    std::string snapshot_path = reader->get_path() + "/_snapshot_";
    LOG_DEBUG() << "Loading snapshot from " << snapshot_path;
    try {
        braft::SnapshotMeta meta;
        int r = reader->load_meta(&meta);
        if (r != 0) {
            LOG_WARN() << "Error loading snapshot: " << r;
            return r;
        }
        std::vector<std::string> files;
        reader->list_files(&files);
        LOG_DEBUG() << "Snapshot files: " << fma_common::ToString(files);
        galaxy_->LoadSnapshot(snapshot_path);
        LOG_DEBUG() << "Successfully loaded snapshot";
    } catch (std::exception& e) {
        LOG_FATAL() << "Failed to load snapshot from " << snapshot_path << ": "
                                  << e.what();
        reader->set_error(EIO, "error loading snapshot");
    }
    return 0;
#endif
}

void lgraph::HaStateMachine::on_apply(braft::Iterator& iter) {
    // A batch of tasks are committed, which must be processed through
    // |iter|
    for (; iter.valid(); iter.next()) {
        if (config_.ha_is_witness) {
            LOG_DEBUG() << "addr " << node_->node_id().to_string()
                                    << " skip witness apply " << iter.index();
            continue;
        }
        braft::AsyncClosureGuard closure_guard(iter.done());
        const LGraphRequest* req;
        bool need_delete_req_resp = false;
        LGraphResponse* resp = nullptr;
        if (iter.done()) {
            // This task is applied by this node, get value from this
            // closure to avoid additional parsing.
            auto* c = dynamic_cast<ReqRespClosure*>(iter.done());
            resp = c->Response();
            req = c->Request();
        } else {
            // Have to parse FetchAddRequest from this log.
            butil::IOBufAsZeroCopyInputStream wrapper(iter.data());
            auto* request = new LGraphRequest();
            CHECK((request->ParseFromZeroCopyStream(&wrapper)));
            req = request;
            resp = new LGraphResponse;
            need_delete_req_resp = true;
        }
        // execute
        /* RAFT does not guarantee that each log is delivered to DB only once.
         * In order to make sure each log is applied exactly once to the DB, we commit its
         * log index to the DB. If the log index is not greater than current committed index,
         * then the log must have been applied. In that case, we should ignore it.
         */
        _HoldReadLock(galaxy_->GetReloadLock());
        int64_t committed_index = galaxy_->GetRaftLogIndex();
        LOG_DEBUG() << "Trying to apply log " << iter.index() << ", term "
                    << iter.term() << ", current_idx=" << committed_index;
        if (iter.index() % 1000 == 0) {
            LOG_WARN() << "Trying to apply log " << iter.index() << ", term "
                                    << iter.term() << ", current_idx=" << committed_index;
        }

        bool should_apply = (iter.index() > committed_index);
        if (should_apply) {
            ApplyRequestDirectly(req, resp);
            galaxy_->SetRaftLogIndexBeforeWrite(iter.index());
        } else {
            RespondBadInput(resp, fma_common::StringFormatter::Format(
                                      "Skipping old request. Request seq={}, current DB version={}",
                                      iter.index(), committed_index));
        }
        if (need_delete_req_resp) {
            delete req;
            delete resp;
        }
    }
}

bool lgraph::HaStateMachine::ApplyHaRequest(const LGraphRequest* req, LGraphResponse* resp) {
    FMA_DBG_ASSERT(req->Req_case() == LGraphRequest::kHaRequest);
    std::lock_guard<std::mutex> l(hb_mutex_);
    auto& hareq = req->ha_request();
    try {
        switch (hareq.Req_case()) {
        case HARequest::kHeartbeatRequest:
            {
                // handle heartbeat
                if (!node_->is_leader()) {
                    return RespondRedirect(resp, master_rpc_addr_);
                }
                auto& hbreq = hareq.heartbeat_request();
                auto it = heartbeat_states_.find(hbreq.rpc_addr());
                if (it == heartbeat_states_.end()) {
                    LOG_DEBUG() << "new peer joined: " << hbreq.DebugString();
                    HeartbeatStatus state;
                    state.rpc_addr = hbreq.rpc_addr();
                    state.rest_addr = hbreq.rest_addr();
                    it = heartbeat_states_.insert(it, std::make_pair(hbreq.rpc_addr(), state));
                }
                it->second.state = hbreq.state();
                it->second.role = hbreq.role();
                it->second.last_heartbeat = fma_common::GetTime();
                if (it->second.rest_addr.empty()) it->second.rest_addr = hbreq.rest_addr();
                return RespondSuccess(resp);
            }
        case HARequest::kGetMasterRequest:
            {
                if (!node_->is_leader()) {
                    return RespondRedirect(resp, master_rpc_addr_);
                }
                auto* gmresp = resp->mutable_ha_response()->mutable_get_master_response();
                auto* master = gmresp->mutable_master();
                master->set_rpc_addr(my_rpc_addr_);
                master->set_rest_addr(my_rest_addr_);
                master->set_state(NodeState::JOINED_MASTER);
                master->set_role(config_.ha_is_witness ? NodeRole::WITNESS : NodeRole::REPLICA);
                return RespondSuccess(resp);
            }
        case HARequest::kListPeersRequest:
            {
                if (!node_->is_leader()) {
                    return RespondRedirect(resp, master_rpc_addr_);
                }
                auto* peers =
                    resp->mutable_ha_response()->mutable_list_peers_response()->mutable_peers();
                peers->Reserve(heartbeat_states_.size() + 1);
                auto* peer = peers->Add();
                peer->set_state(NodeState::JOINED_MASTER);
                peer->set_rpc_addr(my_rpc_addr_);
                peer->set_rest_addr(my_rest_addr_);
                peer->set_role(config_.ha_is_witness ? NodeRole::WITNESS : NodeRole::REPLICA);
                for (auto& kv : heartbeat_states_) {
                    peer = peers->Add();
                    peer->set_state(kv.second.state);
                    peer->set_rpc_addr(kv.second.rpc_addr);
                    peer->set_rest_addr(kv.second.rest_addr);
                    peer->set_role(kv.second.role);
                }
                return RespondSuccess(resp);
            }
        case HARequest::kSyncMetaRequest:
            {
                if (!node_->is_leader()) {
                    return RespondSuccess(resp);
                }
                _HoldReadLock(galaxy_->GetReloadLock());
                bool is_admin = false;
                GetCurrUser(req, &is_admin);
                if (!is_admin) {
                    return RespondDenied(resp, "Only admin can issue SyncMeta.");
                }
                galaxy_->ReloadFromDisk();
                return RespondSuccess(resp);
            }
        default:
            LOG_WARN() << "Unhandled ha request type: " << req->Req_case();
            return RespondException(resp, "Unhandled ha request type.");
        }
    } catch (lgraph_api::LgraphException& e) {
        switch (e.code()) {
            case lgraph_api::ErrorCode::TaskKilled: {
                return RespondException(resp, e.msg());
            }
            case lgraph_api::ErrorCode::Unauthorized: {
                return RespondDenied(resp, e.msg());
            }
            case lgraph_api::ErrorCode::Timeout: {
                return RespondTimeout(resp, e.msg());
            }
            case lgraph_api::ErrorCode::InputError: {
                return RespondBadInput(resp, e.msg());
            }
            default: {
                return RespondException(resp, e.msg());
            }
        }
    } catch (std::exception& e) {
        return RespondException(resp, e.what());
    }
}

void lgraph::HaStateMachine::HeartbeatThread() {
    std::unique_lock<std::mutex> l(hb_mutex_);
    while (!exit_flag_) {
        hb_cond_.wait_for(l, std::chrono::milliseconds(config_.ha_heartbeat_interval_ms));
        if (exit_flag_) return;
        if (!node_) continue;
        try {
            if (node_->is_leader()) {
                // if we have just started as master, then initialize heartbeat list
                ScanHeartbeatStatusLocked();
            } else {
                SendHeartbeatToMasterLocked();
            }
        } catch (std::exception& e) {
            LOG_WARN() << "Exception in heartbeat thread: " << e.what();
        }
    }
}

void lgraph::HaStateMachine::ScanHeartbeatStatusLocked() {
    double now = fma_common::GetTime();
    double kick_timeline = now - (double)config_.ha_node_offline_ms / 1000;
    double dismiss_timeline = now - (double)config_.ha_node_remove_ms / 1000;

    if (!master_rpc_addr_.empty()) {
        master_rpc_addr_.clear();
        master_rest_addr_.clear();
        // if there is recent peer change, we should synchronize peer list
        std::vector<braft::PeerId> peers;
        auto status = node_->list_peers(&peers);
        if (!status.ok()) {
            LOG_WARN() << "Failed to list peers: " << status.error_str();
            return;
        }
        std::unordered_set<std::string> rpc_addrs;
        for (auto& p : peers) {
            auto str = butil::endpoint2str(p.addr);
            rpc_addrs.insert(str.c_str());
        }
        rpc_addrs.erase(my_rpc_addr_);
        for (auto& p : peers) {
            // make sure every follow is in the heartbeat list
            auto str = std::string(butil::endpoint2str(p.addr).c_str());
            auto it = heartbeat_states_.find(str);
            if (it == heartbeat_states_.end()) {
                HeartbeatStatus state;
                state.last_heartbeat = now;
                state.state = NodeState::JOINED_FOLLOW;
                if (p.is_witness()) {
                    state.role = NodeRole::WITNESS;
                } else {
                    state.role = NodeRole::REPLICA;
                }
                state.rpc_addr = str;
                heartbeat_states_.insert(it, std::make_pair(str, state));
            }
        }
        // make sure we don't track follow that has left
        for (auto it = heartbeat_states_.begin(); it != heartbeat_states_.end();) {
            auto& p = it->second;
            if (p.state == NodeState::JOINED_FOLLOW) {
                if (rpc_addrs.find(p.rpc_addr) == rpc_addrs.end()) {
                    LOG_DEBUG() << "removing follow since it has died: " << p.rpc_addr;
                    it = heartbeat_states_.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }

    for (auto it = heartbeat_states_.begin(); it != heartbeat_states_.end();) {
        auto& state = it->second;
        if (state.last_heartbeat < kick_timeline) {
            if (state.state == NodeState::JOINED_FOLLOW) {
                // timeout, need to kick out, but still stays in list
                state.state = NodeState::OFFLINE;
                // remove peer from group
                LOG_DEBUG() << "Kicking out dead node: " << state.rpc_addr;
                node_->remove_peer(braft::PeerId(state.rpc_addr), nullptr);
            }
            if (state.last_heartbeat < dismiss_timeline) {
                // died for a long time, remove it from list
                LOG_DEBUG() << "Removing long dead node from list: " << state.rpc_addr;
                it = heartbeat_states_.erase(it);
                continue;
            }
        }
        ++it;
    }
}

void lgraph::HaStateMachine::SendHeartbeatToMasterLocked() {
    if (butil::endpoint2str(node_->leader_id().addr).c_str() != master_rpc_addr_.c_str()) {
        std::unique_ptr<brpc::Channel> channel(new brpc::Channel);
        if (channel->Init(node_->leader_id().addr, nullptr) != 0) {
            LOG_WARN() << "failed to connect to master " << node_->leader_id().to_string();
            return;
        }
        rpc_stub_.reset(new LGraphRPCService_Stub(channel.release(),
                                                  google::protobuf::Service::STUB_OWNS_CHANNEL));
        // get master addresses
        LGraphRequest req;
        req.set_token("");
        req.set_is_write_op(false);
        req.mutable_ha_request()->mutable_get_master_request();
        LGraphResponse resp;
        brpc::Controller controller;
        controller.set_timeout_ms(1000);
        controller.set_max_retry(3);
        rpc_stub_->HandleRequest(&controller, &req, &resp, nullptr);
        if (controller.Failed() || resp.error_code() != LGraphResponse::SUCCESS) {
            if (controller.Failed()) {
                LOG_WARN() << "Connection failed, error getting master addr: "
                           << controller.ErrorText();
            } else {
                LOG_WARN() << "Rpc failed, error getting master addr: " << resp.error();
            }
            return;
        }
        auto& master = resp.ha_response().get_master_response().master();
        master_rpc_addr_ = master.rpc_addr();
        master_rest_addr_ = master.rest_addr();
    }
    FMA_DBG_ASSERT(rpc_stub_);
    LGraphRequest req;
    req.set_token("");
    req.set_is_write_op(false);
    auto* hbreq = req.mutable_ha_request()->mutable_heartbeat_request();
    hbreq->set_rpc_addr(my_rpc_addr_);
    hbreq->set_rest_addr(my_rest_addr_);
    hbreq->set_state(NodeState::JOINED_FOLLOW);
    if (config_.ha_is_witness) {
        hbreq->set_role(NodeRole::WITNESS);
    } else {
        hbreq->set_role(NodeRole::REPLICA);
    }
    LGraphResponse resp;
    brpc::Controller controller;
    controller.set_timeout_ms(1000);
    controller.set_max_retry(3);
    rpc_stub_->HandleRequest(&controller, &req, &resp, nullptr);
    if (controller.Failed() || resp.error_code() != LGraphResponse::SUCCESS) {
        LOG_WARN() << "Failed to send heartbeat: " << resp.error();
        return;
    }
}

void lgraph::HaStateMachine::LeaveGroup() {
    if (node_) {
        // if i am the leader, transfer leadership
        if (node_->is_leader()) {
            std::vector<braft::PeerId> peers;
            node_->list_peers(&peers);
            // do not remove the last peer, or the group won't be able to restart
            if (peers.size() <= 1) return;
            int ec = node_->transfer_leadership_to(braft::ANY_PEER);
            if (ec != 0) {
                LOG_WARN() << "Failed to transfer leadership in LeaveGroup, ec=" << ec;
                return;
            } else {
                LOG_DEBUG() << "Node is no longer leader.";
            }
        }
        // remove myself from replication group
        braft::Configuration conf;
        conf.add_peer(node_->leader_id());
        LOG_INFO() << "Leaving replication group...";
        butil::Status status;
        for (int i = 0; i < 3; i++) {
            status = braft::cli::remove_peer(braft::GroupId("lgraph"), conf,
                                             node_->node_id().peer_id, braft::cli::CliOptions());
            if (!status.ok()) fma_common::SleepS(0.5);
        }
        if (!status.ok()) {
            LOG_WARN() << "Failed to leave group: " << status.error_str();
        }
    }
}

std::vector<lgraph::HaStateMachine::Peer> lgraph::HaStateMachine::ListPeers() const {
    std::vector<Peer> ret;
    if (node_->is_leader()) {
        std::lock_guard<std::mutex> l(hb_mutex_);
        ret.reserve(heartbeat_states_.size() + 1);
        ret.emplace_back(my_rpc_addr_, my_rest_addr_, NodeState::JOINED_MASTER,
                         config_.ha_is_witness ? NodeRole::WITNESS : NodeRole::REPLICA);
        for (auto& p : heartbeat_states_) {
            ret.emplace_back(p.second.rpc_addr, p.second.rest_addr, p.second.state,
                             p.second.role);
        }
        return ret;
    } else {
        std::shared_ptr<LGraphRPCService_Stub> rpc_stub;
        {
            std::lock_guard<std::mutex> l(hb_mutex_);
            rpc_stub = rpc_stub_;
        }
        if (!rpc_stub) return ret;
        // get peers
        LGraphRequest req;
        req.set_token("");
        req.set_is_write_op(false);
        req.mutable_ha_request()->mutable_list_peers_request();
        LGraphResponse resp;
        brpc::Controller controller;
        controller.set_timeout_ms(1000);
        controller.set_max_retry(3);
        rpc_stub->HandleRequest(&controller, &req, &resp, nullptr);
        if (controller.Failed() || resp.error_code() != LGraphResponse::SUCCESS) {
            if (controller.Failed()) {
                LOG_WARN() << "Error getting peers from master: " << controller.ErrorText();
            } else {
                LOG_WARN() << "Error getting peers from master: " << resp.error();
            }
            return ret;
        }
        auto& peers = resp.ha_response().list_peers_response().peers();
        ret.clear();
        ret.reserve(peers.size());
        for (auto& p : peers) {
            ret.emplace_back(p.rpc_addr(), p.rest_addr(), p.state(), p.role());
        }
        return ret;
    }
}
