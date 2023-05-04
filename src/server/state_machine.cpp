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

#include <random>

#include "core/audit_logger.h"
#include "core/killable_rw_lock.h"
#include "db/galaxy.h"
#include "import/import_online.h"
#include "protobuf/ha.pb.h"
#include "server/proto_convert.h"
#include "server/state_machine.h"
#ifndef _WIN32
#include "brpc/controller.h"
#endif

using namespace fma_common;


lgraph::StateMachine::StateMachine(const Config& config,
                                   std::shared_ptr<GlobalConfig> global_config)
    : config_(config), global_config_(global_config) {}

lgraph::StateMachine::~StateMachine() { Stop(); }

void lgraph::StateMachine::Start() {
    if (galaxy_) return;
    galaxy_.reset();
    Galaxy::Config conf;
    conf.dir = config_.db_dir;
    conf.durable = config_.durable;
    conf.optimistic_txn = config_.optimistic_txn;
    conf.load_plugins = true;
    galaxy_.reset(new Galaxy(conf, true, global_config_));
    if (global_config_ && global_config_->enable_backup_log) {
        backup_log_.reset(new BackupLog(global_config_->backup_log_dir,
                                        global_config_->max_backup_log_file_size));
        // check if snapshot already taken
        if (fma_common::file_system::ListSubDirs(global_config_->snapshot_dir, true).empty()) {
            // no snapshot, take one
            FMA_INFO_STREAM(logger_) << "Taking initial snapshot...";
            TakeSnapshot();
        }
    }
}

int64_t lgraph::StateMachine::GetVersion() { return galaxy_->GetRaftLogIndex(); }

void lgraph::StateMachine::Stop() {
    galaxy_.reset();
    backup_log_.reset();
}

bool lgraph::StateMachine::IsWriteRequest(const lgraph::LGraphRequest* req) {
    if (req->has_is_write_op()) {
        return req->is_write_op();
    } else {
        // determine if this is a write op
        if (req->Req_case() != LGraphRequest::kCypherRequest &&
            req->Req_case() != LGraphRequest::kPluginRequest) {
            throw InputError("is_write_op must be set for non-Cypher and non-plugin request.");
        }
        _HoldReadLock(galaxy_->GetReloadLock());
        if (req->Req_case() == LGraphRequest::kCypherRequest) {
#ifdef _WIN32
            throw InternalError("Cypher is not supported on Windows yet.");
#else
            // determine if this is a write op
            std::string name;
            std::string type;
            bool ret =
                cypher::Scheduler::DetermineReadOnly(req->cypher_request().query(), name, type);
            if (name.empty() || type.empty()) {
                return !ret;
            } else {
                const CypherRequest& creq = req->cypher_request();
                const std::string& user = GetCurrUser(req);
                AccessControlledDB db = galaxy_->OpenGraph(user, creq.graph());
                type.erase(remove(type.begin(), type.end(), '\"'), type.end());
                name.erase(remove(name.begin(), name.end(), '\"'), name.end());
                return !db.IsReadOnlyPlugin(type == "CPP" ? PluginManager::PluginType::CPP
                                                          : PluginManager::PluginType::PYTHON,
                                            req->token(), name);
            }
#endif
        } else {
            // must be plugin request
            FMA_DBG_CHECK_EQ(req->Req_case(), LGraphRequest::kPluginRequest);
            const PluginRequest& preq = req->plugin_request();
            if (preq.Req_case() != PluginRequest::kCallPluginRequest) {
                throw InputError("is_write_op must be set for load/unload plugin requests.");
            }
            const std::string& user = GetCurrUser(req);
            AccessControlledDB db = galaxy_->OpenGraph(user, preq.graph());
            return !db.IsReadOnlyPlugin(preq.type() == PluginRequest::CPP
                                            ? PluginManager::PluginType::CPP
                                            : PluginManager::PluginType::PYTHON,
                                        req->token(), preq.call_plugin_request().name());
        }
    }
}

bool lgraph::StateMachine::IsFromLegalHost(::google::protobuf::RpcController* controller) const {
#ifdef _WIN32
    return true;
#else
    if (controller && global_config_ &&
        global_config_->enable_ip_check.load(std::memory_order_acquire)) {
        auto ctl = dynamic_cast<brpc::Controller*>(controller);
        std::string ip = butil::ip2str(ctl->remote_side().ip).c_str();
        _HoldReadLock(galaxy_->GetReloadLock());
        if (!galaxy_->IsIpInWhitelist(ip)) {
            FMA_WARN_STREAM(logger_) << "illegal access from ip: " << ip;
            return false;
        }
    }
    return true;
#endif
}

void lgraph::StateMachine::HandleRequest(::google::protobuf::RpcController* controller,
                                         const LGraphRequest* req, LGraphResponse* resp,
                                         google::protobuf::Closure* on_done) {
    MyDoneGuard done_guard(on_done);
    try {
        if (!IsFromLegalHost(controller)) {
            RespondDenied(resp, "Client banned.");
            return;
        }
        bool is_write = IsWriteRequest(req);
        if (_F_UNLIKELY(is_write && backup_log_)) {
            backup_log_->Write(req);
        }
        DoRequest(is_write, req, resp, done_guard.Release());
    }
    // typically only TaskKilledException can occur here
    // other types of exceptions are already handled in ApplyRequestDirectly
    catch (TimeoutException& e) {
        RespondTimeout(resp, e.what());
    } catch (InputError& e) {
        RespondBadInput(resp, e.what());
    } catch (AuthError& e) {
        RespondDenied(resp, e.what());
    } catch (TaskKilledException& e) {
        RespondException(resp, e.what());
    } catch (std::exception& e) {
        RespondException(resp, std::string("Unhandled exception: ") + e.what());
    }
}

std::vector<std::string> lgraph::StateMachine::ListBackupLogFiles() {
    if (backup_log_)
        return backup_log_->ListLogFiles();
    else
        return std::vector<std::string>();
}

