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

//
// Created by botu.wzy
//

#pragma once
#include <rocksdb/utilities/transaction_db.h>

#include <boost/asio.hpp>
#include <utility>

#include "common/type_traits.h"
#include "common/value.h"
#include "ftindex/include/lib.rs.h"
#include "graphdb/embedding/index.h"
#include "graphdb/graph_cf.h"
#include "graphdb/id_generator.h"
#include "proto/meta.pb.h"

namespace txn {
class Transaction;
}
namespace graphdb {
struct VertexPropertyIndex {
   public:
    VertexPropertyIndex(meta::VertexPropertyIndex meta,
                        rocksdb::ColumnFamilyHandle* cf, uint32_t index_id,
                        uint32_t lid, uint32_t pid)
        : meta_(std::move(meta)),
          cf_(cf),
          index_id_(index_id),
          lid_(lid),
          pid_(pid) {}
    void AddIndex(txn::Transaction* txn, int64_t vid, rocksdb::Slice value);
    void UpdateIndex(txn::Transaction* txn, int64_t vid,
                     rocksdb::Slice new_value, const std::string* old_value);
    void DeleteIndex(txn::Transaction* txn, rocksdb::Slice value);
    std::string IndexKey(const std::string& val);
    meta::VertexPropertyIndex& meta() { return meta_; }
    rocksdb::ColumnFamilyHandle* cf() { return cf_; }
    uint32_t lid() const { return lid_; }
    uint32_t pid() const { return pid_; }

   private:
    meta::VertexPropertyIndex meta_;
    rocksdb::ColumnFamilyHandle* cf_;
    uint32_t index_id_;
    uint32_t lid_;
    uint32_t pid_;
};

enum UpdateType {
    Add,
    Delete,
};

class VertexFullTextIndex {
   public:
    VertexFullTextIndex(rocksdb::TransactionDB* db,
                        boost::asio::io_service& service, GraphCF* graph_cf,
                        IdGenerator* id_generator,
                        meta::VertexFullTextIndex meta, uint32_t index_id,
                        const std::unordered_set<uint32_t>& lids,
                        const std::unordered_set<uint32_t>& pids,
                        size_t commit_interval);
    void AddVertex(int64_t id, std::vector<std::string> fields,
                   std::vector<std::string> values);
    void DeleteVertex(int64_t id);
    void ApplyWAL();
    [[nodiscard]] bool MatchLabelIds(
        const std::unordered_set<uint32_t>& lids) const;
    [[nodiscard]] bool MatchPropertyIds(
        const std::unordered_set<uint32_t>& pids) const;
    ::rust::Vec<::IdScore> Query(const std::string& query, size_t top_n);
    [[nodiscard]] const std::unordered_set<uint32_t>& LabelIds() const {
        return lids_;
    }
    [[nodiscard]] const std::unordered_set<uint32_t>& PropertyIds() const {
        return pids_;
    }
    [[nodiscard]] const std::string& Name() const { return meta_.name(); }
    const meta::VertexFullTextIndex& meta() const { return meta_; }
    void Load();
    std::string IndexKey(int64_t vid);
    std::string NextWALKey();
    bool IsIndexed(txn::Transaction* txn, int64_t vid);
    void AddIndex(txn::Transaction* txn, int64_t vid,
                  const meta::FullTextIndexUpdate& wal);
    void DeleteIndex(txn::Transaction* txn, int64_t vid,
                     const meta::FullTextIndexUpdate& wal);

   private:
    void StartTimer();
    void Commit(const std::string& payload);

    rocksdb::TransactionDB* db_ = nullptr;
    GraphCF* graph_cf_ = nullptr;
    IdGenerator* id_generator_ = nullptr;
    meta::VertexFullTextIndex meta_;
    uint32_t index_id_;
    std::atomic<uint64_t> next_wal_id_ = 1;
    uint64_t apply_id_ = 0;
    std::unordered_set<uint32_t> lids_;
    std::unordered_set<uint32_t> pids_;
    ::FTIndex* ft_index_ = nullptr;
    std::unique_ptr<::rust::Box<::FTIndex>> instance_;
    std::mutex mutex_;
    size_t interval_ = 5;
    boost::asio::steady_timer timer_;
};

struct BusyIndex {
    std::unordered_set<uint32_t> lids;
    std::unordered_set<uint32_t> pids;

    void Mark(std::unordered_set<uint32_t> _lids,
              std::unordered_set<uint32_t> _pids) {
        lids = std::move(_lids);
        pids = std::move(_pids);
    }
    bool LabelBusy(const std::unordered_set<uint32_t>& _lids) const {
        for (auto id : _lids) {
            if (lids.count(id)) {
                return true;
            }
        }
        return false;
    }
    bool Busy(const std::unordered_set<uint32_t>& _lids, uint32_t _pid) {
        return std::any_of(
                   lids.begin(), lids.end(),
                   [&_lids](uint32_t id) { return _lids.count(id) > 0; }) &&
               pids.count(_pid);
    }
    bool Busy(const std::unordered_set<uint32_t>& _lids,
              const std::unordered_set<uint32_t>& _pids) {
        return std::any_of(
                   lids.begin(), lids.end(),
                   [&_lids](uint32_t id) { return _lids.count(id) > 0; }) &&
               std::any_of(pids.begin(), pids.end(), [&_pids](uint32_t id) {
                   return _pids.count(id) > 0;
               });
    }
    void Clear() {
        lids.clear();
        pids.clear();
    }
};

struct VectorIndexUpdate {
    uint64_t index_name;
    UpdateType type;
    int64_t vid;
    std::vector<Value> vectors;
};

class VertexVectorIndex {
   public:
    VertexVectorIndex(rocksdb::TransactionDB* db, GraphCF* graph_cf,
                      uint32_t lid, uint32_t pid, meta::VertexVectorIndex meta);
    void Add(int64_t vid, std::unique_ptr<float[]> embedding);
    void RemoveIfExists(int64_t vid);
    std::vector<std::pair<int64_t, float>> KnnSearch(const float* query,
                                                     int top_k, int ef_search);
    int64_t GetElementsNum();
    int64_t GetMemoryUsage();
    int64_t GetDeletedIdsNum();
    const meta::VertexVectorIndex& meta() { return meta_; }
    uint32_t lid() const { return lid_; }
    uint32_t pid() const { return pid_; }
    void Load();
    void LoadFromDisk(const meta::VectorIndexManifest& manifest);

   private:
    rocksdb::TransactionDB* db_ = nullptr;
    GraphCF* graph_cf_ = nullptr;
    uint32_t lid_;
    uint32_t pid_;
    meta::VertexVectorIndex meta_;
    std::unique_ptr<embedding::Index> index_;
};

}  // namespace graphdb
