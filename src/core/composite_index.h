/**
 * Copyright 2024 AntGroup CO., Ltd.
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

#include <atomic>
#include <exception>
#include <unordered_map>

#include "core/field_data_helper.h"
#include "core/data_type.h"
#include "core/iterator_base.h"
#include "core/kv_store.h"
#include "core/kv_table_comparators.h"
#include "core/type_convert.h"
#include "core/value.h"

namespace lgraph {

class CompositeIndexValue {
    friend class CompositeIndex;

    Value v_;

    CompositeIndexValue(): v_(1) { *(uint8_t*)v_.Data() = 0; }

    explicit CompositeIndexValue(const Value& v): v_(v) {}

    explicit CompositeIndexValue(Value&& v): v_(std::move(v)) {}

    int GetVidCount() const { return static_cast<int>(*(uint8_t*)v_.Data()); }

    template <typename IT>
    CompositeIndexValue(const IT& beg, const IT& end) {
        size_t n = end - beg;
        FMA_DBG_ASSERT(n < std::numeric_limits<uint8_t>::max());
        v_.Resize(1 + _detail::VID_SIZE * n);
        char* p = v_.Data();
        *(uint8_t*)p = (uint8_t)n;
        p++;
        for (auto it = beg; it < end; it++) {
            _detail::WriteVid(p, *it);
            p += _detail::VID_SIZE;
        }
    }

    /**
     * Patch the key with the last vid in this VertexCompositeIndexValue.
     */
    Value CreateKey(const Value& key) const;

    const Value& GetBuf() const { return v_; }

    Value& GetBuf() { return v_; }
};

/**
 * The indices
 */
class CompositeIndex {
    friend class LightningGraph;
    friend class Transaction;

    std::shared_ptr<KvTable> table_;
    std::vector<FieldType> key_types;
    std::atomic<bool> ready_;
    std::atomic<bool> disabled_;
    CompositeIndexType type_;

 public:
    CompositeIndex(std::shared_ptr<KvTable> table, std::vector<FieldType> key_types,
                   CompositeIndexType type);

    CompositeIndex(const CompositeIndex& rhs);

    CompositeIndex(CompositeIndex&& rhs) = delete;

    CompositeIndex& operator=(const CompositeIndex& rhs) = delete;

    CompositeIndex& operator=(CompositeIndex&& rhs) = delete;

    static std::unique_ptr<KvTable> OpenTable(KvTransaction& txn, KvStore& store,
                                              const std::string& name,
                                              const std::vector<FieldType> &dt,
                                              CompositeIndexType type);

    void _AppendCompositeIndexEntry(KvTransaction& txn, const Value& k, VertexId vid);

    bool Add(KvTransaction& txn, const Value& k, int64_t vid);

    bool IsReady() const {
        return ready_.load(std::memory_order_acquire) && !disabled_.load(std::memory_order_acquire);
    }

 private:
    void Clear(KvTransaction& txn) { table_->Drop(txn); }

    void SetReady() { ready_.store(true, std::memory_order_release); }

    void Disable() { disabled_.store(true, std::memory_order_release); }

    void Enable() { disabled_.store(false, std::memory_order_release); }

    [[nodiscard]] bool IsDisabled() const { return disabled_.load(std::memory_order_acquire); }
};

class CompositeIndexIterator : public ::lgraph::IteratorBase {
    friend class CompositeIndex;

    CompositeIndex* index_;
    std::unique_ptr<KvIterator> it_;
    Value key_end_;
    Value curr_key_;  // current indexed key, excluding vid
    bool valid_;
    int pos_;
    VertexId vid_;  // current vid
    CompositeIndexType type_;

    /**
     * The constructor for VertexIndexIterator
     *
     * \param [in,out]  idx         The VertexIndex object this iterator will point to.
     * \param [in,out]  txn         The transaction.
     * \param [in,out]  table       The index table.
     * \param           key_start   The start key.
     * \param           key_end     The end key.
     * \param           vid         The vid from which to start searching.
     * \param           unique      Whether the index is a unique index.
     */
    CompositeIndexIterator(CompositeIndex* idx, Transaction* txn, KvTable& table,
                        const Value& key_start,
                        const Value& key_end,
                        VertexId vid, CompositeIndexType type);

    CompositeIndexIterator(CompositeIndex* idx, KvTransaction* txn, KvTable& table,
                        const Value& key_start,
                        const Value& key_end,
                        VertexId vid, CompositeIndexType type);

    bool KeyOutOfRange();

    /**
     * Traverse to the previous key/value pair. Used when an inserted vid is
     * greater than all existing vids. Should be used together with KeyEquals.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool PrevKV();

    /**
     * Check whether the iterator's current key starts with key.
     *
     * \param        key       The key.
     * \return  Whether the iterator's current key starts with key.
     */
    bool KeyEquals(const Value& key);

    /** Loads content from iterator, assuming iterator is already at the right
     * position. */
    void LoadContentFromIt();

    DISABLE_COPY(CompositeIndexIterator);
    CompositeIndexIterator& operator=(CompositeIndexIterator&&) = delete;

 protected:
    void CloseImpl() override;

 public:
    CompositeIndexIterator(CompositeIndexIterator&& rhs);

    /**
     * Query if this iterator is valid, i.e. the Key and Vid can be queried.
     *
     * \return  True if valid, false if not.
     */
    bool IsValid() const { return valid_; }

    /**
     * Move to the next vertex id in the list, which consists of all the valid
     * vertex ids of the iterator and is sorted from small to large.
     *
     * \return  True if it succeeds, otherwise false.
     */
    bool Next();

    /**
     * Gets the current key.
     *
     * \return  The current key.
     */
    Value GetKey() const;

    /**
     * Gets the current vertex id.
     *
     * \return  The current vertex id.
     */
    int64_t GetVid() const { return vid_; }


    /**
     * Determines if we can refresh content if kv iterator modified
     *
     * @return  True if KvIterator was modified.
     */
    void RefreshContentIfKvIteratorModified() override;
};
}  // namespace lgraph
