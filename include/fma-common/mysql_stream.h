//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#if FMA_HAS_LIBMYSQL

/*!
 * \file    fma-common\file_stream.h.
 *
 * \breif   Declares the unbuffered input mysql stream
 *
 * \author  Ke Huang
 *
 * \last_modified   2018/10/23
 */
#pragma once

#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "fma-common/buffered_file_stream.h"
#include "fma-common/file_stream.h"
#include "fma-common/stream_buffer.h"
#include "fma-common/logger.h"

namespace fma_common {
/*!
 * \class  UnbufferedInputMysqlStream
 *
 * \brief  An UnbufferedInputMysqlStream describes a read-only mysql connection.
 */
class UnbufferedInputMysqlStream : public InputFileStream {
    std::unique_ptr<sql::Connection> connection_;
    std::unique_ptr<sql::Statement> statement_;
    std::string host_name_;
    std::string query_;
    std::string table_name_;
    std::string one_page_;
    size_t page_offset_ = 0;
    size_t offset_ = 0;
    std::function<void(std::string&)> filter_;
    static const size_t PAGE_SIZE = 10000000;

 public:
    DISABLE_COPY(UnbufferedInputMysqlStream);
    UnbufferedInputMysqlStream() {}

    /*!
     * \fn  UnbufferedInputMysqlStream::UnbufferedInputMysqlStream(const std::string& path, size_t
     buffer_size = 0)
     *
     * \breif  Constructor
     *
     * \param  path         include host name, user, password, query
     *                      e.g.
     "tcp://127.0.0.1:3306/{db_name}?user={user_name}&password={password}&query=select name,id from
     {table_name}"

     * \param  buffer_size  UnbufferedInputMysqlStream does not have buffer, buffer_size = 0
     */
    UnbufferedInputMysqlStream(
        const std::string& path, size_t buffer_size = 0,
        std::function<void(std::string&)> filter = [](std::string&) {}) {
        Open(path, buffer_size, filter);
    }

    UnbufferedInputMysqlStream(UnbufferedInputMysqlStream&& rhs) {
        connection_ = std::move(rhs.connection_);
        statement_ = std::move(rhs.statement_);
        host_name_ = std::move(rhs.host_name_);
        query_ = std::move(rhs.query_);
        table_name_ = std::move(rhs.table_name_);
        one_page_ = std::move(rhs.one_page_);
        page_offset_ = rhs.page_offset_;
        offset_ = rhs.offset_;
    }

    virtual ~UnbufferedInputMysqlStream() { Close(); }

    /*!
     * \fn  virtual void UnbufferedInputMysqlStream::Open(const std::string& path, size_t
     * buffer_size = 0)
     *
     * \brief  Open an input Mysql connection
     *
     * \param  path         include host name, user, password, query
     *                      e.g.
     * "mysql://127.0.0.1:3306/{db_name}?user={user_name}&password={password}&query=select name,id
     * from {table_name}"
     *
     * \param  buffer_size  UnbufferedInputMysqlStream does not have buffer, buffer_size = 0
     */
    void Open(const std::string& path, size_t buffer_size = 0) override {
        std::string user;
        std::string password;
        std::vector<std::string> values = fma_common::Split(path, "?");
        std::string host_name_start_with_tcp = "tcp://";
        host_name_ = host_name_start_with_tcp +
                     values[0].substr(fma_common::FilePath::MYSQL_SCHEME_LEN,
                                      values[0].size() - fma_common::FilePath::MYSQL_SCHEME_LEN);

        for (auto& s : fma_common::Split(values[1], "&")) {
            std::vector<std::string> kv = fma_common::Split(s, "=");
            if (kv[0] == "user") {
                user = kv[1];
            } else if (kv[0] == "password") {
                password = kv[1];
            } else if (kv[0] == "query") {
                query_ = kv[1];
                std::string separator = " from ";
                auto found = query_.find(separator);
                if (found == std::string::npos) {
                    FMA_WARN() << "Failed to parse query: " << query_;
                }
                table_name_ = query_.substr(found + separator.size(), query_.size());
            }
        }
        connection_ = std::unique_ptr<sql::Connection>(
            get_driver_instance()->connect(host_name_, user, password));
    }

    virtual void Open(const std::string& path, size_t buffer_size,
                      std::function<void(std::string&)> filter) {
        filter_ = std::move(filter);
        Open(path, buffer_size);
    }

