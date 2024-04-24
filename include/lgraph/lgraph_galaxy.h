//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once

#include <map>

#include "lgraph/lgraph_db.h"
#include "lgraph/lgraph_types.h"

namespace lgraph {
class Galaxy;
}

namespace lgraph_api {

/**
 * @brief   A galaxy is the storage engine for one TuGraph instance. It manages a set of
 *          User/Role/GraphDBs.
 *          
 *          A galaxy can be opened in async mode, in which case *ALL* write transactions will be
 *          treated as async, whether they declare async or not. This can come in handy if we are
 *          performing a lot of writing, but can cause data loss for online processing.
 */
class Galaxy {
    std::string user_;
    lgraph::Galaxy* db_;

    explicit Galaxy(lgraph::Galaxy* db);
    Galaxy(const Galaxy&) {}
    Galaxy& operator=(const Galaxy&) { return *this; }

 public:
    /**
     * @brief   Constructor.
     *
     * @exception   DBNotExist Thrown if DB does not exist and create_if_not_exist is false.
     * @exception   IOError    Thrown if DB does not exist, but we failed to create the DB
     *                         due to IO error.
     * @exception   InputError Thrown if there are other input errors. e.g., dir is actually
     *                         a plain file, or DB is corruptted.
     *
     * @param   dir                 The TuGraph dir.
     * @param   durable             (Optional) True to open in durable mode. If set to false, ALL
     *                              write transactions are async, whether they declare async or
     *                              not.
     * @param   create_if_not_exist (Optional) If true, the TuGraph DB will be created if dir
     *                              does not exist; otherwise, an exception is thrown.
     */
    explicit Galaxy(const std::string& dir, bool durable = false, bool create_if_not_exist = true);

    /**
     * @brief   Constructor. Open the Galaxy and try to login with specified user and password.
     *
     * @exception   DBNotExist     Thrown if DB does not exist and create_if_not_exist is
     *                             false.
     * @exception   IOError        Thrown if DB does not exist, but we failed to create the
     *                             DB due to IO error.
     * @exception   InputError     Thrown if there are other input errors. e.g., dir is
     *                             actually a plain file, or DB is corruptted.
     * @exception   Unauthorized   Thrown if user/password is not correct.
     *
     * @param   dir                 The dir.
     * @param   user                The user.
     * @param   password            The password.
     * @param   durable             True to open the Galaxy in durable mode.
     * @param   create_if_not_exist True to create if DB does not exist.
     */
    Galaxy(const std::string& dir, const std::string& user, const std::string& password,
        bool durable, bool create_if_not_exist);

    Galaxy(Galaxy&&);

    Galaxy& operator=(Galaxy&&);

    ~Galaxy();

    /**
     * @brief   Validate and set current user
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user/password is incorrect.
     *
     * @param   user        The user.
     * @param   password    The password.
     */
    void SetCurrentUser(const std::string& user, const std::string& password);

    /**
     * @brief   Set current user
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if token is incorrect.
     *
     * @param   user   The current user.
     */
    void SetUser(const std::string& user);

    /**
     * @brief   Validate token and set current user
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission to create graph.
     * @exception   InputError     Other input errors such as invalid graph name, size, etc.
     *
     * @param   graph_name      Name of the graph to create.
     *          description     (Optional) Description of the graph.
     *          max_size        (Optional) Maximum size of the graph.
     *          
     * @returns     True if it succeeds, false if graph already exists.
     */
    bool CreateGraph(const std::string& graph_name, const std::string& description = "",
                     size_t max_size = (size_t)1 << 40);

    /**
     * @brief   Delete a graph
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission to delete graph.
     *
     * @param   graph_name  Name of the graph.
     *
     * @returns True if it succeeds, false if the graph does not exist.
     */
    bool DeleteGraph(const std::string& graph_name);

