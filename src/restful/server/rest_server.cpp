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

#include <set>

#include "fma-common/file_system.h"
#include "fma-common/fma_stream.h"
#include "restful/server/rest_server.h"
#include "restful/server/json_convert.h"

#include "import/import_online.h"
#include "import/import_client.h"

#include "core/audit_logger.h"
#include "core/task_tracker.h"
#include "protobuf/ha.pb.h"
#include "server/proto_convert.h"
#include "server/state_machine.h"

#ifndef _WIN32
#include "cypher/resultset/record.h"
#endif

// qw
#include "fma-common/hardware_info.h"

using namespace fma_common;

using namespace std;
using namespace web;
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;

namespace lgraph {

struct DoneClosure : public google::protobuf::Closure {
    std::mutex mu_;
    std::condition_variable cv_;
    bool done_ = false;

 public:
    void Run() {
        mu_.lock();
        done_ = true;
        mu_.unlock();
        cv_.notify_all();
    }

    void Wait() {
        std::unique_lock<std::mutex> l(mu_);
        while (!done_) cv_.wait(l);
    }
};

LGraphResponse RestServer::ApplyToStateMachine(const LGraphRequest& lgreq) const {
    LGraphResponse resp;
    DoneClosure d;
    state_machine_->HandleRequest(nullptr, &lgreq, &resp, &d);
    d.Wait();
    return resp;
}

#define TRY_PARSE_STR_ID(req, str, id, id_desc)                                                 \
    if (!ExtractVidFromString((str), id)) {                                                     \
        return RespondBadRequest(request,                                                       \
                                 "Failed to parse " id_desc " from URI: (" + _TS((str)) + ")"); \
    }

template <typename T>
inline utility::string_t JoinPath(const T& str) {
    return _TU(str);
}

inline utility::string_t JoinPath(const utility::string_t& str) { return str; }

inline utility::string_t JoinPath(const char* str) { return _TU(str); }

#ifdef _WIN32
inline utility::string_t JoinPath(const std::string& str) { return _TU(str); }
#endif

template <typename T, typename... Ts>
inline utility::string_t JoinPath(const T& str, const Ts&... strs) {
    return _TU(str) + JoinPath(strs...);
}

template <typename... Ts>
inline web::json::value JsonPath(const Ts&... str) {
    return web::json::value::string(JoinPath(str...));
}

static bool ExtractEdgeUidFromUri(const std::string& str, EdgeUid& uid) {
    const char* beg = str.data();
    const char* end = str.data() + str.size();
    // src
    size_t s = fma_common::TextParserUtils::ParseT(beg, end, uid.src);
    if (!s) return false;
    beg += s;
    if (beg >= end || *beg != '_') return false;
    beg++;
    // dst
    s = fma_common::TextParserUtils::ParseT(beg, end, uid.dst);
    if (!s) return false;
    beg += s;
    if (beg >= end || *beg != '_') return false;
    beg++;
    // label
    s = fma_common::TextParserUtils::ParseT(beg, end, uid.lid);
    if (!s) return false;
    beg += s;
    if (beg >= end || *beg != '_') return false;
    beg++;
    // tid
    s = fma_common::TextParserUtils::ParseT(beg, end, uid.tid);
    if (!s) return false;
    beg += s;
    if (beg >= end || *beg != '_') return false;
    beg++;
    // eid
    s = fma_common::TextParserUtils::ParseT(beg, end, uid.eid);
    if (!s) return false;
    beg += s;
    return (beg == end);
}

static bool JsonToProtoFieldData(const web::json::value& js, ProtoFieldData* fd) {
    if (js.is_boolean()) {
        fd->set_boolean(js.as_bool());
        return true;
    }
    if (js.is_integer()) {
        fd->set_int64_(js.as_number().to_int64());
        return true;
    }
    if (js.is_double()) {
        fd->set_dp(js.as_double());
        return true;
    }
    if (js.is_string()) {
        fd->set_str(_TS(js.as_string()));
        return true;
    }
    if (js.is_null()) {
        fd->Clear();
        return true;
    }
    return false;
}

#define GET_REQUIRED_JSON_FIELD(js, key, value_ref)                                        \
    do {                                                                                   \
        if (!ExtractTypedField((js), _TU(key), (value_ref)))                               \
            return RespondBadRequest(                                                      \
                request, "Failed to parse required parameter [" + std::string(key) + "]"); \
    } while (0)

/**
 * Convert js to ListOfProtoFieldData.
 *
 * @param          js           The js.
 * @param [in,out] field_values If non-null, the field values.
 *
 * @return  True if it succeeds, false if it fails.
 */
static bool JsonToListOfProtoFieldData(const web::json::value& js,
                                       ListOfProtoFieldData* field_values) {
    auto& fvs = js.as_array();
    auto* arr = field_values->mutable_values();
    arr->Reserve(static_cast<int>(fvs.size()));
    for (auto& v : fvs) {
        if (!JsonToProtoFieldData(v, arr->Add())) return false;
    }
    return true;
}

template <typename T>
static bool ExtractObjectArray(const web::json::value& js, const utility::string_t& key,
                               ::google::protobuf::RepeatedPtrField<T>* values) {
    if (!js.has_array_field(key)) return false;
    auto& arr = js.at(key).as_array();
    values->Reserve(static_cast<int>(arr.size()));
    for (auto& v : arr) {
        if (!JsonToType<T>(v, *values->Add())) return false;
    }
    return true;
}

template <typename T>
static bool ExtractPODArray(const web::json::value& js, const utility::string_t& key,
                            ::google::protobuf::RepeatedField<T>* values) {
    if (!js.has_array_field(key)) return false;
    auto& arr = js.at(key).as_array();
    values->Reserve(static_cast<int>(arr.size()));
    for (auto& v : arr) {
        T d;
        if (!JsonToType<T>(v, d)) return false;
        values->Add(d);
    }
    return true;
}

static bool JsonToFieldNameValueDict(const web::json::value& js,
                                     ::google::protobuf::RepeatedPtrField<std::string>* field_names,
                                     ListOfProtoFieldData* values) {
    for (auto dit = js.as_object().cbegin(); dit != js.as_object().cend(); dit++) {
        *field_names->Add() = _TS(dit->first);
        if (!JsonToProtoFieldData(dit->second, values->add_values())) return false;
    }
    return true;
}

static bool ExtractFieldNameValueDict(
    const web::json::value& js, ::google::protobuf::RepeatedPtrField<std::string>* field_names,
    ListOfProtoFieldData* values) {
    if (!js.has_field(RestStrings::PROP)) return false;
    auto& v = js.at(RestStrings::PROP);
    for (auto dit = v.as_object().cbegin(); dit != v.as_object().cend(); dit++) {
        *field_names->Add() = _TS(dit->first);
        if (!JsonToProtoFieldData(dit->second, values->add_values())) return false;
    }
    return true;
}

static bool ExtractFieldNamesAndValues(
    const web::json::value& js, ::google::protobuf::RepeatedPtrField<std::string>* field_names,
    ::google::protobuf::RepeatedPtrField<ListOfProtoFieldData>* field_values_list,
    bool& batch_mode) {
    // old format, single data
    if (js.has_field(RestStrings::PROP)) {
        batch_mode = false;
        return ExtractFieldNameValueDict(js, field_names, field_values_list->Add());
    }
    // new format, list of fields and then list of list of values
    if (js.has_array_field(RestStrings::FIELDS) && js.has_array_field(RestStrings::VALUES)) {
        batch_mode = true;
        auto& fields = js.at(RestStrings::FIELDS).as_array();
        for (auto& f : fields) {
            *field_names->Add() = _TS(f.as_string());
        }
        auto& vertexes = js.at(RestStrings::VALUES).as_array();
        for (auto& v : vertexes) {
            if (v.size() != (size_t)field_names->size()) return false;
            ListOfProtoFieldData* values = field_values_list->Add();
            if (!JsonToListOfProtoFieldData(v, values)) return false;
        }
        return true;
    }
    return false;
}

static bool ExtractLabel(const web::json::value& js, std::string& label) {
    if (!js.has_string_field(RestStrings::LABEL)) return false;
    label = _TS(js.at(RestStrings::LABEL).as_string());
    return true;
}

static bool ExtractFieldSpec(const web::json::value& js, ProtoFieldSpec* ret) {
    FieldSpec fs;
    if (!ExtractStringField(js, RestStrings::NAME, fs.name)) return false;
    if (!js.has_string_field(RestStrings::TYPE)) return false;
    if (!field_data_helper::TryGetFieldType(_TS(js.at(RestStrings::TYPE).as_string()), fs.type))
        return false;
    fs.optional = false;
    ExtractBoolField(js, RestStrings::NULLABLE, fs.optional);
    FieldSpecConvert::FromLGraphT(fs, ret);
    return true;
}

/**
 * Extracts the edge data in js[EDGES]
 *
 * @param          js       The js.
 * @param [in,out] edges    If non-null, the edges.
 *
 * @return  True if it succeeds, false if it fails.
 */
static bool ExtractEdgeData(const web::json::value& js,
                            ::google::protobuf::RepeatedPtrField<SrcDstFieldValues>* edges) {
    if (!js.has_array_field(RestStrings::EDGE)) return false;
    auto& edges_json = js.at(RestStrings::EDGE).as_array();
    if (edges_json.size() == 0) return false;
    edges->Reserve(static_cast<int>(edges_json.size()));
    for (auto& ej : edges_json) {
        auto* e = edges->Add();
        int64_t src, dst;
        if (!ExtractIntField(ej, RestStrings::SRC, src) ||
            !ExtractIntField(ej, RestStrings::DST, dst) ||
            !ej.has_array_field(RestStrings::VALUES) ||
            !JsonToListOfProtoFieldData(ej.at(RestStrings::VALUES), e->mutable_values())) {
            return false;
        }
        e->set_src(src);
        e->set_dst(dst);
    }
    return true;
}

static bool ExtractAddLabelRequest(const web::json::value& js, AddLabelRequest* req) {
    if (!ExtractStringField(js, RestStrings::NAME, *req->mutable_label())) return false;
    bool is_vertex;
    if (!ExtractBoolField(js, RestStrings::ISV, is_vertex)) return false;
    req->set_is_vertex(is_vertex);
    if (!js.has_array_field(RestStrings::FIELDS)) return false;
    for (auto& f : js.at(RestStrings::FIELDS).as_array()) {
        if (!ExtractFieldSpec(f, req->add_fields())) return false;
    }
    if (is_vertex) {
        if (!ExtractStringField(js, RestStrings::PRIMARY, *req->mutable_primary())) return false;
    } else {
        if (js.has_array_field(RestStrings::EDGE_CONSTRAINTS)) {
            for (const auto& f : js.at(RestStrings::EDGE_CONSTRAINTS).as_array()) {
                if (!(f.is_array() && f.size() == 2)) return false;
                auto ec = req->add_edge_constraints();
                ec->set_src_label(ToStdString(f.at(0).as_string()));
                ec->set_dst_label(ToStdString(f.at(1).as_string()));
            }
        }
    }
    return true;
}

web::json::value ProtoFieldDataToJson(const ProtoFieldData& data) {
    switch (data.Data_case()) {
    case ProtoFieldData::DATA_NOT_SET:
        return web::json::value::null();
    case ProtoFieldData::kBoolean:
        return web::json::value::boolean(data.boolean());
    case ProtoFieldData::kInt8:
        return web::json::value::number(data.int8_());
    case ProtoFieldData::kInt16:
        return web::json::value::number(data.int16_());
    case ProtoFieldData::kInt32:
        return web::json::value::number(data.int32_());
    case ProtoFieldData::kInt64:
        return web::json::value::number(data.int64_());
    case ProtoFieldData::kSp:
        return web::json::value::number(data.sp());
    case ProtoFieldData::kDp:
        return web::json::value::number(data.dp());
    case ProtoFieldData::kDate:
        return web::json::value::string(_TU(Date(data.date()).ToString()));
    case ProtoFieldData::kDatetime:
        return web::json::value::string(_TU(DateTime(data.datetime()).ToString()));
    case ProtoFieldData::kStr:
        return web::json::value(_TU(data.str()));
    case ProtoFieldData::kBlob:
        return web::json::value(_TU(::lgraph_api::base64::Encode(data.blob())));
    }
    FMA_ASSERT(false);
    return web::json::value::null();
}

template <>
inline web::json::value ValueToJson(const StateMachine::Peer& peer) {
    web::json::value v;
    v[RestStrings::RPC_ADDR] = ValueToJson(peer.rpc_addr);
    v[RestStrings::REST_ADDR] = ValueToJson(peer.rest_addr);
    v[RestStrings::STATE] = ValueToJson(peer.StateString());
    return v;
}

RestServer::RestServer(StateMachine* state_machine, const Config& config,
                       const std::shared_ptr<GlobalConfig> service_config)
    : logger_(fma_common::Logger::Get("RestServer")),
      state_machine_(state_machine),
      config_(config),
      global_config_(service_config),
      path_to_case_(GetPathToCaseDict()) {
    Start();
}

RestServer::RestPathCases RestServer::GetRestPathCase(const utility::string_t& first_path) const {
    auto it = path_to_case_.find(first_path);
    if (it == path_to_case_.end()) return RestPathCases::INVALID;
    return it->second;
}

bool RestServer::IsClientAddressAllowed(const web::http::http_request& request) {
    // assuming a read lock is already held with galaxy_
    if (global_config_ && global_config_->enable_ip_check) {
        const std::string& ip = _TS(request.remote_address());
        if (!galaxy_->IsIpInWhitelist(ip)) {
            FMA_WARN_STREAM(logger_) << "Access from illegal host: " << ip;
            return false;
        }
    }
    return true;
}

void RestServer::Start() {
    if (started_) {
        FMA_WARN_STREAM(logger_) << "Failed to start REST server: it is already started";
        return;
    }
    galaxy_ = state_machine_->GetGalaxy();
#ifndef _WIN32
    cypher_scheduler_ = state_machine_->GetCypherScheduler();
#endif
    // construct addr
    std::string address = config_.use_ssl ? "https://" : "http://";
    address.append(config_.host).append((":")).append(std::to_string(config_.port));
    address.append("/");
    // construct listener config
    auto addr = _TU(address);
    if (config_.use_ssl) {
#ifndef _WIN32
        // for http over ssl, https
        fma_common::InputFmaStream is(config_.server_key);
        if (!is.Good()) throw InternalError("Failed to open server key file " + config_.server_key);
        std::string key_buf(is.Size(), 0);
        is.Read(&key_buf[0], key_buf.size());
        is.Close();
        is.Open(config_.server_cert);
        if (!is.Good())
            throw InternalError("Failed to open server cert file " + config_.server_cert);
        std::string cert_buf(is.Size(), 0);
        is.Read(&cert_buf[0], cert_buf.size());
        is.Close();
        http_listener_config server_config;
        server_config.set_ssl_context_callback([key_buf, cert_buf](boost::asio::ssl::context& ctx) {
            try {
                boost::asio::const_buffer cert(cert_buf.data(), cert_buf.size());
                boost::asio::const_buffer key(key_buf.data(), key_buf.size());
                ctx.set_options(boost::asio::ssl::context::default_workarounds);
                ctx.use_certificate_chain(cert);
                ctx.use_private_key(key, boost::asio::ssl::context::pem);
            } catch (std::exception& e) {
                FMA_ERR() << "Oops, error occurred! Please make sure server's cert & key "
                             "are correct. Detail: "
                          << e.what();
            }
        });
        listener_ = http_listener(addr, server_config);
#else
        throw std::runtime_error("SSL is not supported on Windows.");
#endif
    } else {
        listener_ = http_listener(addr);
    }
    init_server();
    try {
        listener_.open().wait();
        FMA_INFO_STREAM(logger_) << "Listening for REST on port " << config_.port;
        started_ = true;
    } catch (std::exception& e) {
        FMA_FATAL_STREAM(logger_) << "Error initializing REST server: " << e.what();
    }
}

void RestServer::Stop() {
    if (!started_) {
        return;
    }
    try {
        listener_.close().wait();
        // cpprestsdk has some mysterious bugs that crashes server if we exit too quickly
        fma_common::SleepS(0.5);
        FMA_INFO_STREAM(logger_) << "REST server stopped.";
        started_ = false;
#ifndef _WIN32
        cypher_scheduler_ = nullptr;
#endif
    } catch (std::exception& e) {
        FMA_WARN_STREAM(logger_) << "Failed to stop REST server: " << e.what();
    }
}

RestServer::~RestServer() { Stop(); }

static void ListDirFiles(const std::string& directory, std::vector<std::string>& files) {
    auto& fs = fma_common::FileSystem::GetFileSystem(directory);
    auto f = fs.ListFiles(directory, nullptr);
    files.insert(files.end(), f.begin(), f.end());
    auto d = fs.ListSubDirs(directory);
    for (auto& subdir : d) {
        ListDirFiles(subdir, files);
    }
}

static bool InitHtmlContentMap(
    const std::string& resource_dir,
    std::map<utility::string_t, std::tuple<utility::string_t, utility::string_t>>& hcm) {
    static const std::map<std::string, std::string> content_type = {
        {"html", "text/html"},
        {"png", "image/png"},
        {"svg", "image/svg+xml"},
        {"ico", "image/x-icon"},
        {"js", "application/javascript"},
        {"css", "text/css"},
        {"ttf", "font/ttf"},
        {"woff", "font/woff"},
        {"gz", "application/gzip"},
    };
    static const std::string prefix = "/resource";
    std::vector<std::string> files;
    ListDirFiles(resource_dir, files);

    hcm[_TU("")] = hcm[_TU("/")] = hcm[_TU(prefix)] = hcm[_TU(prefix + "/")] =
        hcm[_TU(prefix + "/index.html")] =
            std::make_tuple(_TU(resource_dir + "/index.html"), _TU("text/html"));
    for (auto& f : files) {
#ifdef _WIN32
        for (auto& c : f)
            if (c == '\\') c = '/';
#endif
        auto local_f = f;
        local_f.replace(0, resource_dir.size(), prefix);
        auto suffix = f.substr(f.find_last_of('.') + 1);
        auto it = content_type.find(suffix);
        std::string type = it == content_type.end() ? "" : it->second;
        // FMA_DBG() << "Found HTML content: " << local_f << " -> " << f;
        hcm[_TU(local_f)] = std::make_tuple(_TU(f), _TU(type));
    }
    return true;
}

void RestServer::init_server() {
    listener_.support(methods::GET,
                      std::bind(&RestServer::handle_get, this, std::placeholders::_1));
    listener_.support(methods::PUT,
                      std::bind(&RestServer::handle_put, this, std::placeholders::_1));
    listener_.support(methods::POST,
                      std::bind(&RestServer::handle_post, this, std::placeholders::_1));
    listener_.support(methods::DEL,
                      std::bind(&RestServer::handle_delete, this, std::placeholders::_1));
    listener_.support(methods::OPTIONS,
                      std::bind(&RestServer::handle_options, this, std::placeholders::_1));

    InitHtmlContentMap(config_.resource_dir, html_content_map_);
}

std::string RestServer::GetUser(const web::http::http_request& request,
                                std::string* token_ret) const {
    if (config_.disable_auth) return lgraph::_detail::DEFAULT_ADMIN_NAME;
    auto& headers = request.headers();
    auto it = headers.find(_TU("Authorization"));
    if (it == headers.end()) {
        throw AuthError("No token given in the request.");
    }
    const std::string& auth_str = _TS(it->second);
    if (!fma_common::StartsWith(auth_str, "Bearer ")) {
        throw AuthError("Malformed token: " + auth_str);
    }
    std::string token = auth_str.substr(7);
    if (token_ret) *token_ret = token;
    return galaxy_->ParseAndValidateToken(token);
}

/**
 * Gets reader version. This is used in replicated mode.
 *
 * @param          request  The message.
 * @param [in,out] ver      The returned version.
 *
 * @return  true if ReaderVersion is specified in header, false otherwise.
 */
static bool GetReaderVersion(const http_request& message, int64_t& ver) {
    auto& headers = message.headers();
    auto it = headers.find(RestStrings::SVR_VER);
    if (it == headers.end()) return false;
    const std::string& vstr = _TS(it->second);
    size_t r = fma_common::TextParserUtils::ParseInt64(vstr.data(), vstr.data() + vstr.size(), ver);
    if (r == 0)
        throw InternalError("Failed to parse " + _TS(RestStrings::SVR_VER) + " from header.");
    return true;
}

/**
 * Gets a http response with CORS.
 *
 * @param code  The code.
 *
 * @return  The CORS response.
 */
http_response RestServer::GetCorsResponse(status_code code) const {
    http_response response(code);
    response.headers().add(_TU("Access-Control-Allow-Origin"), _TU("*"));
    response.headers().add(_TU("Access-Control-Allow-Headers"),
                           _TU("X-Requested-With, Origin, Authorization, Content-Type, Accept"));
    response.headers().add(_TU("Access-Control-Allow-Credentials"), true);
    response.headers().add(_TU("Access-Control-Allow-Methods"),
                           _TU("GET, POST, PUT, OPTIONS, DELETE"));
    response.headers().add(RestStrings::SVR_VER, state_machine_->GetVersion());
    return response;
}

/**
 * Respond "HTTP BadRequest" (400).
 *
 * @param request   The request.
 */
void RestServer::RespondUnauthorized(const http_request& request,
                                     const std::string& error = "") const {
    FMA_WARN_STREAM(logger_) << "Unauthorized request: " << _TS(request.to_string()) << ": "
                             << error;
    http_response response = GetCorsResponse(status_codes::Unauthorized);
    web::json::value body;
    body[RestStrings::ERR_MSG] = web::json::value(_TU("Unauthorized: " + error));
    response.set_body(body);
    request.reply(response);
}

/**
 * Respond "HTTP BadRequest" (400) by calling message.reply() directly.
 *
 * @param request   The request.
 * @param error     The detailed error message.
 */
void RestServer::RespondBadRequest(const http_request& request, const std::string& error) const {
    AUDIT_LOG_FAIL(error);
    try {
        // request.to_string() may throw an exception
        auto req = request.to_string();
        FMA_INFO_STREAM(logger_) << "Illegal request: " << _TS(req);
    } catch (std::exception& e) {
        FMA_INFO_STREAM(logger_) << "request.to_string throw an exception: " << e.what();
    }
    FMA_INFO_STREAM(logger_) << "RespondBadRequest error info : " << error;
    http_response response = GetCorsResponse(status_codes::BadRequest);
    web::json::value body;
    body[RestStrings::ERR_MSG] = web::json::value(_TU(error));
    response.set_body(body);
    request.reply(response);
}

void RestServer::RespondBadURI(const http_request& request) const {
    AUDIT_LOG_FAIL("Bad URI");
    return RespondBadRequest(request, "Illegal URI.");
}

void RestServer::RespondBadJSON(const http_request& request) const {
    AUDIT_LOG_FAIL("Illegal request");
    return RespondBadRequest(request, "Illegal JSON content.");
}

/**
 * Respond "HTTP OK" (200) by calling message.reply() directly.
 *
 * @param request   The request.
 * @param body      The response body.
 */
void RestServer::RespondSuccess(const http_request& request, const web::json::value& body) const {
    AUDIT_LOG_SUCC();
    FMA_DBG_STREAM(logger_) << "request successful";
    http_response response = GetCorsResponse(status_codes::OK);
    response.set_body(body);
    request.reply(response);
}

void RestServer::RespondSuccess(const http_request& request, const std::string& body) const {
    AUDIT_LOG_SUCC();
    FMA_DBG_STREAM(logger_) << "request successful";
    http_response response = GetCorsResponse(status_codes::OK);
    response.set_body(body);
    request.reply(response);
}

void RestServer::RespondSuccess(const http_request& request) const {
    AUDIT_LOG_SUCC();
    FMA_DBG_STREAM(logger_) << "request successful";
    http_response response = GetCorsResponse(status_codes::OK);
    request.reply(response);
}

/**
 * Respond "HTTP Temporary Redirect" (307) by calling message.reply() directly.
 *
 * @param request       The request.
 * @param host          The alternative host to redirect to.
 * @param relative_path Relative path of the URL.
 */
void RestServer::RespondRedirect(const http_request& request, const std::string& host,
                                 const utility::string_t& relative_path, bool need_leader) const {
    AUDIT_LOG_FAIL("Request redirected");
    FMA_DBG_STREAM(logger_) << "Request redirected during handling of request "
                            << _TS(request.to_string());
    http_response response = GetCorsResponse(status_codes::TemporaryRedirect);
    web::json::value body;
    body[RestStrings::LOCATION] = web::json::value(_TU(host));
    if (need_leader) {
        body[RestStrings::ERR_MSG] =
            web::json::value(_TU("Request can only be served in leader node."));
    } else {
        body[RestStrings::ERR_MSG] =
            web::json::value(_TU("Request need to be served in a server with newer version."));
    }
    body[RestStrings::SVR_VER] = state_machine_->GetVersion();
    response.set_body(body);
    request.reply(response);
}

/**
 * Respond "HTTP Internal Error" (500) by calling message.reply() directly.
 *
 * @param request   The request.
 * @param e         A std::exception to process.
 */
void RestServer::RespondInternalError(const http_request& request, const std::string& e) const {
    AUDIT_LOG_FAIL(e);
    FMA_WARN_STREAM(logger_) << "Exception occurred during handling of request "
                             << _TS(request.to_string()) << ": " << e;
    http_response response = GetCorsResponse(status_codes::InternalError);
    web::json::value body;
    body[RestStrings::ERR_MSG] = web::json::value(_TU(e));
    response.set_body(body);
    request.reply(response);
}

/**
 * Respond "HTTP Internal Error" (500) by calling message.reply() directly.
 *
 * @param request   The request.
 * @param e         A std::exception to process.
 */
void RestServer::RespondInternalException(const http_request& request,
                                          const std::exception& e) const {
    AUDIT_LOG_FAIL(e.what());
    FMA_WARN_STREAM(logger_) << "Exception occurred during handling of request "
                             << _TS(request.to_string()) << ": " << e.what();
    RespondInternalError(request, e.what());
}

void RestServer::RespondRSMError(const http::http_request& request,
                                 const LGraphResponse& proto_resp,
                                 const utility::string_t& relative_path,
                                 const std::string& request_type) const {
    AUDIT_LOG_FAIL(proto_resp.error());
    FMA_DBG_ASSERT(proto_resp.error_code() != LGraphResponse::SUCCESS);
    switch (proto_resp.error_code()) {
    case LGraphResponse::AUTH_ERROR:
        return RespondUnauthorized(request, proto_resp.error());
    case LGraphResponse::BAD_REQUEST:
        return RespondBadRequest(request, proto_resp.error());
    case LGraphResponse::REDIRECT:
        return RespondRedirect(request, proto_resp.redirect(), relative_path, false);
    case LGraphResponse::EXCEPTION:
        return RespondInternalError(request, proto_resp.error());
    case LGraphResponse::KILLED:
        return RespondBadRequest(request, "Task killed.");
    default:
        return RespondInternalError(request, proto_resp.error());
    }
}

/**
 * Store the key and value as key-value pair in json value.
 *
 * @param          key      The key.
 * @param [in,out] value    The value.
 * @param [in,out] json     The JSON.
 */
static void FieldDataToJson(const std::string& key, lgraph::FieldData& value,
                            web::json::value& json) {
    switch (value.type) {
    case FieldType::NUL:
        json[_TU(key)] = web::json::value::null();
        return;
    case FieldType::BOOL:
        json[_TU(key)] = web::json::value::boolean(value.data.boolean);
        return;
    case FieldType::INT8:
        json[_TU(key)] = web::json::value::number(value.data.int8);
        return;
    case FieldType::INT16:
        json[_TU(key)] = web::json::value::number(value.data.int16);
        return;
    case FieldType::INT32:
        json[_TU(key)] = web::json::value::number(value.data.int32);
        return;
    case FieldType::INT64:
        json[_TU(key)] = web::json::value::number(value.data.int64);
        return;
    case FieldType::FLOAT:
        json[_TU(key)] = web::json::value::number(value.data.sp);
        return;
    case FieldType::DOUBLE:
        json[_TU(key)] = web::json::value::number(value.data.dp);
        return;
    case FieldType::DATE:
        json[_TU(key)] = web::json::value::string(_TU(Date(value.data.int32).ToString()));
        return;
    case FieldType::DATETIME:
        json[_TU(key)] = web::json::value::string(_TU(DateTime(value.data.int64).ToString()));
        return;
    case FieldType::STRING:
        json[_TU(key)] = web::json::value::string(_TU(std::move(*value.data.buf)));
        return;
    case FieldType::BLOB:
        json[_TU(key)] =
            web::json::value::string(_TU(::lgraph_api::base64::Encode(*value.data.buf)));
        return;
    default:
        FMA_ASSERT(false);
    }
}

/**
 * Convert JSON to field data. If JSON value is null integer, double or string, convert to
 * corresponding FieldData, otherwise fd is left unchanged and return false.
 *
 * @param          v    A web::json::value to process.
 * @param [in,out] fd   The fd.
 *
 * @return  True if success, otherwise false
 */
static bool JsonToFieldData(const web::json::value& v, FieldData& fd) {
    if (v.is_integer()) {
        fd = lgraph::FieldData(static_cast<int64_t>(v.as_number().to_int64()));
        return true;
    }
    if (v.is_double()) {
        fd = lgraph::FieldData(v.as_double());
        return true;
    }
    if (v.is_string()) {
        fd = lgraph::FieldData(_TS(v.as_string()));
        return true;
    }
    if (v.is_null()) {
        fd = lgraph::FieldData();
        return true;
    }
    return false;
}

/**
 * Gets node properties and convert to JSON. This assumes that the node iterator is valid.
 *
 * @param [in,out] txn          The transaction.
 * @param [in,out] vit          The vit.
 * @param [in,out] response     The output response.
 */
static void GetNodeProperties(lgraph::Transaction& txn, lgraph::graph::VertexIterator& vit,
                              web::json::value& response) {
    FMA_DBG_ASSERT(vit.IsValid());
    auto field_defs = txn.GetVertexSchema(vit);
    std::vector<size_t> field_ids;
    for (size_t i = 0; i < field_defs.size(); i++) field_ids.push_back(i);
    auto fields = txn.GetVertexFields(vit, field_ids);
    for (size_t i = 0; i < fields.size(); i++) {
        FieldDataToJson(field_defs[i].name, fields[i], response);
    }
}

/**
 * Gets edge properties and convert to JSON. This assumes that the edge iterator is valid.
 *
 * @param [in,out] txn      The transaction.
 * @param [in,out] eit      The eit.
 * @param [in,out] response The response.
 */
static void GetEdgeProperties(lgraph::Transaction& txn, lgraph::graph::OutEdgeIterator& eit,
                              web::json::value& response) {
    auto field_defs = txn.GetEdgeSchema(eit);
    std::vector<size_t> field_ids;
    for (size_t i = 0; i < field_defs.size(); i++) field_ids.push_back(i);
    auto fields = txn.GetEdgeFields(eit, field_ids);
    for (size_t i = 0; i < fields.size(); i++) {
        FieldDataToJson(field_defs[i].name, fields[i], response);
    }
}

/**
 * Extracts a vid from string.
 *
 * @param          buf  The string.
 * @param [in,out] vid  The vid.
 *
 * @return  True if it succeeds, false if it fails.
 */
static bool ExtractVidFromString(const utility::string_t& str, lgraph::VertexId& vid) {
    const std::string& s = _TS(str);
    if (_F_UNLIKELY(s.empty())) return false;
    size_t r = fma_common::TextParserUtils::ParseInt64(s.data(), s.data() + s.size(), vid);
    return r == s.size();
}

static bool ExtractLidFromString(const utility::string_t& str, lgraph::LabelId& lid) {
    const std::string& s = _TS(str);
    if (_F_UNLIKELY(s.empty())) return false;
    size_t r = fma_common::TextParserUtils::ParseT<LabelId>(s.data(), s.data() + s.size(), lid);
    return r == s.size();
}

/**
 * Send a Redirect response and return true if server data version is too old for this request.
 *
 * @param request       The message.
 * @param relative_path Relative path of current request.
 *
 * @return  True if it succeeds, false if it fails.
 */
bool RestServer::RedirectIfServerTooOld(const http_request& request,
                                        const utility::string_t& relative_path) const {
    int64_t ver = -1;
    if (GetReaderVersion(request, ver) && state_machine_->GetVersion() < ver) {
        RespondRedirect(request, state_machine_->GetMasterRestAddr(), relative_path, false);
        return true;
    }
    return false;
}

// qw add cpu_rate in handle_get()
static web::json::value GetCPURate() {
    fma_common::HardwareInfo::CPURate cpuRate = fma_common::HardwareInfo::GetCPURate();
    web::json::value js_cpu;
    js_cpu[_TU("self")] = web::json::value::number((size_t)cpuRate.selfCPURate);
    js_cpu[_TU("server")] = web::json::value::number((size_t)cpuRate.serverCPURate);
    js_cpu[_TU("unit")] = web::json::value::string(_TU("%"));
    return js_cpu;
}

static web::json::value GetDiskRate() {
    fma_common::HardwareInfo::DiskRate diskRate = fma_common::HardwareInfo::GetDiskRate();
    web::json::value js_disk;
    js_disk[_TU("read")] = web::json::value::number((size_t)diskRate.readRate);
    js_disk[_TU("write")] = web::json::value::number((size_t)diskRate.writeRate);
    js_disk[_TU("unit")] = web::json::value::string(_TU("B/s"));
    return js_disk;
}

// qw add memory in handle_get()
static web::json::value GetMemory() {
    struct fma_common::HardwareInfo::MemoryInfo memoryInfo;
    fma_common::HardwareInfo::GetMemoryInfo(memoryInfo);
    web::json::value js_mem;
    js_mem[_TU("self")] = web::json::value::number((uint64_t)(memoryInfo.selfMemory));
    js_mem[_TU("server_avail")] = web::json::value::number((uint64_t)(memoryInfo.available));
    js_mem[_TU("server_total")] = web::json::value::number((uint64_t)(memoryInfo.total));
    js_mem[_TU("unit")] = web::json::value::string(_TU("KB"));
    return js_mem;
}

// qw add db_space in handle_get()
static web::json::value GetDbSpace(const std::string& dir) {
    size_t dbSpace = fma_common::GetDirSpace(dir.c_str());
    struct fma_common::DiskInfo diskInfo;
    fma_common::GetDiskInfo(diskInfo, dir.c_str());
    web::json::value js_space;
    js_space[_TU("space")] = web::json::value::number(dbSpace);
    js_space[_TU("disk_total")] = web::json::value::number((uint64_t)diskInfo.total);
    js_space[_TU("disk_avail")] = web::json::value::number((uint64_t)diskInfo.avail);
    js_space[_TU("unit")] = web::json::value::string(_TU("B"));
    return js_space;
}

// qw add db_config in handle_get()
static web::json::value GetDbConfig(std::shared_ptr<GlobalConfig> global_conf) {
    web::json::value js_dbconfig;
    if (global_conf != nullptr) {
        js_dbconfig = ValueToJson(*global_conf);
    }
    return js_dbconfig;
}

void RestServer::HandleGetWeb(const std::string& user, const http_request& request,
                              const utility::string_t& relative_path,
                              const std::vector<utility::string_t>& paths) const {
    // handle resource requests, like html, css, etc.
    auto content_data = html_content_map_.find(relative_path);
    if (content_data != html_content_map_.end()) {
        if (!enable_web_service_) return RespondBadRequest(request, "Web is disabled");
        auto& file_name = std::get<0>(content_data->second);
        auto& content_type = std::get<1>(content_data->second);
        ::concurrency::streams::fstream::open_istream(file_name, std::ios::in)
            .then([=](::concurrency::streams::istream is) {
                request.reply(status_codes::OK, is, content_type).then([this](pplx::task<void> t) {
                    try {
                        t.get();
                    } catch (std::exception& e) {
                        // Ignore the error, Log it if a logger is available
                        FMA_WARN_STREAM(logger_) << "Failed to send reply message: " << e.what();
                    }
                });
            })
            .then([this, request](pplx::task<void> t) {
                try {
                    t.get();
                } catch (std::exception& e) {
                    FMA_WARN_STREAM(logger_) << "Failed to open file: " << e.what();
                    // opening the file (open_istream) failed. Reply with an error.
                    request.reply(status_codes::InternalError).then([this](pplx::task<void> t) {
                        FMA_WARN_STREAM(logger_) << "Replied with error code";
                    });
                }
            });
        return;
    }
    return RespondBadURI(request);
}

void RestServer::HandleGetInfo(const std::string& user, const http_request& request,
                               const utility::string_t& relative_path,
                               const std::vector<utility::string_t>& paths) const {
    BEG_AUDIT_LOG(user, "", lgraph::LogApiType::SingleApi, false, "Get info");

    web::json::value response;
    if (paths.size() <= 1) {
        std::string prefix;
        response[RestStrings::NODE] = JsonPath(prefix, "/", RestStrings::NODE);
        response[RestStrings::REL] = JsonPath(prefix, "/", RestStrings::REL);
        // qw add cpu_rate memory  db_space and db_config
        response[RestStrings::CPU] = GetCPURate();
        response[RestStrings::MEM] = GetMemory();
        response[RestStrings::DISK] = GetDiskRate();
        response[RestStrings::DBSPACE] = GetDbSpace(galaxy_->GetConfig().dir);
        response[RestStrings::UP_TIME] =
            web::json::value::number((int64_t)state_machine_->GetUpTimeInSeconds());
        response[RestStrings::DBCONFIG] = GetDbConfig(global_config_);
        // qw add finish
        std::string version;
        version.append(std::to_string(lgraph::_detail::VER_MAJOR))
            .append(".")
            .append(std::to_string(lgraph::_detail::VER_MINOR))
            .append(".")
            .append(std::to_string(lgraph::_detail::VER_PATCH));
        response[RestStrings::VER] = web::json::value::string(_TU(version));
        response[RestStrings::BRANCH] = web::json::value::string(_TU(GIT_BRANCH));
        response[RestStrings::COMMIT] = web::json::value::string(_TU(GIT_COMMIT_HASH));
        response[RestStrings::WEB_COMMIT] = web::json::value::string(_TU(WEB_GIT_COMMIT_HASH));
        response[RestStrings::CPP_ID] = web::json::value::string(_TU(CXX_COMPILER_ID));
        response[RestStrings::CPP_VERSION] = web::json::value::string(_TU(CXX_COMPILER_VERSION));
        response[RestStrings::PYTHON_VERSION] = web::json::value::string(_TU(PYTHON_LIB_VERSION));
        return RespondSuccess(request, response);
    }
    if (paths.size() != 2) return RespondBadURI(request);
    // /info/log/?begin_time=time1&end_time=time2&user=user1&num_log=100
    if (paths[1] == RestStrings::LOG) {
        if (!AuditLogger::IsEnabled()) {
            return RespondBadRequest(request, "LOG is disabled.");
        }
        auto query = uri::split_query(uri::decode(request.relative_uri().query()));
        if (query.empty()) {
            return RespondBadRequest(request,
                                     "You must specify search time, user and amount of logs.");
        }
        auto it_begin_time = query.find(RestStrings::BEGIN_TIME);
        auto it_end_time = query.find(RestStrings::END_TIME);
        auto it_user = query.find(RestStrings::USER);
        auto it_num = query.find(RestStrings::NUM_LOG);
        auto it_descending_order = query.find(RestStrings::DESCENDING_ORDER);
        if (it_begin_time == query.end()) {
            return RespondBadRequest(request, "You must specify begin time of search.");
        }
        std::string begin_time, end_time, search_user;
        int num_log = 100;
        bool descending_order = true;
        begin_time = _TS(it_begin_time->second);
        if (it_end_time != query.end())
            end_time = _TS(it_end_time->second);
        else
            end_time = DateTime::LocalNow().ToString();
        if (it_user != query.end())
            search_user = _TS(it_user->second);
        else
            search_user = "";  // all users
        if (it_num != query.end()) {
            size_t s = fma_common::TextParserUtils::ParseT<int>(_TS(it_num->second), num_log);
            if (s == 0) num_log = 100;
        }
        if (it_descending_order != query.end())
            size_t s = fma_common::TextParserUtils::ParseT<bool>(_TS(it_descending_order->second),
                                                                 descending_order);
        FMA_DBG_STREAM(logger_) << " log_order: " << descending_order;

        std::vector<lgraph::AuditLog> logs = AuditLogger::GetInstance().GetLog(
            begin_time, end_time, search_user, num_log, descending_order);
        response = ValueToJson(logs);
        FMA_DBG_STREAM(logger_) << "Get " << logs.size()
                                << " logs.";  //<< " " << _TS(response.serialize());
        return RespondSuccess(request, response);
    }
    if (paths[1] == RestStrings::CPU) {
        response = GetCPURate();
        return RespondSuccess(request, response);
    }
    if (paths[1] == RestStrings::DISK) {
        response = GetDiskRate();
        return RespondSuccess(request, response);
    }
    if (paths[1] == RestStrings::MEM) {
        response = GetMemory();
        return RespondSuccess(request, response);
    }
    if (paths[1] == RestStrings::DBSPACE) {
        response = GetDbSpace(galaxy_->GetConfig().dir);
        return RespondSuccess(request, response);
    }
    if (paths[1] == RestStrings::DBCONFIG) {
        response = GetDbConfig(global_config_);
        return RespondSuccess(request, response);
    }
    if (paths[1] == RestStrings::HA_STATE) {
        std::string state;
        if (!state_machine_->IsInHaMode())
            state = "NO_HA";
        else if (state_machine_->IsCurrentMaster())
            state = "MASTER";
        else
            state = "SLAVE";
        return RespondSuccess(request, ValueToJson(state));
    }
    if (paths[1] == RestStrings::PEERS) {
        if (!state_machine_->IsInHaMode())
            return RespondBadRequest(request, "HA mode is not enabled.");
        auto peers = state_machine_->ListPeers();
        response = VectorToJson(peers);
        return RespondSuccess(request, response);
    }
    if (paths[1] == RestStrings::LEADER) {
        if (!state_machine_->IsInHaMode())
            return RespondBadRequest(request, "HA mode is not enabled.");
        response[RestStrings::RPC_ADDR] = web::json::value(_TU(state_machine_->GetMasterRpcAddr()));
        response[RestStrings::REST_ADDR] =
            web::json::value(_TU(state_machine_->GetMasterRestAddr()));
        return RespondSuccess(request, response);
    }
    if (paths[1] == RestStrings::STATISTICS) {
        response = ValueToJson(state_machine_->GetStats());
        return RespondSuccess(request, response);
    }
    return RespondBadURI(request);
}

void RestServer::HandleGetNode(const std::string& user, AccessControlledDB& db,
                               const web::http::http_request& request,
                               const utility::string_t& relative_path,
                               const std::vector<utility::string_t>& paths) const {
    const std::string& graph_name = db.GetConfig().name;
    BEG_AUDIT_LOG(user, graph_name, lgraph::LogApiType::SingleApi, false, _TS(relative_path));

    // /db/{db_name}/node/...
    web::json::value response;
    if (paths.size() == 3) {  // summary of node
        auto txn = db.CreateReadTxn();
        auto n1 = txn.GetNumLabels(true);
        response[RestStrings::NUM_LABELS] = web::json::value::number(n1);
        response[RestStrings::NUMV] = web::json::value::number(txn.GetLooseNumVertex());
        return RespondSuccess(request, response);
    }
    lgraph::VertexId id;
    TRY_PARSE_STR_ID(request, paths[3], id, "vertex id");
    // /db/{db_name}/node/{id}
    if (paths.size() == 4) {
        auto txn = db.CreateReadTxn();
        auto it = txn.GetVertexIterator(id);
        if (!it.IsValid()) {
            return RespondBadRequest(request, "Vertex does not exist.");
        }
        auto label = txn.GetVertexLabel(it);
        response[RestStrings::LABEL] = web::json::value::string(_TU(label));
        web::json::value response_data;
        GetNodeProperties(txn, it, response_data);
        response[RestStrings::PROP] = response_data;
        return RespondSuccess(request, response);
    }
    // node/{id}/property/...
    if (paths[4] == RestStrings::PROP) {
        auto txn = db.CreateReadTxn();
        auto it = txn.GetVertexIterator(id);
        if (!it.IsValid()) {
            return RespondBadRequest(request, "Vertex does not exist.");
        }
        if (paths.size() == 5) {  // node/{id}/property
            GetNodeProperties(txn, it, response);
            return RespondSuccess(request, response);
        }
        if (paths.size() == 6) {  // node/7/property/{key}
            const auto& field_name = _TS(paths[5]);
            auto field = txn.GetVertexField(it, field_name);
            response = ValueToJson(field);
            return RespondSuccess(request, response);
        }
        return RespondBadURI(request);
    }
    // node/{id}/relationship/...
    if (paths[4] == RestStrings::REL) {
        lgraph::VertexId src = id;
        auto txn = db.CreateReadTxn();
        if (!txn.GetVertexIterator(src).IsValid()) {
            return RespondBadRequest(request, "Source vertex does not exist.");
        }
        web::json::value response_body;
        if (paths.size() == 6) {  // node/{src}/relationship/[out|in|all]
            if (paths[5] == RestStrings::OUTE) {
                auto edges = txn.ListOutEdges(static_cast<lgraph::VertexId>(src));
                response_body = VectorToJson(edges);
                return RespondSuccess(request, response_body);
            }
            if (paths[5] == RestStrings::INE) {
                auto edges = txn.ListInEdges(static_cast<lgraph::VertexId>(src));
                response_body = VectorToJson(edges);
                return RespondSuccess(request, response_body);
            }
            if (paths[5] == RestStrings::ALL) {
                auto edges = txn.ListOutEdges(static_cast<lgraph::VertexId>(src));
                response_body[RestStrings::OUTE] = VectorToJson(edges);
                edges = txn.ListInEdges(static_cast<lgraph::VertexId>(src));
                response_body[RestStrings::INE] = VectorToJson(edges);
                return RespondSuccess(request, response_body);
            }
        }
        return RespondBadURI(request);
    }
    return RespondBadURI(request);
}

void RestServer::HandleGetRelationship(const std::string& user, AccessControlledDB& db,
                                       const web::http::http_request& request,
                                       const utility::string_t& relative_path,
                                       const std::vector<utility::string_t>& paths) const {
    const std::string& graph_name = db.GetConfig().name;
    BEG_AUDIT_LOG(user, graph_name, lgraph::LogApiType::SingleApi, false, _TS(relative_path));

    // /db/{db_name}/relationship
    web::json::value response;
    // /relationship
    if (paths.size() == 3) {
        auto txn = db.CreateReadTxn();
        auto n2 = txn.GetNumLabels(false);
        response[RestStrings::NUM_LABELS] = web::json::value::number(n2);
        return RespondSuccess(request, response);
    }
    // /relationship/uid/...
    EdgeUid euid;
    if (!ExtractEdgeUidFromUri(_TS(paths[3]), euid)) {
        return RespondBadRequest(request, "Failed to parse Edge UID.");
    }
    auto txn = db.CreateReadTxn();
    auto eit = txn.GetOutEdgeIterator(euid, false);
    if (!eit.IsValid()) {
        return RespondBadRequest(request, "Edge does not exist.");
    }
    // /relationship/uid
    if (paths.size() == 4) {
        web::json::value response;
        response[RestStrings::LABEL] = JsonPath(txn.GetEdgeLabel(eit));
        web::json::value response_data;
        GetEdgeProperties(txn, eit, response_data);
        response[RestStrings::PROP] = std::move(response_data);
        return RespondSuccess(request, response);
    }
    FMA_DBG_ASSERT(paths.size() >= 5);
    // /relationship/uid/property
    if (paths.size() == 5) {
        if (paths[4] != RestStrings::PROP) {
            return RespondBadURI(request);
        }
        web::json::value response_data;
        GetEdgeProperties(txn, eit, response_data);
        return RespondSuccess(request, response_data);
    }
    FMA_DBG_ASSERT(paths.size() >= 6);
    // /relationship/uid/property/{field_name}
    if (paths.size() == 6) {
        auto field_name = _TS(paths[5]);
        auto field = txn.GetEdgeField(eit, field_name);
        web::json::value response = ValueToJson(field);
        return RespondSuccess(request, response);
    }
    return RespondBadURI(request);
}

void RestServer::HandleGetLabel(const std::string& user, AccessControlledDB& db,
                                const web::http::http_request& request,
                                const utility::string_t& relative_path,
                                const std::vector<utility::string_t>& paths) const {
    const std::string& graph_name = db.GetConfig().name;
    BEG_AUDIT_LOG(user, graph_name, lgraph::LogApiType::SingleApi, false, _TS(relative_path));

    // /db/{db_name}/label/...
    web::json::value response = web::json::value::object();
    auto txn = db.CreateReadTxn();
    if (paths.size() == 3) {
        auto labels_v = txn.GetAllLabels(true);
        auto labels_e = txn.GetAllLabels(false);
        txn.Abort();
        std::vector<web::json::value> labels;
        for (auto& l : labels_v) labels.push_back(web::json::value::string(_TU(l)));
        response[RestStrings::VERTEX] = web::json::value::array(labels);
        labels.clear();
        for (auto& l : labels_e) labels.push_back(web::json::value::string(_TU(l)));
        response[RestStrings::EDGE] = web::json::value::array(labels);
        return RespondSuccess(request, response);
    }
    FMA_DBG_ASSERT(paths.size() >= 4);
    // /db/{db_name}/label/[node|relationship]
    if (paths.size() == 4) {
        if (paths[3] == RestStrings::NODE) {
            auto labels_v = txn.GetAllLabels(true);
            txn.Abort();
            response = VectorToJson(labels_v);
            return RespondSuccess(request, response);
        } else if (paths[3] == RestStrings::REL) {
            auto labels_e = txn.GetAllLabels(false);
            txn.Abort();
            response = VectorToJson(labels_e);
            return RespondSuccess(request, response);
        } else {
            return RespondBadURI(request);
        }
    }
    // /db/{db_name}/label/[node|relationship]/{label_name}
    if (paths.size() == 5) {
        const auto& label_name = _TS(paths[4]);
        std::vector<lgraph::FieldSpec> fds;
        if (paths[3] == RestStrings::NODE) {
            fds = txn.GetSchema(true, label_name);
        } else if (paths[3] == RestStrings::REL) {
            fds = txn.GetSchema(false, label_name);
        } else {
            return RespondBadURI(request);
        }
        txn.Abort();
        for (auto& fd : fds) {
            web::json::value field_def;
            field_def[RestStrings::TYPE] =
                web::json::value::string(_TU(field_data_helper::FieldTypeName(fd.type)));
            field_def[RestStrings::NULLABLE] = web::json::value::boolean(fd.optional);
            response[_TU(fd.name)] = std::move(field_def);
        }
        return RespondSuccess(request, response);
    }
    return RespondBadURI(request);
}

void RestServer::HandleGetIndex(const std::string& user, AccessControlledDB& db,
                                const http_request& request, const utility::string_t& relative_path,
                                const std::vector<utility::string_t>& paths) const {
    const std::string& graph_name = db.GetConfig().name;
    BEG_AUDIT_LOG(user, graph_name, lgraph::LogApiType::SingleApi, false, _TS(relative_path));

    // /db/{db_name}/index
    web::json::value response;
    // /index
    if (paths.size() == 3) {
        auto txn = db.CreateReadTxn();
        auto idx = txn.ListVertexIndexes();
        txn.Abort();
        response = VectorToJson(idx);
        return RespondSuccess(request, response);
    }
    // /index/{label}
    auto query = uri::split_query(uri::decode(request.relative_uri().query()));
    if (paths.size() == 4 && query.empty()) {
        std::vector<IndexSpec> indexes;
        const auto& label = _TS(paths[3]);
        auto txn = db.CreateReadTxn();
        // TODO: ListVertexIndexByLabel() // NOLINT
        auto idx = txn.ListVertexIndexes();
        txn.Abort();
        for (auto& i : idx) {
            if (i.label == label) {
                indexes.push_back(i);
            }
        }
        response = VectorToJson(indexes);
        return RespondSuccess(request, response);
    }
    // index/{label}/?field={field}&value={value}
    if (paths.size() == 4 && !query.empty()) {
        const auto& label = _TS(paths[3]);
        auto it_key = query.find(RestStrings::FIELD), it_value = query.find(RestStrings::VALUE);
        if (it_key == query.end() || it_value == query.end()) {
            return RespondBadRequest(
                request,
                "To get indexed vids, you must specify the label, field name and field value.");
        }
        const auto& field = _TS(it_key->second);
        const auto& value = _TS(it_value->second);
        auto txn = db.CreateReadTxn();
        auto it = txn.GetVertexIndexIterator(label, field, value, value);
        std::vector<web::json::value> vids;
        while (it.IsValid()) {
            vids.push_back(web::json::value::number(it.GetVid()));
            it.Next();
        }
        txn.Abort();
        response = web::json::value::array(vids);
        return RespondSuccess(request, response);
    }
    return RespondBadURI(request);
}

// /user/   lists all users, operation only allowed for admin
void RestServer::HandleGetUser(const std::string& user, const http_request& request,
                               const utility::string_t& relative_path,
                               const std::vector<utility::string_t>& paths) const {
    BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Security, false, _TS(relative_path));
    web::json::value response;
    switch (paths.size()) {
    case 1:
        {
            // /user
            auto us = galaxy_->ListUsers(user);
            response = ValueToJson(us);
            return RespondSuccess(request, response);
        }
    case 2:
        {
            // /user/{user_name}
            auto u = galaxy_->GetUserInfo(user, _TS(paths[1]));
            response = ValueToJson(u);
            return RespondSuccess(request, response);
        }
    case 3:
        {
            // /user/{user_name}/graph
            if (paths[2] != RestStrings::GRAPH) return RespondBadURI(request);
            auto graphs = galaxy_->ListUserGraphs(user, _TS(paths[1]));
            response = ValueToJson(graphs);
            return RespondSuccess(request, response);
        }
    default:
        {
            return RespondBadURI(request);
        }
    }
}

