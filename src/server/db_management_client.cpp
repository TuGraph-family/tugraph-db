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

bool DBManagementClient::exit_flag = false;
std::mutex DBManagementClient::hb_mutex_;
std::condition_variable DBManagementClient::hb_cond_;

DBManagementClient::DBManagementClient()
    : job_stub_(db_management::JobManagementService_Stub(&channel_)),
      heartbeat_stub_(db_management::HeartbeatService_Stub(&channel_)) {
}

void DBManagementClient::InitChannel(std::string server) {
     // Initialize brpc channel to db_management.
    brpc::ChannelOptions options;
    options.protocol = "baidu_std";
    options.connection_type = "";
    options.timeout_ms = 1000;
    options.max_retry = 3;
    if (this->channel_.Init(server.c_str(), "", &options) != 0) {
        DEBUG_LOG(ERROR) << "Fail to initialize channel";
        std::runtime_error("failed to initialize channel.");
    }
}

void DBManagementClient::SetHeartbeat(bool heartbeat) {
    if (this->heartbeat_ == false && heartbeat == true) {
        GENERAL_LOG(INFO) << "connected to db management";
    } else if (this->heartbeat_ == true && heartbeat == false) {
        GENERAL_LOG(INFO) << "lost connection to db management";
    }
    this->heartbeat_ = heartbeat;
}

bool DBManagementClient::GetHeartbeat() {
    return this->heartbeat_;
}

void DBManagementClient::SetHeartbeatCount(int heartbeat_count) {
    this->heartbeat_count_ = heartbeat_count;
}

int DBManagementClient::GetHeartbeatCount() {
    return this->heartbeat_count_;
}

db_management::HeartbeatService_Stub& DBManagementClient::GetHeartbeatStub() {
    return this->heartbeat_stub_;
}

db_management::JobManagementService_Stub& DBManagementClient::GetJobStub() {
    return this->job_stub_;
}

DBManagementClient& DBManagementClient::GetInstance() {
    brpc::FLAGS_usercode_in_pthread = true;
    static DBManagementClient instance;
    return instance;
}

void DBManagementClient::DetectHeartbeat() {
    std::unique_lock<std::mutex> l(hb_mutex_);
    db_management::HeartbeatService_Stub& stub =
        DBManagementClient::GetInstance().GetHeartbeatStub();
    while (!exit_flag) {
        hb_cond_.wait_for(l, std::chrono::seconds(3));
        if (exit_flag) return;
        db_management::HeartbeatRequest request;
        db_management::HeartbeatResponse response;
        brpc::Controller cntl;
        int heartbeat_count = DBManagementClient::GetInstance().GetHeartbeatCount();
        request.set_request_msg("this is a heartbeat request message.");
        request.set_heartbeat_count(heartbeat_count);
        stub.detectHeartbeat(&cntl, &request, &response, NULL);
        if (!cntl.Failed()) {
            if (response.heartbeat_count() == heartbeat_count + 1) {
                DBManagementClient::GetInstance().SetHeartbeat(true);
                DBManagementClient::GetInstance().SetHeartbeatCount(heartbeat_count + 1);
            } else {
                DBManagementClient::GetInstance().SetHeartbeat(false);
                DBManagementClient::GetInstance().SetHeartbeatCount(0);
            }
        } else {
            DBManagementClient::GetInstance().SetHeartbeat(false);
            DBManagementClient::GetInstance().SetHeartbeatCount(0);
        }
    }
}

int DBManagementClient::CreateJob(std::string host, std::string port, std::int64_t start_time,
                                  std::string period, std::string name, std::string type,
                                  std::string user, std::int64_t create_time) {
    db_management::JobManagementService_Stub& stub =
        DBManagementClient::GetInstance().GetJobStub();
    db_management::JobManagementRequest request;
    db_management::JobManagementResponse response;
    brpc::Controller cntl;

    // build create_job_request
    request.set_db_host(host);
    request.set_db_port(port);
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
    db_management::JobManagementService_Stub& stub =
        DBManagementClient::GetInstance().GetJobStub();
    db_management::JobManagementRequest request;
    db_management::JobManagementResponse response;
    brpc::Controller cntl;

    // build create_job_request
    request.set_db_host(host);
    request.set_db_port(port);
    request.mutable_update_job_status_request()->set_job_id(job_id);
    request.mutable_update_job_status_request()->set_status(status);
    request.mutable_update_job_status_request()->set_runtime(runtime);
    request.mutable_update_job_status_request()->set_result(result);

    stub.handleRequest(&cntl, &request, &response, NULL);
    if (!cntl.Failed()) {
        DEBUG_LOG(INFO) << "[UPDATE JOB REQUEST]: " << "success";
    } else {
        DEBUG_LOG(ERROR) << "[UPDATE JOB REQUEST]: " << cntl.ErrorText();
        throw std::runtime_error("failed to connect to db management.");
    }
}

