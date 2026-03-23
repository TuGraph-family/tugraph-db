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

#include "id_generator.h"

#include <algorithm>
#include <boost/endian/conversion.hpp>
#include <chrono>
#include <iostream>
#include <thread>

#include "byte_utils.h"
#include "common/exceptions.h"
#include "common/logger.h"
using namespace boost::endian;
namespace graphdb {
namespace {

std::string TokenKey(MetaDataType type, const std::string &name) {
  std::string key;
  key.append(1, static_cast<char>(type));
  key.append(name);
  return key;
}

}  // namespace

int64_t SnowflakeIdGenerator::CurrentTimeMs() {
  auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now())
                 .time_since_epoch()
                 .count();
  if (now < kEpochMs) {
    THROW_CODE(InvalidParameter,
               "system clock is earlier than snowflake epoch");
  }
  return now;
}

int64_t SnowflakeIdGenerator::WaitNextMillis(int64_t last_timestamp_ms) const {
  int64_t now_ms = CurrentTimeMs();
  while (now_ms <= last_timestamp_ms) {
    std::this_thread::yield();
    now_ms = CurrentTimeMs();
  }
  return now_ms;
}

int64_t SnowflakeIdGenerator::ComposeId(int64_t timestamp_ms,
                                        int64_t sequence) const {
  return ((timestamp_ms - kEpochMs) << kTimestampShift) |
         (static_cast<int64_t>(worker_id_) << kWorkerShift) | sequence;
}

void SnowflakeIdGenerator::SetWorkerId(uint16_t worker_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (worker_id > kMaxWorkerId) {
    THROW_CODE(InvalidParameter, "snowflake worker id {} exceeds max {}",
               worker_id, kMaxWorkerId);
  }
  worker_id_ = worker_id;
}

int64_t SnowflakeIdGenerator::NextId() {
  std::lock_guard<std::mutex> lock(mutex_);
  int64_t timestamp_ms = CurrentTimeMs();
  if (timestamp_ms < last_timestamp_ms_) {
    timestamp_ms = WaitNextMillis(last_timestamp_ms_);
  }
  if (timestamp_ms == last_timestamp_ms_) {
    sequence_ = (sequence_ + 1) & kSequenceMask;
    if (sequence_ == 0) {
      timestamp_ms = WaitNextMillis(last_timestamp_ms_);
    }
  } else {
    sequence_ = 0;
  }
  last_timestamp_ms_ = timestamp_ms;
  return ComposeId(timestamp_ms, sequence_);
}

void IdGenerator::Bind(rocksdb::TransactionDB *db, GraphCF *graph_cf,
                       uint16_t server_id) {
  db_ = db;
  graph_cf_ = graph_cf;
  id_generator_.SetWorkerId(server_id);
}

void IdGenerator::LoadToken(MetaDataType type, const std::string &name,
                            uint32_t id) {
  if (type == MetaDataType::VertexLabel) {
    vertex_labels_name_to_id_[name] = id;
    vertex_labels_id_to_name_[id] = name;
  } else if (type == MetaDataType::EdgeType) {
    edge_types_name_to_id_[name] = id;
    edge_types_id_to_name_[id] = name;
  } else if (type == MetaDataType::Property) {
    properties_name_to_id_[name] = id;
    properties_id_to_name_[id] = name;
  } else {
    THROW_CODE(InvalidParameter, "unsupported token metadata type {}",
               static_cast<int>(type));
  }
}

void IdGenerator::SetMaxIds(uint32_t max_lid, uint32_t max_pid,
                            uint32_t max_tid, uint32_t max_index_id) {
  LOG_INFO("max_lid:{}, max_pid:{}, max_tid:{}, max_index_id:{}", max_lid,
           max_pid, max_tid, max_index_id);
  label_next_lid_ = max_lid + 1;
  label_next_pid_ = max_pid + 1;
  label_next_tid_ = max_tid + 1;
  index_next_id_ = max_index_id + 1;
}

int64_t IdGenerator::GetNextVid() {
  return native_to_big(id_generator_.NextId());
}

int64_t IdGenerator::GetNextEid() {
  return native_to_big(id_generator_.NextId());
}

uint32_t IdGenerator::GetNextIndexId() {
  return native_to_big(index_next_id_++);
}

std::optional<uint32_t> IdGenerator::GetLid(const std::string &name) {
  if (name.empty()) {
    THROW_CODE(InvalidParameter, "label name is empty");
  }
  std::shared_lock read_lock(vertex_labels_mutex_);
  auto iter = vertex_labels_name_to_id_.find(name);
  if (iter != vertex_labels_name_to_id_.end()) {
    return iter->second;
  } else {
    return {};
  }
}

std::optional<uint32_t> IdGenerator::GetPid(const std::string &name) {
  if (name.empty()) {
    THROW_CODE(InvalidParameter, "property name is empty");
  }
  std::shared_lock read_lock(properties_mutex_);
  auto iter = properties_name_to_id_.find(name);
  if (iter != properties_name_to_id_.end()) {
    return iter->second;
  } else {
    return {};
  }
}

std::optional<uint32_t> IdGenerator::GetTid(const std::string &name) {
  if (name.empty()) {
    THROW_CODE(InvalidParameter, "edge type is empty");
  }
  std::shared_lock read_lock(edge_types_mutex_);
  auto iter = edge_types_name_to_id_.find(name);
  if (iter != edge_types_name_to_id_.end()) {
    return iter->second;
  } else {
    return {};
  }
}

