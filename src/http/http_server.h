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

#pragma once

#include <brpc/server.h>
#include <brpc/restful.h>
#include "protobuf/ha.pb.h"
#include "protobuf/tugraph_db_management.pb.h"
#include "core/global_config.h"
#include "db/galaxy.h"
#include "server/state_machine.h"
#include "http/import_manager.h"
#include "fma-common/thread_pool.h"

namespace lgraph {
namespace http {

// functions for each url
typedef const std::function<void(const brpc::Controller*, std::string&)> bindingFunction;

class HttpService : public LGraphHttpService {
 public:
    friend class ImportTask;
    friend class AlgorithmTask;

    struct DoneClosure : public google::protobuf::Closure {
        std::mutex mu_;
        std::condition_variable cv_;
        bool done_ = false;

     public:
        void Run() {
            mu_.lock();
            done_ = true;
            mu_.unlock();
            cv_.notify_all();
        }

        void Wait() {
            std::unique_lock<std::mutex> l(mu_);
            while (!done_) cv_.wait(l);
        }
    };

 public:
    explicit HttpService(StateMachine* state_machine);

    virtual ~HttpService() {}

    void Start(lgraph::GlobalConfig* config);

    void Query(google::protobuf::RpcController* cntl_base, const HttpRequest*, HttpResponse*,
               google::protobuf::Closure* done);

 private:
    void InitFuncMap();

    void DoCypherRequest(const brpc::Controller* cntl, std::string& res);

    void DoGqlRequest(const brpc::Controller* cntl, std::string& res);

    void DoLoginRequest(const brpc::Controller* cntl, std::string& res);

    void DoRefreshRequest(const brpc::Controller* cntl, std::string& res);

    void DoLogoutRequest(const brpc::Controller* cntl, std::string& res);

    void DoUploadRequest(const brpc::Controller* cntl, std::string& res);

    void DoClearCache(const brpc::Controller* cntl, std::string& res);

    void DoCheckFile(const brpc::Controller* cntl, std::string& res);

    void DoImportFile(const brpc::Controller* cntl, std::string& res);

    void DoImportSchema(const brpc::Controller* cntl, std::string& res);

    void DoImportProgress(const brpc::Controller* cntl, std::string& res);

    void DoUploadProcedure(const brpc::Controller* cntl, std::string& res);

    void DoListProcedures(const brpc::Controller* cntl, std::string& res);

    void DoGetProcedure(const brpc::Controller* cntl, std::string& res);

    void DoGetProcedureDemo(const brpc::Controller* cntl, std::string& res);

    void DoDeleteProcedure(const brpc::Controller* cntl, std::string& res);

    void DoCallProcedure(const brpc::Controller* cntl, std::string& res);

    void DoCreateProcedureJob(const brpc::Controller* cntl, std::string& res);

    void DoListProcedureJobs(const brpc::Controller* cntl, std::string& res);

    void DoGetProcedureJobResult(const brpc::Controller* cntl, std::string& res);

    void BuildPbGraphQueryRequest(const brpc::Controller* cntl,
                                  const lgraph_api::GraphQueryType& query_type,
                                  const std::string& token, LGraphRequest& pb);

    void BuildJsonGraphQueryResponse(LGraphResponse& res, std::string& json);

    void BuildrocedureV1Request(const brpc::Controller* cntl, const std::string& token,
                                LGraphRequest& pb);

    void BuildrocedureV2Request(const brpc::Controller* cntl, const std::string& token,
                                LGraphRequest& pb);

    void ProcessSchemaRequest(const std::string& graph, const std::string& desc,
                              const std::string& token, LGraphRequest& req);

    int OpenUserFile(const std::string& token, std::string file_name);

    uint64_t GetSerialNumber() { return serial_number_.fetch_add(1); }

    uint64_t GetAlgoTaskSeq() { return algo_task_seq_.fetch_add(1); }

    void DeleteSpecifiedFile(const std::string& token, const std::string& file_name);

    void DeleteSpecifiedUserFiles(const std::string& token, const std::string& user_name);

    void DeleteAllUserFiles(const std::string& token);

    void ApplyToStateMachine(LGraphRequest& req, LGraphResponse& res);

    void AddAccessControlCORS(brpc::Controller* cntl);

    void RespondUnauthorized(brpc::Controller* cntl, const std::string& res) const;

    void RespondSuccess(brpc::Controller* cntl, const std::string& res) const;

    void RespondInternalError(brpc::Controller* cntl, const std::string& res) const;

    void RespondBadRequest(brpc::Controller* cntl, const std::string& res) const;

    void AdjustFilePath(const std::string& req, const std::string& user, nlohmann::json& schema);

    off_t GetFileSize(const std::string& file_name);

    std::string CheckTokenOrThrowException(const brpc::Controller* cntl) const;

    std::string GetOrCreateTaskId(const brpc::Controller* cntl) const;

    StateMachine* sm_;
    lgraph::Galaxy* galaxy_;
    std::string resource_dir_;
    ImportManager import_manager_;
    fma_common::ThreadPool import_pool_;
    fma_common::ThreadPool algo_pool_;
    std::atomic<uint64_t> serial_number_;
    std::atomic<uint64_t> algo_task_seq_;
    std::unordered_map<std::string, bindingFunction> functions_map_;
};

}  // end of namespace http
}  // end of namespace lgraph