    /**
     * @brief   Modify graph info
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission to modify graph.
     *
     * @param   graph_name      Name of the graph.
     * @param   mod_desc        True to modifier description.
     * @param   desc            The new description.
     * @param   mod_size        True to modifier size.
     * @param   new_max_size    New maximum size.
     *
     * @returns True if it succeeds, false if it fails.
     */
    bool ModGraph(const std::string& graph_name, bool mod_desc, const std::string& desc,
                  bool mod_size, size_t new_max_size);

    /**
     * @brief   List graphs
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission to list graphs.
     *
     * @returns A dictionary of {graph_name: (description, max_size)}
     */
    std::map<std::string, std::pair<std::string, size_t>> ListGraphs() const;

    /**
     * @brief   Creates a user
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     * @exception   InputError          Thrown if other input errors, such as illegal user name,
     *                                  password, etc.
     *
     * @param   user        The user.
     * @param   password    The password.
     * @param   desc        (Optional) The description.
     *
     * @returns True if it succeeds, false if user already exists.
     */
    bool CreateUser(const std::string& user, const std::string& password,
                    const std::string& desc = "");

    /**
     * @brief   Deletes the user.
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     *
     * @param   user    The user.
     *
     * @returns True if it succeeds, false if user does not exist.
     */
    bool DeleteUser(const std::string& user);

    /**
     * @brief   Set the password of the specified user.
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission, or
     *                             curr_user==user, but old_password is incorrect.
     * @exception   InputError     Thrown if new_password is illegal.
     *
     * @param   user            The user to modify.
     * @param   old_password    The old password, required if curr_user==user.
     * @param   new_password    The new password.
     *
     * @returns True if it succeeds, false if user does not exist.
     */
    bool SetPassword(const std::string& user, const std::string& old_password,
                     const std::string& new_password);

    /**
     * @brief   Sets user description.
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     * @exception   InputError     Thrown if desc is illegal.
     *
     * @param   user    The user.
     * @param   desc    The new description.
     *
     * @returns True if it succeeds, false if user does not exist.
     */
    bool SetUserDesc(const std::string& user, const std::string& desc);

    /**
     * @brief   Set the roles of the specified user. If you need to add or delete a role, you
     *          will need to use GetUserInfo to get the roles first.
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     * @exception   InputError     Thrown if any role does not exist.
     *
     * @param   user    The user.
     * @param   roles   A list of roles.
     *
     * @returns True if it succeeds, false if user does not exist.
     */
    bool SetUserRoles(const std::string& user, const std::vector<std::string>& roles);

    /**
     * @brief   Sets user access rights on a graph.
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     * @exception   InputError     Thrown if graph does not exist.
     *
     * @param   user    The user.
     * @param   graph   The graph.
     * @param   access  The access level.
     *
     * @returns True if it succeeds, false if it user does not exist.
     */
    bool SetUserGraphAccess(const std::string& user, const std::string& graph,
                            const AccessLevel& access);

    /**
     * @brief   Disable a user. A disabled user is not able to login or perform any operation. A
     *          user cannot disable itself.
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     * @exception   InputError     Thrown if user name is illegal.
     *
     * @param   user    The user to disable.
     *
     * @returns True if it succeeds, false if user does not exist.
     */
    bool DisableUser(const std::string& user);

    /**
     * @brief   Enables the user.
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     * @exception   InputError     Thrown if user name is illegal.
     *
     * @param   user    The user.
     *
     * @returns True if it succeeds, false if user does not exist.
     */
    bool EnableUser(const std::string& user);

    /**
     * @brief   List all users
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     *
     * @returns A dictionary of {user_name:user_info}
     */
    std::map<std::string, UserInfo> ListUsers() const;

    /**
     * @brief   Gets user information
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     *
     * @param   user    The user.
     *
     * @returns The user information.
     */
    UserInfo GetUserInfo(const std::string& user) const;

    /**
     * @brief   Create a role. A role has different access levels to different graphs. Every user
     *          must be assigned some role to get access to graphs.
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     * @exception   InputError     Thrown if role name or desc is illegal.
     *
     * @param   role    The role.
     * @param   desc    The description.
     *
     * @returns True if it succeeds, false if role already exists.
     */
    bool CreateRole(const std::string& role, const std::string& desc);

