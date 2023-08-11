/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "braft/protobuf_file.h"
#include "braft/raft.h"
#include "braft/util.h"

#include "brpc/channel.h"
#include "braft/cli.h"
#include "brpc/closure_guard.h"

#include "server/ha_state_machine.h"
#include "restful/server/rest_server.h"

void lgraph::HaStateMachine::Start() {
    static const int64_t BOOTSTRAP_LOG_INDEX = 1024;

    if (node_) {
        GENERAL_LOG_STREAM(WARNING, logger_.GetName()) << "HaStateMachine already started.";
        return;
    }

    ::lgraph::StateMachine::Start();
    if (config_.ha_bootstrap_role == 1) {
#if LGRAPH_SHARE_DIR
        GENERAL_LOG(WARNING) << "Bootstrapping is not necessary in this version, ignored";
#else
        if (galaxy_->GetRaftLogIndex() != -1) {
            GENERAL_LOG_STREAM(INFO, logger_.GetName()) << "Bootstrapping from existing data...";
        }

        GENERAL_LOG(DEBUG) << "Bootstrapping...";
        int64_t new_log_index = galaxy_->GetRaftLogIndex() + BOOTSTRAP_LOG_INDEX;
        // change the log index in DB
        galaxy_->BootstrapRaftLogIndex(new_log_index);
        // now bootstrap
        braft::BootstrapOptions options;
        if (options.group_conf.parse_from(config_.ha_conf) != 0) {
            ::lgraph::StateMachine::Stop();
            throw std::runtime_error("Fail to parse configuration " + config_.ha_conf);
        }
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
        GENERAL_LOG(DEBUG) << "Bootstrap succeed. Log index set to " << galaxy_->GetRaftLogIndex();
#endif
    }

    butil::EndPoint addr;
    butil::str2endpoint(config_.host.c_str(), config_.rpc_port, &addr);
    if (butil::IP_ANY == addr.ip) {
        throw std::runtime_error("TuGraph can't be started from IP_ANY (0.0.0.0) in HA mode.");
    }
    braft::Node* node = new braft::Node("lgraph", braft::PeerId(addr));
    braft::NodeOptions node_options;
    if (node_options.initial_conf.parse_from(config_.ha_conf) != 0) {
        ::lgraph::StateMachine::Stop();
        throw std::runtime_error("Fail to parse configuration " + config_.ha_conf);
    }
    node_options.election_timeout_ms = config_.ha_election_timeout_ms;
    node_options.fsm = this;
    node_options.node_owns_fsm = false;
    node_options.snapshot_interval_s = config_.ha_snapshot_interval_s;
    std::string prefix = "local://" + config_.ha_dir;
    node_options.log_uri = prefix + "/log";
    node_options.raft_meta_uri = prefix + "/raft_meta";
    node_options.snapshot_uri = prefix + "/snapshot";
    node_options.disable_cli = false;
    int r = node->init(node_options);
    if (r != 0) {
        ::lgraph::StateMachine::Stop();
        throw std::runtime_error(
            fma_common::StringFormatter::Format("Failed to init raft node: ec={}", r));
    }
    node_ = node;
    if (config_.ha_bootstrap_role != 2) {
        GENERAL_LOG_STREAM(INFO, logger_.GetName()) << "Start HA by initial_conf";
        int t = 0;
        while (!joined_group_.load(std::memory_order_acquire)) {
            GENERAL_LOG_STREAM(INFO, logger_.GetName()) << "Waiting to join replication group...";
            if (t++ > config_.ha_node_join_group_s) {
                break;
            }
            fma_common::SleepS(1);
        }
        if (t <= config_.ha_node_join_group_s) {
            return;
        }
    }
    if (config_.ha_bootstrap_role != 1) {
        GENERAL_LOG_STREAM(INFO, logger_.GetName()) << "Start HA by add_peer";
        braft::Configuration init_conf;
        int t = 0;
        while (!joined_group_.load(std::memory_order_acquire)) {
            if (init_conf.parse_from(config_.ha_conf) == 0) {
                braft::PeerId my_id(addr);
                butil::Status status = braft::cli::add_peer(braft::GroupId("lgraph"), init_conf,
                                                            my_id, braft::cli::CliOptions());
                if (!status.ok()) {
                    GENERAL_LOG_STREAM(WARNING, logger_.GetName()) <<
                        "Failed to join group: " << status.error_str();
                }
            }
            GENERAL_LOG_STREAM(INFO, logger_.GetName()) << "Waiting to join replication group...";
            if (t++ > config_.ha_node_join_group_s) {
                throw std::runtime_error("Failed to join replication group");
            }
            fma_common::SleepS(1);
        }
    }
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
        if (is_write && !IsCurrentMaster()) return RespondRedirect(resp, master_rest_addr_);
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
    GENERAL_LOG_STREAM(INFO, logger_.GetName()) << "Node becomes leader";
    joined_group_ = true;
    std::lock_guard<std::mutex> l(hb_mutex_);
    peers_changed_ = true;
    node_state_ = NodeState::JOINED_MASTER;
#if LGRAPH_SHARE_DIR
    _HoldWriteLock(galaxy_->GetRWLock());
    galaxy_->ReloadFromDisk();
#endif
}