std::optional<std::string> IdGenerator::GetPropertyName(uint32_t pid) {
  std::shared_lock read_lock(properties_mutex_);
  auto iter = properties_id_to_name_.find(pid);
  if (iter != properties_id_to_name_.end()) {
    return iter->second;
  } else {
    return {};
  }
}

std::optional<std::string> IdGenerator::GetVertexLabelName(uint32_t lid) {
  std::shared_lock read_lock(vertex_labels_mutex_);
  auto iter = vertex_labels_id_to_name_.find(lid);
  if (iter != vertex_labels_id_to_name_.end()) {
    return iter->second;
  } else {
    return {};
  }
}

std::optional<std::string> IdGenerator::GetEdgeTypeName(uint32_t tid) {
  std::shared_lock read_lock(edge_types_mutex_);
  auto iter = edge_types_id_to_name_.find(tid);
  if (iter != edge_types_id_to_name_.end()) {
    return iter->second;
  } else {
    return {};
  }
}

uint32_t IdGenerator::GetOrCreateLid(const std::string &name) {
  if (name.empty()) {
    THROW_CODE(InvalidParameter, "label name is empty");
  }
  {
    std::shared_lock read_lock(vertex_labels_mutex_);
    auto iter = vertex_labels_name_to_id_.find(name);
    if (iter != vertex_labels_name_to_id_.end()) {
      return iter->second;
    }
  }
  {
    std::unique_lock write_lock(vertex_labels_mutex_);
    auto iter = vertex_labels_name_to_id_.find(name);
    if (iter != vertex_labels_name_to_id_.end()) {
      return iter->second;
    }
    std::string key = TokenKey(MetaDataType::VertexLabel, name);
    std::string val;
    uint32_t bigendian_lid = native_to_big(label_next_lid_++);
    val.append(AsChars(bigendian_lid), sizeof(bigendian_lid));
    rocksdb::WriteOptions options;
    auto s = db_->Put(options, graph_cf_->meta_info, key, val);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    vertex_labels_name_to_id_[name] = bigendian_lid;
    vertex_labels_id_to_name_[bigendian_lid] = name;
    return bigendian_lid;
  }
}

uint32_t IdGenerator::GetOrCreateTid(const std::string &name) {
  if (name.empty()) {
    THROW_CODE(InvalidParameter, "edge type is empty");
  }
  {
    std::shared_lock read_lock(edge_types_mutex_);
    auto iter = edge_types_name_to_id_.find(name);
    if (iter != edge_types_name_to_id_.end()) {
      return iter->second;
    }
  }
  {
    std::unique_lock write_lock(edge_types_mutex_);
    auto iter = edge_types_name_to_id_.find(name);
    if (iter != edge_types_name_to_id_.end()) {
      return iter->second;
    }
    std::string key = TokenKey(MetaDataType::EdgeType, name);
    std::string val;
    uint32_t bigendian_tid = native_to_big(label_next_tid_++);
    val.append(AsChars(bigendian_tid), sizeof(bigendian_tid));
    rocksdb::WriteOptions options;
    auto s = db_->Put(options, graph_cf_->meta_info, key, val);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    edge_types_name_to_id_[name] = bigendian_tid;
    edge_types_id_to_name_[bigendian_tid] = name;
    return bigendian_tid;
  }
}

uint32_t IdGenerator::GetOrCreatePid(const std::string &name) {
  if (name.empty()) {
    THROW_CODE(InvalidParameter, "property name is empty");
  }
  {
    std::shared_lock read_lock(properties_mutex_);
    auto iter = properties_name_to_id_.find(name);
    if (iter != properties_name_to_id_.end()) {
      return iter->second;
    }
  }
  {
    std::unique_lock write_lock(properties_mutex_);
    auto iter = properties_name_to_id_.find(name);
    if (iter != properties_name_to_id_.end()) {
      return iter->second;
    }
    std::string key = TokenKey(MetaDataType::Property, name);
    std::string val;
    uint32_t bigendian_pid = native_to_big(label_next_pid_++);
    val.append(AsChars(bigendian_pid), sizeof(bigendian_pid));
    rocksdb::WriteOptions options;
    auto s = db_->Put(options, graph_cf_->meta_info, key, val);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    properties_name_to_id_[name] = bigendian_pid;
    properties_id_to_name_[bigendian_pid] = name;
    return bigendian_pid;
  }
}

std::unordered_set<std::string> IdGenerator::GetProperties() {
  std::unordered_set<std::string> ret;
  std::shared_lock read_lock(properties_mutex_);
  for (const auto &[key, val] : properties_name_to_id_) {
    ret.insert(key);
  }
  return ret;
}

std::unordered_set<std::string> IdGenerator::GetVertexLabels() {
  std::unordered_set<std::string> ret;
  std::shared_lock read_lock(vertex_labels_mutex_);
  for (const auto &[key, val] : vertex_labels_name_to_id_) {
    ret.insert(key);
  }
  return ret;
}

std::unordered_set<std::string> IdGenerator::GetEdgeTypes() {
  std::unordered_set<std::string> ret;
  std::shared_lock read_lock(edge_types_mutex_);
  for (const auto &[key, val] : edge_types_name_to_id_) {
    ret.insert(key);
  }
  return ret;
}
}  // namespace graphdb
