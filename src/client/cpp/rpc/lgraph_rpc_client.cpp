
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

#include "lgraph/lgraph_rpc_client.h"
#include "client/cpp/rpc/lgraph_rpc.h"
#include "client/cpp/rpc/field_spec_serializer.h"
#include "client/cpp/rpc/rpc_exception.h"
#include "fma-common/logger.h"
#include "fma-common/fma_stream.h"
#include "tools/json.hpp"
#include "protobuf/ha.pb.h"
#include "import/import_config_parser.h"
#include "import/file_cutter.h"
#include "import/parse_delimiter.h"
#include "fma-common/encrypt.h"

namespace lgraph {

DEFINE_string(attachment, "", "Carry this along with requests");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "0.0.0.0:7072", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 60 * 60 * 1000, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)");
DEFINE_int32(interval_ms, 1000, "Milliseconds between consecutive requests");

RpcClient::RpcSingleClient::RpcSingleClient(const std::string& url,
                                            const std::string& user,
                                            const std::string& pass)
    : url(url),
      user(user),
      password(pass),
      channel(std::make_shared<lgraph_rpc::m_channel>()),
      cntl(std::make_shared<lgraph_rpc::m_controller>()),
      options(std::make_shared<lgraph_rpc::m_channel_options>()),
      logger_(fma_common::Logger::Get("lgraph.RpcClient")) {
    server_version = -1;
    options->protocol = FLAGS_protocol;
    options->connection_type = FLAGS_connection_type;
    options->timeout_ms = FLAGS_timeout_ms /*milliseconds*/;
    options->max_retry = FLAGS_max_retry;
    LGraphRPCService_Stub stub(channel.get());
    if (channel->Init(url.c_str(), FLAGS_load_balancer.c_str(), options.get()) != 0)
        throw RpcException("Fail to initialize channel");
    LGraphRequest request;
    request.set_is_write_op(false);
    auto* req = request.mutable_acl_request();
    auto* auth = req->mutable_auth_request()->mutable_login();
    auth->set_user(user);
    auth->set_password(pass);
    // send data
    token = HandleRequest(&request).acl_response().auth_response().token();
    FMA_DBG_STREAM(logger_) << "[RpcClient] RpcClient succeeded";
}

int64_t RpcClient::RpcSingleClient::Restore(const std::vector<lgraph::BackupLogEntry>& requests) {
    LGraphRequest lreq;
    lreq.set_is_write_op(true);
    auto req = lreq.mutable_restore_request();
    for (auto& r : requests) *req->add_logs() = r;
    return HandleRequest(&lreq).restore_response().last_success_idx();
}

LGraphResponse RpcClient::RpcSingleClient::HandleRequest(LGraphRequest* req) {
    cntl->Reset();
    cntl->request_attachment().append(FLAGS_attachment);
    req->set_client_version(server_version);
    req->set_token(token);
    LGraphRPCService_Stub stub(channel.get());
    LGraphResponse resp;
    stub.HandleRequest(cntl.get(), req, &resp, nullptr);
    if (cntl->Failed()) throw RpcConnectionException(cntl->ErrorText());
    server_version = std::max(server_version, resp.server_version());
    if (resp.error_code() != LGraphResponse::SUCCESS) throw RpcStatusException(resp.error());
    return resp;
}

bool RpcClient::RpcSingleClient::HandleGraphQueryRequest(lgraph::ProtoGraphQueryType type,
                                                     LGraphResponse* res, const std::string& query,
                                                     const std::string& graph, bool json_format,
                                                     double timeout) {
    assert(res);
    cntl->Reset();
    cntl->request_attachment().append(FLAGS_attachment);
    LGraphRequest req;
    req.set_client_version(server_version);
    req.set_token(token);
    lgraph::GraphQueryRequest* cypher_req = req.mutable_graph_query_request();
    cypher_req->set_type(type);
    cypher_req->set_graph(graph);
    cypher_req->set_query(query);
    cypher_req->set_timeout(timeout);
    // TODO(jzj)
    if (!json_format) return false;
    cypher_req->set_result_in_json_format(true);
    LGraphRPCService_Stub stub(channel.get());
    stub.HandleRequest(cntl.get(), &req, res, nullptr);
    if (cntl->Failed()) {
        res->set_error(cntl->ErrorText());
        return false;
    }
    if (res->error_code() != LGraphResponse::SUCCESS) {
        return false;
    }
    server_version = std::max(server_version, res->server_version());
    return true;
}

