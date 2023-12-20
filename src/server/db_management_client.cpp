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

#include "client/cpp/rpc/rpc_exception.h"
#include "server/db_management_client.h"

namespace lgraph {

DEFINE_string(mgr_protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(mgr_connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_int32(mgr_timeout_ms, 60 * 60 * 1000, "RPC timeout in milliseconds");
DEFINE_int32(mgr_max_retry, 3, "Max retries(not including the first RPC)");
DEFINE_string(mgr_load_balancer, "", "The algorithm for load balancing");
// DEFINE_bool(usercode_in_pthread);

DBManagementClient::DBManagementClient()
    : exit_(false),
      connected_(false),
      heartbeat_count_(0),
      heartbeat_interval_(10),
      channel_(std::make_shared<brpc::Channel>()) {}

void DBManagementClient::Init(const std::string& hostname, const uint16_t port,
                              const std::string& url) {
    host_ = hostname;
    port_ = std::to_string(port);

    // Initialize brpc channel to DbManagement
    brpc::ChannelOptions options;
    options.protocol = FLAGS_mgr_protocol;
    options.connection_type = FLAGS_mgr_connection_type;
    options.timeout_ms = FLAGS_mgr_timeout_ms;
    options.max_retry = FLAGS_mgr_max_retry;
    if (channel_->Init(url.c_str(), FLAGS_mgr_load_balancer.c_str(), &options) != 0) {
        throw RpcException("failed to initialize channel.");
    }
}

void DBManagementClient::DetectHeartbeat() {
    DbMgr::JobManagementService_Stub stub(channel_.get());
    static const uint64_t MAX_UINT64 = std::numeric_limits<uint64_t>::max();

    std::unique_lock<std::mutex> l(heartbeat_mutex_);
    while (!exit_) {
        brpc::Controller cntl;
        DbMgr::HeartbeatRequest request;
        DbMgr::HeartbeatResponse response;
        request.set_request_msg("this is a heartbeat request message.");
        request.set_heartbeat_count(heartbeat_count_);
        stub.detectHeartbeat(&cntl, &request, &response, nullptr);
        if (!connected_ && !cntl.Failed()) {
            GENERAL_LOG(INFO) << "connection to management service established";
        } else if (connected_ && cntl.Failed()) {
            GENERAL_LOG(INFO) << "connection to management service lost";
        }
        connected_ = !cntl.Failed();
        heartbeat_count_ = (heartbeat_count_ + 1) % MAX_UINT64;

        heartbeat_cond_.wait_for(l, std::chrono::seconds(heartbeat_interval_));
    }
}

void DBManagementClient::StopHeartbeat() {
    exit_ = true;
    heartbeat_cond_.notify_all();
}

void DBManagementClient::CreateJob(const std::string task_id, const std::string task_name,
                                   const int64_t create_time, const std::string period,
                                   const std::string name, const std::string type,
                                   const std::string user) {
    DbMgr::JobManagementRequest request;
    request.set_db_host(host_);
    request.set_db_port(port_);
    DbMgr::CreateJobRequest* creq = request.mutable_create_job_request();
    creq->set_task_id(task_id);
    creq->set_task_name(task_name);
    creq->set_start_time(create_time);  // start when created for procedure calls
    creq->set_period(period);
    creq->set_procedure_name(name);
    creq->set_procedure_type(type);
    creq->set_user(user);
    creq->set_create_time(create_time);

    DbMgr::JobManagementService_Stub stub(channel_.get());
    brpc::Controller cntl;
    DbMgr::JobManagementResponse response;
    stub.handleRequest(&cntl, &request, &response, nullptr);
    if (cntl.Failed()) {
        DEBUG_LOG(ERROR) << "[CreateJob REQUEST]: " << cntl.ErrorText();
        throw std::runtime_error("Request failed. Reason: " + cntl.ErrorText());
    }
    if (response.response_code() != DbMgr::ResponseCode::SUCCESS) {
        DEBUG_LOG(ERROR) << "[CreateJob REQUEST]: response code is not success.";
        throw std::runtime_error("failed to create job. Reason: " + response.message());
    }
    DEBUG_LOG(INFO) << "[CreateJob REQUEST]: success, TaskId is " << task_id;
}

void DBManagementClient::UpdateJobStatus(const std::string task_id, const std::string status,
                                         const std::int64_t runtime, const std::string result) {
    DbMgr::JobManagementRequest request;
    request.set_db_host(host_);
    request.set_db_port(port_);
    DbMgr::UpdateJobStatusRequest* ureq = request.mutable_update_job_status_request();
    ureq->set_task_id(task_id);
    ureq->set_status(status);
    ureq->set_runtime(runtime);
    ureq->set_result(result);

    DbMgr::JobManagementService_Stub stub(channel_.get());
    DbMgr::JobManagementResponse response;
    brpc::Controller cntl;
    stub.handleRequest(&cntl, &request, &response, nullptr);
    if (cntl.Failed()) {
        DEBUG_LOG(ERROR) << "[UpdateJobStatus REQUEST]: " << cntl.ErrorText();
        throw std::runtime_error("Request failed. Reason: " + cntl.ErrorText());
    }
    if (response.response_code() != DbMgr::ResponseCode::SUCCESS) {
        DEBUG_LOG(ERROR) << "[UpdateJobStatus REQUEST]: response code is not success.";
        throw std::runtime_error("failed to update job status. Reason: " + response.message());
    }

    DEBUG_LOG(INFO) << "[UpdateJobStatus REQUEST]: success";
}

std::vector<DbMgr::Job> DBManagementClient::GetJobStatus() {
    DbMgr::JobManagementRequest request;
    request.set_db_host(host_);
    request.set_db_port(port_);
    request.mutable_get_job_status_request();

    DbMgr::JobManagementService_Stub stub(channel_.get());
    DbMgr::JobManagementResponse response;
    brpc::Controller cntl;
    stub.handleRequest(&cntl, &request, &response, nullptr);
    if (cntl.Failed()) {
        DEBUG_LOG(ERROR) << "[GetJobStatus REQUEST]: " << cntl.ErrorText();
        throw std::runtime_error("Request failed. Reason: " + cntl.ErrorText());
    }
    if (response.response_code() != DbMgr::ResponseCode::SUCCESS) {
        DEBUG_LOG(ERROR) << "[GetJobStatus REQUEST]: response code is not success.";
        throw std::runtime_error("failed to get job status. Reason: " + response.message());
    }

    std::vector<DbMgr::Job> job_list;
    for (int i = 0; i < response.get_job_status_response().job_size(); i++) {
        job_list.push_back(response.get_job_status_response().job(i));
    }
    DEBUG_LOG(INFO) << "[GetJobStatus REQUEST]: success";
    return job_list;
}

DbMgr::Job DBManagementClient::GetJobStatusById(const std::string task_id) {
    DbMgr::JobManagementRequest request;
    request.set_db_host(host_);
    request.set_db_port(port_);
    request.mutable_get_job_status_request()->set_task_id(task_id);

    DbMgr::JobManagementService_Stub stub(channel_.get());
    DbMgr::JobManagementResponse response;
    brpc::Controller cntl;
    stub.handleRequest(&cntl, &request, &response, nullptr);
    if (cntl.Failed()) {
        DEBUG_LOG(ERROR) << "[GetJobStatusById REQUEST]: " << cntl.ErrorText();
        throw std::runtime_error("Request failed. Reason: " + cntl.ErrorText());
    }
    if (response.response_code() != DbMgr::ResponseCode::SUCCESS) {
        DEBUG_LOG(ERROR) << "[GetJobStatusById REQUEST]: response code is not success.";
        throw std::runtime_error("failed to get job status by id. Reason: " + response.message());
    }

    DEBUG_LOG(INFO) << "[GetJobStatusById REQUEST]: success";
    std::vector<DbMgr::Job> job_list;
    DbMgr::Job job;
    job = response.get_job_status_response().job(0);
    return job;
}

DbMgr::AlgoResult DBManagementClient::GetJobResult(const std::string task_id) {
    DbMgr::JobManagementRequest request;
    request.set_db_host(host_);
    request.set_db_port(port_);
    request.mutable_get_algo_result_request()->set_task_id(task_id);

    DbMgr::JobManagementService_Stub stub(channel_.get());
    DbMgr::JobManagementResponse response;
    brpc::Controller cntl;
    stub.handleRequest(&cntl, &request, &response, nullptr);
    if (cntl.Failed()) {
        DEBUG_LOG(ERROR) << "[GetJobResult REQUEST]: " << cntl.ErrorText();
        throw std::runtime_error("Request failed. Reason: " + cntl.ErrorText());
    }
    if (response.response_code() != DbMgr::ResponseCode::SUCCESS) {
        DEBUG_LOG(ERROR) << "[GetJobResult REQUEST]: response code is not success.";
        throw std::runtime_error("failed to get job result. Reason: " + response.message());
    }

    DEBUG_LOG(INFO) << "[GetJobResult REQUEST]: success";
    return response.get_algo_result_response().algo_result();
}

void DBManagementClient::DeleteJob(const std::string task_id) {
    DbMgr::JobManagementRequest request;
    request.set_db_host(host_);
    request.set_db_port(port_);
    request.mutable_delete_job_request()->set_task_id(task_id);

    DbMgr::JobManagementService_Stub stub(channel_.get());
    DbMgr::JobManagementResponse response;
    brpc::Controller cntl;
    stub.handleRequest(&cntl, &request, &response, nullptr);
    if (cntl.Failed()) {
        DEBUG_LOG(ERROR) << "[DeleteJob REQUEST]: " << cntl.ErrorText();
        throw std::runtime_error("Request failed. Reason: " + cntl.ErrorText());
    }
    if (response.response_code() != DbMgr::ResponseCode::SUCCESS) {
        DEBUG_LOG(ERROR) << "[DeleteJob REQUEST]: response code is not success.";
        throw std::runtime_error("failed to delete job. Reason: " + response.message());
    }

    DEBUG_LOG(INFO) << "[DeleteJob REQUEST]: success";
}
}  // namespace lgraph


