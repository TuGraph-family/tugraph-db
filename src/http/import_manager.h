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
    enum TASKSTATE {
        PREPARE = 0,
        PROCESSING = 1,
        SUCCESS = 2,
        ERROR = 3
    };

    struct ImportRecord {
        std::string task_id_;
        std::string err_msg_;
        uint64_t processed_packages_;
        uint64_t processed_bytes_;
        uint64_t total_in_bytes_;
        bool done_;
        TASKSTATE state_;
        double start_timestamp_;
        double finish_timestamp_;

        ImportRecord()
            : task_id_("None")
            , err_msg_()
            , processed_packages_(0)
            , processed_bytes_(0.0)
            , total_in_bytes_(0)
            , done_(false)
            , state_(PREPARE) {}
    };

    ImportManager() {}

    void Init(lgraph::GlobalConfig* config);

    std::string GetRootPath();

    std::string GetUserPath(const std::string& user);

    void RecordProgress(const std::string& id, uint64_t packages, uint64_t progress);

    void NewRecord(const std::string& id);

    void RecordErrorMsg(const std::string& id,
                        const nlohmann::json& schema,
                        const std::string& err_msg);

    void RecordTotalBytes(const std::string& id, uint64_t total_bytes);

    void RecordProcessing(const std::string& id);

    void RecordFinish(const std::string& id, const nlohmann::json& schema, bool correct);

    uint64_t GetProcessedPackages(const std::string& id);

    int GetImportProgress(const std::string& id, std::string& progress, std::string& reason);

    void DeleteDataFiles(const std::string& id, const nlohmann::json& schema);

 private:
    std::unordered_map<std::string, ImportRecord> task_record_;
    lgraph::GlobalConfig* config_;
};

}  //  end of namespace http
}  //  end of namespace lgraph
