//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\snappy_stream.h.
 *
 * \brief   This file contains the definition of compressed streams.
 *
 *          A compressed file is in the following format:
 *              {{CompressedBlockHeader} {compressed bytes}}*
 *
 *          CompressedBlockHeader is defined as:
 *          {
 *              int32_t MAGIC_NUM = 0x67841;
 *              uint64_t orig_size_;        // number of bytes uncompressed
 *              uint64_t compressed_size_;  // number of bytes after compression
 *          }
 *
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/15.
 */
#pragma once

#if ENABLE_SNAPPY
#include <algorithm>
#include <atomic>
#include <queue>
#include <future>

#include "snappy/snappy-c.h"

#include "fma-common/binary_read_write_helper.h"
#include "fma-common/local_file_stream.h"
#if HAS_LIBHDFS
#include "fma-common/libhdfs_stream.h"
#else
#include "fma-common/piped_hdfs_stream.h"
#endif
#include "fma-common/pipeline.h"
#include "fma-common/type_traits.h"
#include "fma-common/utils.h"

namespace fma_common {
struct SnappyBlockHeader {
    static const uint32_t MAGIC_NUM() { return 0x67841; }

    /*! \brief   A magic number to make sure we are reading a snappy file */
    uint32_t magic_num_ = MAGIC_NUM();
    /*! \brief   Number of bytes before compression */
    uint64_t orig_size_;
    /*! \brief   Number of bytes after compression */
    uint64_t compressed_size_;

    size_t HeaderSize() {
        return sizeof(magic_num_) + sizeof(orig_size_) + sizeof(compressed_size_);
    }

    bool Load(InputFileStream& in) {
        return BinaryRead(in, magic_num_) && magic_num_ == MAGIC_NUM() &&
               BinaryRead(in, orig_size_) && BinaryRead(in, compressed_size_);
    }

    void Store(OutputFileStream& out) {
        BinaryWrite(out, magic_num_);
        BinaryWrite(out, orig_size_);
        BinaryWrite(out, compressed_size_);
    }
};

/*!
 * \class   SnappyInputStream
 *
 * \brief   An input stream that reads Snappy compressed file
 */
class SnappyInputStream : public InputFileStream {
    using UniqueLock = std::unique_lock<std::mutex>;
    using LockGuard = std::lock_guard<std::mutex>;
    static const size_t DEFAULT_PREFETCH = 1;
    struct Workspace {
        std::string raw_buf;
        std::string data;
    };

    InputFileStream* file_{};
    std::atomic<bool> file_opened_ = {false};
    std::atomic<bool> all_buffered_ = {false};
    size_t n_prefetch_ = DEFAULT_PREFETCH;

    std::vector<Workspace> workspace_;  // circular buffer used in prefetcher
    PipelineStage<int, int>* read_stage_ = nullptr;
    PipelineStage<int, int>* unzip_stage_ = nullptr;
    BoundedQueue<int> unzipped_data_;
    std::string curr_buf_;
    size_t curr_buf_offset_ = 0;

 public:
    DISABLE_COPY(SnappyInputStream);
    SnappyInputStream() : file_opened_(false), all_buffered_(false) {}

    /*!
     * \fn  SnappyInputStream::SnappyInputStream(const std::string& path, size_t n_prefetch =
     * DEFAULT_PREFETCH)
     *
     * \brief   Constructor.
     *
     * \param   path        Full pathname of the file.
     * \param   n_prefetch  (Optional) The number of blocks to prefetch.
     */
    explicit SnappyInputStream(const std::string& path, size_t n_prefetch = DEFAULT_PREFETCH)
        : file_opened_(false), all_buffered_(false) {
        Open(path, n_prefetch);
    }

    ~SnappyInputStream() { Close(); }

