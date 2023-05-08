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

/**
 * Declares the Galaxy, which manages the graphs and users.
 */

#pragma once

#include <mutex>
#include <tuple>
#include <unordered_set>

#include "fma-common/thread_pool.h"
#include "fma-common/rw_lock.h"

#include "core/global_config.h"
#include "core/killable_rw_lock.h"
#include "core/lightning_graph.h"
#include "core/managed_object.h"

#include "db/acl.h"
#include "db/db.h"
#include "db/graph_manager.h"
#include "db/token_manager.h"
#include "protobuf/ha.pb.h"

namespace lgraph {
class HaStateMachine;
class Galaxy {
    friend class LightningGraph;  // for IncDbCount() and DecDbCount()
    friend class HaStateMachine;  // for BootstrapRaftLogIndex()
 public:
    struct Config {
        std::string dir = "./TuGraph";
        bool durable = false;
        bool optimistic_txn = false;
        std::string jwt_secret = "fma.ai";
        bool load_plugins = true;
    };

 private:
    fma_common::Logger& logger_ = fma_common::Logger::Get("Galaxy");
    mutable KillableRWLock reload_lock_;
    Config config_;
    std::shared_ptr<GlobalConfig> global_config_;
    // During testing, we might get a null global_config_. In this
    // case, we set global_config_ to dummy_global_config_, which is
    // created at construction. This avoids testing for null pointers.
    std::shared_ptr<GlobalConfig> dummy_global_config_;
    std::unique_ptr<KvStore> store_;
    std::unique_ptr<AclManager> acl_;
    mutable KillableRWLock acl_lock_;
    std::unique_ptr<GraphManager> graphs_;
    mutable KillableRWLock graphs_lock_;
    TokenManager token_manager_;
    KvTable db_info_table_;
    KvTable ip_whitelist_table_;
    std::unordered_set<std::string> ip_whitelist_;
    mutable KillableRWLock ip_whitelist_rw_lock_;

 public:
    explicit Galaxy(const std::string& dir,
                    bool create_if_not_exist = true);

    Galaxy(const Config& config, bool create_if_not_exist,
           std::shared_ptr<GlobalConfig> global_config);

    ~Galaxy();

    inline const Config& GetConfig() const { return config_; }
    inline const std::shared_ptr<GlobalConfig> GetGlobalConfigPtr() const { return global_config_; }

    // get token for a user
    std::string GetUserToken(const std::string& user, const std::string& password) const;

    // parse user token to get user name
    std::string ParseAndValidateToken(const std::string& token) const;

    // refresh token
    std::string RefreshUserToken(const std::string& token, const std::string& user) const;

    // unbind token and user
    bool UnBindTokenUser(const std::string& token);

    // judge token
    bool JudgeRefreshTime(const std::string& token);

    // modify tokenmanager time
    void ModifyTokenTime(const std::string& token,
                         const int refresh_time = 0, const int expire_time = 0);

    // get tokenmanager time
    std::pair<int, int> GetTokenTime(const std::string& token);

    // parse user token and check if user is admin
    // returns user name
    std::string ParseTokenAndCheckIfIsAdmin(const std::string& token, bool* is_admin) const;

    // create a graph
    bool CreateGraph(const std::string& curr_user, const std::string& graph_name,
                     const DBConfig& config);

    // delete a graph
    bool DeleteGraph(const std::string& curr_user, const std::string& graph_name);

    bool ModGraph(const std::string& curr_user, const std::string& graph_name,
                  const GraphManager::ModGraphActions& actions);

    std::map<std::string, DBConfig> ListGraphs(const std::string& curr_user) const;

    bool CreateUser(const std::string& curr_user, const std::string& name,
                    const std::string& password, const std::string& desc);

    bool DeleteUser(const std::string& curr_user, const std::string& name);

    bool ModUser(const std::string& curr_user, const ModUserRequest& request);

    std::map<std::string, AclManager::UserInfo> ListUsers(const std::string& curr_user) const;

    AclManager::UserInfo GetUserInfo(const std::string& curr_user, const std::string& user);

    size_t GetUserMemoryLimit(const std::string& curr_user, const std::string& user);

    std::unordered_map<std::string, AccessLevel> ListUserGraphs(const std::string& curr_user,
                                                                const std::string& user) const;

    bool CreateRole(const std::string& curr_user, const std::string& role, const std::string& desc);

    bool DeleteRole(const std::string& curr_user, const std::string& role);

    bool ModRole(const std::string& curr_user, const ModRoleRequest& request);

