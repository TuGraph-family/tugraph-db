//  Copyright 2024 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\piped_hdfs_stream.h.
 *
 * \brief   Declares the piped hdfs stream class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/15.
 */
#pragma once

#include <exception>

#if (!HAS_LIBHDFS)
#include <stdio.h>
#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif
#include "tools/lgraph_log.h"
#include "fma-common/buffered_file_stream.h"
#include "fma-common/file_stream.h"
#include "fma-common/string_util.h"
#include "fma-common/type_traits.h"

namespace fma_common {
namespace _detail {
template <typename T>
inline bool GetOutputOfCmd(const std::string& cmd, size_t field_id, T& val, int max_buf = 2048) {
    FILE* f = _detail::OpenPipe(cmd, "r");
    std::string buf(max_buf, 0);
    char* r = fgets(&buf[0], max_buf, f);
    if (!r) {
        LOG_ERROR() << "Error reading output of command " << cmd;
    }
    fclose(f);
    std::vector<std::string> parts = Split(buf);
    if (parts.size() <= field_id) {
        LOG_ERROR() << "Error parsing result of command: " << cmd << "\n\tOutput: " << buf
                  << "\n\tfield_id: " << field_id;
    }
    return ParseString(parts[field_id], val);
}
}  // namespace _detail

/*!
 * \class   UnbufferedInputHdfsStream
 *
 * \brief   A stream that uses pipe and "hdfs dfs -cat" to read files
 *          from HDFS. It supports plain text files and .GZ files.
 */
class UnbufferedInputHdfsStream : public InputFileStream {
    FILE* pipe_ = nullptr;
    size_t size_ = 0;
    std::string path_;
    size_t read_bytes_ = 0;

 public:
    DISABLE_COPY(UnbufferedInputHdfsStream);
    UnbufferedInputHdfsStream() {}

    /*!
     * \fn  UnbufferedInputHdfsStream::UnbufferedInputHdfsStream(const std::string& path, size_t
     * buf_size = 0)
     *
     * \brief   Constructor.
     *
     * \param   path        Full pathname of the file. Can be with or without
     *                      hdfs://. If the path ends with .GZ, then it is
     *                      treated as a compressed file.
     * \param   buf_size    (Optional) Size of the buffer. Ignored here.
     */
    explicit UnbufferedInputHdfsStream(const std::string& path, size_t buf_size = 0) {
        Open(path, buf_size);
    }

    virtual ~UnbufferedInputHdfsStream() { Close(); }

    /*!
     * \fn  virtual void UnbufferedInputHdfsStream::Open(const std::string& path, size_t buf_size =
     * 0)
     *
     * \brief   Opens the given file.
     *
     * \param   path        Full pathname of the file. Can be with or without
     *                      hdfs://. If the path ends with .GZ, then it is
     *                      treated as a compressed file.
     * \param   buf_size    (Optional) Size of the buffer. Ignored here.
     */
    void Open(const std::string& path, size_t buf_size = 0) override {
        size_ = GetFileSize(path);
        if (size_ == size_t(-1)) {
            size_ = 0;
            return;
        }
        std::string cmd = HDFS_CMD() + " -cat " + path;
        if (EndsWith(path, ".gz", false)) {
            cmd += "| gunzip -c";
        }
        pipe_ = _detail::OpenPipe(cmd, "r");
        path_ = path;
        read_bytes_ = 0;
    }

    /*!
     * \fn  virtual size_t UnbufferedInputHdfsStream::Read(void* buf, size_t size)
     *
     * \brief   Read a block of data into the buffer
     *
     * \param   buf     Buffer used to receive the data
     * \param   size    Number of bytes to read
     *
     * \return  Number of bytes actually read
     */
    size_t Read(void* buf, size_t size) override {
        FMA_ASSERT(pipe_ != nullptr)
            << "Error reading file " << path_ << ": File is not opened properly";
        size_t s = fread(buf, 1, size, pipe_);
        read_bytes_ += s;
        return s;
    }

    /*!
     * \fn  virtual bool UnbufferedInputHdfsStream::Good() const
     *
     * \brief   Whether this file is opened correctly.
     *
     * \return  True if the file is opened correctly.
     */
    bool Good() const override { return pipe_ != nullptr; }

    /*!
     * \fn  virtual void UnbufferedInputHdfsStream::Close()
     *
     * \brief   Closes this file.
     */
    void Close() override {
        if (pipe_) {
            pclose(pipe_);
            pipe_ = nullptr;
            size_ = 0;
            path_.clear();
            read_bytes_ = 0;
        }
    }

    /*!
     * \fn  virtual bool UnbufferedInputHdfsStream::Seek(size_t offset)
     *
     * \brief   (NOT IMPLEMENTED!!) Seeks the given offset.
     *
     * \param   offset  The offset.
     *
     * \return  Always false.
     */
    bool Seek(size_t offset) override {
        throw std::runtime_error("Seek is not implemented for UnbufferedInputHdfsStream!");
        return false;
    }

