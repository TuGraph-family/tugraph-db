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
#include "core/global_config.h"
#include "server/service.h"
#include "server/state_machine.h"
#include "restful/server/rest_server.h"
#include "http/http_server.h"

#ifndef _WIN32
#include "brpc/server.h"
#include "butil/logging.h"
#endif

namespace lgraph {
class Signal {
    bool signal;
    std::mutex lock;
    std::condition_variable cv;

 public:
    Signal() : signal(false) {}

    bool Wait(double timeout_s = 0) {
        std::unique_lock<std::mutex> l(lock);
        if (timeout_s <= 0) {
            cv.wait(l, [this]() { return signal; });
            return true;
        } else {
            cv.wait_for(l, std::chrono::milliseconds((size_t)(timeout_s * 1000)),
                        [this]() { return signal; });
            return signal;
        }
    }

    void Notify() {
        std::lock_guard<std::mutex> l(lock);
        signal = true;
        cv.notify_all();
    }
};

class RPCService : public lgraph::LGraphRPCService {
 public:
    explicit RPCService(lgraph::StateMachine *sm) : sm_(sm) {}

    void HandleRequest(::google::protobuf::RpcController *controller,
                       const ::lgraph::LGraphRequest *request, ::lgraph::LGraphResponse *response,
                       ::google::protobuf::Closure *done) override {
        sm_->HandleRequest(controller, request, response, done);
    }

 private:
    lgraph::StateMachine *sm_;
};

class LGraphServer {
#ifndef _WIN32
    class FMALogSink : public logging::LogSink {
     public:
        bool OnLogMessage(int severity, const char *file, int line,
                          const butil::StringPiece &log_content) override {
            LOG_INFO() << "[StateMachine] " << log_content.as_string();
            return true;
        }
    };
#endif
    protected:  // NOLINT
    Signal server_exit_;
    std::shared_ptr<lgraph::GlobalConfig> config_;
    std::unique_ptr<lgraph::StateMachine> state_machine_;
    std::unique_ptr<lgraph::RestServer> rest_server_;
    std::thread heartbeat_detect;
#ifndef _WIN32
    std::unique_ptr<lgraph::RPCService> rpc_service_;
    std::unique_ptr<lgraph::http::HttpService> http_service_;
    std::unique_ptr<brpc::Server> rpc_server_;
    std::unique_ptr<logging::LogSink> blog_sink_;
#endif
 public:  // NOLINT
    explicit LGraphServer(std::shared_ptr<lgraph::GlobalConfig> config);
    ~LGraphServer();

    // just start services
    int Start();

    // wait till service terminated
    int WaitTillKilled();

    // stop all services
    int Stop(bool force_exit = false);
};
}  // namespace lgraph
