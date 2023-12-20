//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\fma_stream.h.
 *
 * \brief   Declares the fma stream class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/12.
 */
#pragma once

#include <memory>

#include "fma-common/file_system.h"
#include "fma-common/local_file_stream.h"
#include "fma-common/piped_hdfs_stream.h"
#include "fma-common/type_traits.h"

#if FMA_HAS_LIBMYSQL
#include "fma-common-enterprise/buffered_sql_stream.h"
#include "fma-common-enterprise/sql_stream.h"
#endif

#if FMA_HAS_LIBHDFS
#include "fma-common/libhdfs_stream.h"
#else
// #include "fma-common/piped_hdfs_stream.h"
#endif

#if FMA_ENABLE_SNAPPY
#include "fma-common/snappy_stream.h"
#endif
#include "fma-common/utils.h"

namespace fma_common {
/*!
 * \class   InputFmaStream
 *
 * \brief   Unified interface for local file and hdfs file access,
 *          with or without snappy compression
 */
class InputFmaStream : public InputFileStream {
    std::unique_ptr<InputFileStream> file_;

 public:
    DISABLE_COPY(InputFmaStream);
    InputFmaStream() {}

    InputFmaStream(InputFmaStream&& rhs) { file_ = std::move(rhs.file_); }

    InputFmaStream& operator=(InputFmaStream&& rhs) {
        file_ = std::move(rhs.file_);
        return *this;
    }

    /*!
     * \fn  InputFmaStream::InputFmaStream(const std::string& path, size_t buf_size = 64 << 20, bool
     * snappy_compressed = false)
     *
     * \brief   Constructor.
     *
     * \param   path                Full pathname of the file.
     * \param   buf_size            (Optional) Size of the buffer.
     * \param   snappy_compressed   (Optional) True if snappy compressed.
     */
    InputFmaStream(const std::string& path, size_t buf_size = 64 << 20,
                   bool snappy_compressed = false) {
        Open(path, buf_size, snappy_compressed);
    }

    ~InputFmaStream() { Close(); }

    void Open(const std::string& path, size_t buf_size = 64 << 20) override {
        Open(path, buf_size, false);
    }

    /*!
     * \fn  void InputFmaStream::Open(const std::string& path, size_t buf_size, bool
     * snappy_compressed)
     *
     * \brief   Opens the given file.
     *
     * \param   path                Full pathname of the file.
     * \param   buf_size            Size of the buffer.
     * \param   snappy_compressed   True if snappy compressed.
     */
    void Open(const std::string& path, size_t buf_size, bool snappy_compressed) {
        Close();
        if (snappy_compressed) {
#if FMA_ENABLE_SNAPPY
            file_ = new SnappyInputStream(path, buf_size);
#else
            LOG_ERROR() << "Please define FMA_ENABLE_SNAPPY=1 to enable snappy stream";
#endif
        } else {
            if (FilePath(path).Scheme() == FilePath::SchemeType::LOCAL) {
                file_ = std::make_unique<InputLocalFileStream>(path, buf_size);
            } else if (FilePath(path).Scheme() == FilePath::SchemeType::HDFS) {
                file_ = std::make_unique<InputHdfsStream>(path, buf_size);
            } else if (FilePath(path).Scheme() == FilePath::SchemeType::MYSQL ||
                       FilePath(path).Scheme() == FilePath::SchemeType::SQLSERVER) {
#if FMA_HAS_LIBMYSQL
                file_ = std::make_unique<InputSqlStream>(path, buf_size);
#else
                LOG_ERROR() << "Mysql stream is not enabled. Please define FMA_HAS_LIBMYSQL=1 to "
                             "enable it.";
#endif
            }
        }
    }

    /*!
     * \fn  virtual size_t InputFmaStream::Read(void* buf, size_t size)
     *
     * \brief   Read a block of data into the buffer
     *
     * \param   buf     Buffer used to receive the data
     * \param   size    Number of bytes to read
     *
     * \return  Number of bytes actually read
     */
    size_t Read(void* buf, size_t size) override {
        if (!file_) return 0;
        return file_->Read(buf, size);
    }

