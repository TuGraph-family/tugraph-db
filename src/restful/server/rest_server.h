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
#include <unordered_map>

#include "restful/server/stdafx.h"
#include "fma-common/rw_lock.h"
#include "core/lightning_graph.h"
#include "core/global_config.h"
#include "db/galaxy.h"
#include "lgraph/lgraph.h"
#include "server/state_machine.h"

#include "restful/server/json_convert.h"

#ifndef _WIN32
#include "cypher/execution_plan/scheduler.h"
#endif

namespace lgraph {
class RestServer {
    fma_common::Logger& logger_;
    enum RestPathCases {
        INVALID = 0,
        WEB = 1,
        INFO = 2,
        NODE = 3,
        RELATIONSHIP = 4,
        LABEL = 5,
        INDEX = 6,
        USER = 7,
        CPP_PLUGIN = 8,
        PYTHON_PLUGIN = 9,
        TASK = 10,
        LOGIN = 11,
        CYPHER = 12,
        MISC = 13,
        IMPORT = 14,
        DB = 15,
        GRAPH = 16,
        ROLE = 17,
        CONFIG = 18,
        SCHEMA = 19,
        REFRESH = 20,
        LOGOUT = 21,
        EXPORT = 22,
    };

    static std::unordered_map<utility::string_t, RestPathCases> GetPathToCaseDict() {
        return std::unordered_map<utility::string_t, RestPathCases>(
            {{_TU(""), RestPathCases::INVALID},
             {RestStrings::WEB, RestPathCases::WEB},
             {RestStrings::INFO, RestPathCases::INFO},
             {RestStrings::NODE, RestPathCases::NODE},
             {RestStrings::REL, RestPathCases::RELATIONSHIP},
             {RestStrings::LABEL, RestPathCases::LABEL},
             {RestStrings::INDEX, RestPathCases::INDEX},
             {RestStrings::USER, RestPathCases::USER},
             {RestStrings::CPP, RestPathCases::CPP_PLUGIN},
             {RestStrings::PYTHON, RestPathCases::PYTHON_PLUGIN},
             {RestStrings::TASKS, RestPathCases::TASK},
             {RestStrings::LOGIN, RestPathCases::LOGIN},
             {RestStrings::CYPHER, RestPathCases::CYPHER},
             {RestStrings::MISC, RestPathCases::MISC},
             {RestStrings::IMPORT, RestPathCases::IMPORT},
             {RestStrings::EXPORT, RestPathCases::EXPORT},
             {RestStrings::SCHEMA, RestPathCases::SCHEMA},
             {RestStrings::DB, RestPathCases::DB},
             {RestStrings::GRAPH, RestPathCases::GRAPH},
             {RestStrings::ROLE, RestPathCases::ROLE},
             {RestStrings::CONFIG, RestPathCases::CONFIG},
             {RestStrings::REFRESH, RestPathCases::REFRESH},
             {RestStrings::LOGOUT, RestPathCases::LOGOUT}});
    }

 public:
    struct Config {
        std::string host = "0.0.0.0";
        uint16_t port = 7070;
        bool use_ssl = false;
        std::string server_key;
        std::string server_cert;
        std::string resource_dir = "./resource";
        bool disable_auth = false;

        Config() {}
        explicit Config(const GlobalConfig& c) {
            host = c.bind_host;
            port = c.http_port;
            use_ssl = c.enable_ssl;
            server_key = c.server_key_file;
            server_cert = c.server_cert_file;
            resource_dir = c.http_web_dir;
            disable_auth = c.http_disable_auth;
        }
    };

    RestServer(StateMachine* state_machine, const Config& config,
               const std::shared_ptr<GlobalConfig>
                   service_config = std::shared_ptr<GlobalConfig>());
    ~RestServer();

    void Start();
    void Stop();

 private:
    void handle_get(web::http::http_request request);
    void handle_put(web::http::http_request request);
    void handle_post(web::http::http_request request);
    void handle_delete(web::http::http_request request);
    void handle_options(web::http::http_request request);
    void do_handle_put(web::http::http_request request, const web::json::value& body);
    void do_handle_post(web::http::http_request request, const web::json::value& body);

    void init_server();

    std::string GetUser(const web::http::http_request& request, std::string* token) const;

    RestPathCases GetRestPathCase(const utility::string_t& first_path) const;

    // check client address if necessary
    // throws Unauthorized error if ip check is enabled and ip is not in whitelist
    // requires that a read lock is help on galaxy_
    bool IsClientAddressAllowed(const web::http::http_request& request);

