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

#include "core/audit_logger.h"
#include "core/defs.h"
#include "core/killable_rw_lock.h"
#include "db/galaxy.h"
#include "db/token_manager.h"

lgraph::Galaxy::Galaxy(const std::string& dir, bool create_if_not_exist)
    : Galaxy(Config{dir, false, true, "fma.ai"}, create_if_not_exist, nullptr) {}

static inline std::string GetMetaStoreDir(const std::string& parent_dir) {
    return parent_dir + "/.meta";
}

lgraph::Galaxy::Galaxy(const lgraph::Galaxy::Config& config, bool create_if_not_exist,
                       std::shared_ptr<GlobalConfig> global_config)
    : config_(config), global_config_(global_config), token_manager_(config.jwt_secret) {
    if (!global_config_) {
        dummy_global_config_.reset(new GlobalConfig);
        global_config_ = dummy_global_config_;
    }
    global_config_->durable.store(config.durable);
    // check if dir exists
    auto& fs = fma_common::FileSystem::GetFileSystem(config.dir);
    if (!fs.IsDir(config.dir)) {
        if (!create_if_not_exist) {
            throw lgraph_api::DBNotExistError("Database directory " + config.dir +
                                              " does not exist!");
        } else if (!fs.Mkdir(config.dir)) {
            throw lgraph_api::IOError("Failed to create data directory " + config.dir);
        }
    }
    std::string meta_dir = GetMetaStoreDir(config.dir);
    if (!create_if_not_exist && !fs.IsDir(meta_dir)) {
        throw InputError("Directory " + config.dir + " is not a valid DB.");
    }
    // now load dbs
    ReloadFromDisk(create_if_not_exist);
}

lgraph::Galaxy::~Galaxy() {
    // make sure all graphs are closed, so that they are persisted if necessary
    graphs_->CloseAllGraphs();
}

static inline void CheckUserName(const std::string& user) {
    if (!lgraph::IsValidUserName(user)) throw lgraph::InputError("Invalid user name.");
}

static inline void CheckGraphName(const std::string& user) {
    if (!lgraph::IsValidGraphName(user)) throw lgraph::InputError("Invalid graph name.");
}

static inline void CheckPasswordString(const std::string& password) {
    if (!lgraph::IsValidPassword(password)) throw lgraph::InputError("Invalid password string.");
}

std::string lgraph::Galaxy::GetUserToken(const std::string& user,
                                         const std::string& password) const {
    CheckUserName(user);
    _HoldReadLock(acl_lock_);
    bool r = acl_->ValidateUser(user, password);
    if (!r) return "";
    std::string jwt = token_manager_.IssueFirstToken();
    acl_->BindTokenUser("", jwt, user);
    return jwt;
}

std::string lgraph::Galaxy::ParseAndValidateToken(const std::string& token) const {
    std::string user, password;
    _HoldReadLock(acl_lock_);
    if (!acl_->DecipherToken(token, user, password)) throw AuthError("Invalid token.");
    return user;
}

std::string lgraph::Galaxy::RefreshUserToken(const std::string& token,
                                        const std::string& user) const {
    std::string new_token = token_manager_.UpdateToken(token);
    if (new_token != "") {
        acl_->BindTokenUser(token, new_token, user);
    } else {
        throw InputError("token has time out.");
    }
    return new_token;
}

bool lgraph::Galaxy::UnBindTokenUser(const std::string& token) {
    return acl_->UnBindTokenUser(token);
}

bool lgraph::Galaxy::JudgeRefreshTime(const std::string& token) {
    return token_manager_.JudgeRefreshTime(token);
}

void lgraph::Galaxy::ModifyTokenTime(const std::string& token,
             const int refresh_time, const int expire_time) {
    token_manager_.ModifyRefreshTime(token, refresh_time);
    token_manager_.ModifyExpireTime(token, expire_time);
}

std::pair<int, int> lgraph::Galaxy::GetTokenTime(const std::string& token) {
    return token_manager_.GetTokenTime(token);
}

std::string lgraph::Galaxy::ParseTokenAndCheckIfIsAdmin(const std::string& token,
                                                        bool* is_admin) const {
    std::string user, password;
    _HoldReadLock(acl_lock_);
    if (!acl_->DecipherToken(token, user, password)) throw AuthError("Invalid token.");
    if (is_admin) *is_admin = acl_->IsAdmin(user);
    return user;
}