bool RpcClient::RpcSingleClient::CallCypher(std::string& result, const std::string& cypher,
                                            const std::string& graph, bool json_format,
                                            double timeout) {
    LGraphResponse res;
    if (!HandleGraphQueryRequest(lgraph::ProtoGraphQueryType::CYPHER,
                                 &res, cypher, graph, json_format, timeout)) {
        result = res.error();
        return false;
    }
    const GraphQueryResponse& cypher_res = res.graph_query_response();
    result = GraphQueryResponseExtractor(cypher_res);
    return true;
}

bool RpcClient::RpcSingleClient::CallGql(std::string& result, const std::string& gql,
                                         const std::string& graph, bool json_format,
                                         double timeout) {
    LGraphResponse res;
    if (!HandleGraphQueryRequest(lgraph::ProtoGraphQueryType::GQL,
                                 &res, gql, graph, json_format, timeout)) {
        result = res.error();
        return false;
    }
    const GraphQueryResponse& gql_res = res.graph_query_response();
    result = GraphQueryResponseExtractor(gql_res);
    return true;
}

#ifdef BINARY_RESULT_BUG_TO_BE_SOLVE
std::string RpcClient::RpcSingleClient::SingleElementExtractor(const CypherResult cypher) {
    nlohmann::json arr;
    int rsize = cypher.result_size();
    for (int i = 0; i < rsize; ++i) {
        ListOfProtoFieldData list = cypher.result(i);
        int dsize = list.values_size();
        for (int j = 0; j < dsize; ++j) {
            ProtoFieldData data = list.values(j);
            switch (data.Data_case()) {
            case ProtoFieldData::kBoolean:
                arr[i] = nlohmann::json(data.boolean());
                break;
            case ProtoFieldData::kInt8:
                arr[i] = nlohmann::json(data.int8_());
                break;
            case ProtoFieldData::kInt16:
                arr[i] = nlohmann::json(data.int16_());
                break;
            case ProtoFieldData::kInt32:
                arr[i] = nlohmann::json(data.int32_());
                break;
            case ProtoFieldData::kInt64:
                arr[i] = nlohmann::json(data.int64_());
                break;
            case ProtoFieldData::kSp:
                arr[i] = nlohmann::json(data.sp());
                break;
            case ProtoFieldData::kDp:
                arr[i] = nlohmann::json(data.dp());
                break;
            case ProtoFieldData::kDate:
                arr[i] = nlohmann::json(data.date());
                break;
            case ProtoFieldData::kDatetime:
                arr[i] = nlohmann::json(data.datetime());
                break;
            case ProtoFieldData::kStr:
                arr[i] = nlohmann::json(data.str());
                break;
            case ProtoFieldData::kBlob:
                arr[i] = nlohmann::json(data.blob());
                break;
            case ProtoFieldData::DATA_NOT_SET:
                FMA_ERR() << "ProtoFieldData::DATA_NOT_SET";
                break;
            }
        }
    }
    return nlohmann::to_string(arr);
}

std::string RpcClient::RpcSingleClient::MultElementExtractor(const CypherResult cypher) {
    nlohmann::json arr;
    int hsize = cypher.header_size();
    int rsize = cypher.result_size();
    for (int i = 0; i < rsize; ++i) {
        ListOfProtoFieldData list = cypher.result(i);
        int dsize = list.values_size();
        assert(dsize != hsize);
        nlohmann::json obj;
        for (int j = 0; j < dsize; ++j) {
            ProtoFieldData data = list.values(j);
            switch (data.Data_case()) {
            case ProtoFieldData::kBoolean:
                obj[cypher.header(j).name()] = nlohmann::json(data.boolean());
                break;
            case ProtoFieldData::kInt8:
                obj[cypher.header(j).name()] = nlohmann::json(data.int8_());
                break;
            case ProtoFieldData::kInt16:
                obj[cypher.header(j).name()] = nlohmann::json(data.int16_());
                break;
            case ProtoFieldData::kInt32:
                obj[cypher.header(j).name()] = nlohmann::json(data.int32_());
                break;
            case ProtoFieldData::kInt64:
                obj[cypher.header(j).name()] = nlohmann::json(data.int64_());
                break;
            case ProtoFieldData::kSp:
                obj[cypher.header(j).name()] = nlohmann::json(data.sp());
                break;
            case ProtoFieldData::kDp:
                obj[cypher.header(j).name()] = nlohmann::json(data.dp());
                break;
            case ProtoFieldData::kDate:
                obj[cypher.header(j).name()] = nlohmann::json(data.date());
                break;
            case ProtoFieldData::kDatetime:
                obj[cypher.header(j).name()] = nlohmann::json(data.datetime());
                break;
            case ProtoFieldData::kStr:
                obj[cypher.header(j).name()] = nlohmann::json(data.str());
                break;
            case ProtoFieldData::kBlob:
                obj[cypher.header(j).name()] = nlohmann::json(data.blob());
                break;
            case ProtoFieldData::DATA_NOT_SET:
                FMA_ERR() << "ProtoFieldData::DATA_NOT_SET";
                break;
            }
        }
        arr[i] = obj;
    }
    return nlohmann::to_string(arr);
}

