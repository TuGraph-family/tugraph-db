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

#include "index.h"

#include <rocksdb/utilities/write_batch_with_index.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_set>

#include "common/byte_utils.h"
#include "common/flags.h"
#include "common/logger.h"
#include "ftindex/include/lib.rs.h"
#include "spdlog/stopwatch.h"
#include "transaction/transaction.h"

using namespace txn;
using namespace boost::endian;
using common::AsChars;
using common::ReadValue;
namespace graphdb {

namespace {

const char* kFaissHnswIndexFileName = "hnsw.index.data";
const char* kFaissHnswMetaFileName = "hnsw.index.meta";

std::string FaissHnswIndexFilePath(const meta::VertexVectorIndex& meta) {
  return meta.path() + "/" + kFaissHnswIndexFileName;
}

std::string FaissHnswMetaFilePath(const meta::VertexVectorIndex& meta) {
  return meta.path() + "/" + kFaissHnswMetaFileName;
}

}  // namespace

void VertexPropertyIndex::AddIndex(Transaction* txn, int64_t vid,
                                   rocksdb::Slice value) {
  if (meta_.is_unique()) {
    rocksdb::ReadOptions ro;
    std::string exists_val;
    // lock index

    std::string index_key = IndexKey(value.ToString());
    auto s = txn->dbtxn()->GetForUpdate(ro, cf_, index_key, &exists_val);
    if (s.ok()) {
      THROW_CODE(IndexValueAlreadyExist);
    } else if (!s.IsNotFound()) {
      THROW_CODE(StorageEngineError, s.ToString());
    }
    rocksdb::Slice index_val(AsChars(vid), sizeof(vid));
    s = txn->dbtxn()->GetWriteBatch()->Put(cf_, index_key, index_val);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
  }
}

void VertexPropertyIndex::UpdateIndex(Transaction* txn, int64_t vid,
                                      rocksdb::Slice new_value,
                                      const std::string* old_value) {
  if (meta_.is_unique()) {
    std::string tmp;
    rocksdb::ReadOptions ro;
    // lock index
    std::string index_key = IndexKey(new_value.ToString());
    bool keep_existing_entry = false;
    auto s = txn->dbtxn()->GetForUpdate(ro, cf_, index_key, &tmp);
    if (s.ok()) {
      if (tmp.size() != sizeof(int64_t)) {
        THROW_CODE(StorageEngineError,
                   "vertex unique index stores invalid vid size");
      }
      if (ReadValue<int64_t>(tmp.data()) != vid) {
        THROW_CODE(IndexValueAlreadyExist);
      }
      keep_existing_entry = true;
    } else if (!s.IsNotFound()) {
      THROW_CODE(StorageEngineError, s.ToString());
    }
    if (old_value) {
      // lock index
      std::string key = IndexKey(*old_value);
      if (key != index_key) {
        s = txn->dbtxn()->GetForUpdate(ro, cf_, key,
                                       static_cast<std::string*>(nullptr));
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
        s = txn->dbtxn()->GetWriteBatch()->SingleDelete(cf_, key);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
        keep_existing_entry = false;
      }
    }
    if (!keep_existing_entry) {
      s = txn->dbtxn()->GetWriteBatch()->Put(
          cf_, index_key, rocksdb::Slice(AsChars(vid), sizeof(vid)));
      if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }
  }
}

std::string VertexPropertyIndex::IndexKey(const std::string& val) {
  std::string index_key(AsChars(index_id_), sizeof(index_id_));
  index_key.append(val);
  return index_key;
}

void VertexPropertyIndex::DeleteIndex(Transaction* txn, rocksdb::Slice value) {
  if (meta_.is_unique()) {
    rocksdb::ReadOptions ro;
    // lock index
    std::string index_key = IndexKey(value.ToString());
    auto s = txn->dbtxn()->GetForUpdate(ro, cf_, index_key,
                                        static_cast<std::string*>(nullptr));
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    s = txn->dbtxn()->GetWriteBatch()->SingleDelete(cf_, index_key);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
  }
}

void VertexFullTextIndex::StartTimer() {
  {
    std::lock_guard<std::mutex> lock(timer_mutex_);
    if (stopped_) {
      return;
    }
  }
  timer_.expires_after(std::chrono::seconds(interval_));
  timer_.async_wait([this](const boost::system::error_code& e) {
    if (e) {
      if (e != boost::asio::error::operation_aborted) {
        LOG_ERROR("timer async_wait error: {}", e.message());
      }
      return;
    }
    {
      std::lock_guard<std::mutex> lock(timer_mutex_);
      if (stopped_) {
        timer_cv_.notify_all();
        return;
      }
      active_callbacks_++;
    }
    ApplyWAL();
    bool restart = false;
    {
      std::lock_guard<std::mutex> lock(timer_mutex_);
      active_callbacks_--;
      timer_cv_.notify_all();
      restart = !stopped_;
    }
    if (restart) {
      StartTimer();
    }
  });
}

void VertexFullTextIndex::Start() {
  {
    std::lock_guard<std::mutex> lock(timer_mutex_);
    if (started_ || stopped_) {
      return;
    }
    started_ = true;
  }
  StartTimer();
}

void VertexFullTextIndex::Stop() {
  {
    std::lock_guard<std::mutex> lock(timer_mutex_);
    if (stopped_) {
      return;
    }
    stopped_ = true;
    if (!started_) {
      return;
    }
  }

  std::promise<void> cancelled;
  auto future = cancelled.get_future();
  boost::asio::post(timer_.get_executor(), [this, &cancelled]() mutable {
    boost::system::error_code ec;
    timer_.cancel(ec);
    cancelled.set_value();
  });
  future.wait();

  std::unique_lock<std::mutex> lock(timer_mutex_);
  timer_cv_.wait(lock, [this] { return active_callbacks_ == 0; });
}

VertexFullTextIndex::VertexFullTextIndex(
    rocksdb::TransactionDB* db, boost::asio::io_service& service,
    GraphCF* graph_cf, IdGenerator* id_generator,
    meta::VertexFullTextIndex meta, uint32_t index_id,
    const std::unordered_set<uint32_t>& lids,
    const std::unordered_set<uint32_t>& pids, size_t commit_interval)
    : db_(db),
      graph_cf_(graph_cf),
      id_generator_(id_generator),
      meta_(std::move(meta)),
      index_id_(index_id),
      lids_(lids),
      pids_(pids),
      interval_(commit_interval),
      timer_(service) {
  ::rust::Vec<::rust::String> fields;
  for (auto& prop : meta_.properties()) {
    fields.push_back(prop);
  }
  instance_ = std::make_unique<::rust::Box<::FTIndex>>(
      new_ftindex(meta_.path(), fields));
  ft_index_ = instance_->operator->();
  auto payload = ft_get_payload(*ft_index_);
  if (!payload.empty()) {
    apply_id_ =
        native_to_big(static_cast<uint64_t>(std::stoull(payload.c_str())));
  }

  std::string prefix(AsChars(index_id_), sizeof(index_id_));
  prefix.append(8, 0xFF);
  rocksdb::ReadOptions ro;
  std::unique_ptr<rocksdb::Iterator> iter(db_->NewIterator(ro, graph_cf_->wal));
  iter->SeekForPrev(prefix);
  if (iter->Valid()) {
    auto key = iter->key();
    if (key.starts_with({AsChars(index_id_), sizeof(index_id_)})) {
      key.remove_prefix(sizeof(index_id_));
      if (key.size() != sizeof(uint64_t)) {
        THROW_CODE(StorageEngineError,
                   "fulltext index wal key has invalid size while loading next "
                   "wal id, expect {}, actual {}",
                   sizeof(uint64_t), key.size());
      }
      uint64_t wal_id = ReadValue<uint64_t>(key.data());
      next_wal_id_ = big_to_native(wal_id) + 1;
    }
  }
  next_wal_id_ = std::max(next_wal_id_.load(), big_to_native(apply_id_) + 1);
}

void VertexFullTextIndex::AddIndex(txn::Transaction* txn, int64_t vid,
                                   const meta::FullTextIndexUpdate& wal) {
  auto s =
      txn->dbtxn()->GetWriteBatch()->Put(graph_cf_->index, IndexKey(vid), {});
  if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
  txn->AppendFullTextIndexWAL(shared_from_this(), wal.SerializeAsString());
}

void VertexFullTextIndex::DeleteIndex(txn::Transaction* txn, int64_t vid,
                                      const meta::FullTextIndexUpdate& wal) {
  auto s =
      txn->dbtxn()->GetWriteBatch()->Delete(graph_cf_->index, IndexKey(vid));
  if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
  txn->AppendFullTextIndexWAL(shared_from_this(), wal.SerializeAsString());
}

bool VertexFullTextIndex::IsIndexed(Transaction* txn, int64_t vid) {
  std::string index_key = IndexKey(vid);
  std::string val;
  auto s = txn->dbtxn()->Get({}, graph_cf_->index, index_key, &val);
  if (s.ok()) {
    return true;
  } else if (s.IsNotFound()) {
    return false;
  } else {
    THROW_CODE(StorageEngineError, s.ToString());
  }
}

std::string VertexFullTextIndex::IndexKey(int64_t vid) {
  std::string ret(AsChars(index_id_), sizeof(index_id_));
  ret.append(AsChars(vid), sizeof(vid));
  return ret;
}

std::string VertexFullTextIndex::NextWALKey() {
  std::string ret(AsChars(index_id_), sizeof(index_id_));
  uint64_t wal_id = native_to_big(next_wal_id_++);
  ret.append(AsChars(wal_id), sizeof(wal_id));
  return ret;
}

void VertexFullTextIndex::Load() {
  int count = 0;
  std::unordered_set<int64_t> loaded_vids;
  for (auto lid : lids_) {
    rocksdb::ReadOptions ro;
    std::unique_ptr<rocksdb::Iterator> iter(
        db_->NewIterator(ro, graph_cf_->vertex_label_vid));
    rocksdb::Slice prefix(AsChars(lid), sizeof(lid));
    for (iter->Seek(prefix); iter->Valid() && iter->key().starts_with(prefix);
         iter->Next()) {
      auto key = iter->key();
      key.remove_prefix(sizeof(uint32_t));
      std::vector<std::string> fields;
      std::vector<std::string> values;
      for (auto pid : pids_) {
        std::string prop_name = id_generator_->GetPropertyName(pid).value();
        std::string property_val;
        std::string property_key = key.ToString();
        property_key.append(AsChars(pid), sizeof(pid));
        auto s = db_->Get(ro, graph_cf_->vertex_property, property_key,
                          &property_val);
        if (s.IsNotFound()) {
          continue;
        } else if (!s.ok()) {
          THROW_CODE(StorageEngineError, s.ToString());
        }
        Value pv;
        pv.Deserialize(property_val.data(), property_val.size());
        if (!pv.IsString()) {
          continue;
        }
        fields.push_back(prop_name);
        values.push_back(pv.AsString());
      }
      if (!fields.empty()) {
        int64_t id = ReadValue<int64_t>(key.data());
        if (!loaded_vids.emplace(id).second) {
          continue;
        }
        auto s = db_->Put({}, graph_cf_->index, IndexKey(id), {});
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
        AddVertex(id, fields, values);
        count++;
        if (count == 10000) {
          Commit("0");
          count = 0;
        }
      }
    }
  }
  if (count > 0) {
    Commit("0");
  }
}

void VertexFullTextIndex::AddVertex(int64_t id, std::vector<std::string> fields,
                                    std::vector<std::string> values) {
  ::rust::Vec<::rust::String> rust_fields;
  ::rust::Vec<::rust::String> rust_values;
  for (auto& item : fields) {
    rust_fields.emplace_back(std::move(item));
  }
  for (auto& item : values) {
    rust_values.emplace_back(std::move(item));
  }
  ft_add_document(*ft_index_, id, rust_fields, rust_values);
}

bool VertexFullTextIndex::MatchLabelIds(
    const std::unordered_set<uint32_t>& lids) const {
  return std::any_of(lids.begin(), lids.end(), [this](uint32_t lid) {
    return lids_.find(lid) != lids_.end();
  });
}

bool VertexFullTextIndex::MatchPropertyIds(
    const std::unordered_set<uint32_t>& pids) const {
  return std::any_of(pids.begin(), pids.end(), [this](uint32_t lid) {
    return pids_.find(lid) != pids_.end();
  });
}

void VertexFullTextIndex::DeleteVertex(int64_t id) {
  ft_delete_document(*ft_index_, id);
}

void VertexFullTextIndex::Commit(const std::string& payload) {
  ft_commit(*ft_index_, payload);
}

void VertexFullTextIndex::ApplyWAL() {
  std::lock_guard<std::mutex> lock(mutex_);
  std::string prefix(AsChars(index_id_), sizeof(index_id_));
  std::string start_key(prefix);
  uint64_t next = big_to_native(apply_id_) + 1;
  native_to_big_inplace(next);
  start_key.append(AsChars(next), sizeof(next));
  int count = 0;
  uint64_t consumed_wal_id = 0;
  rocksdb::WriteBatch delete_batch;
  rocksdb::ReadOptions ro;
  rocksdb::WriteOptions wo;
  std::unique_ptr<rocksdb::Iterator> iter(db_->NewIterator(ro, graph_cf_->wal));
  for (iter->Seek(start_key); iter->Valid() && iter->key().starts_with(prefix);
       iter->Next()) {
    auto key = iter->key();
    delete_batch.Delete(graph_cf_->wal, key.ToString());

    key.remove_prefix(sizeof(index_id_));
    if (key.size() != sizeof(apply_id_)) {
      THROW_CODE(
          StorageEngineError,
          "fulltext index wal key has invalid size, expect {}, actual {}",
          sizeof(apply_id_), key.size());
    }
    consumed_wal_id = ReadValue<uint64_t>(key.data());
    meta::FullTextIndexUpdate update;
    auto val = iter->value();
    auto ret = update.ParseFromArray(val.data(), val.size());
    if (!ret) {
      THROW_CODE(StorageEngineError,
                 "failed to parse fulltext index wal payload");
    }
    if (update.type() == meta::UpdateType::Add) {
      AddVertex(update.vid(),
                {std::make_move_iterator(update.mutable_fields()->begin()),
                 std::make_move_iterator(update.mutable_fields()->end())},
                {std::make_move_iterator(update.mutable_values()->begin()),
                 std::make_move_iterator(update.mutable_values()->end())});
    } else {
      DeleteVertex(update.vid());
    }
    if (++count == 1000) {
      auto payload = std::to_string(big_to_native(consumed_wal_id));
      Commit(payload);
      LOG_DEBUG("apply {} wal, payload: {}", count, payload);
      count = 0;
      rocksdb::TransactionDBWriteOptimizations two;
      two.skip_concurrency_control = true;
      two.skip_duplicate_key_check = true;
      auto s = db_->Write(wo, two, &delete_batch);
      if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
      delete_batch.Clear();
    }
  }
  if (count > 0) {
    auto payload = std::to_string(big_to_native(consumed_wal_id));
    Commit(payload);
    LOG_DEBUG("apply {} wal, payload: {}", count, payload);
    count = 0;
    rocksdb::TransactionDBWriteOptimizations two;
    two.skip_concurrency_control = true;
    two.skip_duplicate_key_check = true;
    auto s = db_->Write(wo, two, &delete_batch);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    delete_batch.Clear();
  }
  if (consumed_wal_id != 0) {
    apply_id_ = consumed_wal_id;
  }
}

::rust::Vec<::IdScore> VertexFullTextIndex::Query(const std::string& query,
                                                  size_t top_n) {
  return ft_query(*ft_index_, query, QueryOptions{top_n});
}

VertexVectorIndex::VertexVectorIndex(rocksdb::TransactionDB* db,
                                     boost::asio::io_service& service,
                                     graphdb::GraphCF* graph_cf,
                                     uint32_t index_id, uint32_t lid,
                                     uint32_t pid, meta::VertexVectorIndex meta,
                                     size_t commit_interval)
    : db_(db),
      graph_cf_(graph_cf),
      index_id_(index_id),
      lid_(lid),
      pid_(pid),
      meta_(std::move(meta)),
      interval_(commit_interval),
      timer_(service) {
  if (meta_.distance_type() != meta::VectorDistanceType::L2 &&
      meta_.distance_type() != meta::VectorDistanceType::IP) {
    THROW_CODE(VectorIndexException, "invalid metric_type: {}",
               meta::VectorDistanceType_Name(meta_.distance_type()));
  }
  hnsw_index_ = std::make_unique<FaissHnswIndex>(
      meta_.dimensions(), meta_.distance_type(), meta_.hnsw_m(),
      meta_.hnsw_ef_construction());

  {
    std::ifstream metafile(FaissHnswMetaFilePath(meta_), std::ios::in);
    if (metafile) {
      LOG_INFO("Begin load vector index {} from data file", meta_.name());
      nlohmann::json meta_info;
      metafile >> meta_info;
      metafile.close();
      hnsw_index_ = FaissHnswIndex::Load(
          FaissHnswIndexFilePath(meta_), meta_.dimensions(),
          meta_.distance_type(), meta_.hnsw_m(), meta_.hnsw_ef_construction());
      uint64_t apply_id = meta_info["apply_id"];
      apply_id_ = native_to_big(apply_id);
      LOG_INFO("End load vector index {} from data file, num:{}", meta_.name(),
               hnsw_index_->GetNumElements());
    }
    LOG_INFO("vector index {}, apply_id:{}", meta_.name(),
             big_to_native(apply_id_));
  }
  {
    std::string prefix(AsChars(index_id_), sizeof(index_id_));
    prefix.append(8, 0xFF);
    rocksdb::ReadOptions ro;
    std::unique_ptr<rocksdb::Iterator> iter(
        db_->NewIterator(ro, graph_cf_->wal));
    iter->SeekForPrev(prefix);
    if (iter->Valid()) {
      auto key = iter->key();
      if (key.starts_with({AsChars(index_id_), sizeof(index_id_)})) {
        key.remove_prefix(sizeof(index_id_));
        if (key.size() != sizeof(uint64_t)) {
          THROW_CODE(
              VectorIndexException,
              "vector index wal key has invalid size while loading next wal "
              "id, expect {}, actual {}",
              sizeof(uint64_t), key.size());
        }
        uint64_t wal_id = ReadValue<uint64_t>(key.data());
        next_wal_id_ = big_to_native(wal_id) + 1;
      }
    }
    LOG_INFO("vector index {}, next_wal_id: {}", meta_.name(),
             next_wal_id_.load());
  }
  {
    spdlog::stopwatch sw;
    int64_t max_vector_id = -1;
    std::string prefix(AsChars(index_id_), sizeof(index_id_));
    rocksdb::ReadOptions ro;
    std::unique_ptr<rocksdb::Iterator> iter(
        db_->NewIterator(ro, graph_cf_->index));
    for (iter->Seek(prefix); iter->Valid() && iter->key().starts_with(prefix);
         iter->Next()) {
      auto key = iter->key();
      auto val = iter->value();
      key.remove_prefix(sizeof(index_id_));
      if (key.size() != 1 + sizeof(int64_t)) {
        THROW_CODE(
            VectorIndexException,
            "vector index entry key has invalid size, expect {}, actual {}",
            1 + sizeof(int64_t), key.size());
      }
      char flag = *key.data();
      key.remove_prefix(1);
      if (flag == 0) {
        if (val.size() != sizeof(int64_t)) {
          THROW_CODE(
              VectorIndexException,
              "vector index entry value has invalid size for vid mapping, "
              "expect {}, actual {}",
              sizeof(int64_t), val.size());
        }
        auto vid = ReadValue<int64_t>(key.data());
        auto vector_id = ReadValue<int64_t>(val.data());
        max_vector_id = std::max(max_vector_id, vector_id);
        vectorid_vid_.emplace(vector_id, vid);
      } else if (flag == 1) {
        if (!val.empty()) {
          THROW_CODE(
              VectorIndexException,
              "vector index delete marker has invalid value size, expect 0, "
              "actual {}",
              val.size());
        }
        auto vector_id = ReadValue<int64_t>(key.data());
        max_vector_id = std::max(max_vector_id, vector_id);
        deleted_vector_ids_.emplace(vector_id);
      } else {
        THROW_CODE(VectorIndexException,
                   "vector index entry has invalid flag: {}",
                   static_cast<int>(flag));
      }
    }
    if (max_vector_id != -1) {
      next_vector_id_ = max_vector_id + 1;
    }
    LOG_INFO(
        "vector index {}, vectorid_vid size:{}, deleted_vector_ids size:{}, "
        "elapsed:{}",
        meta_.name(), vectorid_vid_.size(), deleted_vector_ids_.size(), sw);
  }
}

void VertexVectorIndex::StartTimer() {
  {
    std::lock_guard<std::mutex> lock(timer_mutex_);
    if (stopped_) {
      return;
    }
  }
  timer_.expires_after(std::chrono::seconds(interval_));
  timer_.async_wait([this](const boost::system::error_code& e) {
    if (e) {
      if (e != boost::asio::error::operation_aborted) {
        LOG_ERROR("timer async_wait error: {}", e.message());
      }
      return;
    }
    {
      std::lock_guard<std::mutex> lock(timer_mutex_);
      if (stopped_) {
        timer_cv_.notify_all();
        return;
      }
      active_callbacks_++;
    }
    ApplyWAL();
    bool restart = false;
    {
      std::lock_guard<std::mutex> lock(timer_mutex_);
      active_callbacks_--;
      timer_cv_.notify_all();
      restart = !stopped_;
    }
    if (restart) {
      StartTimer();
    }
  });
}

void VertexVectorIndex::Start() {
  {
    std::lock_guard<std::mutex> lock(timer_mutex_);
    if (started_ || stopped_) {
      return;
    }
    started_ = true;
  }
  StartTimer();
}

void VertexVectorIndex::Stop() {
  {
    std::lock_guard<std::mutex> lock(timer_mutex_);
    if (stopped_) {
      return;
    }
    stopped_ = true;
    if (!started_) {
      return;
    }
  }

  std::promise<void> cancelled;
  auto future = cancelled.get_future();
  boost::asio::post(timer_.get_executor(), [this, &cancelled]() mutable {
    boost::system::error_code ec;
    timer_.cancel(ec);
    cancelled.set_value();
  });
  future.wait();

  std::unique_lock<std::mutex> lock(timer_mutex_);
  timer_cv_.wait(lock, [this] { return active_callbacks_ == 0; });
}

int64_t VertexVectorIndex::GetElementsNum() {
  std::shared_lock read(mutex_);
  return hnsw_index_->GetNumElements();
}

int64_t VertexVectorIndex::GetMemoryUsage() {
  std::shared_lock read(mutex_);
  return hnsw_index_->GetMemoryUsage();
}

int64_t VertexVectorIndex::GetDeletedIdsNum() {
  std::shared_lock read(mutex_);
  return deleted_vector_ids_.size();
}

std::vector<std::pair<int64_t, float>> VertexVectorIndex::KnnSearch(
    const float* query, int top_k, int ef_search) {
  std::vector<std::pair<int64_t, float>> ret;
  std::shared_lock read(mutex_);
  auto result = hnsw_index_->KnnSearch(
      query, top_k, ef_search, [this](int64_t label_id) -> bool {
        return deleted_vector_ids_.count(label_id) > 0;
      });
  for (size_t i = 0; i < result.ids.size(); ++i) {
    if (result.ids[i] < 0) {
      continue;
    }
    auto vector_id = result.ids[i];
    auto iter = vectorid_vid_.find(vector_id);
    if (iter == vectorid_vid_.end()) {
      THROW_CODE(VectorIndexException,
                 "vector id {} returned by faiss is missing in vid mapping",
                 vector_id);
    }
    ret.emplace_back(iter->second, result.distances[i]);
  }
  return ret;
}

void VertexVectorIndex::TryDeleteIndex(txn::Transaction* txn, int64_t vid) {
  std::string index_key = IndexKey(vid);
  std::string val;
  auto s = txn->dbtxn()->Get({}, graph_cf_->index, index_key, &val);
  if (s.ok()) {
    if (val.size() != sizeof(int64_t)) {
      THROW_CODE(VectorIndexException,
                 "vector index entry has invalid vector id size, expect {}, "
                 "actual {}",
                 sizeof(int64_t), val.size());
    }
    auto vector_id = ReadValue<int64_t>(val.data());
    s = txn->dbtxn()->GetWriteBatch()->Delete(graph_cf_->index, index_key);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    s = txn->dbtxn()->GetWriteBatch()->Put(graph_cf_->index,
                                           DeleteMarkKey(vector_id), {});
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    meta::VectorIndexUpdate wal;
    wal.set_type(meta::UpdateType::Delete);
    wal.set_vector_id(vector_id);
    txn->AppendVectorIndexWAL(shared_from_this(), wal.SerializeAsString());
  } else if (!s.IsNotFound()) {
    THROW_CODE(StorageEngineError, s.ToString());
  }
}

std::string VertexVectorIndex::NextWALKey() {
  std::string ret(AsChars(index_id_), sizeof(index_id_));
  uint64_t wal_id = native_to_big(next_wal_id_++);
  ret.append(AsChars(wal_id), sizeof(wal_id));
  return ret;
}

std::string VertexVectorIndex::IndexKey(int64_t vid) {
  char flag = 0;
  std::string ret(AsChars(index_id_), sizeof(index_id_));
  ret.append(1, flag);
  ret.append(AsChars(vid), sizeof(vid));
  return ret;
}

std::string VertexVectorIndex::DeleteMarkKey(int64_t vector_id) {
  char flag = 1;
  std::string ret(AsChars(index_id_), sizeof(index_id_));
  ret.append(1, flag);
  ret.append(AsChars(vector_id), sizeof(vector_id));
  return ret;
}

void VertexVectorIndex::ApplyWAL() {
  std::lock_guard<std::mutex> lock(apply_mutex_);
  std::string prefix(AsChars(index_id_), sizeof(index_id_));
  std::string start_key(prefix);
  uint64_t next = big_to_native(apply_id_) + 1;
  native_to_big_inplace(next);
  start_key.append(AsChars(next), sizeof(next));
  uint64_t consumed_wal_id = 0;
  rocksdb::ReadOptions ro;
  std::unique_ptr<rocksdb::Iterator> iter(db_->NewIterator(ro, graph_cf_->wal));
  for (iter->Seek(start_key); iter->Valid() && iter->key().starts_with(prefix);
       iter->Next()) {
    auto key = iter->key();
    rocksdb::Slice tmp = key;
    tmp.remove_prefix(sizeof(index_id_));
    if (tmp.size() != sizeof(apply_id_)) {
      THROW_CODE(VectorIndexException,
                 "vector index wal key has invalid size, expect {}, actual {}",
                 sizeof(apply_id_), tmp.size());
    }
    consumed_wal_id = ReadValue<uint64_t>(tmp.data());
    meta::VectorIndexUpdate update;
    auto val = iter->value();
    auto ret = update.ParseFromArray(val.data(), val.size());
    if (!ret) {
      THROW_CODE(VectorIndexException,
                 "failed to parse vector index wal payload");
    }
    if (update.type() == meta::UpdateType::Delete) {
      std::unique_lock write(mutex_);
      deleted_vector_ids_.emplace(update.vector_id());
      continue;
    }
    if (update.type() != meta::UpdateType::Add) {
      THROW_CODE(VectorIndexException,
                 "vector index wal has invalid update type: {}",
                 static_cast<int>(update.type()));
    }
    std::unique_ptr<float[]> embedding(new float[update.vector_size()]);
    for (int i = 0; i < update.vector_size(); i++) {
      embedding[i] = update.vector(i);
    }
    {
      std::unique_lock write(mutex_);
      const int64_t vector_id = update.vector_id();
      hnsw_index_->Add(embedding.get(), 1, &vector_id);
      vectorid_vid_.emplace(update.vector_id(), update.vid());
    }
    if (hnsw_index_->GetNumElements() % FLAGS_vt_serialize_interval == 0) {
      LOG_INFO("Vector Index {} begin serialization", meta_.name());
      hnsw_index_->WriteToFile(FaissHnswIndexFilePath(meta_));
      uint64_t apply_id = boost::endian::big_to_native(consumed_wal_id);
      nlohmann::json meta_info{{"apply_id", apply_id}};
      std::string path = FaissHnswMetaFilePath(meta_);
      std::ofstream metafile(path, std::ios::out | std::ios::trunc);
      if (!metafile.is_open()) {
        // WAL entries have already been applied to the in-memory index.
        // Keep apply_id_ aligned for the current process, but surface the
        // checkpoint persistence failure to the caller.
        apply_id_ = consumed_wal_id;
        THROW_CODE(IOException, "failed to open vector index meta file: {}",
                   path);
      }
      metafile << meta_info.dump();
      metafile.close();
      if (!metafile) {
        apply_id_ = consumed_wal_id;
        THROW_CODE(IOException, "failed to write vector index meta file: {}",
                   path);
      }
      LOG_INFO("write file: {}", FaissHnswIndexFilePath(meta_));
      LOG_INFO("write file: {}", path);
      LOG_INFO("Vector Index {} finish serialization, num:{}, apply_id: {}",
               meta_.name(), hnsw_index_->GetNumElements(), apply_id);
      rocksdb::WriteOptions wo;
      rocksdb::TransactionDBWriteOptimizations two;
      two.skip_concurrency_control = true;
      two.skip_duplicate_key_check = true;
      rocksdb::WriteBatch batch;
      batch.DeleteRange(graph_cf_->wal, prefix, key);
      auto s = db_->Write(wo, two, &batch);
      if (!s.ok()) {
        apply_id_ = consumed_wal_id;
        THROW_CODE(StorageEngineError,
                   "VertexVectorIndex db DeleteRange error: {}", s.ToString());
      }
    }
  }
  if (consumed_wal_id != 0) {
    apply_id_ = consumed_wal_id;
  }
}

void VertexVectorIndex::AddIndex(txn::Transaction* txn, int64_t vid,
                                 meta::VectorIndexUpdate& wal) {
  // vid -> vector id
  auto vector_id = next_vector_id_++;
  auto s = txn->dbtxn()->GetWriteBatch()->Put(
      graph_cf_->index, IndexKey(vid),
      rocksdb::Slice(AsChars(vector_id), sizeof(vector_id)));
  if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
  wal.set_vid(vid);
  wal.set_vector_id(vector_id);
  txn->AppendVectorIndexWAL(shared_from_this(), wal.SerializeAsString());
}

void VertexVectorIndex::Load() {
  rocksdb::ReadOptions ro;
  std::unique_ptr<rocksdb::Iterator> iter(
      db_->NewIterator(ro, graph_cf_->vertex_label_vid));
  SPDLOG_INFO("Begin to load vector index: {}", meta_.name());
  int count = 0;
  rocksdb::Slice prefix(AsChars(lid_), sizeof(lid_));
  for (iter->Seek(prefix); iter->Valid() && iter->key().starts_with(prefix);
       iter->Next()) {
    auto key = iter->key();
    key.remove_prefix(sizeof(uint32_t));
    int64_t vid = ReadValue<int64_t>(key.data());
    std::string property_key = key.ToString();
    property_key.append(AsChars(pid_), sizeof(pid_));
    std::string property_val;
    auto s =
        db_->Get(ro, graph_cf_->vertex_property, property_key, &property_val);
    if (s.IsNotFound()) {
      continue;
    } else if (!s.ok()) {
      THROW_CODE(StorageEngineError, s.ToString());
    }
    Value pv;
    pv.Deserialize(property_val.data(), property_val.size());
    if (!pv.IsArray()) {
      continue;
    }
    auto& array = pv.AsArray();
    if (array.empty() || array.size() != meta_.dimensions()) {
      continue;
    }
    if (!array[0].IsDouble() && !array[0].IsFloat()) {
      continue;
    }
    std::unique_ptr<float[]> embedding(new float[array.size()]);
    for (size_t i = 0; i < array.size(); i++) {
      if (array[i].IsDouble()) {
        embedding[i] = static_cast<float>(array[i].AsDouble());
      } else {
        embedding[i] = array[i].AsFloat();
      }
    }
    auto vector_id = next_vector_id_++;
    s = db_->Put({}, graph_cf_->index, IndexKey(vid),
                 rocksdb::Slice(AsChars(vector_id), sizeof(vector_id)));
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    vectorid_vid_.emplace(vector_id, vid);
    hnsw_index_->Add(embedding.get(), 1, &vector_id);
    count++;
    if (count % 10000 == 0) {
      SPDLOG_INFO("{} vector indexes have been load", count);
    }
  }
  SPDLOG_INFO("End to load vector index: {}, index num: {}", meta_.name(),
              count);
  if (hnsw_index_->GetNumElements() == 0) {
    return;
  }
  LOG_INFO("Vector Index {} begin serialization", meta_.name());
  hnsw_index_->WriteToFile(FaissHnswIndexFilePath(meta_));
  nlohmann::json meta_info{{"apply_id", 0}};
  std::string path = FaissHnswMetaFilePath(meta_);
  std::ofstream metafile(path, std::ios::out);
  metafile << meta_info.dump();
  metafile.close();
  LOG_INFO("write file: {}", FaissHnswIndexFilePath(meta_));
  LOG_INFO("write file: {}", path);
  SPDLOG_INFO("Vector Index {} Serialize, num:{}", meta_.name(),
              hnsw_index_->GetNumElements());
}

}  // namespace graphdb
