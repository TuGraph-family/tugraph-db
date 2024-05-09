//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once

#include <algorithm>
#include <memory>

#include "fma-common/pipeline.h"
#include "fma-common/stream_base.h"
#include "fma-common/type_traits.h"

namespace fma_common {
class PrefetchingtStreamBuffer : public InputStreamBase {
    InputStreamBase* stream_;
    std::string buf_;
    size_t capacity_ = 0;
    size_t offset_ = 0;
    size_t size_ = 0;
    size_t file_pos_ = 0;  // gpos w.r.t the file begin
    bool good_ = false;
    std::unique_ptr<PipelineStage<int, size_t>> prefetcher_;
    std::unique_ptr<BoundedQueue<size_t>> prefetched_;
    std::string prefetch_buf_;

 public:
    DISABLE_COPY(PrefetchingtStreamBuffer);
    DISABLE_MOVE(PrefetchingtStreamBuffer);
    PrefetchingtStreamBuffer() {}

    PrefetchingtStreamBuffer(InputStreamBase* stream, size_t buf_size) { Open(stream, buf_size); }

    ~PrefetchingtStreamBuffer() { Close(); }

    void Close() {
        capacity_ = 0;
        offset_ = 0;
        size_ = 0;
        file_pos_ = 0;
        prefetcher_.reset();
        prefetched_.reset();
        prefetch_buf_.clear();
    }

    void Open(InputStreamBase* stream, size_t buf_size) {
        Close();
        stream_ = stream;
        if (!stream->Good()) {
            good_ = false;
            return;
        }
        buf_size = std::max<size_t>(buf_size / 2, 64 << 10);
        capacity_ = buf_size;
        offset_ = 0;
        size_ = 0;
        file_pos_ = 0;
        buf_.resize(capacity_);
        prefetch_buf_.resize(capacity_);
        prefetched_ = std::make_unique<BoundedQueue<size_t>>();
        prefetcher_ = std::make_unique<PipelineStage<int, size_t>>(
            [=](int d) -> size_t { return PrefetchBlock(d, stream_, prefetch_buf_, capacity_); },
            nullptr, 1, 1, 1);
        prefetcher_->SetNextStage(prefetched_.get());
        prefetcher_->Push(0);
        good_ = true;
    }

    size_t Read(void* buf, size_t size) {
        if (!stream_) return 0;
        size_t bytes_left = size;
        size_t bytes_read = 0;
        // fill buffer and then read from buffer
        while (bytes_left != 0) {
            if (offset_ >= size_) {
                size_t d = 0;
                if (prefetched_->Pop(d)) {
                    if (d != 0) {
                        buf_.swap(prefetch_buf_);
                    } else {
                        prefetcher_->Push(-1);
                        good_ = false;
                        return bytes_read;
                    }
                } else {
                    return bytes_read;
                }
                size_ = d;
                offset_ = 0;
                if (bytes_left > capacity_) {
                    // large block, read prefetched data and then read directly
                    memcpy((char*)buf + bytes_read, &buf_[0], size_);
                    offset_ = size_;
                    bytes_left -= size_;
                    bytes_read += size_;
                    // read the remaining bytes directly
                    size_t b = stream_->Read((char*)buf + bytes_read, bytes_left);
                    bytes_read += b;
                    bytes_left = 0;
                }
                prefetcher_->Push(d == capacity_ ? 0 : -1);
                continue;
            }
            size_t to_copy = std::min(bytes_left, size_ - offset_);
            memcpy((char*)buf + bytes_read, &buf_[offset_], to_copy);
            offset_ += to_copy;
            bytes_left -= to_copy;
            bytes_read += to_copy;
        }
        file_pos_ += bytes_read;
        return bytes_read;
    }

    size_t Offset() const override { return file_pos_; }

    bool Good() const { return good_; }

 protected:
    size_t FillBuffer() {
        size_t d = 0;
        bool r = prefetched_->Pop(d);
        if (r && d != 0) {
            buf_.swap(prefetch_buf_);
        }
        prefetcher_->Push(0);
        size_ = d;
        offset_ = 0;
        return d;
    }

    static size_t PrefetchBlock(int d, InputStreamBase* stream, std::string& buf, size_t capacity) {
        if (d == -1) return 0;
        size_t s = stream->Read(&buf[0], capacity);
        return s;
    }
};

class ThreadedOutputStreamBuffer {
    OutputStreamBase* stream_ = nullptr;
    std::string buf_;
    size_t capacity_ = 0;
    size_t size_ = 0;
    std::string writing_buf_;
    std::unique_ptr<PipelineStage<size_t, size_t>> writer_;
    std::unique_ptr<BoundedQueue<size_t>> write_token_;