    /**
     * @brief   Deletes the role described by role
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     * @exception   InputError     Thrown if role name is illegal.
     *
     * @param   role    The role.
     *
     * @returns True if it succeeds, false if role does not exist.
     */
    bool DeleteRole(const std::string& role);

    /**
     * @brief   Disable a role. A disabled role still has the data, but is not effective.
     *          i.e., users will not have access rights to graphs that are obtained by having
     *          this role.
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     * @exception   InputError     Thrown if role name is illegal.
     *
     * @param   role    The role.
     *
     * @returns True if it succeeds, false if the role does not exist.
     */
    bool DisableRole(const std::string& role);

    /**
     * @brief   Enables the role.
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     * @exception   InputError     Thrown if role name is illegal.
     *
     * @param   role    The role.
     *
     * @returns True if it succeeds, false if role does not exist.
     */
    bool EnableRole(const std::string& role);

    /**
     * @brief   Set the description of the specified role
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     * @exception   InputError     Thrown if role name or desc is illegal.
     *
     * @param   role    The role.
     * @param   desc    The description.
     *
     * @returns True if it succeeds, false if role does not exist.
     */
    bool SetRoleDesc(const std::string& role, const std::string& desc);

    /**
     * @brief   Set access of the role to graphs. If you need to add or remove access to part of
     *          the graphs, you need to get full graph_access map by using GetRoleInfo first.
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     * @exception   InputError     Thrown if role name or any of the graph name is illegal.
     *
     * @param   role            The role.
     * @param   graph_access    The graph access.
     *
     * @returns True if it succeeds, false if role does not exist.
     */
    bool SetRoleAccessRights(const std::string& role,
                             const std::map<std::string, AccessLevel>& graph_access);

    /**
     * @brief   Incrementally modify the access right of the specified role. For example, for a
     *          role that has access right {graph1:READ, graph2:WRITE}, calling this function
     *          with graph_access={graph2:READ, graph3:FULL}
     *          will set the access right of this role to {graph1:READ, graph2:READ, graph3:FULL}
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     * @exception   InputError     Thrown if role name or any of the graph name is illegal.
     *
     * @param   role            The role.
     * @param   graph_access    The graph access.
     *
     * @returns True if it succeeds, false if role does not exist.
     */
    bool SetRoleAccessRightsIncremental(const std::string& role,
                                        const std::map<std::string, AccessLevel>& graph_access);

    /**
     * @brief   Gets role information
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     * @exception   InputError     Thrown if role name is illegal.
     *
     * @param   role    The role.
     *
     * @returns The role information.
     */
    RoleInfo GetRoleInfo(const std::string& role) const;

    /**
     * @brief   List all the roles
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     *
     * @returns A dictionary of {role_name:RoleInfo}
     */
    std::map<std::string, RoleInfo> ListRoles() const;

    /**
     * @brief   Get the access level that the specified user have to the graph
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     * @exception   InputError     Thrown if user name or graph name is illegal.
     *
     * @param   user    The user.
     * @param   graph   The graph.
     *
     * @returns The access level.
     */
    AccessLevel GetAccessLevel(const std::string& user, const std::string& graph) const;

    /**
     * @brief   Opens a graph.
     *
     * @exception   InvalidGalaxy  Thrown if current galaxy is invalid.
     * @exception   Unauthorized   Thrown if user does not have permission.
     * @exception   InputError     Thrown if graph name is illegal.
     *
     * @param   graph       The graph.
     * @param   read_only   (Optional) True to open in read-only mode. A read-only GraphDB cannot
     *                      be written to.
     *
     * @returns A GraphDB.
     */
    GraphDB OpenGraph(const std::string& graph, bool read_only = false) const;

    /** @brief   Closes this Galaxy, turning it into an invalid state. */
    void Close();
};
}  // namespace lgraph_api
