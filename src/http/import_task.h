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

#pragma once

#include <string>
#include <atomic>
#include "tools/json.hpp"
#include "http/import_manager.h"
#include "http/http_server.h"

namespace lgraph {
namespace http {

class ImportTask {
 public:
    ImportTask(HttpService* http_service, ImportManager* import_manager, const std::string& id,
               const std::string& token, const std::string& graph, const std::string& delimiter,
               bool continue_on_error, uint64_t skip_packages, const nlohmann::json& schema);

    void operator()();

    std::string SendImportRequest(const std::string& description, const std::string& data);

 private:
    HttpService* http_service_;
    ImportManager* import_manager_;
    std::string id_;
    std::string user_;
    std::string token_;
    std::string graph_;
    std::string delimiter_;
    bool continue_on_error_;
    uint64_t skip_packages_;
    nlohmann::json schema_;
};

}  //  end of namespace http
}  //  end of namespace lgraph
