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

#include "tools/lgraph_log.h"
#include "fma-common/utils.h"
#include "protobuf/tugraph_db_management.pb.h"
#include "gflags/gflags.h"
#include "brpc/channel.h"

namespace lgraph {
namespace db_management = com::antgroup::tugraph;
class DBManagementClient {
  // TODO(qsp): get host and port from license
  // TODO(qsp): change name of ReadJobResult
  // TODO(qsp): kill all process after ut
 private:
  bool heartbeat_ = false;
  int heartbeat_count_ = 0;
  static const int detect_freq_ = 5;
  brpc::Channel channel_;
  db_management::JobManagementService_Stub job_stub_;
  db_management::HeartbeatService_Stub heartbeat_stub_;

 public:
  DBManagementClient();

  /**
    * @brief   Get an instance of DBManagementClient.
    */
  static DBManagementClient& GetInstance();

  /**
    * @brief   Init brpc channel.
    *
    * @param   server   The server address of db management, like "localhost:6091".
    */
  void InitChannel(std::string sever);

  /**
    * @brief   set heartbeat of db management.
    *
    * @param   heartbeat   true if connected, false if not connected.
    */
  void SetHeartbeat(bool heartbeat);

  /**
    * @brief   Get heartbeat of db management.
    *
    * @returns   true if connected, false if not.
    */
  bool GetHeartbeat();

  /**
    * @brief   Set heartbeat count of db management.
    *
    * @param   heartbeat_count  count of heartbeat.
    */
  void SetHeartbeatCount(int heartbeat_count);

  /**
    * @brief   Get heartbeat count of db management.
    *
    * @returns   heartbeat count.
    */
  int GetHeartbeatCount();

  /**
    * @brief   Get brpc stub for heart detection.
    c
    */
  db_management::HeartbeatService_Stub& GetHeartbeatStub();

  /**
    * @brief   Get brpc stub for job management.
    *
    * @returns   job management stub.
    */
  db_management::JobManagementService_Stub& GetJobStub();

  /**
    * @brief  Heartbeat detection function. Runs in a detached thread.
    *
    */
  static void DetectHeartbeat();

  /**
    * @brief   create a job record in db management.
    *
    * @param   host   host address of current db.
    *
    * @param   port   port of current db.
    *
    * @param   start_time   start time of a job, echo time in ms, int64.
    *
    * @param   period   job period type, PERIODIC, IMMEDIATE, DELAYED.
    *
    * @param   name   name of this job.
    *
    * @param   type   type of this job.
    *
    * @param   user   user of db who create this job.
    *
    * @param   create_time   create time of a job, echo time in ms, int64.
    *
    * @returns   unique job_id for created job record.
    */
  int CreateJob(std::string host, std::string port, std::int64_t start_time, std::string period,
                std::string name, std::string type, std::string user, std::int64_t create_time);

  /**
    * @brief   update a job record in db management with job_id.
    *
    * @param   host   host address of current db.
    *
    * @param   port   port of current db.
    *
    * @param   status   job status, pending, success, failed.
    *
    * @param   runtime   total runtime of a job, echo time in ms, int64.
    *
    * @param   result   result of job in string.
    */
  void UpdateJob(std::string host, std::string port, int job_id, std::string status,
                 std::int64_t runtime, std::string result);

  /**
    * @brief   read all job status in db management.
    *
    * @param   host   host address of current db.
    *
    * @param   port   port of current db.
    *
    * @returns   a list for all job status in db management.
    */
  std::vector<db_management::Job> ReadJob(std::string host, std::string port);

  /**
    * @brief   read job status with given job_id in db management.
    *
    * @param   host   host address of current db.
    *
    * @param   port   port of current db.
    *
    * @param   job_id   job_id of the job you want to read.
    *
    * @returns   job.
    */
  db_management::Job ReadJob(std::string host, std::string port, int job_id);

  /**
    * @brief   read job result with given job_id in db management.
    *
    * @param   host   host address of current db.
    *
    * @param   port   port of current db.
    *
    * @param   job_id   job_id of the job you want to read.
    *
    * @returns   job result.
    */
  db_management::JobResult ReadJobResult(std::string host, std::string port, int job_id);

  /**
    * @brief   delete a job record with given job_id in db management.
    *
    * @param   host   host address of current db.
    *
    * @param   port   port of current db.
    *
    * @param   job_id   job_id of the job you want to read.
    */
  void DeleteJob(std::string host, std::string port, int job_id);
};
}  // namespace lgraph
