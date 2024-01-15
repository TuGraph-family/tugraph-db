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

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <limits>

#include <boost/functional/hash.hpp>

#include "core/defs.h"
#include "core/kv_store.h"
#include "core/killable_rw_lock.h"
#include "lgraph/lgraph_types.h"
#include "protobuf/ha.pb.h"

namespace lgraph {
typedef lgraph_api::AccessLevel AccessLevel;
typedef lgraph_api::FieldAccessLevel FieldAccessLevel;

// Manages ACL. modification must be sequentail, only one writer is allowed
// at the same time.
// Role-based access control. There are users, roles and graphs. Each user
// has one or more roles. Each role has some access to multiple graphs.
class AclManager {
 public:
    struct CachedUserInfo;

    // information stored in user table
    struct UserInfo {
        std::unordered_set<std::string> roles;
        std::string password_md5;
        std::string desc;
        std::string auth_method;
        bool disabled = false;
        size_t memory_limit = _detail::DEFAULT_MEM_LIMIT;

        template <typename StreamT>
        size_t Serialize(StreamT& stream) const {
            return fma_common::BinaryWrite(stream, roles) +
                   fma_common::BinaryWrite(stream, password_md5) +
                   fma_common::BinaryWrite(stream, desc) +
                   fma_common::BinaryWrite(stream, auth_method) +
                   fma_common::BinaryWrite(stream, disabled) +
                   fma_common::BinaryWrite(stream, memory_limit);
        }

        template <typename StreamT>
        size_t Deserialize(StreamT& stream) {
            return fma_common::BinaryRead(stream, roles) +
                   fma_common::BinaryRead(stream, password_md5) +
                   fma_common::BinaryRead(stream, desc) +
                   fma_common::BinaryRead(stream, auth_method) +
                   fma_common::BinaryRead(stream, disabled) +
                   fma_common::BinaryRead(stream, memory_limit);
        }

        void UpdateWith(const CachedUserInfo& info) {
            password_md5 = info.password_md5;
            auth_method = info.auth_method;
            disabled = info.disabled;
            memory_limit = info.memory_limit;
        }
    };

    typedef std::unordered_map<std::string, AccessLevel> AclTable;

    struct LabelFieldSpec {
        bool is_vertex;
        std::string label;
        std::string field;

        template <typename StreamT>
        size_t Serialize(StreamT& stream) const {
            return fma_common::BinaryWrite(stream, is_vertex) +
                   fma_common::BinaryWrite(stream, label) + fma_common::BinaryWrite(stream, field);
        }

        template <typename StreamT>
        size_t Deserialize(StreamT& stream) {
            return fma_common::BinaryRead(stream, is_vertex) +
                   fma_common::BinaryRead(stream, label) + fma_common::BinaryRead(stream, field);
        }

        LabelFieldSpec() {}

        LabelFieldSpec(bool is_vertex_, const std::string& label_, const std::string& field_) {
            is_vertex = is_vertex_;
            label = label_;
            field = field_;
        }

        bool operator==(const LabelFieldSpec& other) const {
            if (is_vertex == other.is_vertex && label == other.label && field == other.field)
                return true;
            return false;
        }
    };

    class LabelFieldInfoHash {
     public:
        size_t operator()(const LabelFieldSpec& p) const noexcept {
            size_t res = 0;
            boost::hash_combine(res, p.is_vertex);
            boost::hash_combine(res, p.label);
            boost::hash_combine(res, p.field);
            return res;
        }
    };

    /**
     * FieldAccess
     * std::unordered_map<LabelFieldSpec label_field_infos
     *                    FieldAccessLevel field_access_level>
     */
    typedef std::unordered_map<LabelFieldSpec, FieldAccessLevel, LabelFieldInfoHash> FieldAccess;

    /**
     * FieldAccessTable
     * std::unordered_map<std::string graph_name,
     *                    FieldAccess field_access>
     */
    typedef std::unordered_map<std::string, FieldAccess> FieldAccessTable;
    // information stored in role table
    struct RoleInfo {
        bool disabled = false;
        bool is_primary = false;
        std::string desc;
        AclTable graph_access;
        FieldAccessTable field_access;

        template <typename StreamT>
        size_t Serialize(StreamT& stream) const {
            return fma_common::BinaryWrite(stream, disabled) +
                   fma_common::BinaryWrite(stream, is_primary) +
                   fma_common::BinaryWrite(stream, desc) +
                   fma_common::BinaryWrite(stream, graph_access) +
                   fma_common::BinaryWrite(stream, field_access);
        }

