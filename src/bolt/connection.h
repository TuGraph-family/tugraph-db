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
#include <pthread.h>
#include <memory>
#include <regex>
#include <functional>
#include <deque>
#include <mutex>
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <utility>
#include "boost/asio.hpp"
#include "bolt/hydrator.h"
#include "bolt/pack_stream.h"
#include "fma-common/logger.h"

namespace bolt {
using boost::asio::ip::tcp;

void socket_set_options(tcp::socket& socket);

class Connection : private boost::asio::noncopyable {
 public:
    explicit Connection(boost::asio::io_service& io_service)
        : io_service_(io_service), socket_(io_service_), has_closed_(false) {
    }
    tcp::socket& socket() { return socket_; }
    virtual void Close() {
        // std::cout << "close conn[id:" << conn_id_ << "]" << std::endl;
        socket_.close();
        has_closed_ = true;
    }
    virtual bool has_closed() {return has_closed_;}
    int64_t& conn_id() { return conn_id_;}
    boost::asio::io_service& io_service() {return io_service_;}
    virtual void Start() = 0;
    virtual void PostResponse(std::string res) {}
 private:
    boost::asio::io_service& io_service_;
    tcp::socket socket_;
    int64_t conn_id_ = 0;
    std::atomic<bool> has_closed_;
};

class BoltConnection
        : public Connection,
          public std::enable_shared_from_this<BoltConnection> {
 public:
    BoltConnection(boost::asio::io_service& io_service,
                   std::function<void(BoltConnection& conn, BoltMsg msg,
                                     const std::vector<std::any>& fields)> handle)
            : Connection(io_service),
          handle_(std::move(handle)) {}
    void Start() override;
    void Close() override;
    void PostResponse(std::string res) override;
    void Respond(std::string str);
    void SetContext(std::shared_ptr<void> ctx) {
        context_ = std::move(ctx);
    }
    void* GetContext() {
        return context_.get();
    }

 private:
    void ReadHandshakeDone(const boost::system::error_code &ec, size_t size);
    void ReadChunkSizeDone(const boost::system::error_code &ec, size_t size);
    void ReadChunkDone(const boost::system::error_code &ec, size_t size);
    void WriteResponseDone(const boost::system::error_code &ec, size_t size);
    void DoSend();

    std::function<void(BoltConnection& conn, BoltMsg msg,
                       std::vector<std::any> fields)> handle_;
    uint8_t handshake_buffer_[20] = {0};
    uint8_t version_buffer_[4] = {0, 0, 4, 4};
    uint16_t chunk_size_ = 0;
    std::vector<uint8_t> chunk_;
    Unpacker unpacker_;
    std::deque<std::string> msg_queue_;
    std::vector<boost::asio::const_buffer> send_buffers_;
    // only shared_ptr can store void pointer
    std::shared_ptr<void> context_;
};

}  // namespace bolt
