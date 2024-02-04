//  Copyright 2024 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\multi_disk_stream.h.
 *
 * \brief   Declares the multi disk stream class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/15.
 */
#pragma once

#include <algorithm>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "fma-common/fma_stream.h"
#include "fma-common/file_stream.h"
#include "fma-common/string_util.h"
#include "fma-common/type_traits.h"
#include "fma-common/utils.h"

namespace fma_common {
const size_t _MULTI_DISK_FILE_CHUNK_SIZE_ = 1024 * 256;

/*!
 * \class   InputMultiDiskStream
 *
 * \brief   An InputMultiDiskStream describes a read-only file that is splitted across
 *          multiple files on different hard drives. This is to accelerate the read
 *          speed when we have multiple disks but don't have a RAID.
 *
 *          The stream is implemented with a meta file, which describes the different
 *          data parts of the file.
 */
class InputMultiDiskStream : public InputFileStream {
    static const size_t DEFAULT_BUFFER_SIZE = 64 * 1024 * 1024;

    /*! \brief   Full pathname of the meta file */
    std::string meta_file_path_;
    /*! \brief   The chunk files */
    std::vector<InputFmaStream> files_;
    size_t off_ = 0;
    size_t size_ = 0;

 public:
    DISABLE_COPY(InputMultiDiskStream);
    InputMultiDiskStream() {}

    /*!
     * \fn  InputMultiDiskStream::InputMultiDiskStream(const std::string& paths, size_t buffer_size
     * = DEFAULT_BUFFER_SIZE)
     *
     * \brief   Open a multi-disk stream.
     *
     * \param   paths       Path of meta_file which contains the path to different data parts
     * \param   buffer_size (Optional) Buffer size to use for this stream. It is the
     *                      total amount of memory this stream will use.
     */
    explicit InputMultiDiskStream(const std::string& paths,
                                  size_t buffer_size = DEFAULT_BUFFER_SIZE) {
        Open(paths, buffer_size);
    }

    InputMultiDiskStream(InputMultiDiskStream&&) = default;

    virtual ~InputMultiDiskStream() { Close(); }

    /*!
     * \fn  virtual void InputMultiDiskStream::Open(const std::string& meta_file, size_t buffer_size
     * = DEFAULT_BUFFER_SIZE) override
     *
     * \brief   Opens.
     *
     * \param   meta_file   Path of meta_file which contains the path to different data parts
     * \param   buffer_size (Optional) Buffer size to use for this stream. It is the
     *                      total amount of memory this stream will use.
     */
    void Open(const std::string& meta_file, size_t buffer_size = DEFAULT_BUFFER_SIZE) override {
        meta_file_path_ = meta_file;
        InputFmaStream meta_stream(meta_file);
        std::vector<std::string> parts = StreamLineReader(meta_stream).ReadAllLines();
        FMA_ASSERT(!parts.empty())
            << "Multi-disk file " << meta_file << " is corruppted: meta file empty";
        size_t buffer_per_part = buffer_size / parts.size();
        for (auto& p : parts) {
            files_.emplace_back(p, buffer_per_part);
            if (!files_.back().Good()) {
                files_.clear();
                FMA_ASSERT(false) << "Corruppted data part: " << p
                                  << " while opening multi-disk file " << meta_file;
                break;
            }
            size_ += files_.back().Size();
        }
        off_ = 0;
    }

    /*!
     * \fn  void InputMultiDiskStream::Close() override
     *
     * \brief   Closes this file.
     */
    void Close() override {
        off_ = 0;
        size_ = 0;
        files_.clear();
        meta_file_path_.clear();
    }

