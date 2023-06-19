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
    if (record.task_id_ == "None")
        record.task_id_ = id;
}

uint64_t ImportManager::GetProcessedPackages(const std::string& id) {
    auto it = task_record_.find(id);
    if (it == task_record_.end()) throw lgraph_api::BadRequestException("Not a valid task id");
    return it->second.processed_packages_;
}

void ImportManager::RecordErrorMsg(const std::string& id, const std::string& err_msg) {
    ImportRecord& record = task_record_[id];
    if (record.err_msg_.empty()) {
        record.err_msg_ = err_msg;
    } else {
        std::string append_str = " /n " + err_msg;
        record.err_msg_ += err_msg;
    }
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

std::string ImportManager::GetImportProgress(const std::string& id, std::string& reason) {
    auto it = task_record_.find(id);
    if (it == task_record_.end()) throw lgraph_api::BadRequestException("Not a valid task id");
    if (it->second.err_msg_.size()) reason = it->second.err_msg_;
    if (it->second.processed_bytes_ == 0 || it->second.total_in_bytes_ == 0) return "0";
    double progress = (double)it->second.processed_bytes_ / (double)it->second.total_in_bytes_;
    std::string ret;
    try {
        ret = boost::lexical_cast<std::string>(progress);
    } catch (boost::bad_lexical_cast& e) {
        throw lgraph_api::InternalErrorException(
            "current progress cannot be obtained");
    }
    return ret;
}

}  //  end of namespace http
}  //  end of namespace lgraph