std::string lgraph::StateMachine::TakeSnapshot() {
    if (!global_config_) throw InputError("Snapshot cannot be taken in embedded mode.");
    // get old snapshot
    std::vector<std::string> old =
        fma_common::file_system::ListSubDirs(global_config_->snapshot_dir, false);
    // take new snapshot
    std::string now = DateTime::LocalNow().ToString();
    std::string new_name(now.size(), ' ');
    std::transform(now.begin(), now.end(), new_name.begin(), [](char c) {
        return c == ':' ? '.' : c == ' ' ? '_' : c;
    });
    std::string path = global_config_->snapshot_dir + "/" + new_name;
    TakeSnapshot(path, true);
    // delete old snapshot
    for (auto& p : old) {
        // remove it only if it is a snapshot dir
        if (p.size() != now.size()) continue;
        std::transform(p.begin(), p.end(), now.begin(), [](char c) {
            return c == '.' ? ':' : c == '_' ? ' ' : c;
        });
        DateTime d;
        if (!DateTime::Parse(now, d)) continue;
        // ok, remove
        FMA_INFO_STREAM(logger_) << "Removing old snapshot " << p;
        fma_common::file_system::RemoveDir(global_config_->snapshot_dir + "/" + p);
    }
    return path;
}

void lgraph::StateMachine::TakeSnapshot(const std::string& path, bool truncate_log) {
    galaxy_->SaveSnapshot(path);
    if (truncate_log && backup_log_) backup_log_->TruncateLogs();
}

bool lgraph::StateMachine::DoRequest(bool is_write, const LGraphRequest* req, LGraphResponse* resp,
                                     google::protobuf::Closure* on_done) {
    MyDoneGuard done_guard(on_done);
    return ApplyRequestDirectly(req, resp);
}

bool lgraph::StateMachine::ApplyRequestDirectly(const lgraph::LGraphRequest* req,
                                                lgraph::LGraphResponse* resp) {
    resp->set_error_code(LGraphResponse::SUCCESS);
    double start_time = fma_common::GetTime();
    int retry_time = 0;
    int max_retries = 10;
    while (true) {
        bool retry = false;
        try {
            _HoldReadLock(galaxy_->GetReloadLock());

            switch (req->Req_case()) {
            case LGraphRequest::kGraphApiRequest:
                {
                    FMA_DBG_STREAM(logger_) << "Apply a native request.";
                    ApplyGraphApiRequest(req, resp);
                    break;
                }
            case LGraphRequest::kCypherRequest:
                {
                    FMA_DBG_STREAM(logger_) << "Apply a cypher request.";
                    ApplyCypherRequest(req, resp);
                    break;
                }
            case LGraphRequest::kPluginRequest:
                {
                    FMA_DBG_STREAM(logger_) << "Apply a plugin request.";
                    ApplyPluginRequest(req, resp);
                    break;
                }
            case LGraphRequest::kAclRequest:
                {
                    FMA_DBG_STREAM(logger_) << "Apply a acl request.";
                    ApplyAclRequest(req, resp);
                    break;
                }
            case LGraphRequest::kGraphRequest:
                {
                    FMA_DBG_STREAM(logger_) << "Apply a graph request.";
                    ApplyGraphRequest(req, resp);
                    break;
                }
            case LGraphRequest::kImportRequest:
                {
                    FMA_DBG_STREAM(logger_) << "Received an import request.";
                    ApplyImportRequest(req, resp);
                    break;
                }
            case LGraphRequest::kConfigRequest:
                {
                    FMA_DBG_STREAM(logger_) << "Received config request: " << req->DebugString();
                    ApplyConfigRequest(req, resp);
                    break;
                }
            case LGraphRequest::kRestoreRequest:
                {
                    FMA_DBG_STREAM(logger_) << "Received restore request.";
                    ApplyRestoreRequest(req, resp);
                    break;
                }
            case LGraphRequest::kSchemaRequest:
                {
                    FMA_DBG_STREAM(logger_) << "Received a schema request.";
                    ApplySchemaRequest(req, resp);
                    break;
                }
            default:
                {
                    FMA_WARN_STREAM(logger_) << "Unhandled request type: " << req->Req_case();
                    RespondException(resp, "Unhandled request type.");
                    break;
                }
            }
        } catch (TimeoutException& e) {
            RespondTimeout(resp, e.what());
        } catch (InputError& e) {
            RespondBadInput(resp, e.what());
        } catch (AuthError& e) {
            RespondDenied(resp, e.what());
        } catch (TaskKilledException& e) {
            RespondException(resp, e.what());
        } catch (LockUpgradeFailedException& e) {
            if (retry_time < max_retries) {
                std::default_random_engine engine;
                std::uniform_int_distribution<size_t> t(1000, 5000);
                fma_common::SleepUs(t(engine));
                retry = true;
                retry_time += 1;
            } else {
                RespondException(resp, e.what());
            }
        } catch (std::exception& e) {
            RespondException(resp, e.what());
        }
        if (!retry) {
            break;
        }
    }
    double end_time = fma_common::GetTime();
    if (req->Req_case() == LGraphRequest::kPluginRequest &&
        req->plugin_request().Req_case() == PluginRequest::kCallPluginRequest) {
        auto preq = req->plugin_request().call_plugin_request();
        auto param = preq.param();
        boost::replace_all(param, "\n", "");
        FMA_DBG_STREAM(logger_) << fma_common::StringFormatter::Format(
            "[Calling Plugin] plugin_name={}, elapsed={}, res={}, param=[{}]", preq.name(),
            end_time - start_time, LGraphResponse_ErrorCode_Name(resp->error_code()), param);
    } else if (req->Req_case() == LGraphRequest::kCypherRequest) {
        auto query = req->cypher_request().query();
        boost::replace_all(query, "\n", "");
        FMA_DBG_STREAM(logger_) << fma_common::StringFormatter::Format(
            "[Calling Cypher] elapsed={}, res={}, cypher=[{}]", end_time - start_time,
            LGraphResponse_ErrorCode_Name(resp->error_code()), query);
    }
    if (resp->error_code() == LGraphResponse::SUCCESS)
        AUDIT_LOG_SUCC();
    else
        AUDIT_LOG_FAIL(resp->error());
    return resp->error_code() == LGraphResponse::SUCCESS;
}