    /*!
     * \fn  size_t InputMultiDiskStream::Read(void* buf, size_t size) override
     *
     * \brief   Read a block of data into the buffer
     *
     * \param   buf     Buffer used to receive the data
     * \param   size    Number of bytes to read
     *
     * \return  Number of bytes actually read
     */
    size_t Read(void* buf, size_t size) override {
        size_t bytes_read = 0;
        while (bytes_read < size) {
            size_t part = (off_ / _MULTI_DISK_FILE_CHUNK_SIZE_) % files_.size();
            size_t to_read = std::min(size - bytes_read, _MULTI_DISK_FILE_CHUNK_SIZE_ -
                                                             off_ % _MULTI_DISK_FILE_CHUNK_SIZE_);
            size_t read = files_[part].Read((char*)buf + bytes_read, to_read);
            bytes_read += read;
            off_ += read;
            if (read < to_read && off_ < size_) {
                LOG_ERROR() << "Error reading file " << meta_file_path_ << ": part " << part
                          << " is corrupted.";
                break;
            }
        }
        return bytes_read;
    }

    /*!
     * \fn  bool InputMultiDiskStream::Good() const override
     *
     * \brief   Whether this file is opened correctly
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Good() const override { return !files_.empty() && off_ < size_; }

    /*!
     * \fn  bool InputMultiDiskStream::GoodForRead()
     *
     * \brief   Determines if we can read more bytes from the file.
     *
     * \return  True if there are more bytes to read, false otherwise.
     */
    bool GoodForRead() { return Good(); }

    /*!
     * \fn  uint64_t InputMultiDiskStream::Size() const override
     *
     * \brief   Gets the size of the file
     *
     * \return  Size of the file
     */
    uint64_t Size() const override {
        CheckOpen();
        return size_;
    }

    /*!
     * \fn  bool InputMultiDiskStream::IsOpen() const
     *
     * \brief   Query if this file is open.
     *
     * \return  True if open, false if not.
     */
    bool IsOpen() const { return !files_.empty(); }

    /*!
     * \fn  bool InputMultiDiskStream::Seek(size_t offset) override
     *
     * \brief   Seeks to a position in the file
     *
     * \param   offset  Offset from the beginning of the file
     *
     * \return  true if operation is successful, otherwise false
     */
    bool Seek(size_t offset) override {
        CheckOpen();
        if (offset <= size_) {
            off_ = offset;
            size_t n_blocks = off_ / _MULTI_DISK_FILE_CHUNK_SIZE_;
            size_t segmented_part = n_blocks % files_.size();
            size_t blocks_per_part = n_blocks / files_.size();
            for (size_t i = 0; i < files_.size(); i++) {
                size_t seek_off = blocks_per_part * _MULTI_DISK_FILE_CHUNK_SIZE_;
                if (i < segmented_part) {
                    seek_off += _MULTI_DISK_FILE_CHUNK_SIZE_;
                } else if (i == segmented_part) {
                    seek_off += off_ % _MULTI_DISK_FILE_CHUNK_SIZE_;
                }
                bool r = files_[i].Seek(seek_off);
                if (!r) return false;
            }
            return true;
        }
        return false;
    }

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    const std::string& Path() const override { return meta_file_path_; }

    size_t Offset() const override { return off_; }

 private:
    void CheckOpen() const {
        if (files_.empty()) {
            throw std::runtime_error("Failed to open multi-disk file " + meta_file_path_ +
                                     " for write.");
        }
    }
};

/*!
 * \class   OutputMultiDiskStream
 *
 * \brief   An OutputMultiDiskStream describes a write-only file that is splitted across
 *          multiple files on different hard drives. This is to accelerate the write
 *          speed when we have multiple disks but don't have a RAID.
 *
 *          The stream is implemented with a meta file, which describes the different
 *          data parts of the file.
 */
class OutputMultiDiskStream : public OutputFileStream {
    static const size_t DEFAULT_BUFFER_SIZE = 64 * 1024 * 1024;

    /*! \brief   Full pathname of the meta file */
    std::string meta_file_path_;
    /*! \brief   The chunk files */
    std::vector<OutputFmaStream> files_;
    size_t size_;
    FileSystem* fs_ = nullptr;

 public:
    DISABLE_COPY(OutputMultiDiskStream);
    OutputMultiDiskStream() {}