void RestServer::HandleGetGraph(const std::string& user, const web::http::http_request& request,
                                const utility::string_t& relative_path,
                                const std::vector<utility::string_t>& paths) const {
    BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Security, false, _TS(relative_path));
    // /graph or /graph/{graph_name}
    if (paths.size() == 1) {
        // list all graphs
        auto graphs = galaxy_->ListGraphs(user);
        return RespondSuccess(request, ValueToJson(graphs));
    } else if (paths.size() == 2) {
        auto graph_ref = galaxy_->OpenGraph(user, _TS(paths[1]));
        const DBConfig& conf = graph_ref.GetLightningGraph()->GetConfig();
        return RespondSuccess(request, ValueToJson(conf));
    }
    return RespondBadURI(request);
}

void RestServer::HandleGetRole(const std::string& curr_user, const web::http::http_request& request,
                               const utility::string_t& relative_path,
                               const std::vector<utility::string_t>& paths) const {
    BEG_AUDIT_LOG(curr_user, "", lgraph::LogApiType::Security, false, _TS(relative_path));
    // /graph or /graph/{graph_name}
    if (paths.size() == 1) {
        // list all graphs
        auto roles = galaxy_->ListRoles(curr_user);
        return RespondSuccess(request, ValueToJson(roles));
    } else if (paths.size() == 2) {
        const std::string& role = _TS(paths[1]);
        return RespondSuccess(request, ValueToJson(galaxy_->GetRoleInfo(curr_user, role)));
    }
    return RespondBadURI(request);
}

