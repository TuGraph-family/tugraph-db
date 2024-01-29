//  Copyright 2024 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\file_stream.h.
 *
 * \brief   Declares the file stream class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/12.
 */
#pragma once
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

#include "fma-common/buffered_file_stream.h"
#include "fma-common/file_stream.h"
#include "fma-common/type_traits.h"
#include "fma-common/utils.h"

namespace fma_common {
/*!
 * \class   UnbufferedInputLocalFileStream
 *
 * \brief   An UnbufferedInputLocalFileStream describes a read-only local file
 *          We implement bufferring in this stream so as to avoid fstream overhead
 *          when reading small chunks
 */
class UnbufferedInputLocalFileStream : public InputFileStream {
    std::string path_;
    std::unique_ptr<std::ifstream> file_;
    /*! \brief   File size */
    uint64_t size_;

 public:
    DISABLE_COPY(UnbufferedInputLocalFileStream);
    UnbufferedInputLocalFileStream() {}

    /*!
     * \fn  UnbufferedInputLocalFileStream::UnbufferedInputLocalFileStream(const std::string& path,
     * size_t buffer_size)
     *
     * \brief   Constructor.
     *
     * \param   path        Full pathname of the file.
     * \param   buffer_size (Optional) Size of the buffer.
     */
    explicit UnbufferedInputLocalFileStream(const std::string& path, size_t buffer_size = 0) {
        Open(path, buffer_size);
    }

    UnbufferedInputLocalFileStream(UnbufferedInputLocalFileStream&& rhs) {
        path_ = std::move(rhs.path_);
        file_ = std::move(rhs.file_);
        size_ = rhs.size_;
        rhs.size_ = 0;
    }

    virtual ~UnbufferedInputLocalFileStream() { Close(); }

    /*!
     * \fn  virtual void UnbufferedInputLocalFileStream::Open(const std::string& path, size_t
     * buf_size, std::ofstream::openmode mode);
     *
     * \brief   Opens an input stream
     *
     * \param   path        path of the file, or URI of a HDFS file
     * \param   buf_size    Size of the read buffer in bytes
     */
    void Open(const std::string& path, size_t buffer_size = 0) override {
        Close();
        file_ = std::make_unique<std::ifstream>();
        file_->open(path, std::ios_base::binary);
        if (!file_->good()) {
            file_.reset();
            return;
        }
        file_->seekg(0, std::ios_base::beg);
        std::ifstream::off_type begin = file_->tellg();
        file_->seekg(0, std::ios_base::end);
        std::ifstream::off_type end = file_->tellg();
        file_->seekg(0, std::ios_base::beg);
        size_ = end - begin;
        path_ = path;
        file_->exceptions(std::ios::badbit);
    }

    /*!
     * \fn  virtual void UnbufferedInputLocalFileStream::Close()
     *
     * \brief   Closes this file.
     */
    void Close() override {
        file_.reset();
        path_.clear();
        size_ = 0;
    }

    /*!
     * \fn  virtual size_t UnbufferedInputLocalFileStream::Read(void* buf, size_t size)
     *
     * \brief   Read a block of data into the buffer
     *
     * \param   buf     Buffer used to receive the data
     * \param   size    Number of bytes to read
     *
     * \return  Number of bytes actually read
     */
    size_t Read(void* buf, size_t size) override {
        CheckOpen();
        if (file_) {
            file_->read((char*)buf, size);
            return file_->gcount();
        } else {
            return 0;
        }
    }

    /*!
     * \fn  bool UnbufferedInputLocalFileStream::Good() const override
     *
     * \brief   Whether this file is opened correctly.
     *
     * \return  True if the file is opened correctly.
     */
    bool Good() const override { return file_ != nullptr && file_->good(); }

    /*!
     * \fn  virtual bool UnbufferedInputLocalFileStream::Seek(size_t offset)
     *
     * \brief   Gets the size of the file
     *
     * \return  Size of the file
     */
    size_t Size() const override {
        CheckOpen();
        return size_;
    }

    /*!
     * \fn  bool UnbufferedInputLocalFileStream::IsOpen() const
     *
     * \brief   Query if this file is open.
     *
     * \return  True if open, false if not.
     */
    bool IsOpen() const { return file_ != nullptr && file_->is_open(); }

    /*!
     * \fn  virtual bool UnbufferedInputLocalFileStream::Seek(size_t offset)
     *
     * \brief   Seeks to a position in the file
     *
     * \param   offset  Offset from the beginning of the file
     *
     * \return  true if operation is successful, otherwise false
     */
    bool Seek(size_t offset) override {
        file_->clear();
        file_->seekg(offset, std::ios_base::beg);
        return file_->good();
    }

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    const std::string& Path() const override { return path_; }

    size_t Offset() const override { return file_->tellg(); }

