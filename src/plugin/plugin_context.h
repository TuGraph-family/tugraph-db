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

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>

#include "fma-common/bounded_queue.h"
#include "fma-common/binary_buffer.h"
#include "fma-common/binary_read_write_helper.h"

#include "core/defs.h"
#include "plugin/plugin_desc.h"

namespace lgraph {
class LightningGraph;

namespace python_plugin {
struct MessageQueueUtils {
    static constexpr size_t MAX_MSG_SIZE() { return 4096; }

    static inline void SendBufToMq(boost::interprocess::message_queue& mq, const void* buf,
                                   size_t s) {
        size_t offset = 0;
        const char* p = (const char*)buf;
        while (offset < s) {
            size_t bytes_to_send = std::min<size_t>(s - offset, MAX_MSG_SIZE());
            mq.send(p + offset, bytes_to_send, 0);
            offset += bytes_to_send;
        }
    }

    static inline void RecvBufFromMq(boost::interprocess::message_queue& mq, void* buf, size_t s,
                                     size_t& rsize, unsigned& priority) {
        size_t offset = 0;
        char* p = (char*)buf;
        char tmp[MAX_MSG_SIZE()];
        while (offset < s) {
            size_t rsize = 0;
            mq.receive(tmp, MAX_MSG_SIZE(), rsize, priority);
            memcpy(p + offset, tmp, rsize);
            offset += rsize;
        }
        rsize = offset;
        if (offset != s) throw std::runtime_error("error receiving message");
    }

    template <typename T>
    static inline bool ReadFromMq(boost::interprocess::message_queue& mq, T& data,
                                  size_t timeout_milliseconds = 0) {
        if (timeout_milliseconds == 0) timeout_milliseconds = (size_t)1000 * 3600 * 24 * 365;

        size_t s;
        size_t rsize = 0;
        unsigned int priority;
        char tmp[MAX_MSG_SIZE()];
        auto now = boost::posix_time::microsec_clock::universal_time();
        auto to = boost::posix_time::ptime(now) +
                  boost::posix_time::microseconds(timeout_milliseconds * 1000);
        bool input_ready = mq.timed_receive(&tmp, MAX_MSG_SIZE(), rsize, priority, to);
        if (!input_ready) return false;
        if (rsize != sizeof(s)) throw std::runtime_error("broken input");
        memcpy(&s, tmp, sizeof(s));
        std::string bytes(s, 0);
        RecvBufFromMq(mq, &bytes[0], s, rsize, priority);
        if (rsize != s) throw std::runtime_error("broken input");
        fma_common::BinaryBuffer buf(bytes.data(), bytes.size());
        size_t r = data.Deserialize(buf);
        if (r != bytes.size()) throw std::runtime_error("broken input: cannot deserialize");
        return true;
    }

    template <typename T>
    static inline void SendToMq(boost::interprocess::message_queue& mq, const T& data) {
        fma_common::BinaryBuffer buf;
        size_t s = data.Serialize(buf);
        mq.send(&s, sizeof(s), 0);
        SendBufToMq(mq, buf.GetBuf(), buf.GetSize());
    }
};

class AutoRemoveMQ {
    bool dummy_;
    std::string name_;
    boost::interprocess::message_queue mq_;

    DISABLE_COPY(AutoRemoveMQ);
    DISABLE_MOVE(AutoRemoveMQ);

 public:
    explicit AutoRemoveMQ(const std::string& name)
        : dummy_(boost::interprocess::message_queue::remove(name.c_str())),
          name_(name),
          mq_(boost::interprocess::create_only, name.c_str(), 10,
              MessageQueueUtils::MAX_MSG_SIZE()) {}

    ~AutoRemoveMQ() { boost::interprocess::message_queue::remove(name_.c_str()); }

    boost::interprocess::message_queue& GetMQ() { return mq_; }
};

struct TaskInput {
    std::string user;
    std::string graph;
    std::string plugin_dir;
    std::string function;
    std::string input;
    bool read_only;

    ~TaskInput();

    template <typename T>
    size_t Serialize(T& os) const {
        return fma_common::BinaryWrite(os, user) + fma_common::BinaryWrite(os, graph) +
               fma_common::BinaryWrite(os, plugin_dir) + fma_common::BinaryWrite(os, function) +
               fma_common::BinaryWrite(os, input) + fma_common::BinaryWrite(os, read_only);
    }

    template <typename T>
    size_t Deserialize(T& is) {
        return fma_common::BinaryRead(is, user) + fma_common::BinaryRead(is, graph) +
               fma_common::BinaryRead(is, plugin_dir) + fma_common::BinaryRead(is, function) +
               fma_common::BinaryRead(is, input) + fma_common::BinaryRead(is, read_only);
    }

    bool ReadFromMessageQueue(boost::interprocess::message_queue& mq, size_t timeout_milliseconds) {
        return MessageQueueUtils::ReadFromMq(mq, *this, timeout_milliseconds);
    }

    void WriteToMessageQueue(boost::interprocess::message_queue& mq) {
        MessageQueueUtils::SendToMq(mq, *this);
    }
};

struct TaskOutput {
    enum ErrorCode { SUCCESS = 1, INPUT_ERR = 2, INTERNAL_ERR = 3, SUCCESS_WITH_SIGNATURE = 4 };

    ErrorCode error_code;
    std::string output;

    ~TaskOutput();

    template <typename T>
    size_t Serialize(T& os) const {
        return fma_common::BinaryWrite(os, error_code) + fma_common::BinaryWrite(os, output);
    }

    template <typename T>
    size_t Deserialize(T& is) {
        return fma_common::BinaryRead(is, error_code) + fma_common::BinaryRead(is, output);
    }

    bool ReadFromMessageQueue(boost::interprocess::message_queue& mq, size_t timeout_milliseconds) {
        return MessageQueueUtils::ReadFromMq(mq, *this, timeout_milliseconds);
    }

    void WriteToMessageQueue(boost::interprocess::message_queue& mq) {
        MessageQueueUtils::SendToMq(mq, *this);
    }
};
}  // namespace python_plugin
}  // namespace lgraph