void RestServer::HandleGetTask(const std::string& user, const web::http::http_request& request,
                               const utility::string_t& relative_path,
                               const std::vector<utility::string_t>& paths) const {
    BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Security, false, _TS(relative_path));
    if (paths.size() != 1) return RespondBadURI(request);
    // /task
    if (!galaxy_->IsAdmin(user)) {
        return RespondUnauthorized(request, "User does not have permission to list tasks.");
    }
    return RespondSuccess(request, ValueToJson(TaskTracker::GetInstance().ListRunningTasks()));
}

#define _GET_PLUGIN_TYPE(_type)                        \
    do {                                               \
        if (paths[2] == RestStrings::CPP)              \
            _type = PluginManager::PluginType::CPP;    \
        else if (paths[2] == RestStrings::PYTHON)      \
            _type = PluginManager::PluginType::PYTHON; \
        else                                           \
            return RespondBadURI(request);             \
    } while (0)

void RestServer::HandleGetPlugin(const std::string& user, const std::string& token,
                                 AccessControlledDB& db, const http_request& request,
                                 const utility::string_t& relative_path,
                                 const std::vector<utility::string_t>& paths) const {
    const std::string& graph_name = db.GetConfig().name;
    BEG_AUDIT_LOG(user, graph_name, lgraph::LogApiType::Plugin, false, _TS(relative_path));

    if (paths.size() < 3) return RespondBadURI(request);
    // /db/{db_name}/cpp_plugin/...
    PluginManager::PluginType type;
    _GET_PLUGIN_TYPE(type);

    if (paths.size() == 3) {
        // /cpp_plugin or /python_plugin returns list of plugins
        return RespondSuccess(request, ValueToJson(db.ListPlugins(type, token)));
    } else if (paths.size() == 4) {
        // /cpp_plugin|python_plugin/{plugin_name}
        PluginCode co;
        bool exists = db.GetPluginCode(type, token, _TS(paths[3]), co);
        if (!exists) {
            return RespondBadRequest(request, "Plugin does not exist.");
        }
        std::string encoded = lgraph_api::base64::Encode(co.code);
        co.code = encoded;
        return RespondSuccess(request, ValueToJson(co));
    }
    return RespondBadURI(request);
}

