/**
* Copyright 2022 AntGroup CO., Ltd.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once

class BufferedBlobWriter {
    typedef std::vector<std::pair<lgraph::BlobManager::BlobKey, lgraph::Value>> Buffer;
    Buffer buffer_;
    lgraph::BlobManager::BlobKey next_key_;
    size_t curr_buf_size_ = 0;
    size_t max_buf_size_ = 1 << 20;
    std::mutex mtx_;
    fma_common::PipelineStage<Buffer, void> writer_;

 public:
    // `1 << 20` in the constructor will trigger cpplint bug and throw an exception
    explicit BufferedBlobWriter(lgraph::LightningGraph* db, size_t max_buf_size = 1024 * 1024)
        : max_buf_size_(max_buf_size),
          writer_(
              [db](Buffer&& buf) {
                  auto txn = db->CreateWriteTxn();
                  txn._BatchAddBlobs(buf);
                  txn.Commit();
              },
              nullptr, 0, 1, 2, false) {
        next_key_ = db->CreateReadTxn()._GetNextBlobKey();
    }

    ~BufferedBlobWriter() {
        std::lock_guard<std::mutex> l(mtx_);
        if (!buffer_.empty()) FlushWithHeldLock();
        writer_.WaitTillClear();
    }

    lgraph::BlobManager::BlobKey AddBlob(lgraph::Value&& v) {
        std::lock_guard<std::mutex> l(mtx_);
        curr_buf_size_ += v.Size();
        lgraph::BlobManager::BlobKey key = next_key_++;
        buffer_.emplace_back(key, std::move(v));
        if (curr_buf_size_ > max_buf_size_) FlushWithHeldLock();
        return key;
    }

 private:
    void FlushWithHeldLock() {
        writer_.Push(std::move(buffer_));
        curr_buf_size_ = 0;
    }
};
