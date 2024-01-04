//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\text_dir_stream.h.
 *
 * \brief   Declares the hdfs text dir stream class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/18.
 */
#pragma once

#include <memory>

#include "fma-common/file_stream.h"
#include "fma-common/fma_stream.h"
#include "fma-common/pipeline.h"
#include "fma-common/string_util.h"
#include "fma-common/text_parser.h"

namespace fma_common {
/*!
 * \class   InputTextDirStream
 *
 * \brief   A text dir stream is a stream that read all the files as text
 *          in a HDFS dir. It can read multiple HDFS files at the same time
 *          to fully utilize the network bandwidth.
 *
 * \note    When reading multiple HDFS files in parallel, the lines in
 *          different HDFS files will be interleaved.
 */
class InputTextDirStream : public InputFileStream {
    struct FileRange {
        std::string path;
        size_t off;
        size_t size;

        FileRange(const std::string& p, size_t o, size_t s) : path(p), off(o), size(s) {}
    };

    std::string dir_;
    size_t block_size_ = 0;

    std::unique_ptr<PipelineStage<FileRange, void>> read_stage_;
    std::unique_ptr<BoundedQueue<std::string>> contents_;
    size_t total_files_ = 0;
    size_t total_size_ = 0;
    size_t total_bytes_read_ = 0;
    size_t files_read_ = 0;

    std::string curr_block_;
    size_t curr_block_offset_ = 0;

 public:
    DISABLE_COPY(InputTextDirStream);
    DISABLE_MOVE(InputTextDirStream);

    InputTextDirStream() {}

    ~InputTextDirStream() { Close(); }

    /**
     * Constructs a reader that will read part of the text files in the directory.
     *
     * \param   path            Full pathname of the directory. HDFS dir must start with hdfs://
     * \param   n_readers       Number of readers that will read the text.
     * \param   my_reader_id    Identifier for this reader.
     * \param   buf_size        (Optional) Size of the buffer.
     * \param   n_parallel      (Optional) Number of parallel files to read at the same time.
     *                          When n_parallel>1, lines in different text files will be mixed.
     */
    InputTextDirStream(const std::string& path, size_t n_readers, size_t my_reader_id,
                       size_t buf_size = 64 << 20, size_t n_parallel = 1) {
        Open(path, n_readers, my_reader_id, buf_size, n_parallel);
    }

    /*!
     * \fn  void InputTextDirStream::Open(const std::string& path, size_t buf_size) override
     *
     * \brief   Opens the given file.
     *
     * \param   path        Full pathname of the file.
     * \param   buf_size    Size of the buffer.
     */
    void Open(const std::string& path, size_t buf_size = 64 << 20) override {
        Open(path, 1, 0, buf_size, 1);
    }