#define CHECK_IS_MASTER()                                                                       \
    do {                                                                                        \
        if (!state_machine_->IsCurrentMaster()) {                                               \
            return RespondRedirect(request, state_machine_->GetMasterRestAddr(), relative_path, \
                                   true);                                                       \
        }                                                                                       \
    } while (0)

// /login
void RestServer::HandlePostLogin(const web::http::http_request& request,
                                 const utility::string_t& relative_path,
                                 const std::vector<utility::string_t>& paths,
                                 const web::json::value& body) const {
    if (paths.size() != 1) {
        BEG_AUDIT_LOG("", "", lgraph::LogApiType::Security, false, "POST " + _TS(relative_path));
        return RespondBadURI(request);
    }
    std::string username, password;
    if (!ExtractStringField(body, RestStrings::USER, username) ||
        !ExtractStringField(body, RestStrings::PASS, password)) {
        BEG_AUDIT_LOG(username, "", lgraph::LogApiType::Security, false,
                      "POST " + _TS(relative_path));
        return RespondBadRequest(request, "`user` or `password` not specified.");
    }
    BEG_AUDIT_LOG(username, "", lgraph::LogApiType::Security, false, "POST " + _TS(relative_path));
    _HoldReadLock(galaxy_->GetReloadLock());
    std::string token = galaxy_->GetUserToken(username, password);
    if (token.empty()) return RespondUnauthorized(request, "Bad user/password.");
    web::json::value response;
    response[RestStrings::TOKEN] = web::json::value::string(_TU(token));
    response[RestStrings::ISADMIN] = web::json::value(galaxy_->IsAdmin(username));
    return RespondSuccess(request, response);
}

// /refresh
void RestServer::HandlePostRefresh(const std::string& user, const std::string& token,
                                const web::http::http_request& request,
                                const utility::string_t& relative_path,
                                const std::vector<utility::string_t>& paths,
                                const web::json::value& body) const {
    if (paths.size() != 1) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Security, false, "POST " + _TS(relative_path));
        return RespondBadURI(request);
    }
    if (token.empty()) return RespondUnauthorized(request, "Bad token.");
    _HoldReadLock(galaxy_->GetReloadLock());
    std::string new_token = galaxy_->RefreshUserToken(token, user);
    web::json::value response;
    response[RestStrings::TOKEN] = web::json::value::string(_TU(new_token));
    response[RestStrings::ISADMIN] = web::json::value(galaxy_->IsAdmin(user));
    return RespondSuccess(request, response);
}

// /update_token_time
void RestServer::HandlePostUpdateTokenTime(const std::string& user, const std::string& token,
                                const web::http::http_request& request,
                                 const utility::string_t& relative_path,
                                 const std::vector<utility::string_t>& paths,
                                 const web::json::value& body) const {
    if (paths.size() != 1) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Security, false, "POST " + _TS(relative_path));
        return RespondBadURI(request);
    }
    if (token.empty()) return RespondUnauthorized(request, "Bad token.");
    int refresh_time;
    int expire_time;
    if (!ExtractIntField(body, RestStrings::REFRESH_TIME, refresh_time)
        || !ExtractIntField(body, RestStrings::EXPIRE_TIME, expire_time)) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Security, false,
                      "POST " + _TS(relative_path));
        return RespondBadRequest(request, "`refresh_time` or `expire_time` not specified.");
    }
    _HoldReadLock(galaxy_->GetReloadLock());
    galaxy_->ModifyTokenTime(token, refresh_time, expire_time);
    web::json::value response;
    response[RestStrings::ISADMIN] = web::json::value(galaxy_->IsAdmin(user));
    return RespondSuccess(request, response);
}