std::string RpcClient::RpcSingleClient::CypherResultExtractor(const CypherResult cypher) {
    int hsize = cypher.header_size();
    if (hsize == 1) return SingleElementExtractor(cypher);
    return MultElementExtractor(cypher);
}
#endif

std::string RpcClient::RpcSingleClient::GraphQueryResponseExtractor(const GraphQueryResponse&
                                                                    cypher) {
    switch (cypher.Result_case()) {
    case GraphQueryResponse::kJsonResult:
        {
            return cypher.json_result();
        }
    case GraphQueryResponse::kBinaryResult:
        {
#ifdef BINARY_RESULT_BUG_TO_BE_SOLVE
            return CypherResultExtractor(cypher.binary_result());
#endif
        }
    case GraphQueryResponse::RESULT_NOT_SET:
        FMA_ERR() << "GraphQueryResponse::RESULT_NOT_SET";
        break;
    }
    //  Just to pass the compilation
    return "";
}

bool RpcClient::RpcSingleClient::LoadProcedure(std::string& result, const std::string& source_file,
                                               const std::string& procedure_type,
                                               const std::string& procedure_name,
                                               const std::string& code_type,
                                               const std::string& procedure_description,
                                               bool read_only, const std::string& version,
                                               const std::string& graph) {
    try {
        std::string content;
        if (!FieldSpecSerializer::FileReader(source_file, content)) {
            std::swap(content, result);
            return false;
        }
        LGraphRequest req;
        req.set_is_write_op(true);
        lgraph::PluginRequest* pluginRequest = req.mutable_plugin_request();
        pluginRequest->set_graph(graph);
        pluginRequest->set_type(procedure_type == "CPP" ? lgraph::PluginRequest::CPP
                                                        : lgraph::PluginRequest::PYTHON);
        if (version != lgraph::plugin::PLUGIN_VERSION_1 &&
            version != lgraph::plugin::PLUGIN_VERSION_2)
            throw RpcException("Invalid plugin version");
        pluginRequest->set_version(version);
        lgraph::LoadPluginRequest* loadPluginRequest = pluginRequest->mutable_load_plugin_request();
        loadPluginRequest->set_code_type([](const std::string& type) {
            std::unordered_map<std::string, lgraph::LoadPluginRequest_CodeType> um{
                {"SO", lgraph::LoadPluginRequest::SO},
                {"PY", lgraph::LoadPluginRequest::PY},
                {"ZIP", lgraph::LoadPluginRequest::ZIP},
                {"CPP", lgraph::LoadPluginRequest::CPP}};
            return um[type];
        }(code_type));
        loadPluginRequest->set_name(procedure_name);
        loadPluginRequest->set_desc(procedure_description);
        loadPluginRequest->set_read_only(read_only);
        loadPluginRequest->set_code(content);
        HandleRequest(&req);
    } catch (std::exception& e) {
        result = e.what();
        return false;
    }
    return true;
}

bool RpcClient::RpcSingleClient::CallProcedure(std::string& result,
                                               const std::string& procedure_type,
                                               const std::string& procedure_name,
                                               const std::string& param,
                                               double procedure_time_out, bool in_process,
                                               const std::string& graph, bool json_format) {
    try {
        LGraphRequest req;
        lgraph::PluginRequest* pluginRequest = req.mutable_plugin_request();
        pluginRequest->set_graph(graph);
        pluginRequest->set_type(procedure_type == "CPP" ? lgraph::PluginRequest::CPP
                                                        : lgraph::PluginRequest::PYTHON);
        lgraph::CallPluginRequest* cpRequest = pluginRequest->mutable_call_plugin_request();
        cpRequest->set_name(procedure_name);
        cpRequest->set_in_process(in_process);
        cpRequest->set_param(param);
        cpRequest->set_timeout(procedure_time_out);
        cpRequest->set_result_in_json_format(json_format);
        LGraphResponse res = HandleRequest(&req);
        if (json_format) {
            result = res.mutable_plugin_response()->mutable_call_plugin_response()->json_result();
        } else {
            result = res.mutable_plugin_response()->mutable_call_plugin_response()->reply();
        }
    } catch (std::exception& e) {
        result = e.what();
        return false;
    }
    return true;
}

