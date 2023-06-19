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

#include <memory>
#include "core/kv_store.h"
#include "core/global_config.h"

namespace lgraph {
namespace http {

class ImportManager {
 public:
    struct ImportRecord {
        std::string task_id_;
        std::string err_msg_;
        uint64_t processed_packages_;
        uint64_t processed_bytes_;
        uint64_t total_in_bytes_;

        ImportRecord()
            : task_id_("None")
            , err_msg_()
            , processed_packages_(0)
            , processed_bytes_(0.0)
            , total_in_bytes_(0) {}
    };

    ImportManager() {}

    void Init(lgraph::GlobalConfig* config);

    std::string GetRootPath();

    std::string GetUserPath(const std::string& user);

    void RecordProgress(const std::string& id, uint64_t packages, uint64_t progress);

    void NewRecord(const std::string& id);

    void RecordErrorMsg(const std::string& id, const std::string& err_msg);

    void RecordTotalBytes(const std::string& id, uint64_t total_bytes);

    uint64_t GetProcessedPackages(const std::string& id);

    std::string GetImportProgress(const std::string& id, std::string& reason);

 private:
    std::unordered_map<std::string, ImportRecord> task_record_;
    lgraph::GlobalConfig* config_;
};

}  //  end of namespace http
}  //  end of namespace lgraph
