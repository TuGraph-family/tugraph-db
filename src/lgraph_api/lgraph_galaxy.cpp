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

#include "db/db.h"
#include "db/galaxy.h"
#include "db/acl.h"

#include "lgraph/lgraph_galaxy.h"

#define CHECK_DB_NOT_NULL()                                          \
    do {                                                             \
        if (!db_) throw lgraph_api::InvalidGalaxyError();            \
    } while (0)

#define CHECK_DB_AND_USER()                                                                    \
    do {                                                                                       \
        if (!db_) throw lgraph_api::InvalidGalaxyError();                                      \
        if (user_.empty()) throw lgraph_api::UnauthorizedError("User is not authorized yet."); \
    } while (0)

lgraph_api::Galaxy::Galaxy(lgraph::Galaxy* db) : db_(db) {}

lgraph_api::Galaxy::Galaxy(const std::string& dir, bool durable, bool create_if_not_exist) {
    lgraph::Galaxy::Config conf;
    conf.dir = dir;
    conf.durable = durable;
    conf.optimistic_txn = false;
    conf.load_plugins = false;
    db_ = new lgraph::Galaxy(conf, create_if_not_exist, nullptr);
}

lgraph_api::Galaxy::Galaxy(const std::string& dir, const std::string& user,
                           const std::string& password, bool durable,
                           bool create_if_not_exist)
    : Galaxy(dir, durable, create_if_not_exist) {
    SetCurrentUser(user, password);
}

lgraph_api::Galaxy::Galaxy(Galaxy&& rhs) {
    db_ = rhs.db_;
    rhs.db_ = nullptr;
    user_ = std::move(rhs.user_);
}

lgraph_api::Galaxy& lgraph_api::Galaxy::operator=(Galaxy&& rhs) {
    if (this == &rhs) return *this;
    Close();
    db_ = rhs.db_;
    rhs.db_ = nullptr;
    user_ = std::move(rhs.user_);
    return *this;
}

lgraph_api::Galaxy::~Galaxy() { Close(); }

void lgraph_api::Galaxy::SetCurrentUser(const std::string& user, const std::string& password) {
    CHECK_DB_NOT_NULL();
    std::string token = db_->GetUserToken(user, password);
    if (token.empty()) throw lgraph_api::UnauthorizedError("Bad user/password.");
    user_ = user;
}

void lgraph_api::Galaxy::SetUser(const std::string& user) {
    CHECK_DB_NOT_NULL();
    user_ = user;
}

bool lgraph_api::Galaxy::DeleteGraph(const std::string& graph_name) {
    CHECK_DB_AND_USER();
    return db_->DeleteGraph(user_, graph_name);
}

bool lgraph_api::Galaxy::ModGraph(const std::string& graph_name, bool mod_desc,
                                  const std::string& desc, bool mod_size, size_t new_size) {
    CHECK_DB_AND_USER();
    lgraph::GraphManager::ModGraphActions act;
    act.mod_desc = mod_desc;
    act.mod_size = mod_size;
    act.desc = desc;
    act.max_size = new_size;
    return db_->ModGraph(user_, graph_name, act);
}

std::map<std::string, std::pair<std::string, size_t>> lgraph_api::Galaxy::ListGraphs() const {
    CHECK_DB_AND_USER();
    std::map<std::string, std::pair<std::string, size_t>> ret;
    auto graphs = db_->ListGraphs(user_);
    for (auto& kv : graphs)
        ret.emplace(kv.first, std::make_pair(kv.second.desc, kv.second.db_size));
    return ret;
}

bool lgraph_api::Galaxy::CreateUser(const std::string& user, const std::string& password,
                                    const std::string& desc) {
    CHECK_DB_AND_USER();
    return db_->CreateUser(user_, user, password, desc);
}

bool lgraph_api::Galaxy::DeleteUser(const std::string& name) {
    CHECK_DB_AND_USER();
    return db_->DeleteUser(user_, name);
}

bool lgraph_api::Galaxy::SetPassword(const std::string& name, const std::string& old_password,
                                     const std::string& new_password) {
    CHECK_DB_AND_USER();
    lgraph::ModUserRequest req;
    req.set_user(name);
    auto* mod_pass = req.mutable_set_password();
    mod_pass->set_old_pass(old_password);
    mod_pass->set_new_pass(new_password);
    return db_->ModUser(user_, req);
}

bool lgraph_api::Galaxy::SetUserDesc(const std::string& user, const std::string& desc) {
    CHECK_DB_AND_USER();
    lgraph::ModUserRequest req;
    req.set_user(user);
    req.set_set_desc(desc);
    return db_->ModUser(user_, req);
}

bool lgraph_api::Galaxy::SetUserRoles(const std::string& name,
                                      const std::vector<std::string>& roles) {
    CHECK_DB_AND_USER();
    lgraph::ModUserRequest req;
    req.set_user(name);
    auto* set_roles = req.mutable_set_roles()->mutable_values();
    for (auto& r : roles) set_roles->Add()->assign(r);
    return db_->ModUser(user_, req);
}

bool lgraph_api::Galaxy::SetUserGraphAccess(const std::string& user, const std::string& graph,
                                            const AccessLevel& access) {
    CHECK_DB_AND_USER();
    lgraph::ModRoleRequest req;
    req.set_role(user);
    (*req.mutable_set_diff_graph_access()->mutable_values())[graph] =
        static_cast<lgraph::ProtoAccessLevel>(access);
    return db_->ModRole(user_, req);
}

