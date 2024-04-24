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

#include "fma-common/binary_buffer.h"
#include "fma-common/binary_read_write_helper.h"
#include "fma-common/encrypt.h"

#include "db/acl.h"

static inline std::string PasswordMd5(const std::string& password) {
    return fma_common::encrypt::MD5::Encrypt(password, lgraph::_detail::PASSWORD_MD5_SALT);
}

template <typename T>
inline T DeserializeFromValue(const lgraph::Value& v) {
    fma_common::BinaryBuffer buf(v.Data(), v.Size());
    T ret;
    size_t r = fma_common::BinaryRead(buf, ret);
    FMA_CHECK_EQ(r, v.Size()) << "Failed to load meta data from DB. Check your DB version.";
    return ret;
}

template <typename T>
inline lgraph::Value SerializeToValue(const T& data) {
    fma_common::BinaryBuffer buf;
    fma_common::BinaryWrite(buf, data);
    lgraph::Value v;
    v.Copy(buf.GetBuf(), buf.GetSize());
    return v;
}

static inline bool IsBuiltinRole(const std::string& role) {
    return role == lgraph::_detail::ADMIN_ROLE;
}

lgraph::AclManager::UserInfo lgraph::AclManager::GetUserInfoFromKv(KvTransaction& txn,
                                                                   const std::string& user) {
    if (user.empty()) THROW_CODE(InputError, "Invalid user name.");
    auto value = user_tbl_->GetValue(txn, Value::ConstRef(user));
    return DeserializeFromValue<UserInfo>(value);
}

void lgraph::AclManager::StoreUserInfoToKv(KvTransaction& txn, const std::string& user,
                                           const UserInfo& info) {
    user_tbl_->SetValue(txn, Value::ConstRef(user), SerializeToValue(info));
}

lgraph::AclManager::RoleInfo lgraph::AclManager::GetRoleInfoFromKv(KvTransaction& txn,
                                                                   const std::string& role) {
    if (role.empty()) THROW_CODE(InputError, "Illegal role name.");
    auto value = role_tbl_->GetValue(txn, Value::ConstRef(role));
    return DeserializeFromValue<RoleInfo>(value);
}

void lgraph::AclManager::StoreRoleInfoToKv(KvTransaction& txn, const std::string& role,
                                           const RoleInfo& info) {
    role_tbl_->SetValue(txn, Value::ConstRef(role), SerializeToValue(info));
}

static inline bool IsAdminRoleInfo(const lgraph::AclManager::RoleInfo& rinfo) {
    auto it = rinfo.graph_access.find(lgraph::_detail::META_GRAPH);
    return it != rinfo.graph_access.end() && it->second == lgraph::AccessLevel::FULL;
}

void lgraph::AclManager::CachedUserInfo::UpdateAclInfo(KvTransaction& txn, KvTable& role_tbl,
                                                       const UserInfo& uinfo) {
    acl.clear();
    is_admin = false;
    if (uinfo.disabled) return;
    for (auto& role : uinfo.roles) {
        auto value = role_tbl.GetValue(txn, Value::ConstRef(role));
        RoleInfo rinfo = DeserializeFromValue<RoleInfo>(value);
        if (rinfo.disabled) continue;
        if (IsAdminRoleInfo(rinfo)) is_admin = true;
        for (auto& kv : rinfo.graph_access) {
            auto& ac = acl[kv.first];
            ac = std::max(kv.second, ac);
        }
    }
}

void lgraph::AclManager::CachedUserInfo::UpdateAclInfo(
    const std::unordered_map<std::string, RoleInfo>& roles, const UserInfo& uinfo) {
    acl.clear();
    is_admin = false;
    if (uinfo.disabled) return;
    for (auto& role : uinfo.roles) {
        const RoleInfo& rinfo = roles.at(role);
        if (rinfo.disabled) continue;
        if (IsAdminRoleInfo(rinfo)) is_admin = true;
        for (auto& kv : rinfo.graph_access) {
            auto& ac = acl[kv.first];
            ac = std::max(kv.second, ac);
        }
    }
}

std::unordered_map<std::string, lgraph::AclManager::RoleInfo> lgraph::AclManager::GetAllRolesFromKv(
    KvTransaction& txn) {
    std::unordered_map<std::string, RoleInfo> ret;
    auto it = role_tbl_->GetIterator(txn);
    for (it->GotoFirstKey(); it->IsValid(); it->Next())
        ret.emplace(it->GetKey().AsString(), DeserializeFromValue<RoleInfo>(it->GetValue()));
    return ret;
}

bool lgraph::AclManager::CreateRoleInternal(KvTransaction& txn, const std::string& role_name,
                                            const std::string& desc, bool is_primary) {
    if (role_tbl_->HasKey(txn, Value::ConstRef(role_name))) return false;
    RoleInfo rinfo;
    rinfo.desc = desc;
    rinfo.disabled = false;
    rinfo.is_primary = is_primary;
    StoreRoleInfoToKv(txn, role_name, rinfo);
    return true;
}