bool lgraph::Galaxy::CreateGraph(const std::string& curr_user, const std::string& graph,
                                 const lgraph::DBConfig& config) {
    CheckGraphName(graph);
    _HoldReadLock(acl_lock_);
    if (!acl_->IsAdmin(curr_user)) throw AuthError("Non-admin cannot create graphs.");
    AutoWriteLock l1(acl_lock_, GetMyThreadId());  // upgrade to write lock
    AutoWriteLock l2(graphs_lock_, GetMyThreadId());
    std::unique_ptr<AclManager> acl_new(new AclManager(*acl_));
    std::unique_ptr<GraphManager> gm_new(new GraphManager(*graphs_));
    auto txn = store_->CreateWriteTxn(false);
    bool r = gm_new->CreateGraph(txn, graph, config);
    if (!r) return r;
    acl_new->AddGraph(txn, curr_user, graph);
    txn.Commit();
    acl_ = std::move(acl_new);
    graphs_ = std::move(gm_new);
    return r;
}

bool lgraph::Galaxy::DeleteGraph(const std::string& curr_user, const std::string& graph) {
    CheckGraphName(graph);
    _HoldReadLock(acl_lock_);
    if (!acl_->IsAdmin(curr_user)) throw AuthError("Non-admin cannot create graphs.");
    AutoWriteLock l1(acl_lock_, GetMyThreadId());
    AutoWriteLock l2(graphs_lock_, GetMyThreadId());
    // remove graph from list and then wait till no ref so we can destroy the db
    std::unique_ptr<AclManager> acl_new(new AclManager(*acl_));
    std::unique_ptr<GraphManager> gm_new(new GraphManager(*graphs_));
    auto gref = graphs_->GetGraphRef(graph);
    auto txn = store_->CreateWriteTxn(false);
    acl_new->DelGraph(txn, curr_user, graph);
    auto db = gm_new->DelGraph(txn, graph);
    if (!db) return false;
    txn.Commit();
    acl_ = std::move(acl_new);
    graphs_ = std::move(gm_new);
    // now destroy db asynchronously
    // since the obj has been removed from graphs_, we are now the only manager ref to the obj.
    // so we can delete the db safely as soon as the ref count becomes zero
    db.Assign(nullptr, [](LightningGraph* db) {
        std::string dir = db->GetConfig().dir;
        db->Close();
        fma_common::FileSystem::GetFileSystem(dir).RemoveDir(dir);
        FMA_LOG() << "GraphDB " << dir << " deleted.";
    });
    return true;
}

bool lgraph::Galaxy::ModGraph(const std::string& curr_user, const std::string& graph_name,
                              const GraphManager::ModGraphActions& actions) {
    _HoldReadLock(acl_lock_);
    if (!acl_->IsAdmin(curr_user)) throw AuthError("Non-admin user cannot modify graph configs.");
    auto txn = store_->CreateWriteTxn(false);
    AutoWriteLock l2(graphs_lock_, GetMyThreadId());
    std::unique_ptr<GraphManager> gm_new(new GraphManager(*graphs_));
    bool r = gm_new->ModGraph(txn, graph_name, actions);
    if (!r) return r;
    txn.Commit();
    graphs_ = std::move(gm_new);
    return true;
}

std::map<std::string, lgraph::DBConfig> lgraph::Galaxy::ListGraphs(
    const std::string& curr_user) const {
    _HoldReadLock(acl_lock_);
    if (!acl_->IsAdmin(curr_user)) throw AuthError("Non-admin user cannot list graphs.");
    AutoReadLock l2(graphs_lock_, GetMyThreadId());
    return graphs_->ListGraphs();
}

template <typename FT>
bool lgraph::Galaxy::ModifyACL(const FT& func) {
    _HoldWriteLock(acl_lock_);
    std::unique_ptr<AclManager> acl_new(new AclManager(*acl_));
    auto txn = store_->CreateWriteTxn();
    bool r = func(acl_new.get(), txn);
    if (!r) return r;
    txn.Commit();
    acl_ = std::move(acl_new);
    return true;
}

bool lgraph::Galaxy::CreateUser(const std::string& curr_user, const std::string& name,
                                const std::string& password, const std::string& desc) {
    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->AddUser(txn, curr_user, name, password, desc);
    });
}

bool lgraph::Galaxy::DeleteUser(const std::string& curr_user, const std::string& name) {
    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->DelUser(txn, curr_user, name);
    });
}

bool lgraph::Galaxy::ModUser(const std::string& curr_user, const ModUserRequest& request) {
    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->ModUser(txn, curr_user, request);
    });
}