        template <typename StreamT>
        size_t Deserialize(StreamT& stream) {
            return fma_common::BinaryRead(stream, disabled) +
                   fma_common::BinaryRead(stream, is_primary) +
                   fma_common::BinaryRead(stream, desc) +
                   fma_common::BinaryRead(stream, graph_access) +
                   fma_common::BinaryRead(stream, field_access);
        }
    };

    // auth info
    // how to authenticate a user
    struct CachedUserInfo {
        AclTable acl;
        std::string password_md5;
        std::string auth_method;
        bool builtin_auth = true;
        bool disabled = false;
        bool is_admin = false;
        size_t memory_limit;

        // updates auth info
        void UpdateAuthInfo(const UserInfo& info) {
            password_md5 = info.password_md5;
            auth_method = info.auth_method;
            builtin_auth = (auth_method == _detail::BUILTIN_AUTH);
            disabled = info.disabled;
            memory_limit = info.memory_limit;
        }

        // updates acl and is_admin
        void UpdateAclInfo(KvTransaction& txn, KvTable& role_tbl, const UserInfo& uinfo);

        // updates acl and is_admin
        void UpdateAclInfo(const std::unordered_map<std::string, RoleInfo>& roles,
                           const UserInfo& uinfo);
    };

 private:
    // {user: use_built_in}
    std::unordered_map<std::string, CachedUserInfo> user_cache_;

    // {jwt: user_name}
    std::unordered_map<std::string, std::string> token_mapping_;

    // user -> UserInfo
    std::shared_ptr<KvTable> user_tbl_;
    // role -> RoleInfo
    std::shared_ptr<KvTable> role_tbl_;

 protected:
    UserInfo GetUserInfoFromKv(KvTransaction& txn, const std::string& user);
    void StoreUserInfoToKv(KvTransaction& txn, const std::string& user, const UserInfo& info);
    RoleInfo GetRoleInfoFromKv(KvTransaction& txn, const std::string& role);
    void StoreRoleInfoToKv(KvTransaction& txn, const std::string& role, const RoleInfo& info);
    std::unordered_map<std::string, RoleInfo> GetAllRolesFromKv(KvTransaction& txn);
    bool CreateRoleInternal(KvTransaction& txn, const std::string& role_name,
                            const std::string& desc, bool is_primary = false);
    bool DeleteRoleInternal(KvTransaction& txn, const std::string& role,
                            bool force_delete_primary = false);
    template <typename T>
    void CheckRolesExist(KvTransaction& txn, const T& roles);

 public:
    /**
     * \brief   Constructor
     *
     * \param [in,out]  store               If non-null, the store.
     * \param [in,out]  txn                 The transaction.
     * \param           auth_tbl_name       Name of the authentication table.
     * \param           user_graph_tbl_name Name of the user graph table.
     */
    void Init(KvStore* store, KvTransaction& txn, const std::string& user_tbl_name,
              const std::string& role_tbl_name);

    void Destroy();

    // validate user password
    bool ValidateUser(const std::string& user, const std::string& password) const;

    /**
     * \brief   get access right of user on graph. curr_user must either be the
     *          same as user, or be admin.
     *
     * \param           curr_user   The current user.
     * \param           user        The user to query.
     * \param           graph       The graph.
     *
     * \return  The access right.
     */
    AccessLevel GetAccessRight(const std::string& curr_user, const std::string& user,
                               const std::string& graph) const;

    /**
     * \brief   add a user. Admin-only.
     *
     * \param [in,out]  txn         The transaction.
     * \param           curr_user   The curr user.
     * \param           name        The name.
     * \param           password    The password.
     *
     * \return  True if it succeeds, false if user already exists.
     */
    bool AddUser(KvTransaction& txn, const std::string& curr_user, const std::string& name,
                 const std::string& password, const std::string& desc,
                 const std::string& auth_method = _detail::BUILTIN_AUTH);

    /**
     * \brief   modify user information. curr_user must either be admin, or the
     *          same as user.
     *
     * \param [in,out]  txn         The transaction.
     * \param           curr_user   The curr user.
     * \param           name        The name of the user to be added.
     * \param           actions     A list of ModActions
     *
     * \return  True if it succeeds, false if user does not exist.
     */
    bool ModUser(KvTransaction& txn, const std::string& curr_user, const ModUserRequest& request);