    /*!
     * \fn  virtual void SnappyInputStream::Open(const std::string& path, size_t buf_size = 0)
     * override
     *
     * \brief   Open an input stream
     *
     * \param   path        path of the file, or URI of a HDFS file
     * \param   buf_size    (Optional) Size of the read buffer in bytes
     *                      Setting the buf_size to 0 will disable buffering.
     *                      The buffer size will be rounded to nearest multiple of
     *                      buffer size;
     */
    void Open(const std::string& path, size_t buf_size = 0) override {
        Close();
        if (FilePath(path).Scheme() == FilePath::SchemeType::LOCAL) {
            file_ = new InputLocalFileStream(path, 0);
        } else {
            file_ = new InputHdfsStream(path, 0);
        }
        file_opened_ = file_->Good();
        SnappyBlockHeader header;
        if (!PeekHeader(header)) {
            file_opened_ = false;
            return;
        }
        n_prefetch_ = (size_t)std::round((double)buf_size / header.orig_size_);
        all_buffered_ = false;
        if (!file_opened_) return;
        if (n_prefetch_ > 0) {
            workspace_.resize(n_prefetch_);
            read_stage_ = new PipelineStage<int, int>([this](int i) -> int { return ReadStage(i); },
                                                      nullptr, 0, 1, n_prefetch_);
            unzip_stage_ = new PipelineStage<int, int>(
                [this](int i) -> int { return UnzipStage(i); }, nullptr, 0, n_prefetch_);
            read_stage_->SetNextStage(unzip_stage_);
            unzip_stage_->SetNextStage(&unzipped_data_);
            for (int i = 0; i < n_prefetch_; i++) {
                read_stage_->Push(i);
            }
        }
    }

    /*!
     * \fn  void SnappyInputStream::Close() override
     *
     * \brief   Closes this file.
     */
    void Close() override {
        if (!file_opened_) return;
        if (n_prefetch_ > 0) {
            read_stage_->Stop();
            unzip_stage_->Stop();
            delete read_stage_;
            delete unzip_stage_;
        }
        workspace_.clear();
        file_->Close();
        delete file_;
        file_ = nullptr;
        file_opened_ = false;
    }

    /*!
     * \fn  size_t SnappyInputStream::Read(void* buf, size_t s) override
     *
     * \brief   Read a block of data into the buffer
     *
     * \param   buf     Buffer used to receive the data
     * \param   size    Number of bytes to read
     *
     * \return  Number of bytes actually read
     */
    size_t Read(void* buf, size_t s) override {
        CheckOpen();
        size_t read_bytes = 0;
        while (read_bytes < s) {
            if (curr_buf_.size() == curr_buf_offset_) {
                bool r = Read(curr_buf_);
                if (!r) break;
                curr_buf_offset_ = 0;
            }
            size_t to_read = std::min(s - read_bytes, curr_buf_.size() - curr_buf_offset_);
            memcpy((char*)buf + read_bytes, &curr_buf_[curr_buf_offset_], to_read);
            curr_buf_offset_ += to_read;
            read_bytes += to_read;
        }
        return read_bytes;
    }

    /*!
     * \fn  bool SnappyInputStream::Good() const override
     *
     * \brief   Whether this file is opened correctly
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Good() const override { return file_opened_; }

    /*!
     * \fn  bool SnappyInputStream::IsOpen() const
     *
     * \brief   Query if this file is open.
     *
     * \return  True if open, false if not.
     */
    bool IsOpen() const { return file_opened_; }

    /*!
     * \fn  bool SnappyInputStream::Seek(size_t) override
     *
     * \brief   (NOT IMPLEMENTED!!)Seeks to a position in the file
     *
     * \param   offset  Offset from the beginning of the file
     *
     * \return  Always false.
     */
    bool Seek(size_t) override {
        FMA_ERR() << "SnappyInputStream::Seek() is not supported";
        return false;
    }

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    const std::string& Path() const override {
        return file_ ? file_->Path() : _detail::EMPTY_STRING();
    }

