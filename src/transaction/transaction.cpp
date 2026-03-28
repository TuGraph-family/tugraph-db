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

#include "transaction/transaction.h"

#include <rocksdb/utilities/write_batch_with_index.h>

#include <algorithm>
#include <boost/endian/conversion.hpp>
#include <cstring>

#include "common/byte_utils.h"
#include "common/exceptions.h"
#include "common/logger.h"
#include "cypher/execution_plan/result_iterator.h"
#include "graphdb/graph_db.h"
using namespace graphdb;
using namespace boost::endian;

namespace {

bool IsRangeComparableValue(const Value& value) {
  return !value.IsArray() && !value.IsMap();
}

std::shared_ptr<VertexPropertyIndex> ResolveVertexPropertyIndexOrThrow(
    txn::Transaction* txn, const std::string& index_name) {
  auto index = txn->db()->meta_info().GetVertexPropertyIndex(index_name);
  if (!index) {
    THROW_CODE(VertexUniqueIndexNotFound, "No such vertex index [{}]",
               index_name);
  }
  return index;
}

std::vector<Value> BuildPropertyIndexQueryValues(
    const std::shared_ptr<VertexPropertyIndex>& index, const Value& query,
    const std::string& arg_name) {
  std::vector<Value> values;
  if (index->PropertyCount() == 1) {
    values.emplace_back(query);
    return values;
  }

  if (!query.IsArray()) {
    THROW_CODE(ReminderException,
               "{} type should be Array for composite index {}", arg_name,
               index->meta().name());
  }
  const auto& items = query.AsArray();
  if (items.size() != index->PropertyCount()) {
    THROW_CODE(ReminderException,
               "{} element count should be {}, but {} are given", arg_name,
               index->PropertyCount(), items.size());
  }
  return {items.begin(), items.end()};
}

std::optional<std::string> BuildPropertyIndexRangeKey(
    const std::shared_ptr<VertexPropertyIndex>& index,
    const std::optional<Value>& bound, const std::string& arg_name) {
  if (!bound.has_value()) {
    return std::nullopt;
  }
  auto values = BuildPropertyIndexQueryValues(index, *bound, arg_name);
  for (const auto& value : values) {
    if (!IsRangeComparableValue(value)) {
      THROW_CODE(ReminderException,
                 "{} does not support ARRAY or MAP component", arg_name);
    }
  }
  return index->IndexKey(values);
}

bool IsEmptyPropertyIndexRange(const std::optional<std::string>& lower_key,
                               const std::optional<std::string>& upper_key,
                               bool left_closed, bool right_closed) {
  if (!lower_key.has_value() || !upper_key.has_value()) {
    return false;
  }
  size_t common_size = std::min(lower_key->size(), upper_key->size());
  int cmp = std::memcmp(lower_key->data(), upper_key->data(), common_size);
  if (cmp == 0) {
    if (lower_key->size() < upper_key->size()) {
      cmp = -1;
    } else if (lower_key->size() > upper_key->size()) {
      cmp = 1;
    }
  }
  return cmp > 0 || (cmp == 0 && (!left_closed || !right_closed));
}

}  // namespace

