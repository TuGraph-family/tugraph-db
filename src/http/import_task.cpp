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

#include "http/import_task.h"
#include "tools/lgraph_log.h"
#include "import/import_config_parser.h"
#include "import/file_cutter.h"
#include "import/parse_delimiter.h"

namespace lgraph {
namespace http {

ImportTask::ImportTask(HttpService* http_service, ImportManager* import_manager,
                       const std::string& id, const std::string& token, const std::string& graph,
                       const std::string& delimiter, bool continue_on_error, uint64_t skip_packages,
                       const nlohmann::json& schema)
    : http_service_(http_service),
      import_manager_(import_manager),
      id_(std::move(id)),
      token_(std::move(token)),
      graph_(std::move(graph)),
      delimiter_(std::move(delimiter)),
      continue_on_error_(continue_on_error),
      skip_packages_(skip_packages),
      schema_(std::move(schema)) {}

void ImportTask::operator()() {
    std::vector<std::string> imported_files;
    size_t imported_line = 0;
    std::string filename;
    try {
        import_manager_->NewRecord(id_);
        std::vector<import_v2::CsvDesc> data_files =
            import_v2::ImportConfParser::ParseFiles(schema_);
        if (data_files.empty()) {
            return import_manager_->RecordFinish(id_, schema_, true);
        }

        std::stable_sort(data_files.begin(), data_files.end(),
                         [](const import_v2::CsvDesc& a, const import_v2::CsvDesc& b) {
                             return a.is_vertex_file > b.is_vertex_file;
                         });
        size_t bytes_total = 0;
        for (const import_v2::CsvDesc& fd : data_files) {
            bytes_total += fd.size;
        }
        import_manager_->RecordTotalBytes(id_, bytes_total);
        uint64_t processed_packages = import_manager_->GetProcessedPackages(id_);
        uint64_t skip_packages =
            skip_packages_ > processed_packages ? skip_packages_ : processed_packages;
        import_manager_->RecordProcessing(id_);

        for (import_v2::CsvDesc& fd : data_files) {
            filename = fd.path;
            std::string desc = fd.Dump();
            bool is_first_package = true;
            char *begin, *end;
            imported_line = 0;

            import_v2::FileCutter cutter(filename);
            for (; cutter.Cut(begin, end); is_first_package = false) {
                size_t lines = std::count(begin, end + 1, '\n');
                if (skip_packages > 0) {
                    --skip_packages;
                    imported_line += lines;
                    continue;
                }
                ++processed_packages;
                if (is_first_package) {
                    if (fd.n_header_line >
                        static_cast<size_t>(std::count(begin, end, '\n')) + (end[-1] != '\n')) {
                        std::string res = boost::algorithm::join(imported_files, ",") +
                                          " were imported successfully.\n";
                        res += filename + " was imported "
                               + std::to_string(imported_line) + " lines.\n";
                        return import_manager_->RecordErrorMsg(id_, schema_,
                                                               "HEADER too large.\n" + res);
                    }
                } else {
                    fd.n_header_line = 0;
                    desc = fd.Dump();
                }
                std::string res = SendImportRequest(desc, std::string(begin, end));
                if (res.size()) {
                    if (!continue_on_error_) {
                        res += "\n" + boost::algorithm::join(imported_files, ",") +
                                          " were imported successfully.\n";
                        res += filename + " was imported " +
                               std::to_string(imported_line) + " lines.\n";
                        return import_manager_->RecordErrorMsg(id_, schema_, res);
                    }
                }
                uint64_t bytes_sent = end - begin;
                import_manager_->RecordProgress(id_, processed_packages, bytes_sent);
                imported_line += lines;
            }
            imported_files.emplace_back(filename);
        }
        import_manager_->RecordFinish(id_, schema_, true);
    } catch (std::exception& e) {
        std::string res = boost::algorithm::join(imported_files, ",") +
                          " were imported successfully.\n";
        res += filename + " was imported " + std::to_string(imported_line) + " lines.\n";
        import_manager_->RecordErrorMsg(id_, schema_,
                                               std::string(e.what()) + "\n" + res);
    }
}

std::string ImportTask::SendImportRequest(const std::string& description, const std::string& data) {
    LGraphRequest proto_req;
    proto_req.set_is_write_op(true);
    proto_req.set_token(token_);
    ImportRequest* req = proto_req.mutable_import_request();
    req->set_graph(graph_);
    req->set_continue_on_error(continue_on_error_);
    req->set_data(data);
    req->set_description(description);
    req->set_delimiter(delimiter_);
    LGraphResponse proto_resp;
    http_service_->ApplyToStateMachine(proto_req, proto_resp);
    std::string log;
    if (proto_resp.error_code() == LGraphResponse::SUCCESS) {
        if (proto_resp.import_response().has_error_message())
            log = std::move(*proto_resp.mutable_import_response()->mutable_error_message());
        if (proto_resp.import_response().has_log())
            log = std::move(*proto_resp.mutable_import_response()->mutable_log());
    } else {
        log = std::move(*proto_resp.mutable_error());
    }
    return log;
}

}  //  end of namespace http
}  //  end of namespace lgraph