 private:
    void CheckOpen() const {
        if (file_ == nullptr || !file_->is_open()) {
            throw std::runtime_error("Failed to open file " + path_ + " for read.");
        }
    }
};

/*!
 * \class   UnbufferedOutputLocalFileStream
 *
 * \brief   An output file stream.
 */
class UnbufferedOutputLocalFileStream : public OutputFileStream {
    std::unique_ptr<std::ofstream> file_;
    std::string path_;
    uint64_t size_;

 public:
    DISABLE_COPY(UnbufferedOutputLocalFileStream);

    UnbufferedOutputLocalFileStream() {}

    /*!
     * \fn  UnbufferedOutputLocalFileStream::UnbufferedOutputLocalFileStream(const std::string&
     * path, uint64_t buffer_size = DEFAULT_BUFFER_SIZE, std::ofstream::openmode mode =
     * std::ofstream::trunc)
     *
     * \brief   Constructor.
     *
     * \param   path        Full pathname of the file.
     * \param   buffer_size (Optional) Size of the buffer.
     * \param   mode        (Optional) std::ofstream::openmode, effective flags are:
     *                      ofstream::app, ofstream::trunc
     *                      Since HDFS files are append-only, we will assume trunc
     *                      by default, and use append only when app is specified.
     */
    UnbufferedOutputLocalFileStream(const std::string& path, uint64_t buffer_size = 0,
                                    std::ofstream::openmode mode = std::ofstream::trunc) {
        Open(path, buffer_size, mode);
    }

    UnbufferedOutputLocalFileStream(UnbufferedOutputLocalFileStream&& rhs) {
        file_ = std::move(rhs.file_);
        path_ = std::move(rhs.path_);
        size_ = rhs.size_;
        rhs.size_ = 0;
    }

    virtual ~UnbufferedOutputLocalFileStream() { Close(); }

    /*!
     * \fn  virtual void UnbufferedOutputLocalFileStream::Open(const std::string& path, size_t
     * buf_size = 0, std::ofstream::openmode mode = std::ofstream::trunc) = 0;
     *
     * \brief   Open an output stream
     *
     * \param   path        path of a local file, or URI of an HDFS file
     * \param   buf_size    (Optional) Size of the read buffer in bytes.
     * \param   mode        (Optional) std::ofstream::openmode, effective flags are:
     *                      ofstream::app, ofstream::trunc
     */
    void Open(const std::string& path, size_t buffer_size = 0,
                      std::ofstream::openmode mode = std::ofstream::trunc) override {
        if (IsOpen()) {
            Close();
        }
        path_ = path;
        file_ = std::make_unique<std::ofstream>();
        file_->open(path, std::fstream::binary | mode);
        if (!file_->is_open()) {
            file_.reset();
            return;
        }
        file_->seekp(0, std::ofstream::beg);
        std::ofstream::off_type begin = file_->tellp();
        file_->seekp(0, std::ofstream::end);
        std::ofstream::off_type end = file_->tellp();
        size_ = end - begin;
        file_->exceptions(std::ios::badbit);
    }

    /*!
     * \fn  virtual void UnbufferedOutputLocalFileStream::Close()
     *
     * \brief   Closes this file.
     */
    void Close() override {
        if (file_) file_->flush();
        file_.reset();
        path_.clear();
        size_ = 0;
    }

    /*!
     * \fn  bool UnbufferedOutputLocalFileStream::IsOpen() const
     *
     * \brief   Query if this file is open.
     *
     * \return  True if open, false if not.
     */
    bool IsOpen() const { return file_ != nullptr && file_->is_open(); }

    /*!
     * \fn  size_t UnbufferedOutputLocalFileStream::Size() const
     *
     * \brief   Gets the size of the file.
     *
     * \return  Size of the file.
     */
    size_t Size() const {
        CheckOpen();
        return size_;
    }

    /*!
     * \fn  virtual void UnbufferedOutputLocalFileStream::Write(const void*, size_t)
     *
     * \brief   Append data to the file
     *
     * \param   buffer  Buffer which stores the data to be written
     * \param   size    Number of bytes to write
     */
    void Write(const void* p, size_t size) override {
        CheckOpen();
        file_->write((const char*)p, size);
        size_ += size;
    }

    /*!
     * \fn  virtual bool UnbufferedOutputLocalFileStream::Good() const
     *
     * \brief   Whether this file is opened correctly
     *
     * \return  True if file is opened correctly, false otherwise
     */
    bool Good() const override { return file_ != nullptr && file_->good(); }

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    const std::string& Path() const override { return path_; }

    void Flush() override {
        CheckOpen();
        file_->flush();
    }

 private:
    void CheckOpen() const {
        if (!IsOpen()) {
            throw std::runtime_error("Failed to open file " + path_ + " for write.");
        }
    }
};

typedef InputBufferedFileStream<UnbufferedInputLocalFileStream, PrefetchingtStreamBuffer>
    InputLocalFileStream;
typedef OutputBufferedFileStream<UnbufferedOutputLocalFileStream, ThreadedOutputStreamBuffer>
    OutputLocalFileStream;
}  // namespace fma_common
