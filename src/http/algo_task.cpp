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

#include "http/algo_task.h"
#include "core/defs.h"
#include "fma-common/utils.h"
#include "server/db_management_client.h"

namespace lgraph {
namespace http {

AlgorithmTask::AlgorithmTask(HttpService* http_service, const std::string& taskId,
                             const std::string& taskName, const std::string& user,
                             const std::string& token, const std::string& graphName,
                             const std::string& algoName, const std::string& algoType,
                             const std::string& version, const std::string& jobParam,
                             const std::string& nodeType, const std::string& edgeType,
                             const std::string& outputType, const double timeout,
                             const bool inProcess)
    : http_service_(http_service),
      taskId_(std::move(taskId)),
      taskName_(std::move(taskName)),
      user_(std::move(user)),
      token_(std::move(token)),
      graphName_(std::move(graphName)),
      algoName_(std::move(algoName)),
      algoType_(algoType),
      version_(std::move(version)),
      jobParam_(std::move(jobParam)),
      nodeType_(std::move(nodeType)),
      edgeType_(std::move(edgeType)),
      outputType_(std::move(outputType)),
      timeout_(timeout),
      inProcess_(inProcess) {}

void AlgorithmTask::operator()() {
    std::string status, result;
    lgraph_api::DateTime start_time(std::chrono::system_clock::now());
    const int64_t start_epoch_ms = start_time.MicroSecondsSinceEpoch();

    try {
        DBManagementClient::GetInstance().CreateJob(taskId_, taskName_, start_epoch_ms, "IMMEDIATE",
                                                    algoName_, algoType_, user_);

        // TODO(qishipeng.qsp): implement the task when v1 algorithms are ready
        fma_common::SleepS(10);  // simulate the algorithm task

        status = "finished";
        result = "simulation done";
    } catch (std::exception& e) {
        status = "failed";
        result = e.what();
        LOG_INFO() << "Failed to CreateJob: " << e.what();
    }

    lgraph_api::DateTime finish_time(std::chrono::system_clock::now());
    const int64_t runtime = finish_time.MicroSecondsSinceEpoch() - start_epoch_ms;
    try {
        DBManagementClient::GetInstance().UpdateJobStatus(taskId_, status, runtime, result);
    } catch (std::exception& e) {
        LOG_INFO() << "Failed to UpdateJobStatus: " << e.what();
    }
}

}  //  end of namespace http
}  //  end of namespace lgraph