// /get_token_time
void RestServer::HandlePostGetTokenTime(const std::string& user, const std::string& token,
                                const web::http::http_request& request,
                                 const utility::string_t& relative_path,
                                 const std::vector<utility::string_t>& paths,
                                 const web::json::value& body) const {
    if (paths.size() != 1) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Security, false, "POST " + _TS(relative_path));
        return RespondBadURI(request);
    }
    if (token.empty()) return RespondUnauthorized(request, "Bad token.");
    _HoldReadLock(galaxy_->GetReloadLock());
    std::pair<int64_t, int64_t> time;
    time = galaxy_->GetTokenTime(token);
    int64_t refresh_time = time.first;
    int64_t expire_time = time.second;
    web::json::value response;
    response[RestStrings::REFRESH_TIME] = web::json::value::number(refresh_time);
    response[RestStrings::EXPIRE_TIME] = web::json::value::number(expire_time);
    return RespondSuccess(request, response);
}

// /logout
void RestServer::HandlePostLogout(const std::string& user, const std::string& token,
                                const web::http::http_request& request,
                                 const utility::string_t& relative_path,
                                 const std::vector<utility::string_t>& paths,
                                 const web::json::value& body) const {
    if (paths.size() != 1) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Security, false, "POST " + _TS(relative_path));
        return RespondBadURI(request);
    }
    if (token.empty()) return RespondUnauthorized(request, "Bad token.");
    _HoldReadLock(galaxy_->GetReloadLock());
    if (!galaxy_->UnBindTokenUser(token)) return RespondUnauthorized(request, "Bad token.");
    web::json::value response;
    response[RestStrings::ISADMIN] = web::json::value(galaxy_->IsAdmin(user));
    return RespondSuccess(request, response);
}

// /misc/
//    /misc/sync_meta
void RestServer::HandlePostMisc(const std::string& user, const std::string& token,
                                const web::http::http_request& request,
                                const utility::string_t& relative_path,
                                const std::vector<utility::string_t>& paths,
                                const web::json::value& body) const {
    BEG_AUDIT_LOG(user, "", lgraph::LogApiType::SingleApi, false, "POST " + _TS(relative_path));
    if (paths.size() != 2) return RespondBadURI(request);
    if (_TS(paths[1]) == "sync_meta") {
        // /misc/sync_meta
        LGraphRequest req;
        req.set_token(token);
        req.mutable_ha_request()->mutable_sync_meta_request()->set_confirm(
            "This will cause server to stop responding for a while.");
        LGraphResponse proto_resp = ApplyToStateMachine(req);
        if (proto_resp.error_code() == LGraphResponse::SUCCESS)
            return RespondSuccess(request);
        else
            return RespondInternalError(request, proto_resp.error());
    } else {
        return RespondBadURI(request);
    }
}

// POST /cypher
// executes a cypher query on the graph specified
// input JSON: {"graph":"default", "script":"MATCH (n) return n", "parameters":""}
// output:
void RestServer::HandlePostCypher(const std::string& user, const std::string& token,
                                  const web::http::http_request& request,
                                  const utility::string_t& relative_path,
                                  const std::vector<utility::string_t>& paths,
                                  const web::json::value& body) const {
    // /cypher
    if (paths.size() != 1) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Cypher, false, "POST " + _TS(relative_path));
        return RespondBadURI(request);
    }

#ifdef _WIN32
    return RespondInternalError(request, "Cypher is not supported on windows servers.");
#else
    std::string graph, query;
    if (!ExtractStringField(body, RestStrings::GRAPH, graph)) graph = "";
    if (!ExtractStringField(body, RestStrings::SCRIPT, query)) {
        BEG_AUDIT_LOG(user, graph, lgraph::LogApiType::Cypher, false, "[CYPHER] " + query);
        return RespondBadJSON(request);
    }
    BEG_AUDIT_LOG(user, graph, lgraph::LogApiType::Cypher, false, "[CYPHER]" + query);
    double timeout = 0;
    ExtractTypedField(body, RestStrings::TIMEOUT, timeout);
    web::json::value response;

    LGraphRequest proto_req;
    proto_req.set_token(token);
    std::string name;
    std::string type;
    bool ret = cypher::Scheduler::DetermineReadOnly(query, name, type);
    if (name.empty() || type.empty()) {
        proto_req.set_is_write_op(!ret);
    } else {
        type.erase(remove(type.begin(), type.end(), '\"'), type.end());
        name.erase(remove(name.begin(), name.end(), '\"'), name.end());
        AccessControlledDB db = galaxy_->OpenGraph(user, graph);
        ret = !db.IsReadOnlyPlugin(
            type == "CPP" ? PluginManager::PluginType::CPP : PluginManager::PluginType::PYTHON,
            token, name);
        proto_req.set_is_write_op(!ret);
    }

    CypherRequest* creq = proto_req.mutable_cypher_request();
    creq->set_result_in_json_format(false);
    creq->set_query(query);
    creq->set_graph(graph);
    creq->set_timeout(timeout);
    // TODO: support array of primitive types as parameter // NOLINT
    if (body.has_field(RestStrings::PARAMETERS)) {
        // parameters is a dictionary of key:value
        const auto& parameters = body.at(RestStrings::PARAMETERS);
        if (!parameters.is_object()) {
            return RespondBadJSON(request);
        }
        if (!JsonToFieldNameValueDict(parameters, creq->mutable_param_names(),
                                      creq->mutable_param_values())) {
            return RespondBadJSON(request);
        }
    }

    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);

    if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
        const auto& resp = proto_resp.cypher_response();
        if (resp.Result_case() == CypherResponse::kJsonResult) {
            return RespondSuccess(request, resp.json_result());
        } else if (resp.Result_case() == CypherResponse::kBinaryResult) {
            std::vector<web::json::value> vec_header;
            for (auto& c : resp.binary_result().header()) {
                web::json::value col;
                col["name"] = web::json::value::string(c.name());
                col["type"] = web::json::value::number(c.type());
                vec_header.emplace_back(col);
            }
            std::vector<web::json::value> vec_result;
            for (auto& r : resp.binary_result().result()) {
                std::vector<web::json::value> record;
                for (auto& v : r.values()) {
                    record.emplace_back(ProtoFieldDataToJson(v));
                }
                vec_result.emplace_back(web::json::value::array(record));
            }
            response[RestStrings::HEADER] = web::json::value::array(vec_header);
            response[RestStrings::RESULT] = web::json::value::array(vec_result);
            response[RestStrings::SZ] = web::json::value::number(vec_result.size());
            response[RestStrings::ELAPSED] =
                web::json::value::number(resp.binary_result().elapsed());
            return RespondSuccess(request, response);
        }
    }

    return RespondRSMError(request, proto_resp, relative_path, "Cypher");
#endif
}

void RestServer::HandlePostNode(const std::string& user, const std::string& token,
                                const web::http::http_request& request,
                                const utility::string_t& relative_path,
                                const std::vector<utility::string_t>& paths,
                                const web::json::value& body) const {
    // /db/{db_name}/node/...
    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);
    GraphApiRequest* greq = proto_req.mutable_graph_api_request();
    greq->set_graph(_TS(paths[1]));

    // /node
    if (paths.size() == 3) {
        AddVertexesRequest* req = greq->mutable_add_vertexes_request();
        bool batch_mode;
        if (!ExtractLabel(body, *req->mutable_label()) ||
            !ExtractFieldNamesAndValues(body, req->mutable_fields(), req->mutable_vertexes(),
                                        batch_mode)) {
            BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::SingleApi, true,
                          "POST " + _TS(relative_path));
            return RespondBadJSON(request);
        }
        LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
        if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
            web::json::value response;
            auto& resp = proto_resp.graph_api_response().add_vertexes_response();
            if (batch_mode) {
                std::vector<web::json::value> rets;
                rets.reserve(resp.vid_size());
                for (auto& vid : resp.vid()) {
                    rets.push_back(web::json::value::number(vid));
                }
                web::json::value v;
                response = web::json::value::array(rets);
            } else {
                FMA_DBG_ASSERT(resp.vid_size() == 1);
                response = web::json::value::number(*resp.vid().begin());
            }
            return RespondSuccess(request, response);
        }
        return RespondRSMError(request, proto_resp, relative_path, "Vertex");
        // /node/{src}/relationship
    } else if (paths.size() == 5) {
        if (paths[4] != RestStrings::REL) {
            BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::SingleApi, true,
                          "POST " + _TS(relative_path));
            return RespondBadURI(request);
        }
        lgraph::VertexId src, dst;
        if (!ExtractVidFromString(paths[3], src)) {
            BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::SingleApi, true,
                          "POST " + _TS(relative_path));
            return RespondBadURI(request);
        }
        AddEdgesRequest* req = greq->mutable_add_edges_request();
        auto* edge = req->add_edges();
        if (!ExtractIntField(body, RestStrings::DST, dst) ||
            !ExtractLabel(body, *req->mutable_label()) ||
            !ExtractFieldNameValueDict(body, req->mutable_fields(), edge->mutable_values())) {
            BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::SingleApi, true,
                          "POST " + _TS(relative_path));
            return RespondBadJSON(request);
        }
        edge->set_src(src);
        edge->set_dst(dst);
        LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
        if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
            FMA_DBG_ASSERT(proto_resp.graph_api_response().add_edges_response().eids_size() == 1);
            const auto& resp = proto_resp.graph_api_response().add_edges_response();
            web::json::value response =
                web::json::value::string(_TU(fma_common::StringFormatter::Format(
                    "{}_{}_{}_{}_{}", src, dst, resp.lid(), resp.tid(), *resp.eids().begin())));
            return RespondSuccess(request, response);
        }
        return RespondRSMError(request, proto_resp, relative_path, "Edge");
    }
    BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::SingleApi, true,
                  "POST " + _TS(relative_path));
    return RespondBadURI(request);
}

void RestServer::HandlePostRelationship(const std::string& user, const std::string& token,
                                        const web::http::http_request& request,
                                        const utility::string_t& relative_path,
                                        const std::vector<utility::string_t>& paths,
                                        const web::json::value& body) const {
    // /db/{db_name}/relationship
    if (paths.size() != 3) {
        BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::SingleApi, true,
                      "POST " + _TS(relative_path));
        return RespondBadURI(request);
    }
    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);
    GraphApiRequest* greq = proto_req.mutable_graph_api_request();
    greq->set_graph(_TS(paths[1]));

    AddEdgesRequest* req = greq->mutable_add_edges_request();
    if (!ExtractLabel(body, *req->mutable_label()) ||
        !ExtractObjectArray(body, RestStrings::FIELDS, req->mutable_fields()) ||
        !ExtractEdgeData(body, req->mutable_edges())) {
        BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::SingleApi, true,
                      "POST " + _TS(relative_path));
        return RespondBadJSON(request);
    }
    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
    if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
        FMA_DBG_ASSERT(req->edges_size() ==
                       proto_resp.graph_api_response().add_edges_response().eids_size());
        std::vector<web::json::value> rets;
        rets.reserve(req->edges_size());
        const auto& resp = proto_resp.graph_api_response().add_edges_response();
        const auto& eids = resp.eids();
        const auto& req_edges = req->edges();
        for (int i = 0; i < eids.size(); i++) {
            std::string ed = FMA_FMT("{}_{}_{}_{}_{}", req_edges[i].src(), req_edges[i].dst(),
                                     resp.tid(), resp.lid(), eids[i]);
            rets.emplace_back(JsonPath(relative_path, "/", _TU(ed)));
        }
        web::json::value response = web::json::value::array(std::move(rets));
        return RespondSuccess(request, response);
    }
    return RespondRSMError(request, proto_resp, relative_path, "Edge");
}

void RestServer::HandlePostLabel(const std::string& user, const std::string& token,
                                 const web::http::http_request& request,
                                 const utility::string_t& relative_path,
                                 const std::vector<utility::string_t>& paths,
                                 const web::json::value& body) const {
    // /db/{db_name}/label
    if (paths.size() != 3) {
        BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::SingleApi, true,
                      "POST " + _TS(relative_path));
        return RespondBadURI(request);
    }
    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);
    GraphApiRequest* greq = proto_req.mutable_graph_api_request();
    greq->set_graph(_TS(paths[1]));

    AddLabelRequest* req = greq->mutable_add_label_request();
    if (!ExtractAddLabelRequest(body, req)) {
        BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::SingleApi, true,
                      "POST " + _TS(relative_path));
        return RespondBadJSON(request);
    }
    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
    if (proto_resp.error_code() == LGraphResponse::SUCCESS)
        return RespondSuccess(request);
    else
        return RespondRSMError(request, proto_resp, relative_path, "Label");
}

void RestServer::HandlePostIndex(const std::string& user, const std::string& token,
                                 const web::http::http_request& request,
                                 const utility::string_t& relative_path,
                                 const std::vector<utility::string_t>& paths,
                                 const web::json::value& body) const {
    // /db/{db_name}/index
    if (paths.size() != 3) {
        BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::SingleApi, true,
                      "POST " + _TS(relative_path));
        return RespondBadURI(request);
    }
    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);
    GraphApiRequest* greq = proto_req.mutable_graph_api_request();
    greq->set_graph(_TS(paths[1]));

    AddIndexRequest* req = greq->mutable_add_index_request();
    bool is_unique;
    if (!ExtractStringField(body, RestStrings::LABEL, *req->mutable_label()) ||
        !ExtractStringField(body, RestStrings::FIELD, *req->mutable_field()) ||
        !ExtractBoolField(body, RestStrings::ISUNIQUE, is_unique)) {
        BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::SingleApi, true,
                      "POST " + _TS(relative_path));
        return RespondBadJSON(request);
    }
    req->set_is_unique(is_unique);
    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
    if (proto_resp.error_code() == LGraphResponse::SUCCESS)
        return RespondSuccess(request);
    else
        return RespondRSMError(request, proto_resp, relative_path, "VertexIndex");
}