    /*!
     * \fn  OutputMultiDiskStream::OutputMultiDiskStream(const std::string& meta_file, uint64_t
     * buffer_size = DEFAULT_BUFFER_SIZE, std::ofstream::openmode mode = std::ofstream::app)
     *
     * \brief   Open an existing multi-disk stream for append
     *
     * \param   meta_file   The meta_file which contains the path to different
     *                      data parts
     * \param   buffer_size (Optional) buffer size to use for this stream. It is the
     *                      total amount of memory this stream will use.
     * \param   mode        (Optional) std::ofstream::openmode, effective flags are:
     *                      ofstream::app, ofstream::trunc
     */
    OutputMultiDiskStream(const std::string& meta_file, uint64_t buffer_size = DEFAULT_BUFFER_SIZE,
                          std::ofstream::openmode mode = std::ofstream::app) {
        fs_ = &(FileSystem::GetFileSystem(meta_file));
        if (!fs_->FileExists(meta_file)) {
            LOG_ERROR() << "OutputMultiDiskStream can only be created with "
                      << "Open(meta_file, data_parts, buffer_size)";
        }
        Open(meta_file, buffer_size, mode);
    }

    /*!
     * \fn  OutputMultiDiskStream::OutputMultiDiskStream(const std::string& meta_file, const
     * std::vector<std::string>& data_part_files, uint64_t buffer_size = DEFAULT_BUFFER_SIZE,
     * std::ofstream::openmode mode = std::ofstream::trunc)
     *
     * \brief   If data_part_files is empty, open an existing file. Otherwise, create
     *          a new multi-disk-file with the data parts.
     *
     * \param   meta_file       The meta_file which will contains the path to different
     *                          data parts
     * \param   data_part_files Paths to the data part files.
     * \param   buffer_size     (Optional) buffer size to use for this stream. It is the
     *                          total amount of memory this stream will use.
     * \param   mode            (Optional) std::ofstream::openmode, effective flags are:
     *                          ofstream::app, ofstream::trunc
     */
    OutputMultiDiskStream(const std::string& meta_file,
                          const std::vector<std::string>& data_part_files,
                          uint64_t buffer_size = DEFAULT_BUFFER_SIZE,
                          std::ofstream::openmode mode = std::ofstream::trunc) {
        Open(meta_file, data_part_files, buffer_size, mode);
    }

    virtual ~OutputMultiDiskStream() { Close(); }

    /*!
     * \fn  virtual void OutputMultiDiskStream::Open(const std::string& meta_file, uint64_t
     * buffer_size = DEFAULT_BUFFER_SIZE, std::ofstream::openmode mode = std::ofstream::app)
     * override
     *
     * \brief   Open an existing multi-disk stream.
     *
     * \param   meta_file   meta_file which contains the path to different data parts
     * \param   buffer_size (Optional) buffer size to use for this stream. It is the total amout
     *                       of memory this stream will use.
     * \param   mode        (Optional) std::ofstream::openmode, effective flags are:
     *                      ofstream::app, ofstream::trunc
     */
    void Open(const std::string& meta_file, uint64_t buffer_size = DEFAULT_BUFFER_SIZE,
              std::ofstream::openmode mode = std::ofstream::app) override {
        fs_ = &(FileSystem::GetFileSystem(meta_file));
        if (!fs_->FileExists(meta_file)) {
            LOG_ERROR() << "OutputMultiDiskStream can only be created with "
                      << "Open(meta_file, data_parts, buffer_size)";
        }
        Open(meta_file, std::vector<std::string>(), buffer_size, mode);
    }

