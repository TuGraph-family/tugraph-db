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

#include <boost/lexical_cast.hpp>
#include "http/import_manager.h"
#include "http/http_utils.h"

namespace lgraph {
namespace http {

void ImportManager::Init(lgraph::GlobalConfig* config) {
    config_ = config;
}

std::string ImportManager::GetRootPath() {
    return config_->db_dir + "/" + HTTP_FILE_DIRECTORY;
}

std::string ImportManager::GetUserPath(const std::string& user) {
    return GetRootPath() + "/" + user;
}

void ImportManager::NewRecord(const std::string& id) {
    ImportRecord& record = task_record_[id];
    if (record.task_id_ == "None") {
        record.task_id_ = id;
    } else {
        record.err_msg_ = "";
        record.done_ = false;
        record.state_ = PREPARE;
    }
}

uint64_t ImportManager::GetProcessedPackages(const std::string& id) {
    auto it = task_record_.find(id);
    if (it == task_record_.end()) throw lgraph_api::BadRequestException("Not a valid task id");
    return it->second.processed_packages_;
}

void ImportManager::RecordErrorMsg(const std::string& id,
                                   const nlohmann::json& schema,
                                   const std::string& err_msg) {
    ImportRecord& record = task_record_[id];
    if (record.err_msg_.empty()) {
        record.err_msg_ = err_msg;
    } else {
        std::string append_str = " /n " + err_msg;
        record.err_msg_ += err_msg;
    }
    RecordFinish(id, schema, false);
}

void ImportManager::RecordProgress(const std::string& id, uint64_t packages, uint64_t progress) {
    ImportRecord& record = task_record_[id];
    record.processed_packages_ += packages;
    record.processed_bytes_ += progress;
}

void ImportManager::RecordTotalBytes(const std::string& id, uint64_t total_bytes) {
    ImportRecord& record = task_record_[id];
    record.total_in_bytes_ = total_bytes;
}

void ImportManager::RecordProcessing(const std::string& id) {
    ImportRecord& record = task_record_[id];
    record.state_ = PROCESSING;
    record.start_timestamp_ = fma_common::GetTime();
}

void ImportManager::RecordFinish(const std::string& id,
                                 const nlohmann::json& schema, bool correct) {
    ImportRecord& record = task_record_[id];
    record.done_ = true;
    if (correct) {
        record.state_ = SUCCESS;
    } else {
        record.state_ = ERROR;
    }
    record.finish_timestamp_ = fma_common::GetTime();
    DeleteDataFiles(id, schema);
}

int ImportManager::GetImportProgress(const std::string& id,
                                     std::string& progress,
                                     std::string& reason) {
    auto it = task_record_.find(id);
    if (it == task_record_.end()) throw lgraph_api::BadRequestException("Not a valid task id");

    switch (it->second.state_) {
    case PROCESSING: {
            if (it->second.total_in_bytes_ == 0) return SUCCESS;
            double p = (double)it->second.processed_bytes_ / (double)it->second.total_in_bytes_;
            try {
                progress = boost::lexical_cast<std::string>(p);
            } catch (boost::bad_lexical_cast& e) {
                throw lgraph_api::InternalErrorException("current progress cannot be obtained");
            }
            return PROCESSING;
        }
    case ERROR: {
            reason = it->second.err_msg_;
            return ERROR;
        }
    case SUCCESS:
        return SUCCESS;
    case PREPARE:
        return PREPARE;
    }
    return PREPARE;
}

void ImportManager::DeleteDataFiles(const std::string& id, const nlohmann::json& schema) {
    const nlohmann::json& files = schema.at(HTTP_FILES);
    for (size_t idx = 0; idx < files.size(); ++idx) {
        const nlohmann::json& item = files[idx];
        std::string file = item[HTTP_PATH].get<std::string>();
        fma_common::LocalFileSystem fs;
        if (!fs.Remove(file)) {
            FMA_LOG() << FMA_FMT("{} file remove failed in {} task", file, id);
        }
    }
}

}  //  end of namespace http
}  //  end of namespace lgraph