bool lgraph::StateMachine::RespondRedirect(LGraphResponse* resp, const std::string& target,
                                           const std::string& reason) const {
    resp->set_error_code(LGraphResponse::REDIRECT);
    resp->set_redirect(target);
    resp->set_error(reason);
    resp->set_server_version(galaxy_->GetRaftLogIndex());
    return false;
}

bool lgraph::StateMachine::RespondException(LGraphResponse* resp, const std::string& reason) const {
    resp->set_error_code(LGraphResponse::EXCEPTION);
    resp->set_error(reason + (resp->error().empty() ? "" : ": " + resp->error()));
    resp->set_server_version(galaxy_->GetRaftLogIndex());
    return false;
}

bool lgraph::StateMachine::RespondSuccess(LGraphResponse* resp) const {
    resp->set_error_code(LGraphResponse::SUCCESS);
    resp->set_server_version(galaxy_->GetRaftLogIndex());
    TaskTracker::GetInstance().GetThreadContext()->MarkTaskSuccess();
    return true;
}

bool lgraph::StateMachine::RespondDenied(LGraphResponse* resp, const std::string& what) const {
    resp->set_error_code(LGraphResponse::AUTH_ERROR);
    resp->set_error("Access denied: " + what);
    resp->set_server_version(galaxy_->GetRaftLogIndex());
    return false;
}

bool lgraph::StateMachine::RespondTimeout(LGraphResponse* resp, const std::string& what) const {
    resp->set_error_code(LGraphResponse::KILLED);
    resp->set_error("Task killed.");
    resp->set_server_version(galaxy_->GetRaftLogIndex());
    return false;
}

bool lgraph::StateMachine::RespondBadInput(LGraphResponse* resp, const std::string& error) const {
    resp->set_error_code(LGraphResponse::BAD_REQUEST);
    resp->set_error(error);
    resp->set_server_version(galaxy_->GetRaftLogIndex());
    return false;
}

inline std::string AlreadExistsMsg(const std::string& what, const std::string& name) {
    return FMA_FMT("{} [{}] already exists.", what, name);
}

inline std::string NotFoundMsg(const std::string& what, const std::string& name) {
    return FMA_FMT("{} [{}] does not exist.", what, name);
}

lgraph::AccessControlledDB lgraph::StateMachine::GetDB(const std::string& token,
                                                       const std::string& graph) {
    const std::string& user = galaxy_->ParseAndValidateToken(token);
    return galaxy_->OpenGraph(user, graph);
}

std::string lgraph::StateMachine::GetCurrUser(const LGraphRequest* lgraph_req) {
    return galaxy_->ParseAndValidateToken(lgraph_req->token());
}

std::string lgraph::StateMachine::GetCurrUser(const LGraphRequest* lgraph_req, bool* is_admin) {
    return galaxy_->ParseTokenAndCheckIfIsAdmin(lgraph_req->token(), is_admin);
}

bool lgraph::StateMachine::ApplySchemaRequest(const LGraphRequest* req, LGraphResponse* resp) {
    if (req->Req_case() != LGraphRequest::kSchemaRequest) {
        std::string curr_user = req->has_user() ?
                req->user() : galaxy_->ParseAndValidateToken(req->token());
        BEG_AUDIT_LOG(curr_user, "", lgraph::LogApiType::SingleApi, true,
                                      req->DebugString());
        return RespondBadInput(resp, FMA_FMT("Unhandled request type [{}].", req->Req_case()));
    }
    auto& schema = req->schema_request();
    std::string user = req->has_user() ?
            req->user() : galaxy_->ParseAndValidateToken(req->token());
    AutoTaskTracker task_tracker("schema", true, true);
    BEG_AUDIT_LOG(user, schema.graph(), lgraph::LogApiType::SingleApi, true,
                                  FMA_FMT("Schema [{}].", schema.description()));
    AccessControlledDB db = galaxy_->OpenGraph(user, schema.graph());
    if (db.GetAccessLevel() < AccessLevel::WRITE)
        return RespondDenied(resp, "Need write permission to do import.");
    std::string desc = schema.description();
    std::string log = ::lgraph::import_v2::ImportOnline::HandleOnlineSchema(std::move(desc), db);
    if (log != "") {
        resp->mutable_schema_response()->set_log(log);
        return RespondBadInput(resp, log);
    }
    resp->mutable_schema_response()->set_log(std::move(log));
    return RespondSuccess(resp);
}
bool lgraph::StateMachine::ApplyImportRequest(const LGraphRequest* req, LGraphResponse* resp) {
    if (req->Req_case() != LGraphRequest::kImportRequest) {
        std::string curr_user = req->has_user() ?
                req->user() : galaxy_->ParseAndValidateToken(req->token());
        BEG_AUDIT_LOG(curr_user, "", lgraph::LogApiType::SingleApi, true,
                                      req->DebugString());
        return RespondBadInput(resp, FMA_FMT("Unhandled request type [{}].", req->Req_case()));
    }
    auto& import = req->import_request();
    std::string user = req->has_user() ? req->user() : galaxy_->ParseAndValidateToken(req->token());
    AutoTaskTracker task_tracker("import", true, true);
    BEG_AUDIT_LOG(user, import.graph(), lgraph::LogApiType::SingleApi, true,
                                  FMA_FMT("Import [{}].", import.description()));
    AccessControlledDB db = galaxy_->OpenGraph(user, import.graph());
    if (db.GetAccessLevel() < AccessLevel::WRITE)
        return RespondDenied(resp, "Need write permission to do import.");
    std::string desc = import.description();
    std::string data = import.data();
    ::lgraph::import_v2::ImportOnline::Config config;
    config.continue_on_error = import.continue_on_error();
    config.n_threads = 8;
    config.delimiter = import.delimiter();
    std::string log = ::lgraph::import_v2::ImportOnline::HandleOnlineTextPackage(
        std::move(desc), std::move(data), db.GetLightningGraph(), config);
    resp->mutable_import_response()->set_log(std::move(log));
    return RespondSuccess(resp);
}

