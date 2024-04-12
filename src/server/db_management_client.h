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

#pragma once

#include <condition_variable>
#include "tools/lgraph_log.h"
#include "fma-common/utils.h"
#include "protobuf/tugraph_db_management.pb.h"
#include "gflags/gflags.h"
#include "brpc/channel.h"

namespace lgraph {
namespace DbMgr = lgraph::management;

class DBManagementClient {
 private:
    // DbMgr::JobManagementService_Stub stub_;
    bool exit_;
    bool connected_;
    uint64_t heartbeat_count_;
    int heartbeat_interval_;
    std::shared_ptr<brpc::Channel> channel_;
    std::condition_variable heartbeat_cond_;
    std::mutex heartbeat_mutex_;
    std::string host_;
    std::string port_;

 public:
    DBManagementClient();

    DISABLE_COPY(DBManagementClient);
    DISABLE_MOVE(DBManagementClient);

    static DBManagementClient& GetInstance() {
        // brpc::FLAGS_usercode_in_pthread = true;
        static DBManagementClient instance;
        return instance;
    }

    void Init(const std::string& hostname, const uint16_t port, const std::string& url);

    bool GetConnected() { return connected_; }

    void DetectHeartbeat();

    void StopHeartbeat();

    /**
     * @brief   create a job record in db management.
     * @param   start_time   start time of a job, echo time in ms, int64.
     * @param   period   job period type, PERIODIC, IMMEDIATE, DELAYED.
     * @param   name   name of this job.
     * @param   type   type of this job.
     * @param   user   user of db who create this job.
     *
     * @returns   unique job_id for created job record.
     */
    void CreateJob(const std::string task_id, const std::string task_name,
                   const int64_t create_time, const std::string period, const std::string name,
                   const std::string type, const std::string user);

    /**
     * @brief   update a job record in db management with job_id.
     * @param   status   job status, pending, success, failed.
     * @param   runtime   total runtime of a job, echo time in ms, int64.
     * @param   result   result of job in string.
     */
    void UpdateJobStatus(const std::string task_id, const std::string status,
                         const std::int64_t runtime, const std::string result);

    /**
     * @brief   get all job status in db management.
     *
     * @returns   a list for all job status in db management.
     */
    std::vector<DbMgr::Job> GetJobStatus();

    /**
     * @brief   get job status with given job_id in db management.
     * @param   job_id
     *
     * @returns job.
     */
    DbMgr::Job GetJobStatusById(const std::string task_id);

    /**
     * @brief   get job result with given job_id in db management.
     * @param   job_id   job_id of the job you want to get.
     *
     * @returns job result.
     */
    DbMgr::AlgoResult GetJobResult(const std::string task_id);

    /**
     * @brief   delete a job record with given job_id in db management.
     * @param   job_id   job_id of the job you want to get.
     */
    void DeleteJob(const std::string task_id);
};
}  // namespace lgraph
