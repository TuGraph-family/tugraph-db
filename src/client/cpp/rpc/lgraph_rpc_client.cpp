
/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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

RpcClient::RpcClient(const std::string& url, const std::string& user, const std::string& pass)
    : logger_(fma_common::Logger::Get("lgraph.RpcClient")),
      channel(std::make_shared<lgraph_rpc::m_channel>()),
      cntl(std::make_shared<lgraph_rpc::m_controller>()),
      options(std::make_shared<lgraph_rpc::m_channel_options>()) {
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

int64_t RpcClient::Restore(const std::vector<lgraph::BackupLogEntry>& requests) {
    LGraphRequest lreq;
    lreq.set_is_write_op(true);
    auto req = lreq.mutable_restore_request();
    for (auto& r : requests) *req->add_logs() = r;
    return HandleRequest(&lreq).restore_response().last_success_idx();
}

LGraphResponse RpcClient::HandleRequest(LGraphRequest* req) {
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

bool RpcClient::HandleCypherRequest(LGraphResponse* res, const std::string& query,
                                    const std::string& graph, bool json_format, double timeout) {
    assert(res);
    cntl->Reset();
    cntl->request_attachment().append(FLAGS_attachment);
    LGraphRequest req;
    req.set_client_version(server_version);
    req.set_token(token);
    lgraph::CypherRequest* cypher_req = req.mutable_cypher_request();
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

bool RpcClient::CallCypher(std::string& result, const std::string& cypher, const std::string& graph,
                           bool json_format, double timeout) {
    LGraphResponse res;
    if (!HandleCypherRequest(&res, cypher, graph, json_format, timeout)) {
        result = res.error();
        return false;
    }
    CypherResponse cypher_res = res.cypher_response();
    result = std::move(CypherResponseExtractor(cypher_res));
    return true;
}

#ifdef BINARY_RESULT_BUG_TO_BE_SOLVE
std::string RpcClient::SingleElementExtractor(const CypherResult cypher) {
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

std::string RpcClient::MultElementExtractor(const CypherResult cypher) {
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

std::string RpcClient::CypherResultExtractor(const CypherResult cypher) {
    int hsize = cypher.header_size();
    if (hsize == 1) return SingleElementExtractor(cypher);
    return MultElementExtractor(cypher);
}
#endif

std::string RpcClient::CypherResponseExtractor(const CypherResponse cypher) {
    switch (cypher.Result_case()) {
    case CypherResponse::kJsonResult:
        {
            return cypher.json_result();
        }
    case CypherResponse::kBinaryResult:
        {
#ifdef BINARY_RESULT_BUG_TO_BE_SOLVE
            return CypherResultExtractor(cypher.binary_result());
#endif
        }
    case CypherResponse::RESULT_NOT_SET:
        FMA_ERR() << "CypherResponse::RESULT_NOT_SET";
        break;
    }
    //  Just to pass the compilation
    return "";
}

bool RpcClient::LoadPlugin(std::string& result, const std::string& source_file,
                           const std::string& plugin_type, const std::string& plugin_name,
                           const std::string& code_type, const std::string& plugin_description,
                           bool read_only, const std::string& graph, bool json_format,
                           double timeout) {
    try {
        std::string content;
        if (!FieldSpecSerializer::FileReader(source_file, content)) {
            std::swap(content, result);
            return false;
        }
        fma_common::encrypt::Base64 base64;
        std::string cypher(FMA_FMT("CALL db.plugin.loadPlugin('{}', '{}', '{}', '{}', '{}', {})",
                                   plugin_type, plugin_name, base64.Encode(content), code_type,
                                   plugin_description,
                                   FieldSpecSerializer::FormatBoolean(read_only)));
        if (!CallCypher(result, cypher, graph, json_format, timeout)) {
            return false;
        }
    } catch (std::exception& e) {
        result = e.what();
        return false;
    }
    return true;
}

bool RpcClient::CallPlugin(std::string& result, const std::string& plugin_type,
                           const std::string& plugin_name, const std::string& param,
                           double plugin_time_out, bool in_process, const std::string& graph,
                           bool json_format, double timeout) {
    std::string cypher(FMA_FMT("CALL db.plugin.callPlugin('{}', '{}', '{}', {}, {})", plugin_type,
                               plugin_name, param, std::to_string(plugin_time_out),
                               FieldSpecSerializer::FormatBoolean(in_process)));
    if (!CallCypher(result, cypher, graph, json_format, timeout)) return false;
    return true;
}

bool RpcClient::ImportSchemaFromFile(std::string& result, const std::string& schema_file,
                                     const std::string& graph, bool json_format, double timeout) {
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

bool RpcClient::ImportDataFromFile(std::string& result, const std::string& conf_file,
                                   const std::string& delimiter, bool continue_on_error,
                                   int thread_nums, int skip_packages, const std::string& graph,
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
        size_t bytes_total = 0;
        for (const import_v2::CsvDesc& fd : data_files) bytes_total += fd.size;

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

bool RpcClient::ImportSchemaFromContent(std::string& result, const std::string& schema,
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

bool RpcClient::ImportDataFromContent(std::string& result, const std::string& desc,
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

void RpcClient::Logout() {
    LGraphRequest request;
    request.set_is_write_op(false);
    auto* req = request.mutable_acl_request();
    auto* auth = req->mutable_auth_request()->mutable_logout();
    auth->set_token(token);
    HandleRequest(&request);
    FMA_DBG_STREAM(logger_) << "[RpcClient] RpcClient logout succeeded";
}

RpcClient::~RpcClient() {
    try {
        Logout();
    } catch (const RpcConnectionException & e) {
        FMA_DBG_STREAM(logger_) << "[RpcClient] RpcClient Commection Exception";
    }
}

}  // end of namespace lgraph