void RestServer::HandlePostPlugin(const std::string& user, const std::string& token,
                                  const web::http::http_request& request,
                                  const utility::string_t& relative_path,
                                  const std::vector<utility::string_t>& paths,
                                  const web::json::value& body) const {
    // /db/{db_name}/cpp_plugin
    LGraphRequest proto_req;
    proto_req.set_token(token);
    PluginRequest* preq = proto_req.mutable_plugin_request();
    preq->set_graph(_TS(paths[1]));

    PluginManager::PluginType type;
    _GET_PLUGIN_TYPE(type);
    preq->set_type(type == PluginManager::PluginType::CPP ? PluginRequest::CPP
                                                          : PluginRequest::PYTHON);

    if (paths.size() == 3) {
        // load plugin
        CHECK_IS_MASTER();
        proto_req.set_is_write_op(true);

        LoadPluginRequest* req = preq->mutable_load_plugin_request();
        bool read_only = false;
        std::string code;
        if (!ExtractStringField(body, RestStrings::NAME, *req->mutable_name()) ||
            !ExtractBoolField(body, RestStrings::READONLY, read_only) ||
            !ExtractStringField(body, RestStrings::CODE, code)) {
            BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::Plugin, true,
                          "POST " + _TS(relative_path));
            return RespondBadJSON(request);
        }
        req->set_read_only(read_only);
        {
            std::vector<unsigned char> decoded = utility::conversions::from_base64(_TU(code));
            req->set_code(std::string(decoded.begin(), decoded.end()));
        }
        ExtractStringField(body, RestStrings::DESC, *req->mutable_desc());
        LoadPluginRequest::CodeType code_type = (type == PluginManager::PluginType::CPP)
                                                    ? LoadPluginRequest::SO
                                                    : LoadPluginRequest::PY;
        std::string code_type_string;
        if (ExtractStringField(body, RestStrings::CODE_TYPE, code_type_string)) {
            if (code_type_string == "so") {
                code_type = LoadPluginRequest::SO;
            } else if (code_type_string == "py") {
                code_type = LoadPluginRequest::PY;
            } else if (code_type_string == "zip") {
                code_type = LoadPluginRequest::ZIP;
            } else if (code_type_string == "cpp") {
                code_type = LoadPluginRequest::CPP;
            } else {
                BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::Plugin, true,
                              "POST " + _TS(relative_path));
                return RespondBadRequest(request,
                                         FMA_FMT("Invalid code_type [{}].", code_type_string));
            }
        }
        req->set_code_type(code_type);
        LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
        if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
            return RespondSuccess(request);
        }
        return RespondRSMError(request, proto_resp, relative_path, "Plugin");
    } else if (paths.size() == 4) {
        // /cpp_plugin/{plugin_name} or /python_plugin/{plugin_name}
        // call plugin
        const std::string& graph = _TS(paths[1]);
        preq->set_graph(graph);
        const std::string& pname = _TS(paths[3]);
        CallPluginRequest* req = preq->mutable_call_plugin_request();
        req->set_name(pname);
        double timeout = 0;
        ExtractDoubleField(body, RestStrings::TIMEOUT, timeout);
        req->set_timeout(timeout);
        bool in_process = false;
        ExtractBoolField(body, RestStrings::INPROCESS, in_process);
        req->set_in_process(in_process);
        std::string param;
        ExtractStringField(body, RestStrings::DATA, param);
        req->set_param(std::move(param));
        FMA_DBG_STREAM(logger_) << "Forwarding to state machine";
        LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
        if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
            FMA_DBG_STREAM(logger_) << "Plugin request successfully returned";
            web::json::value body;
            body[RestStrings::RESULT] =
                web::json::value(_TU(proto_resp.plugin_response().call_plugin_response().reply()));
            return RespondSuccess(request, body);
        } else {
            FMA_DBG_STREAM(logger_) << "Plugin request failed with RSM error "
                                    << proto_resp.error_code() << ": " << proto_resp.error();
            return RespondRSMError(request, proto_resp, relative_path, "Plugin");
        }
    }
    BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::Plugin, true,
                  "POST " + _TS(relative_path));
    return RespondBadURI(request);
}

void RestServer::HandlePostSubGraph(const std::string& user, const std::string& token,
                                    const web::http::http_request& request,
                                    const utility::string_t& relative_path,
                                    const std::vector<utility::string_t>& paths,
                                    const web::json::value& body) const {
    // /db/{db_name}/misc/sub_graph
    if (paths.size() != 4 || paths[3] != RestStrings::SUB_GRAPH) {
        BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::SingleApi, false,
                      "POST " + _TS(relative_path));
        return RespondBadURI(request);
    }

    LGraphRequest proto_req;
    proto_req.set_is_write_op(false);
    proto_req.set_token(token);
    GraphApiRequest* greq = proto_req.mutable_graph_api_request();
    greq->set_graph(_TS(paths[1]));

    SubGraphRequest* sreq = greq->mutable_sub_graph_request();
    if (!ExtractPODArray(body, RestStrings::VIDS, sreq->mutable_vids())) {
        BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::SingleApi, false,
                      "POST " + _TS(relative_path));
        return RespondBadJSON(request);
    }
    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
    if (proto_resp.error_code() != LGraphResponse::SUCCESS) {
        return RespondRSMError(request, proto_resp, relative_path, "SubGraph");
    }
    web::json::value response;
    std::vector<web::json::value> res_nodes, res_relps;
    const SubGraphResponse& sresp = proto_resp.graph_api_response().sub_graph_response();
    for (auto& node : sresp.nodes()) {
        web::json::value jnode;
        jnode[RestStrings::VID] = web::json::value::number(node.vid());
        jnode[RestStrings::LABEL] = web::json::value(_TU(node.label()));
        web::json::value props;
        for (auto& p : node.properties()) {
            props[_TU(p.key())] = ProtoFieldDataToJson(p.value());
        }
        jnode[RestStrings::PROPS] = props;
        res_nodes.emplace_back(jnode);
    }
    for (auto& relp : sresp.edges()) {
        web::json::value jrelp;
        jrelp[RestStrings::SRC] = web::json::value::number(relp.src());
        jrelp[RestStrings::DST] = web::json::value::number(relp.dst());
        jrelp[RestStrings::EUID] = web::json::value::string(_TU(fma_common::StringFormatter::Format(
            "{}_{}_{}_{}_{}", relp.src(), relp.dst(), relp.lid(), relp.tid(), relp.eid())));
        jrelp[RestStrings::LABEL] = web::json::value(_TU(relp.label()));
        web::json::value props;
        for (auto& p : relp.properties()) {
            props[_TU(p.key())] = ProtoFieldDataToJson(p.value());
        }
        jrelp[RestStrings::PROPS] = std::move(props);
        res_relps.emplace_back(jrelp);
    }
    response[RestStrings::NODES] = web::json::value::array(res_nodes);
    response[RestStrings::RELS] = web::json::value::array(res_relps);
    return RespondSuccess(request, response);
}

void RestServer::HandlePostSchema(const std::string& user, const std::string& token,
                                  const web::http::http_request& request,
                                  const utility::string_t& relative_path,
                                  const std::vector<utility::string_t>& paths,
                                  const web::json::value& body) const {
    // /db/{db_name}/schema/text
    if (paths.size() != 4 || paths[3] != RestStrings::SCHEMA_TEXT) {
        BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::SingleApi, true,
                      "POST " + _TS(relative_path));
        return RespondBadURI(request);
    }

    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);
    SchemaRequest* req = proto_req.mutable_schema_request();
    const std::string& graph = _TS(paths[1]);
    req->set_graph(graph);

    std::string description;
    GET_REQUIRED_JSON_FIELD(body, "description", description);

    req->set_description(description);
    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
    if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
        std::string log;
        if (!proto_resp.has_schema_response())
            throw std::runtime_error("should have schema response!");
        if (proto_resp.schema_response().has_error_message())
            return RespondBadRequest(request, proto_resp.schema_response().error_message());
        if (proto_resp.schema_response().has_log())
            log = std::move(*proto_resp.mutable_schema_response()->mutable_log());
        web::json::value response;
        response[_TU("log")] = web::json::value::string(_TU(log));
        return RespondSuccess(request, response);
    } else {
        return RespondRSMError(request, proto_resp, relative_path, "Schema");
    }
}

void RestServer::HandlePostExport(const std::string& user, const std::string& token,
                                  const web::http::http_request& request,
                                  const utility::string_t& relative_path,
                                  const std::vector<utility::string_t>& paths,
                                  const web::json::value& body) const {
    std::string label;
    std::vector<std::string> properties;
    bool is_vertex;
    std::string src_label, dst_label;
    GET_REQUIRED_JSON_FIELD(body, "label", label);
    GET_REQUIRED_JSON_FIELD(body, "properties", properties);
    GET_REQUIRED_JSON_FIELD(body, "is_vertex", is_vertex);
    if (!is_vertex) {
        GET_REQUIRED_JSON_FIELD(body, "src_label", src_label);
        GET_REQUIRED_JSON_FIELD(body, "dst_label", dst_label);
    }
    if (properties.empty()) {
        return RespondBadRequest(request, FMA_FMT("properties is empty"));
    }
    if (label.empty()) {
        return RespondBadRequest(request, FMA_FMT("label is empty"));
    }

    AccessControlledDB db = galaxy_->OpenGraph(user, _TS(paths[1]));
    auto txn = db.CreateReadTxn();
    auto schema = txn.GetSchema(label, is_vertex);
    if (!schema) {
        return RespondBadRequest(
            request, FMA_FMT("No such {} label: {}", is_vertex ? "vertex" : "edge", label));
    }
    auto label_id = schema->GetLabelId();
    size_t NOT_FOUND = size_t(-1);
    size_t src_id_index = NOT_FOUND, dst_id_index = NOT_FOUND;
    size_t edge_prop_size = 0;
    if (!is_vertex) {
        edge_prop_size = properties.size();
        for (size_t i = 0; i < properties.size(); i++) {
            if (properties[i] == "SRC_ID") {
                src_id_index = i;
            } else if (properties[i] == "DST_ID") {
                dst_id_index = i;
            }
        }
        if (src_id_index != NOT_FOUND) {
            auto iter = std::find_if(properties.begin(), properties.end(),
                                     [](auto& item){return item == "SRC_ID";});
            properties.erase(iter);
        }
        if (dst_id_index != NOT_FOUND) {
            auto iter = std::find_if(properties.begin(), properties.end(),
                                     [](auto& item){return item == "DST_ID";});
            properties.erase(iter);
        }
    }
    auto field_ids = schema->GetFieldIds(properties);

    Concurrency::streams::producer_consumer_buffer<uint8_t> buffer;
    http_response response = GetCorsResponse(status_codes::OK);
    // enable chunked transfer
    response.headers().add(header_names::transfer_encoding, _TU("chunked"));
    response.set_body(buffer.create_istream(), _TU("text/plain"));
    auto response_done = request.reply(response);
    std::string chunk;
    auto write_fields = [&](const std::vector<FieldData>& fds){
        nlohmann::json line;
        for (auto& fd : fds) {
            line.push_back(fd.ToString());
        }
        chunk.append(line.dump()).append(1, '\n');
        if (chunk.size() > 1024*1024) {
            // TODO(botu) : Is there a better way to confirm the other peer is disconnected?
            if (response_done.is_done()) {
                FMA_ERR() << "response is done, the client may have been disconnected";
                return false;
            }
            auto size = buffer.putn_nocopy((const uint8_t*)chunk.data(), chunk.size()).get();
            if (size != chunk.size()) {
                FMA_ERR() << FMA_FMT("unexpected size {}", size);
                return false;
            }
            chunk.clear();
        }
        return true;
    };

    if (is_vertex) {
        for (auto iter = txn.GetVertexIndexIterator(label, schema->GetPrimaryField());
             iter.IsValid(); iter.Next()) {
            auto vit = txn.GetVertexIterator(iter.GetVid());
            auto fds = txn.GetVertexFields(vit, field_ids);
            if (!write_fields(fds)) {
                // if the client is disconnected, abort data scanning
                break;
            }
        }
    } else {
        auto src_schema = txn.GetSchema(src_label, true);
        if (!src_schema) {
            return RespondBadRequest(
                request, FMA_FMT("No such vertex label: {}", src_label));
        }
        auto dst_schema = txn.GetSchema(dst_label, true);
        if (!dst_schema) {
            return RespondBadRequest(
                request, FMA_FMT("No such vertex label: {}", dst_label));
        }
        auto dst_vit = txn.GetVertexIterator();
        auto src_fid = src_schema->GetFieldId(src_schema->GetPrimaryField());
        auto dst_fid = src_schema->GetFieldId(dst_schema->GetPrimaryField());
        for (auto iter = txn.GetVertexIndexIterator(src_label, src_schema->GetPrimaryField());
             iter.IsValid(); iter.Next()) {
            auto src_vit = txn.GetVertexIterator(iter.GetVid());
            for (auto eit = src_vit.GetOutEdgeIterator(EdgeUid(0, 0, label_id, 0, 0), true);
                 eit.IsValid(); eit.Next()) {
                if (eit.GetLabelId() != label_id) {
                    break;
                }
                dst_vit.Goto(eit.GetDst());
                if (txn.GetVertexLabelId(dst_vit) != dst_schema->GetLabelId()) {
                    continue;
                }
                auto fds = txn.GetEdgeFields(eit, field_ids);
                int index = 0;
                std::vector<FieldData> new_fds;
                for (size_t i = 0; i < edge_prop_size; i++) {
                    if (i == src_id_index) {
                        new_fds.emplace_back(txn.GetVertexField(src_vit, src_fid));
                    } else if (i == dst_id_index) {
                        new_fds.emplace_back(txn.GetVertexField(dst_vit, dst_fid));
                    } else {
                        new_fds.emplace_back(std::move(fds.at(index)));
                        index++;
                    }
                }
                if (!write_fields(new_fds)) {
                    break;
                }
            }
        }
    }

    if (!chunk.empty()) {
        auto size = buffer.putn_nocopy((const uint8_t*)chunk.data(), chunk.size()).get();
        if (size != chunk.size()) {
            FMA_ERR() << FMA_FMT("unexpected size {}", size);
        }
    }
    buffer.sync().get();
    buffer.close(std::ios_base::out).get();
}

void RestServer::HandlePostImport(const std::string& user, const std::string& token,
                                  const web::http::http_request& request,
                                  const utility::string_t& relative_path,
                                  const std::vector<utility::string_t>& paths,
                                  const web::json::value& body) const {
    // /db/{db_name}/import/text
    if (paths.size() != 4 || paths[3] != RestStrings::IMPORT_TEXT) {
        BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::SingleApi, true,
                      "POST " + _TS(relative_path));
        return RespondBadURI(request);
    }

    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);
    ImportRequest* req = proto_req.mutable_import_request();
    const std::string& graph = _TS(paths[1]);
    req->set_graph(graph);

    std::string description, data;
    bool continue_on_error = false;
    std::string delimiter = ",";
    GET_REQUIRED_JSON_FIELD(body, "description", description);
    ExtractBoolField(body, _TU("continue_on_error"), continue_on_error);
    ExtractStringField(body, _TU("delimiter"), delimiter);

    if (body.has_string_field(_TU("data"))) {
        GET_REQUIRED_JSON_FIELD(body, "data", data);
    } else if (body.has_object_field(_TU("data"))) {
        auto& dict = body.at(_TU("data"));
        for (size_t i = 0;; i++) {
            auto key = _TU(std::to_string(i));
            if (!dict.has_integer_field(key)) break;
            data += (char)dict.at(key).as_integer();
        }
    } else if (body.has_array_field(_TU("data"))) {
        auto& arr = body.at(_TU("data")).as_array();
        size_t n = arr.size();
        data.resize(n);
        for (size_t i = 0; i < n; i++) data[i] = arr.at(i).as_integer();
    } else {
        return RespondBadRequest(request, "could not determine json data type");
    }

    const size_t limit = ::lgraph::import_v2::ONLINE_IMPORT_LIMIT_HARD;
    if (description.length() >= limit || data.length() >= limit) {
        BEG_AUDIT_LOG(user, _TS(paths[1]), lgraph::LogApiType::SingleApi, true,
                      "POST " + _TS(relative_path));
        return RespondBadRequest(request, "Request too large");
    }

    req->set_continue_on_error(continue_on_error);
    req->set_data(data);
    req->set_description(description);
    req->set_delimiter(delimiter);
    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
    if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
        std::string log;
        if (!proto_resp.has_import_response())
            throw std::runtime_error("should have import response!");
        if (proto_resp.import_response().has_error_message())
            return RespondBadRequest(request, proto_resp.import_response().error_message());
        if (proto_resp.import_response().has_log())
            log = std::move(*proto_resp.mutable_import_response()->mutable_log());
        web::json::value response;
        response[_TU("log")] = web::json::value::string(_TU(log));
        return RespondSuccess(request, response);
    } else {
        return RespondRSMError(request, proto_resp, relative_path, "Import");
    }
}