bool lgraph::AclManager::DeleteRoleInternal(KvTransaction& txn, const std::string& role,
                                            bool deleting_user) {
    // delete key from role table
    auto it = role_tbl_->GetIterator(txn, Value::ConstRef(role));
    if (!it->IsValid()) return false;
    auto rinfo = DeserializeFromValue<RoleInfo>(it->GetValue());
    it->Close();
    if (rinfo.is_primary && !deleting_user)
        THROW_CODE(InputError, "Cannot delete a primary role that is still in use.");
    // now update user info and refresh the whole acl table
    bool role_still_in_use = false;
    std::unordered_map<std::string, RoleInfo> roles = GetAllRolesFromKv(txn);
    for (auto& kv : user_cache_) {
        const std::string& user = kv.first;
        UserInfo uinfo = GetUserInfoFromKv(txn, user);
        auto& uroles = uinfo.roles;
        auto it = uroles.find(role);
        if (it == uroles.end()) continue;
        role_still_in_use = true;
        // primary role that is still in use will not be deleted, so just break here
        if (rinfo.is_primary) break;
        // otherwise, we must be deleting a non-primary role, need to update users
        uroles.erase(role);
        StoreUserInfoToKv(txn, user, uinfo);
        // refresh user acl
        kv.second.UpdateAclInfo(roles, uinfo);
    }
    if (role_still_in_use && rinfo.is_primary) {
        // primary role still in use, mark as non-primary and return
        rinfo.is_primary = false;
        StoreRoleInfoToKv(txn, role, rinfo);
    } else {
        // primary role not in use any more, or not primary
        role_tbl_->DeleteKey(txn, Value::ConstRef(role));
    }
    return true;
}

void lgraph::AclManager::Init(KvStore* store, KvTransaction& txn, const std::string& auth_tbl_name,
                              const std::string& user_graph_tbl_name) {
    ReloadFromDisk(store, txn, auth_tbl_name, user_graph_tbl_name);
}

void lgraph::AclManager::Destroy() {}

bool lgraph::AclManager::ValidateUser(const std::string& user, const std::string& password) const {
    auto it = user_cache_.find(user);
    if (it == user_cache_.end()) return false;
    // if disabled, reject login
    if (it->second.disabled) return false;
    // authenticate according to auth_method
    if (it->second.builtin_auth)
        return it->second.password_md5 == PasswordMd5(password);
    else
        THROW_CODE(InternalError, "External authentication not supported yet.");
}

lgraph::AccessLevel lgraph::AclManager::GetAccessRight(const std::string& curr_user,
                                                       const std::string& user,
                                                       const std::string& graph) const {
    if (curr_user != user && !IsAdmin(curr_user))
        THROW_CODE(Unauthorized, "Non-admin user cannot get other users' access right.");
    auto it = user_cache_.find(user);
    if (it == user_cache_.end()) {
        THROW_CODE(InputError, "User does not exist.");
    }
    if (it->second.is_admin) return AccessLevel::FULL;
    auto& access = it->second.acl;
    auto git = access.find(graph);
    if (git == access.end()) return AccessLevel::NONE;
    return git->second;
}

bool lgraph::AclManager::AddUser(KvTransaction& txn, const std::string& curr_user,
                                 const std::string& name, const std::string& password,
                                 const std::string& desc, const std::string& auth_method) {
    std::string err_msg;
    lgraph::CheckValidUserName(name);
    lgraph::CheckValidPassword(password);
    lgraph::CheckValidDescLength(desc.size());
    lgraph::CheckValidUserNum(user_cache_.size());
    if (!IsAdmin(curr_user)) THROW_CODE(Unauthorized, "Non-admin user cannot add user.");
    if (user_cache_.find(name) != user_cache_.end()) return false;
    if (!CreateRoleInternal(txn, name, "primary role for user " + name, true))
        THROW_CODE(InputError, "A primary group named [{}] already exists.", name);
    UserInfo uinfo;
    uinfo.password_md5 = PasswordMd5(password);
    uinfo.desc = desc;
    uinfo.auth_method = auth_method;
    uinfo.disabled = false;
    uinfo.roles.emplace(name);
    // update cache
    auto& u = user_cache_[name];
    u.UpdateAuthInfo(uinfo);
    // update db
    StoreUserInfoToKv(txn, name, uinfo);
    return true;
}

template <typename T>
void lgraph::AclManager::CheckRolesExist(KvTransaction& txn, const T& roles) {
    for (auto& r : roles) {
        lgraph::CheckValidRoleName(r);
        if (!role_tbl_->HasKey(txn, Value::ConstRef(r)))
            THROW_CODE(InputError, "Role {} does not exist.", r);
    }
}