    /**
     * \brief   deletes user. Admin-only.
     *
     * \param [in,out]  txn         The transaction.
     * \param           curr_user   The curr user.
     * \param           user        The user to be deleted.
     *
     * \return  True if it succeeds, false if user does not exist.
     */
    bool DelUser(KvTransaction& txn, const std::string& curr_user, const std::string& user_to_del);

    /**
     * \brief   list all users. Admin-only.
     *
     * \param           curr_user   The curr user.
     *
     * \return  A map of {user : is_admin}.
     */
    std::map<std::string, UserInfo> ListUsers(KvTransaction& txn, const std::string& curr_user);

    UserInfo GetUserInfo(KvTransaction& txn, const std::string& curr_user, const std::string& user);

    size_t GetUserMemoryLimit(KvTransaction& txn, const std::string& curr_user,
                              const std::string& user);

    std::unordered_map<std::string, lgraph::AccessLevel> ListUserGraphs(
        KvTransaction& txn, const std::string& curr_user, const std::string& user);

    bool CreateRole(KvTransaction& txn, const std::string& curr_user, const std::string& role_name,
                    const std::string& desc);

    bool ModRole(KvTransaction& txn, const std::string& curr_user, const ModRoleRequest& request);

    bool DelRole(KvTransaction& txn, const std::string& curr_user, const std::string& role_to_del);

    std::map<std::string, RoleInfo> ListRoles(KvTransaction& txn, const std::string& curr_user);

    RoleInfo GetRoleInfo(KvTransaction& txn, const std::string& curr_user, const std::string& role);

    void AddGraph(KvTransaction& txn, const std::string& curr_user, const std::string& graph);

    // delete graph and remove access rights to the graph
    void DelGraph(KvTransaction& txn, const std::string& curr_user, const std::string& graph);

    // Is this user an admin account? That means, this user has full access to the meta graph
    bool IsAdmin(const std::string& user) const;

    // reload data from disk
    void ReloadFromDisk(KvStore* store, KvTransaction& txn, const std::string& user_tbl_name,
                        const std::string& role_tbl_name);

    bool ModRoleDisable(KvTransaction& txn, const std::string& role, bool disable);

    bool ModRoleDesc(KvTransaction& txn, const std::string& role, const std::string& desc);

    bool ModAllRoleAccessLevel(KvTransaction& txn, const std::string& role,
                               const std::unordered_map<std::string, AccessLevel>& acs);

    bool ModRoleAccessLevel(KvTransaction& txn, const std::string& role, const AclTable& acs);

    bool ModRoleFieldAccessLevel(KvTransaction& txn, const std::string& role,
                                 const FieldAccessTable& acs);

    FieldAccess GetRoleFieldAccessLevel(KvTransaction& txn, const std::string& user,
                                        const std::string& graph);

    bool ModUserDisable(KvTransaction& txn, const std::string& curr_user, const std::string& user,
                        bool disable);

    bool ChangeCurrentPassword(KvTransaction& txn, const std::string& user,
                               const std::string& old_password, const std::string& new_password,
                               bool force_reset_password = false);

    bool ChangeUserPassword(KvTransaction& txn, const std::string& current_user,
                            const std::string& user, const std::string& password);

    bool SetUserDescription(KvTransaction& txn, const std::string& current_user,
                            const std::string& user, const std::string& desc);

    bool DeleteUserRoles(KvTransaction& txn, const std::string& current_user,
                         const std::string& user, const std::vector<std::string>& roles);

    bool RebuildUserRoles(KvTransaction& txn, const std::string& current_user,
                          const std::string& user, const std::vector<std::string>& roles);

    bool AddUserRoles(KvTransaction& txn, const std::string& current_user, const std::string& user,
                      const std::vector<std::string>& roles);

    bool SetUserMemoryLimit(KvTransaction& txn, const std::string& current_user,
                            const std::string& user, const size_t& memmory_limit);

    void BindTokenUser(const std::string& old_token, const std::string& new_token,
                                                        const std::string& user);

    bool DecipherToken(const std::string& token, std::string& user, std::string& pwd);

    bool UnBindTokenUser(const std::string& token);

    bool UnBindUserAllToken(const std::string& user);

    int GetUserTokenNum(const std::string& user);
};
}  // namespace lgraph
