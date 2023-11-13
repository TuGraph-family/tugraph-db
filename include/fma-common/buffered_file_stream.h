//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once

#include "fma-common/file_stream.h"
#include "fma-common/stream_buffer.h"
#include "fma-common/type_traits.h"

namespace fma_common {
template <typename StreamT, typename BufferT>
class InputBufferedFileStream : public InputFileStream {
    StreamT stream_;
    BufferT buffer_;
    size_t buf_size_ = 0;

 public:
    DISABLE_COPY(InputBufferedFileStream);
    DISABLE_MOVE(InputBufferedFileStream);

    virtual ~InputBufferedFileStream() { Close(); }

    InputBufferedFileStream() {}

    InputBufferedFileStream(const std::string& path, size_t buf_size = 64 << 20) {
        Open(path, buf_size);
    }

    /*!
     * \fn  virtual void InputFileStream::Open(const std::string& path, size_t buf_size) = 0;
     *
     * \brief   Opens an input stream
     *
     * \param   path        path of the file, or URI of a HDFS file
     * \param   buf_size    Size of the read buffer in bytes
     */
    void Open(const std::string& path, size_t buf_size = 64 << 20) override {
        stream_.Open(path, 0);
        if (stream_.Good()) {
            buffer_.Open(&stream_, std::min<size_t>(buf_size, stream_.Size()));
        }
        buf_size_ = buf_size;
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
    size_t Read(void* buf, size_t size) override { return buffer_.Read(buf, size); }

    /*!
     * \fn  virtual bool InputFileStream::Good() const = 0;
     *
     * \brief   Whether this file is opened correctly
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Good() const override { return buffer_.Good(); }

    /*!
     * \fn  virtual void InputFileStream::Close() = 0;
     *
     * \brief   Closes this file.
     */
    void Close() override {
        buffer_.Close();
        stream_.Close();
        buf_size_ = 0;
    }

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
     *
     * \note    This operation is costly because it requires flushing the buffer
     *       and then restarting prefetch. If you need to call this frequently,
     *       you shoud use unbuffered stream.
     */
    bool Seek(size_t offset) override {
        buffer_.Close();
        if (!stream_.Seek(offset)) return false;
        buffer_.Open(&stream_, buf_size_);
        return true;
    }

    /*!
     * \fn  virtual size_t InputFileStream::Size() const = 0;
     *
     * \brief   Gets the size of the file
     *
     * \return  Size of the file
     */
    size_t Size() const override { return stream_.Size(); }

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    const std::string& Path() const override { return stream_.Path(); }

    size_t Offset() const override { return buffer_.Offset(); }
};

template <typename StreamT, typename BufferT>
class OutputBufferedFileStream : public OutputFileStream {
    std::unique_ptr<StreamT> stream_;
    std::unique_ptr<BufferT> buffer_;
    size_t size_ = 0;

 public:
    DISABLE_COPY(OutputBufferedFileStream);

    virtual ~OutputBufferedFileStream() { Close(); }

    OutputBufferedFileStream() : stream_(new StreamT()), buffer_(new BufferT()) {}

    OutputBufferedFileStream(const std::string& path, size_t buf_size = 64 << 20,
                             std::ofstream::openmode mode = std::ofstream::trunc)
        : stream_(new StreamT()), buffer_(new BufferT()) {
        Open(path, buf_size, mode);
    }

    OutputBufferedFileStream(OutputBufferedFileStream&& rhs)
        : stream_(std::move(rhs.stream_)), buffer_(std::move(rhs.buffer_)), size_(rhs.size_) {}

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
    virtual void Open(const std::string& path, size_t buf_size,
                      std::ofstream::openmode mode) override {
        stream_->Open(path, 0, mode);
        if (stream_->Good()) buffer_->Open(stream_.get(), buf_size);
        size_ = 0;
    }

    template <typename... StreamParams, typename... BufferParams>
    void Open(const std::string& path, size_t buf_size, std::ofstream::openmode mode,
              const std::tuple<StreamParams...>& sp, const std::tuple<BufferParams...>& bp) {
        _detail::ApplyTuple(
            [&](StreamParams... params) { stream_->Open(path, 0, mode, params...); }, sp);
        if (stream_->Good()) {
            _detail::ApplyTuple(
                [&](BufferParams... params) { buffer_->Open(stream_.get(), buf_size, params...); },
                bp);
        }
    }

    /*!
     * \fn  virtual void OutputFileStream::Write(const void*, size_t) = 0;
     *
     * \brief   Append data to the file
     *
     * \param   buffer  Buffer which stores the data to be written
     * \param   size    Number of bytes to write
     */
    void Write(const void* buffer, size_t size) override {
        buffer_->Write(buffer, size);
        size_ += size;
    }

    /*!
     * \fn  virtual bool OutputFileStream::Good() const = 0;
     *
     * \brief   Whether this file is opened correctly
     *
     * \return  True if file is opened correctly, false otherwise
     */
    bool Good() const override { return stream_ && stream_->Good(); }

    void Flush() override {
        if (buffer_) buffer_->Flush();
        if (stream_) stream_->Flush();
    }

    /*!
     * \fn  virtual void OutputFileStream::Close() = 0;
     *
     * \brief   Closes this file.
     */
    void Close() override {
        if (buffer_) buffer_->Close();
        if (stream_) stream_->Close();
        size_ = 0;
    }

    /*!
     * \fn    virtual size_t OutputFileStream::Size() = 0;
     *
     * \brief    Gets the size of this file.
     *
     * \return    A size_t.
     */
    size_t Size() const override { return size_; }

    /*!
     * \brief    Gets the path of the file by returning the path parameter
     *           passed to Open() or the constructor.
     *
     * \return    Path as a string.
     */
    const std::string& Path() const override { return stream_->Path(); }
};
}  // namespace fma_common
