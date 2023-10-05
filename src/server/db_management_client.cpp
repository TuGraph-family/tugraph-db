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

#include "server/db_management_client.h"

namespace lgraph {
void DBManagementClient::DetectHeartbeat() {
    // Initialize brpc channel to db_management.
    brpc::Channel channel;
    brpc::ChannelOptions options;
    options.protocol = "baidu_std";
    options.connection_type = "";
    options.timeout_ms = 100/*milliseconds*/;
    options.max_retry = 3;
    if (channel.Init("localhost:6091", "", &options) != 0) {
        DEBUG_LOG(ERROR) << "Fail to initialize channel";
        return;
    }
    com::antgroup::tugraph::HeartbeatService_Stub stub(&channel);

    while (true) {
        DEBUG_LOG(ERROR) << "testing db management heart detection.";
        com::antgroup::tugraph::HeartbeatRequest request;
        com::antgroup::tugraph::HeartbeatResponse response;
        brpc::Controller cntl;
        request.set_request_msg("this is a heartbeat request message.");
        stub.detectHeartbeat(&cntl, &request, &response, NULL);
        if (!cntl.Failed()) {
            DEBUG_LOG(ERROR) << response.response_msg() << " "
                << " latency=" << cntl.latency_us() << "us";
        } else {
            DEBUG_LOG(ERROR) << cntl.ErrorText();
        }

        fma_common::SleepS(detect_freq_);
    }
}
}  // namespace lgraph


