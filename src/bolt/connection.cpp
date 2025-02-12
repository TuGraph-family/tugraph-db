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
namespace beast = boost::beast;

void socket_set_options(tcp::socket& socket) {
    socket.set_option(ip::tcp::no_delay(true));
    socket.set_option(socket_base::keep_alive(true));
    socket.set_option(ip::tcp::socket::reuse_address(true));
}

void BoltConnection::WebSocketReadSomeDone(const boost::system::error_code &ec,
                                           std::size_t bytes_transferred) {
    if (ec) {
        LOG_WARN() << FMA_FMT("WebSocketReadSomeDone error: {}", ec.message());
        Close();
        return;
    }
    ws_total_read_ += bytes_transferred;
    if (ws_total_read_ < ws_buffer_size_) {
        WebSocketReadSome();
    } else {
        ws_cb_(ec);
    }
}

void BoltConnection::WebSocketReadSome() {
    ws().async_read_some(
        buffer(ws_buffer_ + ws_total_read_, ws_buffer_size_ - ws_total_read_),
        std::bind(&BoltConnection::WebSocketReadSomeDone, shared_from_this(),
                  std::placeholders::_1, std::placeholders::_2));
}

void BoltConnection::WebSocketAsyncRead(
    const mutable_buffer& buffer,
    const std::function<void(const boost::system::error_code &ec)>& cb) {
    ws_buffer_ = (char*)buffer.data();
    ws_buffer_size_ = buffer.size();
    ws_total_read_ = 0;
    ws_cb_ = cb;
    WebSocketReadSome();
}


void BoltConnection::ReadMagicDone(const boost::system::error_code &ec) {
    if (ec) {
        LOG_WARN() << FMA_FMT("ReadMagicDone error: {}", ec.message());
        Close();
        return;
    }
    if (*(uint32_t*)buffer4_ == *(uint32_t*)bolt_magic_) {
        protocol_ = Protocol::Socket;
        // Read bolt versions
        async_read(socket(), buffer(buffer16_),
                   std::bind(&BoltConnection::ReadVersionNegotiationDone,
                             shared_from_this(), std::placeholders::_1));
    } else if (*(uint32_t*)buffer4_ == *(uint32_t*)ws_magic_) {
        protocol_ = Protocol::WebSocket;
        ResetToWebSocket();
        // Accept the websocket handshake
        ws().async_accept(buffer(buffer4_),
                          std::bind(&BoltConnection::WebSocketAcceptDone,
                                    shared_from_this(), std::placeholders::_1));
    } else {
        LOG_WARN() << "Unknown protocol magic";
        Close();
    }
}

