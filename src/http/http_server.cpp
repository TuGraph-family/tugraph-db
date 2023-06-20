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
#include "server/json_convert.h"
#include "fma-common/string_formatter.h"
#include "fma-common/file_system.h"

namespace lgraph {
namespace http {

const string_t HttpService::HTTP_CYPHER_METHOD = "cypher";
const uint16_t HttpService::HTTP_CYPHER_METHOD_IDX = 0;
const string_t HttpService::HTTP_REFRESH_METHOD = "refresh";
const uint16_t HttpService::HTTP_REFRESH_METHOD_IDX = 1;
const string_t HttpService::HTTP_LOGIN_METHOD = "login";
const uint16_t HttpService::HTTP_LOGIN_METHOD_IDX = 2;
const string_t HttpService::HTTP_LOGOUT_METHOD = "logout";
const uint16_t HttpService::HTTP_LOGOUT_METHOD_IDX = 3;
const string_t HttpService::HTTP_UPLOAD_METHOD = "upload_files";
const uint16_t HttpService::HTTP_UPLOAD_METHOD_IDX = 4;
const string_t HttpService::HTTP_CLEAR_CACHE_METHOD = "clear_cache";
const uint16_t HttpService::HTTP_CLEAR_CACHE_METHOD_IDX = 5;
const string_t HttpService::HTTP_CHECK_FILE_METHOD = "check_file";
const uint16_t HttpService::HTTP_CHECK_FILE_METHOD_IDX = 6;
const string_t HttpService::HTTP_IMPORT_METHOD = "import_data";
const uint16_t HttpService::HTTP_IMPORT_METHOD_IDX = 7;
const string_t HttpService::HTTP_IMPORT_PROGRESS_METHOD = "import_progress";
const uint16_t HttpService::HTTP_IMPORT_PROGRESS_METHOD_IDX = 8;
const string_t HttpService::HTTP_IMPORT_SCHEMA_METHOD = "import_schema";
const uint16_t HttpService::HTTP_IMPORT_SCHEMA_METHOD_IDX = 9;

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
     : sm_(state_machine)
     , pool_(1)
     , serial_number_(0) {}

void HttpService::Start(lgraph::GlobalConfig* config) {
    InitFuncMap();
    galaxy_ = sm_->GetGalaxy();
    import_manager_.Init(config);
}

void HttpService::Query(google::protobuf::RpcController* cntl_base,
           const HttpRequest*,
           HttpResponse*,
           google::protobuf::Closure* done) {
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl =
        static_cast<brpc::Controller*>(cntl_base);
    AddAccessControlCORS(cntl);
    std::string method = cntl->http_request().unresolved_path();
    std::transform(method.begin(), method.end(), method.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    std::string res;
    try {
        DispatchRequest(method, cntl, res);
    } catch (const lgraph_api::UnauthorizedError & e) {
        return RespondUnauthorized(cntl, e.what());
    } catch (const lgraph_api::BadRequestException & e) {
        return RespondBadRequest(cntl, e.what());
    } catch (const lgraph_api::InternalErrorException & e) {
        return RespondInternalError(cntl, e.what());
    } catch (const std::exception& e) {
        return RespondBadRequest(cntl, e.what());
    }
    return RespondSuccess(cntl, res);
}

void HttpService::DoUploadRequest(brpc::Controller* cntl) {
    const std::string* token = cntl->http_request().GetHeader(HTTP_AUTHORIZATION);

    if (token == nullptr) throw lgraph_api::UnauthorizedError();
    if (!galaxy_->JudgeRefreshTime(*token))
        throw lgraph_api::UnauthorizedError("token has already expire");

    const std::string* file_name = cntl->http_request().GetHeader(HTTP_HEADER_FILE_NAME);
    const std::string* begin_str = cntl->http_request().GetHeader(HTTP_HEADER_BEGIN_POS);
    const std::string* size_str = cntl->http_request().GetHeader(HTTP_HEADER_SIZE);
    if (file_name == nullptr || begin_str == nullptr|| size_str == nullptr) {
        throw lgraph_api::BadRequestException("request header should has a fileName, "
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

void HttpService::DoClearCache(brpc::Controller* cntl) {
    const std::string* token = cntl->http_request().GetHeader(HTTP_AUTHORIZATION);
    if (token == nullptr) throw lgraph_api::UnauthorizedError();
    if (!galaxy_->JudgeRefreshTime(*token))
        throw lgraph_api::UnauthorizedError("token has already expire");

    std::string req = cntl->request_attachment().to_string();
    std::string flag_str;
    if (!ExtractTypedField<std::string>(req, HTTP_FLAG, flag_str))
        throw lgraph_api::BadRequestException("`flag` not specified.");
    int16_t flag = -1;
    try {
        flag = boost::lexical_cast<int16_t>(flag_str);
    } catch (boost::bad_lexical_cast& e) {
        throw lgraph_api::BadRequestException(
            "`flag` should be an integer of the string type");
    }
    if (flag < 0)
        throw lgraph_api::BadRequestException("`flag` should be greater than or equal to 0");
    switch (flag) {
        case HTTP_SPECIFIED_FILE: {
            std::string file_name;
            if (!ExtractTypedField<std::string>(req, HTTP_FILE_NAME, file_name))
                throw lgraph_api::BadRequestException("`fileName` not specified.");
            return DeleteSpecifiedFile(*token, file_name);
        }
        case HTTP_SPECIFIED_USER: {
            std::string user_name;
            if (!ExtractTypedField<std::string>(req, HTTP_USER_NAME, user_name))
                throw lgraph_api::BadRequestException("`userName` not specified.");
            return DeleteSpecifiedUserFiles(*token, user_name);
        }
        case HTTP_ALL_USER: {
            return DeleteAllUserFiles(*token);
        }
    }
}

void HttpService::DoCheckFile(brpc::Controller* cntl, std::string& res) {
    const std::string* token = cntl->http_request().GetHeader(HTTP_AUTHORIZATION);
    if (token == nullptr) throw lgraph_api::UnauthorizedError();
    if (!galaxy_->JudgeRefreshTime(*token))
        throw lgraph_api::UnauthorizedError("token has already expire");

    std::string req = cntl->request_attachment().to_string();

    std::string flag_str;
    if (!ExtractTypedField<std::string>(req, HTTP_FLAG, flag_str))
        throw lgraph_api::BadRequestException("`flag` not specified.");
    int16_t flag = -1;
    try {
        flag = boost::lexical_cast<int16_t>(flag_str);
    } catch (boost::bad_lexical_cast& e) {
        throw lgraph_api::BadRequestException(
            "`flag` should be an integer of the string type");
    }
    if (flag < 0)
        throw lgraph_api::BadRequestException("`flag` should be greater than or equal to 0");

    std::string file_name;
    if (!ExtractTypedField<std::string>(req, HTTP_FILE_NAME, file_name))
        throw lgraph_api::BadRequestException("`fileName` not specified.");

    const std::string user = galaxy_->ParseAndValidateToken(*token);
    std::string absolute_file_name = import_manager_.GetUserPath(user) + "/" + file_name;
    nlohmann::json js;
    switch (flag) {
    case 1: {
            std::string md5sum;
            if (!ExtractTypedField<std::string>(req, "checkSum", md5sum))
                throw lgraph_api::BadRequestException(
                    "check file with check sum must specified `checkSum`");
            // TODO(jzj) : implement md5 checksum from file
        }
    case 2: {
            std::string size_str;
            if (!ExtractTypedField<std::string>(req, "fileSize", size_str))
                throw lgraph_api::BadRequestException(
                    "check file with file size must specified `fileSize`");
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

void HttpService::DoImportFile(brpc::Controller* cntl, std::string& res) {
    const std::string* token = cntl->http_request().GetHeader(HTTP_AUTHORIZATION);
    if (token == nullptr) throw lgraph_api::UnauthorizedError();
    if (!galaxy_->JudgeRefreshTime(*token))
        throw lgraph_api::UnauthorizedError("token has already expire");

    std::string req = cntl->request_attachment().to_string();
    std::string graph;
    if (!ExtractTypedField<std::string>(req, HTTP_GRAPH, graph))
        throw lgraph_api::BadRequestException("graph` not specified.");

    std::string user = galaxy_->ParseAndValidateToken(*token);
    nlohmann::json schema;
    AdjustFilePath(req, user, schema);

    std::string delimiter;
    if (!ExtractTypedField<std::string>(req, HTTP_DELIMITER, delimiter))
        throw lgraph_api::BadRequestException("delimiter` not specified.");

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

    ImportTask task(this, &import_manager_, id, user, *token, graph,
                    delimiter, continue_on_error, skip_packages, schema);
    pool_.PushTask(0, GetSerialNumber(), task);
    nlohmann::json js;
    js[HTTP_TASK_ID] = id;
    res = js.dump();
}

void HttpService::DoImportSchema(brpc::Controller* cntl, std::string& res) {
    const std::string* token = cntl->http_request().GetHeader(HTTP_AUTHORIZATION);
    if (token == nullptr) throw lgraph_api::UnauthorizedError();
    if (!galaxy_->JudgeRefreshTime(*token))
        throw lgraph_api::UnauthorizedError("token has already expire");

    std::string req = cntl->request_attachment().to_string();
    std::string graph;
    if (!ExtractTypedField<std::string>(req, HTTP_GRAPH, graph))
        throw lgraph_api::BadRequestException("graph` not specified.");
    nlohmann::json desc;
    if (!ExtractTypedField<nlohmann::json>(req, HTTP_DESCRIPTION, desc)) {
        throw lgraph_api::BadRequestException("description` not specified.");
    }
    LGraphRequest pb_req;
    ProcessSchemaRequest(graph, desc.dump(), *token, pb_req);
}

void HttpService::DoImportProgress(brpc::Controller* cntl, std::string& res) {
    const std::string* token = cntl->http_request().GetHeader(HTTP_AUTHORIZATION);
    if (token == nullptr) throw lgraph_api::UnauthorizedError();
    if (!galaxy_->JudgeRefreshTime(*token))
        throw lgraph_api::UnauthorizedError("token has already expire");

    std::string req = cntl->request_attachment().to_string();
    std::string task_id;
    if (!ExtractTypedField<std::string>(req, HTTP_TASK_ID, task_id))
        throw lgraph_api::BadRequestException("`taskId` not specified.");
    std::string reason, progress;
    int state = import_manager_.GetImportProgress(task_id, progress, reason);
    nlohmann::json js;
    switch (state) {
    case 0: {
            js[HTTP_PROGRESS_STATE] = "0";
            break;
        }
    case 1: {
            js[HTTP_PROGRESS_STATE] = "1";
            js[HTTP_PROGRESS] = progress;
            break;
        }
    case 2: {
            js[HTTP_PROGRESS_STATE] = "2";
            break;
        }
    case 3: {
            js[HTTP_PROGRESS_STATE] = "3";
            js[HTTP_REASON] = reason;
            break;
        }
    }
    res = js.dump();
}

void HttpService::DoCypherRequest(const brpc::Controller* cntl, std::string& res) {
    const std::string* token = cntl->http_request().GetHeader(HTTP_AUTHORIZATION);
    if (token == nullptr) throw lgraph_api::UnauthorizedError();
    if (!galaxy_->JudgeRefreshTime(*token))
        throw lgraph_api::UnauthorizedError("token has already expire");

    LGraphRequest pb_req;
    BuildPbCypherRequest(cntl, *token, pb_req);
    LGraphResponse pb_res;
    ApplyToStateMachine(pb_req, pb_res);
    BuildJsonCypherResponse(pb_res, res);
}

void HttpService::DoLoginRequest(const brpc::Controller* cntl, std::string& res) {
    std::string req = cntl->request_attachment().to_string();
    std::string username, password;
    if (!ExtractTypedField<std::string>(req, HTTP_USER_NAME, username) ||
        !ExtractTypedField<std::string>(req, HTTP_PASSWORD, password))
            throw lgraph_api::BadRequestException("`userName` or `password` not specified.");
    _HoldReadLock(galaxy_->GetReloadLock());
    std::string token = galaxy_->GetUserToken(username, password);
    if (token.empty()) throw lgraph_api::BadRequestException(
        FMA_FMT("userName {} is invalid", username));
    nlohmann::json js;
    js[HTTP_AUTHORIZATION] = token;
    res = js.dump();
}

void HttpService::DoLogoutRequest(const brpc::Controller* cntl) {
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

void HttpService::BuildPbCypherRequest(const brpc::Controller* cntl,
                          const std::string& token,
                          LGraphRequest& pb) {
    std::string req = cntl->request_attachment().to_string();
    std::string graph;
    std::string query;
    bool json_format = true;
    double timeout = 0;
    if (!ExtractTypedField<std::string>(req, HTTP_GRAPH, graph) ||
        !ExtractTypedField<std::string>(req, HTTP_SCRIPT, query))
            throw lgraph_api::BadRequestException("`graph` or `script` not specified.");
    ExtractTypedField<bool>(req, HTTP_JSON_FORMAT, json_format);
    ExtractTypedField<double>(req, HTTP_TIMEOUT, timeout);
    const std::string user = galaxy_->ParseAndValidateToken(token);
    pb.set_token(token);
    auto field_access = galaxy_->GetRoleFieldAccessLevel(user, graph);
    cypher::RTContext ctx(sm_, galaxy_, token, user, graph, field_access);
    std::string name;
    std::string type;
    bool ret = cypher::Scheduler::DetermineReadOnly(&ctx, query, name, type);
    if (name.empty() || type.empty()) {
        pb.set_is_write_op(!ret);
    } else {
        type.erase(remove(type.begin(), type.end(), '\"'), type.end());
        name.erase(remove(name.begin(), name.end(), '\"'), name.end());
        AccessControlledDB db = galaxy_->OpenGraph(user, graph);
        ret = !db.IsReadOnlyPlugin(
            type == "CPP" ? PluginManager::PluginType::CPP
                               : PluginManager::PluginType::PYTHON, token, name);
        pb.set_is_write_op(!ret);
    }
    CypherRequest* creq = pb.mutable_cypher_request();
    creq->set_result_in_json_format(false);
    creq->set_query(query);
    creq->set_graph(graph);
    creq->set_timeout(timeout);
    creq->set_result_in_json_format(json_format);
}

void HttpService::BuildJsonCypherResponse(LGraphResponse& res, std::string& json) {
    switch (res.error_code()) {
    case LGraphResponse::SUCCESS: {
            const auto& resp = res.cypher_response();
            if (resp.Result_case() == CypherResponse::kJsonResult) {
                nlohmann::json response;
                response[HTTP_RESULT] = nlohmann::json::parse(resp.json_result());
                json = response.dump();
                return;
            } else if (resp.Result_case() == CypherResponse::kBinaryResult) {
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
            return;
        }
    case LGraphResponse::AUTH_ERROR:
        throw lgraph_api::UnauthorizedError(res.error());
    case LGraphResponse::BAD_REQUEST:
        throw lgraph_api::BadRequestException(res.error());
    case LGraphResponse::REDIRECT:
        // return RespondRedirect(request, proto_resp.redirect(), relative_path, false);
    case LGraphResponse::EXCEPTION:
        throw lgraph_api::InternalErrorException(res.error());
    case LGraphResponse::KILLED:
        throw lgraph_api::BadRequestException("Task killed.");
    default:
        throw lgraph_api::InternalErrorException(res.error());
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
    switch (res.error_code()) {
    case LGraphResponse::SUCCESS:
        {
            if (res.schema_response().has_error_message()) {
                throw lgraph_api::BadRequestException(res.schema_response().error_message());
            }
            return;
        }
    case LGraphResponse::AUTH_ERROR:
        throw lgraph_api::UnauthorizedError(res.error());
    case LGraphResponse::BAD_REQUEST:
        throw lgraph_api::BadRequestException(res.error());
    case LGraphResponse::REDIRECT:
        // return RespondRedirect(request, proto_resp.redirect(), relative_path, false);
    case LGraphResponse::EXCEPTION:
        throw lgraph_api::InternalErrorException(res.error());
    case LGraphResponse::KILLED:
        throw lgraph_api::BadRequestException("Task killed.");
    default:
        throw lgraph_api::InternalErrorException(res.error());
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
    if (!ExtractTypedField<nlohmann::json>(req, HTTP_SCHEMA, schema)) {
        throw lgraph_api::BadRequestException("schema` not specified.");
    }
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
        throw lgraph_api::BadRequestException(
            FMA_FMT("{} not exists", file_name));
    }
    if (fs.IsDir(absolute_file_name)) {
        throw lgraph_api::BadRequestException(
            FMA_FMT("{} is a directory", file_name));
    }
    if (!fs.Remove(absolute_file_name)) {
        throw lgraph_api::InternalErrorException(
            FMA_FMT("{} file remove failed", file_name));
    }
}

void HttpService::DeleteSpecifiedUserFiles(const std::string& token, const std::string& user_name) {
    const std::string user = galaxy_->ParseAndValidateToken(token);
    if (user != user_name || !galaxy_->IsAdmin(user)) {
        throw lgraph_api::BadRequestException(
            FMA_FMT("{} is not admin user, can't remove other people's file directories",
                    user_name));
    }
    const std::string user_directory = import_manager_.GetUserPath(user);

    fma_common::LocalFileSystem fs;
    if (!fs.FileExists(user_directory)) {
        throw lgraph_api::BadRequestException(
            FMA_FMT("{} not exists", user_name));
    }

    if (!fs.IsDir(user_directory)) {
        throw lgraph_api::BadRequestException(
            FMA_FMT("{} is not a directory", user_name));
    }

    if (!fs.RemoveDir(user_directory)) {
        throw lgraph_api::InternalErrorException(
            FMA_FMT("{} directory remove failed", user_name));
    }
}

void HttpService::DeleteAllUserFiles(const std::string& token) {
    bool is_admin = false;
    std::string user_name;
    user_name = galaxy_->ParseTokenAndCheckIfIsAdmin(token, &is_admin);
    if (!is_admin) {
        throw lgraph_api::BadRequestException(
            FMA_FMT("{} is not admin user, can't remove all directories",
                    user_name));
    }
    const std::string all_directory = import_manager_.GetRootPath();

    fma_common::LocalFileSystem fs;
    if (!fs.FileExists(all_directory)) {
        throw lgraph_api::BadRequestException(
            FMA_FMT("{} not exists", HTTP_FILE_DIRECTORY));
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

void HttpService::InitFuncMap() {
    functions_map_.clear();
    functions_map_.emplace(HTTP_CYPHER_METHOD, HTTP_CYPHER_METHOD_IDX);
    functions_map_.emplace(HTTP_REFRESH_METHOD, HTTP_REFRESH_METHOD_IDX);
    functions_map_.emplace(HTTP_LOGIN_METHOD, HTTP_LOGIN_METHOD_IDX);
    functions_map_.emplace(HTTP_LOGOUT_METHOD, HTTP_LOGOUT_METHOD_IDX);
    functions_map_.emplace(HTTP_UPLOAD_METHOD, HTTP_UPLOAD_METHOD_IDX);
    functions_map_.emplace(HTTP_CLEAR_CACHE_METHOD, HTTP_CLEAR_CACHE_METHOD_IDX);
    functions_map_.emplace(HTTP_CHECK_FILE_METHOD, HTTP_CHECK_FILE_METHOD_IDX);
    functions_map_.emplace(HTTP_IMPORT_METHOD, HTTP_IMPORT_METHOD_IDX);
    functions_map_.emplace(HTTP_IMPORT_PROGRESS_METHOD, HTTP_IMPORT_PROGRESS_METHOD_IDX);
    functions_map_.emplace(HTTP_IMPORT_SCHEMA_METHOD, HTTP_IMPORT_SCHEMA_METHOD_IDX);
}

void HttpService::DispatchRequest(const std::string& method,
                                  brpc::Controller* cntl, std::string& res) {
    auto it = functions_map_.find(method);
    if (it == functions_map_.end())
        throw lgraph_api::BadRequestException("Unsupported method");
    switch (it->second) {
    case HTTP_CYPHER_METHOD_IDX :
        DoCypherRequest(cntl, res);
        break;
    case HTTP_REFRESH_METHOD_IDX :
        DoRefreshRequest(cntl, res);
        break;
    case HTTP_LOGIN_METHOD_IDX :
        DoLoginRequest(cntl, res);
        break;
    case HTTP_LOGOUT_METHOD_IDX :
        DoLogoutRequest(cntl);
        break;
    case HTTP_UPLOAD_METHOD_IDX :
        DoUploadRequest(cntl);
        break;
    case HTTP_CLEAR_CACHE_METHOD_IDX :
        DoClearCache(cntl);
        break;
    case HTTP_CHECK_FILE_METHOD_IDX :
        DoCheckFile(cntl, res);
        break;
    case HTTP_IMPORT_METHOD_IDX :
        DoImportFile(cntl, res);
        break;
    case HTTP_IMPORT_PROGRESS_METHOD_IDX :
        DoImportProgress(cntl, res);
        break;
    case HTTP_IMPORT_SCHEMA_METHOD_IDX :
        DoImportSchema(cntl, res);
        break;
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

}  // end of namespace http
}  // end of namespace lgraph