    /*!
     * \fn  virtual bool InputFmaStream::Good() const
     *
     * \brief   Whether this file is opened correctly
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Good() const override { return file_ && file_->Good(); }

    /*!
     * \fn  virtual void InputFmaStream::Close()
     *
     * \brief   Closes this file.
     */
    void Close() override { file_.reset(); }

    /*!
     * \fn  virtual bool InputFmaStream::Seek(size_t offset)
     *
     * \brief   Seeks to a position in the file
     *
     * \param   offset  Offset from the beginning of the file
     *
     * \return  true if operation is successful, otherwise false
     *          Some streams like snappy streams cannot read from arbitrary offsets,
     *          and they will always return false.
     */
    bool Seek(size_t offset) override {
        if (!file_) return false;
        return file_->Seek(offset);
    }

    /*!
     * \fn  virtual size_t InputFmaStream::Size() const
     *
     * \brief   Gets the size of the file
     *
     * \return  Size of the file
     */
    size_t Size() const override { return Good() ? file_->Size() : 0; }

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    const std::string& Path() const override {
        return file_ ? file_->Path() : _detail::EMPTY_STRING();
    }

    size_t Offset() const override { return file_ ? file_->Offset() : 0; }
};

class OutputFmaStream : public OutputFileStream {
    std::unique_ptr<OutputFileStream> file_;

 public:
    DISABLE_COPY(OutputFmaStream);
    /*! \brief   The default block size for snappy files */
    static const size_t DEFAULT_BLOCK_SIZE = 64 << 20;
    OutputFmaStream() {}

    OutputFmaStream(OutputFmaStream&& rhs) { file_ = std::move(rhs.file_); }

    OutputFmaStream& operator=(OutputFmaStream&& rhs) {
        file_ = std::move(rhs.file_);
        return *this;
    }

    /*!
     * \fn  OutputFmaStream::OutputFmaStream(const std::string& path, bool snappy_compressed =
     * false, size_t n_buffers = 1, size_t buf_size = DEFAULT_BLOCK_SIZE, std::ofstream::openmode
     * mode = std::ofstream::trunc)
     *
     * \brief   Constructor.
     *
     * \param   path                Full pathname of the file.
     * \param   snappy_compressed   (Optional) True if snappy compressed.
     * \param   n_buffers           (Optional) The buffers.
     * \param   buf_size            (Optional) Size of the buffer.
     * \param   mode        (Optional) std::ofstream::openmode, effective flags are:
     *                      ofstream::app, ofstream::trunc
     *                      Since HDFS files are append-only, we will assume trunc
     *                      by default, and use append only when app is specified.
     */
    OutputFmaStream(const std::string& path, size_t buf_size = DEFAULT_BLOCK_SIZE,
                    std::ofstream::openmode mode = std::ofstream::trunc,
                    bool snappy_compressed = false, size_t n_buffers = 1) {
        if (snappy_compressed) {
            OpenSnappy(path, n_buffers, buf_size, mode);
        } else {
            OpenNoSnappy(path, buf_size, mode);
        }
    }

    ~OutputFmaStream() { Close(); }

    /*!
     * \fn  virtual void OutputFmaStream::Open(const std::string& path, size_t buf_size = 0,
     * std::ofstream::openmode mode = std::ofstream::trunc) override
     *
     * \brief   Opens the given file.
     *
     * \param   path        path of a local file, or URI of an HDFS file
     * \param   buf_size    (Optional) Size of the read buffer in bytes.
     * \param   mode        (Optional) std::ofstream::openmode, effective flags are:
     *                      ofstream::app, ofstream::trunc
     *                      Since HDFS files are append-only, we will assume trunc
     *                      by default, and use append only when app is specified.
     */
    void Open(const std::string& path, size_t buf_size = DEFAULT_BLOCK_SIZE,
                      std::ofstream::openmode mode = std::ofstream::trunc) override {
        OpenNoSnappy(path, buf_size, mode);
    }