std::map<std::string, lgraph::AclManager::UserInfo> lgraph::Galaxy::ListUsers(
    const std::string& curr_user) const {
    _HoldReadLock(acl_lock_);
    auto txn = store_->CreateReadTxn();
    return acl_->ListUsers(txn, curr_user);
}

lgraph::AclManager::UserInfo lgraph::Galaxy::GetUserInfo(const std::string& curr_user,
                                                         const std::string& user) {
    _HoldReadLock(acl_lock_);
    auto txn = store_->CreateReadTxn();
    return acl_->GetUserInfo(txn, curr_user, user);
}

size_t lgraph::Galaxy::GetUserMemoryLimit(const std::string& curr_user, const std::string& user) {
    _HoldReadLock(acl_lock_);
    auto txn = store_->CreateReadTxn();
    return acl_->GetUserMemoryLimit(txn, curr_user, user);
}

std::unordered_map<std::string, lgraph::AccessLevel> lgraph::Galaxy::ListUserGraphs(
    const std::string& curr_user, const std::string& user) const {
    _HoldReadLock(acl_lock_);
    auto txn = store_->CreateReadTxn();
    return acl_->ListUserGraphs(txn, curr_user, user);
}

bool lgraph::Galaxy::CreateRole(const std::string& curr_user, const std::string& role,
                                const std::string& desc) {
    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->CreateRole(txn, curr_user, role, desc);
    });
}

bool lgraph::Galaxy::DeleteRole(const std::string& curr_user, const std::string& role) {
    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->DelRole(txn, curr_user, role);
    });
}

bool lgraph::Galaxy::ModRole(const std::string& curr_user, const ModRoleRequest& request) {
    _HoldWriteLock(reload_lock_);
    // if request modifies graph acess, should check graph existence first
    const google::protobuf::Map<std::string, ProtoAccessLevel>* graph_access = nullptr;
    if (request.action_case() == ModRoleRequest::kSetDiffGraphAccess)
        graph_access = &request.set_diff_graph_access().values();
    else if (request.action_case() == ModRoleRequest::kSetFullGraphAccess)
        graph_access = &request.set_full_graph_access().values();
    if (graph_access) {
        for (auto& kv : *graph_access) {
            const std::string& graph = kv.first;
            if (!this->graphs_->GraphExists(graph))
                throw InputError(FMA_FMT("Graph {} does not exist.", graph));
        }
    }

    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->ModRole(txn, curr_user, request);
    });
}

std::map<std::string, lgraph::AclManager::RoleInfo> lgraph::Galaxy::ListRoles(
    const std::string& curr_user) const {
    _HoldReadLock(acl_lock_);
    auto txn = store_->CreateReadTxn();
    return acl_->ListRoles(txn, curr_user);
}

lgraph::AclManager::RoleInfo lgraph::Galaxy::GetRoleInfo(const std::string& curr_user,
                                                         const std::string& role) {
    _HoldReadLock(acl_lock_);
    auto txn = store_->CreateReadTxn();
    return acl_->GetRoleInfo(txn, curr_user, role);
}

lgraph::AccessControlledDB lgraph::Galaxy::OpenGraph(const std::string& user,
                                                     const std::string& graph) const {
    _HoldReadLock(acl_lock_);
    AccessLevel ar = acl_->GetAccessRight(user, user, graph);
    if (ar == AccessLevel::NONE)
        throw AuthError("User does not have access to the graph specified.");
    AutoReadLock l2(graphs_lock_, GetMyThreadId());
    return AccessControlledDB(graphs_->GetGraphRef(graph), ar, user);
}

std::unordered_map<std::string, lgraph::AccessControlledDB> lgraph::Galaxy::OpenUserGraphs(
    const std::string& curr_user, const std::string& user) const {
    std::unordered_map<std::string, AccessLevel> acl = ListUserGraphs(curr_user, user);
    _HoldReadLock(acl_lock_);
    std::unordered_map<std::string, lgraph::AccessControlledDB> ret;
    AutoReadLock l2(graphs_lock_, GetMyThreadId());
    for (auto& kv : acl) {
        if (kv.second == AccessLevel::NONE || kv.first == _detail::META_GRAPH) continue;
        ret.insert(std::make_pair(kv.first,
              AccessControlledDB(graphs_->GetGraphRef(kv.first), kv.second, user)));
    }
    return ret;
}