std::vector<db_management::Job> DBManagementClient::ReadJob(std::string host,
                                                            std::string port) {
    db_management::JobManagementService_Stub& stub =
        DBManagementClient::GetInstance().GetJobStub();
    db_management::JobManagementRequest request;
    db_management::JobManagementResponse response;
    brpc::Controller cntl;

    // build create_job_request
    request.set_db_host(host);
    request.set_db_port(port);
    request.mutable_get_job_status_request();

    stub.handleRequest(&cntl, &request, &response, NULL);
    if (!cntl.Failed()) {
        DEBUG_LOG(INFO) << "[READ JOB REQUEST]: " << "success";
        std::vector<db_management::Job> job_list;
        for (int i = 0; i < response.get_job_status_response().job_size(); i++) {
            job_list.push_back(response.get_job_status_response().job(i));
        }
        return job_list;
    } else {
        DEBUG_LOG(ERROR) << "[READ JOB REQUEST]: " << cntl.ErrorText();
        throw std::runtime_error("failed to connect to db management.");
    }
}

db_management::Job DBManagementClient::ReadJob(std::string host,
                                                            std::string port,
                                                            int job_id) {
    db_management::JobManagementService_Stub& stub =
        DBManagementClient::GetInstance().GetJobStub();
    db_management::JobManagementRequest request;
    db_management::JobManagementResponse response;
    brpc::Controller cntl;

    // build create_job_request
    request.set_db_host(host);
    request.set_db_port(port);
    request.mutable_get_job_status_request()->set_job_id(job_id);

    stub.handleRequest(&cntl, &request, &response, NULL);
    if (!cntl.Failed()) {
        DEBUG_LOG(INFO) << "[READ JOB REQUEST]: " << "success";
        std::vector<db_management::Job> job_list;
        db_management::Job job;
        job = response.get_job_status_response().job(0);
        return job;
    } else {
        DEBUG_LOG(ERROR) << "[READ JOB REQUEST]: " << cntl.ErrorText();
        throw std::runtime_error("failed to connect to db management.");
    }
}

db_management::AlgoResult DBManagementClient::ReadJobResult(std::string host,
                                                                    std::string port, int job_id) {
    db_management::JobManagementService_Stub& stub =
        DBManagementClient::GetInstance().GetJobStub();
    db_management::JobManagementRequest request;
    db_management::JobManagementResponse response;
    brpc::Controller cntl;

    // build create_job_request
    request.set_db_host(host);
    request.set_db_port(port);
    request.mutable_get_algo_result_request()->set_job_id(job_id);

    stub.handleRequest(&cntl, &request, &response, NULL);
    if (!cntl.Failed()) {
        DEBUG_LOG(INFO) << "[READ JOB RESULT REQUEST]: " << "success";
        return response.get_algo_result_response().algo_result();
    } else {
        DEBUG_LOG(ERROR) << "[READ JOB RESULT REQUEST]: " << cntl.ErrorText();
        throw std::runtime_error("failed to connect to db management.");
    }
}

void DBManagementClient::DeleteJob(std::string host, std::string port, int job_id) {
    db_management::JobManagementService_Stub& stub =
        DBManagementClient::GetInstance().GetJobStub();
    db_management::JobManagementRequest request;
    db_management::JobManagementResponse response;
    brpc::Controller cntl;

    // build create_job_request
    request.set_db_host(host);
    request.set_db_port(port);
    request.mutable_delete_job_request()->set_job_id(job_id);

    stub.handleRequest(&cntl, &request, &response, NULL);
    if (!cntl.Failed()) {
        DEBUG_LOG(INFO) << "[DELETE JOB REQUEST]: " << "success";
        return;
    } else {
        DEBUG_LOG(ERROR) << "[DELETE JOB REQUEST]: " << cntl.ErrorText();
        throw std::runtime_error("failed to connect to db management.");
    }
}
}  // namespace lgraph


