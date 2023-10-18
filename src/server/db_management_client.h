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
 private:
  bool heartbeat_ = false;
  static const int detect_freq_ = 5;
  brpc::Channel channel_;
  db_management::JobManagementService_Stub job_stub_;
  db_management::HeartbeatService_Stub heartbeat_stub_;

 public:
  DBManagementClient();

  static DBManagementClient& GetInstance();

  void InitChannel(std::string sever);

  void SetHeartbeat(bool heartbeat);

  bool GetHeartbeat();

  db_management::HeartbeatService_Stub& GetHeartbeatStub();

  db_management::JobManagementService_Stub& GetJobStub();

  static void DetectHeartbeat();

  int CreateJob(std::string host, std::string port, std::int64_t start_time, std::string period,
                std::string name, std::string type, std::string user, std::int64_t create_time);

  void UpdateJob(std::string host, std::string port, int job_id, std::string status,
                 std::int64_t runtime, std::string result);

  std::vector<db_management::Job> ReadJob(std::string host, std::string port);

  db_management::Job ReadJob(std::string host, std::string port, int job_id);

  db_management::JobResult ReadJobResult(std::string host, std::string port, int job_id);

  void DeleteJob(std::string host, std::string port, int job_id);
};
}  // namespace lgraph
