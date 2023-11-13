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

#include <gtest/gtest_prod.h>
#include <atomic>
#include "core/kv_store.h"
class TestBlobManager_BlobManager_Test;

namespace lgraph {
class BlobManager {
    FRIEND_TEST(::TestBlobManager, BlobManager);
    std::unique_ptr<KvTable> table_;

 public:
    typedef int64_t BlobKey;

    static std::unique_ptr<KvTable> OpenTable(
        KvTransaction& txn, KvStore& store, const std::string& name) {
        ComparatorDesc desc;
        desc.comp_type = ComparatorDesc::SINGLE_TYPE;
        desc.data_type = FieldType::INT64;
        return store.OpenTable(txn, name, true, desc);
    }

    BlobManager(KvTransaction& txn, std::unique_ptr<KvTable> t) : table_(std::move(t)) {}

    // add a new blob and return its key
    BlobKey Add(KvTransaction& txn, const Value& v) {
        BlobKey key = GetNextBlobKey(txn);
        table_->AppendKv(txn, Value::ConstRef(key), v);
        return key;
    }

    // get a blob
    Value Get(KvTransaction& txn, const BlobKey& key) {
        auto it = table_->GetIterator(txn, Value::ConstRef(key));
        FMA_DBG_CHECK(it->IsValid());
        return it->GetValue();
    }

    // replace existing blob
    void Replace(KvTransaction& txn, const BlobKey& key, const Value& v) {
        auto it = table_->GetIterator(txn, Value::ConstRef(key));
        FMA_DBG_CHECK(it->IsValid());
        return it->SetValue(v);
    }

    // delete blob
    void Delete(KvTransaction& txn, const BlobKey& key) {
        auto it = table_->GetIterator(txn, Value::ConstRef(key));
        FMA_DBG_CHECK(it->IsValid());
        it->DeleteKey();
    }

    BlobKey GetNextBlobKey(KvTransaction& txn) {
        auto it = table_->GetIterator(txn);
        it->GotoLastKey();
        BlobKey key = 0;
        if (it->IsValid()) {
            key = it->GetKey().AsType<BlobKey>() + 1;
        }
        return key;
    }

    void _BatchAddBlobs(KvTransaction& txn, const std::vector<std::pair<BlobKey, Value>>& blobs) {
        if (blobs.empty()) return;
        BlobKey last_key = GetNextBlobKey(txn) - 1;
        FMA_DBG_ASSERT(blobs.front().first > last_key);
        for (auto& kv : blobs) {
            FMA_DBG_ASSERT(kv.first > last_key);
            last_key = kv.first;
            table_->AppendKv(txn, Value::ConstRef(kv.first), kv.second);
        }
    }

    // static methods

    // blobs are stored as string in the graph table, plus additional value in blob table
    // the format for the blob value is:
    //   [char is_large_blob] [blob_bytes]  for small blobs
    //   [char is_large_blob] [BlobKey] for large blobs

    // check if this is a large blob
    static inline bool IsLargeBlob(const Value& record) {
        if (record.Empty()) return false;
        return *(char*)record.Data();
    }

    // check if this is a large blob
    static inline bool IsLargeBlob(const FieldData& fd) {
        if (!fd.IsBlob()) return false;
        const std::string& buf = fd.AsBlob();
        return !buf.empty() && buf[0];
    }

    // gets blob size given the blob record
    static inline size_t GetSmallBlobSize(const Value& record) {
        if (record.Empty()) return 0;
        FMA_DBG_ASSERT(!IsLargeBlob(record));
        return record.Size() - 1;
    }

    // gets blob content from small blob
    static inline Value GetSmallBlobContent(const Value& record) {
        if (record.Empty()) return Value();
        FMA_DBG_ASSERT(!IsLargeBlob(record));
        return Value(record.Data() + 1, record.Size() - 1);
    }

    // gets blob key given a large blob record
    static inline BlobKey GetLargeBlobKey(const Value& record) {
        FMA_DBG_ASSERT(!record.Empty());
        FMA_DBG_ASSERT(IsLargeBlob(record));
        FMA_DBG_CHECK_EQ(record.Size(), sizeof(BlobKey) + 1);
        return Value(record.Data() + 1, record.Size() - 1).AsType<BlobKey>();
    }

    // construct a large blob value
    static inline Value ComposeLargeBlobData(const BlobKey& blob_key) {
        Value v(1 + sizeof(BlobKey));
        v.Data()[0] = 1;
        _detail::UnalignedSet<BlobKey>(v.Data() + 1, blob_key);
        return v;
    }

    // construct a small blob value
    // If blob is empty, return empty buffer
    static inline Value ComposeSmallBlobData(const Value& blob) {
        FMA_DBG_ASSERT(blob.Size() <= _detail::MAX_IN_PLACE_BLOB_SIZE);
        if (blob.Empty()) return Value();
        Value v(1 + blob.Size());
        v.Data()[0] = 0;
        memcpy(v.Data() + 1, blob.Data(), blob.Size());
        return v;
    }
};
}  // namespace lgraph