bool RpcClient::RpcSingleClient::ListProcedures(std::string& result,
                                                const std::string& procedure_type,
                                                const std::string& version,
                                                const std::string& graph) {
    try {
        LGraphRequest req;
        req.set_is_write_op(false);
        lgraph::PluginRequest* pluginRequest = req.mutable_plugin_request();
        pluginRequest->set_graph(graph);
        pluginRequest->set_type(procedure_type == "CPP" ? lgraph::PluginRequest::CPP
                                                        : lgraph::PluginRequest::PYTHON);
        pluginRequest->set_version(version);
        pluginRequest->mutable_list_plugin_request();
        LGraphResponse res = HandleRequest(&req);
        result = res.mutable_plugin_response()->mutable_list_plugin_response()->reply();
    } catch (std::exception& e) {
        result = e.what();
        return false;
    }
    return true;
}

bool RpcClient::RpcSingleClient::DeleteProcedure(std::string& result,
                                                 const std::string& procedure_type,
                                                 const std::string& procedure_name,
                                                 const std::string& graph) {
    try {
        LGraphRequest req;
        req.set_is_write_op(true);
        lgraph::PluginRequest* pluginRequest = req.mutable_plugin_request();
        pluginRequest->set_graph(graph);
        pluginRequest->set_type(procedure_type == "CPP" ? lgraph::PluginRequest::CPP
                                                        : lgraph::PluginRequest::PYTHON);
        lgraph::DelPluginRequest* dpRequest = pluginRequest->mutable_del_plugin_request();
        dpRequest->set_name(procedure_name);
        HandleRequest(&req);
    } catch (std::exception& e) {
        result = e.what();
        return false;
    }
    return true;
}

bool RpcClient::RpcSingleClient::ImportSchemaFromFile(std::string& result,
                                                      const std::string& schema_file,
                                                      const std::string& graph, bool json_format,
                                                      double timeout) {
    try {
        std::ifstream ifs(schema_file);
        nlohmann::json conf;
        ifs >> conf;
        if (conf.contains("schema")) {
            nlohmann::json schema_conf;
            schema_conf["schema"] = conf["schema"];
            fma_common::encrypt::Base64 base64;
            std::string content = base64.Encode(schema_conf.dump(4));
            std::string cypher(FMA_FMT("CALL db.importor.schemaImportor('{}')", content));
            if (!CallCypher(result, cypher, graph, json_format, timeout)) {
                return false;
            }
        }
    } catch (std::exception& e) {
        result = e.what();
        return false;
    }
    return true;
}

bool RpcClient::RpcSingleClient::ImportDataFromFile(std::string& result,
                                                    const std::string& conf_file,
                                                    const std::string& delimiter,
                                                    bool continue_on_error, int thread_nums,
                                                    int skip_packages, const std::string& graph,
                                                    bool json_format, double timeout) {
    std::ifstream ifs(conf_file);
    nlohmann::json conf;
    ifs >> conf;
    bool finished = true;
    try {
        std::vector<import_v2::CsvDesc> data_files = import_v2::ImportConfParser::ParseFiles(conf);
        if (data_files.empty()) return finished;

        std::stable_sort(data_files.begin(), data_files.end(),
                         [](const import_v2::CsvDesc& a, const import_v2::CsvDesc& b) {
                             return a.is_vertex_file > b.is_vertex_file;
                         });

        for (import_v2::CsvDesc& fd : data_files) {
            const auto& filename = fd.path;
            std::string desc = fd.Dump();

            bool is_first_package = true;
            char *begin, *end;
            import_v2::FileCutter cutter(filename);
            for (; cutter.Cut(begin, end); is_first_package = false) {
                if (skip_packages > 0) {
                    --skip_packages;
                    continue;
                }

                if (is_first_package) {
                    if (fd.n_header_line >
                        static_cast<size_t>(std::count(begin, end, '\n')) + (end[-1] != '\n')) {
                        result = "HEADER too large";
                        return false;
                    }
                } else {
                    fd.n_header_line = 0;
                    desc = fd.Dump();
                }
                fma_common::encrypt::Base64 base64;
                desc = base64.Encode(desc);
                std::string content = base64.Encode(begin, end - begin);
                std::string cypher(
                    FMA_FMT("CALL db.importor.dataImportor('{}', '{}', {}, {}, '{}')", desc,
                            content, FieldSpecSerializer::FormatBoolean(continue_on_error),
                            thread_nums, ParseDelimiter(delimiter)));
                if (!CallCypher(result, cypher, graph, json_format, timeout)) {
                    finished = false;
                }
            }
        }
    } catch (std::exception& e) {
        result = e.what();
        finished = false;
    }
    return finished;
}

