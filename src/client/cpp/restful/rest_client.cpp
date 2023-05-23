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

#include "client/cpp/restful/rest_client.h"
#include "client/cpp/restful/restful_exception.h"
#include "core/lightning_graph.h"
#include "cpprest/http_client.h"
#include "restful/server/json_convert.h"
#include "cpprest/json.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#pragma comment(lib, "winhttp.lib")
#include <windows.h>
#include <winhttp.h>

inline void DisableRedirectCallback(HINTERNET handle) {
    // disable auto redirects
    DWORD dwOptionValue = WINHTTP_DISABLE_REDIRECTS;
    bool sts = WinHttpSetOption(handle, WINHTTP_OPTION_DISABLE_FEATURE, &dwOptionValue,
                                sizeof(dwOptionValue));
    if (!sts) {
        std::cerr << "WINHTTP_DISABLE_REDIRECTS failed.\n";
    }
}
#endif

using namespace utility;            // Common utilities like string conversions
using namespace web;                // Common features like URIs.
using namespace web::http;          // Common HTTP functionality
using namespace web::http::client;  // HTTP client features
using namespace lgraph_api;
using namespace lgraph;

namespace fma_common {
#ifdef _WIN32
// on windows, utility::string_t is not the same as std::string, so we need this convert
static inline typename std::string ToString(const utility::string_t& s) { return _TS(s); }
#endif
}  // namespace fma_common

