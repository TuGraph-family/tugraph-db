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

/*
* written by botu.wzy
 */
#pragma once
#include "bolt/connection.h"
#include "bolt/io_service.h"
#include "server/state_machine.h"

namespace bolt {

class BoltServer final {
 public:
    static BoltServer& Instance() {
        static BoltServer server;
        return server;
    }
    DISABLE_COPY(BoltServer);
    DISABLE_MOVE(BoltServer);
    bool Start(lgraph::StateMachine* sm, int port, int io_thread_num);
    void Stop();
    ~BoltServer() {Stop();}
    lgraph::StateMachine* StateMachine() {
        return sm_;
    }
 private:
    BoltServer() = default;
    lgraph::StateMachine* sm_ = nullptr;
    std::vector<std::thread> threads_;
    bool stopped_ = false;
};
}  // namespace bolt