bool lgraph::AclManager::ModUser(KvTransaction& txn, const std::string& curr_user,
                                 const ModUserRequest& request) {
    if (request.user() != curr_user && !IsAdmin(curr_user))
        THROW_CODE(Unauthorized, "Non-admin user cannot modify other users.");
    lgraph::CheckValidUserName(request.user());
    // check if user exists
    const std::string& user = request.user();
    auto ait = user_cache_.find(user);
    if (ait == user_cache_.end()) return false;
    bool need_refresh_acl_table = false;
    UserInfo uinfo = GetUserInfoFromKv(txn, user);
    switch (request.action_case()) {
    case ModUserRequest::kDisable:
        {
            if (user == curr_user) THROW_CODE(InputError, "User cannot disable itself.");
            uinfo.disabled = true;
            need_refresh_acl_table = true;
            break;
        }
    case ModUserRequest::kEnable:
        {
            uinfo.disabled = false;
            need_refresh_acl_table = true;
            break;
        }
    case ModUserRequest::kSetPassword:
        {
            if (!ait->second.builtin_auth)
                THROW_CODE(InputError,
                           "Cannot set password to users using external authentication.");
            auto r = request.set_password();
            lgraph::CheckValidPassword(r.new_pass());
            if (user == curr_user) {
                // validate current password
                if (!r.has_old_pass() || PasswordMd5(r.old_pass()) != uinfo.password_md5) {
                    THROW_CODE(InputError, "Incorrect current password.");
                }
            }
            uinfo.password_md5 = PasswordMd5(r.new_pass());
            break;
        }
    case ModUserRequest::kSetDesc:
        {
            lgraph::CheckValidDescLength(request.set_desc().size());
            if (curr_user != user && !IsAdmin(curr_user))
                THROW_CODE(Unauthorized, "Non-admin user cannot modify other user's description.");
            uinfo.desc = request.set_desc();
            need_refresh_acl_table = false;
            break;
        }
    case ModUserRequest::kSetRoles:
        {
            if (!IsAdmin(curr_user))
                THROW_CODE(Unauthorized, "Non-admin uesr cannot modify roles.");
            auto& new_roles = request.set_roles().values();
            CheckRolesExist(txn, new_roles);
            uinfo.roles.clear();
            uinfo.roles.insert(new_roles.begin(), new_roles.end());
            uinfo.roles.insert(user);  // user always has a primary role
            if (curr_user == user && uinfo.roles.find(_detail::ADMIN_ROLE) == uinfo.roles.end())
                THROW_CODE(InputError, "User cannot remove itself from admin.");
            need_refresh_acl_table = true;
            break;
        }
    default:
        THROW_CODE(InternalError, "Unhandled ModUserRequest type: {}", request.action_case());
    }
    ait->second.UpdateAuthInfo(uinfo);
    if (need_refresh_acl_table) {
        bool was_admin = ait->second.is_admin;
        ait->second.UpdateAclInfo(txn, *role_tbl_, uinfo);
        if (curr_user == user && was_admin && !ait->second.is_admin) {
            THROW_CODE(InputError, "User cannot remove itself from admin group.");
        }
    }
    // now update kv
    StoreUserInfoToKv(txn, user, uinfo);
    return true;
}

bool lgraph::AclManager::DelUser(KvTransaction& txn, const std::string& curr_user,
                                 const std::string& user) {
    if (!IsAdmin(curr_user)) THROW_CODE(Unauthorized, "Non-admin user cannot delete users.");
    if (curr_user == user) THROW_CODE(InputError, "User cannot delete itself.");
    lgraph::CheckValidUserName(user);
    if (user == _detail::DEFAULT_ADMIN_NAME)
        THROW_CODE(InputError, "Builtin admin user cannot be deleted.");
    auto uit = user_cache_.find(user);
    if (uit == user_cache_.end()) return false;
    user_cache_.erase(uit);
    user_tbl_->DeleteKey(txn, Value::ConstRef(user));
    auto r = DeleteRoleInternal(txn, user, true);
    FMA_DBG_ASSERT(r);
    return true;
}

std::map<std::string, lgraph::AclManager::UserInfo> lgraph::AclManager::ListUsers(
    KvTransaction& txn, const std::string& curr_user) {
    if (!IsAdmin(curr_user)) {
        THROW_CODE(Unauthorized, "Non-admin user cannot list users.");
    }
    std::map<std::string, UserInfo> users;
    for (auto& kv : user_cache_) users.emplace(kv.first, GetUserInfoFromKv(txn, kv.first));
    return users;
}

lgraph::AclManager::UserInfo lgraph::AclManager::GetUserInfo(KvTransaction& txn,
                                                             const std::string& curr_user,
                                                             const std::string& user) {
    if (curr_user != user && !IsAdmin(curr_user)) {
        THROW_CODE(Unauthorized, "Non-admin user cannot get other users' info.");
    }
    lgraph::CheckValidUserName(user);
    if (user_cache_.find(user) == user_cache_.end())
        THROW_CODE(InputError, "User [{}] does not exist.", user);
    return GetUserInfoFromKv(txn, user);
}

size_t lgraph::AclManager::GetUserMemoryLimit(KvTransaction& txn, const std::string& curr_user,
                                              const std::string& user) {
    if (curr_user != user && !IsAdmin(curr_user)) {
        THROW_CODE(Unauthorized, "Non-admin user cannot get other users' info.");
    }
    lgraph::CheckValidUserName(user);
    if (user_cache_.find(user) == user_cache_.end())
        THROW_CODE(InputError, "User [{}] does not exist.", user);
    UserInfo uinfo = GetUserInfoFromKv(txn, user);
    return uinfo.memory_limit;
}

std::unordered_map<std::string, lgraph::AccessLevel> lgraph::AclManager::ListUserGraphs(
    KvTransaction& txn, const std::string& curr_user, const std::string& user) {
    if (curr_user != user && !IsAdmin(curr_user)) {
        THROW_CODE(Unauthorized, "Non-admin user cannot get other users' access right.");
    }
    auto it = user_cache_.find(user);
    if (it == user_cache_.end()) {
        THROW_CODE(InputError, "User does not exist.");
    }
    std::unordered_map<std::string, AccessLevel>& acl = it->second.acl;
    std::unordered_map<std::string, AccessLevel> res;
    for (auto iter = acl.begin(); iter != acl.end(); ++iter) {
        if (iter->first != _detail::META_GRAPH) {
            res.emplace(iter->first, iter->second);
        }
    }
    return res;
}