bool lgraph::StateMachine::ApplyConfigRequest(const LGraphRequest* lgraph_req,
                                              LGraphResponse* resp) {
    static fma_common::Logger& logger =
        fma_common::Logger::Get("lgraph.StateMachine.ApplyConfigRequest");
    const ConfigRequest& req = lgraph_req->config_request();
    ConfigResponse* aresp = resp->mutable_config_response();
    switch (req.Req_case()) {
    case ConfigRequest::kModConfigRequest:
        {
            std::string curr_user = lgraph_req->has_user() ?
                    lgraph_req->user() : GetCurrUser(lgraph_req);
            bool is_admin = galaxy_->IsAdmin(curr_user);
            AutoTaskTracker task_tracker(false, false);
            BEG_AUDIT_LOG(curr_user, "", lgraph::LogApiType::Security, true,
                                          req.DebugString());
            if (!is_admin) {
                return RespondDenied(resp, "Non-admin user is not allowed to update configs.");
            } else {
                const ModConfigRequest& mcreq = req.mod_config_request();
                if (mcreq.keys_size() != mcreq.values_size())
                    return RespondBadInput(
                        resp, FMA_FMT("Number of keys and values does not match: {} vs. {}",
                                      mcreq.keys_size(), mcreq.values_size()));
                std::map<std::string, FieldData> updates;
                for (int i = 0; i < mcreq.keys_size(); i++)
                    updates.emplace(mcreq.keys()[i],
                                    FieldDataConvert::ToLGraphT(mcreq.values()[i]));
                if (updates.empty()) return RespondBadInput(resp, "Empty input.");
                bool need_reload = galaxy_->UpdateConfig(curr_user, updates);
                if (need_reload) galaxy_->ReloadFromDisk(false);
                return RespondSuccess(resp);
            }
        }
    default:
        {
            std::string err = FMA_FMT("Unhandled config request type [{}].", req.Req_case());
            return RespondBadInput(resp, err);
        }
    }
}

bool lgraph::StateMachine::ApplyRestoreRequest(const lgraph::LGraphRequest* lgraph_req,
                                               lgraph::LGraphResponse* resp) {
    std::string curr_user = lgraph_req->has_user() ? lgraph_req->user() : GetCurrUser(lgraph_req);
    bool is_admin = galaxy_->IsAdmin(curr_user);
    if (!is_admin) return RespondDenied(resp, "Non-admin cannot perform restore.");
    AutoTaskTracker task_tracker("restore", true, true);
    RestoreResponse* rresp = resp->mutable_restore_response();
    BEG_AUDIT_LOG(curr_user, "", lgraph::LogApiType::Security, true, "Restore");
    for (auto& log : lgraph_req->restore_request().logs()) {
        LGraphResponse lresp;
        lgraph::LGraphRequest m_req;
        m_req.CopyFrom(log.req());
        m_req.set_user(curr_user);
        ApplyRequestDirectly(&m_req, &lresp);
        if (lresp.error_code() != LGraphResponse::SUCCESS)
            return RespondBadInput(
                resp, FMA_FMT("Failed to apply log [{}]: {}", log.DebugString(), lresp.error()));
        rresp->set_last_success_idx(log.index());
    }
    return RespondSuccess(resp);
}

#define RETURN_SUCCESS_OR_BAD_INPUT(success, resp, error_msg) \
    if (success)                                              \
        return RespondSuccess(resp);                          \
    else                                                      \
        return RespondBadInput(resp, (error_msg));

bool lgraph::StateMachine::ApplyGraphRequest(const LGraphRequest* lgraph_req,
                                             LGraphResponse* resp) {
    static fma_common::Logger& logger =
        fma_common::Logger::Get("lgraph.StateMachine.ApplyGraphRequest");
    const GraphRequest& req = lgraph_req->graph_request();
    GraphResponse* gresp = resp->mutable_graph_response();
    std::string curr_user = lgraph_req->has_user() ? lgraph_req->user() : GetCurrUser(lgraph_req);
    switch (req.Req_case()) {
    case GraphRequest::kAddGraphRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            BEG_AUDIT_LOG(curr_user, "", lgraph::LogApiType::Security, true,
                                          req.DebugString());
            const AddGraphRequest& ar = req.add_graph_request();
            bool r = galaxy_->CreateGraph(curr_user, ar.name(), convert::ToLGraphT(ar.config()));
            RETURN_SUCCESS_OR_BAD_INPUT(r, resp, FMA_FMT("Graph [{}] already exists.", ar.name()));
        }
    case GraphRequest::kDeleteGraphRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            BEG_AUDIT_LOG(curr_user, "", lgraph::LogApiType::Security, true,
                                          req.DebugString());
            const DeleteGraphRequest& dr = req.delete_graph_request();
            bool r = galaxy_->DeleteGraph(curr_user, dr.name());
            RETURN_SUCCESS_OR_BAD_INPUT(r, resp, FMA_FMT("Graph [{}] does not exist.", dr.name()));
        }
    default:
        {
            BEG_AUDIT_LOG(curr_user, "", lgraph::LogApiType::Security, true,
                                          req.DebugString());
            return RespondBadInput(resp, FMA_FMT("Unhandled request type [{}].", req.Req_case()));
        }
    }
}