bool RpcClient::RpcSingleClient::ImportSchemaFromContent(std::string& result,
                                        const std::string& schema,
                                        const std::string& graph, bool json_format,
                                        double timeout) {
    try {
        fma_common::encrypt::Base64 base64;
        std::string content = base64.Encode(schema);
        std::string cypher(FMA_FMT("CALL db.importor.schemaImportor('{}')", content));
        if (!CallCypher(result, cypher, graph, json_format, timeout)) {
            return false;
        }
    } catch (std::exception& e) {
        result = e.what();
        return false;
    }
    return true;
}

bool RpcClient::RpcSingleClient::ImportDataFromContent(std::string& result, const std::string& desc,
                                      const std::string& data, const std::string& delimiter,
                                      bool continue_on_error, int thread_nums,
                                      const std::string& graph, bool json_format, double timeout) {
    try {
        fma_common::encrypt::Base64 base64;
        std::string description = base64.Encode(desc);
        std::string content = base64.Encode(data);
        std::string cypher(FMA_FMT("CALL db.importor.dataImportor('{}', '{}', {}, {}, '{}')",
                                   description, content,
                                   FieldSpecSerializer::FormatBoolean(continue_on_error),
                                   thread_nums, ParseDelimiter(delimiter)));
        if (!CallCypher(result, cypher, graph, json_format, timeout)) {
            return false;
        }
    } catch (std::exception& e) {
        result = e.what();
        return false;
    }
    return true;
}

std::string RpcClient::RpcSingleClient::GetUrl() {
    return url;
}

void RpcClient::RpcSingleClient::Logout() {
    LGraphRequest request;
    request.set_is_write_op(false);
    auto* req = request.mutable_acl_request();
    auto* auth = req->mutable_auth_request()->mutable_logout();
    auth->set_token(token);
    HandleRequest(&request);
    FMA_DBG_STREAM(logger_) << "[RpcSingleClient] RpcSingleClient logout succeeded";
}

RpcClient::RpcSingleClient::~RpcSingleClient() {
    try {
        Logout();
    } catch (const RpcConnectionException& e) {
        FMA_DBG_STREAM(logger_) << "[RpcSingleClient] RpcSingleClient Connection Exception";
    }
}

RpcClient::RpcClient(const std::string &url, const std::string &user,
                         const std::string &password)
    : logger_(fma_common::Logger::Get("lgraph.RpcClient")),
      user(user),
      password(password),
      base_client(std::make_shared<RpcSingleClient>(url, user, password)) {
    std::string result;
    bool ret = base_client->CallCypher(result, "CALL dbms.ha.clusterInfo()");
    if (ret) {
        client_type = DIRECT_HA_CONNECTION;
        cypher_write_constant = {"create ", "set ", "delete ", "remove ", "merge "};
        gql_write_constant = {"create ", "set ", "delete ", "remove ", "insert ", "drop "};
        RefreshConnection();
    } else {
        client_type = SINGLE_CONNECTION;
    }
}

RpcClient::RpcClient(std::vector<std::string> &urls, std::string user,
                         std::string password)
    : logger_(fma_common::Logger::Get("lgraph.RpcClient")),
      user(std::move(user)),
      password(std::move(password)),
      urls(urls) {
    client_type = INDIRECT_HA_CONNECTION;
    cypher_write_constant = {"create ", "set ", "delete ", "remove ", "merge "};
    gql_write_constant = {"create ", "set ", "delete ", "remove ", "insert ", "drop "};
    RefreshConnection();
}

RpcClient::~RpcClient() {
    try {
        Logout();
    } catch (const RpcConnectionException& e) {
        FMA_DBG_STREAM(logger_) << "[RpcClient] RpcClient Connection Exception";
    }
}

