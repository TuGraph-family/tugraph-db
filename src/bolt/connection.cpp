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
#include <boost/endian/conversion.hpp>
#include "fma-common/string_formatter.h"
#include "fma-common/utils.h"
#include "tools/json.hpp"
#include "bolt/connection.h"
#include "bolt/messages.h"
#include "bolt/to_string.h"

namespace bolt {
using namespace boost::asio;
using namespace lgraph_log;

void socket_set_options(tcp::socket& socket) {
    socket.set_option(ip::tcp::no_delay(true));
    socket.set_option(socket_base::keep_alive(true));
    socket.set_option(ip::tcp::socket::reuse_address(true));
}

void BoltConnection::Start() {
    // read handshake
    async_read(socket(), buffer(handshake_buffer_),
               std::bind(&BoltConnection::ReadHandshakeDone,
                         shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void BoltConnection::Close() {
    Connection::Close();
}

void BoltConnection::DoSend() {
    for (size_t i = 0; i < msg_queue_.size(); i++) {
        send_buffers_.emplace_back(boost::asio::buffer(msg_queue_[i]));
        if (send_buffers_.size() >= 5) {
            break;
        }
    }
    async_write(socket(), send_buffers_,
     [this](const boost::system::error_code& ec, std::size_t) {
         if (ec) {
             LOG_WARN() << FMA_FMT("async write error: {}, clear {} pending message",
                                  ec.message(), msg_queue_.size());
             msg_queue_.clear();
             msg_queue_size_ = 0;
             Close();
             return;
         }
         assert(msg_queue_.size() >= send_buffers_.size());
         msg_queue_.erase(msg_queue_.begin(), msg_queue_.begin() + send_buffers_.size());
         msg_queue_size_ = msg_queue_.size();
         send_buffers_.clear();
         if (!msg_queue_.empty()) {
             DoSend();
         }
     });
}
// async respond
// used in io thread
void BoltConnection::Respond(std::string str) {
    if (!socket().is_open()) {
        LOG_WARN() << "connection is not available, drop this message";
        return;
    }
    bool need_invoke = msg_queue_.empty();
    msg_queue_.push_back(std::move(str));
    msg_queue_size_ = msg_queue_.size();
    if (need_invoke) {
        DoSend();
    }
}

// async respond
// used in non-io thread, thread safe
void BoltConnection::PostResponse(std::string str) {
    while (!has_closed() && msg_queue_size_ > 1024) {
        fma_common::SleepUs(1000);  // sleep 1 ms
    }
    if (has_closed()) {
        LOG_WARN() << "connection is closed, drop this message";
        return;
    }
    io_service().post([self = shared_from_this(), msg = std::move(str)]() mutable {
        self->Respond(std::move(msg));
    });
}

void BoltConnection::ReadHandshakeDone(const boost::system::error_code& ec, const size_t) {
    if (ec) {
        LOG_WARN() << FMA_FMT("ReadHandshakeDone error: {}", ec.message());
        Close();
        return;
    }
    if (*(uint32_t*)handshake_buffer_ != *(uint32_t*)bolt_identification_) {
        LOG_WARN() << "Bolt connection identification is wrong";
        Close();
        return;
    }
    bool match = false;
    for (int i = 1; i < 5; i++) {
        if (handshake_buffer_[i*4 + 3] == 4) {
            *(uint32_t*)version_buffer_ = *(uint32_t*)(handshake_buffer_ + 4*i);
            match = true;
            break;
        }
    }
    if (!match) {
        LOG_WARN() << "No matching bolt version found";
    }
    if (LoggerManager::GetInstance().GetLevel() >= severity_level::DEBUG) {
        for (int i = 1; i < 5; i++) {
            LOG_DEBUG() << "protocol version " + std::to_string(i)
                      << ": major:" << (int)handshake_buffer_[i*4+3] << ", minor:"
                      << (int)handshake_buffer_[i*4+2];
        }
    }
    // write accepted version
    async_write(socket(),buffer(version_buffer_), // NOLINT
                std::bind(&BoltConnection::WriteResponseDone, shared_from_this(),
                          std::placeholders::_1, std::placeholders::_2));
}

void BoltConnection::WriteResponseDone(const boost::system::error_code& ec, const size_t) {
    if (ec) {
        LOG_WARN() << FMA_FMT("WriteResponseDone error: {}", ec.message());
        Close();
        return;
    }
    // read chunk size
    async_read(socket(),buffer(&chunk_size_, sizeof(chunk_size_)), //NOLINT
               std::bind(&BoltConnection::ReadChunkSizeDone, shared_from_this(),
                         std::placeholders::_1, std::placeholders::_2));
}

void BoltConnection::ReadChunkSizeDone(const boost::system::error_code& ec, const size_t) {
    if (ec) {
        LOG_WARN() << FMA_FMT("ReadChunkSizeDone error: {}", ec.message());
        Close();
        return;
    }
    boost::endian::big_to_native_inplace(chunk_size_);
    if (chunk_size_ == 0 && !chunk_.empty()) {
        unpacker_.Reset(std::string_view((const char*)chunk_.data(), chunk_.size()));
        unpacker_.Next();
        auto len = unpacker_.Len();
        auto tag = static_cast<BoltMsg>(unpacker_.StructTag());
        std::vector<std::any> fields;
        for (uint32_t i = 0; i < len; i++) {
            unpacker_.Next();
            fields.push_back(bolt::ServerHydrator(unpacker_));
        }
        LOG_DEBUG() << FMA_FMT("msg: {}, fields: {}",
                               ToString(tag), Print(fields));
        try {
            handle_(*this, tag, std::move(fields));
        } catch (const std::exception& e) {
            LOG_ERROR() << "Exception in bolt connection: " << e.what();
            Close();
            return;
        }
        if (has_closed()) {
            return;
        }
        chunk_.resize(0);
    }
    auto old_size = chunk_.size();
    chunk_.resize(old_size + chunk_size_);
    async_read(socket(),buffer(chunk_.data() + old_size, chunk_size_), // NOLINT
               std::bind(&BoltConnection::ReadChunkDone, shared_from_this(),
                         std::placeholders::_1, std::placeholders::_2));
}

void BoltConnection::ReadChunkDone(const boost::system::error_code& ec, const size_t) {
    if (ec) {
        LOG_WARN() << FMA_FMT("ReadChunkDone error: {}", ec.message());
        Close();
        return;
    }
    async_read(socket(),  buffer(&chunk_size_, sizeof(chunk_size_)),
               std::bind(&BoltConnection::ReadChunkSizeDone, shared_from_this(),
                         std::placeholders::_1, std::placeholders::_2));
}

}  // namespace bolt