lgraph::AccessLevel lgraph::Galaxy::GetAcl(const std::string& curr_user, const std::string& user,
                                           const std::string& graph) {
    CheckUserName(user);
    CheckGraphName(graph);
    _HoldReadLock(acl_lock_);
    AutoReadLock l2(graphs_lock_, GetMyThreadId());
    if (!graphs_->GraphExists(graph)) throw InputError("Graph does not exist.");
    return acl_->GetAccessRight(curr_user, user, graph);
}

std::unordered_set<std::string> lgraph::Galaxy::GetIpWhiteList(const std::string& curr_user) const {
    _HoldReadLock(acl_lock_);
    if (!acl_->IsAdmin(curr_user)) throw AuthError("Non-admin user cannot access IP whitelist.");
    AutoReadLock l2(ip_whitelist_rw_lock_, GetMyThreadId());
    return ip_whitelist_;
}

bool lgraph::Galaxy::IsIpInWhitelist(const std::string& ip) const {
    _HoldReadLock(ip_whitelist_rw_lock_);
    return ip_whitelist_.find(ip) != ip_whitelist_.end() || ip == "127.0.0.1" ||
           ip == global_config_->bind_host;
}

size_t lgraph::Galaxy::AddIpsToWhitelist(const std::string& curr_user,
                                         const std::vector<std::string>& ips) {
    _HoldReadLock(acl_lock_);
    if (!acl_->IsAdmin(curr_user)) throw AuthError("Non-admin user cannot access IP whitelist.");
    AutoWriteLock l2(ip_whitelist_rw_lock_, GetMyThreadId());
    std::unordered_set<std::string> new_ips;
    auto txn = store_->CreateWriteTxn();
    for (auto& ip : ips) {
        if (ip.size() >= _detail::MAX_HOST_ADDR_LEN)
            throw InputError("Host address length limit exceeded.");
        if (ip_whitelist_.find(ip) != ip_whitelist_.end() || new_ips.find(ip) != new_ips.end())
            continue;
        new_ips.insert(ip);
        char c = 'd';  // dummy
        bool r = ip_whitelist_table_.AddKV(txn, Value::ConstRef(ip), Value::ConstRef(c));
        FMA_DBG_ASSERT(r);
    }
    txn.Commit();
    ip_whitelist_.insert(new_ips.begin(), new_ips.end());
    return new_ips.size();
}

size_t lgraph::Galaxy::RemoveIpsFromWhitelist(const std::string& curr_user,
                                              const std::vector<std::string>& ips) {
    _HoldReadLock(acl_lock_);
    if (!acl_->IsAdmin(curr_user)) throw AuthError("Non-admin user cannot access IP whitelist.");
    AutoWriteLock l2(ip_whitelist_rw_lock_, GetMyThreadId());
    std::unordered_set<std::string> to_remove;
    auto txn = store_->CreateWriteTxn();
    for (auto& ip : ips) {
        if (ip_whitelist_.find(ip) == ip_whitelist_.end() || to_remove.find(ip) != to_remove.end())
            continue;
        to_remove.insert(ip);
        bool r = ip_whitelist_table_.DeleteKey(txn, Value::ConstRef(ip));
        FMA_DBG_ASSERT(r);
    }
    txn.Commit();
    for (auto& ip : to_remove) ip_whitelist_.erase(ip);
    return to_remove.size();
}

void lgraph::Galaxy::LoadIpWhitelist(KvTransaction& txn) {
    _HoldWriteLock(ip_whitelist_rw_lock_);
    ip_whitelist_.clear();
    for (auto it = ip_whitelist_table_.GetIterator(txn); it.IsValid(); it.Next()) {
        // key is ip, value is dummy byte
        ip_whitelist_.insert(it.GetKey().AsString());
    }
}

bool lgraph::Galaxy::LoadSnapshot(const std::string& dir) {
    _HoldWriteLock(reload_lock_);
    auto confs = graphs_->ListGraphs();
    auto metaDir = GetMetaStoreDir(config_.dir);
    store_.reset(nullptr);
    graphs_->CloseAllGraphs();
    graphs_.reset();
    auto& fs = fma_common::FileSystem::GetFileSystem(dir);
    for (auto& conf : confs) {
        fs.RemoveDir(conf.second.dir);
    }
    fs.RemoveDir(metaDir);
    fs.CopyToLocal(dir, config_.dir);
    ReloadFromDisk(false);
    return true;
}

static inline void TryMkDir(const std::string& dir, const fma_common::FileSystem& fs) {
    if (!fs.Mkdir(dir)) {
        FMA_WARN() << "Failed to create dir " << dir;
        throw std::runtime_error("Failed to create dir " + dir);
    }
}