bool RpcClient::CallCypher(std::string &result, const std::string &cypher,
                             const std::string &graph, bool json_format,
                             double timeout, const std::string& url) {
    if (client_type == SINGLE_CONNECTION) {
        return base_client->CallCypher(result, cypher, graph, json_format, timeout);
    }
    auto fun = [&]{
        if (!url.empty())
            return GetClientByNode(url)->CallCypher(result, cypher, graph, json_format, timeout);
        return GetClient(lgraph::ProtoGraphQueryType::CYPHER, cypher, graph)->
            CallCypher(result, cypher, graph, json_format, timeout);
    };
    return DoubleCheckQuery(fun);
}

bool RpcClient::CallCypherToLeader(std::string& result, const std::string& cypher,
                                   const std::string& graph, bool json_format, double timeout) {
    if (client_type == SINGLE_CONNECTION) {
        return base_client->CallCypher(result, cypher, graph, json_format, timeout);
    } else {
        return DoubleCheckQuery([&] {
            return leader_client->CallCypher(result, cypher, graph, json_format, timeout);
        });
    }
}

bool RpcClient::CallGql(std::string &result, const std::string &gql,
                           const std::string &graph, bool json_format,
                           double timeout, const std::string& url) {
    if (client_type == SINGLE_CONNECTION) {
        return base_client->CallGql(result, gql, graph, json_format, timeout);
    }
    auto fun = [&]{
        if (!url.empty())
            return GetClientByNode(url)->CallGql(result, gql, graph, json_format, timeout);
        return GetClient(lgraph::ProtoGraphQueryType::GQL, gql, graph)->
            CallGql(result, gql, graph, json_format, timeout);
    };
    return DoubleCheckQuery(fun);
}

bool RpcClient::CallGqlToLeader(std::string& result, const std::string& gql,
                                const std::string& graph, bool json_format, double timeout) {
    if (client_type == SINGLE_CONNECTION) {
        return base_client->CallGql(result, gql, graph, json_format, timeout);
    } else {
        return DoubleCheckQuery([&] {
            return leader_client->CallGql(result, gql, graph, json_format, timeout);
        });
    }
}

bool RpcClient::CallProcedure(std::string &result, const std::string &procedure_type,
                             const std::string &procedure_name, const std::string &param,
                             double procedure_time_out, bool in_process, const std::string &graph,
                             bool json_format, const std::string& url) {
    if (client_type == SINGLE_CONNECTION) {
        return base_client->CallProcedure(result, procedure_type, procedure_name, param,
                                          procedure_time_out, in_process, graph, json_format);
    }
    bool is_read_procedure = false;
    for (auto &op : user_defined_procedures) {
        if (op["plugins"]["name"] == procedure_name) {
            is_read_procedure = op["plugins"]["read_only"]; break;
        }
    }
    auto fun = [&]{
        if (!url.empty())
            return GetClientByNode(url)->
                CallProcedure(result, procedure_type, procedure_name, param,
                              procedure_time_out, in_process, graph, json_format);
        return GetClient(is_read_procedure)
            ->CallProcedure(result, procedure_type, procedure_name, param,
                            procedure_time_out, in_process, graph, json_format);
    };
    return DoubleCheckQuery(fun);
}

bool RpcClient::CallProcedureToLeader(std::string& result, const std::string& procedure_type,
                                      const std::string& procedure_name, const std::string& param,
                                      double procedure_time_out, bool in_process,
                                      const std::string& graph, bool json_format) {
    if (client_type == SINGLE_CONNECTION) {
        return base_client->CallProcedure(result, procedure_type, procedure_name, param,
                                          procedure_time_out, in_process, graph, json_format);
    } else {
        return DoubleCheckQuery([&] {
            return leader_client->CallProcedure(result, procedure_type, procedure_name, param,
                                                procedure_time_out, in_process, graph, json_format);
        });
    }
}

bool RpcClient::LoadProcedure(std::string &result, const std::string &source_file,
                              const std::string &procedure_type, const std::string &procedure_name,
                              const std::string &code_type,
                              const std::string &procedure_description,
                              bool read_only,
                              const std::string& version,
                              const std::string &graph) {
    if (client_type == SINGLE_CONNECTION) {
        return base_client->LoadProcedure(result, source_file, procedure_type, procedure_name,
                                          code_type, procedure_description, read_only,
                                          version, graph);
    }
    auto fun = [&]{
        bool succeed = GetClient(false)->
                       LoadProcedure(result, source_file, procedure_type, procedure_name,
                                     code_type, procedure_description, read_only, version, graph);
        if (succeed) {
            RefreshUserDefinedProcedure();
        }
        return succeed;
    };
    return DoubleCheckQuery(fun);
}

