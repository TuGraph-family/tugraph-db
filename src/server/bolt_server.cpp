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
#include "server/bolt_server.h"
#include "bolt_raft/io_service.h"
#include "bolt_raft/raft_driver.h"

namespace bolt {
static boost::asio::io_service listener(BOOST_ASIO_CONCURRENCY_HINT_UNSAFE);
extern std::function<void(bolt::BoltConnection &conn, bolt::BoltMsg msg,
                          std::vector<std::any> fields, std::vector<uint8_t> raw_data)> BoltHandler;
bool BoltServer::Start(lgraph::StateMachine* sm, int port, int io_thread_num) {
    sm_ = sm;
    bolt::MarkersInit();
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();
    threads_.emplace_back([port, io_thread_num, &promise](){
        bool promise_done = false;
        try {
            bolt::IOService<bolt::BoltConnection, decltype(bolt::BoltHandler)> bolt_service(
                listener, port, io_thread_num, bolt::BoltHandler);
            boost::asio::io_service::work holder(listener);
            LOG_INFO() << "bolt server run";
            promise.set_value(true);
            promise_done = true;
            pthread_setname_np(pthread_self(), "bolt_listener");
            listener.run();
        } catch (const std::exception& e) {
            LOG_WARN() << "bolt server expection: " << e.what();
            if (!promise_done) {
                promise.set_value(false);
            }
        }
    });
    return future.get();
}

void BoltServer::Stop() {
    if (stopped_) {
        return;
    }
    listener.stop();
    for (auto& t : threads_) {
        t.join();
    }
    threads_.clear();
    stopped_ = true;
    LOG_INFO() << "bolt server stopped.";
}
}  // namespace bolt
