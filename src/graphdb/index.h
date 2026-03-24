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
#include <condition_variable>
#include <future>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "common/type_traits.h"
#include "common/value.h"
#include "ftindex/include/lib.rs.h"
#include "graphdb/graph_cf.h"
#include "graphdb/hnsw_index.h"
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
                      uint32_t lid, std::vector<uint32_t> pids)
      : meta_(std::move(meta)),
        cf_(cf),
        index_id_(index_id),
        lid_(lid),
        pids_(std::move(pids)),
        pid_set_(pids_.begin(), pids_.end()) {}
  void AddIndex(txn::Transaction* txn, int64_t vid,
                const std::vector<std::string>& values);
  void UpdateIndex(txn::Transaction* txn, int64_t vid,
                   const std::optional<std::vector<std::string>>& new_values,
                   const std::optional<std::vector<std::string>>& old_values);
  void DeleteIndex(txn::Transaction* txn, int64_t vid,
                   const std::vector<std::string>& values);
  std::string IndexKey(const std::vector<std::string>& values) const;
  std::string EntryKey(const std::vector<std::string>& values,
                       int64_t vid) const;
  std::optional<std::vector<std::string>> LoadVertexPropertyValues(
      txn::Transaction* txn, int64_t vid,
      const std::unordered_map<uint32_t, std::string>* overrides = nullptr,
      const std::unordered_set<uint32_t>* removed = nullptr) const;
  bool ContainsProperty(uint32_t pid) const { return pid_set_.count(pid) > 0; }
  bool TouchesAnyProperty(const std::unordered_set<uint32_t>& pids) const;
  bool AllPropertiesPresent(const std::unordered_set<uint32_t>& pids) const;
  bool is_unique() const { return meta_.is_unique(); }
  meta::VertexPropertyIndex& meta() { return meta_; }
  rocksdb::ColumnFamilyHandle* cf() { return cf_; }
  uint32_t lid() const { return lid_; }
  const std::vector<uint32_t>& pids() const { return pids_; }
  size_t PropertyCount() const { return pids_.size(); }
  uint32_t index_id() const { return index_id_; }

 private:
  meta::VertexPropertyIndex meta_;
  rocksdb::ColumnFamilyHandle* cf_;
  uint32_t index_id_;
  uint32_t lid_;
  std::vector<uint32_t> pids_;
  std::unordered_set<uint32_t> pid_set_;
};

class VertexFullTextIndex
    : public std::enable_shared_from_this<VertexFullTextIndex> {
 public:
  VertexFullTextIndex(rocksdb::TransactionDB* db,
                      boost::asio::io_service& service, GraphCF* graph_cf,
                      IdGenerator* id_generator, meta::VertexFullTextIndex meta,
                      uint32_t index_id,
                      const std::unordered_set<uint32_t>& lids,
                      const std::unordered_set<uint32_t>& pids,
                      size_t commit_interval);
  void AddVertex(int64_t id, std::vector<std::string> fields,
                 std::vector<std::string> values);
  void DeleteVertex(int64_t id);
  void ApplyWAL();
  void Start();
  void Stop();
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
  uint32_t index_id() const { return index_id_; }
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
  std::mutex timer_mutex_;
  std::condition_variable timer_cv_;
  size_t active_callbacks_ = 0;
  bool started_ = false;
  bool stopped_ = false;
  size_t interval_ = 5;
  boost::asio::steady_timer timer_;
};

struct BusyIndex {
 public:
  class ScopedMark {
   public:
    ScopedMark() = default;
    ScopedMark(BusyIndex* owner, std::unordered_set<uint32_t> lids,
               std::unordered_set<uint32_t> pids)
        : owner_(owner) {
      if (owner_) {
        owner_->Mark(std::move(lids), std::move(pids));
      }
    }
    ScopedMark(const ScopedMark&) = delete;
    ScopedMark& operator=(const ScopedMark&) = delete;
    ScopedMark(ScopedMark&& other) noexcept
        : owner_(std::exchange(other.owner_, nullptr)) {}
    ScopedMark& operator=(ScopedMark&& other) noexcept {
      if (this == &other) {
        return *this;
      }
      Reset();
      owner_ = std::exchange(other.owner_, nullptr);
      return *this;
    }
    ~ScopedMark() { Reset(); }