bool lgraph::AclManager::CreateRole(KvTransaction& txn, const std::string& curr_user,
                                    const std::string& role_name, const std::string& desc) {
    if (!IsAdmin(curr_user)) {
        THROW_CODE(Unauthorized, "Non-admin user cannot add roles.");
    }
    lgraph::CheckValidRoleName(role_name);
    lgraph::CheckValidDescLength(desc.size());
    return CreateRoleInternal(txn, role_name, desc, false);
}

bool lgraph::AclManager::ModRole(KvTransaction& txn, const std::string& curr_user,
                                 const ModRoleRequest& request) {
    if (!IsAdmin(curr_user)) THROW_CODE(Unauthorized, "Non-admin user cannot modify roles.");
    if (IsBuiltinRole(request.role())) THROW_CODE(InputError, "Builtin roles cannot be modified.");
    const std::string& role = request.role();
    lgraph::CheckValidRoleName(role);
    {
        auto it = role_tbl_->GetIterator(txn, Value::ConstRef(role));
        if (!it->IsValid()) return false;
    }
    RoleInfo rinfo = GetRoleInfoFromKv(txn, role);
    bool need_refresh_acl_table = false;
    switch (request.action_case()) {
    case ModRoleRequest::kDisable:
        {
            if (!rinfo.disabled) {
                rinfo.disabled = true;
                need_refresh_acl_table = true;
            }
            break;
        }
    case ModRoleRequest::kEnable:
        {
            if (rinfo.disabled) {
                rinfo.disabled = false;
                need_refresh_acl_table = true;
            }
            break;
        }
    case ModRoleRequest::kModDesc:
        {
            lgraph::CheckValidDescLength(request.mod_desc().size());
            rinfo.desc = request.mod_desc();
            need_refresh_acl_table = false;
            break;
        }
    case ModRoleRequest::kSetFullGraphAccess:
        {
            rinfo.graph_access.clear();
            for (auto& kv : request.set_full_graph_access().values()) {
                rinfo.graph_access.emplace(kv.first, static_cast<AccessLevel>(kv.second));
            }
            need_refresh_acl_table = true;
            break;
        }
    case ModRoleRequest::kSetDiffGraphAccess:
        {
            for (auto& kv : request.set_diff_graph_access().values()) {
                auto al = static_cast<AccessLevel>(kv.second);
                if (al == AccessLevel::NONE)
                    rinfo.graph_access.erase(kv.first);
                else
                    rinfo.graph_access[kv.first] = al;
            }
            need_refresh_acl_table = true;
            break;
        }
    default:
        THROW_CODE(InternalError, "Unhandled ModRoleRequest type: {}", request.action_case());
    }
    StoreRoleInfoToKv(txn, role, rinfo);
    if (need_refresh_acl_table) {
        // refresh user acl one by one
        std::unordered_map<std::string, RoleInfo> roles = GetAllRolesFromKv(txn);
        auto it = user_tbl_->GetIterator(txn);
        for (it->GotoFirstKey(); it->IsValid(); it->Next()) {
            UserInfo uinfo = DeserializeFromValue<UserInfo>(it->GetValue());
            auto& uroles = uinfo.roles;
            const std::string& user = it->GetKey().AsString();
            if (uroles.find(role) != uroles.end()) {
                user_cache_[user].UpdateAclInfo(roles, uinfo);
            }
        }
    }
    if (!IsAdmin(curr_user))
        THROW_CODE(InputError, "Modification will remove current user from administrators.");
    return true;
}

bool lgraph::AclManager::DelRole(KvTransaction& txn, const std::string& curr_user,
                                 const std::string& role) {
    if (!IsAdmin(curr_user)) {
        THROW_CODE(Unauthorized, "Non-admin user cannot delete roles.");
    }
    if (IsBuiltinRole(role)) THROW_CODE(InputError, "Builtin roles cannot be deleted.");
    lgraph::CheckValidRoleName(role);
    return DeleteRoleInternal(txn, role, false);
}

std::map<std::string, lgraph::AclManager::RoleInfo> lgraph::AclManager::ListRoles(
    KvTransaction& txn, const std::string& curr_user) {
    if (!IsAdmin(curr_user)) THROW_CODE(Unauthorized, "Non-admin users cannot list roles.");
    auto roles = GetAllRolesFromKv(txn);
    return std::map<std::string, RoleInfo>(roles.begin(), roles.end());
}

lgraph::AclManager::RoleInfo lgraph::AclManager::GetRoleInfo(KvTransaction& txn,
                                                             const std::string& curr_user,
                                                             const std::string& role) {
    if (!IsAdmin(curr_user)) {
        UserInfo uinfo = GetUserInfoFromKv(txn, curr_user);
        if (uinfo.roles.find(role) == uinfo.roles.end())
            THROW_CODE(Unauthorized, "User is not an admin and does not have the specified role.");
    }
    lgraph::CheckValidRoleName(role);
    auto v = role_tbl_->GetValue(txn, Value::ConstRef(role));
    if (v.Empty())
        THROW_CODE(InputError, "Role [{}] does not exist.", role);
    else
        return DeserializeFromValue<RoleInfo>(v);
}

