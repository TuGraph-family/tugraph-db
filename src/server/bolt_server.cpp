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

namespace bolt {
boost::asio::io_service workers;
static boost::asio::io_service listener(BOOST_ASIO_CONCURRENCY_HINT_UNSAFE);
extern std::function<void(bolt::BoltConnection &conn, bolt::BoltMsg msg,
                          std::vector<std::any> fields)> BoltHandler;
bool BoltServer::Start(lgraph::StateMachine* sm, int port, int workerNum) {
    sm_ = sm;
    port_ = port;
    workerNum_ = workerNum;
    bolt::MarkersInit();
    for (int i = 0; i < workerNum_; i++) {
        threads_.emplace_back([](){
            boost::asio::io_service::work holder(workers);
            workers.run();
        });
    }
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();
    threads_.emplace_back([this, &promise](){
        bool promise_done = false;
        try {
            bolt::IOService<bolt::BoltConnection, decltype(bolt::BoltHandler)> bolt_service(
                listener, port_, 1, bolt::BoltHandler);
            boost::asio::io_service::work holder(listener);
            promise.set_value(true);
            promise_done = true;
            FMA_LOG() << "bolt server run";
            listener.run();
        } catch (const std::exception& e) {
            FMA_WARN() << "bolt server expection: " << e.what();
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
    workers.stop();
    for (auto& t : threads_) {
        t.join();
    }
    stopped_ = true;
    FMA_LOG() << "bolt server stopped.";
}

}  // namespace bolt
