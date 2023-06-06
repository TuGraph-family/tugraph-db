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
#include "core/global_config.h"
#include "db/galaxy.h"
#include "server/state_machine.h"
#include "http/import_manager.h"
#include "fma-common/thread_pool.h"

namespace lgraph {
namespace http {

typedef std::string string_t;

class HttpService : public LGraphHttpService {
 public:
    friend class ImportTask;

    static const string_t HTTP_CYPHER_METHOD;
    static const uint16_t HTTP_CYPHER_METHOD_IDX;
    static const string_t HTTP_REFRESH_METHOD;
    static const uint16_t HTTP_REFRESH_METHOD_IDX;
    static const string_t HTTP_LOGIN_METHOD;
    static const uint16_t HTTP_LOGIN_METHOD_IDX;
    static const string_t HTTP_LOGOUT_METHOD;
    static const uint16_t HTTP_LOGOUT_METHOD_IDX;
    static const string_t HTTP_UPLOAD_METHOD;
    static const uint16_t HTTP_UPLOAD_METHOD_IDX;
    static const string_t HTTP_CLEAR_CACHE_METHOD;
    static const uint16_t HTTP_CLEAR_CACHE_METHOD_IDX;
    static const string_t HTTP_CHECK_FILE_METHOD;
    static const uint16_t HTTP_CHECK_FILE_METHOD_IDX;
    static const string_t HTTP_IMPORT_METHOD;
    static const uint16_t HTTP_IMPORT_METHOD_IDX;
    static const string_t HTTP_IMPORT_PROGRESS_METHOD;
    static const uint16_t HTTP_IMPORT_PROGRESS_METHOD_IDX;
    static const string_t HTTP_IMPORT_SCHEMA_METHOD;
    static const uint16_t HTTP_IMPORT_SCHEMA_METHOD_IDX;

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

    void Query(google::protobuf::RpcController* cntl_base,
               const HttpRequest*,
               HttpResponse*,
               google::protobuf::Closure* done);

 private:
    void InitFuncMap();

    void DispatchRequest(const std::string& method, brpc::Controller* cntl, std::string& res);

    void DoCypherRequest(const brpc::Controller* cntl, std::string& res);

    void DoLoginRequest(const brpc::Controller* cntl, std::string& res);

    void DoRefreshRequest(const brpc::Controller* cntl, std::string& res);

    void DoLogoutRequest(const brpc::Controller* cntl);

    void DoUploadRequest(brpc::Controller* cntl);

    void DoClearCache(brpc::Controller* cntl);

    void DoCheckFile(brpc::Controller* cntl, std::string& res);

    void DoImportFile(brpc::Controller* cntl, std::string& res);

    void DoImportSchema(brpc::Controller* cntl, std::string& res);

    void DoImportProgress(brpc::Controller* cntl, std::string& res);

    void BuildPbCypherRequest(const brpc::Controller* cntl,
                              const std::string& token,
                              LGraphRequest& pb);

    void BuildJsonCypherResponse(LGraphResponse& res, std::string& json);

    void ProcessSchemaRequest(const std::string& graph, const std::string& desc,
                              const std::string& token, LGraphRequest& req);

    int OpenUserFile(const std::string& token, std::string file_name);

    uint64_t GetSerialNumber() {
        return serial_number_.fetch_add(1);
    }

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

    StateMachine* sm_;
    lgraph::Galaxy* galaxy_;
    ImportManager import_manager_;
    fma_common::ThreadPool pool_;
    std::atomic<uint64_t> serial_number_;
    std::unordered_map<std::string, uint16_t> functions_map_;
};

}  // end of namespace http
}  // end of namespace lgraph