void lgraph::AclManager::AddGraph(KvTransaction& txn, const std::string& curr_user,
                                  const std::string& graph) {
    // we don't need to check user access here, this is used internally
    lgraph::CheckValidGraphName(graph);
    // add graph to admin role
    RoleInfo rinfo = DeserializeFromValue<RoleInfo>(
        role_tbl_->GetValue(txn, Value::ConstRef(_detail::ADMIN_ROLE)));
    rinfo.graph_access.emplace_hint(rinfo.graph_access.end(), graph, AccessLevel::FULL);
    StoreRoleInfoToKv(txn, _detail::ADMIN_ROLE, rinfo);
    // update admin users cache
    for (auto& kv : user_cache_) {
        if (IsAdmin(kv.first)) {
            kv.second.acl.emplace(graph, AccessLevel::FULL);
        }
    }
}

void lgraph::AclManager::DelGraph(KvTransaction& txn, const std::string& curr_user,
                                  const std::string& graph) {
    if (!IsAdmin(curr_user)) {
        THROW_CODE(Unauthorized, "Non-admin user cannot delete graphs.");
    }
    if (graph == _detail::META_GRAPH) THROW_CODE(InputError, "Builtin graph cannot be deleted.");
    lgraph::CheckValidGraphName(graph);
    // remove graph from roles
    auto it = role_tbl_->GetIterator(txn);
    for (it->GotoFirstKey(); it->IsValid(); it->Next()) {
        RoleInfo rinfo = DeserializeFromValue<RoleInfo>(it->GetValue());
        auto& access = rinfo.graph_access;
        auto ait = access.find(graph);
        if (ait == access.end()) continue;
        // role affected, need to update
        access.erase(ait);
        it->SetValue(SerializeToValue(rinfo));
    }
    // remove graph from cache
    for (auto& kv : user_cache_) {
        kv.second.acl.erase(graph);
    }
}

bool lgraph::AclManager::IsAdmin(const std::string& user) const {
    auto it = user_cache_.find(user);
    if (it == user_cache_.end()) return false;
    return it->second.is_admin;
}

void lgraph::AclManager::ReloadFromDisk(KvStore* store, KvTransaction& txn,
                                        const std::string& user_tbl_name,
                                        const std::string& role_tbl_name) {
    user_cache_.clear();
    role_tbl_ = store->OpenTable(txn, role_tbl_name, true, ComparatorDesc::DefaultComparator());
    if (role_tbl_->GetKeyCount(txn) == 0) {
        // empty role table, create a default admin role
        RoleInfo rinfo;
        rinfo.desc = _detail::ADMIN_ROLE_DESC;
        rinfo.disabled = false;
        rinfo.graph_access.emplace(_detail::META_GRAPH, AccessLevel::FULL);
        StoreRoleInfoToKv(txn, _detail::ADMIN_ROLE, rinfo);
    }
    user_tbl_ = store->OpenTable(txn, user_tbl_name, true, ComparatorDesc::DefaultComparator());
    if (user_tbl_->GetKeyCount(txn) == 0) {
        // empty user table, create a default admin user
        UserInfo uinfo;
        uinfo.auth_method = _detail::BUILTIN_AUTH;
        uinfo.disabled = false;
        uinfo.password_md5 = PasswordMd5(_detail::DEFAULT_ADMIN_PASS);
        uinfo.roles.emplace(_detail::ADMIN_ROLE);
        StoreUserInfoToKv(txn, _detail::DEFAULT_ADMIN_NAME, uinfo);
    }
    // populate cache tables
    auto roles = GetAllRolesFromKv(txn);
    std::vector<std::pair<std::string, UserInfo>> users;
    auto it = user_tbl_->GetIterator(txn);
    for (it->GotoFirstKey(); it->IsValid(); it->Next()) {
        users.emplace_back(it->GetKey().AsString(), DeserializeFromValue<UserInfo>(it->GetValue()));
    }
    for (auto& kv : users) {
        const std::string& user = kv.first;
        UserInfo& uinfo = kv.second;
        auto& u = user_cache_[user];
        u.UpdateAuthInfo(uinfo);
        u.UpdateAclInfo(roles, uinfo);
    }
}

bool lgraph::AclManager::ModRoleDisable(KvTransaction& txn, const std::string& role, bool disable) {
    if (IsBuiltinRole(role)) THROW_CODE(InputError, "Builtin roles cannot be modified.");
    lgraph::CheckValidRoleName(role);
    {
        auto it = role_tbl_->GetIterator(txn, Value::ConstRef(role));
        if (!it->IsValid()) return false;
    }

    RoleInfo rinfo = GetRoleInfoFromKv(txn, role);
    if (rinfo.disabled != disable) {
        rinfo.disabled = disable;
        StoreRoleInfoToKv(txn, role, rinfo);
        // refresh user acl one by one
        std::unordered_map<std::string, RoleInfo> roles = GetAllRolesFromKv(txn);
        auto it = user_tbl_->GetIterator(txn);
        for (it->GotoFirstKey(); it->IsValid(); it->Next()) {
            auto uinfo = DeserializeFromValue<UserInfo>(it->GetValue());
            auto& uroles = uinfo.roles;
            const std::string& user = it->GetKey().AsString();
            if (uroles.find(role) != uroles.end()) {
                user_cache_[user].UpdateAclInfo(roles, uinfo);
            }
        }
    }
    return true;
}