void BoltConnection::Start() {
    async_read(socket(), buffer(buffer4_),
               std::bind(&BoltConnection::ReadMagicDone, shared_from_this(),
                         std::placeholders::_1));
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
    auto cb = [this](const boost::system::error_code& ec, std::size_t) {
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
    };
    if (protocol_ == Protocol::Socket) {
        async_write(socket(), send_buffers_, std::move(cb));
    } else {
        ws().async_write(send_buffers_, std::move(cb));
    }
}
// async respond
// used in io thread
void BoltConnection::Respond(std::string str) {
    if (has_closed()) {
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

void BoltConnection::ReadBoltIdentificationDone(const boost::system::error_code& ec) {
    if (ec) {
        LOG_WARN() << FMA_FMT("ReadBoltIdentificationDone error: {}", ec.message());
        Close();
        return;
    }
    if (*(uint32_t*)buffer4_ == *(uint32_t*)bolt_magic_) {
        // read bolt versions
        WebSocketAsyncRead(
            buffer(buffer16_),
            std::bind(&BoltConnection::ReadVersionNegotiationDone,
                      shared_from_this(), std::placeholders::_1));
    } else {
        LOG_WARN() << "Bolt connection identification is wrong";
        Close();
    }
}


void BoltConnection::WebSocketAcceptDone(const boost::system::error_code &ec) {
    if (ec) {
        LOG_WARN() << FMA_FMT("WebSocketAcceptDone error: {}", ec.message());
        Close();
        return;
    }
    WebSocketAsyncRead(buffer(buffer4_),
               std::bind(&BoltConnection::ReadBoltIdentificationDone,
                         shared_from_this(), std::placeholders::_1));
}

void BoltConnection::ReadVersionNegotiationDone(const boost::system::error_code &ec) {
    if (ec) {
        LOG_WARN() << FMA_FMT("ReadVersionNegotiationDone error: {}", ec.message());
        Close();
        return;
    }
    bool match = false;
    for (int i = 0; i < 4; i++) {
        if (buffer16_[i*4 + 3] == 4) {
            *(uint32_t*)buffer4_ = *(uint32_t*)(buffer16_ + 4*i);
            match = true;
            break;
        }
    }
    if (LoggerManager::GetInstance().GetLevel() <= severity_level::DEBUG) {
        for (int i = 0; i < 4; i++) {
            LOG_DEBUG() << "protocol version " + std::to_string(i)
                        << ": major:" << (int)buffer16_[i*4+3] << ", minor:"
                        << (int)buffer16_[i*4+2];
        }
    }
    if (!match) {
        LOG_WARN() << "No matching bolt version found";
        Close();
        return;
    }
    // write accepted version
    if (protocol_ == Protocol::Socket) {
        async_write(socket(), buffer(buffer4_),  // NOLINT
                    std::bind(&BoltConnection::WriteResponseDone,
                              shared_from_this(), std::placeholders::_1));
    } else {
        ws().async_write(buffer(buffer4_),
                         std::bind(&BoltConnection::WriteResponseDone,
                                   shared_from_this(), std::placeholders::_1));
    }
}

void BoltConnection::WriteResponseDone(const boost::system::error_code& ec) {
    if (ec) {
        LOG_WARN() << FMA_FMT("WriteResponseDone error: {}", ec.message());
        Close();
        return;
    }
    // read chunk size
    if (protocol_ == Protocol::Socket) {
        async_read(socket(), buffer(&chunk_size_, sizeof(chunk_size_)),  // NOLINT
                   std::bind(&BoltConnection::ReadChunkSizeDone, shared_from_this(),
                             std::placeholders::_1));
    } else {
        WebSocketAsyncRead(buffer(&chunk_size_, sizeof(chunk_size_)),
                              std::bind(&BoltConnection::ReadChunkSizeDone, shared_from_this(),
                                        std::placeholders::_1));
    }
}

void BoltConnection::ReadChunkSizeDone(const boost::system::error_code& ec) {
    if (ec) {
        // LOG_WARN() << FMA_FMT("ReadChunkSizeDone error: {}", ec.message());
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
        try {
            for (uint32_t i = 0; i < len; i++) {
                unpacker_.Next();
                fields.push_back(bolt::ServerHydrator(unpacker_));
            }
            LOG_DEBUG() << FMA_FMT(
                "msg: {}, fields: {}", ToString(tag), Print(fields).substr(0, 1024));
            handle_(*this, tag, std::move(fields), std::move(chunk_));
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
    if (protocol_ == Protocol::Socket) {
        async_read(socket(), buffer(chunk_.data() + old_size, chunk_size_),  // NOLINT
                   std::bind(&BoltConnection::ReadChunkDone, shared_from_this(),
                             std::placeholders::_1));
    } else {
        WebSocketAsyncRead(
            buffer(chunk_.data() + old_size, chunk_size_),
            std::bind(&BoltConnection::ReadChunkDone, shared_from_this(),
                      std::placeholders::_1));
    }
}

void BoltConnection::ReadChunkDone(const boost::system::error_code& ec) {
    if (ec) {
        LOG_WARN() << FMA_FMT("ReadChunkDone error: {}", ec.message());
        Close();
        return;
    }
    if (protocol_ == Protocol::Socket) {
        async_read(socket(), buffer(&chunk_size_, sizeof(chunk_size_)),
                   std::bind(&BoltConnection::ReadChunkSizeDone, shared_from_this(),
                             std::placeholders::_1));
    } else {
        WebSocketAsyncRead(
            buffer(&chunk_size_, sizeof(chunk_size_)),
            std::bind(&BoltConnection::ReadChunkSizeDone, shared_from_this(),
                      std::placeholders::_1));
    }
}

}  // namespace bolt
