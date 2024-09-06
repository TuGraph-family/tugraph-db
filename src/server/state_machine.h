/**
 * Copyright 2022 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

//
// Created by hct on 18-12-26.
//
#pragma once

#include <utility>

#include "fma-common/rotating_files.h"

// The 'U' macro can be used to create a string or character literal of the platform type, i.e.
// utility::char_t. If you are using a library causing conflicts with 'U' macro, it can be turned
// off by defining the macro '_TURN_OFF_PLATFORM_STRING' before including the C++ REST SDK header
// files, and e.g. use '_XPLATSTR' instead.
#define _TURN_OFF_PLATFORM_STRING
#include "cpprest/json.h"

#include "core/backup_log.h"
#include "core/global_config.h"
#include "core/task_tracker.h"
#include "lgraph/lgraph.h"
#include "plugin/plugin_manager.h"
#include "protobuf/ha.pb.h"
#include "server/proto_convert.h"

#ifndef _WIN32
#include "cypher/execution_plan/scheduler.h"
#endif

namespace lgraph {
class Galaxy;
class RestServer;

class SynchronousClosure : public google::protobuf::Closure {
    std::mutex mutex_;
    std::condition_variable cond_;
    bool done_ = false;

 public:
    ~SynchronousClosure() {}

    void Run() override {
        {
            std::lock_guard<std::mutex> l(mutex_);
            done_ = true;
        }
        cond_.notify_one();
    }

    void Wait() {
        std::unique_lock<std::mutex> l(mutex_);
        while (!done_) cond_.wait(l);
    }
};

struct MyDoneGuard {
    google::protobuf::Closure* on_done_;

    explicit MyDoneGuard(google::protobuf::Closure* on_done) : on_done_(on_done) {}

    ~MyDoneGuard() {
        if (on_done_) on_done_->Run();
    }

    google::protobuf::Closure* Release() {
        auto d = on_done_;
        on_done_ = nullptr;
        return d;
    }
};

class StateMachine {
 public:
    struct Config {
        std::string db_dir = "./testdb";
        bool durable = false;
        std::string host;
        uint16_t rpc_port = 0;
        bool optimistic_txn = false;

        Config() = default;
        explicit Config(const GlobalConfig& c) {
            db_dir = c.db_dir;
            durable = c.durable;
            host = c.bind_host;
            rpc_port = c.rpc_port;
            optimistic_txn = c.txn_optimistic;
        }
    };

    struct Peer {
        Peer() {}
        Peer(std::string rpca, std::string resta, NodeState ns, NodeRole role = NodeRole::REPLICA)
            : rpc_addr(std::move(rpca)), rest_addr(std::move(resta)), state(ns), role(role) {}

        std::string rpc_addr;
        std::string rest_addr;
        NodeState state;
        NodeRole role;

        [[nodiscard]] std::string StateString() const {
            switch (state) {
            case NodeState::UNINITIALIZED:
                return "UNINITIALIZED";
            case NodeState::LOADING_SNAPSHOT:
                return "LOADING_SNAPSHOT";
            case NodeState::REPLAYING_LOG:
                return "REPLAYING_LOG";
            case NodeState::JOINED_FOLLOW:
                return "FOLLOW";
            case NodeState::JOINED_MASTER:
                return "MASTER";
            case NodeState::OFFLINE:
                return "OFFLINE";
            default:
                return "UNKNOWN";
            }
        }

        [[nodiscard]] std::string RoleString() const {
            switch (role) {
            case WITNESS:
                return "WITNESS";
            case REPLICA:
            default:
                return "REPLICA";
            }
        }
    };

 protected:
    Config config_;
    std::shared_ptr<GlobalConfig> global_config_;
    std::unique_ptr<Galaxy> galaxy_;
#ifndef _WIN32
    cypher::Scheduler cypher_scheduler_;
#endif
    double start_time_ = fma_common::GetTime();

    std::unique_ptr<BackupLog> backup_log_;

 public:
    StateMachine(const Config& config, std::shared_ptr<GlobalConfig> global_config);
    virtual ~StateMachine();

    virtual void Start();

    inline Galaxy* GetGalaxy() { return galaxy_.get(); }

    inline bool IsCypherV2() { return global_config_->is_cypher_v2; }

#ifndef _WIN32
    inline cypher::Scheduler* GetCypherScheduler() { return &cypher_scheduler_; }
#endif
    virtual double GetUpTimeInSeconds() const { return fma_common::GetTime() - start_time_; }

    virtual TaskTracker::Stats GetStats() const { return TaskTracker::GetInstance().GetStats(); }

    /**
     * Gets the current version in HA mode. In non-HA mode, it returns 0.
     *
     * @return  The version.
     */
    virtual int64_t GetVersion();

    /**
     * @brief  Set token to unlimited time
     */
    void SetTokenTimeUnlimited();

    /**
     * @brief  reset password if the password is forgotten
     */
    bool ResetAdminPassword(const std::string& user,
        const std::string& new_password, bool set_password);

    /**
     * Gets current master rest address.
     *
     * @return  The current master.
     */
    virtual std::string GetMasterRestAddr() { return ""; }

    virtual std::string GetMasterRpcAddr() { return ""; }

    /**
     * Is this state machine the current master? In non-HA mode, it returns true.
     *
     * @return  True if current master, false if not.
     */
    virtual bool IsCurrentMaster() { return true; }

    /** Stops this state machine. */
    virtual void Stop();

    /**
     * Handles a request. For single machine, this directly translates to the operations. For HA, it
     * will parse the request and replicate the write requests. Read requests will be served in
     * slave servers if request.last_txn_id &lt; curr_txn_id.
     *
     * @param [in,out] req      The request.
     * @param [in,out] resp     The response.
     * @param [in,out] on_done  If non-null, the callback.
     */
    virtual void HandleRequest(::google::protobuf::RpcController* controller,
                               const LGraphRequest* req, LGraphResponse* resp,
                               google::protobuf::Closure* on_done);

    bool ApplyRequestDirectly(const LGraphRequest* req, LGraphResponse* resp);

    /**
     * Is the state machine running in HA mode?
     *
     * @return  True if in ha mode, false if not.
     */
    virtual bool IsInHaMode() const { return false; }

    /**
     * List all the peers. Return empty if in non-HA mode.
     *
     * @return  A std::vector&lt;Peer&gt;
     */
    virtual std::vector<Peer> ListPeers() const { return {}; }

    // list current backup log files
    std::vector<std::string> ListBackupLogFiles();

    // take a new snapshot, overwriting existing one, and truncating backup logs
    std::string TakeSnapshot();

 protected:
    // take a snapshot and store it in path
    void TakeSnapshot(const std::string& snapshot_path, bool truncate_log);

    // Carry out necessary operations for the request
    virtual bool DoRequest(bool is_write, const LGraphRequest* req, LGraphResponse* resp,
                           google::protobuf::Closure* on_done);

    bool IsWriteRequest(const LGraphRequest* req);

    bool IsFromLegalHost(::google::protobuf::RpcController* controller) const;

    bool RespondRedirect(LGraphResponse* resp, const std::string& target,
                         const std::string& reason = "Not a leader.") const;

    bool RespondException(LGraphResponse* resp, const std::string& reason) const;

    bool RespondSuccess(LGraphResponse* resp) const;

    bool RespondDenied(LGraphResponse* resp, const std::string& what = "") const;

    bool RespondTimeout(LGraphResponse* resp, const std::string& what) const;

    bool RespondBadInput(LGraphResponse* resp, const std::string& error) const;

    lgraph::AccessControlledDB GetDB(const std::string& token, const std::string& graph);

    std::string GetCurrUser(const LGraphRequest* lgraph_req);

    std::string GetCurrUser(const LGraphRequest* lgraph_req, bool* is_admin);

    bool ApplyConfigRequest(const LGraphRequest* lgraph_req, LGraphResponse* resp);

    bool ApplyRestoreRequest(const LGraphRequest* lgraph_req, LGraphResponse* resp);

    bool ApplyGraphRequest(const LGraphRequest* lgraph_req, LGraphResponse* resp);

    bool ApplyAclRequest(const LGraphRequest* lgraph_req, LGraphResponse* resp);

    bool ApplyGraphApiRequest(const LGraphRequest* lgraph_req, LGraphResponse* resp);

    bool ApplyGraphQueryRequest(const LGraphRequest* lgraph_req, LGraphResponse* resp);

    bool ApplyPluginRequest(const LGraphRequest* lgraph_req, LGraphResponse* resp);

    bool ApplyImportRequest(const LGraphRequest* lgraph_req, LGraphResponse* resp);

    bool ApplySchemaRequest(const LGraphRequest* lgraph_req, LGraphResponse* resp);
};
}  // namespace lgraph