 public:
    DISABLE_COPY(ThreadedOutputStreamBuffer);
    DISABLE_MOVE(ThreadedOutputStreamBuffer);

    ThreadedOutputStreamBuffer() {}

    ThreadedOutputStreamBuffer(OutputStreamBase* stream, size_t buf_size) {
        Open(stream, buf_size);
    }

    ~ThreadedOutputStreamBuffer() { Close(); }

    void Open(OutputStreamBase* stream, size_t buf_size) {
        Close();
        buf_size = std::max<size_t>(64 << 10, buf_size / 2);
        stream_ = stream;
        capacity_ = buf_size;
        size_ = 0;
        buf_.resize(capacity_);
        writing_buf_.resize(capacity_);
        writer_ = std::make_unique<PipelineStage<size_t, size_t>>(
            [this](size_t s) -> size_t { return WriteBufferToFile(s); }, nullptr, 0, 1, 1);
        write_token_ = std::make_unique<BoundedQueue<size_t>>();
        write_token_->Push(capacity_);
        writer_->SetNextStage(write_token_.get());
    }

    void Close() {
        if (!stream_) return;
        Flush();
        writer_.reset();
        write_token_.reset();
        capacity_ = 0;
    }

    void Write(const void* buf, size_t size) {
        if (size_ + size >= capacity_) {
            PushToWrite();
        }
        if (size >= capacity_) {
            // large block, wait for writer thread and then write directly
            if (writer_) writer_->WaitTillClear();
            stream_->Write(buf, size);
            return;
        }
        memcpy(&buf_[size_], buf, size);
        size_ += size;
    }

    void Flush() {
        PushToWrite();
        if (writer_) writer_->WaitTillClear();
    }

 private:
    void PushToWrite() {
        if (size_) {
            size_t d = 0;
            if (!write_token_->Pop(d) && d > 0) return;
            buf_.swap(writing_buf_);
            writer_->Push(size_);
            size_ = 0;
        }
    }

    size_t WriteBufferToFile(size_t size) {
        if (size == 0) return size;
        stream_->Write(&writing_buf_[0], size);
        return size;
    }
};

class ThreadPoolOutputStreamBuffer {
    ThreadPool* pool_ = nullptr;
    OutputStreamBase* stream_ = nullptr;
    std::string buf_;
    size_t capacity_ = 0;
    size_t size_ = 0;
    std::string writing_buf_;

    std::atomic<bool> writing_;
    std::mutex mu_;
    std::condition_variable cv_;

 public:
    DISABLE_COPY(ThreadPoolOutputStreamBuffer);
    DISABLE_MOVE(ThreadPoolOutputStreamBuffer);

    ThreadPoolOutputStreamBuffer() : writing_(false) {}

    ThreadPoolOutputStreamBuffer(OutputStreamBase* stream, size_t buf_size) : writing_(false) {
        Open(stream, buf_size);
    }

    ~ThreadPoolOutputStreamBuffer() { Close(); }

    void Open(OutputStreamBase* stream, size_t buf_size, ThreadPool* p = nullptr) {
        Close();
        if (p == nullptr)
            throw std::runtime_error(
                "ThreadPool must be specified for ThreadPoolOutputStreamBuffer.");
        pool_ = p;
        stream_ = stream;
        capacity_ = buf_size;
        size_ = 0;
        buf_.resize(capacity_);
        writing_buf_.resize(capacity_);
    }

    void Close() {
        if (!stream_) return;
        Flush();
        capacity_ = 0;
    }

    void Write(const void* buf, size_t size) {
        if (size_ + size >= capacity_) {
            PushToWrite();
        }
        if (size >= capacity_) {
            // large block, wait for existing write and then write directly
            WaitTillWriteDone();
            stream_->Write(buf, size);
        } else {
            memcpy(&buf_[size_], buf, size);
            size_ += size;
        }
    }

    void Flush() {
        PushToWrite();
        WaitTillWriteDone();
    }

 private:
    void WaitTillWriteDone() {
        std::unique_lock<std::mutex> l(mu_);
        while (writing_) cv_.wait(l);
    }

    void PushToWrite() {
        if (size_) {
            WaitTillWriteDone();
            buf_.swap(writing_buf_);
            size_t size = size_;
            size_ = 0;
            {
                std::lock_guard<std::mutex> l(mu_);
                writing_ = true;
            }
            pool_->PushTask(0, 0, [this, size]() {
                stream_->Write(writing_buf_.data(), size);
                std::lock_guard<std::mutex> l(mu_);
                writing_ = false;
                cv_.notify_all();
            });
        }
    }
};
}  // namespace fma_common