void RestServer::HandlePutConfig(const std::string& user, const std::string& token,
                                 const web::http::http_request& request,
                                 const utility::string_t& relative_path,
                                 const std::vector<utility::string_t>& paths,
                                 const web::json::value& body) const {
    // /config
    if (paths.size() != 1) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Security, true, "PUT " + _TS(relative_path));
        return RespondBadURI(request);
    }

    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);

    ModConfigRequest* req = proto_req.mutable_config_request()->mutable_mod_config_request();
    std::map<std::string, FieldData> params;
    if (!JsonToType(body, params) || params.empty()) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Security, true, "PUT " + _TS(relative_path));
        return RespondBadJSON(request);
    }
    for (auto& kv : params) {
        req->add_keys(kv.first);
        FieldDataConvert::FromLGraphT(kv.second, req->add_values());
    }
    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
    if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
        return RespondSuccess(request);
    }
    return RespondRSMError(request, proto_resp, relative_path, "Config");
}

void RestServer::HandlePostUser(const std::string& user, const std::string& token,
                                const web::http::http_request& request,
                                const utility::string_t& relative_path,
                                const std::vector<utility::string_t>& paths,
                                const web::json::value& body) const {
    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);
    if (paths.size() == 1) {
        // /user
        auto* req = proto_req.mutable_acl_request()->mutable_add_user_request();
        if (!ExtractStringField(body, RestStrings::PASS, *req->mutable_password()) ||
            !ExtractStringField(body, RestStrings::USER, *req->mutable_user())) {
            BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Security, true,
                          "POST " + _TS(relative_path));
            return RespondBadJSON(request);
        }
        ExtractStringField(body, RestStrings::DESC, *req->mutable_desc());
    } else if (paths.size() == 3) {
        // /user/{user_name}/[enable | disable | password | roles | graph]
        auto* req = proto_req.mutable_acl_request()->mutable_mod_user_request();
        req->set_user(_TS(paths[1]));
        auto& action = paths[2];
        if (action == RestStrings::DISABLE) {
            req->set_disable(true);
        } else if (action == RestStrings::ENABLE) {
            req->set_enable(true);
        } else if (action == RestStrings::PASS) {
            std::string oldpass, newpass;
            ExtractStringField(body, RestStrings::CURR_PASS, oldpass);
            if (!ExtractStringField(body, RestStrings::NEW_PASS, newpass))
                return RespondBadRequest(request, "No new password was specified.");
            req->mutable_set_password()->set_new_pass(newpass);
            req->mutable_set_password()->set_old_pass(oldpass);
        } else if (action == RestStrings::DESC) {
            if (!ExtractStringField(body, RestStrings::DESC, *req->mutable_set_desc()))
                return RespondBadJSON(request);
        } else if (action == RestStrings::ROLE) {
            // set roles
            std::set<std::string> roles;
            if (!JsonToType(body, roles))
                return RespondBadRequest(request, "The roles field was not specified.");
            for (auto& r : roles) req->mutable_set_roles()->mutable_values()->Add()->assign(r);
        } else {
            BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Security, true,
                          "PUT " + _TS(relative_path));
            return RespondBadURI(request);
        }
    } else {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Security, true, "POST " + _TS(relative_path));
        return RespondBadURI(request);
    }
    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
    if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
        if (paths.size() == 3 && paths[2] == RestStrings::PASS && !galaxy_->IsAdmin(user)) {
            if (!galaxy_->UnBindTokenUser(token)) return RespondBadRequest(request, "Bad token.");
        }
        return RespondSuccess(request);
    }
    return RespondRSMError(request, proto_resp, relative_path, "User");
}

// POST /db
// Creates a graphDB
// JSON input: {"name":"graph_name", "config":{"max_size_gb":1024, "async":false}}
void RestServer::HandlePostGraph(const std::string& user, const std::string& token,
                                 const web::http::http_request& request,
                                 const utility::string_t& relative_path,
                                 const std::vector<utility::string_t>& paths,
                                 const web::json::value& body) const {
    if (paths.size() != 1) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::SingleApi, true, "POST " + _TS(relative_path));
        return RespondBadURI(request);
    }

    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);
    AddGraphRequest* req = proto_req.mutable_graph_request()->mutable_add_graph_request();
    DBConfig conf;
    if (!ExtractTypedField(body, RestStrings::NAME, *req->mutable_name()) ||
        !ExtractTypedField(body, RestStrings::CONFIG, conf)) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::SingleApi, true, "POST " + _TS(relative_path));
        return RespondBadJSON(request);
    }
    convert::FromLGraphT(conf, req->mutable_config());
    LGraphResponse resp = ApplyToStateMachine(proto_req);
    if (resp.error_code() == LGraphResponse::SUCCESS) {
        return RespondSuccess(request);
    }
    return RespondRSMError(request, resp, relative_path, "Graph");
}

// /role
// Creates a role, or modify its setting
void RestServer::HandlePostRole(const std::string& user, const std::string& token,
                                const web::http::http_request& request,
                                const utility::string_t& relative_path,
                                const std::vector<utility::string_t>& paths,
                                const web::json::value& body) const {
    BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Security, true, "POST " + _TS(relative_path));
    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);
    if (paths.size() == 1) {
        // /role
        // creates a role
        // JSON: {"role":name_of_the_new_role, "description":"description of the role"}
        auto* req = proto_req.mutable_acl_request()->mutable_add_role_request();
        if (!ExtractTypedField(body, RestStrings::ROLE, *req->mutable_role()) ||
            !ExtractTypedField(body, RestStrings::DESC, *req->mutable_desc())) {
            return RespondBadJSON(request);
        }
    } else if (paths.size() == 3) {
        // /role/{role_name}/[desc | disable | enable | graph]
        const std::string& role = _TS(paths[1]);
        auto* req = proto_req.mutable_acl_request()->mutable_mod_role_request();
        req->set_role(role);
        auto& action = paths[2];
        if (action == RestStrings::DESC) {
            // JSON: {"description":"description of the role"}
            if (!ExtractStringField(body, RestStrings::DESC, *req->mutable_mod_desc())) {
                return RespondBadJSON(request);
            }
        } else if (action == RestStrings::DISABLE) {
            req->set_disable(true);
        } else if (action == RestStrings::ENABLE) {
            req->set_enable(true);
        } else if (action == RestStrings::PERMISSIONS) {
            // JSON:
            //     {"graph1":"full", "graph2":"read", ...}
            std::map<std::string, AccessLevel> graph_access;
            if (!JsonToType(body, graph_access)) return RespondBadJSON(request);
            auto access = req->mutable_set_full_graph_access()->mutable_values();
            for (auto& kv : graph_access) (*access)[kv.first] = convert::FromLGraphT(kv.second);
        } else {
            return RespondBadURI(request);
        }
    } else {
        return RespondBadURI(request);
    }
    LGraphResponse resp = ApplyToStateMachine(proto_req);
    if (resp.error_code() == LGraphResponse::SUCCESS) {
        return RespondSuccess(request);
    } else {
        return RespondRSMError(request, resp, relative_path, "Role");
    }
}

void RestServer::HandleDeleteNode(const std::string& user, const std::string& token,
                                  const web::http::http_request& request,
                                  const utility::string_t& relative_path,
                                  const std::vector<utility::string_t>& paths) const {
    BEG_AUDIT_LOG(user, "", lgraph::LogApiType::SingleApi, true, "DELETE " + _TS(relative_path));
    // /db/{db_name}/node/{vid}
    if (paths.size() != 4) {
        return RespondBadURI(request);
    }

    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);
    GraphApiRequest* greq = proto_req.mutable_graph_api_request();
    greq->set_graph(_TS(paths[1]));

    lgraph::VertexId vid;
    if (!ExtractVidFromString(paths[3], vid)) return RespondBadURI(request);
    DelVertexRequest* req = greq->mutable_del_vertex_request();
    req->set_vid(vid);
    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
    if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
        const DelVertexResponse& resp = proto_resp.graph_api_response().del_vertex_response();
        web::json::value body;
        body[RestStrings::INE] = resp.n_ins();
        body[RestStrings::OUTE] = resp.n_outs();
        return RespondSuccess(request, body);
    }
    return RespondRSMError(request, proto_resp, relative_path, "Vertex");
}

void RestServer::HandleDeleteRelationship(const std::string& user, const std::string& token,
                                          const web::http::http_request& request,
                                          const utility::string_t& relative_path,
                                          const std::vector<utility::string_t>& paths) const {
    BEG_AUDIT_LOG(user, "", lgraph::LogApiType::SingleApi, true, "DELETE " + _TS(relative_path));
    // /db/{db_name}/relationship/{euid}
    if (paths.size() != 4) return RespondBadURI(request);

    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);
    GraphApiRequest* greq = proto_req.mutable_graph_api_request();
    greq->set_graph(_TS(paths[1]));

    EdgeUid euid;
    if (!ExtractEdgeUidFromUri(_TS(paths[3]), euid))
        return RespondBadRequest(request, "Failed to parse Edge UID.");
    DelEdgeRequest* req = greq->mutable_del_edge_request();
    req->set_src(euid.src);
    req->set_tid(euid.tid);
    req->set_lid(euid.lid);
    req->set_dst(euid.dst);
    req->set_eid(euid.eid);
    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
    if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
        return RespondSuccess(request);
    }
    return RespondRSMError(request, proto_resp, relative_path, "Edge");
}

void RestServer::HandleDeleteUser(const std::string& user, const std::string& token,
                                  const web::http::http_request& request,
                                  const utility::string_t& relative_path,
                                  const std::vector<utility::string_t>& paths) const {
    // /user/{name}
    if (paths.size() != 2) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Security, true, "DELETE " + _TS(relative_path));
        return RespondBadURI(request);
    }

    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);

    auto* req = proto_req.mutable_acl_request()->mutable_del_user_request();
    req->set_user(_TS(paths[1]));
    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
    if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
        return RespondSuccess(request);
    }
    return RespondRSMError(request, proto_resp, relative_path, "User");
}

// DELETE /db/{graph_name}
void RestServer::HandleDeleteGraph(const std::string& user, const std::string& token,
                                   const web::http::http_request& request,
                                   const utility::string_t& relative_path,
                                   const std::vector<utility::string_t>& paths) const {
    if (paths.size() != 2) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::SingleApi, true,
                      "DELETE " + _TS(relative_path));
        return RespondBadURI(request);
    }

    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);

    DeleteGraphRequest* req = proto_req.mutable_graph_request()->mutable_delete_graph_request();
    req->set_name(_TS(paths[1]));
    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
    if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
        return RespondSuccess(request);
    }
    return RespondRSMError(request, proto_resp, relative_path, "Graph");
}

// DELETE /role/{role_name}
// Deletes the specified role
void RestServer::HandleDeleteRole(const std::string& user, const std::string& token,
                                  const web::http::http_request& request,
                                  const utility::string_t& relative_path,
                                  const std::vector<utility::string_t>& paths) const {
    if (paths.size() != 2) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Security, true, "DELETE " + _TS(relative_path));
        return RespondBadURI(request);
    }
    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);
    auto* req = proto_req.mutable_acl_request()->mutable_del_role_request();
    req->set_role(_TS(paths[1]));
    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
    if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
        return RespondSuccess(request);
    }
    return RespondRSMError(request, proto_resp, relative_path, "Role");
}

void RestServer::HandleDeletePlugin(const std::string& user, const std::string& token,
                                    const web::http::http_request& request,
                                    const utility::string_t& relative_path,
                                    const std::vector<utility::string_t>& paths) const {
    // /db/{db_name}/cpp_plugin/{plugin_name}
    if (paths.size() != 4) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::Plugin, true, "DELETE " + _TS(relative_path));
        return RespondBadURI(request);
    }

    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);

    PluginRequest* preq = proto_req.mutable_plugin_request();
    preq->set_graph(_TS(paths[1]));
    if (paths[2] == RestStrings::CPP) {
        preq->set_type(PluginRequest::CPP);
    } else {
        preq->set_type(PluginRequest::PYTHON);
    }
    preq->mutable_del_plugin_request()->set_name(_TS(paths[3]));
    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
    if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
        return RespondSuccess(request);
    }
    return RespondRSMError(request, proto_resp, relative_path, "Plugin");
}

void RestServer::HandleDeleteIndex(const std::string& user, const std::string& token,
                                   const web::http::http_request& request,
                                   const utility::string_t& relative_path,
                                   const std::vector<utility::string_t>& paths) const {
    // /db/{db_name}/index/{label}/{field}
    if (paths.size() != 5) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::SingleApi, true,
                      "DELETE " + _TS(relative_path));
        return RespondBadURI(request);
    }

    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);
    GraphApiRequest* greq = proto_req.mutable_graph_api_request();
    greq->set_graph(_TS(paths[1]));

    DelIndexRequest* req = proto_req.mutable_graph_api_request()->mutable_del_index_request();
    req->set_label(_TS(paths[3]));
    req->set_field(_TS(paths[4]));
    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
    if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
        return RespondSuccess(request);
    }
    return RespondRSMError(request, proto_resp, relative_path, "VertexIndex");
}

void RestServer::HandleDeleteTask(const std::string& user, const web::http::http_request& request,
                                  const utility::string_t& relative_path,
                                  const std::vector<utility::string_t>& paths) const {
    BEG_AUDIT_LOG(user, "", lgraph::LogApiType::SingleApi, true, "DELETE " + _TS(relative_path));
    _HoldReadLock(galaxy_->GetReloadLock());
    // /task/{thread_id}_{task_id}
    if (paths.size() != 2) {
        return RespondBadURI(request);
    }

    if (!galaxy_->IsAdmin(user)) {
        return RespondUnauthorized(request, "Only admin can kill tasks.");
    }

    TaskTracker::TaskId task_id;
    if (!task_id.FromString(_TS(paths[1]))) {
        return RespondBadRequest(request, "Failed to parse task id.");
    }
    auto r = TaskTracker::GetInstance().KillTask(task_id);
    switch (r) {
    case TaskTracker::SUCCESS:
        return RespondSuccess(request);
    case TaskTracker::NOTFOUND:
        return RespondBadRequest(request, "No such task.");
    case TaskTracker::FAIL_TO_KILL:
        return RespondInternalError(request, "Task did not respond to kill signal.");
    default:
        return RespondInternalError(
            request, fma_common::StringFormatter::Format("Unrecognized error code: {}", r));
    }
}

void RestServer::HandlePutNode(const std::string& user, const std::string& token,
                               const web::http::http_request& request,
                               const utility::string_t& relative_path,
                               const std::vector<utility::string_t>& paths,
                               const web::json::value& body) const {
    // /db/{db_name}/node/{vid}
    if (paths.size() != 4) return RespondBadURI(request);
    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);
    GraphApiRequest* greq = proto_req.mutable_graph_api_request();
    greq->set_graph(_TS(paths[1]));

    lgraph::VertexId vid;
    if (!ExtractVidFromString(paths[3], vid)) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::SingleApi, true, "PUT " + _TS(relative_path));
        return RespondBadURI(request);
    }
    ModVertexRequest* req = proto_req.mutable_graph_api_request()->mutable_mod_vertex_request();
    req->set_vid(vid);
    if (!ExtractFieldNameValueDict(body, req->mutable_fields(), req->mutable_values())) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::SingleApi, true, "PUT " + _TS(relative_path));
        return RespondBadJSON(request);
    }
    // if label is not the same as original one, do a Recreate, otherwise change property
    ExtractStringField(body, RestStrings::LABEL, *req->mutable_label());
    if (req->label().empty()) req->clear_label();
    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
    if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
        return RespondSuccess(request);
    }
    return RespondRSMError(request, proto_resp, relative_path, "Vertex");
}