    /*!
     * \fn  virtual void OutputFmaStream::OpenSnappy(const std::string& path, size_t n_buffers = 1,
     * size_t block_size = DEFAULT_BLOCK_SIZE, std::ofstream::openmode mode = std::ofstream::trunc)
     *
     * \brief   Opens a snappy.
     *
     * \param   path        path of a local file, or URI of an HDFS file
     * \param   n_buffers   (Optional) Number of buffers to use
     * \param   block_size  (Optional) Size of each block
     * \param   mode        (Optional) std::ofstream::openmode, effective flags are:
     *                      ofstream::app, ofstream::trunc
     *                      Since HDFS files are append-only, we will assume trunc
     *                      by default, and use append only when app is specified.
     */
    virtual void OpenSnappy(const std::string& path, size_t n_buffers = 1,
                            size_t block_size = DEFAULT_BLOCK_SIZE,
                            std::ofstream::openmode mode = std::ofstream::trunc) {
#if FMA_ENABLE_SNAPPY
        file_ = new SnappyOutputStream(path, n_buffers, block_size, mode);
#else
        LOG_ERROR() << "Please define ENABLE_SNAPPY=1 to enable snappy stream";
#endif
    }

    /*!
     * \fn  virtual void OutputFmaStream::OpenNoSnappy(const std::string& path, size_t buf_size,
     * std::ofstream::openmode mode)
     *
     * \brief   Opens a non snappy file.
     *
     * \param   path        path of a local file, or URI of an HDFS file
     * \param   buf_size    (Optional) Size of the read buffer in bytes.
     * \param   mode        (Optional) std::ofstream::openmode, effective flags are:
     *                      ofstream::app, ofstream::trunc
     *                      Since HDFS files are append-only, we will assume trunc
     *                      by default, and use append only when app is specified.
     */
    virtual void OpenNoSnappy(const std::string& path, size_t buf_size,
                              std::ofstream::openmode mode) {
        if (FilePath(path).Scheme() == FilePath::SchemeType::LOCAL) {
            if (buf_size == 0)
                file_ = std::make_unique<UnbufferedOutputLocalFileStream>(path, buf_size, mode);
            else
                file_ = std::make_unique<OutputLocalFileStream>(path, buf_size, mode);
        } else {
            // NOTE: we should never set a small buffer size for HDFS, since it reads
            // from remote, and the overhead can be significant if we use small buffer.
            file_ =
                std::make_unique<OutputHdfsStream>(path, std::max<size_t>(buf_size, 4 << 20), mode);
        }
    }

    /*!
     * \fn  virtual void OutputFmaStream::Write(const void*, size_t)
     *
     * \brief   Append data to the file
     *
     * \param   buffer  Buffer which stores the data to be written
     * \param   size    Number of bytes to write
     */
    void Write(const void* buffer, size_t size) override {
        if (file_) return file_->Write(buffer, size);
    }

    /*!
     * \fn  virtual bool OutputFmaStream::Good() const
     *
     * \brief   Whether this file is opened correctly
     *
     * \return  True if file is opened correctly, false otherwise
     */
    bool Good() const override { return file_ && file_->Good(); }

    /*!
     * \fn  virtual void OutputFmaStream::Close()
     *
     * \brief   Closes this file.
     */
    void Close() override {
        if (file_) file_->Flush();
        file_.reset();
    }

    void Flush() override {
        if (file_) file_->Flush();
    }

    size_t Size() const override { return file_ ? file_->Size() : (size_t)0; }

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    const std::string& Path() const override {
        return file_ ? file_->Path() : _detail::EMPTY_STRING();
    }
};
}  // namespace fma_common