bool lgraph_api::Galaxy::DisableUser(const std::string& user) {
    CHECK_DB_AND_USER();
    lgraph::ModUserRequest req;
    req.set_user(user);
    req.set_disable(true);
    return db_->ModUser(user_, req);
}

bool lgraph_api::Galaxy::EnableUser(const std::string& user) {
    CHECK_DB_AND_USER();
    lgraph::ModUserRequest req;
    req.set_user(user);
    req.set_enable(true);
    return db_->ModUser(user_, req);
}

static inline lgraph_api::UserInfo Convert(const lgraph::AclManager::UserInfo& info) {
    lgraph_api::UserInfo ret;
    ret.desc = info.desc;
    ret.disabled = info.disabled;
    ret.roles.insert(info.roles.begin(), info.roles.end());
    ret.memory_limit = info.memory_limit;
    return ret;
}

std::map<std::string, lgraph_api::UserInfo> lgraph_api::Galaxy::ListUsers() const {
    CHECK_DB_AND_USER();
    std::map<std::string, lgraph_api::UserInfo> ret;
    for (auto& kv : db_->ListUsers(user_)) {
        ret.emplace(kv.first, Convert(kv.second));
    }
    return ret;
}

lgraph_api::UserInfo lgraph_api::Galaxy::GetUserInfo(const std::string& user) const {
    return Convert(db_->GetUserInfo(user_, user));
}

bool lgraph_api::Galaxy::CreateRole(const std::string& role, const std::string& desc) {
    CHECK_DB_AND_USER();
    return db_->CreateRole(user_, role, desc);
}

bool lgraph_api::Galaxy::DeleteRole(const std::string& role) {
    CHECK_DB_AND_USER();
    return db_->DeleteRole(user_, role);
}

bool lgraph_api::Galaxy::DisableRole(const std::string& role) {
    CHECK_DB_AND_USER();
    lgraph::ModRoleRequest req;
    req.set_disable(true);
    req.set_role(role);
    return db_->ModRole(user_, req);
}

bool lgraph_api::Galaxy::EnableRole(const std::string& role) {
    CHECK_DB_AND_USER();
    lgraph::ModRoleRequest req;
    req.set_enable(true);
    req.set_role(role);
    return db_->ModRole(user_, req);
}

bool lgraph_api::Galaxy::SetRoleDesc(const std::string& role, const std::string& desc) {
    CHECK_DB_AND_USER();
    lgraph::ModRoleRequest req;
    req.set_role(role);
    req.mutable_mod_desc()->assign(desc);
    return db_->ModRole(user_, req);
}

bool lgraph_api::Galaxy::SetRoleAccessRights(
    const std::string& role, const std::map<std::string, AccessLevel>& graph_access) {
    CHECK_DB_AND_USER();
    lgraph::ModRoleRequest req;
    req.set_role(role);
    auto* access = req.mutable_set_full_graph_access()->mutable_values();
    for (auto& kv : graph_access)
        (*access)[kv.first] = static_cast<lgraph::ProtoAccessLevel>(kv.second);
    return db_->ModRole(user_, req);
}

bool lgraph_api::Galaxy::SetRoleAccessRightsIncremental(
    const std::string& role, const std::map<std::string, AccessLevel>& graph_access) {
    CHECK_DB_AND_USER();
    lgraph::ModRoleRequest req;
    req.set_role(role);
    auto* access = req.mutable_set_diff_graph_access()->mutable_values();
    for (auto& kv : graph_access)
        (*access)[kv.first] = static_cast<lgraph::ProtoAccessLevel>(kv.second);
    return db_->ModRole(user_, req);
}

static inline lgraph_api::RoleInfo Convert(const lgraph::AclManager::RoleInfo& info) {
    lgraph_api::RoleInfo ret;
    ret.desc = info.desc;
    ret.disabled = info.disabled;
    ret.graph_access.insert(info.graph_access.begin(), info.graph_access.end());
    return ret;
}

lgraph_api::RoleInfo lgraph_api::Galaxy::GetRoleInfo(const std::string& role) const {
    CHECK_DB_AND_USER();
    return Convert(db_->GetRoleInfo(user_, role));
}

std::map<std::string, lgraph_api::RoleInfo> lgraph_api::Galaxy::ListRoles() const {
    CHECK_DB_AND_USER();
    auto roles = db_->ListRoles(user_);
    std::map<std::string, lgraph_api::RoleInfo> ret;
    for (auto& kv : roles) {
        ret.emplace_hint(ret.end(), kv.first, Convert(kv.second));
    }
    return ret;
}

lgraph_api::AccessLevel lgraph_api::Galaxy::GetAccessLevel(const std::string& user,
                                                           const std::string& graph) const {
    CHECK_DB_AND_USER();
    return db_->GetAcl(user_, user, graph);
}

lgraph_api::GraphDB lgraph_api::Galaxy::OpenGraph(const std::string& graph, bool read_only) const {
    CHECK_DB_AND_USER();
    lgraph::AccessControlledDB* db = new lgraph::AccessControlledDB(db_->OpenGraph(user_, graph));
    return lgraph_api::GraphDB(db, read_only, true);
}

void lgraph_api::Galaxy::Close() {
    if (db_) {
        delete db_;
        db_ = nullptr;
    }
}

bool lgraph_api::Galaxy::CreateGraph(const std::string& graph_name, const std::string& description,
                                     size_t mmap_size) {
    CHECK_DB_AND_USER();
    lgraph::DBConfig config;
    config.db_size = mmap_size;
    config.desc = description;
    return db_->CreateGraph(user_, graph_name, config);
}
