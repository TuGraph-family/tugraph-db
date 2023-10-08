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

DBManagementClient::DBManagementClient(){
    // Initialize brpc channel to db_management.
    brpc::ChannelOptions options;
    options.protocol = "baidu_std";
    options.connection_type = "";
    options.timeout_ms = 100/*milliseconds*/;
    options.max_retry = 3;
    if (this->channel_.Init("localhost:6091", "", &options) != 0) {
        DEBUG_LOG(ERROR) << "Fail to initialize channel";
        throw;
    }
}

void DBManagementClient::SetHeartbeat(bool heartbeat) {
    this->heartbeat_ = heartbeat;
}

bool DBManagementClient::GetHeartbeat() {
    return this->heartbeat_;
}

brpc::Channel& DBManagementClient::GetChannel() {
    return this->channel_;
}

DBManagementClient& DBManagementClient::GetInstance() {
    static DBManagementClient instance;
    return instance;
}

void DBManagementClient::DetectHeartbeat() {
    com::antgroup::tugraph::HeartbeatService_Stub stub(&DBManagementClient::GetInstance().GetChannel());
    while (true) {
        DEBUG_LOG(ERROR) << "testing db management heart detection.";
        com::antgroup::tugraph::HeartbeatRequest request;
        com::antgroup::tugraph::HeartbeatResponse response;
        brpc::Controller cntl;
        request.set_request_msg("this is a heartbeat request message.");
        stub.detectHeartbeat(&cntl, &request, &response, NULL);
        if (!cntl.Failed()) {
            DBManagementClient::GetInstance().SetHeartbeat(true);
            DEBUG_LOG(ERROR) << response.response_msg() << " "
                << " latency=" << cntl.latency_us() << "us";
        } else {
            DBManagementClient::GetInstance().SetHeartbeat(false);
            DEBUG_LOG(ERROR) << cntl.ErrorText();
        }

        fma_common::SleepS(detect_freq_);
    }
}

void DBManagementClient::CreateJob() {
    // // We will receive response synchronously, safe to put variables
    // // on stack.
    // com::antgroup::tugraph::JobManagementRequest request;
    // com::antgroup::tugraph::JobManagementResponse response;
    // brpc::Controller cntl;

    // request.set_db_host("127.0.0.1");
    // request.set_db_port("8888");

    // // test create_job_request
    // request.set_allocated_create_job_request(new com::antgroup::tugraph::CreateJobRequest());
    // request.mutable_create_job_request()->set_start_time(1694138458457);
    // request.mutable_create_job_request()->set_period("IMMEDIATE");
    // request.mutable_create_job_request()->set_procedure_name("Khop_test");
    // request.mutable_create_job_request()->set_procedure_type("Khop");
    // // request.mutable_create_job_request()->set_creator("lsl");
    // request.mutable_create_job_request()->set_create_time(1694138458457);


    // cntl.set_log_id(log_id ++);  // set by user
    // // Set attachment which is wired to network directly instead of
    // // being serialized into protobuf messages.
    // cntl.request_attachment().append(FLAGS_attachment);

    // // Because `done'(last parameter) is NULL, this function waits until
    // // the response comes back or error occurs(including timedout).
    // stub.handleRequest(&cntl, &request, &response, NULL);
    // if (!cntl.Failed()) {
    //     LOG(INFO) << "Received response from " << cntl.remote_side()
    //         << " to " << cntl.local_side()
    //         << ": " << response.response_code() << " (attached="
    //         << cntl.response_attachment() << ")"
    //         << " latency=" << cntl.latency_us() << "us";
    //     return response.create_job_response().job_id();
    // } else {
    //     LOG(WARNING) << cntl.ErrorText();
    //     return -1;
    // }
    return;
}

void DBManagementClient::UpdateJob() {
    return;
}

void DBManagementClient::ReadJob() {
    return;
}

void DBManagementClient::ReadJobResult() {
    return;
}

void DBManagementClient::DeleteJob() {
    return;
}
}  // namespace lgraph


