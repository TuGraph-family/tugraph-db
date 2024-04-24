//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once

#include <stdio.h>
#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

#include "tools/lgraph_log.h"
#include "fma-common/file_stream.h"
#include "fma-common/string_util.h"

namespace fma_common {
class InputPipeStream : public InputFileStream {
    FILE* pipe_;
    std::string cmd_;
    size_t read_bytes_ = 0;

 public:
    /*!
     * \fn    InputPipeStream::InputPipeStream(const std::string& cmd)
     *
     * \brief    Executes the command and pipes the output.
     *
     * \param    cmd            Command line to be executed.
     */
    explicit InputPipeStream(const std::string& cmd) { Open(cmd); }

    ~InputPipeStream() { Close(); }

    /*!
     * \fn    virtual void InputPipeStream::Open(const std::string& cmd, size_t buf_size = 0)
     * override
     *
     * \brief    Executes the command and pipes the output.
     *
     * \param    cmd            Command line to be executed.
     * \param    buf_size    (Optional) Buffer size, ignored.
     */
    void Open(const std::string& cmd, size_t buf_size = 0) override {
        cmd_ = cmd;
        pipe_ = _detail::OpenPipe(cmd, "r");
        read_bytes_ = 0;
    }

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
    size_t Read(void* buf, size_t size) override {
        FMA_ASSERT(pipe_ != nullptr) << "Error reading from pipe, file not opened properly."
                                     << " Command of pipe is: " << cmd_;
        size_t s = fread(buf, 1, size, pipe_);
        read_bytes_ += s;
        return s;
    }

    /*!
     * \fn    bool InputPipeStream::Good() const override
     *
     * \brief    Whether this file is opened correctly.
     *
     * \return    True if the file is opened correctly.
     */
    bool Good() const override { return pipe_ != nullptr; }

    /*!
     * \fn    void InputPipeStream::Close() override
     *
     * \brief    Closes this file.
     */
    void Close() override {
        if (pipe_) {
            int r = pclose(pipe_);
            if (r != 0) {
                LOG_ERROR() << "Error closing pipe for command " << cmd_
                            << ": pclose returned " << r;
            }
            pipe_ = nullptr;
            cmd_.clear();
            read_bytes_ = 0;
        }
    }

    /*!
     * \fn    bool InputPipeStream::Seek(size_t offset) override
     *
     * \brief    (NOT IMPLEMENTED!!) Seeks the given offset.
     *
     * \param    offset    The offset.
     *
     * \return    Always false.
     */
    bool Seek(size_t offset) override {
        LOG_ERROR() << "Seek() is not implemented for InputPipeStream.";
        return false;
    }

    /*!
     * \fn  virtual size_t InputFileStream::Size() const = 0;
     *
     * \brief   (NOT IMPLEMENTED!!) Gets the size of the file
     *
     * \return  Always 0
     */
    size_t Size() const override { return 0; }

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    const std::string& Path() const override { return cmd_; }

    size_t Offset() const override { return read_bytes_; }
};
}  // namespace fma_common