    /**
     * Constructs a reader that will read part of the text files in the directory.
     *
     * \param   path            Full pathname of the directory. HDFS dir must start with hdfs://
     * \param   n_readers       Number of readers that will read the text.
     * \param   my_reader_id    Identifier for this reader.
     * \param   buf_size        (Optional) Size of the buffer.
     * \param   n_parallel      (Optional) Number of parallel files to read at the same time.
     *                          When n_parallel>1, lines in different text files will be mixed.
     */
    void Open(const std::string& path, size_t n_readers, size_t my_reader_id,
              size_t buf_size = 64 << 20, size_t n_parallel = 1) {
        if (n_parallel <= 0) n_parallel = 1;
        if (n_readers <= 0) n_readers = 1;
        FMA_ASSERT(my_reader_id >= 0);
        Close();
        dir_ = path;
#ifdef NDEBUG
        block_size_ = std::max<size_t>(64 << 10, buf_size / n_parallel);
#else
        block_size_ = buf_size / n_parallel;
#endif
        total_bytes_read_ = 0;
        curr_block_offset_ = 0;
        total_files_ = 0;
        total_size_ = 0;
        files_read_ = 0;

        std::vector<size_t> sizes;
        std::vector<std::string> files = GetFilePaths(dir_, &sizes);
        if (files.empty())
            throw std::runtime_error(
                std::string("Error opening text dir " + dir_ + " for read.").c_str());
        bool hdfs_small_file_opt = false;
#if !HAS_LIBHDFS
        {
            size_t total_size = 0;
            for (auto& s : sizes) total_size += s;
            hdfs_small_file_opt = FilePath(dir_).Scheme() == FilePath::HDFS && n_readers == 1 &&
                                  total_size / sizes.size() / 1024 / 1024 < 64;
        }
#endif
        read_stage_ = std::make_unique<PipelineStage<FileRange, void>>(
            [this](const FileRange& fr) { ReadFileIntoBuffer(fr); }, nullptr, 0, n_parallel);
        contents_ = std::make_unique<BoundedQueue<std::string>>(n_parallel);
        if (hdfs_small_file_opt) {
            // a lot of small files read by one single reader, using piped hdfs
            // we use "hdfs dfs -cat dir/*" to read the files
            for (auto& s : sizes) total_size_ += s;
            total_files_ = 1;
            if (files.size() == 1 && files[0] == dir_) {
                // dir_ is actually a single file
                read_stage_->Push(FileRange(dir_, 0, total_size_));
            } else {
                // dir_ is a directory
                read_stage_->Push(FileRange(dir_ + "/*", 0, total_size_));
            }
        } else {
            size_t file_per_reader = files.size() / n_readers;
            for (size_t i = 0; i < file_per_reader; i++) {
                size_t fid = i * n_readers + my_reader_id;
                read_stage_->Push(FileRange(files[fid], 0, sizes[fid]));
                total_size_ += sizes[fid];
            }
            total_files_ += file_per_reader;
            size_t left_over = files.size() % n_readers;
            // check if the file system supports seek
            bool support_seek = false;
            {
                InputFmaStream in(files[0]);
                try {
                    support_seek = in.Seek(0);
                } catch (std::exception&) {
                }
            }
            if (support_seek) {
                // remaining ones are read block by block
                size_t total_blocks = 0;
                std::vector<size_t> file_block_start;
                for (size_t i = 0; i < left_over; i++) {
                    size_t s = sizes[i + file_per_reader * n_readers];
                    size_t n_blocks = (s + block_size_ - 1) / (block_size_);
                    file_block_start.push_back(total_blocks);
                    total_blocks += n_blocks;
                }
                file_block_start.push_back(total_blocks);
                size_t block_per_reader = (total_blocks + n_readers - 1) / n_readers;
                size_t start_b = block_per_reader * my_reader_id;
                size_t start_f =
                    std::upper_bound(file_block_start.begin(), file_block_start.end(), start_b) -
                    file_block_start.begin() - 1;
                size_t end_b = block_per_reader * (my_reader_id + 1);
                auto it = std::lower_bound(file_block_start.begin(), file_block_start.end(), end_b);
                size_t end_f = (it == file_block_start.end()) ? file_block_start.size() - 1
                                                              : it - file_block_start.begin();
                for (size_t i = start_f; i < end_f; i++) {
                    size_t first_block_id = file_block_start[i];
                    size_t fsize = sizes[i + n_readers * file_per_reader];
                    size_t beg = (i == start_f) ? (start_b - first_block_id) * block_size_ : 0;
                    size_t end = (i == end_f - 1) ? (end_b - first_block_id) * block_size_
                                                  : file_block_start[i + 1] * block_size_;
                    size_t s = std::min(end, fsize) - std::min(beg, fsize);
                    if (s == 0) continue;
                    read_stage_->Push(FileRange(files[i + n_readers * file_per_reader], beg, s));
                    total_size_ += s;
                }
                total_files_ += (end_f - start_f);
            } else {
                // spread the left-over files to workers
                if (my_reader_id < left_over) {
                    size_t fid = n_readers * file_per_reader + my_reader_id;
                    read_stage_->Push(FileRange(files[fid], 0, sizes[fid]));
                    total_size_ += sizes[fid];
                    total_files_++;
                }
            }
        }
    }

    /*!
     * \fn  void InputTextDirStream::Close() override
     *
     * \brief   Closes this stream.
     */
    void Close() override {
        if (read_stage_) read_stage_->Stop();
        read_stage_.reset();
        if (contents_) contents_->EndQueue();
        contents_.reset();
        curr_block_.clear();
    }

    /*!
     * \fn  size_t InputTextDirStream::Read(void* buf, size_t size) override
     *
     * \brief   Reads.
     *
     * \param buf     The buffer to read into.
     * \param size    The size to read.
     *
     * \return  A size_t.
     */
    size_t Read(void* buf, size_t size) override {
        size_t bytes_read = 0;
        while (bytes_read < size) {
            if (curr_block_offset_ < curr_block_.size()) {
                size_t bytes_to_read =
                    std::min(curr_block_.size() - curr_block_offset_, size - bytes_read);
                memcpy((char*)buf + bytes_read, &curr_block_[curr_block_offset_], bytes_to_read);
                bytes_read += bytes_to_read;
                curr_block_offset_ += bytes_to_read;
            } else {
                if (files_read_ == total_files_) {
                    return bytes_read;
                }
                bool r = contents_->Pop(curr_block_);
                curr_block_offset_ = 0;
                if (!r) {
                    LOG_WARN() << "InputTextDirStream::Read() was interruppted. "
                                  "It seems the program was killed.";
                    break;
                }
                if (curr_block_.empty()) {
                    files_read_++;
                    if (files_read_ == total_files_) {
                        return bytes_read;
                    }
                }
            }
        }
        return bytes_read;
    }