    /*!
     * \fn  virtual size_t UnbufferedInputHdfsStream::Size() const
     *
     * \brief   Gets the size of the file.
     *
     * \note    This function is not acurate for .GZ files. It returns the compressed
     *          size, while Read() returns decompressed bytes.
     *
     * \return  Size of the file.
     */
    virtual size_t Size() const { return size_; }

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    const std::string& Path() const override { return path_; }

    size_t Offset() const override { return read_bytes_; }

 private:
    static size_t GetFileSize(const std::string& path) {
        std::string cmd = HDFS_CMD() + " -ls " + path;
        size_t size = size_t(-1);
        bool r = _detail::GetOutputOfCmd(cmd, 4, size);
        if (!r) {
            LOG_WARN() << "Failed to get size of " << path << ": File does not exist.";
        }
        return size;
    }
};

/*!
 * \class   UnbufferedOutputHdfsStream
 *
 * \brief   An output stream that writes to HDFS using pipe and "hdfs dfs -put".
 *          It supports both plain text and .GZ files.
 */
class UnbufferedOutputHdfsStream : public OutputFileStream {
    FILE* pipe_ = nullptr;
    size_t size_ = 0;
    std::string path_;

 public:
    DISABLE_COPY(UnbufferedOutputHdfsStream);

    UnbufferedOutputHdfsStream() {}

    /*!
     * \fn  UnbufferedOutputHdfsStream::UnbufferedOutputHdfsStream(const std::string& path, size_t
     * buf_size = 0, std::ofstream::openmode mode = std::ofstream::trunc)
     *
     * \brief   Constructor.
     *
     * \param   path        Full pathname of the file, with or without hdfs://.
     * \param   buf_size    (Optional) Size of the buffer. Ignored.
     * \param   mode        (Optional) The mode. Ignored. Existing file is
     *                      always replaced.
     */
    UnbufferedOutputHdfsStream(const std::string& path, size_t buf_size = 0,
                               std::ofstream::openmode mode = std::ofstream::trunc) {
        Open(path, buf_size, mode);
    }

    virtual ~UnbufferedOutputHdfsStream() { Close(); }

    /*!
     * \fn  virtual void UnbufferedOutputHdfsStream::Open(const std::string& path, size_t buf_size =
     * 0, std::ofstream::openmode mode = std::ofstream::trunc)
     *
     * \brief   Opens the given file.
     *
     * \param   path        Full pathname of the file, with or without hdfs://.
     * \param   buf_size    (Optional) Size of the buffer. Ignored.
     * \param   mode        (Optional) The mode. Ignored. Existing file is
     *                      always replaced.
     */
    void Open(const std::string& path, size_t buf_size = 0,
                      std::ofstream::openmode mode = std::ofstream::trunc) override {
        Close();
        FMA_ASSERT(mode == std::ofstream::trunc)
            << "UnbufferedOutputHdfsStream can only be opened with ofstream::trunc mode";
        std::string cmd;
        if (EndsWith(path, ".gz", false)) {
            cmd += "gzip -c |";
        }
        cmd += HDFS_CMD() + " -put -f - " + path;
        pipe_ = _detail::OpenPipe(cmd, "w");
        size_ = 0;
        path_ = path;
    }

    /*!
     * \fn  virtual void UnbufferedOutputHdfsStream::Write(const void* buf, size_t size)
     *
     * \brief   Append data to the file
     *
     * \param   buffer  Buffer which stores the data to be written
     * \param   size    Number of bytes to write
     */
    void Write(const void* buf, size_t size) override {
        CheckOpen();
        WritePipe(buf, size);
        size_ += size;
    }

    /*!
     * \fn  virtual bool UnbufferedOutputHdfsStream::Good() const
     *
     * \brief   Whether this file is opened correctly
     *
     * \return  True if file is opened correctly, false otherwise
     */
    bool Good() const override { return pipe_ != nullptr; }

    /*!
     * \fn  virtual void UnbufferedOutputHdfsStream::Close()
     *
     * \brief   Closes this file.
     */
    void Close() override {
        if (pipe_) {
            fflush(pipe_);
            pclose(pipe_);
            pipe_ = nullptr;
            size_ = 0;
            path_.clear();
        }
    }

    size_t Size() const override { return size_; }

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    const std::string& Path() const override { return path_; }

    void Flush() override {
        if (pipe_) fflush(pipe_);
    }

 private:
    void CheckOpen() {
        if (!pipe_) {
            throw std::runtime_error("Failed to open file " + path_ + " for write.");
        }
    }

    void WritePipe(const void* buf, size_t size) {
        size_t s = fwrite(buf, 1, size, pipe_);
        FMA_CHECK_EQ(s, size) << "Error writing to PipedHdfsStream " << path_
                              << ": error=" << ferror(pipe_);
    }
};

typedef InputBufferedFileStream<UnbufferedInputHdfsStream, PrefetchingtStreamBuffer>
    InputHdfsStream;
typedef OutputBufferedFileStream<UnbufferedOutputHdfsStream, ThreadedOutputStreamBuffer>
    OutputHdfsStream;
}  // namespace fma_common
#endif