bool lgraph::StateMachine::ApplyAclRequest(const LGraphRequest* lgraph_req, LGraphResponse* resp) {
    static fma_common::Logger& logger =
        fma_common::Logger::Get("lgraph.StateMachine.ApplyAclRequest");
    const AclRequest& req = lgraph_req->acl_request();
    AclResponse* aresp = resp->mutable_acl_response();

    // get current user if this is not auth request
    // for auth request, curr_user is not specified
    std::string curr_user;
    if (req.Req_case() != AclRequest::kAuthRequest) {
        curr_user = lgraph_req->has_user() ? lgraph_req->user() : GetCurrUser(lgraph_req);
    }
    const AuthRequest& re = req.auth_request();
    switch (req.Req_case()) {
    case AclRequest::kAddRoleRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            const auto& areq = req.add_role_request();
            BEG_AUDIT_LOG(curr_user, "", lgraph::LogApiType::Security, true,
                                          req.DebugString());
            bool success = galaxy_->CreateRole(curr_user, areq.role(), areq.desc());
            RETURN_SUCCESS_OR_BAD_INPUT(success, resp,
                                        FMA_FMT("Role [{}] already exists.", areq.role()));
        }
    case AclRequest::kAddUserRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            const auto& areq = req.add_user_request();
            BEG_AUDIT_LOG(curr_user, "", lgraph::LogApiType::Security, true,
                                          req.DebugString());
            bool success =
                galaxy_->CreateUser(curr_user, areq.user(), areq.password(), areq.desc());
            RETURN_SUCCESS_OR_BAD_INPUT(success, resp,
                                        FMA_FMT("User [{}] already exists.", areq.user()));
        }
    case AclRequest::kAuthRequest:
        switch (re.action_case()) {
            case AuthRequest::kLogin:
                {
                    AutoTaskTracker task_tracker(false, true);
                    const auto& auth_info = req.auth_request().login();
                    BEG_AUDIT_LOG(auth_info.user(), "",
                            lgraph::LogApiType::Security, false, "login");
                    std::string token = galaxy_->GetUserToken(auth_info.user(),
                                                            auth_info.password());
                    if (token.empty()) return RespondDenied(resp, "Bad user/password.");
                    aresp->mutable_auth_response()->set_token(std::move(token));
                    return RespondSuccess(resp);
                }
            case AuthRequest::kLogout:
                {
                    AutoTaskTracker task_tracker(false, true);
                    const auto& auth_info = req.auth_request().logout();
                    galaxy_->UnBindTokenUser(auth_info.token());
                    return RespondSuccess(resp);
                }
            default:
            {
                BEG_AUDIT_LOG("", "", lgraph::LogApiType::Security, true,
                                            req.DebugString());
                return RespondBadInput(resp, FMA_FMT("Unhandled request type [{}].",
                                                                    req.Req_case()));
            }
        }
    case AclRequest::kDelRoleRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            const auto& areq = req.del_role_request();
            BEG_AUDIT_LOG(curr_user, "", lgraph::LogApiType::Security, true,
                                          req.DebugString());
            bool success = galaxy_->DeleteRole(curr_user, areq.role());
            RETURN_SUCCESS_OR_BAD_INPUT(success, resp,
                                        FMA_FMT("Role [{}] does not exist.", areq.role()));
        }
    case AclRequest::kDelUserRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            const auto& areq = req.del_user_request();
            BEG_AUDIT_LOG(curr_user, "", lgraph::LogApiType::Security, true,
                                          req.DebugString());
            bool success = galaxy_->DeleteUser(curr_user, areq.user());
            RETURN_SUCCESS_OR_BAD_INPUT(success, resp,
                                        FMA_FMT("User [{}] does not exist.", areq.user()));
        }
    case AclRequest::kModRoleRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            const auto& areq = req.mod_role_request();
            BEG_AUDIT_LOG(curr_user, "", lgraph::LogApiType::Security, true,
                                          req.DebugString());
            bool success = galaxy_->ModRole(curr_user, areq);
            RETURN_SUCCESS_OR_BAD_INPUT(success, resp,
                                        FMA_FMT("Role [{}] does not exist.", areq.role()));
        }
    case AclRequest::kModUserRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            const auto& areq = req.mod_user_request();
            BEG_AUDIT_LOG(curr_user, "", lgraph::LogApiType::Security, true,
                                          req.DebugString());
            bool success = galaxy_->ModUser(curr_user, areq);
            RETURN_SUCCESS_OR_BAD_INPUT(success, resp,
                                        FMA_FMT("Role [{}] does not exist.", areq.user()));
        }
    default:
        {
            BEG_AUDIT_LOG("", "", lgraph::LogApiType::Security, true,
                                          req.DebugString());
            return RespondBadInput(resp, FMA_FMT("Unhandled request type [{}].", req.Req_case()));
        }
    }
}

