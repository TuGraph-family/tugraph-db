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
#include "connection.h"
#include "io_service.h"
#include "common/type_traits.h"

namespace bolt {
class BoltServer final {
public:
    static BoltServer& Instance() {
        static BoltServer server;
        return server;
    }
    DISABLE_COPY(BoltServer);
    DISABLE_MOVE(BoltServer);
    void Start(uint32_t port, uint32_t io_thread_num, const std::function<void(bolt::BoltConnection &conn, bolt::BoltMsg msg, std::vector<std::any> fields)>& handler);
    void Stop();
    ~BoltServer() {Stop();}
private:
    BoltServer() = default;
    bool stopped_ = false;
    boost::asio::io_service listener_{BOOST_ASIO_CONCURRENCY_HINT_UNSAFE};
};
}  // namespace bolt
