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
#include "bolt_server.h"
#include "common/logger.h"

namespace bolt {
void BoltServer::Start(uint32_t port, uint32_t io_thread_num,
                       const std::function<void(bolt::BoltConnection &conn, bolt::BoltMsg msg, std::vector<std::any> fields)>& handler) {
    bolt::MarkersInit();
    try {
        bolt::IOService<bolt::BoltConnection, decltype(handler)> bolt_service(
            listener_, port, io_thread_num, handler);
        boost::asio::io_service::work holder(listener_);
        LOG_INFO("Bolt server run");
        listener_.run();
    } catch (const std::exception& e) {
        LOG_ERROR("bolt server expection: {}", e.what());
    }
    LOG_INFO("Bolt server exit");
}

void BoltServer::Stop() {
    if (stopped_) {
        return;
    }
    listener_.stop();
    stopped_ = true;
}

}  // namespace bolt