bool RpcClient::ListProcedures(std::string &result, const std::string &procedure_type,
                               const std::string& version,
                               const std::string &graph, const std::string &url) {
    if (client_type == SINGLE_CONNECTION) {
        return base_client->ListProcedures(result, procedure_type, version, graph);
    }
    auto fun = [&]{
        if (!url.empty()) {
            return GetClientByNode(url)->ListProcedures(result, procedure_type, version, graph);
        }
        return GetClient(true)->ListProcedures(result, procedure_type, version, graph);
    };
    return DoubleCheckQuery(fun);
}

bool RpcClient::DeleteProcedure(std::string &result, const std::string &procedure_type,
                                  const std::string &procedure_name, const std::string &graph) {
    if (client_type == SINGLE_CONNECTION) {
        return base_client->DeleteProcedure(result, procedure_type, procedure_name, graph);
    }
    auto fun = [&]{
        return GetClient(false)->DeleteProcedure(result, procedure_type, procedure_name, graph);
    };
    return DoubleCheckQuery(fun);
}

bool RpcClient::ImportSchemaFromContent(std::string &result, const std::string &schema,
                                          const std::string &graph, bool json_format,
                                          double timeout) {
    if (client_type == SINGLE_CONNECTION) {
        return base_client->ImportSchemaFromContent(result, schema, graph, json_format, timeout);
    }
    auto fun = [&]{
        return GetClient(false)
            ->ImportSchemaFromContent(result, schema, graph, json_format, timeout);
    };
    return DoubleCheckQuery(fun);
}

bool RpcClient::ImportDataFromContent(std::string &result, const std::string &desc,
                                        const std::string &data, const std::string &delimiter,
                                        bool continue_on_error, int thread_nums,
                                        const std::string &graph, bool json_format,
                                        double timeout) {
    if (client_type == SINGLE_CONNECTION) {
        return base_client->ImportDataFromContent(result, desc, data, delimiter, continue_on_error,
                                                  thread_nums, graph, json_format, timeout);
    }
    auto fun = [&]{
        return GetClient(false)
            ->ImportDataFromContent(result, desc, data, delimiter, continue_on_error,
                                    thread_nums, graph, json_format, timeout);
    };
    return DoubleCheckQuery(fun);
}

bool RpcClient::ImportSchemaFromFile(std::string &result, const std::string &schema_file,
                                       const std::string &graph, bool json_format, double timeout) {
    if (client_type == SINGLE_CONNECTION) {
        return base_client->ImportSchemaFromFile(result, schema_file, graph, json_format, timeout);
    }
    auto fun = [&]{
        return GetClient(false)
            ->ImportSchemaFromFile(result, schema_file, graph, json_format, timeout);
    };
    return DoubleCheckQuery(fun);
}

bool RpcClient::ImportDataFromFile(std::string &result, const std::string &conf_file,
                                     const std::string &delimiter, bool continue_on_error,
                                     int thread_nums, int skip_packages, const std::string &graph,
                                     bool json_format, double timeout) {
    if (client_type == SINGLE_CONNECTION) {
        return base_client->ImportDataFromFile(result, conf_file, delimiter, continue_on_error,
                            thread_nums, skip_packages, graph, json_format, timeout);
    }
    auto fun = [&]{
        return GetClient(false)
            ->ImportDataFromFile(result, conf_file, delimiter, continue_on_error, thread_nums,
                                 skip_packages, graph, json_format, timeout);
    };
    return DoubleCheckQuery(fun);
}

void RpcClient::Logout() {
    if (client_type != INDIRECT_HA_CONNECTION)
        base_client->Logout();
    for (auto &c : client_pool) {
        c->Logout();
    }
}

void RpcClient::RefreshUserDefinedProcedure() {
    std::string result;
    GetClient(true)->
        CallCypher(result, "CALL db.plugin.listUserPlugins()", "default", true, 10);
    user_defined_procedures = nlohmann::json::parse(result.c_str());
}

void RpcClient::RefreshBuiltInProcedure() {
    std::string result;
    GetClient(true)->CallCypher(result, "CALL dbms.procedures()", "default", true, 10);
    built_in_procedures = nlohmann::json::parse(result.c_str());
}