    std::map<std::string, AclManager::RoleInfo> ListRoles(const std::string& curr_user) const;

    AclManager::RoleInfo GetRoleInfo(const std::string& curr_user, const std::string& role);

    AccessLevel GetAcl(const std::string& curr_user, const std::string& user,
                       const std::string& graph);

    AccessControlledDB OpenGraph(const std::string& user, const std::string& graph) const;

    std::unordered_map<std::string, lgraph::AccessControlledDB> OpenUserGraphs(
        const std::string& curr_user, const std::string& user) const;

    //------------
    // IP whitelist
    std::unordered_set<std::string> GetIpWhiteList(const std::string& curr_user) const;

    bool IsIpInWhitelist(const std::string& ip) const;

    // add ip to whitelist
    // returns number of ips successfully added, existing ones are ignored
    size_t AddIpsToWhitelist(const std::string& curr_user, const std::vector<std::string>& ips);

    // remove ip from whitelist
    // returns number of ips successfully removed, non-existing ones are ignored
    size_t RemoveIpsFromWhitelist(const std::string& curr_user,
                                  const std::vector<std::string>& ips);

    // load snapshot from dir, write lock must be held
    bool LoadSnapshot(const std::string& dir);

    // save snapshot
    // for now, write lock must be held before this operation
    // returns the list of files which contains the snapshot
    std::vector<std::string> SaveSnapshot(const std::string& dir);

    bool IsAdmin(const std::string& user) const;

    // Galaxy is guarded by a TLSRWLock
    // Users of galaxy will grab a reader lock, while ReloadFromDisk requires
    // a write lock to be held.
    // Lock is acquired in HandleRequest and released automatically.
    KillableRWLock& GetReloadLock();

    // reload data from disk
    void ReloadFromDisk(bool create_if_not_exist = false);

    // get RaftLogIndex
    int64_t GetRaftLogIndex() const;

    void SetRaftLogIndexBeforeWrite(int64_t id);

    // warmup a list of graphs
    void WarmUp(const std::string& curr_user, const std::vector<std::string>& graphs);

    // backup the whole db into dst directory, possibly with compaction
    void Backup(const std::string& dst, bool compact);

    // update global configs hosted in MutableConfig
    // supposed to be called with Galaxy write lock held
    // returns whether we need to reload galaxy
    bool UpdateConfig(const std::string& curr_user,
                      const std::map<std::string, FieldData>& configs);

    bool ModRoleDisable(const std::string& role, bool disable);

    bool ModRoleDesc(const std::string& role, const std::string& desc);

    bool ModAllRoleAccessLevel(const std::string& role,
                               const AclManager::AclTable& acs);

    bool ModRoleAccessLevel(const std::string& role,
                            const AclManager::AclTable& acs);

    bool ModRoleFieldAccessLevel(const std::string& role,
                                 const AclManager::FieldAccessTable& acs);

    AclManager::FieldAccess GetRoleFieldAccessLevel(const std::string& user,
                                                    const std::string& graph);

    bool ModUserDisable(const std::string& curr_user, const std::string& user, bool disable);

    bool ChangeCurrentPassword(const std::string& user, const std::string& old_password,
                               const std::string& new_password);

    bool ChangeUserPassword(const std::string& current_user, const std::string& user,
                            const std::string& password);
    bool SetUserMemoryLimit(const std::string& current_user, const std::string& user,
                            const size_t& memory_limit);

    bool SetUserDescription(const std::string& current_user, const std::string& user,
                            const std::string& desc);

    bool DeleteUserRoles(const std::string& current_user, const std::string& user,
                         const std::vector<std::string>& roles);

    bool RebuildUserRoles(const std::string& current_user, const std::string& user,
                          const std::vector<std::string>& roles);

    bool AddUserRoles(const std::string& current_user, const std::string& user,
                      const std::vector<std::string>& roles);

 private:
    // load config from db
    // overwrites content of global_config_ if it is not null
    void LoadConfigTable(KvTransaction& txn);

    // load ip whitelist from db
    void LoadIpWhitelist(KvTransaction& txn);

    std::tuple<int, int, int> GetAndSetTuGraphVersionIfNecessary(KvTransaction& txn);

    void CheckTuGraphVersion(KvTransaction& txn);

    void BootstrapRaftLogIndex(int64_t log_id);  // used in HaStateMachine when bootstrapping

    template <typename FT>
    bool ModifyACL(const FT& func);
};
}  // namespace lgraph
