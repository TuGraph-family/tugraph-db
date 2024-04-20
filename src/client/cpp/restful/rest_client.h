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
#include "tools/lgraph_log.h"

// The 'U' macro can be used to create a string or character literal of the platform type, i.e.
// utility::char_t. If you are using a library causing conflicts with 'U' macro, it can be turned
// off by defining the macro '_TURN_OFF_PLATFORM_STRING' before including the C++ REST SDK header
// files, and e.g. use '_XPLATSTR' instead.
#define _TURN_OFF_PLATFORM_STRING
#include "cpprest/json.h"
#include "lgraph/lgraph.h"
#include "plugin/plugin_desc.h"
#include "core/data_type.h"

class RestClient {
    typedef lgraph_api::FieldData FieldData;
    typedef lgraph_api::FieldSpec FieldSpec;
    typedef lgraph_api::IndexSpec IndexSpec;
    typedef lgraph_api::EdgeUid EdgeUid;
    typedef lgraph::PluginDesc PluginDesc;
    typedef lgraph::EdgeConstraints EdgeConstraints;

    web::json::value DoPost(const std::string& url, const web::json::value& body,
                            bool expect_return = true);

    web::json::value DoGraphPost(const std::string& db, const std::string& url,
                                 const web::json::value& body, bool expect_return = true);

    web::json::value DoGet(const std::string& url, bool expect_return = true);

    web::json::value DoGraphGet(const std::string& db, const std::string& url,
                                bool expect_return = true);

    web::json::value DoDelete(const std::string& url, bool expect_return = false);

    web::json::value DoGraphDelete(const std::string& db, const std::string& url,
                                   bool expect_return = false);

    void DoPut(const std::string& url, const web::json::value& body);

    void DoGraphPut(const std::string& db, const std::string& url, const web::json::value& body);

    bool AddLabel(const std::string& db, const std::string& label,
                  const std::vector<FieldSpec>& fds, bool is_vertex, const std::string& primary,
                  const EdgeConstraints& edge_constraints);

 public:
    struct CPURate {
        int64_t self_cpu_rate;
        int64_t server_cpu_rate;
        std::string unit;
    };

    struct MemoryInfo {  // unit: KB
        int64_t total;
        int64_t available;
        int64_t self_memory;
        std::string unit;
    };

    struct DBSpace {
        size_t space;
        int64_t total;
        int64_t avail;
        std::string unit;
    };

    struct DBConfig {
        bool valid = false;
        bool durable, disable_auth, enable_ha, enable_rpc, use_ssl, enable_audit, enable_ip_check,
            optimistic_txn;
        uint16_t port, rpc_port;
        int thread_limit, verbose;
        std::string host;
    };

    RestClient(const std::string& url, const std::string& cert_path = "",
               size_t timeout_seconds = 600);

    ~RestClient();

    bool Login(const std::string& username, const std::string& password);

    std::string Refresh(const std::string& token);

    bool UpdateTokenTime(const std::string& token, int refresh_time, int expire_time);

    std::pair<int, int> GetTokenTime(const std::string& token);

    bool Logout(const std::string& token);

    void* GetClient() const;

    std::string SendImportData(const std::string& db, const std::string& desc,
                               const std::string& data, bool continue_on_error,
                               const std::string& delimiter);

    std::string SendSchema(const std::string& db, const std::string& desc);

    bool AddVertexLabel(const std::string& db, const std::string& label,
                        const std::vector<FieldSpec>& fds, const std::string& primary);

    bool AddEdgeLabel(const std::string& db, const std::string& label,
                      const std::vector<FieldSpec>& fds,
                      const EdgeConstraints& edge_constraints = {});

    bool AddIndex(const std::string& db, const std::string& label, const std::string& field,
                  int index_type);

    std::vector<std::string> ListVertexLabels(const std::string& db);

    std::vector<std::string> ListEdgeLabels(const std::string& db);

    std::vector<FieldSpec> GetSchema(const std::string& db, bool is_vertex,
                                     const std::string& label_name);

    std::vector<FieldSpec> GetVertexSchema(const std::string& db, const std::string& label);

    std::vector<FieldSpec> GetEdgeSchema(const std::string& db, const std::string& label);

    std::vector<IndexSpec> ListIndexes(const std::string& db);

    std::vector<IndexSpec> ListIndexesAboutLabel(const std::string& db, const std::string& label);

    void DeleteIndex(const std::string& db, const std::string& label, const std::string& field);

    std::vector<int64_t> GetVidsByIndex(const std::string& db, const std::string& label,
                                        const std::string& field, const std::string& value);

    // create
    int64_t AddVertex(const std::string& db, const std::string& label_name,
                      const std::vector<std::string>& field_names,
                      const std::vector<std::string>& field_value_strings);

    int64_t AddVertex(const std::string& db, const std::string& label_name,
                      const std::vector<std::string>& field_names,
                      const std::vector<FieldData>& field_values);

    std::vector<int64_t> AddVertexes(
        const std::string& db, const std::string& label_name,
        const std::vector<std::string>& field_names,
        const std::vector<std::vector<FieldData>>& field_values_vector);