 private:
    void CheckOpen() {
        if (!file_opened_) {
            throw std::runtime_error("Failed to open file " + file_->Path() + " for read.");
        }
    }

    /*
     * \brief read a snappy block
     * \param buf: buf to read into, contents in buf will be overwritten
     * \return true if read success, false if any error occurs, or EOF
     */
    bool Read(std::string& buf) {
        if (!file_opened_) return false;
        if (n_prefetch_ <= 0) {
            // no prefetch
            std::string raw_buf;
            if (!ReadRawBlock(raw_buf)) {
                return false;
            } else {
                Unzip(raw_buf, buf);
                return true;
            }
        } else {
            int d;
            bool r = unzipped_data_.PopFront(d);
            if (r) {
                if (d == -1) return false;  // read error, or reached eof
                buf.swap(workspace_[d].data);
                read_stage_->Push(d);
            }
            return r;
        }
    }

    void Unzip(const std::string& raw_buf, std::string& data) {
        size_t len;
        snappy_status st = snappy_uncompressed_length(&raw_buf[0], raw_buf.size(), &len);
        FMA_CHECK_EQ(st, SNAPPY_OK);
        data.resize(len);
        st = snappy_uncompress(&raw_buf[0], raw_buf.size(), &data[0], &len);
        FMA_CHECK_EQ(st, SNAPPY_OK);
    }

    bool ReadRawBlock(std::string& raw_buf) {
        SnappyBlockHeader header;
        if (!file_->Good()) return false;
        if (!header.Load(*file_)) return false;
        raw_buf.resize(header.compressed_size_);
        if (file_->Read(&raw_buf[0], header.compressed_size_) != header.compressed_size_)
            return false;
        return true;
    }

    bool PeekHeader(SnappyBlockHeader& header) {
        if (!file_->Good()) return false;
        if (!header.Load(*file_)) return false;
        file_->Seek(0);
        return true;
    }

    int ReadStage(int d) {
        Workspace& e = workspace_[d];
        bool r = ReadRawBlock(e.raw_buf);
        if (r) {
            return d;
        } else {
            e.raw_buf.clear();
            e.data.clear();
            return -1;
        }
    }

    int UnzipStage(int d) {
        if (d == -1) return d;
        Workspace& e = workspace_[d];
        if (!e.raw_buf.empty()) {
            Unzip(e.raw_buf, e.data);
        }
        return d;
    }
};

/*!
 * \class   SnappyOutputStream
 *
 * \brief   A snappy output stream.
 */
class SnappyOutputStream : public OutputFileStream {
    struct Workspace {
        std::string buf;
        size_t size;
        std::string zipped;
        size_t zipped_size;

        void swap(Workspace& rhs) {
            std::swap(size, rhs.size);
            std::swap(zipped_size, rhs.zipped_size);
            buf.swap(rhs.buf);
            zipped.swap(rhs.zipped);
        }
    };

    OutputFileStream* file_;
    bool is_open_ = false;
    size_t block_size_ = DEFAULT_BLOCK_SIZE;

    std::vector<Workspace> workspace_;
    PipelineStage<int, int>* zip_stage_;
    PipelineStage<int, int>* write_stage_;
    BoundedQueue<int> empty_buffers_;
    Workspace curr_buf_;

 public:
    static const int DEFAULT_BLOCK_SIZE = 64 * 1024 * 1024;
    DISABLE_COPY(SnappyOutputStream);
    SnappyOutputStream() {}

    /*!
     * \fn  SnappyOutputStream::SnappyOutputStream(const std::string& path, size_t n_buffers = 1,
     * size_t block_size = DEFAULT_BLOCK_SIZE, std::ofstream::openmode mode = std::ofstream::trunc)
     *
     * \brief   Open a SnappyOutputStream for write with file path, mode,
     *          number of write buffers, and block size
     *
     * \param   path        Full pathname of the file.
     * \param   n_buffers   (Optional) Number of write buffers to use
     * \param   block_size  (Optional) Size of each snappy compressed block
     * \param   mode        (Optional) File open mode, same as ofstream::openmode
     */
    SnappyOutputStream(const std::string& path, size_t n_buffers = 1,
                       size_t block_size = DEFAULT_BLOCK_SIZE,
                       std::ofstream::openmode mode = std::ofstream::trunc) {
        Open(path, n_buffers, block_size, mode);
    }