    void Reset() {
      if (owner_) {
        owner_->Clear();
        owner_ = nullptr;
      }
    }

   private:
    BusyIndex* owner_ = nullptr;
  };

  [[nodiscard]] ScopedMark Hold(std::unordered_set<uint32_t> _lids,
                                std::unordered_set<uint32_t> _pids) {
    return ScopedMark(this, std::move(_lids), std::move(_pids));
  }

  void Mark(std::unordered_set<uint32_t> _lids,
            std::unordered_set<uint32_t> _pids) {
    std::unique_lock lock(mutex_);
    lids_ = std::move(_lids);
    pids_ = std::move(_pids);
  }
  bool LabelBusy(const std::unordered_set<uint32_t>& _lids) const {
    std::shared_lock lock(mutex_);
    for (auto id : _lids) {
      if (lids_.count(id)) {
        return true;
      }
    }
    return false;
  }
  bool Busy(const std::unordered_set<uint32_t>& _lids, uint32_t _pid) const {
    std::shared_lock lock(mutex_);
    return std::any_of(lids_.begin(), lids_.end(),
                       [&_lids](uint32_t id) { return _lids.count(id) > 0; }) &&
           pids_.count(_pid);
  }
  bool Busy(const std::unordered_set<uint32_t>& _lids,
            const std::unordered_set<uint32_t>& _pids) const {
    std::shared_lock lock(mutex_);
    return std::any_of(lids_.begin(), lids_.end(),
                       [&_lids](uint32_t id) { return _lids.count(id) > 0; }) &&
           std::any_of(pids_.begin(), pids_.end(),
                       [&_pids](uint32_t id) { return _pids.count(id) > 0; });
  }
  void Clear() {
    std::unique_lock lock(mutex_);
    lids_.clear();
    pids_.clear();
  }

 private:
  mutable std::shared_mutex mutex_;
  std::unordered_set<uint32_t> lids_;
  std::unordered_set<uint32_t> pids_;
};

class VertexVectorIndex
    : public std::enable_shared_from_this<VertexVectorIndex> {
 public:
  VertexVectorIndex(rocksdb::TransactionDB* db,
                    boost::asio::io_service& service, GraphCF* graph_cf,
                    uint32_t index_id, uint32_t lid, uint32_t pid,
                    meta::VertexVectorIndex meta, size_t commit_interval);
  std::vector<std::pair<int64_t, float>> KnnSearch(const float* query,
                                                   int top_k, int ef_search);
  int64_t GetElementsNum();
  int64_t GetMemoryUsage();
  int64_t GetDeletedIdsNum();
  const meta::VertexVectorIndex& meta() { return meta_; }
  uint32_t lid() const { return lid_; }
  uint32_t pid() const { return pid_; }
  uint32_t index_id() const { return index_id_; }
  void Start();
  void Stop();
  void Load();
  void TryDeleteIndex(txn::Transaction* txn, int64_t vid);
  std::string NextWALKey();
  std::string IndexKey(int64_t vid);
  std::string DeleteMarkKey(int64_t vector_id);
  void AddIndex(txn::Transaction* txn, int64_t vid,
                meta::VectorIndexUpdate& wal);
  void ApplyWAL();

 private:
  void StartTimer();
  rocksdb::TransactionDB* db_ = nullptr;
  GraphCF* graph_cf_ = nullptr;
  uint32_t index_id_;
  uint32_t lid_;
  uint32_t pid_;
  meta::VertexVectorIndex meta_;
  std::unique_ptr<FaissHnswIndex> hnsw_index_;
  std::atomic<int64_t> next_vector_id_ = 1;
  std::atomic<uint64_t> next_wal_id_ = 1;
  uint64_t apply_id_ = 0;
  std::shared_mutex mutex_;
  std::mutex apply_mutex_;
  std::mutex timer_mutex_;
  std::condition_variable timer_cv_;
  size_t active_callbacks_ = 0;
  bool started_ = false;
  bool stopped_ = false;
  size_t interval_ = 5;
  boost::asio::steady_timer timer_;
  std::unordered_set<int64_t> deleted_vector_ids_;
  std::unordered_map<int64_t, int64_t> vectorid_vid_;
};

}  // namespace graphdb