std::vector<std::string> lgraph::Galaxy::SaveSnapshot(const std::string& dir) {
    // TODO: for now, we require TLSRWLock write lock to be held before saving snapshot // NOLINT
    // However, we should be able to leverage MVCC here and thus avoid locking the whole
    // galaxy.
    _HoldWriteLock(reload_lock_);
    // clear dir
    auto& fs = fma_common::FileSystem::GetFileSystem(dir);
    if (fs.IsDir(dir)) {
        FMA_DBG_STREAM(logger_) << "Snapshot dir " << dir << " already exists, overwriting...";
        if (!fs.RemoveDir(dir)) {
            FMA_WARN_STREAM(logger_) << "Failed to remove dir " << dir;
            throw std::runtime_error("Failed to remove dir " + dir);
        }
    }
    TryMkDir(dir, fs);
    // backup meta data
    std::string meta_dir = GetMetaStoreDir(dir);
    TryMkDir(meta_dir, fs);
    store_->Backup(meta_dir);
    std::vector<std::string> ret;
    ret.push_back(meta_dir + "/data.mdb");
    // backup graphs
    auto files = graphs_->Backup(dir);
    for (auto& f : files) ret.push_back(f);
    FMA_DBG_STREAM(logger_) << "Snapshot files: " << fma_common::ToString(files);
    return ret;
}

bool lgraph::Galaxy::IsAdmin(const std::string& user) const {
    _HoldReadLock(acl_lock_);
    return acl_->IsAdmin(user);
}

lgraph::KillableRWLock& lgraph::Galaxy::GetReloadLock() { return reload_lock_; }

void lgraph::Galaxy::ReloadFromDisk(bool create_if_not_exist) {
    FMA_DBG_STREAM(logger_) << "Loading DB state from disk";
    _HoldWriteLock(reload_lock_);
    // now we can do anything we want on the galaxy
    // states: meta_, token_manager_ and raft_log_index_
    // token_manager_ does not have on-disk state, so no need to reload
    // raft_log_index_ not implemented yet

    // now load acl and subgraphs
    if (graphs_) graphs_->CloseAllGraphs();
    graphs_.reset();
    acl_.reset();
    store_.reset();
    KvStore::SetLastOpIdOfAllStores(-1);
    store_.reset(new KvStore(GetMetaStoreDir(config_.dir), (size_t)1 << 30, config_.durable,
                             create_if_not_exist));
    auto txn = store_->CreateWriteTxn(false);
    db_info_table_ =
        store_->OpenTable(txn, _detail::META_TABLE, true, ComparatorDesc::DefaultComparator());
    CheckTuGraphVersion(txn);
    // load configs
    LoadConfigTable(txn);
    // load ip whitelist
    ip_whitelist_table_ = store_->OpenTable(txn, _detail::IP_WHITELIST_TABLE, true,
                                            ComparatorDesc::DefaultComparator());
    LoadIpWhitelist(txn);
    // now load contents
    GraphManager::Config gmc(*global_config_);
    gmc.load_plugins = config_.load_plugins;
    graphs_.reset(new GraphManager());
    graphs_->Init(store_.get(), txn, _detail::GRAPH_CONFIG_TABLE_NAME, config_.dir, gmc);
    acl_.reset(new AclManager());
    acl_->Init(store_.get(), txn, _detail::USER_TABLE_NAME, _detail::ROLE_TABLE_NAME);
    if (graphs_->GraphExists(_detail::DEFAULT_GRAPH_DB_NAME))
        acl_->AddGraph(txn, _detail::DEFAULT_ADMIN_NAME, _detail::DEFAULT_GRAPH_DB_NAME);
    txn.Commit();
}

int64_t lgraph::Galaxy::GetRaftLogIndex() const { return KvStore::GetLastOpIdOfAllStores(); }

void lgraph::Galaxy::SetRaftLogIndexBeforeWrite(int64_t id) { KvStore::SetLastOpIdOfAllStores(id); }

void lgraph::Galaxy::WarmUp(const std::string& user, const std::vector<std::string>& graphs) {
    for (auto& name : graphs) {
        OpenGraph(user, name).WarmUp();
    }
}