    /*!
     * \fn  bool InputTextDirStream::Good() const override
     *
     * \brief   Check if the stream is properly opened.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Good() const override { return total_files_ != 0; }

    /*!
     * \fn  bool InputTextDirStream::Seek(size_t offset) override
     *
     * \brief   [Not Implemented!!] Seeks the given offset.
     *
     * \param   offset  The offset.
     *
     * \return  Always return false.
     */
    bool Seek(size_t offset) override {
        LOG_ERROR() << "InputTextDirStream::Seek() is not implemented.";
        return false;
    }

    /*!
     * \fn  size_t InputTextDirStream::Size() const override
     *
     * \brief   Gets the total size of the files in the directory.
     *
     * \return  Total size of all the files in the directory, in bytes.
     */
    size_t Size() const override { return total_size_; }

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    const std::string& Path() const override { return dir_; }

    size_t GetBytesRead() const { return total_bytes_read_; }

    size_t Offset() const override { return GetBytesRead(); }

 private:
    void ReadFileIntoBuffer(const FileRange& fr) {
        InputFmaStream in(fr.path, std::min(block_size_, fr.size));
        if (fr.off != 0) in.Seek(fr.off);
        std::string block;
        std::string left_over;
        size_t bytes_in_block = 0;
        size_t bytes_read = 0;
        bool eof_reached = false;
        bool partial_head = true;
        while (!eof_reached) {
            block.swap(left_over);
            left_over.clear();
            bytes_in_block = block.size();
            block.resize(bytes_in_block + block_size_);
            size_t r = in.Read(&block[bytes_in_block], std::min(block_size_, fr.size - bytes_read));
            block.resize(bytes_in_block + r);
            bytes_in_block = block.size();
            bytes_read += r;
            eof_reached = (r == 0 || bytes_read == fr.size);
            if (partial_head) {
                partial_head = false;
                if (fr.off != 0) {
                    // get rid of left-over from last reader
                    // if this is the first block in the file, then we don't need to do this
                    size_t p = block.find('\n');
                    if (p != block.npos) {
                        // if p==npos, then we don't have any complete line in this block
                        block.erase(block.begin(), block.begin() + p + 1);
                    } else {
                        block.clear();
                        partial_head = true;
                    }
                }
            }
            if (bytes_read == fr.size) {
                // last block, read till next line break
                if (!partial_head && r != 0) {
                    // If we are still reading a partial line which should be read by the last
                    // reader, we don't need to find the next line break since the last reader will
                    // do it. If we already read to the end of the file, we don't need to read
                    // further now read till the next line
                    while (true) {
                        bytes_in_block = block.size();
                        block.resize(bytes_in_block + 1024);
                        r = in.Read(&block[bytes_in_block], 1024);
                        block.resize(bytes_in_block + r);
                        if (r == 0) break;  // already at the end of file
                        size_t p = block.find('\n', bytes_in_block);
                        if (p != block.npos) {
                            block.resize(p);
                            break;
                        }
                    }
                }
            } else {
                // middle block, ok to leave some bytes in the buffer
                size_t pos = block.rfind('\n');
                if (pos != block.npos) {
                    left_over.assign(block.begin() + pos + 1, block.end());
                    block.resize(pos + 1);
                } else {
                    left_over.swap(block);
                    block.clear();
                }
            }
            if (!block.empty()) {
                total_bytes_read_ += block.size();
                if (block.back() != '\n') block.push_back('\n');
                contents_->Push(std::move(block));
            }
        }
        // push an empty block so the reader won't block
        contents_->Push(std::string());
    }

    /*!
     * \fn  static std::vector<std::string> InputTextDirStream::GetFilePaths(const std::string& dir,
     * size_t total_size)
     *
     * \brief   Gets the files directly under the directory.
     *
     * \param          dir         The dir.
     * \param   [out]  f_sizes     If not null, returns the sizes of the files.
     *
     * \return  The file paths.
     */
    static std::vector<std::string> GetFilePaths(const std::string& dir,
                                                 std::vector<size_t>* total_size) {
        return FileSystem::GetFileSystem(dir).ListFiles(dir, total_size);
    }
};
}  // namespace fma_common
