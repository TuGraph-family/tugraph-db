//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\io_stream.h.
 *
 * \brief   Declares the i/o stream class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/12.
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <string>

#include "fma-common/pipeline.h"
#include "fma-common/stream_base.h"
#include "fma-common/string_util.h"
#include "fma-common/type_traits.h"
#include "fma-common/utils.h"

namespace fma_common {
/*!
 * \class   InputFileStream
 *
 * \brief   Interface for all input streams
 */
class InputFileStream : public InputStreamBase {
 public:
    virtual ~InputFileStream() {}

    /*!
     * \fn  virtual void InputFileStream::Open(const std::string& path, size_t buf_size) = 0;
     *
     * \brief   Opens an input stream
     *
     * \param   path        path of the file, or URI of a HDFS file
     * \param   buf_size    Size of the read buffer in bytes
     */
    virtual void Open(const std::string& path, size_t buf_size) = 0;

    /*!
     * \fn  virtual size_t InputFileStream::Read(void* buf, size_t size) = 0;
     *
     * \brief   Read a block of data into the buffer
     *
     * \param   buf     Buffer used to receive the data
     * \param   size    Number of bytes to read
     *
     * \return  Number of bytes actually read
     */
    virtual size_t Read(void* buf, size_t size) = 0;

    /*!
     * \fn  virtual bool InputFileStream::Good() const = 0;
     *
     * \brief   Whether this file is opened correctly
     *
     * \return  True if it succeeds, false if it fails.
     */
    virtual bool Good() const = 0;

    /*!
     * \fn  virtual void InputFileStream::Close() = 0;
     *
     * \brief   Closes this file.
     */
    virtual void Close() = 0;

    /*!
     * \fn  virtual bool InputFileStream::Seek(size_t offset) = 0;
     *
     * \brief   Seeks to a position in the file
     *
     * \param   offset  Offset from the beginning of the file
     *
     * \return  true if operation is successful, otherwise false
     *          Some streams like snappy streams cannot read from arbitrary offsets,
     *          and they will always return false.
     */
    virtual bool Seek(size_t offset) = 0;

    /*!
     * \fn  virtual size_t InputFileStream::Size() const = 0;
     *
     * \brief   Gets the size of the file
     *
     * \return  Size of the file
     */
    virtual size_t Size() const = 0;

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    virtual const std::string& Path() const = 0;

    /**
     * Gets the current offset in the file.
     *
     * \return  A size_t.
     */
    virtual size_t Offset() const = 0;
};

class InputMemoryFileStream : public InputFileStream {
    std::string buf_;
    const char* beg_;
    size_t size_;
    size_t offset_;

 public:
    InputMemoryFileStream(void* p, size_t s) : beg_((const char*)p), size_(s), offset_(0) {}

    template <typename T>
    explicit InputMemoryFileStream(T&& str)
        : buf_(std::forward<T>(str)), beg_(buf_.data()), size_(buf_.size()), offset_(0) {}

    virtual ~InputMemoryFileStream() {}

    virtual size_t Read(void* buf, size_t size) {
        if (offset_ >= size_) return 0;
        size_t rs = std::min<size_t>(size_ - offset_, size);
        memcpy(buf, beg_ + offset_, rs);
        offset_ += rs;
        return rs;
    }

    virtual size_t Offset() const { return offset_; }

    virtual void Open(const std::string& path, size_t buf_size) {}

    virtual bool Good() const { return offset_ < size_; }

    virtual void Close() {
        buf_.clear();
        beg_ = nullptr;
        size_ = 0;
        offset_ = 0;
    }

    virtual bool Seek(size_t offset) {
        if (offset <= size_) {
            offset_ = offset;
            return true;
        } else {
            return false;
        }
    }

    virtual size_t Size() const { return size_; }

    virtual const std::string& Path() const {
        static std::string p = "memory_file_stream";
        return p;
    }
};

/*!
 * \class   OutputFileStream
 *
 * \brief   Interface for all output streams
 */
class OutputFileStream : public OutputStreamBase {
 public:
    virtual ~OutputFileStream() {}

    /*!
     * \fn  virtual void OutputFileStream::Open(const std::string& path, size_t buf_size = 0,
     * std::ofstream::openmode mode = std::ofstream::trunc) = 0;
     *
     * \brief   Open an output stream
     *
     * \param   path        path of a local file, or URI of an HDFS file
     * \param   buf_size    (Optional) Size of the read buffer in bytes.
     * \param   mode        (Optional) std::ofstream::openmode, effective flags are:
     *                      ofstream::app, ofstream::trunc
     *                      Since HDFS files are append-only, we will assume trunc
     *                      by default, and use append only when app is specified.
     */
    virtual void Open(const std::string& path, size_t buf_size, std::ofstream::openmode) = 0;

    /*!
     * \fn  virtual void OutputFileStream::Write(const void*, size_t) = 0;
     *
     * \brief   Append data to the file
     *
     * \param   buffer  Buffer which stores the data to be written
     * \param   size    Number of bytes to write
     */
    virtual void Write(const void* buffer, size_t size) = 0;

    /*!
     * \fn  virtual bool OutputFileStream::Good() const = 0;
     *
     * \brief   Whether this file is opened correctly
     *
     * \return  True if file is opened correctly, false otherwise
     */
    virtual bool Good() const = 0;

    /*!
     * \fn  virtual void OutputFileStream::Close() = 0;
     *
     * \brief   Closes this file.
     */
    virtual void Close() = 0;

    /*!
     * \fn    virtual size_t OutputFileStream::Size() = 0;
     *
     * \brief    Gets the size of this file.
     *
     * \return    A size_t.
     */
    virtual size_t Size() const = 0;

    virtual void Flush() {}

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    virtual const std::string& Path() const = 0;
};

class OutputMemoryFileStream : public OutputStreamBase {
    std::string path_;
    std::string buffer_;
    std::mutex mutex_;

 public:
    virtual ~OutputMemoryFileStream() {}

    virtual void Open(const std::string& path, size_t buf_size, std::ofstream::openmode) {
        std::lock_guard<std::mutex> l(mutex_);
        path_ = path;
        buffer_.clear();
    }

    virtual void Write(const void* buffer, size_t size) {
        std::lock_guard<std::mutex> l(mutex_);
        buffer_.append((const char*)buffer, size);
    }

    virtual bool Good() const { return true; }

    virtual void Close() {
        std::lock_guard<std::mutex> l(mutex_);
        buffer_.clear();
    }

    virtual size_t Size() const { return buffer_.size(); }

    virtual const std::string& Path() const { return path_; }

    std::string& GetBuf() {
        std::lock_guard<std::mutex> l(mutex_);
        return buffer_;
    }
};
}  // namespace fma_common