    void HandleGetWeb(const std::string& user, const web::http::http_request& request,
                      const utility::string_t& relative_path,
                      const std::vector<utility::string_t>& paths) const;
    void HandleGetInfo(const std::string& user, const web::http::http_request& request,
                       const utility::string_t& relative_path,
                       const std::vector<utility::string_t>& paths) const;
    void HandleGetUser(const std::string& user, const web::http::http_request& request,
                       const utility::string_t& relative_path,
                       const std::vector<utility::string_t>& paths) const;
    void HandleGetGraph(const std::string& user, const web::http::http_request& request,
                        const utility::string_t& relative_path,
                        const std::vector<utility::string_t>& paths) const;
    void HandleGetRole(const std::string& user, const web::http::http_request& request,
                       const utility::string_t& relative_path,
                       const std::vector<utility::string_t>& paths) const;
    void HandleGetTask(const std::string& user, const web::http::http_request& request,
                       const utility::string_t& relative_path,
                       const std::vector<utility::string_t>& paths) const;
    void HandleGetNode(const std::string& user, AccessControlledDB& db,
                       const web::http::http_request& request,
                       const utility::string_t& relative_path,
                       const std::vector<utility::string_t>& paths) const;
    void HandleGetRelationship(const std::string& user, AccessControlledDB& db,
                               const web::http::http_request& request,
                               const utility::string_t& relative_path,
                               const std::vector<utility::string_t>& paths) const;
    void HandleGetLabel(const std::string& user, AccessControlledDB& db,
                        const web::http::http_request& request,
                        const utility::string_t& relative_path,
                        const std::vector<utility::string_t>& paths) const;
    void HandleGetIndex(const std::string& user, AccessControlledDB& db,
                        const web::http::http_request& request,
                        const utility::string_t& relative_path,
                        const std::vector<utility::string_t>& paths) const;
    void HandleGetPlugin(const std::string& user, const std::string& token,
                         AccessControlledDB& db,
                         const web::http::http_request& request,
                         const utility::string_t& relative_path,
                         const std::vector<utility::string_t>& paths) const;

    void HandlePostLogin(const web::http::http_request& request,
                         const utility::string_t& relative_path,
                         const std::vector<utility::string_t>& paths,
                         const web::json::value& body) const;
    void HandlePostRefresh(const std::string& user, const std::string& token,
                        const web::http::http_request& request,
                        const utility::string_t& relative_path,
                        const std::vector<utility::string_t>& paths,
                        const web::json::value& body) const;
    void HandlePostLogout(const std::string& user, const std::string& token,
                        const web::http::http_request& request,
                        const utility::string_t& relative_path,
                        const std::vector<utility::string_t>& paths,
                        const web::json::value& body) const;
    void HandlePostMisc(const std::string& user, const std::string& token,
                        const web::http::http_request& request,
                        const utility::string_t& relative_path,
                        const std::vector<utility::string_t>& paths,
                        const web::json::value& body) const;
    void HandlePostCypher(const std::string& user, const std::string& token,
                          const web::http::http_request& request,
                          const utility::string_t& relative_path,
                          const std::vector<utility::string_t>& paths,
                          const web::json::value& body) const;
    void HandlePostUser(const std::string& user, const std::string& token,
                        const web::http::http_request& request,
                        const utility::string_t& relative_path,
                        const std::vector<utility::string_t>& paths,
                        const web::json::value& body) const;
    void HandlePostGraph(const std::string& user, const std::string& token,
                         const web::http::http_request& request,
                         const utility::string_t& relative_path,
                         const std::vector<utility::string_t>& paths,
                         const web::json::value& body) const;
    void HandlePostRole(const std::string& user, const std::string& token,
                        const web::http::http_request& request,
                        const utility::string_t& relative_path,
                        const std::vector<utility::string_t>& paths,
                        const web::json::value& body) const;
    void HandlePostNode(const std::string& user, const std::string& token,
                        const web::http::http_request& request,
                        const utility::string_t& relative_path,
                        const std::vector<utility::string_t>& paths,
                        const web::json::value& body) const;
    void HandlePostRelationship(const std::string& user, const std::string& token,
                                const web::http::http_request& request,
                                const utility::string_t& relative_path,
                                const std::vector<utility::string_t>& paths,
                                const web::json::value& body) const;
    void HandlePostLabel(const std::string& user, const std::string& token,
                         const web::http::http_request& request,
                         const utility::string_t& relative_path,
                         const std::vector<utility::string_t>& paths,
                         const web::json::value& body) const;
    void HandlePostIndex(const std::string& user, const std::string& token,
                         const web::http::http_request& request,
                         const utility::string_t& relative_path,
                         const std::vector<utility::string_t>& paths,
                         const web::json::value& body) const;
    void HandlePostPlugin(const std::string& user, const std::string& token,
                          const web::http::http_request& request,
                          const utility::string_t& relative_path,
                          const std::vector<utility::string_t>& paths,
                          const web::json::value& body) const;
    void HandlePostSubGraph(const std::string& user, const std::string& token,
                            const web::http::http_request& request,
                            const utility::string_t& relative_path,
                            const std::vector<utility::string_t>& paths,
                            const web::json::value& body) const;
    void HandlePostImport(const std::string& user, const std::string& token,
                          const web::http::http_request& request,
                          const utility::string_t& relative_path,
                          const std::vector<utility::string_t>& paths,
                          const web::json::value& body) const;
    void HandlePostExport(const std::string& user, const std::string& token,
                          const web::http::http_request& request,
                          const utility::string_t& relative_path,
                          const std::vector<utility::string_t>& paths,
                          const web::json::value& body) const;
    void HandlePostSchema(const std::string& user, const std::string& token,
                          const web::http::http_request& request,
                          const utility::string_t& relative_path,
                          const std::vector<utility::string_t>& paths,
                          const web::json::value& body) const;