    /*!
     * \fn  virtual void OutputMultiDiskStream::Open(const std::string& meta_file, const
     * std::vector<std::string>& data_part_files, uint64_t buffer_size = DEFAULT_BUFFER_SIZE,
     * std::ofstream::openmode mode = std::ofstream::trunc)
     *
     * \brief   If data_part_files are empty, then open an existing file. Otherwise,
     *          create a new multi-disk-file using the data part files.
     *
     * \param   meta_file       Meta_file which contains the path to different data parts
     * \param   data_part_files Data parts of this file, must be placed on different disks
     * \param   buffer_size     (Optional) Buffer size to use for this stream. It is the
     *                          total amout of memory this stream will use.
     * \param   mode            (Optional) std::ofstream::openmode, effective flags are:
     *                          ofstream::app, ofstream::trunc
     */
    virtual void Open(const std::string& meta_file, const std::vector<std::string>& data_part_files,
                      uint64_t buffer_size = DEFAULT_BUFFER_SIZE,
                      std::ofstream::openmode mode = std::ofstream::trunc) {
        if (IsOpen()) {
            Close();
        }
        meta_file_path_ = meta_file;
        size_ = 0;
        fs_ = &(FileSystem::GetFileSystem(meta_file));
        if (data_part_files.empty()) {
            // opening an existing file for write
            FMA_ASSERT(fs_->FileExists(meta_file))
                << "Error opening multi-disk file " << meta_file << ": "
                << "Meta-file does not exist and data parts are not specified.";
            InputFmaStream meta_stream(meta_file);
            std::vector<std::string> parts = StreamLineReader(meta_stream).ReadAllLines();
            FMA_ASSERT(!parts.empty())
                << "Multi-disk file " << meta_file << " is corruppted: meta file empty";
            size_t buffer_per_part = buffer_size / parts.size();
            for (auto& p : parts) {
                files_.emplace_back(p, buffer_per_part, mode);
                if (!files_.back().Good()) {
                    LOG_ERROR() << "Error opening part file " << p << " of multi-disk file "
                              << meta_file;
                    Close();
                    return;
                }
                if (mode & std::ofstream::app) {
                    size_ += files_.back().Size();
                }
            }
        } else {
            // opening a new file
            if (fs_->FileExists(meta_file)) {
                LOG_WARN() << "Overwritting existing multi-disk file " << meta_file;
                InputFmaStream meta_stream(meta_file);
                std::vector<std::string> parts = StreamLineReader(meta_stream).ReadAllLines();
                for (auto& p : parts) {
                    fs_->Remove(p);
                }
            }
            std::ofstream meta(meta_file, std::ofstream::trunc);
            size_t buffer_per_part = buffer_size / data_part_files.size();
            for (auto& p : data_part_files) {
                meta << p << std::endl;
                files_.emplace_back(p, buffer_per_part, mode);
                if (!files_.back().Good()) {
                    LOG_ERROR() << "Error opening part file " << p << " of multi-disk file "
                              << meta_file;
                    Close();
                    return;
                }
            }
            meta.close();
        }
    }

    /*!
     * \fn  void OutputMultiDiskStream::Close() override
     *
     * \brief   Closes this file.
     */
    void Close() override {
        meta_file_path_.clear();
        size_ = 0;
        files_.clear();
    }

    /*!
     * \fn  bool OutputMultiDiskStream::IsOpen() const
     *
     * \brief   Query if this file is open.
     *
     * \return  True if open, false if not.
     */
    bool IsOpen() const { return !files_.empty(); }

    /*!
     * \fn  size_t OutputMultiDiskStream::Size() const
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
     * \fn  void OutputMultiDiskStream::Write(const void* buffer, size_t size) override
     *
     * \brief   Append data to the file
     *
     * \param   buffer  Buffer which stores the data to be written
     * \param   size    Number of bytes to write
     */
    void Write(const void* buffer, size_t size) override {
        const char* cp = (char*)buffer;
        size_t written = 0;
        while (written < size) {
            size_t part = (size_ / _MULTI_DISK_FILE_CHUNK_SIZE_) % files_.size();
            size_t part_off = size_ % _MULTI_DISK_FILE_CHUNK_SIZE_;
            size_t s = std::min(size - written, _MULTI_DISK_FILE_CHUNK_SIZE_ - part_off);
            files_[part].Write(cp + written, s);
            size_ += s;
            written += s;
        }
    }

    /*!
     * \fn  bool OutputMultiDiskStream::Good() const override
     *
     * \brief   Whether this file is opened correctly
     *
     * \return  True if file is opened correctly, false otherwise
     */
    bool Good() const override { return !files_.empty(); }

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    const std::string& Path() const override { return meta_file_path_; }

 private:
    void CheckOpen() const {
        if (!IsOpen()) {
            throw std::runtime_error("Failed to open multi-disk file " + meta_file_path_ +
                                     " for write.");
        }
    }
};
}  // namespace fma_common
