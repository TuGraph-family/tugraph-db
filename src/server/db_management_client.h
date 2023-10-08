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
#include "protobuf/tugraph_management.pb.h"
#include "gflags/gflags.h"
#include "brpc/channel.h"

namespace lgraph {
class DBManagementClient {
 private:
   bool heartbeat_ = false;
   static const int detect_freq_ = 5;
   brpc::Channel channel_;
 public:
   DBManagementClient();
   static DBManagementClient& GetInstance();
   void SetHeartbeat(bool heartbeat);
   bool GetHeartbeat();
   brpc::Channel& GetChannel();
   static void DetectHeartbeat();
   void CreateJob();
   void UpdateJob();
   void ReadJob();
   void ReadJobResult();
   void DeleteJob();
};
}  // namespace lgraph