    EdgeUid AddEdge(const std::string& db, int64_t src, int64_t dst, const std::string& label,
                    const std::vector<std::string>& field_names,
                    const std::vector<std::string>& field_value_strings);

    EdgeUid AddEdge(const std::string& db, int64_t src, int64_t dst, const std::string& label,
                    const std::vector<std::string>& field_names,
                    const std::vector<FieldData>& field_values);

    std::vector<EdgeUid> AddEdges(const std::string& db, std::string label,
                                  const std::vector<std::string>& field_names,
                                  const std::vector<std::pair<int64_t, int64_t>>& edges,
                                  const std::vector<std::vector<FieldData>>& field_values_vector);

    // read
    std::map<std::string, FieldData> GetVertexFields(const std::string& db, int64_t vid);

    std::map<std::string, FieldData> GetVertex(const std::string& db, int64_t vid,
                                               std::string& label);

    FieldData GetVertexField(const std::string& db, int64_t vid, const std::string& field_name);

    std::vector<EdgeUid> ListOutEdges(const std::string& db, int64_t src);

    std::vector<EdgeUid> ListInEdges(const std::string& db, int64_t dst);

    std::vector<std::vector<EdgeUid>> ListAllEdges(const std::string& db, int64_t vid);

    std::map<std::string, FieldData> GetEdge(const std::string& db, const EdgeUid& euid,
                                             std::string& label);

    std::map<std::string, FieldData> GetEdgeFields(const std::string& db, const EdgeUid& euid);

    FieldData GetEdgeField(const std::string& db, const EdgeUid& euid,
                           const std::string& field_name);

    // update
    void SetVertexProperty(const std::string& db, int64_t vid,
                           const std::vector<std::string>& field_names,
                           const std::vector<FieldData>& field_data);

    void SetEdgeProperty(const std::string& db, const EdgeUid& euid,
                         const std::vector<std::string>& field_names,
                         const std::vector<FieldData>& field_data);

    // delete
    void DeleteVertex(const std::string& db, int64_t vid, size_t* n_in, size_t* n_out);

    void DeleteEdge(const std::string& db, const EdgeUid& euid);

    web::json::value EvalCypher(const std::string& graph, const std::string& s);

    web::json::value EvalCypherWithParam(const std::string& graph, const std::string& s,
                                         const std::map<std::string, FieldData>& param);

    web::json::value GetSubGraph(const std::string& db, const std::vector<int64_t>& vec_vertex);
    web::json::value GetLabelNum(const std::string& graph);
    web::json::value ListUserLabel(const std::string& graph);
    web::json::value ListUserGraph(const std::string& user_name);
    web::json::value ListGraphs(const std::string& graph);
    web::json::value ListRoles(const std::string& role);

    std::map<std::string, lgraph_api::UserInfo> ListUsers();

    void AddUser(const std::string& user_name, bool is_admin, const std::string& password,
                 const std::string& desc = "");

    void SetPassword(const std::string& user, const std::string& curr_pass,
                     const std::string& new_pass);

    void SetUserDesc(const std::string& user, const std::string& new_desc);

    void SetUserRoles(const std::string& user, const std::vector<std::string>& roles);

    lgraph_api::UserInfo GetUserInfo(const std::string& user);

    void DeleteUser(const std::string& user_name);

    void DisableUser(const std::string& user);

    void EnableUser(const std::string& user);

    void CreateRole(const std::string& role, const std::string& desc);

    void DeleteRole(const std::string& role);

    void DisableRole(const std::string& role);

    void EnableRole(const std::string& role);

    void SetConfig(std::map<std::string, FieldData>& configs);

    void SetMisc();

    void SetRoleGraphAccess(const std::string& role,
                            const std::map<std::string, lgraph_api::AccessLevel>& graph_access);

    void SetRoleDesc(const std::string& role, const std::string& desc);

    bool GetServerInfo(RestClient::CPURate& cpu_rate, RestClient::MemoryInfo& memory_info,
                       RestClient::DBSpace& db_space, RestClient::DBConfig& db_config,
                       std::string& lgraph_version, std::string& node, std::string& relationship);

    bool LoadPlugin(const std::string& db, lgraph_api::PluginCodeType type,
                    const PluginDesc& plugin_info, const std::string& code);

    bool LoadPlugin(const std::string& db, lgraph_api::PluginCodeType type,
                    const PluginDesc& plugin_info, const std::vector<std::string>& codes,
                    const std::vector<std::string>& filenames);

    std::vector<PluginDesc> GetPlugin(const std::string& db, bool is_cpp);

    lgraph::PluginCode GetPluginDetail(const std::string& db, const std::string& name, bool is_cpp);

    std::string ExecutePlugin(const std::string& db, bool is_cpp, const std::string& plugin_name,
                              const std::string& data = "", double timeout = 0,
                              bool in_process = false);

    void DeletePlugin(const std::string& db, bool is_cpp, const std::string& plugin_name);

 private:
    std::map<std::string, std::string> _header;
    void* http_client_ = nullptr;
};