bool lgraph::StateMachine::ApplyGraphApiRequest(const LGraphRequest* lgraph_req,
                                                LGraphResponse* resp) {
    static fma_common::Logger& logger =
        fma_common::Logger::Get("lgraph.StateMachine.ApplyGraphApiRequest");

    const GraphApiRequest& req = lgraph_req->graph_api_request();
    GraphApiResponse* gresp = resp->mutable_graph_api_response();
    std::string curr_user;
    std::unique_ptr<lgraph::AccessControlledDB> db;
    if (lgraph_req->has_user()) {
        curr_user = lgraph_req->user();
        db = std::make_unique<lgraph::AccessControlledDB>(
                galaxy_->OpenGraph(curr_user, req.graph()));
    } else {
        curr_user = GetCurrUser(lgraph_req);
        db = std::make_unique<lgraph::AccessControlledDB>(GetDB(lgraph_req->token(), req.graph()));
    }
    switch (req.Req_case()) {
    // label
    case GraphApiRequest::kAddLabelRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            const auto& lreq = req.add_label_request();
            BEG_AUDIT_LOG(curr_user, req.graph(), lgraph::LogApiType::SingleApi,
                                          true, req.DebugString());
            if (lreq.is_vertex() && !lreq.has_primary()) {
                return RespondBadInput(resp, FMA_FMT("Missing primary field"));
            }
            EdgeConstraints ec;
            if (!lreq.is_vertex()) {
                for (auto& item : lreq.edge_constraints()) {
                    ec.push_back(std::make_pair(item.src_label(), item.dst_label()));
                }
            }
            bool success =
                db->AddLabel(lreq.is_vertex(), lreq.label(),
                            FieldSpecConvert::ToLGraphT(lreq.fields()), lreq.primary(), ec);
            AddLabelResponse* lresp = gresp->mutable_add_label_response();
            if (success) {
                return RespondSuccess(resp);
            } else {
                return RespondBadInput(resp, FMA_FMT("Label [{}] already exists.", req.graph()));
            }
        }
    // index
    case GraphApiRequest::kAddIndexRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            const auto& ireq = req.add_index_request();
            BEG_AUDIT_LOG(curr_user, req.graph(), lgraph::LogApiType::SingleApi,
                                          true, req.DebugString());
            bool success = db->AddVertexIndex(ireq.label(), ireq.field(), ireq.is_unique());
            if (success) {
                return RespondSuccess(resp);
            } else {
                return RespondBadInput(resp, FMA_FMT("VertexIndex [{}:{}] already exists.",
                                                     ireq.label(), ireq.field()));
            }
        }
    case GraphApiRequest::kDelIndexRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            const auto& ireq = req.del_index_request();
            BEG_AUDIT_LOG(curr_user, req.graph(), lgraph::LogApiType::SingleApi,
                                          true, req.DebugString());
            bool success = db->DeleteVertexIndex(ireq.label(), ireq.field());
            if (success) {
                return RespondSuccess(resp);
            } else {
                return RespondBadInput(resp, FMA_FMT("VertexIndex [{}:{}] does not exists.",
                                                     ireq.label(), ireq.field()));
            }
        }
    // vertex
    case GraphApiRequest::kAddVertexesRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            BEG_AUDIT_LOG(curr_user, req.graph(), lgraph::LogApiType::SingleApi,
                                          true, "Add vertexes");
            const auto& vreq = req.add_vertexes_request();
            std::vector<std::string> fields(vreq.fields().begin(), vreq.fields().end());
            AddVertexesResponse* vresp = gresp->mutable_add_vertexes_response();
            lgraph::Transaction txn = db->CreateWriteTxn();
            size_t label_id = txn.GetLabelId(true, vreq.label());
            auto field_ids = txn.GetFieldIds(true, label_id, fields);
            for (auto& vertex : vreq.vertexes()) {
                VertexId vid = txn.AddVertex(label_id, field_ids,
                                             FieldDataConvert::ToLGraphT(vertex.values()));
                vresp->add_vid(vid);
            }
            txn.Commit();
            return RespondSuccess(resp);
        }
    case GraphApiRequest::kDelVertexRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            const auto& vreq = req.del_vertex_request();
            BEG_AUDIT_LOG(curr_user, req.graph(), lgraph::LogApiType::SingleApi,
                                          true, req.DebugString());
            size_t ni, no;
            lgraph::Transaction txn = db->CreateWriteTxn();
            bool success = txn.DeleteVertex(vreq.vid(), &ni, &no);
            txn.Commit();
            if (success) {
                auto* dvresp = resp->mutable_graph_api_response()->mutable_del_vertex_response();
                dvresp->set_n_ins(static_cast<int64_t>(ni));
                dvresp->set_n_outs(static_cast<int64_t>(no));
                return RespondSuccess(resp);
            } else {
                return RespondBadInput(resp, FMA_FMT("Vertex [{}] does not exist.", vreq.vid()));
            }
        }
    case GraphApiRequest::kModVertexRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            const auto& vreq = req.mod_vertex_request();
            BEG_AUDIT_LOG(curr_user, req.graph(), lgraph::LogApiType::SingleApi,
                                          true, req.DebugString());
            lgraph::Transaction txn = db->CreateWriteTxn();
            auto values = FieldDataConvert::ToLGraphT(vreq.values().values());
            std::vector<std::string> fields(vreq.fields().begin(), vreq.fields().end());
            bool success = false;
            if (vreq.has_label()) {
                return RespondBadInput(resp, "Changing vertex label is not allowed for now.");
                // success = txn.RecreateVertexProperty(vreq.vid(), vreq.label(), fields, values);
            } else {
                success = txn.SetVertexProperty(vreq.vid(), fields, values);
            }
            txn.Commit();
            if (success) {
                return RespondSuccess(resp);
            } else {
                return RespondBadInput(resp, FMA_FMT("Vertex [{}] does not exist.", vreq.vid()));
            }
        }
    // edge
    case GraphApiRequest::kAddEdgesRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            BEG_AUDIT_LOG(curr_user, req.graph(), lgraph::LogApiType::SingleApi,
                                          true, "Add edges");
            const auto& ereq = req.add_edges_request();
            std::vector<std::string> fields(ereq.fields().begin(), ereq.fields().end());
            AddEdgesResponse* eresp = gresp->mutable_add_edges_response();
            lgraph::Transaction txn = db->CreateWriteTxn();
            size_t label_id = txn.GetLabelId(false, ereq.label());
            auto field_ids = txn.GetFieldIds(false, label_id, fields);
            eresp->set_lid(label_id);
            eresp->set_tid(0);
            for (auto& edge : ereq.edges()) {
                EdgeUid euid = txn.AddEdge(edge.src(), edge.dst(), label_id, field_ids,
                                           FieldDataConvert::ToLGraphT(edge.values().values()));
                eresp->add_eids(euid.eid);
            }
            txn.Commit();
            return RespondSuccess(resp);
        }
    case GraphApiRequest::kDelEdgeRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            BEG_AUDIT_LOG(curr_user, req.graph(), lgraph::LogApiType::SingleApi,
                                          true, req.DebugString());
            const auto& ereq = req.del_edge_request();
            lgraph::Transaction txn = db->CreateWriteTxn();
            EdgeUid euid(ereq.src(), ereq.dst(),
                static_cast<LabelId>(ereq.lid()), 0, ereq.eid());  // TODO(heng)
            bool success = txn.DeleteEdge(euid);
            txn.Commit();
            if (success) {
                return RespondSuccess(resp);
            } else {
                return RespondBadInput(resp, FMA_FMT("Edge [{}] does not exist.", euid));
            }
        }
    case GraphApiRequest::kModEdgeRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            BEG_AUDIT_LOG(curr_user, req.graph(), lgraph::LogApiType::SingleApi,
                                          true, req.DebugString());
            const auto& ereq = req.mod_edge_request();
            auto values = FieldDataConvert::ToLGraphT(ereq.values().values());
            std::vector<std::string> fields(ereq.fields().begin(), ereq.fields().end());
            if (values.size() != fields.size())
                return RespondBadInput(
                    resp, FMA_FMT("Number of fields and values does not match: [{}] vs [{}].",
                                  fields.size(), values.size()));
            EdgeUid euid(ereq.src(), ereq.dst(),
                static_cast<LabelId>(ereq.lid()), 0, ereq.eid());  // TODO(heng)
            lgraph::Transaction txn = db->CreateWriteTxn();
            bool success = txn.SetEdgeProperty(euid, fields, values);
            txn.Commit();
            if (success) {
                return RespondSuccess(resp);
            } else {
                return RespondBadInput(resp, FMA_FMT("Edge [{}] does not exist.", euid));
            }
        }
    // other
    case GraphApiRequest::kFlushRequest:
        {
            if (!galaxy_->IsAdmin(curr_user))
                return RespondDenied(resp, "Non-admin users cannot flush graph.");
            AutoTaskTracker task_tracker("Flush", true, false);
            BEG_AUDIT_LOG(curr_user, req.graph(), lgraph::LogApiType::SingleApi,
                                          true, "Flush graph");
            db->Flush();
            return RespondSuccess(resp);
        }
    case GraphApiRequest::kSubGraphRequest:
        {
            AutoTaskTracker task_tracker("SubGraph", true, false);
            BEG_AUDIT_LOG(curr_user, req.graph(), lgraph::LogApiType::SingleApi,
                                          false, "Subgraph");
            const auto& sreq = req.sub_graph_request();
            auto* sresp = resp->mutable_graph_api_response()->mutable_sub_graph_response();
            auto txn = db->CreateReadTxn();
            auto it = txn.GetVertexIterator();
            std::set<lgraph::VertexId> set_vids(sreq.vids().begin(), sreq.vids().end());
            for (auto vid : set_vids) {
                if (!it.Goto(vid, false)) continue;
                VertexData* vdata = sresp->add_nodes();
                vdata->set_vid(vid);
                vdata->set_label(txn.GetVertexLabel(it));
                for (auto& f : txn.GetVertexFields(it)) {
                    Property* prop = vdata->add_properties();
                    prop->set_key(f.first);
                    FieldDataConvert::FromLGraphT(f.second, prop->mutable_value());
                }
                for (auto eit = it.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                    auto dst = eit.GetDst();
                    if (set_vids.find(dst) == set_vids.end()) continue;
                    EdgeData* edata = sresp->add_edges();
                    edata->set_src(vid);
                    edata->set_lid(txn.GetEdgeLabelId(eit));
                    edata->set_dst(dst);
                    edata->set_eid(eit.GetEdgeId());
                    edata->set_label(txn.GetEdgeLabel(eit));
                    for (auto& f : txn.GetEdgeFields(eit)) {
                        Property* prop = edata->add_properties();
                        prop->set_key(f.first);
                        FieldDataConvert::FromLGraphT(f.second, prop->mutable_value());
                    }
                }
            }
            return RespondSuccess(resp);
        }
    default:
        {
            return RespondBadInput(resp, FMA_FMT("Unhandled request type [{}].", req.Req_case()));
        }
    }
}