namespace txn {
Vertex Transaction::CreateVertex(
    const std::unordered_set<std::string>& labels,
    const std::unordered_map<std::string, Value>& values) {
  rocksdb::Status s;
  int64_t vid = db_->id_generator().GetNextVid();
  std::unordered_set<uint32_t> lids;
  std::string buffer;
  for (const auto& label : labels) {
    auto lid = db_->id_generator().GetOrCreateLid(label);
    lids.emplace(lid);
    buffer.clear();
    buffer.append((const char*)&lid, sizeof(lid));
    buffer.append((const char*)&vid, sizeof(vid));
    s = txn_->GetWriteBatch()->Put(db_->graph_cf().vertex_label_vid, buffer,
                                   {});
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
  }
  buffer.clear();
  for (auto lid : lids) {
    buffer.append((const char*)&lid, sizeof(lid));
  }
  s = txn_->GetWriteBatch()->Put(db_->graph_cf().graph_topology,
                                 rocksdb::Slice((const char*)&vid, sizeof(vid)),
                                 buffer);
  if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
  std::unordered_map<uint32_t, const Value*> pid_values;
  std::unordered_map<uint32_t, std::string> serialized_values;
  std::unordered_set<uint32_t> pids;
  auto property_indexes = db_->meta_info().GetVertexPropertyIndexes();
  auto ft_indexes = db_->meta_info().GetVertexFullTextIndexes();
  auto vector_indexes = db_->meta_info().GetVertexVectorIndexes();
  for (const auto& [name, value] : values) {
    uint32_t pid = db_->id_generator().GetOrCreatePid(name);
    pid_values[pid] = &value;
    serialized_values.emplace(pid, value.Serialize());
    pids.insert(pid);
  }
  if (db_->busy_index().Busy(lids, pids)) {
    THROW_CODE(IndexBusy);
  }
  for (const auto& index : property_indexes) {
    if (!lids.count(index->lid())) {
      continue;
    }
    auto index_values = index->LoadIndexedPropertyValues(
        this, vid, &serialized_values, nullptr);
    if (!index_values) {
      continue;
    }
    index->AddIndex(this, vid, *index_values);
  }
  for (const auto& [pid, val] : serialized_values) {
    buffer.clear();
    buffer.append((const char*)&vid, sizeof(vid));
    buffer.append((const char*)&pid, sizeof(pid));
    s = txn_->GetWriteBatch()->Put(db_->graph_cf().vertex_property, buffer,
                                   val);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
  }
  // full text index
  for (const auto& ft : ft_indexes) {
    if (!ft->MatchLabelIds(lids)) {
      continue;
    }
    meta::FullTextIndexUpdate add;
    add.set_type(meta::UpdateType::Add);
    add.set_vid(vid);
    for (const auto& [pid, prop] : pid_values) {
      if (prop->IsString() && !prop->AsString().empty() &&
          ft->PropertyIds().count(pid)) {
        add.add_fields(db_->id_generator().GetPropertyName(pid).value());
        add.add_values(prop->AsString());
      }
    }
    if (!add.fields().empty()) {
      ft->AddIndex(this, vid, add);
    }
  }
  // vector index
  for (const auto& vvi : vector_indexes) {
    if (!lids.count(vvi->lid())) {
      continue;
    }
    auto iter = pid_values.find(vvi->pid());
    if (iter == pid_values.end()) {
      continue;
    }
    auto prop = iter->second;
    if (!prop->IsArray()) {
      continue;
    }
    auto& array = prop->AsArray();
    if (array.empty() || (!array[0].IsDouble() && !array[0].IsFloat())) {
      continue;
    }
    if (array.size() != vvi->meta().dimensions()) {
      continue;
    }
    meta::VectorIndexUpdate add;
    add.set_type(meta::UpdateType::Add);
    for (auto& item : array) {
      if (item.IsFloat()) {
        add.add_vector(item.AsFloat());
      } else {
        add.add_vector((float)item.AsDouble());
      }
    }
    vvi->AddIndex(this, vid, add);
  }
  return {this, vid};
}

Edge Transaction::CreateEdge(
    const Vertex& start, const Vertex& end, const std::string& type,
    const std::unordered_map<std::string, Value>& values) {
  rocksdb::Status s;
  {
    rocksdb::ReadOptions ro;
    s = txn_->GetForUpdate(ro, db_->graph_cf().graph_topology,
                           start.GetIdView(), (std::string*)nullptr);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    s = txn_->GetForUpdate(ro, db_->graph_cf().graph_topology, end.GetIdView(),
                           (std::string*)nullptr);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
  }

  int64_t eid = db_->id_generator().GetNextEid();
  uint32_t tid = db_->id_generator().GetOrCreateTid(type);
  std::string key, val;
  // out key
  key.append(start.GetIdView());
  key.append(1, 0);
  key.append((const char*)&tid, sizeof(tid));
  key.append(end.GetIdView());
  key.append((const char*)&eid, sizeof(eid));
  s = txn_->GetWriteBatch()->Put(db_->graph_cf().graph_topology, key, {});
  if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
  // in key
  key.clear();
  key.append(end.GetIdView());
  key.append(1, 1);
  key.append((const char*)&tid, sizeof(tid));
  key.append(start.GetIdView());
  key.append((const char*)&eid, sizeof(eid));
  s = txn_->GetWriteBatch()->Put(db_->graph_cf().graph_topology, key, {});
  if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
  // type
  key.clear();
  key.append((const char*)&tid, sizeof(tid));
  key.append((const char*)&eid, sizeof(eid));
  val.append(start.GetIdView());
  val.append(end.GetIdView());
  s = txn_->GetWriteBatch()->Put(db_->graph_cf().edge_type_eid, key, val);
  if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
  // properties
  for (const auto& [name, value] : values) {
    uint32_t pid = db_->id_generator().GetOrCreatePid(name);
    key.clear();
    key.append((const char*)&eid, sizeof(eid));
    key.append((const char*)&pid, sizeof(pid));
    val = value.Serialize();
    s = txn_->GetWriteBatch()->Put(db_->graph_cf().edge_property, key, val);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
  }
  return {this, eid, start.GetId(), end.GetId(), tid};
}

Vertex Transaction::GetVertexById(int64_t vid) {
  rocksdb::ReadOptions ro;
  std::string val;
  auto s = txn_->Get(ro, db_->graph_cf().graph_topology,
                     rocksdb::Slice((const char*)&vid, sizeof(vid)), &val);
  if (s.ok()) {
    return {this, vid};
  } else if (s.IsNotFound()) {
    THROW_CODE(VertexIdNotFound, "Vertex id {} not found", big_to_native(vid));
  } else {
    THROW_CODE(StorageEngineError, s.ToString());
  }
}

Edge Transaction::GetEdgeById(uint32_t etid, int64_t eid) {
  std::string key;
  key.append((const char*)&etid, sizeof(etid));
  key.append((const char*)&eid, sizeof(eid));
  rocksdb::ReadOptions ro;
  std::string val;
  auto s = txn_->Get(ro, db_->graph_cf().edge_type_eid, key, &val);
  if (s.ok()) {
    auto p = val.data();
    int64_t startId = common::ReadValue<int64_t>(p);
    p += sizeof(int64_t);
    int64_t endId = common::ReadValue<int64_t>(p);
    return {this, eid, startId, endId, etid};
  } else if (s.IsNotFound()) {
    THROW_CODE(EdgeIdNotFound, "Edge [etid:{},eid:{}] not found",
               big_to_native(etid), big_to_native(eid));
  } else {
    THROW_CODE(StorageEngineError, s.ToString());
  }
}

std::unique_ptr<VertexIterator> Transaction::NewVertexIterator() {
  return std::make_unique<ScanAllVertex>(this);
}

std::unique_ptr<VertexIterator> Transaction::NewVertexIterator(
    const std::string& label) {
  auto lid = db_->id_generator().GetLid(label);
  if (lid.has_value()) {
    return std::make_unique<ScanVertexBylabel>(this, lid.value());
  } else {
    return std::make_unique<NoVertexFound>(this);
  }
}

std::string Transaction::GetVertexIteratorInfo(
    const std::optional<std::string>& label,
    const std::optional<std::unordered_set<std::string>>& props) {
  if (!label && !props) {
    return "ScanAllVertex";
  } else if (label && !props) {
    auto lid = db_->id_generator().GetLid(label.value());
    if (!lid.has_value()) {
      return "NoVertexFound";
    } else {
      return "ScanVertexBylabel";
    }
  } else if (!label && props) {
    std::unordered_map<uint32_t, Value> map;
    for (auto& name : props.value()) {
      auto pid = db_->id_generator().GetPid(name);
      if (!pid.has_value()) {
        return "NoVertexFound";
      }
    }
    return "ScanVertexByProperties";
  } else {
    auto lid = db_->id_generator().GetLid(label.value());
    if (!lid.has_value()) {
      return "NoVertexFound";
    }
    std::unordered_set<uint32_t> pids;
    for (auto& name : props.value()) {
      auto pid = db_->id_generator().GetPid(name);
      if (!pid.has_value()) {
        return "NoVertexFound";
      }
      pids.insert(pid.value());
    }
    if (db_->meta_info().GetBestVertexPropertyUniqueIndex(lid.value(), pids)) {
      return "GetVertexByUniqueIndex";
    }
    return "ScanVertexBylabelProperties";
  }
}

std::unique_ptr<VertexIterator> Transaction::NewVertexIterator(
    const std::optional<std::string>& label,
    const std::optional<std::unordered_map<std::string, Value>>& props) {
  if (!label && !props) {
    return std::make_unique<ScanAllVertex>(this);
  } else if (label && !props) {
    auto lid = db_->id_generator().GetLid(label.value());
    if (!lid.has_value()) {
      return std::make_unique<NoVertexFound>(this);
    } else {
      return std::make_unique<ScanVertexBylabel>(this, lid.value());
    }
  } else if (!label && props) {
    std::unordered_map<uint32_t, Value> map;
    for (auto& [name, val] : props.value()) {
      auto pid = db_->id_generator().GetPid(name);
      if (!pid.has_value()) {
        return std::make_unique<NoVertexFound>(this);
      } else {
        map.emplace(pid.value(), val);
      }
    }
    return std::make_unique<ScanVertexByProperties>(this, std::move(map));
  } else {
    auto lid = db_->id_generator().GetLid(label.value());
    if (!lid.has_value()) {
      return std::make_unique<NoVertexFound>(this);
    }
    std::unordered_map<uint32_t, Value> map;
    for (auto& [name, val] : props.value()) {
      auto pid = db_->id_generator().GetPid(name);
      if (!pid.has_value()) {
        return std::make_unique<NoVertexFound>(this);
      }
      map.emplace(pid.value(), val);
    }
    std::unordered_set<uint32_t> pids;
    for (const auto& [pid, _] : map) {
      pids.insert(pid);
    }
    auto unique_index =
        db_->meta_info().GetBestVertexPropertyUniqueIndex(lid.value(), pids);
    if (!unique_index) {
      return std::make_unique<ScanVertexBylabelProperties>(this, lid.value(),
                                                           std::move(map));
    }
    std::vector<Value> indexed_values;
    indexed_values.reserve(unique_index->PropertyCount());
    for (auto pid : unique_index->pids()) {
      indexed_values.push_back(map.at(pid));
      map.erase(pid);
    }
    return std::make_unique<GetVertexByUniqueIndex>(
        this, std::move(unique_index), std::move(indexed_values),
        std::move(map));
  }
}

void Transaction::AppendFullTextIndexWAL(
    std::shared_ptr<graphdb::VertexFullTextIndex> index,
    const meta::FullTextIndexUpdate& update) {
  PendingFullTextWALKey key{index.get(), update.vid()};
  auto it = pending_fulltext_wal_positions_.find(key);
  if (it == pending_fulltext_wal_positions_.end()) {
    pending_fulltext_wal_positions_.emplace(key, pending_fulltext_wals_.size());
    pending_fulltext_wals_.push_back({std::move(index), update});
    return;
  }
  pending_fulltext_wals_[it->second].update.CopyFrom(update);
}

void Transaction::Commit() {
  std::vector<std::shared_ptr<graphdb::VertexFullTextIndex>>
      touched_fulltext_indexes;
  {
    std::unique_lock<std::mutex> fulltext_commit_lock(
        db_->fulltext_index_commit_mutex(), std::defer_lock);
    std::unique_lock<std::mutex> vector_commit_lock(
        db_->vector_index_commit_mutex(), std::defer_lock);
    if (!pending_fulltext_wals_.empty() && !pending_vector_wals_.empty()) {
      std::lock(fulltext_commit_lock, vector_commit_lock);
    } else if (!pending_fulltext_wals_.empty()) {
      fulltext_commit_lock.lock();
    } else if (!pending_vector_wals_.empty()) {
      vector_commit_lock.lock();
    }
    if (!pending_fulltext_wals_.empty() || !pending_vector_wals_.empty()) {
      auto* write_batch = txn_->GetWriteBatch();
      std::unordered_set<graphdb::VertexFullTextIndex*> seen_fulltext_indexes;
      for (const auto& wal : pending_fulltext_wals_) {
        if (seen_fulltext_indexes.insert(wal.index.get()).second) {
          touched_fulltext_indexes.push_back(wal.index);
        }
        auto s = write_batch->Put(db_->graph_cf().wal, wal.index->NextWALKey(),
                                  wal.update.SerializeAsString());
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
      }
      for (const auto& wal : pending_vector_wals_) {
        auto s = write_batch->Put(db_->graph_cf().wal, wal.index->NextWALKey(),
                                  wal.payload);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
      }
    }
    auto s = txn_->Commit();
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    pending_fulltext_wals_.clear();
    pending_fulltext_wal_positions_.clear();
    pending_vector_wals_.clear();
  }
  for (const auto& index : touched_fulltext_indexes) {
    index->NotifyWALWritten();
  }
}

void Transaction::Rollback() {
  auto s = txn_->Rollback();
  if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
  pending_fulltext_wals_.clear();
  pending_fulltext_wal_positions_.clear();
  pending_vector_wals_.clear();
}

std::unique_ptr<VertexScoreIterator> Transaction::QueryVertexByFTIndex(
    const std::string& index_name, const std::string& query, size_t top_n) {
  return std::make_unique<GetVertexByFullTextIndex>(this, index_name, query,
                                                    top_n);
}

std::unique_ptr<graphdb::VertexIterator>
Transaction::QueryVertexByPropertyIndex(const std::string& index_name,
                                        const Value& query) {
  auto index = ResolveVertexPropertyIndexOrThrow(this, index_name);
  auto values = BuildPropertyIndexQueryValues(index, query, "query");
  auto key = index->IndexKey(values);
  return std::make_unique<GetVertexByPropertyIndex>(this, std::move(index),
                                                    std::move(key));
}

std::unique_ptr<graphdb::VertexIterator>
Transaction::QueryVertexByPropertyRange(const std::string& index_name,
                                        const std::optional<Value>& lower,
                                        const std::optional<Value>& upper,
                                        bool left_closed, bool right_closed) {
  auto index = ResolveVertexPropertyIndexOrThrow(this, index_name);
  auto lower_key = BuildPropertyIndexRangeKey(index, lower, "lower");
  auto upper_key = BuildPropertyIndexRangeKey(index, upper, "upper");
  if (IsEmptyPropertyIndexRange(lower_key, upper_key, left_closed,
                                right_closed)) {
    return std::make_unique<NoVertexFound>(this);
  }
  return std::make_unique<GetVertexByPropertyRange>(
      this, std::move(index), std::move(lower_key), std::move(upper_key),
      left_closed, right_closed);
}

std::unique_ptr<graphdb::VertexScoreIterator>
Transaction::QueryVertexByKnnSearch(const std::string& index_name,
                                    const std::vector<float>& query, int top_k,
                                    int ef_search) {
  return std::make_unique<GetVertexByKnnSearch>(this, index_name, query, top_k,
                                                ef_search);
}

std::unique_ptr<ResultIterator> Transaction::Execute(
    void* ctx, const std::string& cypher) {
  return std::make_unique<ResultIterator>(ctx, this, cypher);
}
}  // namespace txn
