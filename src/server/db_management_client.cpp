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

DBManagementClient::DBManagementClient()
    : job_stub_(com::antgroup::tugraph::JobManagementService_Stub(&channel_)),
      heartbeat_stub_(com::antgroup::tugraph::HeartbeatService_Stub(&channel_)) {
    // Initialize brpc channel to db_management.
    brpc::ChannelOptions options;
    options.protocol = "baidu_std";
    options.connection_type = "";
    options.timeout_ms = 1000/*milliseconds*/;
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

com::antgroup::tugraph::HeartbeatService_Stub& DBManagementClient::GetHeartbeatStub() {
    return this->heartbeat_stub_;
}

com::antgroup::tugraph::JobManagementService_Stub& DBManagementClient::GetJobStub() {
    return this->job_stub_;
}

DBManagementClient& DBManagementClient::GetInstance() {
    static DBManagementClient instance;
    return instance;
}

void DBManagementClient::DetectHeartbeat() {
    com::antgroup::tugraph::HeartbeatService_Stub& stub =
        DBManagementClient::GetInstance().GetHeartbeatStub();
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

int DBManagementClient::CreateJob(std::string host, std::string port, std::int64_t start_time,
                                  std::string period, std::string name, std::string type,
                                  std::string user, std::int64_t create_time) {
    com::antgroup::tugraph::JobManagementService_Stub& stub =
        DBManagementClient::GetInstance().GetJobStub();
    com::antgroup::tugraph::JobManagementRequest request;
    com::antgroup::tugraph::JobManagementResponse response;
    brpc::Controller cntl;

    // build create_job_request
    request.set_db_host(host);
    request.set_db_port(port);
    request.set_allocated_create_job_request(new com::antgroup::tugraph::CreateJobRequest());
    request.mutable_create_job_request()->set_start_time(start_time);
    request.mutable_create_job_request()->set_period(period);
    request.mutable_create_job_request()->set_procedure_name(name);
    request.mutable_create_job_request()->set_procedure_type(type);
    request.mutable_create_job_request()->set_user(user);
    request.mutable_create_job_request()->set_create_time(create_time);

    stub.handleRequest(&cntl, &request, &response, NULL);
    if (!cntl.Failed()) {
        int job_id = response.create_job_response().job_id();
        DEBUG_LOG(INFO) << "[CREATE JOB REQUEST]: " << "success, JobId is " << job_id;
        return job_id;
    } else {
        DEBUG_LOG(ERROR) << "[CREATE JOB REQUEST]: " << cntl.ErrorText();
        throw std::runtime_error("failed to connect to db management.");
    }
}

void DBManagementClient::UpdateJob(std::string host, std::string port, int job_id,
                                   std::string status, std::int64_t runtime,
                                   std::string result) {
    com::antgroup::tugraph::JobManagementService_Stub& stub =
        DBManagementClient::GetInstance().GetJobStub();
    com::antgroup::tugraph::JobManagementRequest request;
    com::antgroup::tugraph::JobManagementResponse response;
    brpc::Controller cntl;

    // build create_job_request
    request.set_db_host(host);
    request.set_db_port(port);
    request.set_allocated_update_job_request(new com::antgroup::tugraph::UpdateJobRequest());
    request.mutable_update_job_request()->set_job_id(job_id);
    request.mutable_update_job_request()->set_status(status);
    request.mutable_update_job_request()->set_runtime(runtime);
    request.mutable_update_job_request()->set_result(result);

    stub.handleRequest(&cntl, &request, &response, NULL);
    if (!cntl.Failed()) {
        DEBUG_LOG(INFO) << "[UPDATE JOB REQUEST]: " << "success";
    } else {
        DEBUG_LOG(ERROR) << "[UPDATE JOB REQUEST]: " << cntl.ErrorText();
        throw std::runtime_error("failed to connect to db management.");
    }
}

std::vector<com::antgroup::tugraph::Job> DBManagementClient::ReadJob(std::string host,
                                                                     std::string port) {
    com::antgroup::tugraph::JobManagementService_Stub& stub =
        DBManagementClient::GetInstance().GetJobStub();
    com::antgroup::tugraph::JobManagementRequest request;
    com::antgroup::tugraph::JobManagementResponse response;
    brpc::Controller cntl;

    // build create_job_request
    request.set_db_host(host);
    request.set_db_port(port);
    request.set_allocated_read_job_request(new com::antgroup::tugraph::ReadJobRequest());

    stub.handleRequest(&cntl, &request, &response, NULL);
    if (!cntl.Failed()) {
        DEBUG_LOG(INFO) << "[READ JOB REQUEST]: " << "success";
        std::vector<com::antgroup::tugraph::Job> job_list;
        for (int i = 0; i < response.read_job_response().job_size(); i++) {
            job_list.push_back(response.read_job_response().job(i));
        }
        return job_list;
    } else {
        DEBUG_LOG(ERROR) << "[READ JOB REQUEST]: " << cntl.ErrorText();
        throw std::runtime_error("failed to connect to db management.");
    }
}

com::antgroup::tugraph::JobResult DBManagementClient::ReadJobResult(std::string host,
                                                                    std::string port, int job_id) {
    com::antgroup::tugraph::JobManagementService_Stub& stub =
        DBManagementClient::GetInstance().GetJobStub();
    com::antgroup::tugraph::JobManagementRequest request;
    com::antgroup::tugraph::JobManagementResponse response;
    brpc::Controller cntl;

    // build create_job_request
    request.set_db_host(host);
    request.set_db_port(port);
    request.set_allocated_read_job_result_request(
            new com::antgroup::tugraph::ReadJobResultRequest());
    request.mutable_read_job_result_request()->set_job_id(job_id);

    stub.handleRequest(&cntl, &request, &response, NULL);
    if (!cntl.Failed()) {
        DEBUG_LOG(INFO) << "[READ JOB RESULT REQUEST]: " << "success";
        return response.read_job_result_response().job_result();
    } else {
        DEBUG_LOG(ERROR) << "[READ JOB REQUEST]: " << cntl.ErrorText();
        throw std::runtime_error("failed to connect to db management.");
    }
}

void DBManagementClient::DeleteJob(std::string host, std::string port, int job_id) {
    com::antgroup::tugraph::JobManagementService_Stub& stub =
        DBManagementClient::GetInstance().GetJobStub();
    com::antgroup::tugraph::JobManagementRequest request;
    com::antgroup::tugraph::JobManagementResponse response;
    brpc::Controller cntl;

    // build create_job_request
    request.set_db_host(host);
    request.set_db_port(port);
    request.set_allocated_delete_job_request(new com::antgroup::tugraph::DeleteJobRequest());
    request.mutable_delete_job_request()->set_job_id(job_id);

    stub.handleRequest(&cntl, &request, &response, NULL);
    if (!cntl.Failed()) {
        DEBUG_LOG(INFO) << "[DELETE JOB REQUEST]: " << "success";
        return;
    } else {
        DEBUG_LOG(ERROR) << "[READ JOB REQUEST]: " << cntl.ErrorText();
        throw std::runtime_error("failed to connect to db management.");
    }
}
}  // namespace lgraph