void lgraph::Galaxy::Backup(const std::string& dst, bool compact) {
    _HoldWriteLock(reload_lock_);
    auto& fs = fma_common::FileSystem::GetFileSystem(dst);
    if (!fs.IsDir(dst)) TryMkDir(dst, fs);
    // backup meta store
    std::string meta_dir = GetMetaStoreDir(dst);
    TryMkDir(meta_dir, fs);
    store_->Backup(meta_dir, compact);
    // backup graphs one by one
    for (auto& kv : ListGraphs(_detail::DEFAULT_ADMIN_NAME)) {
        std::string graph_dir = dst + "/" + fs.GetFileName(kv.second.dir);
        TryMkDir(graph_dir, fs);
        OpenGraph(_detail::DEFAULT_ADMIN_NAME, kv.first).Backup(graph_dir, compact);
    }
}

namespace lgraph {
namespace _detail {
enum OPT_ENUMS { ENABLE_AUDIT_LOG = 0, DURABLE = 1, OPTIMISTIC = 2, ENABLE_IP_CHECK = 3 };

static const std::unordered_map<std::string, OPT_ENUMS> config_name_to_enums = {
    {OPT_AUDIT_LOG_ENABLE, OPT_ENUMS::ENABLE_AUDIT_LOG},
    {OPT_DB_DURABLE, OPT_ENUMS::DURABLE},
    {OPT_TXN_OPTIMISTIC, OPT_ENUMS::OPTIMISTIC},
    {OPT_IP_CHECK_ENABLE, OPT_ENUMS::ENABLE_IP_CHECK}};

inline std::string OptionNameToKey(const std::string& name) { return std::string("@") + name; }
static const std::unordered_map<std::string, OPT_ENUMS> config_key_to_enums = {
    {OptionNameToKey(_detail::OPT_AUDIT_LOG_ENABLE), OPT_ENUMS::ENABLE_AUDIT_LOG},
    {OptionNameToKey(_detail::OPT_DB_DURABLE), OPT_ENUMS::DURABLE},
    {OptionNameToKey(_detail::OPT_TXN_OPTIMISTIC), OPT_ENUMS::OPTIMISTIC},
    {OptionNameToKey(_detail::OPT_IP_CHECK_ENABLE), OPT_ENUMS::ENABLE_IP_CHECK}};
}  // namespace _detail
}  // namespace lgraph

bool lgraph::Galaxy::UpdateConfig(const std::string& user,
                                  const std::map<std::string, FieldData>& updates) {
    {
        _HoldReadLock(acl_lock_);
        if (!acl_->IsAdmin(user))
            throw AuthError("Non-admin user is not allowed to update configs.");
    }
    bool need_reload_galaxy = false;
    auto txn = store_->CreateWriteTxn();
    for (auto& kv : updates) {
        auto it = _detail::config_name_to_enums.find(kv.first);
        if (it == _detail::config_name_to_enums.end()) {
            throw InputError("Option " + kv.first + " does not exist.");
        }
        std::string key = _detail::OptionNameToKey(it->first);
        switch (it->second) {
        case _detail::OPT_ENUMS::ENABLE_AUDIT_LOG:
            {
                if (!kv.second.IsBool())
                    throw InputError("Option " + kv.first + " must be BOOL type.");
                bool b = kv.second.AsBool();
                if (b != global_config_->enable_audit_log) {
                    db_info_table_.SetValue(txn, Value::ConstRef(key), Value::ConstRef(b));
                    need_reload_galaxy = true;
                }
                break;
            }
        case _detail::OPT_ENUMS::DURABLE:
            {
                if (!kv.second.IsBool())
                    throw InputError("Option " + kv.first + " must be BOOL type.");
                bool b = kv.second.AsBool();
                if (b != global_config_->durable) {
                    db_info_table_.SetValue(txn, Value::ConstRef(key), Value::ConstRef(b));
                    need_reload_galaxy = true;
                }
                break;
            }
        case _detail::OPT_ENUMS::OPTIMISTIC:
            {
                if (!kv.second.IsBool())
                    throw InputError("Option " + kv.first + " must be BOOL type.");
                bool b = kv.second.AsBool();
                if (b != global_config_->txn_optimistic) {
                    db_info_table_.SetValue(txn, Value::ConstRef(key), Value::ConstRef(b));
                    need_reload_galaxy = true;
                }
                break;
            }
        case _detail::OPT_ENUMS::ENABLE_IP_CHECK:
            {
                if (!kv.second.IsBool())
                    throw InputError("Option " + kv.first + " must be BOOL type.");
                bool b = kv.second.AsBool();
                if (b != global_config_->enable_ip_check) {
                    db_info_table_.SetValue(txn, Value::ConstRef(key), Value::ConstRef(b));
                    need_reload_galaxy = true;
                }
                break;
            }
        default:
            FMA_DBG_ASSERT(false);
        }
    }
    txn.Commit();
    return need_reload_galaxy;
}