bool lgraph::AclManager::ModRoleDesc(KvTransaction& txn, const std::string& role,
                                     const std::string& desc) {
    std::string err_msg;
    if (IsBuiltinRole(role)) THROW_CODE(InputError, "Builtin roles cannot be modified.");
    lgraph::CheckValidRoleName(role);
    lgraph::CheckValidDescLength(desc.size());
    {
        auto it = role_tbl_->GetIterator(txn, Value::ConstRef(role));
        if (!it->IsValid()) return false;
    }

    RoleInfo rinfo = GetRoleInfoFromKv(txn, role);
    rinfo.desc = desc;
    StoreRoleInfoToKv(txn, role, rinfo);
    std::unordered_map<std::string, RoleInfo> roles = GetAllRolesFromKv(txn);
    auto it = user_tbl_->GetIterator(txn);
    for (it->GotoFirstKey(); it->IsValid(); it->Next()) {
        auto uinfo = DeserializeFromValue<UserInfo>(it->GetValue());
        auto& uroles = uinfo.roles;
        const std::string& user = it->GetKey().AsString();
        if (uroles.find(role) != uroles.end()) {
            user_cache_[user].UpdateAclInfo(roles, uinfo);
        }
    }
    return true;
}

bool lgraph::AclManager::ModAllRoleAccessLevel(
    KvTransaction& txn, const std::string& role,
    const std::unordered_map<std::string, AccessLevel>& acs) {
    if (IsBuiltinRole(role)) THROW_CODE(InputError, "Builtin roles cannot be modified.");
    lgraph::CheckValidRoleName(role);
    {
        auto it = role_tbl_->GetIterator(txn, Value::ConstRef(role));
        if (!it->IsValid()) return false;
    }
    RoleInfo rinfo = GetRoleInfoFromKv(txn, role);
    rinfo.graph_access.clear();
    for (auto& kv : acs) {
        rinfo.graph_access.emplace(kv.first, static_cast<AccessLevel>(kv.second));
    }

    StoreRoleInfoToKv(txn, role, rinfo);
    std::unordered_map<std::string, RoleInfo> roles = GetAllRolesFromKv(txn);
    auto it = user_tbl_->GetIterator(txn);
    for (it->GotoFirstKey(); it->IsValid(); it->Next()) {
        auto uinfo = DeserializeFromValue<UserInfo>(it->GetValue());
        auto& uroles = uinfo.roles;
        const std::string& user = it->GetKey().AsString();
        if (uroles.find(role) != uroles.end()) {
            user_cache_[user].UpdateAclInfo(roles, uinfo);
        }
    }
    return true;
}

bool lgraph::AclManager::ModRoleAccessLevel(KvTransaction& txn, const std::string& role,
                                            const AclTable& acs) {
    if (IsBuiltinRole(role)) THROW_CODE(InputError, "Builtin roles cannot be modified.");
    lgraph::CheckValidRoleName(role);
    {
        auto it = role_tbl_->GetIterator(txn, Value::ConstRef(role));
        if (!it->IsValid()) return false;
    }
    RoleInfo rinfo = GetRoleInfoFromKv(txn, role);
    for (auto& kv : acs) {
        auto al = static_cast<AccessLevel>(kv.second);
        if (al == AccessLevel::NONE)
            rinfo.graph_access.erase(kv.first);
        else
            rinfo.graph_access[kv.first] = al;
    }
    StoreRoleInfoToKv(txn, role, rinfo);
    std::unordered_map<std::string, RoleInfo> roles = GetAllRolesFromKv(txn);
    auto it = user_tbl_->GetIterator(txn);
    for (it->GotoFirstKey(); it->IsValid(); it->Next()) {
        UserInfo uinfo = DeserializeFromValue<UserInfo>(it->GetValue());
        auto& uroles = uinfo.roles;
        const std::string& user = it->GetKey().AsString();
        if (uroles.find(role) != uroles.end()) {
            user_cache_[user].UpdateAclInfo(roles, uinfo);
        }
    }
    return true;
}

bool lgraph::AclManager::ModRoleFieldAccessLevel(KvTransaction& txn, const std::string& role,
                                                 const FieldAccessTable& acs) {
    if (IsBuiltinRole(role)) THROW_CODE(InputError, "Builtin roles cannot be modified.");
    lgraph::CheckValidRoleName(role);
    {
        auto it = role_tbl_->GetIterator(txn, Value::ConstRef(role));
        if (!it->IsValid()) return false;
    }
    RoleInfo rinfo = GetRoleInfoFromKv(txn, role);
    for (auto& kv : acs) {
        auto graph = kv.first;
        if (rinfo.field_access.find(graph) == rinfo.field_access.end())
            rinfo.field_access[graph] = FieldAccess();
        for (auto& field_acs : kv.second) {
            if (field_acs.second == FieldAccessLevel::WRITE)
                rinfo.field_access[graph].erase(field_acs.first);
            else
                rinfo.field_access[graph][field_acs.first] = field_acs.second;
            if (rinfo.field_access[graph].empty()) rinfo.field_access.erase(graph);
        }
    }
    StoreRoleInfoToKv(txn, role, rinfo);
    return true;
}