bool lgraph::StateMachine::ApplyCypherRequest(const LGraphRequest* lgraph_req,
                                              LGraphResponse* resp) {
#ifdef _WIN32
    return RespondException(resp, "Cypher is not supported on windows servers.");
#else
    bool is_write;
    using namespace web;
    if (lgraph_req->has_is_write_op()) {
        is_write = lgraph_req->is_write_op();
    } else {
        std::string name;
        std::string type;
        bool ret =
            cypher::Scheduler::DetermineReadOnly(lgraph_req->cypher_request().query(), name, type);
        if (name.empty() || type.empty()) {
            is_write = !ret;
        } else {
            const CypherRequest& creq = lgraph_req->cypher_request();
            std::string user = lgraph_req->has_user() ?
                    lgraph_req->user() : GetCurrUser(lgraph_req);
            AccessControlledDB db = galaxy_->OpenGraph(user, creq.graph());
            type.erase(remove(type.begin(), type.end(), '\"'), type.end());
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());
            is_write = !db.IsReadOnlyPlugin(
                type == "CPP" ? PluginManager::PluginType::CPP : PluginManager::PluginType::PYTHON,
                user, name);
        }
    }
    AutoTaskTracker task_tracker("[CYPHER] " + lgraph_req->cypher_request().query(), true,
                                 is_write);
    static fma_common::Logger& logger =
        fma_common::Logger::Get("lgraph.StateMachine.ApplyCypherRequest");
    const CypherRequest& req = lgraph_req->cypher_request();
    CypherResponse* cresp = resp->mutable_cypher_response();
    // const std::string& user = GetCurrUser(lgraph_req);
    std::string user = lgraph_req->has_user() ? lgraph_req->user() : GetCurrUser(lgraph_req);
    auto field_access = galaxy_->GetRoleFieldAccessLevel(user, req.graph());
    cypher::RTContext ctx(this, galaxy_.get(), lgraph_req->token(), user, req.graph(),
                          field_access);

    BEG_AUDIT_LOG(user, req.graph(), lgraph::LogApiType::Cypher, is_write,
                                  "[CYPHER] " + req.query());
    TimeoutTaskKiller timeout_killer;
    if (req.has_timeout() && req.timeout() != 0) {
        timeout_killer.SetTimeout(req.timeout());
    }
    cypher::ElapsedTime elapsed;
    if (!req.param_names().empty()) {
        const auto& pnames = req.param_names();
        const auto& pvalues = req.param_values().values();
        if (pnames.size() != pvalues.size())
            return RespondBadInput(resp, "Cypher arguments does not match number of parameters.");
        for (size_t i = 0; i < (size_t)pnames.size(); i++) {
            ctx.param_tab_.emplace(pnames.Get(i),
                                   cypher::FieldData(FieldDataConvert::ToLGraphT(pvalues.Get(i))));
        }
    }

    ctx.optimistic_ = config_.optimistic_txn;
    cypher_scheduler_.Eval(&ctx, req.query(), elapsed);
    elapsed.t_total = elapsed.t_compile + elapsed.t_exec;
    if (req.result_in_json_format()) {
        auto result = ctx.result_->Dump(false);
        cresp->set_json_result(std::move(result));
        return RespondSuccess(resp);
    } else {
        CypherResult* cypher_result = cresp->mutable_binary_result();
        for (auto& h : ctx.result_->Header()) {
            Header* header = cypher_result->add_header();
            header->set_name(h.first);
            switch (h.second) {
            case lgraph_api::LGraphType::NUL:
            case lgraph_api::LGraphType::INTEGER:
            case lgraph_api::LGraphType::FLOAT:
            case lgraph_api::LGraphType::DOUBLE:
            case lgraph_api::LGraphType::BOOLEAN:
            case lgraph_api::LGraphType::STRING:
            case lgraph_api::LGraphType::LIST:
            case lgraph_api::LGraphType::MAP:
            case lgraph_api::LGraphType::ANY:
                header->set_type(0);
                break;
            case lgraph_api::LGraphType::NODE:
                header->set_type(1);
                break;
            case lgraph_api::LGraphType::RELATIONSHIP:
                header->set_type(2);
                break;
            case lgraph_api::LGraphType::PATH:
                header->set_type(4);
            default:
                break;
            }
        }
        for (int row_n = 0; row_n < ctx.result_->Size(); row_n++) {
            ListOfProtoFieldData* res = cypher_result->add_result();
            FieldDataConvert::FromLGraphT(ctx.result_->Header(), ctx.result_->RecordView(row_n),
                                          res->mutable_values());
        }
        cypher_result->set_elapsed(elapsed.t_total);
        return RespondSuccess(resp);
    }