void RestServer::HandlePutRelationship(const std::string& user, const std::string& token,
                                       const web::http::http_request& request,
                                       const utility::string_t& relative_path,
                                       const std::vector<utility::string_t>& paths,
                                       const web::json::value& body) const {
    // /db/{db_name}/relationship/{euid}
    if (paths.size() != 4) return RespondBadURI(request);
    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token);
    GraphApiRequest* greq = proto_req.mutable_graph_api_request();
    greq->set_graph(_TS(paths[1]));

    EdgeUid euid;
    if (!ExtractEdgeUidFromUri(_TS(paths[3]), euid)) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::SingleApi, true, "PUT " + _TS(relative_path));
        return RespondBadRequest(request, "Failed to parse Edge UID.");
    }
    ModEdgeRequest* req = proto_req.mutable_graph_api_request()->mutable_mod_edge_request();
    req->set_src(euid.src);
    req->set_tid(euid.tid);
    req->set_lid(euid.lid);
    req->set_dst(euid.dst);
    req->set_eid(euid.eid);
    if (!ExtractFieldNameValueDict(body, req->mutable_fields(), req->mutable_values())) {
        BEG_AUDIT_LOG(user, "", lgraph::LogApiType::SingleApi, true, "PUT " + _TS(relative_path));
        return RespondBadJSON(request);
    }
    LGraphResponse proto_resp = ApplyToStateMachine(proto_req);
    if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
        return RespondSuccess(request);
    }
    return RespondRSMError(request, proto_resp, relative_path, "Edge");
}

void RestServer::HandlePutUser(const std::string& user, const std::string& token,
                               const web::http::http_request& request,
                               const utility::string_t& relative_path,
                               const std::vector<utility::string_t>& paths,
                               const web::json::value& body) const {
    return HandlePostUser(user, token, request, relative_path, paths, body);
}

void RestServer::HandlePutRole(const std::string& user, const std::string& token,
                               const web::http::http_request& request,
                               const utility::string_t& relative_path,
                               const std::vector<utility::string_t>& paths,
                               const web::json::value& body) const {
    return HandlePostRole(user, token, request, relative_path, paths, body);
}

void RestServer::handle_delete(http_request request) {
    try {
        _HoldReadLock(galaxy_->GetReloadLock());
        if (!IsClientAddressAllowed(request))
            return RespondUnauthorized(request, "Host address banned.");
        std::string token;
        std::string user = GetUser(request, &token);
        const auto& relative_path = uri::decode(request.relative_uri().path());
        FMA_DBG_STREAM(logger_) << "\n----------------"
                                << "\n[" << user << "]\tDELETE\t" << _TS(relative_path)
                                << "\n----------------";

        auto paths = uri::split_path(relative_path);
        if (paths.empty()) return RespondBadURI(request);
        if (RedirectIfServerTooOld(request, relative_path)) return;
        _ReleaseReadLock();

        RestPathCases fpc = GetRestPathCase(paths[0]);
        switch (fpc) {
        case RestPathCases::TASK:
            CHECK_IS_MASTER();
            return HandleDeleteTask(user, request, relative_path, paths);
        case RestPathCases::USER:
            CHECK_IS_MASTER();
            return HandleDeleteUser(user, token, request, relative_path, paths);
        case RestPathCases::ROLE:
            CHECK_IS_MASTER();
            return HandleDeleteRole(user, token, request, relative_path, paths);
        case RestPathCases::DB:
            {
                if (paths.size() < 2) return RespondBadURI(request);
                // /db/{db_name}
                if (paths.size() == 2) {
                    CHECK_IS_MASTER();
                    return HandleDeleteGraph(user, token, request, relative_path, paths);
                }
                // /db/{db_name}/...
                switch (GetRestPathCase(paths[2])) {
                case RestPathCases::NODE:
                    CHECK_IS_MASTER();
                    return HandleDeleteNode(user, token, request, relative_path, paths);
                case RestPathCases::RELATIONSHIP:
                    CHECK_IS_MASTER();
                    return HandleDeleteRelationship(user, token, request, relative_path, paths);
                case RestPathCases::INDEX:
                    CHECK_IS_MASTER();
                    return HandleDeleteIndex(user, token, request, relative_path, paths);
                case RestPathCases::CPP_PLUGIN:
                case RestPathCases::PYTHON_PLUGIN:
                    CHECK_IS_MASTER();
                    return HandleDeletePlugin(user, token, request, relative_path, paths);
                default:
                    break;
                }
            }
        default:
            return RespondBadURI(request);
        }
    } catch (InputError& e) {
        return RespondBadRequest(request, e.what());
    } catch (AuthError& e) {
        return RespondUnauthorized(request, e.what());
    } catch (std::exception& e) {
        return RespondInternalException(request, e);
    }
}

void RestServer::handle_get(http_request request) {
    try {
        _HoldReadLock(galaxy_->GetReloadLock());
        if (!IsClientAddressAllowed(request))
            return RespondUnauthorized(request, "Host address banned.");
        const auto& relative_path = uri::decode(request.relative_uri().path());
        if (RedirectIfServerTooOld(request, relative_path)) return;
        auto paths = uri::split_path(relative_path);
        if (relative_path.empty() || relative_path == _TU("/")) {
            return HandleGetWeb("", request, relative_path, paths);
        }

        RestPathCases fp = RestPathCases::INVALID;
        if (!paths.empty()) fp = GetRestPathCase(paths[0]);
        if (fp == RestPathCases::INVALID) return RespondBadURI(request);

        std::string user;
        std::string token;
        if (fp != RestPathCases::WEB) user = GetUser(request, &token);
        FMA_DBG_STREAM(logger_) << "\n----------------"
                                << "\n[" << user << "]\tGET\t" << _TS(relative_path)
                                << "\n----------------";

        switch (fp) {
        case RestPathCases::INVALID:
            return RespondBadURI(request);
        case RestPathCases::WEB:
            return HandleGetWeb(user, request, relative_path, paths);
        case RestPathCases::INFO:
            return HandleGetInfo(user, request, relative_path, paths);
        case RestPathCases::USER:
            return HandleGetUser(user, request, relative_path, paths);
        case RestPathCases::ROLE:
            return HandleGetRole(user, request, relative_path, paths);
        case RestPathCases::TASK:
            return HandleGetTask(user, request, relative_path, paths);
        case RestPathCases::GRAPH:
            return HandleGetGraph(user, request, relative_path, paths);
        case RestPathCases::DB:
            {
                // /db
                if (paths.size() == 1) {
                    return HandleGetGraph(user, request, relative_path, paths);
                }
                AccessControlledDB db = galaxy_->OpenGraph(user, _TS(paths[1]));
                if (paths.size() == 2) {
                    web::json::value response = ValueToJson(db.GetConfig());
                    return RespondSuccess(request, response);
                }
                FMA_DBG_CHECK_GE(paths.size(), 3);
                switch (GetRestPathCase(paths[2])) {
                case RestPathCases::NODE:
                    return HandleGetNode(user, db, request, relative_path, paths);
                case RestPathCases::RELATIONSHIP:
                    return HandleGetRelationship(user, db, request, relative_path, paths);
                case RestPathCases::LABEL:
                    return HandleGetLabel(user, db, request, relative_path, paths);
                case RestPathCases::INDEX:
                    return HandleGetIndex(user, db, request, relative_path, paths);
                case RestPathCases::CPP_PLUGIN:
                case RestPathCases::PYTHON_PLUGIN:
                    return HandleGetPlugin(user, token, db, request, relative_path, paths);
                default:
                    return RespondBadURI(request);
                }
            }
        default:
            return RespondBadURI(request);
        }
        FMA_ASSERT(false);  // we should have treated every case in switch()
    } catch (InputError& e) {
        return RespondBadRequest(request, e.what());
    } catch (AuthError& e) {
        return RespondUnauthorized(request, e.what());
    } catch (std::exception& e) {
        return RespondInternalException(request, e);
    }
}

void RestServer::handle_post(http_request request) {
    /* we cannot do a blocking task get() inside an async task */
    request.extract_json().then([request, this](pplx::task<web::json::value> value) {
        web::json::value body;
        try {
            body = value.get();
        } catch (std::exception& e) {
            return RespondBadRequest(request, std::string("Failed to extract JSON: ") + e.what());
        }
        return do_handle_post(request, body);
    });
}

void RestServer::do_handle_post(http_request request, const web::json::value& body) {
    try {
        _HoldReadLock(galaxy_->GetReloadLock());
        if (!IsClientAddressAllowed(request))
            return RespondUnauthorized(request, "Host address banned.");
        const auto& relative_path = uri::decode(request.relative_uri().path());
        auto paths = uri::split_path(relative_path);
        if (paths.empty()) return RespondBadURI(request);
        if (RedirectIfServerTooOld(request, relative_path)) return;
        RestPathCases fpc = GetRestPathCase(paths[0]);
        std::string token;
        std::string user;
        if (fpc != RestPathCases::LOGIN) user = GetUser(request, &token);
        if (fpc != RestPathCases::LOGIN && fpc != RestPathCases::REFRESH
            && fpc != RestPathCases::UpdateTokenTime && fpc != RestPathCases::GetTokenTime
            && !galaxy_->JudgeRefreshTime(token)) {
            throw AuthError("The token is unvalid.");
            exit(-1);
        }
        FMA_DBG_STREAM(logger_) << "\n----------------"
                                << "\n[" << user << "]\tPOST\t" << _TS(relative_path) << "\n"
                                << _TS(body.serialize()).substr(0, 1024) << "\n--------------";

        // release read lock in case request is handled in another thread and requires
        // write lock
        _ReleaseReadLock();
        switch (fpc) {
        case RestPathCases::MISC:
            return HandlePostMisc(user, token, request, relative_path, paths, body);
        case RestPathCases::LOGIN:
            return HandlePostLogin(request, relative_path, paths, body);
        case RestPathCases::CYPHER:
            return HandlePostCypher(user, token, request, relative_path, paths, body);
        case RestPathCases::USER:
            CHECK_IS_MASTER();
            return HandlePostUser(user, token, request, relative_path, paths, body);
        case RestPathCases::ROLE:
            CHECK_IS_MASTER();
            return HandlePostRole(user, token, request, relative_path, paths, body);
        case RestPathCases::DB:
            {
                // /db
                if (paths.size() == 1) {
                    CHECK_IS_MASTER();
                    return HandlePostGraph(user, token, request, relative_path, paths, body);
                }
                // /db/{db_name}/...
                if (paths.size() < 3) return RespondBadURI(request);
                switch (GetRestPathCase(paths[2])) {
                case RestPathCases::NODE:
                    CHECK_IS_MASTER();
                    return HandlePostNode(user, token, request, relative_path, paths, body);
                case RestPathCases::RELATIONSHIP:
                    CHECK_IS_MASTER();
                    return HandlePostRelationship(user, token, request, relative_path, paths, body);
                case RestPathCases::LABEL:
                    CHECK_IS_MASTER();
                    return HandlePostLabel(user, token, request, relative_path, paths, body);
                case RestPathCases::INDEX:
                    CHECK_IS_MASTER();
                    return HandlePostIndex(user, token, request, relative_path, paths, body);
                case RestPathCases::CPP_PLUGIN:
                case RestPathCases::PYTHON_PLUGIN:
                    return HandlePostPlugin(user, token, request, relative_path, paths, body);
                case RestPathCases::MISC:
                    return HandlePostSubGraph(user, token, request, relative_path, paths, body);
                case RestPathCases::IMPORT:
                    CHECK_IS_MASTER();
                    return HandlePostImport(user, token, request, relative_path, paths, body);
                case RestPathCases::EXPORT:
                    return HandlePostExport(user, token, request, relative_path, paths, body);
                case RestPathCases::SCHEMA:
                    CHECK_IS_MASTER();
                    return HandlePostSchema(user, token, request, relative_path, paths, body);
                default:
                    return RespondBadURI(request);
                }
            }
        case RestPathCases::REFRESH:
            return HandlePostRefresh(user, token, request, relative_path, paths, body);
        case RestPathCases::UpdateTokenTime:
            return HandlePostUpdateTokenTime(user, token, request, relative_path, paths, body);
        case RestPathCases::GetTokenTime:
            return HandlePostGetTokenTime(user, token, request, relative_path, paths, body);
        case RestPathCases::LOGOUT:
            return HandlePostLogout(user, token, request, relative_path, paths, body);
        default:
            return RespondBadURI(request);
        }
        FMA_ASSERT(false);
    } catch (InputError& e) {
        return RespondBadRequest(request, e.what());
    } catch (AuthError& e) {
        return RespondUnauthorized(request, e.what());
    } catch (std::exception& e) {
        return RespondInternalException(request, e);
    }
}

void RestServer::handle_put(http_request request) {
    /* we cannot do a blocking task get() inside an async task */
    request.extract_json().then([request, this](pplx::task<web::json::value> value) {
        web::json::value body;
        try {
            body = value.get();
        } catch (std::exception& e) {
            return RespondBadRequest(request, std::string("Failed to extract JSON: ") + e.what());
        }
        return do_handle_put(request, body);
    });
}

void RestServer::do_handle_put(http_request request, const web::json::value& body) {
    try {
        _HoldReadLock(galaxy_->GetReloadLock());
        if (!IsClientAddressAllowed(request))
            return RespondUnauthorized(request, "Host address banned.");
        std::string token;
        std::string user = GetUser(request, &token);
        const auto& relative_path = uri::decode(request.relative_uri().path());
        FMA_DBG_STREAM(logger_) << "\n----------------"
                                << "\n[" << user << "]\tPUT\t" << _TS(relative_path) << "\n"
                                << _TS(body.serialize()).substr(0, 1024) << "\n----------------";

        auto paths = uri::split_path(relative_path);
        if (paths.empty()) return RespondBadURI(request);

        if (RedirectIfServerTooOld(request, relative_path)) return;
        CHECK_IS_MASTER();

        // release read lock in case request is handled in another thread and requires
        // write lock
        _ReleaseReadLock();

        RestPathCases fpc = GetRestPathCase(paths[0]);
        switch (fpc) {
        case RestPathCases::DB:
            {
                if (paths.size() < 3) return RespondBadURI(request);
                switch (GetRestPathCase(paths[2])) {
                case RestPathCases::NODE:
                    return HandlePutNode(user, token, request, relative_path, paths, body);
                case RestPathCases::RELATIONSHIP:
                    return HandlePutRelationship(user, token, request, relative_path, paths, body);
                default:
                    return RespondBadURI(request);
                }
            }
        case RestPathCases::USER:
            return HandlePutUser(user, token, request, relative_path, paths, body);
        case RestPathCases::ROLE:
            return HandlePutRole(user, token, request, relative_path, paths, body);
        case RestPathCases::CONFIG:
            return HandlePutConfig(user, token, request, relative_path, paths, body);
        default:
            return RespondBadURI(request);
        }
    } catch (InputError& e) {
        return RespondBadRequest(request, e.what());
    } catch (AuthError& e) {
        return RespondUnauthorized(request, e.what());
    } catch (std::exception& e) {
        return RespondInternalException(request, e);
    }
}

void RestServer::handle_options(http_request request) {
    FMA_DBG_STREAM(logger_) << "OPTIONS: \n" << _TS(request.to_string());
    /* server-specific OPTIONS */
    return RespondSuccess(request);
}
}  // namespace lgraph