lgraph::AclManager::FieldAccess lgraph::AclManager::GetRoleFieldAccessLevel(
    KvTransaction& txn, const std::string& role, const std::string& graph) {
    RoleInfo rinfo = GetRoleInfoFromKv(txn, role);
    if (rinfo.field_access.find(graph) != rinfo.field_access.end())
        return rinfo.field_access[graph];
    else
        return {};
}

bool lgraph::AclManager::ModUserDisable(KvTransaction& txn, const std::string& curr_user,
                                        const std::string& user, bool disable) {
    if (curr_user != user && !IsAdmin(curr_user))
        THROW_CODE(Unauthorized, "Non-admin user cannot modify other user's description.");
    if (user == curr_user) THROW_CODE(InputError, "User cannot disable itself.");
    lgraph::CheckValidRoleName(user);
    // check if user exists
    auto ait = user_cache_.find(user);
    if (ait == user_cache_.end()) return false;
    UserInfo uinfo = GetUserInfoFromKv(txn, user);
    uinfo.disabled = disable;
    ait->second.UpdateAuthInfo(uinfo);
    bool was_admin = ait->second.is_admin;
    ait->second.UpdateAclInfo(txn, *role_tbl_, uinfo);
    if (curr_user == user && was_admin && !ait->second.is_admin) {
        THROW_CODE(InputError, "User cannot remove itself from admin group.");
    }
    // now update kv
    StoreUserInfoToKv(txn, user, uinfo);
    return true;
}

bool lgraph::AclManager::ChangeCurrentPassword(KvTransaction& txn, const std::string& user,
                                               const std::string& old_password,
                                               const std::string& new_password,
                                               bool force_reset_password) {
    // check if user exists
    auto ait = user_cache_.find(user);
    if (ait == user_cache_.end()) return false;
    UserInfo uinfo = GetUserInfoFromKv(txn, user);
    lgraph::CheckValidPassword(new_password);
    // validate current password
    if (!force_reset_password) {
        if (old_password.empty() || PasswordMd5(old_password) != uinfo.password_md5)
            THROW_CODE(InputError, "Incorrect current password.");
    }
    uinfo.password_md5 = PasswordMd5(new_password);
    ait->second.UpdateAuthInfo(uinfo);
    // now update kv
    StoreUserInfoToKv(txn, user, uinfo);
    UnBindUserAllToken(user);
    return true;
}

bool lgraph::AclManager::ChangeUserPassword(KvTransaction& txn, const std::string& current_user,
                                            const std::string& user, const std::string& password) {
    if (!IsAdmin(current_user))
        THROW_CODE(Unauthorized, "Non-admin user cannot modify other users.");
    lgraph::CheckValidUserName(user);
    lgraph::CheckValidPassword(password);
    auto ait = user_cache_.find(user);
    if (ait == user_cache_.end()) return false;
    UserInfo uinfo = GetUserInfoFromKv(txn, user);
    if (!ait->second.builtin_auth)
        THROW_CODE(InputError, "Cannot set password to users using external authentication.");
    uinfo.password_md5 = PasswordMd5(password);
    ait->second.UpdateAuthInfo(uinfo);
    // now update kv
    StoreUserInfoToKv(txn, user, uinfo);
    UnBindUserAllToken(user);
    return true;
}

bool lgraph::AclManager::SetUserDescription(KvTransaction& txn, const std::string& current_user,
                                            const std::string& user, const std::string& desc) {
    if (current_user != user && !IsAdmin(current_user))
        THROW_CODE(Unauthorized, "Non-admin user cannot modify other user's description.");
    lgraph::CheckValidUserName(user);
    // check if user exists
    auto ait = user_cache_.find(user);
    if (ait == user_cache_.end()) return false;
    UserInfo uinfo = GetUserInfoFromKv(txn, user);
    lgraph::CheckValidDescLength(desc.size());
    uinfo.desc = desc;
    ait->second.UpdateAuthInfo(uinfo);
    // now update kv
    StoreUserInfoToKv(txn, user, uinfo);
    return true;
}

bool lgraph::AclManager::DeleteUserRoles(KvTransaction& txn, const std::string& current_user,
                                         const std::string& user,
                                         const std::vector<std::string>& roles) {
    if (!IsAdmin(current_user)) THROW_CODE(Unauthorized, "Non-admin uesr cannot modify roles.");
    lgraph::CheckValidUserName(user);
    // check if user exists
    auto ait = user_cache_.find(user);
    if (ait == user_cache_.end()) return false;
    bool need_refresh_acl_table = false;
    UserInfo uinfo = GetUserInfoFromKv(txn, user);
    CheckRolesExist(txn, roles);
    for (auto& role : roles) {
        if (role == user) THROW_CODE(InputError, "Cannnot delete primary role from user.");
        if (uinfo.roles.erase(role)) need_refresh_acl_table = true;
    }
    ait->second.UpdateAuthInfo(uinfo);
    if (need_refresh_acl_table) {
        bool was_admin = ait->second.is_admin;
        ait->second.UpdateAclInfo(txn, *role_tbl_, uinfo);
        if (current_user == user && was_admin && !ait->second.is_admin) {
            THROW_CODE(InputError, "User cannot remove itself from admin group.");
        }
    }
    // now update kv
    StoreUserInfoToKv(txn, user, uinfo);
    return true;
}