    ///*
    //* \brief Open a SnappyOutputStream for write with file path, mode,
    //*        number of write buffers, and block size
    //*
    //* \param path:     Path to the file
    //* \param mode:     File open mode, same as ofstream::openmode
    //* \param buf_size: Size of the read buffer in bytes
    //*                  Setting the buf_size to 0 will disable buffering.
    //*                  The buffer size will be rounded to nearest multiple of
    //*                  buffer size;
    //*/
    // SnappyOutputStream(const std::string& path,
    //    std::ofstream::openmode mode = std::ofstream::trunc,
    //    size_t buf_size = 0) {
    //    Open(path, mode, buf_size);
    //}

    ~SnappyOutputStream() { Close(); }

    /*!
     * \fn  void SnappyOutputStream::Open(const std::string& path, size_t n_buffers, size_t
     * block_size, std::ofstream::openmode mode)
     *
     * \brief   Open a SnappyOutputStream for write with file path, mode,
     *          number of write buffers, and block size
     *
     * \param   path        Full pathname of the file.
     * \param   n_buffers   (Optional) Number of write buffers to use
     * \param   block_size  (Optional) Size of each snappy compressed block
     * \param   mode        (Optional) File open mode, same as ofstream::openmode
     */
    void Open(const std::string& path, size_t n_buffers, size_t block_size,
              std::ofstream::openmode mode) {
        Close();
        if (FilePath(path).Scheme() == FilePath::SchemeType::LOCAL) {
            file_ = new OutputLocalFileStream(path, 0, mode);
        } else {
            file_ = new OutputHdfsStream(path, 0, mode);
        }
        if (!file_->Good()) {
            is_open_ = false;
            FMA_ERR() << "error opening file " << path << " in snappy stream";
            return;
        }
        is_open_ = true;
        block_size_ = block_size;
        curr_buf_.buf.resize(block_size_);
        if (n_buffers <= 0) return;

        workspace_.resize(n_buffers);
        empty_buffers_.SetCapacity(n_buffers);
        zip_stage_ = new PipelineStage<int, int>([this](int d) -> int { return ZipStage(d); },
                                                 nullptr, 0, n_buffers);
        write_stage_ = new PipelineStage<int, int>([this](int d) -> int { return WriteStage(d); },
                                                   nullptr, 0, 1);
        zip_stage_->SetNextStage(write_stage_);
        write_stage_->SetNextStage(&empty_buffers_);
        for (size_t i = 0; i < n_buffers; i++) {
            empty_buffers_.Push(static_cast<int>(i));
        }
    }

    /*!
     * \fn  void SnappyOutputStream::Open(const std::string& path, size_t buf_size,
     * std::ofstream::openmode mode)
     *
     * \brief   Open a SnappyOutputStream for write with file path, mode,
     *          and buffer size, using default block size.
     *
     * \param   path        Full pathname of the file.
     * \param   buf_size    (Optional) Size of the write buffer in bytes
     *                       Setting the buf_size to 0 will disable buffering.
     * \param   mode        (Optional) std::ofstream::openmode, effective flags are:
     *                           ofstream::app, ofstream::trunc
     *                       Since HDFS files are append-only, we will assume trunc
     *                       by default, and use append only when app is specified.
     */
    void Open(const std::string& path, size_t buf_size = 0,
                      std::ofstream::openmode mode = std::ofstream::trunc) override {
        size_t n_buffers = (size_t)std::round((double)buf_size / DEFAULT_BLOCK_SIZE);
        Open(path, n_buffers, DEFAULT_BLOCK_SIZE, mode);
    }

