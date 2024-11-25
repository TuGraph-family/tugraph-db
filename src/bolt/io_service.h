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
#include <thread>
#include <iostream>
#include <unordered_map>
#include "boost/asio.hpp"
#include "boost/lexical_cast.hpp"
#include "tools/lgraph_log.h"
#include "fma-common/string_formatter.h"

namespace bolt {

using boost::asio::ip::tcp;

class IOServicePool : private boost::asio::noncopyable {
 public:
    ~IOServicePool() {Stop();}

    explicit IOServicePool(std::size_t pool_size) : next_io_service_(0) {
        if (pool_size == 0) throw std::runtime_error("io_service_pool size is 0");
        for (std::size_t i = 0; i < pool_size; ++i) {
            io_service_ptr io_service(new boost::asio::io_service(1));
            work_ptr work(new boost::asio::io_service::work(*io_service));
            io_services_.push_back(std::move(io_service));
            works_.push_back(std::move(work));
        }
    }
    void Run() {
        for (std::size_t i = 0; i < io_services_.size(); ++i) {
            boost::asio::io_service &service = *io_services_[i];
            threads_.emplace_back([i, &service]() {
                std::string name = "io-worker-" + std::to_string(i);
                pthread_setname_np(pthread_self(), name.c_str());
              service.run(); });
        }
    }
    void Stop() {
        if (stopped_) {
            return;
        }
        for (std::size_t i = 0; i < io_services_.size(); ++i) {
            io_services_[i]->stop();
        }
        for (auto& t : threads_) {
            t.join();
        }
        stopped_ = true;
    }
    boost::asio::io_service& GetIOService() {
        boost::asio::io_service& io_service = *io_services_[next_io_service_];
        ++next_io_service_;
        if (next_io_service_ == io_services_.size()) next_io_service_ = 0;
        return io_service;
    }

 private:
  typedef std::unique_ptr<boost::asio::io_service> io_service_ptr;
  typedef std::unique_ptr<boost::asio::io_service::work> work_ptr;

  std::vector<io_service_ptr> io_services_;
  std::vector<work_ptr> works_;
  std::size_t next_io_service_;
  std::vector<std::thread> threads_;
  bool stopped_ = false;
};


template<typename T, typename F>
class IOService : private boost::asio::noncopyable {
 public:
    ~IOService() {
        io_service_pool_.Stop();
    }
    IOService(boost::asio::io_service &service,
              int port, int thread_num, F handler)
        : handler_(handler),
          acceptor_(service, tcp::endpoint(tcp::v4(), port),
                    /*reuse_addr*/true),
          io_service_pool_(thread_num), interval_(5), timer_(service) {
        io_service_pool_.Run();
        invoke_async_accept();
        clean_closed_conn();
    }

 private:
    void invoke_async_accept() {
        conn_.reset(new T(io_service_pool_.GetIOService(), handler_));
        acceptor_.async_accept(conn_->socket(), [this](boost::system::error_code ec) {
            if (ec) {
                LOG_WARN() << FMA_FMT("async accept error: {}", ec.message());
            } else {
                LOG_DEBUG() << FMA_FMT("accept new bolt connection {}",
                                     boost::lexical_cast<std::string>(
                                         conn_->socket().remote_endpoint()));
                socket_set_options(conn_->socket());
                conn_->conn_id() = next_conn_id_;
                connections_.emplace(next_conn_id_, conn_);
                next_conn_id_++;
                conn_->Start();
            }

            invoke_async_accept();
        });
    }
    void clean_closed_conn() {
        for (auto it = connections_.cbegin(); it != connections_.cend();) {
            if (it->second->has_closed()) {
                LOG_DEBUG() << FMA_FMT("erase connection[id:{},use_count:{}] from pool",
                                       it->second->conn_id(), it->second.use_count());
                it = connections_.erase(it);
            } else {
                ++it;
            }
        }
        timer_.expires_from_now(interval_);
        timer_.async_wait(std::bind(&IOService::clean_closed_conn, this));
    }
    std::shared_ptr<T> conn_;
    F handler_;
    std::unordered_map<int64_t, std::shared_ptr<T>> connections_;
    tcp::acceptor acceptor_;
    IOServicePool io_service_pool_;
    int next_conn_id_ = 0;
    boost::posix_time::seconds interval_;
    boost::asio::deadline_timer timer_;
};

}  // namespace bolt