#endif
}

inline lgraph::PluginManager::PluginType GetPluginType(
    const lgraph::PluginRequest::PluginType& type) {
    static_assert((int)lgraph::PluginManager::PluginType::CPP == (int)lgraph::PluginRequest::CPP,
                  "");
    static_assert(
        (int)lgraph::PluginManager::PluginType::PYTHON == (int)lgraph::PluginRequest::PYTHON, "");
    return (lgraph::PluginManager::PluginType)type;
}

inline lgraph::plugin::CodeType GetPluginCodeType(const lgraph::LoadPluginRequest::CodeType& type) {
    static_assert((int)lgraph::plugin::CodeType::PY == (int)lgraph::LoadPluginRequest::PY, "");
    static_assert((int)lgraph::plugin::CodeType::SO == (int)lgraph::LoadPluginRequest::SO, "");
    static_assert((int)lgraph::plugin::CodeType::ZIP == (int)lgraph::LoadPluginRequest::ZIP, "");
    static_assert((int)lgraph::plugin::CodeType::CPP == (int)lgraph::LoadPluginRequest::CPP, "");
    return (lgraph::plugin::CodeType)type;
}

bool lgraph::StateMachine::ApplyPluginRequest(const LGraphRequest* lgraph_req,
                                              LGraphResponse* resp) {
    using namespace web;
    FMA_DBG_STREAM(logger_) << "Applying plugin request to state machine";
    const PluginRequest& req = lgraph_req->plugin_request();
    PluginResponse* presp = resp->mutable_plugin_response();
    std::string user;
    std::unique_ptr<lgraph::AccessControlledDB> db;
    if (lgraph_req->has_user()) {
        user = lgraph_req->user();
        db = std::make_unique<lgraph::AccessControlledDB>(
                galaxy_->OpenGraph(user, req.graph()));
    } else {
        user = GetCurrUser(lgraph_req);
        db = std::make_unique<lgraph::AccessControlledDB>(GetDB(lgraph_req->token(), req.graph()));
    }
    PluginManager::PluginType type = GetPluginType(req.type());
    switch (req.Req_case()) {
    case PluginRequest::kLoadPluginRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            const auto& preq = req.load_plugin_request();
            plugin::CodeType code_type = GetPluginCodeType(preq.code_type());
            BEG_AUDIT_LOG(user, req.graph(), lgraph::LogApiType::Plugin, true,
                                          FMA_FMT("Load plugin [{}]", preq.name()));
            bool r = db->LoadPlugin(type, user, preq.name(), preq.code(), code_type,
                                   preq.desc(), preq.read_only());
            if (r)
                return RespondSuccess(resp);
            else
                return RespondBadInput(resp, AlreadExistsMsg("Plugin", preq.name()));
        }
    case PluginRequest::kDelPluginRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            const auto& preq = req.del_plugin_request();
            BEG_AUDIT_LOG(user, req.graph(), lgraph::LogApiType::Plugin, true,
                                          FMA_FMT("Delete plugin [{}]", preq.name()));
            bool r = db->DelPlugin(type, user, preq.name());
            if (r)
                return RespondSuccess(resp);
            else
                return RespondBadInput(resp, NotFoundMsg("Plugin", preq.name()));
        }
    case PluginRequest::kListPluginRequest:
        {
            AutoTaskTracker task_tracker(false, true);
            const auto& preq = req.list_plugin_request();
            BEG_AUDIT_LOG(user, req.graph(), lgraph::LogApiType::Plugin, true,
                          FMA_FMT("List plugin"));

            std::vector<lgraph::PluginDesc> r = db->ListPlugins(type, user);

            return RespondSuccess(resp);
        }
    case PluginRequest::kCallPluginRequest:
        {
            const auto& preq = req.call_plugin_request();
            bool is_write;
            if (lgraph_req->has_is_write_op()) {
                is_write = lgraph_req->is_write_op();
            } else {
                bool r = db->IsReadOnlyPlugin(type, user, preq.name());
                is_write = !r;
            }
            AutoTaskTracker task_tracker(
                (type == PluginManager::PluginType::CPP ? "[CPP_PLUGIN] " : "[PYTHON_PLUGIN] ") +
                    preq.name(),
                true, is_write);
            BEG_AUDIT_LOG(user, req.graph(), lgraph::LogApiType::Plugin,
                                          is_write, req.DebugString());
            FMA_DBG_STREAM(logger_) << "Calling plugin " << preq.name() << " with param "
                                    << preq.param() << " timeout " << preq.timeout();
            TimeoutTaskKiller timeout_killer;
            if (preq.has_timeout() && preq.timeout() != 0) {
                timeout_killer.SetTimeout(preq.timeout());
            }
            bool r = db->CallPlugin(nullptr, type, user, preq.name(), preq.param(),
                                   preq.timeout(), preq.in_process(),
                                   *presp->mutable_call_plugin_response()->mutable_reply());
            FMA_DBG_ASSERT(r);
            return RespondSuccess(resp);
        }
    default:
        {
            BEG_AUDIT_LOG(user, req.graph(), lgraph::LogApiType::Plugin, true,
                                          req.DebugString());
            FMA_WARN_STREAM(logger_) << "Unhandled request type: " << req.Req_case();
            return RespondBadInput(resp,
                                   FMA_FMT("Unhandled plugin request type [{}].", req.Req_case()));
        }
    }
}