    void Close() override {}

    /*!
     * \fn  virtual size_t UnbufferedInputMysqlStream::Read(void* buf, size_t size)
     *
     * \brief  Read a block of data into buffer
     *
     * \param  buf   Buffer used to reveive the data
     * \param  size  Number of bytes to read
     *
     * \return  Number of bytes actually read
     */
    size_t Read(void* buf, size_t size) override {
        size_t _read_size = 0;
        size_t _copy_size = 0;
        std::stringstream _ss;
        // Read pages from mysql until the buf is full.
        while (_read_size < size) {
            _ss.str(std::string());
            if (offset_ == 0) {
                _ReadOnePage(_ss);
                one_page_ = _ss.str();
            }
            if (one_page_.size() - offset_ > size - _read_size) {
                _copy_size = size - _read_size;
                memcpy((char*)buf + _read_size, one_page_.c_str() + offset_, _copy_size);

                offset_ += (_copy_size);
            } else {
                _copy_size = one_page_.size() - offset_;
                memcpy((char*)buf + _read_size, one_page_.c_str() + offset_, _copy_size);
                page_offset_ += PAGE_SIZE;
                offset_ = 0;
            }
            _read_size += _copy_size;
            if (_copy_size == 0) {
                break;
            }
        }
        return _read_size;
    }

    /*!
     * \fn  virtual bool UnbufferedInputMysqlStream::Good()
     *
     * \brief  Whether this connection is valid.
     *
     * \return  True if the connection is valid.
     */
    bool Good() const override { return connection_->isValid(); }

    /*!
     * \fn  virtual size_t UnbufferedInputMysqlStream::Size()
     *
     * \brief   InputBufferedFileStream set buffer_.buffer_size with Min(64<<20, stream_.Size())
     *          Unbuffered Stream should return a fake_size
     *
     * \return  fake_size
     */
    size_t Size() const override {
        size_t fake_size = 64 << 20;
        return fake_size;
    }

    /*!
     * \fn  virtual bool UnbufferedInputMysqlStream::Seek()
     *
     * \brief  (NOT IMPREMENTED!!)
     */

    bool Seek(size_t offset) override {
        throw std::runtime_error(
            "UnbufferedInputMysqlStream::Seek() is not implemented for "
            "UnbufferedInputMysqlStream!");
    }

    /*!
     * \fn  virtual const std::string UnbufferedInputMysqlStream::Path()
     *
     * \brief  Get the host name of Mysql
     *
     * \return  host_name_
     */

    const std::string& Path() const override { return host_name_; }

    /*!
     * \fn  virtual size_t UnbufferedInputMysqlStream::Offset()
     *
     * \brief  (NOT IMPREMENTED!!)
     */
    virtual size_t Offset() const {
        throw std::runtime_error(
            "UnbufferedInputMysqlStream::Offset() is not implemented for "
            "UnbufferedInputMysqlStream!");
    }

 private:
    /*!
     * \fn  void _ReadOnePage(std::stringstream& ss)
     *
     * \brief  Read {PAGE_SIZE} lines from Mysql
     *
     * \param  ss  StringStream used to receive the data from Mysql.
     */

    void _ReadOnePage(std::stringstream& ss) {
        // std::string query_with_limit = query_ +
        //                               " LIMIT " + std::to_string(PAGE_SIZE) +
        //                               " OFFSET " + std::to_string(page_offset_);
        std::string query_with_limit = query_ +
                                       " as a inner join "
                                       "(select id from " +
                                       table_name_ + " limit " + std::to_string(page_offset_) +
                                       "," + std::to_string(PAGE_SIZE) + ") as b on a.id=b.id";
        statement_ = std::unique_ptr<sql::Statement>(connection_->createStatement());
        std::unique_ptr<sql::ResultSet> res(statement_->executeQuery(query_with_limit));
        size_t column_size = res->getMetaData()->getColumnCount();
        while (res->next()) {
            for (unsigned int i = 1; i <= column_size; ++i) {
                std::string field_ = res->getString(i);
                filter_(field_);
                ss << field_;
                if (i == column_size) {
                    ss << "\n";
                } else {
                    ss << ",";
                }
            }
        }
    }
};
}  // namespace fma_common
#endif