void lgraph::Galaxy::LoadConfigTable(lgraph::KvTransaction& txn) {
    for (auto& kv : _detail::config_key_to_enums) {
        auto it = db_info_table_.GetIterator(txn, Value::ConstRef(kv.first));
        if (!it.IsValid()) continue;
        switch (kv.second) {
        case _detail::OPT_ENUMS::ENABLE_AUDIT_LOG:
            {
                // enable/disable audit log
                bool b = it.GetValue().AsType<bool>();
                AuditLogger::SetEnable(b);
                if (b != global_config_->enable_audit_log) {
                    FMA_INFO_STREAM(logger_)
                        << "Option \"enable_audit_log\" is overwritten by value in DB";
                    global_config_->enable_audit_log = b;
                }
                break;
            }
        case _detail::OPT_ENUMS::DURABLE:
            {
                bool b = it.GetValue().AsType<bool>();
                config_.durable = b;
                if (b != global_config_->durable) {
                    FMA_INFO_STREAM(logger_) << "Option \"durable\" is overwritten by value in DB";
                    global_config_->durable = b;
                }
                break;
            }
        case _detail::OPT_ENUMS::OPTIMISTIC:
            {
                bool b = it.GetValue().AsType<bool>();
                config_.optimistic_txn = b;
                if (b != global_config_->txn_optimistic) {
                    FMA_INFO_STREAM(logger_)
                        << "Option \"optimistic_txn\" is overwritten by value in DB";
                    global_config_->txn_optimistic = b;
                }
                break;
            }
        case _detail::OPT_ENUMS::ENABLE_IP_CHECK:
            {
                bool b = it.GetValue().AsType<bool>();
                if (b != global_config_->enable_ip_check) {
                    FMA_INFO_STREAM(logger_)
                        << "Option \"enable_ip_check\" is overwritten by value in DB";
                    global_config_->enable_ip_check = b;
                }
                break;
            }
        default:
            FMA_DBG_ASSERT(false);
        }
    }
}

std::tuple<int, int, int> lgraph::Galaxy::GetAndSetTuGraphVersionIfNecessary(KvTransaction& txn) {
    auto it = db_info_table_.GetIterator(txn, Value::ConstRef(_detail::VER_MAJOR_KEY));
    if (!it.IsValid()) {
        // no version info, set it
        db_info_table_.SetValue(txn, Value::ConstRef(_detail::VER_MAJOR_KEY),
                                Value::ConstRef(lgraph::_detail::VER_MAJOR));
        db_info_table_.SetValue(txn, Value::ConstRef(_detail::VER_MINOR_KEY),
                                Value::ConstRef(lgraph::_detail::VER_MINOR));
        db_info_table_.SetValue(txn, Value::ConstRef(_detail::VER_PATCH_KEY),
                                Value::ConstRef(lgraph::_detail::VER_PATCH));
        return std::make_tuple(_detail::VER_MAJOR, _detail::VER_MINOR, _detail::VER_PATCH);
    } else {
        int major = it.GetValue().AsType<int>();
        it.GotoKey(Value::ConstRef(_detail::VER_MINOR_KEY));
        FMA_DBG_ASSERT(it.IsValid());
        int minor = it.GetValue().AsType<int>();
        it.GotoKey(Value::ConstRef(_detail::VER_PATCH_KEY));
        FMA_DBG_ASSERT(it.IsValid());
        int patch = it.GetValue().AsType<int>();
        return std::make_tuple(major, minor, patch);
    }
}

void lgraph::Galaxy::CheckTuGraphVersion(KvTransaction& txn) {
    auto ver = GetAndSetTuGraphVersionIfNecessary(txn);
    int major = std::get<0>(ver);
    int minor = std::get<1>(ver);
    int patch = std::get<2>(ver);
    if (major != _detail::VER_MAJOR) {
        FMA_WARN_STREAM(logger_) << "Mismatching major version: DB is created with ver " << major
                                 << ", while current TuGraph is ver " << _detail::VER_MAJOR;
        throw std::runtime_error("Mismatch DB and software version.");
    }
    if (minor != _detail::VER_MINOR) {
        FMA_WARN_STREAM(logger_) << "DB is created with ver " << major << "." << minor
                                 << ", while current TuGraph is ver " << _detail::VER_MAJOR << "."
                                 << _detail::VER_MINOR
                                 << ". TuGraph may work just fine, but be ware of compatibility "
                                    "warnings in release notes.";
    }
}

