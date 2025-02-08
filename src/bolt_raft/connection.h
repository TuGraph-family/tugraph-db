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

// written by botu.wzy

#pragma once
#include <utility>
#include <deque>
#include <boost/asio.hpp>
#include <boost/endian/conversion.hpp>
#include "tools/lgraph_log.h"
#include "fma-common/string_formatter.h"
#include "etcd-raft-cpp/raftpb/raft.pb.h"

namespace bolt_raft {

class Connection : private boost::asio::noncopyable {
 public:
    virtual ~Connection() = default;

    explicit Connection(boost::asio::io_service& io_service)
        : io_service_(io_service), socket_(io_service_), has_closed_(false) {}
    boost::asio::ip::tcp::socket& socket() { return socket_; }
    virtual void Close() {
        LOG_DEBUG() << FMA_FMT("close conn[id:{}]", conn_id_);
        socket_.close();
        has_closed_ = true;
    }
    virtual bool has_closed() { return has_closed_; }
    int64_t& conn_id() { return conn_id_; }
    boost::asio::io_service& io_service() { return io_service_; }
    virtual void Start() = 0;

 private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket socket_;
    int64_t conn_id_ = 0;
    std::atomic<bool> has_closed_;
};

class RaftConnection : public Connection, public std::enable_shared_from_this<RaftConnection> {
 public:
    RaftConnection(boost::asio::io_service& io_service,
                   std::function<void(raftpb::Message)> handler)
        : Connection(io_service), handler_(std::move(handler)) {}
    void Start() override;

 private:
    void read_msg_size();
    void read_msg_size_done(const boost::system::error_code& ec);
    void read_magic();
    void read_magic_done(const boost::system::error_code& ec);
    void read_msg_body();
    void read_msg_body_done(const boost::system::error_code& ec);

    uint32_t msg_size_ = 0;
    std::vector<char> msg_body_;
    std::function<void(raftpb::Message)> handler_;
    const uint8_t magic_code_[4] = {0x17, 0xB0, 0x60, 0x60};
    uint8_t buffer4_[4] = {0};
};

inline void RaftConnection::Start() { read_magic(); }

inline void RaftConnection::read_msg_size() {
    async_read(socket(), boost::asio::buffer(&msg_size_, sizeof(msg_size_)),
               [this, self = shared_from_this()](const boost::system::error_code& ec, size_t) {
                   read_msg_size_done(ec);
               });
}

inline void RaftConnection::read_magic() {
    async_read(socket(), boost::asio::buffer(buffer4_),
               [this, self = shared_from_this()](const boost::system::error_code& ec, size_t) {
                   read_magic_done(ec);
               });
}

inline void RaftConnection::read_magic_done(const boost::system::error_code& ec) {
    if (ec) {
        LOG_WARN() << FMA_FMT("raft connection read_magic_done error {}", ec.message());
        Close();
        return;
    }
    if (memcmp(buffer4_, magic_code_, sizeof(magic_code_)) != 0) {
        LOG_WARN() << "receive wrong magic code";
        Close();
        return;
    }
    read_msg_size();
}

inline void RaftConnection::read_msg_size_done(const boost::system::error_code& ec) {
    if (ec) {
        LOG_WARN() << FMA_FMT("raft connection read_msg_size_done error {}", ec.message());
        Close();
        return;
    }

    boost::endian::big_to_native_inplace(msg_size_);
    if (msg_size_ > 1024 * 1024 * 1024) {
        LOG_WARN() << FMA_FMT("receive raft message which is too big, size : {}", msg_size_);
        Close();
        return;
    }
    if (msg_size_ == 0) {
        LOG_WARN() << FMA_FMT("receive raft message with error size, size : {}", msg_size_);
        Close();
        return;
    }
    msg_body_.resize(msg_size_);
    read_msg_body();
}

inline void RaftConnection::read_msg_body() {
    async_read(socket(), boost::asio::buffer(msg_body_),
               [this, self = shared_from_this()](const boost::system::error_code& ec, size_t) {
                   read_msg_body_done(ec);
               });
}

inline void RaftConnection::read_msg_body_done(const boost::system::error_code& ec) {
    if (ec) {
        LOG_WARN() << FMA_FMT("raft connection read_msg_body_done error {}", ec.message());
        Close();
        return;
    }
    raftpb::Message msg;
    auto ret = msg.ParseFromArray(msg_body_.data(), (int)msg_body_.size());
    if (!ret) {
        LOG_WARN() << FMA_FMT("failed to parse raft msg, close connection");
        Close();
        return;
    }
    handler_(std::move(msg));
    read_msg_size();
}

}  // namespace bolt_raft