    void HandleDeleteNode(const std::string& user, const std::string& token,
                          const web::http::http_request& request,
                          const utility::string_t& relative_path,
                          const std::vector<utility::string_t>& paths) const;
    void HandleDeleteRelationship(const std::string& user, const std::string& token,
                                  const web::http::http_request& request,
                                  const utility::string_t& relative_path,
                                  const std::vector<utility::string_t>& paths) const;
    void HandleDeleteIndex(const std::string& user, const std::string& token,
                           const web::http::http_request& request,
                           const utility::string_t& relative_path,
                           const std::vector<utility::string_t>& paths) const;
    void HandleDeleteUser(const std::string& user, const std::string& token,
                          const web::http::http_request& request,
                          const utility::string_t& relative_path,
                          const std::vector<utility::string_t>& paths) const;
    void HandleDeleteGraph(const std::string& user, const std::string& token,
                           const web::http::http_request& request,
                           const utility::string_t& relative_path,
                           const std::vector<utility::string_t>& paths) const;
    void HandleDeleteRole(const std::string& user, const std::string& token,
                          const web::http::http_request& request,
                          const utility::string_t& relative_path,
                          const std::vector<utility::string_t>& paths) const;
    void HandleDeletePlugin(const std::string& user, const std::string& token,
                            const web::http::http_request& request,
                            const utility::string_t& relative_path,
                            const std::vector<utility::string_t>& paths) const;
    void HandleDeleteTask(const std::string& user, const web::http::http_request& request,
                          const utility::string_t& relative_path,
                          const std::vector<utility::string_t>& paths) const;

    void HandlePutNode(const std::string& user, const std::string& token,
                       const web::http::http_request& request,
                       const utility::string_t& relative_path,
                       const std::vector<utility::string_t>& paths,
                       const web::json::value& body) const;
    void HandlePutRelationship(const std::string& user, const std::string& token,
                               const web::http::http_request& request,
                               const utility::string_t& relative_path,
                               const std::vector<utility::string_t>& paths,
                               const web::json::value& body) const;
    void HandlePutUser(const std::string& user, const std::string& token,
                       const web::http::http_request& request,
                       const utility::string_t& relative_path,
                       const std::vector<utility::string_t>& paths,
                       const web::json::value& body) const;
    void HandlePutRole(const std::string& user, const std::string& token,
                       const web::http::http_request& request,
                       const utility::string_t& relative_path,
                       const std::vector<utility::string_t>& paths,
                       const web::json::value& body) const;
    void HandlePutConfig(const std::string& user, const std::string& token,
                         const web::http::http_request& request,
                         const utility::string_t& relative_path,
                         const std::vector<utility::string_t>& paths,
                         const web::json::value& body) const;

    web::http::http_response GetCorsResponse(web::http::status_code code) const;
    void RespondUnauthorized(const web::http::http_request& request,
                             const std::string& error) const;
    void RespondBadRequest(const web::http::http_request& request,
                           const std::string& error) const;
    void RespondBadURI(const web::http::http_request& request) const;
    void RespondBadJSON(const web::http::http_request& request) const;
    void RespondSuccess(const web::http::http_request& request,
                        const web::json::value& body) const;
    void RespondSuccess(const web::http::http_request& request,
                        const std::string& body) const;
    void RespondSuccess(const web::http::http_request& request) const;
    void RespondRedirect(const web::http::http_request& request,
                         const std::string& host,
                         const utility::string_t& relative_path,
                         bool need_leader) const;
    void RespondInternalError(const web::http::http_request& request,
                              const std::string& e) const;
    void RespondInternalException(const web::http::http_request& request,
                                  const std::exception& e) const;
    void RespondRSMError(const web::http::http_request& request,
                         const LGraphResponse& resp,
                         const utility::string_t& relative_path,
                         const std::string& request_type) const;
    bool RedirectIfServerTooOld(const web::http::http_request& request,
                                const utility::string_t& relative_path) const;

    LGraphResponse ApplyToStateMachine(const LGraphRequest& req) const;

    // html_content_map_ contains data about the html contents of the website, their mime types
    // key: relative URI path of the HTML content being requested
    // value: Tuple where:
    // Element1: relative path on the disk of the file being requested
    // Element2: Mime type/content type of the file
    std::map<utility::string_t, std::tuple<utility::string_t, utility::string_t>>
        html_content_map_;
    bool enable_web_service_ = true;

    web::http::experimental::listener::http_listener listener_;
    StateMachine* state_machine_;
    lgraph::Galaxy* galaxy_ = nullptr;
    Config config_;
    // ServiceConfig includes all config info, used in GetConfig
    const std::shared_ptr<GlobalConfig> global_config_;
#ifndef _WIN32
    cypher::Scheduler* cypher_scheduler_;
#endif
    bool started_ = false;
    std::unordered_map<utility::string_t, RestPathCases> path_to_case_;
};
}  // namespace lgraph
