/**
* Copyright 2024 AntGroup CO., Ltd.
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
#include <boost/asio.hpp>
#include "lgraph/lgraph_txn.h"
#include "bolt/blocking_queue.h"

namespace bolt {

enum class SessionState {
    DISCONNECTED = 0,
    DEFUNCT,
    CONNECTED,
    READY,
    STREAMING,
    FAILED
};

struct BoltMsgDetail {
    BoltMsg type;
    int64_t n = 0;
};

struct BoltSession {
    std::optional<BoltMsgDetail> current_msg;
    PackStream ps;
    std::string user;
    SessionState state;
    BlockingQueue<BoltMsgDetail> msgs;
};

}  // namespace bolt