void lgraph::Galaxy::BootstrapRaftLogIndex(int64_t log_id) {
    SetRaftLogIndexBeforeWrite(log_id);
    // need to write something so that raft log id can be written to db
    auto txn = store_->CreateWriteTxn(false);
    db_info_table_.SetValue(txn, Value::ConstRef("_dummy_"), Value::ConstRef(log_id), true);
    txn.Commit();
}

bool lgraph::Galaxy::ModRoleDisable(const std::string& role, bool disable) {
    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->ModRoleDisable(txn, role, disable);
    });
}

bool lgraph::Galaxy::ModRoleDesc(const std::string& role, const std::string& desc) {
    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->ModRoleDesc(txn, role, desc);
    });
}

bool lgraph::Galaxy::ModAllRoleAccessLevel(
    const std::string& role, const std::unordered_map<std::string, AccessLevel>& acs) {
    for (auto& kv : acs) {
        if (!this->graphs_->GraphExists(kv.first))
            throw InputError(FMA_FMT("Graph {} does not exist.", kv.first));
    }

    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->ModAllRoleAccessLevel(txn, role, acs);
    });
}

bool lgraph::Galaxy::ModRoleAccessLevel(const std::string& role, const AclManager::AclTable& acs) {
    for (auto& kv : acs) {
        if (!this->graphs_->GraphExists(kv.first))
            throw InputError(FMA_FMT("Graph {} does not exist.", kv.first));
    }
    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->ModRoleAccessLevel(txn, role, acs);
    });
}

bool lgraph::Galaxy::ModRoleFieldAccessLevel(const std::string& role,
                                             const AclManager::FieldAccessTable& acs) {
    for (auto& kv : acs) {
        if (!this->graphs_->GraphExists(kv.first))
            throw InputError(FMA_FMT("Graph {} does not exist.", kv.first));
    }
    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->ModRoleFieldAccessLevel(txn, role, acs);
    });
}

lgraph::AclManager::FieldAccess lgraph::Galaxy::GetRoleFieldAccessLevel(const std::string& user,
                                                                        const std::string& graph) {
    auto roles = GetUserInfo(user, user).roles;
    _HoldReadLock(acl_lock_);
    auto txn = store_->CreateWriteTxn();
    lgraph::AclManager::FieldAccess merged_access;
    for (auto& role : roles) {
        auto access = acl_->GetRoleFieldAccessLevel(txn, role, graph);
        for (auto& access_kv : access) {
            auto it = merged_access.find(access_kv.first);
            if (it == merged_access.end() ||
                (it != merged_access.end() && it->second > access_kv.second))
                merged_access[access_kv.first] = access_kv.second;
        }
    }
    return merged_access;
}

bool lgraph::Galaxy::ModUserDisable(const std::string& curr_user, const std::string& user,
                                    bool disable) {
    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->ModUserDisable(txn, curr_user, user, disable);
    });
}

bool lgraph::Galaxy::ChangeCurrentPassword(const std::string& user, const std::string& old_password,
                                           const std::string& new_password) {
    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->ChangeCurrentPassword(txn, user, old_password, new_password);
    });
}

bool lgraph::Galaxy::ChangeUserPassword(const std::string& current_user, const std::string& user,
                                        const std::string& password) {
    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->ChangeUserPassword(txn, current_user, user, password);
    });
}

bool lgraph::Galaxy::SetUserMemoryLimit(const std::string& current_user, const std::string& user,
                                        const size_t& memory_limit) {
    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->SetUserMemoryLimit(txn, current_user, user, memory_limit);
    });
}

bool lgraph::Galaxy::SetUserDescription(const std::string& current_user, const std::string& user,
                                        const std::string& desc) {
    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->SetUserDescription(txn, current_user, user, desc);
    });
}

bool lgraph::Galaxy::DeleteUserRoles(const std::string& current_user, const std::string& user,
                                     const std::vector<std::string>& roles) {
    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->DeleteUserRoles(txn, current_user, user, roles);
    });
}

bool lgraph::Galaxy::RebuildUserRoles(const std::string& current_user, const std::string& user,
                                      const std::vector<std::string>& roles) {
    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->RebuildUserRoles(txn, current_user, user, roles);
    });
}

bool lgraph::Galaxy::AddUserRoles(const std::string& current_user, const std::string& user,
                                  const std::vector<std::string>& roles) {
    _HoldWriteLock(reload_lock_);
    return ModifyACL([&](AclManager* new_acl, KvTransaction& txn) {
        return new_acl->AddUserRoles(txn, current_user, user, roles);
    });
}