bool lgraph::AclManager::RebuildUserRoles(KvTransaction& txn, const std::string& current_user,
                                          const std::string& user,
                                          const std::vector<std::string>& roles) {
    if (!IsAdmin(current_user)) THROW_CODE(Unauthorized, "Non-admin uesr cannot modify roles.");
    lgraph::CheckValidUserName(user);
    // check if user exists
    auto ait = user_cache_.find(user);
    if (ait == user_cache_.end()) return false;
    UserInfo uinfo = GetUserInfoFromKv(txn, user);
    CheckRolesExist(txn, roles);
    uinfo.roles.clear();
    uinfo.roles.insert(roles.begin(), roles.end());
    uinfo.roles.insert(user);  // user always has a primary role
    if (current_user == user && uinfo.roles.find(_detail::ADMIN_ROLE) == uinfo.roles.end())
        THROW_CODE(InputError, "User cannot remove itself from admin.");
    ait->second.UpdateAuthInfo(uinfo);
    bool was_admin = ait->second.is_admin;
    ait->second.UpdateAclInfo(txn, *role_tbl_, uinfo);
    if (current_user == user && was_admin && !ait->second.is_admin) {
        THROW_CODE(InputError, "User cannot remove itself from admin group.");
    }
    // now update kv
    StoreUserInfoToKv(txn, user, uinfo);
    return true;
}

bool lgraph::AclManager::AddUserRoles(KvTransaction& txn, const std::string& current_user,
                                      const std::string& user,
                                      const std::vector<std::string>& roles) {
    if (!IsAdmin(current_user)) THROW_CODE(Unauthorized, "Non-admin uesr cannot modify roles.");
    lgraph::CheckValidUserName(user);
    // check if user exists
    if (roles.empty()) return true;
    auto ait = user_cache_.find(user);
    if (ait == user_cache_.end()) return false;
    UserInfo uinfo = GetUserInfoFromKv(txn, user);
    CheckRolesExist(txn, roles);
    uinfo.roles.clear();
    uinfo.roles.insert(roles.begin(), roles.end());
    uinfo.roles.insert(user);  // user always has a primary role
    ait->second.UpdateAuthInfo(uinfo);
    bool was_admin = ait->second.is_admin;
    ait->second.UpdateAclInfo(txn, *role_tbl_, uinfo);
    if (current_user == user && was_admin && !ait->second.is_admin) {
        THROW_CODE(InputError, "User cannot remove itself from admin group.");
    }
    // now update kv
    StoreUserInfoToKv(txn, user, uinfo);
    return true;
}

bool lgraph::AclManager::SetUserMemoryLimit(KvTransaction& txn, const std::string& current_user,
                                            const std::string& user, const size_t& memory_limit) {
    if (!IsAdmin(current_user))
        THROW_CODE(Unauthorized, "Non-admin user cannot modify other users.");
    lgraph::CheckValidUserName(user);
    auto ait = user_cache_.find(user);
    if (ait == user_cache_.end()) return false;
    UserInfo uinfo = GetUserInfoFromKv(txn, user);
    uinfo.memory_limit = memory_limit;
    ait->second.UpdateAuthInfo(uinfo);

    StoreUserInfoToKv(txn, user, uinfo);
    return true;
}

void lgraph::AclManager::BindTokenUser(const std::string& old_token,
                const std::string& new_token, const std::string& user) {
    if (token_mapping_.find(old_token) != token_mapping_.end()) {
        token_mapping_.erase(old_token);
    }
    token_mapping_.emplace(new_token, user);
}

bool lgraph::AclManager::DecipherToken(const std::string& token,
                                std::string& user, std::string& pwd) {
    if (token_mapping_.find(token) != token_mapping_.end()) {
        user = token_mapping_[token];
        auto v = user_cache_.find(user);
        if (v == user_cache_.end()) return false;
        if (v->second.disabled) return false;
        if (v->second.builtin_auth) {
            pwd = v->second.password_md5;
        } else {
            THROW_CODE(InternalError, "External authentication not supported yet.");
        }
        return true;
    } else {
        return false;
    }
}

int lgraph::AclManager::GetUserTokenNum(const std::string& user) {
    int num = 0;
    for (auto& kv : token_mapping_) {
        if (kv.second == user) {
            num++;
        }
    }
    return num;
}

bool lgraph::AclManager::UnBindTokenUser(const std::string& token) {
    if (token_mapping_.find(token) != token_mapping_.end()) {
        token_mapping_.erase(token);
        return true;
    } else {
        return false;
    }
}

bool lgraph::AclManager::UnBindUserAllToken(const std::string& user) {
    std::vector<std::string> tokenToDel;
    for (const auto& pair : token_mapping_) {
        if (pair.second == user) {
            tokenToDel.push_back(pair.first);
        }
    }
    if (tokenToDel.empty()) {
        return false;
    }
    for (const auto& token : tokenToDel) {
        token_mapping_.erase(token);
    }
    return true;
}