void lgraph::HaStateMachine::on_leader_stop(const butil::Status& status) {
    leader_term_.store(-1, std::memory_order_release);
    GENERAL_LOG_STREAM(DEBUG, logger_.GetName()) << "Node stepped down : " << status;
    std::lock_guard<std::mutex> l(hb_mutex_);
    peers_changed_ = true;
    node_state_ = NodeState::JOINED_FOLLOW;
}

void lgraph::HaStateMachine::on_shutdown() {
    GENERAL_LOG_STREAM(DEBUG, logger_.GetName()) << "This node is down";
}

void lgraph::HaStateMachine::on_error(const ::braft::Error& e) {
    LeaveGroup();
    GENERAL_LOG_STREAM(DEBUG, logger_.GetName()) << "Met raft error " << e;
}

void lgraph::HaStateMachine::on_configuration_committed(const ::braft::Configuration& conf) {
    GENERAL_LOG_STREAM(DEBUG, logger_.GetName()) << "Configuration of this group is " << conf;
    if (node_) {
        GENERAL_LOG_STREAM(INFO, logger_.GetName()) <<
            "Master becomes: " << node_->leader_id().to_string();
    }
    std::lock_guard<std::mutex> l(hb_mutex_);
    peers_changed_ = true;
}

void lgraph::HaStateMachine::on_stop_following(const ::braft::LeaderChangeContext& ctx) {
    GENERAL_LOG_STREAM(DEBUG, logger_.GetName()) << "Node stops following " << ctx;
    std::lock_guard<std::mutex> l(hb_mutex_);
    peers_changed_ = true;
    node_state_ = NodeState::UNINITIALIZED;
}