    /*!
     * \fn  void SnappyOutputStream::Close()
     *
     * \brief   Closes this file.
     */
    void Close() {
        if (!is_open_) return;
        is_open_ = false;

        Flush();
        delete file_;
        if (workspace_.empty()) return;

        zip_stage_->WaitTillClear();
        delete zip_stage_;
        write_stage_->WaitTillClear();
        delete write_stage_;
        workspace_.clear();
    }

    /*!
     * \fn  void SnappyOutputStream::Flush()
     *
     * \brief   Flushes the write buffer.
     */
    void Flush() {
        PushCurrentBuffer();
        WaitWrite();
    }

    /*!
     * \fn  void SnappyOutputStream::Write(const void* buf, size_t size) override
     *
     * \brief   Append data to the file
     *
     * \param   buffer  Buffer which stores the data to be written
     * \param   size    Number of bytes to write
     */
    void Write(const void* buf, size_t size) override {
        CheckOpen();
        size_t bytes_copied = 0;
        while (bytes_copied < size) {
            size_t bytes_to_copy =
                std::min<size_t>(size - bytes_copied, block_size_ - curr_buf_.size);
            memcpy(&curr_buf_.buf[0] + curr_buf_.size, (char*)buf + bytes_copied, bytes_to_copy);
            curr_buf_.size += bytes_to_copy;
            bytes_copied += bytes_to_copy;
            if (curr_buf_.size >= block_size_) {
                PushCurrentBuffer();
            }
        }
    }

    /*!
     * \fn  bool SnappyOutputStream::IsOpen() const
     *
     * \brief   Query if this file is open.
     *
     * \return  True if open, false if not.
     */
    bool IsOpen() const { return is_open_; }

    /*!
     * \fn  bool SnappyOutputStream::Good() const override
     *
     * \brief   Whether this file is opened correctly
     *
     * \return  True if file is opened correctly, false otherwise
     */
    bool Good() const override { return file_->Good(); }

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    const std::string& Path() const override {
        return file_ ? file_->Path() : _detail::EMPTY_STRING();
    }

 private:
    void CheckOpen() {
        if (!is_open_) {
            throw std::runtime_error("Failed to open file " + file_->Path() + " for write.");
        }
    }

    void PushCurrentBuffer() {
        if (curr_buf_.size == 0) return;

        if (workspace_.empty()) {
            Zip(curr_buf_);
            WriteBlock(curr_buf_);
            curr_buf_.size = 0;
        } else {
            int i;
            if (!empty_buffers_.PopFront(i)) {
                FMA_ERR() << "Exiting before push finishes";
            }
            Workspace& buf = workspace_[i];
            curr_buf_.swap(buf);
            curr_buf_.buf.resize(block_size_);
            curr_buf_.size = 0;
            zip_stage_->Push(i);
        }
    }

    void Zip(Workspace& buf) {
        size_t l = snappy_max_compressed_length(buf.size);
        buf.zipped.resize(l);
        snappy_status st = snappy_compress(buf.buf.data(), buf.size, &buf.zipped[0], &l);
        FMA_CHECK_EQ(st, SNAPPY_OK) << "Failed to compress data";
        buf.zipped_size = l;
    }

    void WriteBlock(Workspace& buf) {
        SnappyBlockHeader header;
        header.orig_size_ = buf.size;
        header.compressed_size_ = buf.zipped_size;
        header.Store(*file_);
        file_->Write(buf.zipped.data(), buf.zipped_size);
        buf.size = 0;
    }

    int ZipStage(int d) {
        Workspace& w = workspace_[d];
        Zip(w);
        return d;
    }

    int WriteStage(int d) {
        Workspace& w = workspace_[d];
        WriteBlock(w);
        return d;
    }

    void WaitWrite() {
        zip_stage_->WaitTillClear();
        write_stage_->WaitTillClear();
    }
};
}  // namespace fma_common
#endif