void RpcClient::RefreshClientPool() {
    client_pool.clear();
    if (client_type == DIRECT_HA_CONNECTION) {
        std::string result;
        base_client->CallCypher(result, "CALL dbms.ha.clusterInfo()", "default", true, 10);
        nlohmann::json cluster_info = nlohmann::json::parse(result.c_str());
        for (auto &node : cluster_info[0]["cluster_info"]) {
            auto c = std::make_shared<RpcSingleClient>(node["rpc_address"], user, password);
            if (node["state"] == "MASTER") {
                leader_client = c;
            }
            client_pool.push_back(c);
        }
    } else if (client_type == INDIRECT_HA_CONNECTION) {
        for (auto &url : urls) {
            auto c = std::make_shared<RpcSingleClient>(url, user, password);
            std::string result;
            c->CallCypher(result, "CALL dbms.ha.clusterInfo()", "default", true, 10);
            nlohmann::json cluster_info = nlohmann::json::parse(result.c_str());
            if (cluster_info[0]["is_master"])
                leader_client = c;
            client_pool.push_back(c);
        }
    }
}

bool RpcClient::IsReadQuery(lgraph::ProtoGraphQueryType type,
                             const std::string &query, const std::string &graph) {
    bool isReadQuery = true;
    if (boost::to_upper_copy(query).find("CALL ") != std::string::npos) {
        for (auto &op : user_defined_procedures) {
            if (op["graph"] == graph &&
                query.find(op["plugins"]["name"]) != std::string::npos) {
                isReadQuery = isReadQuery && op["plugins"]["read_only"];
                break;
            }
        }
        for (auto &procedure : built_in_procedures) {
            if (query.find(procedure["name"]) != std::string::npos) {
                isReadQuery = isReadQuery && procedure["read_only"];
                break;
            }
        }
        return isReadQuery;
    }
    std::string tmp = query;
    std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
    if (type == lgraph::ProtoGraphQueryType::CYPHER) {
        for (const auto &c : cypher_write_constant) {
            if (tmp.find(c) != std::string::npos) {
                return false;
            }
        }
    } else {
        for (const auto &c : gql_write_constant) {
            if (tmp.find(c) != std::string::npos) {
                return false;
            }
        }
    }
    return true;
}

std::shared_ptr<lgraph::RpcClient::RpcSingleClient>
    RpcClient::GetClient(lgraph::ProtoGraphQueryType type, const std::string &cypher,
                         const std::string &graph) {
    return GetClient(IsReadQuery(type, cypher, graph));
}

std::shared_ptr<lgraph::RpcClient::RpcSingleClient> RpcClient::GetClient(bool isReadQuery) {
    if (isReadQuery) {
        LoadBalanceClientPool();
        if (client_pool.empty())
            throw RpcException("all instance is down, refuse req!");
        return *client_pool.rbegin();
    } else {
        if (leader_client == nullptr)
            throw RpcException("master instance is down, refuse req!");
        return leader_client;
    }
}

std::shared_ptr<lgraph::RpcClient::RpcSingleClient> RpcClient::GetClientByNode(
    const std::string &url) {
    for (auto &c : client_pool) {
        if (c->GetUrl() == url)
            return c;
    }
    throw RpcException("do not exit " + url +" client");
}

void RpcClient::RefreshConnection() {
    try {
        RefreshClientPool();
        RefreshUserDefinedProcedure();
        RefreshBuiltInProcedure();
    } catch (std::exception &e) {
        FMA_ERR_STREAM(logger_) << "[RpcClient] RpcClient Connection Exception, "
                                   "please connect again!";
    }
}

void RpcClient::LoadBalanceClientPool() {
    client_pool.push_back(client_pool.front());
    client_pool.pop_front();
}

int64_t RpcClient::Restore(const std::vector<BackupLogEntry>& requests) {
    if (client_type == SINGLE_CONNECTION) {
        return base_client->Restore(requests);
    } else {
        auto fun = [&]{
            return GetClient(false)->Restore(requests);
        };
        return DoubleCheckQuery(fun);
    }
}

template<typename F>
bool RpcClient::DoubleCheckQuery(const F &f) {
    try {
        return f();
    } catch (std::exception &e) {
        try {
            RefreshConnection();
            return f();
        } catch (std::exception &e) {
            FMA_ERR_STREAM(logger_) << e.what();
            return false;
        }
    }
}
}  // end of namespace lgraph