static bool ExtractEdgeUid(const std::string& str, EdgeUid& uid) {
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

inline void CheckStatusCodeAndThrow(const http_response& response) {
    if (response.status_code() != status_codes::OK) {
        throw lgraph::StatusException(
            FMA_FMT("status_code: {}, error_message: {}", response.status_code(),
                    ToStdString(response.extract_json().get()[_TU("error_message")].as_string())));
    }
}

json::value RestClient::DoPost(const std::string& url, const json::value& body,
                               bool expect_return) {
    http_response response;
    try {
        http_request request;
        for (auto& h : _header) request.headers().add(_TU(h.first), _TU(h.second));
        request.set_method(methods::POST);
        request.set_request_uri(_TU(url));
        request.set_body(body);
        response = static_cast<http_client*>(http_client_)->request(request).get();
    } catch (std::exception& e) {
        throw lgraph::ConnectionException(e.what());
    }
    CheckStatusCodeAndThrow(response);
    return expect_return ? response.extract_json().get() : json::value();
}

json::value RestClient::DoGraphPost(const std::string& db, const std::string& url,
                                    const json::value& body, bool expect_return) {
    return DoPost("/db/" + db + url, body, expect_return);
}

json::value RestClient::DoGet(const std::string& url, bool expect_return) {
    http_response response;
    try {
        http_request request;
        for (auto& h : _header) request.headers().add(_TU(h.first), _TU(h.second));
        request.set_method(methods::GET);
        request.set_request_uri(_TU(url));
        response = static_cast<http_client*>(http_client_)->request(request).get();
    } catch (std::exception& e) {
        throw lgraph::ConnectionException(e.what());
    }
    CheckStatusCodeAndThrow(response);
    return expect_return ? response.extract_json().get() : json::value();
}

json::value RestClient::DoGraphGet(const std::string& db, const std::string& url,
                                   bool expect_return) {
    return DoGet("/db/" + db + url, expect_return);
}

json::value RestClient::GetLabelNum(const std::string& graph) {
    return DoGraphGet(graph, "/relationship", true);
}

json::value RestClient::ListUserLabel(const std::string& graph) {
    return DoGraphGet(graph, "/label", true);
}

json::value RestClient::ListUserGraph(const std::string& user_name) {
    return DoGet(FMA_FMT("/user/{}/graph", user_name), true);
}

json::value RestClient::ListGraphs(const std::string& graph) {
    if (graph.size() == 0) {
        return DoGet("/graph", true);
    } else {
        return DoGet(FMA_FMT("/graph/{}", graph), true);
    }
}

json::value RestClient::ListRoles(const std::string& role) {
    if (role.size() == 0) {
        return DoGet("/role", true);
    } else {
        return DoGet(FMA_FMT("/role/{}", role), true);
    }
}

json::value RestClient::DoDelete(const std::string& url, bool expect_return) {
    http_response response;
    try {
        http_request request;
        for (auto& h : _header) request.headers().add(_TU(h.first), _TU(h.second));
        request.set_method(methods::DEL);
        request.set_request_uri(_TU(url));
        response = static_cast<http_client*>(http_client_)->request(request).get();
    } catch (std::exception& e) {
        throw lgraph::ConnectionException(e.what());
    }
    if (response.status_code() == status_codes::OK && expect_return)
        return response.extract_json().get();
    CheckStatusCodeAndThrow(response);
    return json::value();
}

json::value RestClient::DoGraphDelete(const std::string& db, const std::string& url,
                                      bool expect_return) {
    return DoDelete("/db/" + db + url, expect_return);
}

void RestClient::DoPut(const std::string& url, const json::value& body) {
    http_response response;
    try {
        http_request request;
        for (auto& h : _header) request.headers().add(_TU(h.first), _TU(h.second));
        request.set_method(methods::PUT);
        request.set_request_uri(_TU(url));
        request.set_body(body);
        response = static_cast<http_client*>(http_client_)->request(request).get();
    } catch (std::exception& e) {
        throw lgraph::ConnectionException(e.what());
    }
    CheckStatusCodeAndThrow(response);
}

void RestClient::DoGraphPut(const std::string& db, const std::string& url,
                            const json::value& body) {
    return DoPut("/db/" + db + url, body);
}

void RestClient::SetConfig(std::map<std::string, FieldData>& configs) {
    json::value body;
    body = ValueToJson(configs);
    return DoPut("/config", body);
}

void RestClient::SetMisc() {
    json::value body;
    DoPost("/misc/sync_meta", body, false);
}

RestClient::RestClient(const std::string& url, const std::string& cert_path, size_t timeout_seconds)
    : logger_(fma_common::Logger::Get("lgraph.RestClient")) {
    http_client_config config;
    config.set_validate_certificates(false);
    config.set_timeout(std::chrono::seconds(timeout_seconds));
#ifndef _WIN32
    if (!cert_path.empty()) {
        fma_common::InputFmaStream is(cert_path);
        if (!is.Good()) throw std::runtime_error("Error opening certificate file " + cert_path);
        std::string cert_buf;
        cert_buf.resize(is.Size());
        is.Read(&cert_buf[0], cert_buf.size());
        is.Close();
        config.set_ssl_context_callback([cert_buf](boost::asio::ssl::context& ctx) {
            boost::asio::const_buffer cert(cert_buf.data(), cert_buf.size());
            ctx.add_certificate_authority(cert);
        });
    }
#else
    // on windows, disable auto-redirect
    config.set_nativehandle_options(DisableRedirectCallback);
#endif
    http_client_ = new http_client(_TU(url), config);

    // logger_.SetLevel(fma_common::LogLevel::LL_DEBUG);
    FMA_DBG_STREAM(logger_) << "[RestClient] RestClient succeeded";
}

RestClient::~RestClient() { delete static_cast<http_client*>(http_client_); }

void* RestClient::GetClient() const { return http_client_; }

bool RestClient::Login(const std::string& username, const std::string& password) {
    json::value body;
    body[_TU("user")] = json::value::string(_TU(username));
    body[_TU("password")] = json::value::string(_TU(password));
    auto response = DoPost("/login", body, true);
    if (response.is_null()) return false;
    auto token = response.at(_TU("jwt")).as_string();
    std::string auth_value = std::string("Bearer ") + ToStdString(token);
    _header["Authorization"] = auth_value;
    FMA_DBG_STREAM(logger_) << "[RestClient] Login " << username << " succeeded";
    return true;
}

std::string RestClient::Refresh(const std::string& token) {
    json::value body;
    auto response = DoPost("/refresh", body, true);
    auto new_token = response.at(_TU("jwt")).as_string();
    std::string auth_value = std::string("Bearer ") + ToStdString(new_token);
    _header["Authorization"] = auth_value;
    FMA_DBG_STREAM(logger_) << "[RestClient] Refresh " << " succeeded";
    return _TS(new_token);
}

bool RestClient::Logout(const std::string& token) {
    json::value body;
    auto response = DoPost("/logout", body, true);
    if (response.is_null()) return false;
    return true;
}

std::string RestClient::SendImportData(const std::string& db, const std::string& desc,
                                       const std::string& data, bool continue_on_error,
                                       const std::string& delimiter) {
    json::value body;
    body[_TU("description")] = json::value::string(_TU(desc));
    body[_TU("data")] = json::value::string(_TU(data));
    body[_TU("continue_on_error")] = json::value::boolean(continue_on_error);
    body[_TU("delimiter")] = json::value::string(_TU(delimiter));
    auto resp = DoGraphPost(db, "/import/text", body, true);
    if (resp.has_string_field(_TU("log")))
        return ToStdString(resp.at(_TU("log")).as_string());
    else
        return "";
}

std::string RestClient::SendSchema(const std::string& db, const std::string& desc) {
    json::value body;
    body[_TU("description")] = json::value::string(_TU(desc));
    auto resp = DoGraphPost(db, "/schema/text", body, true);
    if (resp.has_string_field(_TU("log")))
        return ToStdString(resp.at(_TU("log")).as_string());
    else
        return "";
}

bool RestClient::AddVertexLabel(const std::string& db, const std::string& label,
                                const std::vector<FieldSpec>& fds, const std::string& primary) {
    return AddLabel(db, label, fds, true, primary, {});
}

bool RestClient::AddEdgeLabel(const std::string& db, const std::string& label,
                              const std::vector<FieldSpec>& fds,
                              const EdgeConstraints& edge_constraints) {
    return AddLabel(db, label, fds, false, {}, edge_constraints);
}

bool RestClient::AddLabel(
    const std::string& db, const std::string& label, const std::vector<FieldSpec>& fds,
    bool is_vertex, const std::string& primary,
    const std::vector<std::pair<std::string, std::string>>& edge_constraints) {
    json::value body_data;
    body_data[_TU(RestStrings::NAME)] = json::value::string(_TU(label));
    body_data[_TU(RestStrings::FIELDS)] = VectorToJson(fds);
    body_data[_TU(RestStrings::ISV)] = json::value::boolean(is_vertex);
    body_data[_TU(RestStrings::PRIMARY)] = json::value::string(_TU(primary));
    body_data[_TU(RestStrings::EDGE_CONSTRAINTS)] = VectorToJson(edge_constraints);
    DoGraphPost(db, "/label", body_data, true);
    FMA_DBG_STREAM(logger_) << "[RestClient] AddLabel " << label << " succeeded";
    return true;
}

bool RestClient::AddIndex(const std::string& db, const std::string& label, const std::string& field,
                          bool is_unique) {
    json::value data;
    data[_TU(RestStrings::LABEL)] = json::value::string(_TU(label));
    data[_TU(RestStrings::FIELD)] = json::value::string(_TU(field));
    data[_TU(RestStrings::ISUNIQUE)] = json::value::boolean(is_unique);
    DoGraphPost(db, "/index", data, false);
    FMA_DBG_STREAM(logger_) << "[RestClient] AddVertexIndex (" << label << ":" << field
                            << ") succeeded";
    return true;
}

std::vector<std::string> RestClient::ListVertexLabels(const std::string& db) {
    auto response = DoGraphGet(db, "/label/node", true);
    auto resp = response.as_array();
    std::vector<std::string> labels;
    for (auto& l : resp) labels.push_back(_TS(l.as_string()));
    FMA_DBG_STREAM(logger_) << "[RestClient] ListVertexLabels succeeded";
    return labels;
}

std::vector<std::string> RestClient::ListEdgeLabels(const std::string& db) {
    auto response = DoGraphGet(db, "/label/relationship", true);
    auto resp = response.as_array();
    std::vector<std::string> labels;
    for (auto& l : resp) labels.push_back(_TS(l.as_string()));
    FMA_DBG_STREAM(logger_) << "[RestClient] ListEdgeLabels succeeded";
    return labels;
}

std::vector<FieldSpec> RestClient::GetSchema(const std::string& db, bool is_vertex,
                                             const std::string& label_name) {
    auto response = DoGraphGet(
        db, FMA_FMT("/label/{}/{}", is_vertex ? "node" : "relationship", label_name), true);
    std::vector<FieldSpec> fds;
    FieldSpec fs;
    json::value js;
    json::object jsob = response.as_object();
    json::object::iterator iter;
    for (iter = jsob.begin(); iter != jsob.end(); iter++) {
        fs.name = _TS(iter->first);
        js = iter->second;
        field_data_helper::TryGetFieldType(_TS(js.at(RestStrings::TYPE).as_string()), fs.type);
        ExtractBoolField(js, RestStrings::NULLABLE, fs.optional);
        fds.emplace_back(std::move(fs));
    }
    FMA_DBG_STREAM(logger_) << "[RestClient] Get" << (is_vertex ? "Vertex" : "Edge") << "Schema "
                            << label_name << " succeeded";
    return fds;
}

std::vector<FieldSpec> RestClient::GetVertexSchema(const std::string& db,
                                                   const std::string& label) {
    return GetSchema(db, true, label);
}

std::vector<FieldSpec> RestClient::GetEdgeSchema(const std::string& db, const std::string& label) {
    return GetSchema(db, false, label);
}

std::vector<IndexSpec> RestClient::ListIndexes(const std::string& db) {
    auto response = DoGraphGet(db, "/index", true);
    auto resp = response.as_array();
    std::vector<IndexSpec> indexes;
    IndexSpec i;
    for (auto& js : resp) {
        bool b = JsonToType(js, i);
        if (b) {
            indexes.push_back(i);
        } else {
            indexes.clear();
            FMA_WARN_STREAM(logger_) << "[RestClient] ListVertexIndexes failed";
            return indexes;
        }
    }
    FMA_DBG_STREAM(logger_) << "[RestClient] ListVertexIndexes succeeded";
    return indexes;
}

std::vector<IndexSpec> RestClient::ListIndexesAboutLabel(const std::string& db,
                                                         const std::string& label) {
    auto response = DoGraphGet(db, FMA_FMT("/index/{}", label), true);
    std::vector<IndexSpec> indexes;
    auto resp = response.as_array();
    IndexSpec i;
    for (auto& js : resp) {
        bool b = JsonToType(js, i);
        if (b) {
            indexes.push_back(i);
        } else {
            indexes.clear();
            FMA_WARN_STREAM(logger_) << "[RestClient] ListIndexesAboutLabel failed";
            return indexes;
        }
    }
    FMA_DBG_STREAM(logger_) << "[RestClient] ListIndexesAboutLabel succeeded";
    return indexes;
}

void RestClient::DeleteIndex(const std::string& db, const std::string& label,
                             const std::string& field) {
    DoGraphDelete(db, FMA_FMT("/index/{}/{}", label, field), false);
    FMA_DBG_STREAM(logger_) << "[RestClient] DeleteVertexIndex (" << label << ":" << field
                            << ") succeeded";
}

std::vector<int64_t> RestClient::GetVidsByIndex(const std::string& db, const std::string& label,
                                                const std::string& field,
                                                const std::string& value) {
    // index/{label}/?key={field}&value={value}
    auto response =
        DoGraphGet(db, FMA_FMT("/index/{}/?field={}&value={}", label, field, value), true);
    auto resp = response.as_array();
    std::vector<int64_t> vids;
    int64_t vid = -1;
    for (auto& v : resp) {
        JsonToType(v, vid);
        vids.push_back(vid);
    }
    FMA_DBG_STREAM(logger_) << "[RestClient] GetVidsByIndex (" << label << "," << field << ","
                            << value << ") succeeded";
    return vids;
}

int64_t RestClient::AddVertex(const std::string& db, const std::string& label_name,
                              const std::vector<std::string>& field_names,
                              const std::vector<FieldData>& field_values) {
    int64_t ret = -1;
    if (field_names.size() != field_values.size()) {
        FMA_DBG_STREAM(logger_)
            << "[RestClient] AddVertex failed! field_names.size() != field_values.size()";
        return -1;
    }
    json::value js_prop;
    for (size_t i = 0; i < field_names.size(); i++) {
        js_prop[_TU(field_names[i])] = ValueToJson(field_values[i]);
    }
    json::value body_data;
    body_data[_TU(RestStrings::LABEL)] = json::value::string(_TU(label_name));
    body_data[_TU(RestStrings::PROP)] = js_prop;
    auto resp = DoGraphPost(db, "/node", body_data, true);
    if (JsonToType(resp, ret)) {
        FMA_DBG_STREAM(logger_) << "[RestClient] AddVertex succeeded";
        return ret;
    } else {
        FMA_WARN_STREAM(logger_) << "[RestClient] AddVertex failed";
        return -1;
    }
}

std::vector<int64_t> RestClient::AddVertexes(
    const std::string& db, const std::string& label_name,
    const std::vector<std::string>& field_names,
    const std::vector<std::vector<FieldData>>& field_values_vector) {
    std::vector<int64_t> ret;
    int64_t vid = -1;
    auto field_size = field_names.size();
    std::vector<json::value> jsv;
    jsv.reserve(field_values_vector.size());
    for (auto& field_values : field_values_vector) {
        if (field_values.size() != field_size) {
            FMA_WARN_STREAM(logger_)
                << "[RestClient] AddVertexes failed! field_values.size() != field_size";
            return ret;
        }
        jsv.emplace_back(VectorToJson(field_values));
    }
    json::value body_data;
    body_data[_TU(RestStrings::LABEL)] = json::value::string(_TU(label_name));
    body_data[_TU(RestStrings::FIELDS)] = VectorToJson(field_names);
    body_data[_TU(RestStrings::VALUES)] = json::value::array(jsv);
    auto response = DoGraphPost(db, "/node", body_data, true);
    auto resp = response.as_array();
    for (auto& id : resp) {
        JsonToType(id, vid);
        ret.push_back(vid);
    }
    if (ret.size() != field_values_vector.size()) {
        FMA_WARN_STREAM(logger_)
            << "[RestClient] AddVertexes failed! ret.size() != field_values_vector.size()";
        ret.clear();
        return ret;
    } else {
        FMA_DBG_STREAM(logger_) << "[RestClient] AddVertexes succeeded";
        return ret;
    }
}

EdgeUid RestClient::AddEdge(const std::string& db, lgraph::VertexId src, lgraph::VertexId dst,
                            const std::string& label, const std::vector<std::string>& field_names,
                            const std::vector<FieldData>& field_values) {
    EdgeUid ret;
    if (field_names.size() != field_values.size()) {
        FMA_WARN_STREAM(logger_)
            << "[RestClient] AddEdge failed! field_names.size() != field_values.size()";
        return ret;
    }
    json::value js_prop;
    for (size_t i = 0; i < field_names.size(); i++) {
        js_prop[_TU(field_names[i])] = ValueToJson(field_values[i]);
    }
    json::value body_data;
    body_data[_TU(RestStrings::DST)] = json::value::number(dst);
    body_data[_TU(RestStrings::LABEL)] = json::value::string(_TU(label));
    body_data[_TU(RestStrings::PROP)] = js_prop;
    auto response = DoGraphPost(db, FMA_FMT("/node/{}/relationship", src), body_data, true);
    auto resp = response.as_string();
    bool b = ExtractEdgeUid(_TS(resp), ret);
    if (!b)
        FMA_WARN_STREAM(logger_) << "[RestClient] AddEdge (" << src << "," << dst << ") failed";
    else
        FMA_DBG_STREAM(logger_) << "[RestClient] AddEdge (" << src << "," << dst << ") succeeded";
    return ret;
}

std::vector<EdgeUid> RestClient::AddEdges(
    const std::string& db, std::string label, const std::vector<std::string>& field_names,
    const std::vector<std::pair<int64_t, int64_t>>& edges,
    const std::vector<std::vector<FieldData>>& field_values_vector) {
    std::vector<EdgeUid> ret;
    auto field_size = field_names.size();
    auto edge_size = edges.size();
    std::vector<json::value> jsv;
    jsv.reserve(edge_size);
    json::value js;
    if (field_values_vector.size() != edge_size) {
        FMA_WARN_STREAM(logger_)
            << "[RestClient] AddEdges failed! field_values_vector.size() != edge_size";
        return ret;
    }
    for (size_t i = 0; i < edge_size; i++) {
        if (field_values_vector[i].size() != field_size) {
            FMA_WARN_STREAM(logger_)
                << "[RestClient] AddEdges failed! field_values.size() != field_size";
            return ret;
        }
        js[_TU(RestStrings::SRC)] = json::value::number(edges[i].first);
        js[_TU(RestStrings::DST)] = json::value::number(edges[i].second);
        js[_TU(RestStrings::VALUES)] = VectorToJson(field_values_vector[i]);
        jsv.emplace_back(js);
    }
    json::value body_data;
    body_data[_TU(RestStrings::LABEL)] = json::value::string(_TU(label));
    body_data[_TU(RestStrings::FIELDS)] = VectorToJson(field_names);
    body_data[_TU(RestStrings::EDGE)] = json::value::array(jsv);
    auto response = DoGraphPost(db, "/relationship", body_data, true);
    auto resp = response.as_array();
    EdgeUid edge;
    for (auto& r : resp) {
        auto e = fma_common::Split(_TS(r.as_string()), "/");
        if (ExtractEdgeUid(e.back(), edge)) ret.push_back(edge);
    }
    if (ret.size() != edge_size) {
        FMA_WARN_STREAM(logger_) << "[RestClient] AddEdges failed! ret.size() != edge_size";
        ret.clear();
        return ret;
    } else {
        FMA_DBG_STREAM(logger_) << "[RestClient] AddEdges succeeded";
        return ret;
    }
}

// read

std::map<std::string, FieldData> RestClient::GetVertexFields(const std::string& db,
                                                             lgraph::VertexId vid) {
    std::map<std::string, FieldData> ret;
    auto response = DoGraphGet(db, FMA_FMT("/node/{}/property", vid), true);
    json::object jsob = response.as_object();
    json::object::iterator iter;
    FieldData fd;
    for (iter = jsob.begin(); iter != jsob.end(); iter++) {
        JsonToType(iter->second, fd);
        ret.insert(std::make_pair(_TS(iter->first), fd));
    }
    FMA_DBG_STREAM(logger_) << "[RestClient] GetVertexFields " << vid << " succeeded";
    return ret;
}

std::map<std::string, FieldData> RestClient::GetVertex(const std::string& db, lgraph::VertexId vid,
                                                       std::string& label) {
    std::map<std::string, FieldData> ret;
    auto response = DoGraphGet(db, FMA_FMT("/node/{}", vid), true);
    JsonToType(response[RestStrings::LABEL], label);
    json::object jsob = response[RestStrings::PROP].as_object();
    json::object::iterator iter;
    FieldData fd;
    for (iter = jsob.begin(); iter != jsob.end(); iter++) {
        JsonToType(iter->second, fd);
        ret.insert(std::make_pair(_TS(iter->first), fd));
    }
    FMA_DBG_STREAM(logger_) << "[RestClient] GetVertex " << vid << " succeeded";
    return ret;
}

FieldData RestClient::GetVertexField(const std::string& db, lgraph::VertexId vid,
                                     const std::string& field_name) {
    FieldData ret;
    auto resp = DoGraphGet(db, FMA_FMT("/node/{}/property/{}", vid, field_name), true);
    if (JsonToType(resp, ret)) {
        FMA_DBG_STREAM(logger_) << "[RestClient] GetVertexField " << vid << ":" << field_name
                                << " succeeded";
    } else {
        FMA_WARN_STREAM(logger_) << "[RestClient] GetVertexField " << vid << ":" << field_name
                                 << " failed";
    }
    return ret;
}

std::vector<EdgeUid> RestClient::ListOutEdges(const std::string& db, lgraph::VertexId src) {
    std::vector<EdgeUid> ret;
    auto response = DoGraphGet(db, FMA_FMT("/node/{}/relationship/out", src), true);
    auto resp = response.as_array();
    EdgeUid e;
    for (auto& r : resp) {
        bool b = ExtractEdgeUid(_TS(r.as_string()), e);
        if (!b) {
            FMA_WARN_STREAM(logger_) << "[RestClient] ListOutEdges " << src << " failed";
            ret.clear();
            return ret;
        }
        ret.push_back(e);
    }
    FMA_DBG_STREAM(logger_) << "[RestClient] ListOutEdges " << src << " succeeded";
    return ret;
}

std::vector<EdgeUid> RestClient::ListInEdges(const std::string& db, lgraph::VertexId dst) {
    std::vector<EdgeUid> ret;
    auto response = DoGraphGet(db, FMA_FMT("/node/{}/relationship/in", dst), true);
    auto resp = response.as_array();
    EdgeUid e;
    for (auto& r : resp) {
        bool b = ExtractEdgeUid(_TS(r.as_string()), e);
        if (!b) {
            FMA_WARN_STREAM(logger_) << "[RestClient] ListInEdges " << dst << " failed";
            ret.clear();
            return ret;
        }
        ret.push_back(e);
    }
    FMA_DBG_STREAM(logger_) << "[RestClient] ListInEdges " << dst << " succeeded";
    return ret;
}

std::vector<std::vector<EdgeUid>> RestClient::ListAllEdges(const std::string& db,
                                                           lgraph::VertexId vid) {
    std::vector<std::vector<EdgeUid>> ret;
    auto response = DoGraphGet(db, FMA_FMT("/node/{}/relationship/all", vid), true);
    ret.reserve(2);
    auto inEdges = response[RestStrings::INE].as_array();
    auto outEdges = response[RestStrings::OUTE].as_array();
    std::vector<EdgeUid> inEdgesRet;
    std::vector<EdgeUid> outEdgesRet;
    EdgeUid e;
    for (auto& ine : inEdges) {
        bool b = ExtractEdgeUid(_TS(ine.as_string()), e);
        if (!b) {
            FMA_WARN_STREAM(logger_) << "[RestClient] ListAllEdges " << vid << " failed";
            inEdgesRet.clear();
            outEdgesRet.clear();
            return ret;
        }
        inEdgesRet.push_back(e);
    }
    for (auto& oute : outEdges) {
        bool b = ExtractEdgeUid(_TS(oute.as_string()), e);
        if (!b) {
            FMA_WARN_STREAM(logger_) << "[RestClient] ListAllEdges " << vid << " failed";
            inEdgesRet.clear();
            outEdgesRet.clear();
            return ret;
        }
        outEdgesRet.push_back(e);
    }
    ret.push_back(inEdgesRet);
    ret.push_back(outEdgesRet);
    FMA_DBG_STREAM(logger_) << "[RestClient] ListAllEdges " << vid << " succeeded";
    return ret;
}

std::map<std::string, FieldData> RestClient::GetEdge(const std::string& db,
                                                     const lgraph::EdgeUid& euid,
                                                     std::string& label) {
    std::map<std::string, FieldData> ret;
    auto response = DoGraphGet(db, FMA_FMT("/relationship/{}", euid.ToString()), true);
    JsonToType(response[RestStrings::LABEL], label);
    json::object jsob = response[RestStrings::PROP].as_object();
    json::object::iterator iter;
    FieldData fd;
    for (iter = jsob.begin(); iter != jsob.end(); iter++) {
        JsonToType(iter->second, fd);
        ret.insert(std::make_pair(_TS(iter->first), fd));
    }
    FMA_DBG_STREAM(logger_) << "[RestClient] GetEdge " << FMA_FMT("{}", euid.ToString())
                            << " succeeded";
    return ret;
}

std::map<std::string, FieldData> RestClient::GetEdgeFields(const std::string& db,
                                                           const lgraph::EdgeUid& euid) {
    std::map<std::string, FieldData> ret;
    auto response = DoGraphGet(db, FMA_FMT("/relationship/{}/property", euid.ToString()), true);
    json::object jsob = response.as_object();
    json::object::iterator iter;
    FieldData fd;
    for (iter = jsob.begin(); iter != jsob.end(); iter++) {
        JsonToType(iter->second, fd);
        ret.insert(std::make_pair(_TS(iter->first), fd));
    }
    FMA_DBG_STREAM(logger_) << "[RestClient] GetEdgeFields " << FMA_FMT("{}", euid.ToString())
                            << " succeeded";
    return ret;
}

FieldData RestClient::GetEdgeField(const std::string& db, const lgraph::EdgeUid& euid,
                                   const std::string& field_name) {
    FieldData ret;
    auto resp =
        DoGraphGet(db, FMA_FMT("/relationship/{}/property/{}", euid.ToString(), field_name), true);
    if (JsonToType(resp, ret)) {
        FMA_DBG_STREAM(logger_) << "[RestClient] GetEdgeField " << FMA_FMT("{}:", euid.ToString())
                                << field_name << " succeeded";
    } else {
        FMA_WARN_STREAM(logger_) << "[RestClient] GetEdgeField " << FMA_FMT("{}:", euid.ToString())
                                 << field_name << " failed";
    }
    return ret;
}

// update

void RestClient::SetVertexProperty(const std::string& db, lgraph::VertexId vid,
                                   const std::vector<std::string>& field_names,
                                   const std::vector<FieldData>& field_data) {
    if (field_names.size() != field_data.size()) {
        FMA_WARN_STREAM(logger_)
            << "[RestClient] SetVertexProperty failed! field_names.size() != field_data.size()";
        throw std::runtime_error("field_names and field_data should have the same size.");
    }
    json::value js_prop;
    for (size_t i = 0; i < field_names.size(); i++) {
        js_prop[_TU(field_names[i])] = ValueToJson(field_data[i]);
    }
    json::value body_data;
    body_data[_TU(RestStrings::PROP)] = js_prop;
    DoGraphPut(db, FMA_FMT("/node/{}", vid), body_data);
    FMA_DBG_STREAM(logger_) << "[RestClient] SetVertexProperty " << vid << " succeeded";
}

void RestClient::SetEdgeProperty(const std::string& db, const lgraph::EdgeUid& euid,
                                 const std::vector<std::string>& field_names,
                                 const std::vector<FieldData>& field_data) {
    if (field_names.size() != field_data.size()) {
        FMA_WARN_STREAM(logger_)
            << "[RestClient] SetEdgeProperty failed! field_names.size() != field_data.size()";
        throw std::runtime_error("field_names and field_data should have the same size.");
    }
    json::value js_prop;
    for (size_t i = 0; i < field_names.size(); i++) {
        js_prop[_TU(field_names[i])] = ValueToJson(field_data[i]);
    }
    json::value body_data;
    body_data[_TU(RestStrings::PROP)] = js_prop;
    DoGraphPut(db, FMA_FMT("/relationship/{}", euid.ToString()), body_data);
    FMA_DBG_STREAM(logger_) << "[RestClient] SetEdgeProperty " << FMA_FMT("{}", euid.ToString())
                            << " succeeded";
}

// delete

void RestClient::DeleteVertex(const std::string& db, lgraph::VertexId vid, size_t* n_in,
                              size_t* n_out) {
    json::value v = DoGraphDelete(db, FMA_FMT("/node/{}", vid), true);
    *n_in = v.at(_TU(RestStrings::INE)).as_number().to_int64();
    *n_out = v.at(_TU(RestStrings::OUTE)).as_number().to_int64();
    FMA_DBG_STREAM(logger_) << "[RestClient] DeleteVertex " << vid << " succeeded";
}

void RestClient::DeleteEdge(const std::string& db, const lgraph::EdgeUid& euid) {
    DoGraphDelete(db, FMA_FMT("/relationship/{}", euid.ToString()), false);
    FMA_DBG_STREAM(logger_) << "[RestClient] DeleteEdge " << FMA_FMT("{}", euid.ToString())
                            << " succeeded";
}

json::value RestClient::EvalCypher(const std::string& graph, const std::string& s) {
    json::value body;
    body[RestStrings::SCRIPT] = json::value::string(_TU(s));
    body[RestStrings::GRAPH] = json::value::string(_TU(graph));
    auto response = DoPost("/cypher", body, true);
    FMA_DBG_STREAM(logger_) << "[RestClient] EvalCypher succeeded";
    return response;
}

json::value RestClient::EvalCypherWithParam(const std::string& graph, const std::string& s,
                                            const std::map<std::string, FieldData>& param) {
    json::value body;
    body[RestStrings::SCRIPT] = json::value::string(_TU(s));
    body[RestStrings::GRAPH] = json::value::string(_TU(graph));
    body[RestStrings::PARAMETERS] = _ValueToJson<std::map<std::string, FieldData>>::Convert(param);
    auto response = DoPost("/cypher", body, true);
    FMA_DBG_STREAM(logger_) << "[RestClient] EvalCypherWithParam succeeded";
    return response;
}

json::value RestClient::GetSubGraph(const std::string& db, const std::vector<int64_t>& vec_vertex) {
    json::value body;
    body[RestStrings::VIDS] = VectorToJson(vec_vertex);
    auto response = DoGraphPost(
        db, FMA_FMT("/{}/{}", _TS(RestStrings::MISC), _TS(RestStrings::SUB_GRAPH)), body, true);
    FMA_DBG_STREAM(logger_) << "[RestClient] " << __func__ << " succeeded";
    return response;
}

std::map<std::string, lgraph_api::UserInfo> RestClient::ListUsers() {
    std::map<std::string, lgraph_api::UserInfo> ret;
    auto response = DoGet("/user", true);
    if (!JsonToType(response, ret)) {
        std::string err = "Error parsing result:\n" + _TS(response.serialize());
        FMA_ERR_STREAM(logger_) << err;
        throw InternalError(err);
    }
    FMA_DBG_STREAM(logger_) << "[RestClient] ListUsers succeeded";
    return ret;
}

void RestClient::AddUser(const std::string& user_name, bool is_admin, const std::string& password,
                         const std::string& desc) {
    json::value body;
    body[RestStrings::USER] = json::value::string(_TU(user_name));
    body[RestStrings::PASS] = json::value::string(_TU(password));
    if (!desc.empty()) body[RestStrings::DESC] = json::value::string(_TU(desc));
    DoPost("/user", body, false);
    if (is_admin) {
        SetUserRoles(user_name, {lgraph::_detail::ADMIN_ROLE});
    }
    FMA_DBG_STREAM(logger_) << "[RestClient] AddUser " << user_name << " succeeded";
}

void RestClient::SetPassword(const std::string& user, const std::string& curr_pass,
                             const std::string& new_pass) {
    json::value body;
    body[RestStrings::CURR_PASS] = json::value::string(_TU(curr_pass));
    body[RestStrings::NEW_PASS] = json::value::string(_TU(new_pass));
    DoPost(FMA_FMT("/user/{}/{}", user, RestStrings::PASS), body, false);
    FMA_DBG_STREAM(logger_) << "[RestClient] SetPassword " << user << " succeeded";
}

void RestClient::SetUserDesc(const std::string& user, const std::string& desc) {
    json::value body;
    body[RestStrings::DESC] = json::value::string(_TU(desc));
    DoPost(FMA_FMT("/user/{}/{}", user, RestStrings::DESC), body, false);
    FMA_DBG_STREAM(logger_) << "[RestClient] SetPassword " << user << " succeeded";
}

void RestClient::SetUserRoles(const std::string& user, const std::vector<std::string>& roles) {
    json::value body;
    body = ValueToJson(roles);
    DoPost(FMA_FMT("/user/{}/{}", user, RestStrings::ROLE), body, false);
    FMA_DBG_STREAM(logger_) << "[RestClient] SetUserRoles " << user << " succeeded";
}

lgraph_api::UserInfo RestClient::GetUserInfo(const std::string& user) {
    auto resp = DoGet("/user/" + user, true);
    lgraph_api::UserInfo ret;
    if (!JsonToType(resp, ret)) throw InternalError("Error parsing result.");
    return ret;
}

void RestClient::DeleteUser(const std::string& user_name) {
    DoDelete(FMA_FMT("/user/{}", user_name), false);
    FMA_DBG_STREAM(logger_) << "[RestClient] DeleteUser " << user_name << " succeeded";
}

void RestClient::DisableUser(const std::string& user) {
    DoPost(FMA_FMT("/user/{}/disable", user), json::value(), false);
}

void RestClient::EnableUser(const std::string& user) {
    DoPost(FMA_FMT("/user/{}/enable", user), json::value(), false);
}

void RestClient::CreateRole(const std::string& role, const std::string& desc) {
    json::value body;
    body[RestStrings::ROLE] = json::value(_TU(role));
    body[RestStrings::DESC] = json::value(_TU(desc));
    DoPost("/role", body, false);
}

void RestClient::DeleteRole(const std::string& role) { DoDelete("/role/" + role, false); }

void RestClient::DisableRole(const std::string& role) {
    DoPost(FMA_FMT("/role/{}/disable", role), json::value(), false);
}

void RestClient::EnableRole(const std::string& role) {
    DoPost(FMA_FMT("/role/{}/enable", role), json::value(), false);
}

void RestClient::SetRoleGraphAccess(
    const std::string& role, const std::map<std::string, lgraph_api::AccessLevel>& graph_access) {
    json::value body;
    body = ValueToJson(graph_access);
    DoPost(FMA_FMT("/role/{}/{}", role, RestStrings::PERMISSIONS), body, false);
}

void RestClient::SetRoleDesc(const std::string& role, const std::string& desc) {
    json::value body;
    body[RestStrings::ROLE] = json::value(_TU(role));
    body[RestStrings::DESC] = json::value(_TU(desc));
    DoPost(FMA_FMT("/role/{}/{}", role, _TS(RestStrings::DESC)), body, false);
}

bool RestClient::GetServerInfo(RestClient::CPURate& cpu_rate, RestClient::MemoryInfo& memory_info,
                               RestClient::DBSpace& db_space, RestClient::DBConfig& db_config,
                               std::string& lgraph_version, std::string& node,
                               std::string& relationship) {
    auto response = DoGet("/info", true);
    // FMA_DBG_STREAM(logger_) << ToStdString(response.serialize());
    auto js_cpu = response[RestStrings::CPU];
    auto js_mem = response[RestStrings::MEM];
    auto js_db_space = response[RestStrings::DBSPACE];
    auto js_db_config = response[RestStrings::DBCONFIG];
    auto js_version = response[RestStrings::VER];
    auto js_node = response[RestStrings::NODE];
    auto js_rel = response[RestStrings::REL];
    if (!ExtractTypedField(js_cpu, _TU("self"), cpu_rate.self_cpu_rate) ||
        !ExtractTypedField(js_cpu, _TU("server"), cpu_rate.server_cpu_rate) ||
        !ExtractTypedField(js_cpu, _TU("unit"), cpu_rate.unit)) {
        FMA_WARN_STREAM(logger_) << "[RestClient] GetServerInfo:CPU failed";
        return false;
    }
    if (!ExtractTypedField(js_mem, _TU("self"), memory_info.self_memory) ||
        !ExtractTypedField(js_mem, _TU("server_avail"), memory_info.available) ||
        !ExtractTypedField(js_mem, _TU("server_total"), memory_info.total) ||
        !ExtractTypedField(js_mem, _TU("unit"), memory_info.unit)) {
        FMA_WARN_STREAM(logger_) << "[RestClient] GetServerInfo:MEM failed";
        return false;
    }
    if (!ExtractTypedField(js_db_space, _TU("space"), db_space.space) ||
        !ExtractTypedField(js_db_space, _TU("disk_total"), db_space.total) ||
        !ExtractTypedField(js_db_space, _TU("disk_avail"), db_space.avail) ||
        !ExtractTypedField(js_db_space, _TU("unit"), db_space.unit)) {
        FMA_WARN_STREAM(logger_) << "[RestClient] GetServerInfo:DBSPACE failed";
        return false;
    }
    if (js_db_config.is_null()) {
        db_config.valid = false;
    } else {
        db_config.valid = true;
        int64_t port, rpc_port;
        if (!ExtractTypedField(js_db_config, _TU("durable"), db_config.durable) ||
            !ExtractTypedField(js_db_config, _TU("disable_auth"), db_config.disable_auth) ||
            !ExtractTypedField(js_db_config, _TU("enable_rpc"), db_config.enable_rpc) ||
            !ExtractTypedField(js_db_config, _TU("bind_host"), db_config.host) ||
            !ExtractTypedField(js_db_config, _TU("port"), port) ||
            !ExtractTypedField(js_db_config, _TU("thread_limit"), db_config.thread_limit) ||
            !ExtractTypedField(js_db_config, _TU("enable_ssl"), db_config.use_ssl) ||
            !ExtractTypedField(js_db_config, _TU("verbose"), db_config.verbose) ||
            !ExtractTypedField(js_db_config, _TU("enable_audit_log"), db_config.enable_audit) ||
            !ExtractTypedField(js_db_config, _TU("enable_ip_check"), db_config.enable_ip_check) ||
            !ExtractTypedField(js_db_config, _TU("optimistic_txn"), db_config.optimistic_txn)) {
            FMA_WARN_STREAM(logger_) << "[RestClient] GetServerInfo:DBCONFIG failed";
            FMA_LOG() << db_config.disable_auth << db_config.enable_rpc << db_config.thread_limit;
            return false;
        }

        if (db_config.enable_rpc == true) {
            ExtractTypedField(js_db_config, _TU("rpc_port"), rpc_port);
            db_config.rpc_port = (uint16_t)rpc_port;
        }
        db_config.port = (uint16_t)port;
    }
    if (!JsonToType(js_version, lgraph_version)) {
        FMA_WARN_STREAM(logger_) << "[RestClient] GetServerInfo:VER failed";
        return false;
    }
    if (!JsonToType(js_node, node)) {
        FMA_WARN_STREAM(logger_) << "[RestClient] GetServerInfo:NODE failed";
        return false;
    }
    if (!JsonToType(js_rel, relationship)) {
        FMA_WARN_STREAM(logger_) << "[RestClient] GetServerInfo:REL failed";
        return false;
    }
    FMA_DBG_STREAM(logger_) << "[RestClient] GetServerInfo succeeded";
    return true;
}

bool RestClient::LoadPlugin(const std::string& db, lgraph_api::PluginCodeType type,
                            const PluginDesc& plugin_info, const std::string& code) {
    json::value body;
    body[RestStrings::NAME] = json::value::string(_TU(plugin_info.name));
    body[RestStrings::READONLY] = json::value::boolean(plugin_info.read_only);
    body[RestStrings::DESC] = json::value::string(_TU(plugin_info.desc));
    body[RestStrings::CODE] = json::value::string(_TU(lgraph_api::base64::Encode(code)));
    body[RestStrings::CODE_TYPE] = json::value::string(_TU(lgraph_api::PluginCodeTypeStr(type)));
    DoGraphPost(db,
                FMA_FMT("/{}_plugin", type == lgraph_api::PluginCodeType::PY ? "python" : "cpp"),
                body, false);
    FMA_DBG_STREAM(logger_) << "[RestClient] " << __func__ << " succeeded";
    return true;
}

std::vector<PluginDesc> RestClient::GetPlugin(const std::string& db, bool is_cpp) {
    std::vector<PluginDesc> ret;
    auto response = DoGraphGet(db, is_cpp ? "/cpp_plugin" : "/python_plugin");
    auto resp = response.as_array();
    PluginDesc pd;
    for (auto& r : resp) {
        bool b = JsonToType(r, pd);
        if (!b) {
            FMA_WARN_STREAM(logger_) << "[RestClient] " << __func__ << " failed";
            throw InternalError("[RestClient] {} parse response failed", __func__);
            ret.clear();
            return ret;
        }
        ret.push_back(pd);
    }
    FMA_DBG_STREAM(logger_) << "[RestClient] " << __func__ << " succeeded";
    return ret;
}

PluginCode RestClient::GetPluginDetail(const std::string& db, const std::string& name,
                                       bool is_cpp) {
    PluginCode pc;
    json::value response =
        DoGraphGet(db, FMA_FMT("{}/{}", is_cpp ? "/cpp_plugin" : "/python_plugin", name));
    bool b = JsonToType(response, pc);
    if (!b) {
        FMA_WARN_STREAM(logger_) << "[RestClient] " << __func__ << " failed";
        throw InternalError("[RestClient] {} parse response failed", __func__);
    }
    FMA_DBG_STREAM(logger_) << "[RestClient] " << __func__ << " succeeded";
    return pc;
}

std::string RestClient::ExecutePlugin(const std::string& db, bool is_cpp,
                                      const std::string& plugin_name, const std::string& data,
                                      double timeout, bool in_process) {
    json::value body_data;
    if (!data.empty()) body_data[RestStrings::DATA] = json::value::string(_TU(data));
    if (timeout != 0) body_data[RestStrings::TIMEOUT] = json::value::number(timeout);
    if (in_process) body_data[RestStrings::INPROCESS] = json::value::boolean(in_process);
    http_response response;
    try {
        http_request request;
        for (auto& h : _header) request.headers().add(_TU(h.first), _TU(h.second));
        request.set_method(methods::POST);
        request.set_request_uri(
            _TU(FMA_FMT("/db/{}/{}_plugin/{}", db, is_cpp ? "cpp" : "python", plugin_name)));
        request.set_body(body_data);
        response = static_cast<http_client*>(http_client_)->request(request).get();
    } catch (std::exception& e) {
        throw lgraph::ConnectionException(e.what());
    }
    CheckStatusCodeAndThrow(response);
    auto resp = response.extract_json().get();
    FMA_DBG_STREAM(logger_) << "[RestClient] " << __func__ << " succeeded";
    return _TS(resp.at(RestStrings::RESULT).as_string());
}

void RestClient::DeletePlugin(const std::string& db, bool is_cpp, const std::string& plugin_name) {
    DoGraphDelete(db, FMA_FMT("/{}_plugin/{}", is_cpp ? "cpp" : "python", plugin_name), false);
    FMA_DBG_STREAM(logger_) << "[RestClient] DeletePlugin " << plugin_name << " succeeded";
}

EdgeUid RestClient::AddEdge(const std::string& db, lgraph::VertexId src, lgraph::VertexId dst,
                            const std::string& label, const std::vector<std::string>& field_names,
                            const std::vector<std::string>& field_value_strings) {
    EdgeUid ret;
    if (field_names.size() != field_value_strings.size()) {
        FMA_WARN_STREAM(logger_)
            << "[RestClient] AddEdge failed! field_names.size() != field_values.size()";
        return ret;
    }
    json::value js_prop;
    for (size_t i = 0; i < field_names.size(); i++) {
        js_prop[_TU(field_names[i])] = ValueToJson(field_value_strings[i]);
    }
    json::value body_data;
    body_data[_TU(RestStrings::DST)] = json::value::number(dst);
    body_data[_TU(RestStrings::LABEL)] = json::value::string(_TU(label));
    body_data[_TU(RestStrings::PROP)] = js_prop;
    auto response = DoGraphPost(db, FMA_FMT("/node/{}/relationship", src), body_data, true);
    auto resp = response.as_string();
    bool b = ExtractEdgeUid(_TS(resp), ret);
    if (!b)
        FMA_WARN_STREAM(logger_) << "[RestClient] AddEdge (" << src << "," << dst << ") failed";
    else
        FMA_DBG_STREAM(logger_) << "[RestClient] AddEdge (" << src << "," << dst << ") succeeded";
    return ret;
}

// create

int64_t RestClient::AddVertex(const std::string& db, const std::string& label_name,
                              const std::vector<std::string>& field_names,
                              const std::vector<std::string>& field_value_strings) {
    int64_t ret = -1;
    if (field_names.size() != field_value_strings.size()) {
        FMA_DBG_STREAM(logger_)
            << "[RestClient] AddVertex failed! field_names.size() != field_value_strings.size()";
        return -1;
    }
    json::value js_prop;
    for (size_t i = 0; i < field_names.size(); i++) {
        js_prop[_TU(field_names[i])] = ValueToJson(field_value_strings[i]);
    }
    json::value body_data;
    body_data[_TU(RestStrings::LABEL)] = json::value::string(_TU(label_name));
    body_data[_TU(RestStrings::PROP)] = js_prop;
    auto resp = DoGraphPost(db, "/node", body_data, true);
    if (JsonToType(resp, ret)) {
        FMA_DBG_STREAM(logger_) << "[RestClient] AddVertex succeeded";
        return ret;
    } else {
        FMA_WARN_STREAM(logger_) << "[RestClient] AddVertex failed";
        return -1;
    }
}