void lgraph::HaStateMachine::on_start_following(const ::braft::LeaderChangeContext& ctx) {
    GENERAL_LOG_STREAM(INFO, logger_.GetName()) << "Node joined the group as follower.";
    joined_group_ = true;
    std::lock_guard<std::mutex> l(hb_mutex_);
    peers_changed_ = true;
    node_state_ = NodeState::JOINED_FOLLOW;
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
        FMA_ERR_STREAM(logger_) << "Fail to serialize request";
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

void lgraph::HaStateMachine::on_snapshot_save(braft::SnapshotWriter* writer, braft::Closure* done) {
#if LGRAPH_SHARE_DIR
    std::string path = writer->get_path() + "/snapshot.dummy";
    GENERAL_LOG_STREAM(DEBUG, logger_.GetName()) << "Saving snapshot to " << path;
    braft::AsyncClosureGuard closure_guard(done);
    std::ofstream dummy_file(path);
    _HoldReadLock(galaxy_->GetReloadLock());
    dummy_file << galaxy_->GetRaftLogIndex();
    dummy_file.close();
    writer->add_file("snapshot.dummy");
    GENERAL_LOG_STREAM(DEBUG, logger_.GetName()) << "Snapshot saved";
#else
    std::mutex mu;
    std::condition_variable cv;
    bool ok_to_leave = false;
    std::thread worker([&]() {
        // TODO: take snapshot asynchronously // NOLINT
        {
            std::string path = writer->get_path() + "/_snapshot_";
            GENERAL_LOG_STREAM(DEBUG, logger_.GetName()) << "Saving snapshot to " << path;
            braft::AsyncClosureGuard closure_guard(done);
            std::lock_guard<std::mutex> l(mu);
            try {
                _HoldReadLock(galaxy_->GetReloadLock());
                auto files = galaxy_->SaveSnapshot(path);
                GENERAL_LOG_STREAM(DEBUG, logger_.GetName()) <<
                    "Snapshot files: " << fma_common::ToString(files);
                for (auto& f : files) {
                    // normalize file path
                    auto pos = f.find("_snapshot_");
                    if (writer->add_file(f.substr(pos)) != 0) {
                        done->status().set_error(EIO, "Fail to add file to writer");
                        return;
                    }
                }
                writer->list_files(&files);
                GENERAL_LOG_STREAM(DEBUG, logger_.GetName())
                    << "Actual files in snapshot: " << fma_common::ToString(files);
            } catch (std::exception& e) {
                GENERAL_LOG_STREAM(ERROR, logger_.GetName()) <<
                    "Failed to save snapshot to " << path << ": " << e.what();
                done->status().set_error(EIO, "Failed to save snapshot: %s", e.what());
            }
            ok_to_leave = true;
            cv.notify_all();
        }
    });
    worker.detach();
    std::unique_lock<std::mutex> l(mu);
    while (!ok_to_leave) cv.wait(l);
#endif
}

int lgraph::HaStateMachine::on_snapshot_load(braft::SnapshotReader* reader) {
#if LGRAPH_SHARE_DIR
    return 0;
#else
    // Load snapshot from reader, replacing the running StateMachine
    FMA_ASSERT(!IsLeader()) << "Leader is not supposed to load snapshot";
    std::string snapshot_path = reader->get_path() + "/_snapshot_";
    GENERAL_LOG_STREAM(DEBUG, logger_.GetName()) << "Loading snapshot from " << snapshot_path;
    try {
        braft::SnapshotMeta meta;
        int r = reader->load_meta(&meta);
        if (r != 0) {
            GENERAL_LOG_STREAM(WARNING, logger_.GetName()) << "Error loading snapshot: " << r;
            return r;
        }
        std::vector<std::string> files;
        reader->list_files(&files);
        GENERAL_LOG_STREAM(DEBUG, logger_.GetName()) <<
            "Snapshot files: " << fma_common::ToString(files);
        galaxy_->LoadSnapshot(snapshot_path);
        GENERAL_LOG_STREAM(DEBUG, logger_.GetName()) << "Successfully loaded snapshot";
    } catch (std::exception& e) {
        GENERAL_LOG_STREAM(FATAL, logger_.GetName()) <<
            "Failed to load snapshot from " << snapshot_path << ": "
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
        FMA_DBG_STREAM(logger_) << "Trying to apply log " << iter.index() << ", term "
                                << iter.term() << ", current_idx=" << committed_index;

        bool should_apply = (iter.index() > committed_index);
        if (should_apply) {
            galaxy_->SetRaftLogIndexBeforeWrite(iter.index());
            ApplyRequestDirectly(req, resp);
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
                if (node_state_ != NodeState::JOINED_MASTER) {
                    return RespondRedirect(resp, master_rpc_addr_);
                }
                auto& hbreq = hareq.heartbeat_request();
                auto it = heartbeat_states_.find(hbreq.rpc_addr());
                if (it == heartbeat_states_.end()) {
                    GENERAL_LOG_STREAM(DEBUG, logger_.GetName()) <<
                        "new peer joined: " << hbreq.DebugString();
                    HeartbeatStatus state;
                    state.rpc_addr = hbreq.rpc_addr();
                    state.rest_addr = hbreq.rest_addr();
                    it = heartbeat_states_.insert(it, std::make_pair(hbreq.rpc_addr(), state));
                }
                it->second.state = hbreq.state();
                it->second.last_heartbeat = fma_common::GetTime();
                if (it->second.rest_addr.empty()) it->second.rest_addr = hbreq.rest_addr();
                return RespondSuccess(resp);
            }
        case HARequest::kGetMasterRequest:
            {
                if (node_state_ != NodeState::JOINED_MASTER) {
                    return RespondRedirect(resp, master_rpc_addr_);
                }
                auto* gmresp = resp->mutable_ha_response()->mutable_get_master_response();
                auto* master = gmresp->mutable_master();
                master->set_rpc_addr(my_rpc_addr_);
                master->set_rest_addr(my_rest_addr_);
                master->set_state(NodeState::JOINED_MASTER);
                return RespondSuccess(resp);
            }
        case HARequest::kListPeersRequest:
            {
                if (node_state_ != NodeState::JOINED_MASTER) {
                    return RespondRedirect(resp, master_rpc_addr_);
                }
                auto* peers =
                    resp->mutable_ha_response()->mutable_list_peers_response()->mutable_peers();
                peers->Reserve(heartbeat_states_.size() + 1);
                auto* peer = peers->Add();
                peer->set_state(NodeState::JOINED_MASTER);
                peer->set_rpc_addr(my_rpc_addr_);
                peer->set_rest_addr(my_rest_addr_);
                for (auto& kv : heartbeat_states_) {
                    peer = peers->Add();
                    peer->set_state(kv.second.state);
                    peer->set_rpc_addr(kv.second.rpc_addr);
                    peer->set_rest_addr(kv.second.rest_addr);
                }
                return RespondSuccess(resp);
            }
        case HARequest::kSyncMetaRequest:
            {
                if (node_state_ == NodeState::JOINED_MASTER) {
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
            GENERAL_LOG_STREAM(WARNING, logger_.GetName()) <<
                "Unhandled ha request type: " << req->Req_case();
            return RespondException(resp, "Unhandled ha request type.");
        }
    } catch (TimeoutException& e) {
        return RespondTimeout(resp, e.what());
    } catch (InputError& e) {
        return RespondBadInput(resp, e.what());
    } catch (AuthError& e) {
        return RespondDenied(resp, e.what());
    } catch (TaskKilledException& e) {
        return RespondException(resp, e.what());
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
            NodeState state = node_state_;
            switch (state) {
            case NodeState::JOINED_MASTER:
                // if we have just started as master, then initialize heartbeat list
                ScanHeartbeatStatusLocked();
                break;
            case NodeState::UNINITIALIZED:
                break;
            case NodeState::JOINED_FOLLOW:
            case NodeState::LOADING_SNAPSHOT:
            case NodeState::REPLAYING_LOG:
            default:
                SendHeartbeatToMasterLocked(state);
                break;
            }
        } catch (std::exception& e) {
            GENERAL_LOG_STREAM(WARNING, logger_.GetName()) <<
                "Exception in heartbeat thread: " << e.what();
        }
    }
}

void lgraph::HaStateMachine::ScanHeartbeatStatusLocked() {
    double now = fma_common::GetTime();
    double kick_timeline = now - (double)config_.ha_node_offline_ms / 1000;
    double dismiss_timeline = now - (double)config_.ha_node_remove_ms / 1000;

    for (auto it = heartbeat_states_.begin(); it != heartbeat_states_.end();) {
        auto& state = it->second;
        if (state.last_heartbeat < kick_timeline) {
            if (state.state == NodeState::JOINED_FOLLOW) {
                // timeout, need to kick out, but still stays in list
                state.state = NodeState::OFFLINE;
                // remove peer from group
                GENERAL_LOG_STREAM(DEBUG, logger_.GetName()) <<
                    "Kicking out dead node: " << state.rpc_addr;
                node_->remove_peer(braft::PeerId(state.rpc_addr + ":0"), nullptr);
            }
            if (state.last_heartbeat < dismiss_timeline) {
                // died for a long time, remove it from list
                GENERAL_LOG_STREAM(DEBUG, logger_.GetName()) <<
                    "Removing long dead node from list: " << state.rpc_addr;
                it = heartbeat_states_.erase(it);
                continue;
            }
        }
        ++it;
    }
    if (peers_changed_) {
        peers_changed_ = false;
        master_rest_addr_ = my_rest_addr_;
        master_rpc_addr_ = my_rpc_addr_;
        // if there is recent peer change, we should synchronize peer list
        std::vector<braft::PeerId> peers;
        auto status = node_->list_peers(&peers);
        if (!status.ok()) {
            GENERAL_LOG_STREAM(WARNING, logger_.GetName()) <<
                "Failed to list peers: " << status.error_str();
            return;
        }
        std::unordered_set<std::string> rpc_addrs;
        for (auto& p : peers) {
            auto str = butil::endpoint2str(p.addr);
            rpc_addrs.insert(str.c_str());
        }
        rpc_addrs.erase(my_rpc_addr_);
        for (auto& p : rpc_addrs) {
            // make sure every follow is in the heartbeat list
            auto it = heartbeat_states_.find(p);
            if (it == heartbeat_states_.end()) {
                HeartbeatStatus state;
                state.last_heartbeat = now;
                state.state = NodeState::JOINED_FOLLOW;
                state.rpc_addr = p;
                it = heartbeat_states_.insert(it, std::make_pair(p, state));
            }
        }
        // make sure we don't track follow that has left
        for (auto it = heartbeat_states_.begin(); it != heartbeat_states_.end();) {
            auto& p = it->second;
            if (p.state == NodeState::JOINED_FOLLOW) {
                if (rpc_addrs.find(p.rpc_addr) == rpc_addrs.end()) {
                    GENERAL_LOG_STREAM(DEBUG, logger_.GetName()) <<
                        "removing follow since it has died: " << p.rpc_addr;
                    it = heartbeat_states_.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }
}

void lgraph::HaStateMachine::SendHeartbeatToMasterLocked(lgraph::NodeState state) {
    if (peers_changed_) {
        std::unique_ptr<brpc::Channel> channel(new brpc::Channel);
        if (channel->Init(node_->leader_id().addr, nullptr) != 0) {
            GENERAL_LOG_STREAM(WARNING, logger_.GetName())
                << "failed to connect to master " << node_->leader_id().to_string();
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
                GENERAL_LOG_STREAM(WARNING, logger_.GetName()) <<
                    "Error getting master addr: " << controller.ErrorText();
            } else {
                GENERAL_LOG_STREAM(WARNING, logger_.GetName()) <<
                    "Error getting master addr: " << resp.error();
            }
            return;
        }
        auto& master = resp.ha_response().get_master_response().master();
        master_rpc_addr_ = master.rpc_addr();
        master_rest_addr_ = master.rest_addr();
        peers_changed_ = false;
    }
    FMA_DBG_ASSERT(rpc_stub_);
    LGraphRequest req;
    req.set_token("");
    req.set_is_write_op(false);
    auto* hbreq = req.mutable_ha_request()->mutable_heartbeat_request();
    hbreq->set_rpc_addr(my_rpc_addr_);
    hbreq->set_rest_addr(my_rest_addr_);
    hbreq->set_state(state);
    LGraphResponse resp;
    brpc::Controller controller;
    controller.set_timeout_ms(1000);
    controller.set_max_retry(3);
    rpc_stub_->HandleRequest(&controller, &req, &resp, nullptr);
    if (controller.Failed() || resp.error_code() != LGraphResponse::SUCCESS) {
        GENERAL_LOG_STREAM(WARNING, logger_.GetName()) <<
            "Failed to send heartbeat: " << resp.error();
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
                GENERAL_LOG_STREAM(WARNING, logger_.GetName())
                    << "Failed to transfer leadership in LeaveGroup, ec=" << ec;
                return;
            } else {
                GENERAL_LOG_STREAM(DEBUG, logger_.GetName()) << "Node is no longer leader.";
            }
        }
        // remove myself from replication group
        braft::Configuration conf;
        conf.add_peer(node_->leader_id());
        GENERAL_LOG_STREAM(INFO, logger_.GetName()) << "Leaving replication group...";
        butil::Status status;
        for (int i = 0; i < 3; i++) {
            status = braft::cli::remove_peer(braft::GroupId("lgraph"), conf,
                                             node_->node_id().peer_id, braft::cli::CliOptions());
            if (!status.ok()) fma_common::SleepS(0.5);
        }
        if (!status.ok()) {
            GENERAL_LOG_STREAM(WARNING, logger_.GetName()) <<
                "Failed to leave group: " << status.error_str();
        }
    }
}

std::vector<lgraph::HaStateMachine::Peer> lgraph::HaStateMachine::ListPeers() const {
    std::vector<Peer> ret;
    if (IsLeader()) {
        std::lock_guard<std::mutex> l(hb_mutex_);
        ret.reserve(heartbeat_states_.size());
        ret.emplace_back(my_rpc_addr_, my_rest_addr_, NodeState::JOINED_MASTER);
        for (auto& p : heartbeat_states_) {
            ret.emplace_back(p.second.rpc_addr, p.second.rest_addr, p.second.state);
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
                FMA_WARN_STREAM(logger_)
                    << "Error getting peers from master: " << controller.ErrorText();
            } else {
                FMA_WARN_STREAM(logger_) << "Error getting peers from master: " << resp.error();
            }
            return ret;
        }
        auto& peers = resp.ha_response().list_peers_response().peers();
        ret.clear();
        ret.reserve(peers.size());
        for (auto& p : peers) {
            ret.emplace_back(p.rpc_addr(), p.rest_addr(), p.state());
        }
        return ret;
    }
}
