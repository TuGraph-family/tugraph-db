/**
 * Copyright 2023 AntGroup CO., Ltd.
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

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "core/global_config.h"
#include "http/http_server.h"
#include "http/http_utils.h"
#include "http/import_task.h"
#include "protobuf/ha.pb.h"
#include "server/json_convert.h"
#include "fma-common/string_formatter.h"
#include "fma-common/file_system.h"

namespace lgraph {
namespace http {

#define _GET_PLUGIN_TYPE(procedureType, _type)                           \
    do {                                                                 \
        if (procedureType == plugin::PLUGIN_LANG_TYPE_CPP)               \
            _type = PluginManager::PluginType::CPP;                      \
        else if (procedureType == plugin::PLUGIN_LANG_TYPE_PYTHON)       \
            _type = PluginManager::PluginType::PYTHON;                   \
        else if (procedureType == plugin::PLUGIN_LANG_TYPE_ANY)          \
            _type = PluginManager::PluginType::ANY;                      \
        else                                                             \
            throw lgraph_api::BadRequestException(                       \
                FMA_FMT("Invalid procedure type [{}].", procedureType)); \
    } while (0)

#define _GET_PLUGIN_REQUEST_TYPE(procedureType, _type)                   \
    do {                                                                 \
        if (procedureType == plugin::PLUGIN_LANG_TYPE_CPP)               \
            _type = PluginRequest::CPP;                                  \
        else if (procedureType == plugin::PLUGIN_LANG_TYPE_PYTHON)       \
            _type = PluginRequest::PYTHON;                               \
        else if (procedureType == plugin::PLUGIN_LANG_TYPE_ANY)          \
            _type = PluginRequest::ANY;                                  \
        else                                                             \
            throw lgraph_api::BadRequestException(                       \
                FMA_FMT("Invalid procedure type [{}].", procedureType)); \
    } while (0)

#define _GET_PLUGIN_REQUEST_CODE_TYPE(codeType, _type)                                           \
    do {                                                                                         \
        if (codeType == plugin::PLUGIN_CODE_TYPE_PY)                                             \
            _type = LoadPluginRequest::PY;                                                       \
        else if (codeType == plugin::PLUGIN_CODE_TYPE_SO)                                        \
            _type = LoadPluginRequest::SO;                                                       \
        else if (codeType == plugin::PLUGIN_CODE_TYPE_ZIP)                                       \
            _type = LoadPluginRequest::ZIP;                                                      \
        else if (codeType == plugin::PLUGIN_CODE_TYPE_CPP)                                       \
            _type = LoadPluginRequest::CPP;                                                      \
        else                                                                                     \
            throw lgraph_api::BadRequestException(FMA_FMT("Invalid code_type [{}].", codeType)); \
    } while (0)

#define _HANDLE_LGRAPH_RESPONSE_ERROR(lgraph_resp)                         \
    do {                                                                   \
        switch (lgraph_resp.error_code()) {                                \
        case LGraphResponse::AUTH_ERROR:                                   \
            throw lgraph_api::UnauthorizedError(lgraph_resp.error());      \
        case LGraphResponse::BAD_REQUEST:                                  \
            throw lgraph_api::BadRequestException(lgraph_resp.error());    \
        case LGraphResponse::REDIRECT:                                     \
            return;                                                        \
        case LGraphResponse::EXCEPTION:                                    \
            throw lgraph_api::InternalErrorException(lgraph_resp.error()); \
        case LGraphResponse::KILLED:                                       \
            throw lgraph_api::BadRequestException("Task killed.");         \
        default:                                                           \
            throw lgraph_api::InternalErrorException(lgraph_resp.error()); \
        }                                                                  \
    } while (0)

nlohmann::json ProtoFieldDataToJson(const ProtoFieldData& data) {
    switch (data.Data_case()) {
    case ProtoFieldData::DATA_NOT_SET:
        return nlohmann::json();
    case ProtoFieldData::kBoolean:
        return nlohmann::json(data.boolean());
    case ProtoFieldData::kInt8:
        return nlohmann::json(data.int8_());
    case ProtoFieldData::kInt16:
        return nlohmann::json(data.int16_());
    case ProtoFieldData::kInt32:
        return nlohmann::json(data.int32_());
    case ProtoFieldData::kInt64:
        return nlohmann::json(data.int64_());
    case ProtoFieldData::kSp:
        return nlohmann::json(data.sp());
    case ProtoFieldData::kDp:
        return nlohmann::json(data.dp());
    case ProtoFieldData::kDate:
        return nlohmann::json(Date(data.date()).ToString());
    case ProtoFieldData::kDatetime:
        return nlohmann::json(DateTime(data.datetime()).ToString());
    case ProtoFieldData::kStr:
        return nlohmann::json(data.str());
    case ProtoFieldData::kBlob:
        return nlohmann::json(::lgraph_api::base64::Encode(data.blob()));
    }
    FMA_ASSERT(false);
    return nlohmann::json();
}

HttpService::HttpService(StateMachine* state_machine)
    : sm_(state_machine), pool_(1), serial_number_(0) {}

void HttpService::Start(lgraph::GlobalConfig* config) {
    InitFuncMap();
    resource_dir_ = config->http_web_dir;
    galaxy_ = sm_->GetGalaxy();
    import_manager_.Init(config);
}

void HttpService::InitFuncMap() {
    functions_map_.clear();
    functions_map_.emplace(HTTP_CYPHER_METHOD,
                           std::bind(&HttpService::DoCypherRequest, this, std::placeholders::_1,
                                     std::placeholders::_2));
    functions_map_.emplace(HTTP_GQL_METHOD,
                           std::bind(&HttpService::DoGqlRequest, this, std::placeholders::_1,
                                     std::placeholders::_2));
    functions_map_.emplace(HTTP_REFRESH_METHOD,
                           std::bind(&HttpService::DoRefreshRequest, this, std::placeholders::_1,
                                     std::placeholders::_2));
    functions_map_.emplace(HTTP_LOGIN_METHOD,
                           std::bind(&HttpService::DoLoginRequest, this, std::placeholders::_1,
                                     std::placeholders::_2));
    functions_map_.emplace(HTTP_LOGOUT_METHOD,
                           std::bind(&HttpService::DoLogoutRequest, this, std::placeholders::_1,
                                     std::placeholders::_2));
    functions_map_.emplace(HTTP_UPLOAD_METHOD,
                           std::bind(&HttpService::DoUploadRequest, this, std::placeholders::_1,
                                     std::placeholders::_2));
    functions_map_.emplace(
        HTTP_CLEAR_CACHE_METHOD,
        std::bind(&HttpService::DoClearCache, this, std::placeholders::_1, std::placeholders::_2));
    functions_map_.emplace(
        HTTP_CHECK_FILE_METHOD,
        std::bind(&HttpService::DoCheckFile, this, std::placeholders::_1, std::placeholders::_2));
    functions_map_.emplace(
        HTTP_IMPORT_METHOD,
        std::bind(&HttpService::DoImportFile, this, std::placeholders::_1, std::placeholders::_2));
    functions_map_.emplace(HTTP_IMPORT_PROGRESS_METHOD,
                           std::bind(&HttpService::DoImportProgress, this, std::placeholders::_1,
                                     std::placeholders::_2));
    functions_map_.emplace(HTTP_IMPORT_SCHEMA_METHOD,
                           std::bind(&HttpService::DoImportSchema, this, std::placeholders::_1,
                                     std::placeholders::_2));
    functions_map_.emplace(HTTP_UPLOAD_PROCEDURE_METHOD,
                           std::bind(&HttpService::DoUploadProcedure, this, std::placeholders::_1,
                                     std::placeholders::_2));
    functions_map_.emplace(HTTP_LIST_PROCEDURE_METHOD,
                           std::bind(&HttpService::DoListProcedures, this, std::placeholders::_1,
                                     std::placeholders::_2));
    functions_map_.emplace(HTTP_GET_PROCEDURE_METHOD,
                           std::bind(&HttpService::DoGetProcedure, this, std::placeholders::_1,
                                     std::placeholders::_2));
    functions_map_.emplace(HTTP_GET_PROCEDURE_DEMO_METHOD,
                           std::bind(&HttpService::DoGetProcedureDemo, this, std::placeholders::_1,
                                     std::placeholders::_2));
    functions_map_.emplace(HTTP_DELETE_PROCEDURE_METHOD,
                           std::bind(&HttpService::DoDeleteProcedure, this, std::placeholders::_1,
                                     std::placeholders::_2));
    functions_map_.emplace(HTTP_CALL_PROCEDURE_METHOD,
                           std::bind(&HttpService::DoCallProcedure, this, std::placeholders::_1,
                                     std::placeholders::_2));
}

void HttpService::Query(google::protobuf::RpcController* cntl_base, const HttpRequest*,
                        HttpResponse*, google::protobuf::Closure* done) {
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);
    AddAccessControlCORS(cntl);
    std::string method = cntl->http_request().unresolved_path();
    std::transform(method.begin(), method.end(), method.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    std::string res;
    try {
        auto it = functions_map_.find(method);
        if (it == functions_map_.end()) throw lgraph_api::BadRequestException("Unsupported method");
        it->second(cntl, res);
    } catch (const lgraph_api::UnauthorizedError& e) {
        return RespondUnauthorized(cntl, e.what());
    } catch (const lgraph_api::BadRequestException& e) {
        return RespondBadRequest(cntl, e.what());
    } catch (const lgraph_api::InternalErrorException& e) {
        return RespondInternalError(cntl, e.what());
    } catch (const std::exception& e) {
        return RespondBadRequest(cntl, e.what());
    }
    return RespondSuccess(cntl, res);
}

void HttpService::DoUploadRequest(const brpc::Controller* cntl, std::string& res) {
    const std::string* token = cntl->http_request().GetHeader(HTTP_AUTHORIZATION);

    if (token == nullptr) throw lgraph_api::UnauthorizedError();
    if (!galaxy_->JudgeRefreshTime(*token))
        throw lgraph_api::UnauthorizedError("token has already expire");

    const std::string* file_name = cntl->http_request().GetHeader(HTTP_HEADER_FILE_NAME);
    const std::string* begin_str = cntl->http_request().GetHeader(HTTP_HEADER_BEGIN_POS);
    const std::string* size_str = cntl->http_request().GetHeader(HTTP_HEADER_SIZE);
    if (file_name == nullptr || begin_str == nullptr || size_str == nullptr) {
        throw lgraph_api::BadRequestException(
            "request header should has a fileName, "
            "a beginPos and a size parameter");
    }
    off_t begin;
    ssize_t size;
    try {
        begin = boost::lexical_cast<off_t>(*begin_str);
        size = boost::lexical_cast<ssize_t>(*size_str);
    } catch (boost::bad_lexical_cast& e) {
        throw lgraph_api::BadRequestException(
            "beginPos and Size should be an integer of the string type");
    }
    int fd = OpenUserFile(*token, *file_name);
    butil::IOBuf content = cntl->request_attachment();
    ssize_t writed_bytes = 0;
    while (!content.empty()) {
        ssize_t ret = content.pcut_into_file_descriptor(fd, begin, size);
        if (ret < 0) {
            throw lgraph_api::InternalErrorException(FMA_FMT("{} write failed", *file_name));
        }
        begin += ret;
        writed_bytes += ret;
    }
    if (writed_bytes != size) {
        throw lgraph_api::InputError("Size and content are not equal ");
    }
}

void HttpService::DoClearCache(const brpc::Controller* cntl, std::string& res) {
    const std::string* token = cntl->http_request().GetHeader(HTTP_AUTHORIZATION);
    if (token == nullptr) throw lgraph_api::UnauthorizedError();
    if (!galaxy_->JudgeRefreshTime(*token))
        throw lgraph_api::UnauthorizedError("token has already expire");

    std::string req = cntl->request_attachment().to_string();
    std::string flag_str;
    GET_FIELD_OR_THROW_BAD_REQUEST(req, std::string, HTTP_FLAG, flag_str);
    int16_t flag = -1;
    try {
        flag = boost::lexical_cast<int16_t>(flag_str);
    } catch (boost::bad_lexical_cast& e) {
        throw lgraph_api::BadRequestException("`flag` should be an integer of the string type");
    }
    if (flag < 0)
        throw lgraph_api::BadRequestException("`flag` should be greater than or equal to 0");
    switch (flag) {
    case HTTP_SPECIFIED_FILE:
        {
            std::string file_name;
            GET_FIELD_OR_THROW_BAD_REQUEST(req, std::string, HTTP_FILE_NAME, file_name);
            return DeleteSpecifiedFile(*token, file_name);
        }
    case HTTP_SPECIFIED_USER:
        {
            std::string user_name;
            GET_FIELD_OR_THROW_BAD_REQUEST(req, std::string, HTTP_USER_NAME, user_name);
            return DeleteSpecifiedUserFiles(*token, user_name);
        }
    case HTTP_ALL_USER:
        {
            return DeleteAllUserFiles(*token);
        }
    }
}

void HttpService::DoCheckFile(const brpc::Controller* cntl, std::string& res) {
    const std::string* token = cntl->http_request().GetHeader(HTTP_AUTHORIZATION);
    if (token == nullptr) throw lgraph_api::UnauthorizedError();
    if (!galaxy_->JudgeRefreshTime(*token))
        throw lgraph_api::UnauthorizedError("token has already expire");

    std::string req = cntl->request_attachment().to_string();

    std::string flag_str;
    GET_FIELD_OR_THROW_BAD_REQUEST(req, std::string, HTTP_FLAG, flag_str);
    int16_t flag = -1;
    try {
        flag = boost::lexical_cast<int16_t>(flag_str);
    } catch (boost::bad_lexical_cast& e) {
        throw lgraph_api::BadRequestException("`flag` should be an integer of the string type");
    }
    if (flag < 0)
        throw lgraph_api::BadRequestException("`flag` should be greater than or equal to 0");

    std::string file_name;
    GET_FIELD_OR_THROW_BAD_REQUEST(req, std::string, HTTP_FILE_NAME, file_name);

    const std::string user = galaxy_->ParseAndValidateToken(*token);
    std::string absolute_file_name = import_manager_.GetUserPath(user) + "/" + file_name;
    nlohmann::json js;
    switch (flag) {
    case 1:
        {
            std::string md5sum;
            GET_FIELD_OR_THROW_BAD_REQUEST(req, std::string, "checkSum", md5sum);
            // TODO(jzj) : implement md5 checksum from file
        }
    case 2:
        {
            std::string size_str;
            GET_FIELD_OR_THROW_BAD_REQUEST(req, std::string, "fileSize", size_str);
            off_t file_size;
            try {
                file_size = boost::lexical_cast<off_t>(size_str);
            } catch (boost::bad_lexical_cast& e) {
                throw lgraph_api::BadRequestException(
                    "`file_size` should be an integer of the string type");
            }
            if (file_size != GetFileSize(absolute_file_name)) {
                js["pass"] = false;
            } else {
                js["pass"] = true;
            }
        }
    }
    res = js.dump();
}

void HttpService::DoImportFile(const brpc::Controller* cntl, std::string& res) {
    const std::string* token = cntl->http_request().GetHeader(HTTP_AUTHORIZATION);
    if (token == nullptr) throw lgraph_api::UnauthorizedError();
    if (!galaxy_->JudgeRefreshTime(*token))
        throw lgraph_api::UnauthorizedError("token has already expire");

    std::string req = cntl->request_attachment().to_string();
    std::string graph;
    GET_FIELD_OR_THROW_BAD_REQUEST(req, std::string, HTTP_GRAPH, graph);

    std::string user = galaxy_->ParseAndValidateToken(*token);
    nlohmann::json schema;
    AdjustFilePath(req, user, schema);

    std::string delimiter;
    GET_FIELD_OR_THROW_BAD_REQUEST(req, std::string, HTTP_DELIMITER, delimiter);

    bool continue_on_error = false;
    ExtractTypedField<bool>(req, HTTP_CONTINUE_ON_ERROR, continue_on_error);

    std::string skip_str;
    ExtractTypedField<std::string>(req, HTTP_SKIP_PACKAGES, skip_str);
    uint64_t skip_packages = 0;
    if (!skip_str.empty()) {
        try {
            skip_packages = boost::lexical_cast<uint64_t>(skip_str);
        } catch (boost::bad_lexical_cast& e) {
            throw lgraph_api::BadRequestException(
                "skipPackages should be an integer of the string type");
        }
    }

    std::string id;
    ExtractTypedField<std::string>(req, HTTP_TASK_ID, id);
    id = id.empty() ? GetRandomUuid() : id;

    ImportTask task(this, &import_manager_, id, user, *token, graph, delimiter, continue_on_error,
                    skip_packages, schema);
    pool_.PushTask(0, GetSerialNumber(), task);
    nlohmann::json js;
    js[HTTP_TASK_ID] = id;
    res = js.dump();
}

void HttpService::DoImportSchema(const brpc::Controller* cntl, std::string& res) {
    const std::string* token = cntl->http_request().GetHeader(HTTP_AUTHORIZATION);
    if (token == nullptr) throw lgraph_api::UnauthorizedError();
    if (!galaxy_->JudgeRefreshTime(*token))
        throw lgraph_api::UnauthorizedError("token has already expire");

    std::string req = cntl->request_attachment().to_string();
    std::string graph;
    GET_FIELD_OR_THROW_BAD_REQUEST(req, std::string, HTTP_GRAPH, graph);
    nlohmann::json desc;
    GET_FIELD_OR_THROW_BAD_REQUEST(req, nlohmann::json, HTTP_DESCRIPTION, desc);
    LGraphRequest pb_req;
    ProcessSchemaRequest(graph, desc.dump(), *token, pb_req);
}

void HttpService::DoImportProgress(const brpc::Controller* cntl, std::string& res) {
    const std::string* token = cntl->http_request().GetHeader(HTTP_AUTHORIZATION);
    if (token == nullptr) throw lgraph_api::UnauthorizedError();
    if (!galaxy_->JudgeRefreshTime(*token))
        throw lgraph_api::UnauthorizedError("token has already expire");

    std::string req = cntl->request_attachment().to_string();
    std::string task_id;
    GET_FIELD_OR_THROW_BAD_REQUEST(req, std::string, HTTP_TASK_ID, task_id);
    std::string reason, progress;
    int state = import_manager_.GetImportProgress(task_id, progress, reason);
    nlohmann::json js;
    switch (state) {
    case 0:
        {
            js[HTTP_PROGRESS_STATE] = "0";
            break;
        }
    case 1:
        {
            js[HTTP_PROGRESS_STATE] = "1";
            js[HTTP_PROGRESS] = progress;
            break;
        }
    case 2:
        {
            js[HTTP_PROGRESS_STATE] = "2";
            break;
        }
    case 3:
        {
            js[HTTP_PROGRESS_STATE] = "3";
            js[HTTP_REASON] = reason;
            break;
        }
    }
    res = js.dump();
}

void HttpService::DoCypherRequest(const brpc::Controller* cntl, std::string& res) {
    const std::string token = CheckTokenOrThrowException(cntl);
    LGraphRequest pb_req;
    BuildPbGraphQueryRequest(cntl, lgraph_api::GraphQueryType::CYPHER, token, pb_req);
    LGraphResponse pb_res;
    ApplyToStateMachine(pb_req, pb_res);
    BuildJsonGraphQueryResponse(pb_res, res);
}

void HttpService::DoGqlRequest(const brpc::Controller* cntl, std::string& res) {
    const std::string token = CheckTokenOrThrowException(cntl);
    LGraphRequest pb_req;
    BuildPbGraphQueryRequest(cntl, lgraph_api::GraphQueryType::GQL, token, pb_req);
    LGraphResponse pb_res;
    ApplyToStateMachine(pb_req, pb_res);
    BuildJsonGraphQueryResponse(pb_res, res);
}

void HttpService::DoLoginRequest(const brpc::Controller* cntl, std::string& res) {
    std::string req = cntl->request_attachment().to_string();
    std::string username, password;
    GET_FIELD_OR_THROW_BAD_REQUEST(req, std::string, HTTP_USER_NAME, username);
    GET_FIELD_OR_THROW_BAD_REQUEST(req, std::string, HTTP_PASSWORD, password);
    _HoldReadLock(galaxy_->GetReloadLock());
    std::string token = galaxy_->GetUserToken(username, password);
    if ((fabs(galaxy_->retry_login_time - 0.0) < std::numeric_limits<double>::epsilon()) ||
        fma_common::GetTime() - galaxy_->retry_login_time >= RETRY_WAIT_TIME) {
        if ((fabs(galaxy_->retry_login_time - 0.0) >= std::numeric_limits<double>::epsilon())) {
            galaxy_->login_failed_times_.erase(username);
            galaxy_->retry_login_time = 0.0;
        }
        if (token.empty()) {
            if (galaxy_->login_failed_times_.find(username) != galaxy_->login_failed_times_.end()) {
                galaxy_->login_failed_times_[username]++;
            } else {
                galaxy_->login_failed_times_[username] = 1;
            }
            if (galaxy_->login_failed_times_[username] >= MAX_LOGIN_FAILED_TIMES) {
                galaxy_->retry_login_time = fma_common::GetTime();
            }
            throw lgraph_api::BadRequestException(FMA_FMT("userName {} is invalid", username));
        }
    } else {
        throw lgraph_api::BadRequestException(
            "Too many login failures, please try again in a minute");
    }
    nlohmann::json js;
    js[HTTP_AUTHORIZATION] = token;
    res = js.dump();
}

void HttpService::DoLogoutRequest(const brpc::Controller* cntl, std::string& res) {
    const std::string* token = cntl->http_request().GetHeader(HTTP_AUTHORIZATION);
    if (token == nullptr) throw lgraph_api::UnauthorizedError();
    _HoldReadLock(galaxy_->GetReloadLock());
    if (!galaxy_->UnBindTokenUser(*token)) {
        throw lgraph_api::UnauthorizedError();
    }
}

void HttpService::DoRefreshRequest(const brpc::Controller* cntl, std::string& res) {
    const std::string* token = cntl->http_request().GetHeader(HTTP_AUTHORIZATION);
    if (token == nullptr) throw lgraph_api::UnauthorizedError();
    _HoldReadLock(galaxy_->GetReloadLock());
    std::string user = galaxy_->ParseAndValidateToken(*token);
    std::string new_token = galaxy_->RefreshUserToken(*token, user);
    nlohmann::json js;
    js[HTTP_AUTHORIZATION] = new_token;
    res = js.dump();
}

void HttpService::BuildPbGraphQueryRequest(const brpc::Controller* cntl,
                                           const lgraph_api::GraphQueryType& query_type,
                                           const std::string& token, LGraphRequest& pb) {
    std::string req = cntl->request_attachment().to_string();
    std::string graph;
    std::string query;
    bool json_format = true;
    double timeout = 0;
    GET_FIELD_OR_THROW_BAD_REQUEST(req, std::string, HTTP_GRAPH, graph);
    GET_FIELD_OR_THROW_BAD_REQUEST(req, std::string, HTTP_SCRIPT, query);
    ExtractTypedField<bool>(req, HTTP_JSON_FORMAT, json_format);
    ExtractTypedField<double>(req, HTTP_TIMEOUT, timeout);
    const std::string user = galaxy_->ParseAndValidateToken(token);
    pb.set_token(token);
    auto field_access = galaxy_->GetRoleFieldAccessLevel(user, graph);
    cypher::RTContext ctx(sm_, galaxy_, token, user, graph, field_access);
    std::string name;
    std::string type;
    bool ret = cypher::Scheduler::DetermineReadOnly(&ctx, query_type, query,
                                                    name, type);
    if (name.empty() || type.empty()) {
        pb.set_is_write_op(!ret);
    } else {
        type.erase(remove(type.begin(), type.end(), '\"'), type.end());
        name.erase(remove(name.begin(), name.end(), '\"'), name.end());
        AccessControlledDB db = galaxy_->OpenGraph(user, graph);
        ret = !db.IsReadOnlyPlugin(
            type == "CPP" ? PluginManager::PluginType::CPP : PluginManager::PluginType::PYTHON,
            token, name);
        pb.set_is_write_op(!ret);
    }
    GraphQueryRequest* creq = pb.mutable_graph_query_request();
    creq->set_type(convert::FromLGraphT(query_type));
    creq->set_result_in_json_format(false);
    creq->set_query(query);
    creq->set_graph(graph);
    creq->set_timeout(timeout);
    creq->set_result_in_json_format(json_format);
}

void HttpService::BuildJsonGraphQueryResponse(LGraphResponse& res, std::string& json) {
    if (res.error_code() != LGraphResponse::SUCCESS) {
        _HANDLE_LGRAPH_RESPONSE_ERROR(res);
    } else {
        const auto& resp = res.graph_query_response();
        if (resp.Result_case() == GraphQueryResponse::kJsonResult) {
            nlohmann::json response;
            response[HTTP_RESULT] = nlohmann::json::parse(resp.json_result());
            json = response.dump();
            return;
        } else if (resp.Result_case() == GraphQueryResponse::kBinaryResult) {
            std::vector<nlohmann::json> vec_header;
            for (auto& c : resp.binary_result().header()) {
                nlohmann::json col;
                col[HTTP_NAME] = c.name();
                col[HTTP_TYPE] = c.type();
                vec_header.emplace_back(col);
            }
            std::vector<nlohmann::json> vec_result;
            for (auto& r : resp.binary_result().result()) {
                std::vector<nlohmann::json> record;
                for (auto& v : r.values()) {
                    record.emplace_back(ProtoFieldDataToJson(v));
                }
                vec_result.emplace_back(nlohmann::json(record));
            }
            nlohmann::json response;
            response[HTTP_HEADER] = nlohmann::json(vec_header);
            response[HTTP_RESULT] = nlohmann::json(vec_result);
            response[HTTP_SIZE] = nlohmann::json(vec_result.size());
            response[HTTP_ELAPSED] = nlohmann::json(resp.binary_result().elapsed());
            json = response.dump();
        }
    }
}

void HttpService::ProcessSchemaRequest(const std::string& graph, const std::string& desc,
                                       const std::string& token, LGraphRequest& req) {
    req.set_is_write_op(true);
    req.set_token(token);
    SchemaRequest* schema = req.mutable_schema_request();
    schema->set_graph(graph);
    schema->set_description(desc);
    LGraphResponse res;
    ApplyToStateMachine(req, res);

    if (res.error_code() != LGraphResponse::SUCCESS) {
        _HANDLE_LGRAPH_RESPONSE_ERROR(res);
    } else {
        if (res.schema_response().has_error_message()) {
            throw lgraph_api::BadRequestException(res.schema_response().error_message());
        }
    }
}

int HttpService::OpenUserFile(const std::string& token, std::string file_name) {
    const std::string& user = galaxy_->ParseAndValidateToken(token);
    if (file_name.empty() || file_name[0] == '/')
        throw lgraph_api::BadRequestException("File-Name is not a relative path");
    boost::algorithm::trim(file_name);
    if (file_name.size() > 2 && file_name[0] == '.' && file_name[1] == '/') {
        file_name = file_name.substr(2, file_name.size() - 2);
    }
    std::vector<std::string> fields;
    fields.reserve(10);
    boost::split(fields, file_name, boost::is_any_of("/"));
    if (user == fields[0])
        throw lgraph_api::BadRequestException(FMA_FMT("invalid fileName : {}", file_name));
    fma_common::LocalFileSystem fs;
    std::string user_directory = import_manager_.GetUserPath(user);
    for (size_t idx = 0; idx < fields.size() - 1; ++idx) {
        user_directory += "/";
        user_directory += fields[idx];
    }
    if (fs.FileExists(user_directory)) {
        if (!fs.IsDir(user_directory)) {
            throw lgraph_api::InternalErrorException(
                FMA_FMT("{} already exists and it is not a directory", user_directory));
        }
    } else {
        fs.Mkdir(user_directory);
    }
    const std::string absolute_file_name = user_directory + "/" + fields[fields.size() - 1];
    int fd = open(absolute_file_name.data(), O_CREAT | O_WRONLY, 0666);
    if (fd < 0) {
        throw lgraph_api::InternalErrorException(
            FMA_FMT("{} file open failed", absolute_file_name));
    }
    return fd;
}

void HttpService::AdjustFilePath(const std::string& req, const std::string& user,
                                 nlohmann::json& schema) {
    GET_FIELD_OR_THROW_BAD_REQUEST(req, nlohmann::json, HTTP_SCHEMA, schema);
    nlohmann::json& files = schema.at(HTTP_FILES);
    for (size_t idx = 0; idx < files.size(); ++idx) {
        nlohmann::json& item = files[idx];
        if (!item.contains(HTTP_PATH))
            throw InputError(FMA_FMT(R"(Missing "path" in json {})", schema.dump()));
        std::string path = item[HTTP_PATH].get<std::string>();
        std::string absolute_file_name = import_manager_.GetUserPath(user) + "/" + path;
        item[HTTP_PATH] = absolute_file_name;
    }
}

void HttpService::DeleteSpecifiedFile(const std::string& token, const std::string& file_name) {
    const std::string user = galaxy_->ParseAndValidateToken(token);
    const std::string user_directory = import_manager_.GetUserPath(user);
    const std::string absolute_file_name = user_directory + "/" + file_name;

    fma_common::LocalFileSystem fs;
    if (!fs.FileExists(absolute_file_name)) {
        throw lgraph_api::BadRequestException(FMA_FMT("{} not exists", file_name));
    }
    if (fs.IsDir(absolute_file_name)) {
        throw lgraph_api::BadRequestException(FMA_FMT("{} is a directory", file_name));
    }
    if (!fs.Remove(absolute_file_name)) {
        throw lgraph_api::InternalErrorException(FMA_FMT("{} file remove failed", file_name));
    }
}

void HttpService::DeleteSpecifiedUserFiles(const std::string& token, const std::string& user_name) {
    const std::string user = galaxy_->ParseAndValidateToken(token);
    if (user != user_name || !galaxy_->IsAdmin(user)) {
        throw lgraph_api::BadRequestException(FMA_FMT(
            "{} is not admin user, can't remove other people's file directories", user_name));
    }
    const std::string user_directory = import_manager_.GetUserPath(user);

    fma_common::LocalFileSystem fs;
    if (!fs.FileExists(user_directory)) {
        throw lgraph_api::BadRequestException(FMA_FMT("{} not exists", user_name));
    }

    if (!fs.IsDir(user_directory)) {
        throw lgraph_api::BadRequestException(FMA_FMT("{} is not a directory", user_name));
    }

    if (!fs.RemoveDir(user_directory)) {
        throw lgraph_api::InternalErrorException(FMA_FMT("{} directory remove failed", user_name));
    }
}

void HttpService::DeleteAllUserFiles(const std::string& token) {
    bool is_admin = false;
    std::string user_name;
    user_name = galaxy_->ParseTokenAndCheckIfIsAdmin(token, &is_admin);
    if (!is_admin) {
        throw lgraph_api::BadRequestException(
            FMA_FMT("{} is not admin user, can't remove all directories", user_name));
    }
    const std::string all_directory = import_manager_.GetRootPath();

    fma_common::LocalFileSystem fs;
    if (!fs.FileExists(all_directory)) {
        throw lgraph_api::BadRequestException(FMA_FMT("{} not exists", HTTP_FILE_DIRECTORY));
    }

    if (!fs.IsDir(all_directory)) {
        throw lgraph_api::BadRequestException(
            FMA_FMT("{} is not a directory", HTTP_FILE_DIRECTORY));
    }
    if (!fs.RemoveDir(all_directory)) {
        throw lgraph_api::InternalErrorException(
            FMA_FMT("{} directory remove failed", HTTP_FILE_DIRECTORY));
    }
}

off_t HttpService::GetFileSize(const std::string& file_name) {
    struct stat st;
    if (stat(file_name.data(), &st) < 0) {
        throw lgraph_api::BadRequestException(strerror(errno));
    }
    return st.st_size;
}

void HttpService::DoUploadProcedure(const brpc::Controller* cntl, std::string& res) {
    std::string token = CheckTokenOrThrowException(cntl);

    LGraphRequest lgraph_req;
    lgraph_req.set_token(token);
    lgraph_req.set_is_write_op(true);
    std::string params = cntl->request_attachment().to_string();
    std::string graphName, procedureType, version;
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "graphName", graphName);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "procedureType", procedureType);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "version", version);
    if (version != plugin::PLUGIN_VERSION_1 && version != plugin::PLUGIN_VERSION_2) {
        throw lgraph_api::BadRequestException(FMA_FMT(
            "Version must be [{}] or [{}]", plugin::PLUGIN_VERSION_1, plugin::PLUGIN_VERSION_2));
    }
    PluginRequest* preq = lgraph_req.mutable_plugin_request();
    preq->set_graph(graphName);
    preq->set_version(version);
    lgraph::PluginRequest::PluginType type;
    _GET_PLUGIN_REQUEST_TYPE(procedureType, type);
    if (type == PluginRequest::ANY) {
        throw lgraph_api::BadRequestException(
            FMA_FMT("Uploaded procedure can not be of type [{}]", type));
    }
    preq->set_type(type);

    std::string procedureName, content, description, codeType;
    bool readonly;
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "procedureName", procedureName);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "content", content);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "description", description);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "codeType", codeType);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, bool, "readonly", readonly);

    LoadPluginRequest* req = preq->mutable_load_plugin_request();
    req->set_name(procedureName);
    req->set_read_only(readonly);
    std::vector<unsigned char> decoded = utility::conversions::from_base64(content);
    req->set_code(std::string(decoded.begin(), decoded.end()));
    req->set_desc(description);
    lgraph::LoadPluginRequest::CodeType _codeType;
    _GET_PLUGIN_REQUEST_CODE_TYPE(codeType, _codeType);
    req->set_code_type(_codeType);

    LGraphResponse lgraph_resp;
    ApplyToStateMachine(lgraph_req, lgraph_resp);
    if (lgraph_resp.error_code() != LGraphResponse::SUCCESS) {
        _HANDLE_LGRAPH_RESPONSE_ERROR(lgraph_resp);
    }
}

void HttpService::DoListProcedures(const brpc::Controller* cntl, std::string& res) {
    std::string token = CheckTokenOrThrowException(cntl);
    std::string params = cntl->request_attachment().to_string();
    std::string graphName, procedureType, version;
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "graphName", graphName);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "procedureType", procedureType);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "version", version);
    if (version != plugin::PLUGIN_VERSION_1 && version != plugin::PLUGIN_VERSION_2 &&
        version != plugin::PLUGIN_VERSION_ANY) {
        throw lgraph_api::BadRequestException(
            FMA_FMT("Version must be [{}] or [{}] or [{}]", plugin::PLUGIN_VERSION_1,
                    plugin::PLUGIN_VERSION_2, plugin::PLUGIN_VERSION_ANY));
    }

    LGraphRequest lgraph_req;
    lgraph_req.set_token(token);
    lgraph_req.set_is_write_op(false);
    PluginRequest* preq = lgraph_req.mutable_plugin_request();
    preq->set_graph(graphName);
    lgraph::PluginRequest::PluginType type;
    _GET_PLUGIN_REQUEST_TYPE(procedureType, type);
    preq->set_type(type);
    preq->set_version(version);
    preq->mutable_list_plugin_request();

    LGraphResponse lgraph_resp;
    ApplyToStateMachine(lgraph_req, lgraph_resp);
    if (lgraph_resp.error_code() != LGraphResponse::SUCCESS) {
        _HANDLE_LGRAPH_RESPONSE_ERROR(lgraph_resp);
    } else {
        res = lgraph_resp.plugin_response().list_plugin_response().reply();
    }
}

void HttpService::DoGetProcedure(const brpc::Controller* cntl, std::string& res) {
    std::string token = CheckTokenOrThrowException(cntl);
    std::string params = cntl->request_attachment().to_string();
    std::string graphName, procedureName, procedureType;
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "graphName", graphName);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "procedureName", procedureName);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "procedureType", procedureType);
    PluginManager::PluginType type;
    _GET_PLUGIN_TYPE(procedureType, type);

    const std::string user = galaxy_->ParseAndValidateToken(token);
    AccessControlledDB db = galaxy_->OpenGraph(user, graphName);
    PluginCode co;
    bool exists = db.GetPluginCode(type, token, procedureName, co);
    if (!exists) {
        throw lgraph_api::BadRequestException("Plugin does not exist.");
    }
    std::string encoded = lgraph_api::base64::Encode(co.code);

    nlohmann::json js;
    js["name"] = co.name;
    js["description"] = co.desc;
    js["read_only"] = co.read_only;
    js["content"] = encoded;
    js["code_type"] = co.code_type;
    res = js.dump();
}

void HttpService::DoGetProcedureDemo(const brpc::Controller* cntl, std::string& res) {
    std::string token = CheckTokenOrThrowException(cntl);
    std::string params = cntl->request_attachment().to_string();
    std::string type;
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "type", type);

    std::string demo_path = resource_dir_ + HTTP_PROCEDURE_DEMO_PATH_DIRECTORY;
    std::string filename;
    if (type == "cpp_v1") {
        filename = HTTP_PROCEDURE_DEMO_PATH_CPP_V1;
        demo_path += HTTP_PROCEDURE_DEMO_PATH_CPP_V1;
    } else if (type == "cpp_v2") {
        filename = HTTP_PROCEDURE_DEMO_PATH_CPP_V2;
        demo_path += HTTP_PROCEDURE_DEMO_PATH_CPP_V2;
    } else if (type == "py") {
        filename = HTTP_PROCEDURE_DEMO_PATH_PYTHON;
        demo_path += HTTP_PROCEDURE_DEMO_PATH_PYTHON;
    } else {
        throw lgraph_api::BadRequestException(FMA_FMT("No such demo type [{}]", type));
    }

    std::ifstream ifs(demo_path);
    if (!ifs.is_open()) {
        throw lgraph_api::BadRequestException("Demo does not exist.");
    }
    std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    std::string encoded = lgraph_api::base64::Encode(content);

    nlohmann::json js;
    js["type"] = type;
    js["filename"] = filename;
    js["content"] = encoded;
    res = js.dump();
}

void HttpService::DoDeleteProcedure(const brpc::Controller* cntl, std::string& res) {
    std::string token = CheckTokenOrThrowException(cntl);
    std::string params = cntl->request_attachment().to_string();
    std::string graphName, procedureName, procedureType;
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "graphName", graphName);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "procedureName", procedureName);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "procedureType", procedureType);

    LGraphRequest lgraph_req;
    lgraph_req.set_token(token);
    lgraph_req.set_is_write_op(true);
    PluginRequest* preq = lgraph_req.mutable_plugin_request();
    preq->set_graph(graphName);
    lgraph::PluginRequest::PluginType type;
    _GET_PLUGIN_REQUEST_TYPE(procedureType, type);
    preq->set_type(type);
    preq->mutable_del_plugin_request()->set_name(procedureName);

    LGraphResponse lgraph_resp;
    ApplyToStateMachine(lgraph_req, lgraph_resp);
    if (lgraph_resp.error_code() != LGraphResponse::SUCCESS) {
        _HANDLE_LGRAPH_RESPONSE_ERROR(lgraph_resp);
    }
}

void HttpService::DoCallProcedure(const brpc::Controller* cntl, std::string& res) {
    const std::string token = CheckTokenOrThrowException(cntl);
    const std::string user = galaxy_->ParseAndValidateToken(token);

    std::string params = cntl->request_attachment().to_string();
    std::string graphName, procedureName, procedureType, version, procedureParam;
    double timeout = 0;
    bool inProcess = false;
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "graphName", graphName);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "procedureName", procedureName);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "procedureType", procedureType);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "version", version);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, std::string, "param", procedureParam);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, double, "timeout", timeout);
    GET_FIELD_OR_THROW_BAD_REQUEST(params, bool, "inProcess", inProcess);

    if (version == plugin::PLUGIN_VERSION_1) {
        // call plugin for plugin version 1
        LGraphRequest lgraph_req;
        lgraph_req.set_token(token);
        PluginRequest* preq = lgraph_req.mutable_plugin_request();
        preq->set_graph(graphName);
        lgraph::PluginRequest::PluginType type;
        _GET_PLUGIN_REQUEST_TYPE(procedureType, type);
        preq->set_type(type);
        CallPluginRequest* req = preq->mutable_call_plugin_request();
        req->set_name(procedureName);
        req->set_param(std::move(procedureParam));
        req->set_timeout(timeout);
        req->set_in_process(inProcess);

        LGraphResponse lgraph_resp;
        ApplyToStateMachine(lgraph_req, lgraph_resp);
        if (lgraph_resp.error_code() != LGraphResponse::SUCCESS) {
            _HANDLE_LGRAPH_RESPONSE_ERROR(lgraph_resp);
        }
        nlohmann::json body;
        body["result"] = lgraph_resp.plugin_response().call_plugin_response().reply();
        res = body.dump();
    } else if (version == plugin::PLUGIN_VERSION_2) {
        // call cypher for plugin version 2
        bool is_write_op = false;
        auto field_access = galaxy_->GetRoleFieldAccessLevel(user, graphName);
        cypher::RTContext ctx(sm_, galaxy_, token, user, graphName, field_access);
        std::string name, type;
        bool ret = cypher::Scheduler::DetermineReadOnly(&ctx, lgraph_api::GraphQueryType::CYPHER,
                                                        procedureParam, name, type);
        if (name.empty() || type.empty()) {
            is_write_op = !ret;
        } else {
            type.erase(remove(type.begin(), type.end(), '\"'), type.end());
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());
            AccessControlledDB db = galaxy_->OpenGraph(user, graphName);
            ret = !db.IsReadOnlyPlugin(
                type == "CPP" ? PluginManager::PluginType::CPP : PluginManager::PluginType::PYTHON,
                token, name);
            is_write_op = !ret;
        }

        LGraphRequest lgraph_req;
        lgraph_req.set_token(token);
        lgraph_req.set_is_write_op(is_write_op);
        GraphQueryRequest* creq = lgraph_req.mutable_graph_query_request();
        creq->set_type(lgraph::ProtoGraphQueryType::CYPHER);
        creq->set_query(procedureParam);
        creq->set_graph(graphName);
        creq->set_timeout(timeout);
        creq->set_result_in_json_format(true);

        LGraphResponse lgraph_res;
        ApplyToStateMachine(lgraph_req, lgraph_res);
        BuildJsonGraphQueryResponse(lgraph_res, res);
    } else {
        throw lgraph_api::BadRequestException(FMA_FMT(
            "Version must be [{}] or [{}]", plugin::PLUGIN_VERSION_1, plugin::PLUGIN_VERSION_2));
    }
}

void HttpService::ApplyToStateMachine(LGraphRequest& req, LGraphResponse& res) {
    DoneClosure d;
    sm_->HandleRequest(nullptr, &req, &res, &d);
    d.Wait();
}

void HttpService::AddAccessControlCORS(brpc::Controller* cntl) {
    cntl->http_response().set_content_type("application/json");
    cntl->http_response().SetHeader("Access-Control-Allow-Origin", "*");
    cntl->http_response().SetHeader("Access-Control-Allow-Headers",
                                    "Content-Type, Content-Length, Authorization, Accept,"
                                    " X-Requested-With, File-Name, Begin-Pos, Size,"
                                    " Content-Encoding");
    cntl->http_response().SetHeader("Access-Control-Allow-Methods",
                                    "POST, OPTIONS, PUT, GET, DELETE");
    cntl->http_response().SetHeader("Set-Cookie", "HttpOnly; Secure; SameSite=Strict");
}

void HttpService::RespondUnauthorized(brpc::Controller* cntl, const std::string& res) const {
    nlohmann::json js;
    js[HTTP_ERROR_CODE] = "401";
    js[HTTP_ERROR_MESSAGE] = res;
    cntl->response_attachment().append(js.dump());
    cntl->http_response().set_status_code(brpc::HTTP_STATUS_OK);
}

void HttpService::RespondSuccess(brpc::Controller* cntl, const std::string& res) const {
    nlohmann::json js;
    js[HTTP_ERROR_CODE] = "200";
    js[HTTP_ERROR_MESSAGE] = "";
    if (!res.empty()) {
        js[HTTP_DATA] = nlohmann::json::parse(res);
    } else {
        js[HTTP_DATA] = "";
    }
    cntl->response_attachment().append(js.dump());
    cntl->http_response().set_status_code(brpc::HTTP_STATUS_OK);
}

void HttpService::RespondInternalError(brpc::Controller* cntl, const std::string& res) const {
    nlohmann::json js;
    js[HTTP_ERROR_CODE] = "500";
    js[HTTP_ERROR_MESSAGE] = res;
    cntl->response_attachment().append(js.dump());
    cntl->http_response().set_status_code(brpc::HTTP_STATUS_OK);
}

void HttpService::RespondBadRequest(brpc::Controller* cntl, const std::string& res) const {
    nlohmann::json js;
    js[HTTP_ERROR_CODE] = "400";
    js[HTTP_ERROR_MESSAGE] = res;
    cntl->response_attachment().append(js.dump());
    cntl->http_response().set_status_code(brpc::HTTP_STATUS_OK);
}

std::string HttpService::CheckTokenOrThrowException(const brpc::Controller* cntl) const {
    const std::string* token = cntl->http_request().GetHeader(HTTP_AUTHORIZATION);
    if (token == nullptr) throw lgraph_api::UnauthorizedError();
    if (!galaxy_->JudgeRefreshTime(*token))
        throw lgraph_api::UnauthorizedError("token has already expire");
    return *token;
}

}  // end of namespace http
}  // end of namespace lgraph
