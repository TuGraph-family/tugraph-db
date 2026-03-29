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

#include "meta_info.h"

#include <algorithm>
#include <boost/endian/conversion.hpp>
#include <filesystem>

#include "common/byte_utils.h"
#include "common/exceptions.h"
#include "common/logger.h"
#include "proto/meta.pb.h"
using namespace boost::endian;
using common::AsChars;
using common::ReadValue;
namespace graphdb {
namespace {

std::string BuildVertexPropertyIndexKey(uint32_t lid,
                                        const std::vector<uint32_t>& pids) {
  std::vector<uint32_t> sorted_pids = pids;
  std::sort(sorted_pids.begin(), sorted_pids.end());
  std::string key(AsChars(lid), sizeof(lid));
  for (auto pid : sorted_pids) {
    key.append(AsChars(pid), sizeof(pid));
  }
  return key;
}

}  // namespace

std::vector<std::shared_ptr<VertexFullTextIndex>>
MetaInfo::GetVertexFullTextIndexes() {
  std::shared_lock lock(mutex_);
  std::vector<std::shared_ptr<VertexFullTextIndex>> indexes;
  indexes.reserve(vertex_ft_indexes.size());
  for (const auto& [_, index] : vertex_ft_indexes) {
    indexes.push_back(index);
  }
  return indexes;
}

std::shared_ptr<VertexFullTextIndex> MetaInfo::GetVertexFullTextIndex(
    const std::string& name) {
  std::shared_lock lock(mutex_);
  auto iter = vertex_ft_indexes.find(name);
  if (iter != vertex_ft_indexes.end()) {
    return iter->second;
  } else {
    return nullptr;
  }
}

void MetaInfo::DeleteVertexFullTextIndex(const std::string& name) {
  std::unique_lock lock(mutex_);
  vertex_ft_indexes.erase(name);
}

void MetaInfo::ClearVertexFullTextIndexes() {
  std::unique_lock lock(mutex_);
  vertex_ft_indexes.clear();
}

std::shared_ptr<VertexPropertyIndex> MetaInfo::GetVertexPropertyIndex(
    uint32_t lid, uint32_t pid) {
  return GetVertexPropertyIndex(lid, std::vector<uint32_t>{pid});
}

std::shared_ptr<VertexPropertyIndex> MetaInfo::GetVertexPropertyIndex(
    uint32_t lid, const std::vector<uint32_t>& pids) {
  std::shared_lock lock(mutex_);
  auto iter = vertex_property_indexes_by_schema_.find(
      BuildVertexPropertyIndexKey(lid, pids));
  if (iter != vertex_property_indexes_by_schema_.end()) {
    return iter->second;
  } else {
    return nullptr;
  }
}

std::shared_ptr<VertexPropertyIndex> MetaInfo::GetBestVertexPropertyUniqueIndex(
    uint32_t lid, const std::unordered_set<uint32_t>& pids) {
  std::shared_lock lock(mutex_);
  std::shared_ptr<VertexPropertyIndex> best;
  for (const auto& [_, index] : vertex_property_indexes_by_name_) {
    if (!index->is_unique() || index->lid() != lid ||
        !index->AllPropertiesPresent(pids)) {
      continue;
    }
    if (!best || index->PropertyCount() > best->PropertyCount()) {
      best = index;
    }
  }
  return best;
}

std::shared_ptr<VertexPropertyIndex> MetaInfo::GetVertexPropertyIndex(
    const std::string& index_name) {
  std::shared_lock lock(mutex_);
  auto iter = vertex_property_indexes_by_name_.find(index_name);
  if (iter != vertex_property_indexes_by_name_.end()) {
    return iter->second;
  } else {
    return nullptr;
  }
}

std::vector<std::shared_ptr<VertexPropertyIndex>>
MetaInfo::GetVertexPropertyIndexes() {
  std::shared_lock lock(mutex_);
  std::vector<std::shared_ptr<VertexPropertyIndex>> indexes;
  indexes.reserve(vertex_property_indexes_by_name_.size());
  for (const auto& [_, index] : vertex_property_indexes_by_name_) {
    indexes.push_back(index);
  }
  return indexes;
}

std::shared_ptr<VertexVectorIndex> MetaInfo::GetVertexVectorIndex(
    uint32_t lid, uint32_t pid) {
  uint64_t index_key =
      (static_cast<uint64_t>(lid) << 32) | static_cast<uint64_t>(pid);
  std::shared_lock lock(mutex_);
  auto iter = vertex_vector_indexes.find(index_key);
  if (iter != vertex_vector_indexes.end()) {
    return iter->second;
  } else {
    return nullptr;
  }
}

std::vector<std::shared_ptr<VertexVectorIndex>>
MetaInfo::GetVertexVectorIndexes() {
  std::shared_lock lock(mutex_);
  std::vector<std::shared_ptr<VertexVectorIndex>> indexes;
  indexes.reserve(vertex_vector_indexes.size());
  for (const auto& [_, index] : vertex_vector_indexes) {
    indexes.push_back(index);
  }
  return indexes;
}

std::shared_ptr<VertexVectorIndex> MetaInfo::GetVertexVectorIndex(
    const std::string& name) {
  std::shared_lock lock(mutex_);
  for (const auto& [_, index] : vertex_vector_indexes) {
    if (index->meta().name() == name) {
      return index;
    }
  }
  return nullptr;
}

void MetaInfo::DeleteVertexVectorIndex(const std::string& name) {
  std::unique_lock lock(mutex_);
  for (auto iter = vertex_vector_indexes.begin();
       iter != vertex_vector_indexes.end(); iter++) {
    if (iter->second->meta().name() == name) {
      vertex_vector_indexes.erase(iter);
      break;
    }
  }
}

void MetaInfo::ClearVertexVectorIndexes() {
  std::unique_lock lock(mutex_);
  vertex_vector_indexes.clear();
}

bool MetaInfo::AddVertexPropertyIndex(
    std::shared_ptr<graphdb::VertexPropertyIndex> vpi) {
  auto name = vpi->meta().name();
  auto schema_key = BuildVertexPropertyIndexKey(vpi->lid(), vpi->pids());
  std::unique_lock lock(mutex_);
  if (vertex_property_indexes_by_name_.count(name) ||
      vertex_property_indexes_by_schema_.count(schema_key)) {
    return false;
  } else {
    vertex_property_indexes_by_schema_.emplace(schema_key, vpi);
    vertex_property_indexes_by_name_.emplace(std::move(name), std::move(vpi));
    return true;
  }
}

void MetaInfo::DeleteVertexPropertyIndex(const std::string& index_name) {
  std::unique_lock lock(mutex_);
  auto name_iter = vertex_property_indexes_by_name_.find(index_name);
  if (name_iter == vertex_property_indexes_by_name_.end()) {
    return;
  }
  vertex_property_indexes_by_schema_.erase(BuildVertexPropertyIndexKey(
      name_iter->second->lid(), name_iter->second->pids()));
  vertex_property_indexes_by_name_.erase(name_iter);
}

bool MetaInfo::AddVertexFullTextIndex(std::shared_ptr<VertexFullTextIndex> ft) {
  std::string name = ft->Name();
  std::unique_lock lock(mutex_);
  if (vertex_ft_indexes.count(name)) {
    return false;
  } else {
    vertex_ft_indexes.emplace(name, std::move(ft));
    return true;
  }
}

void MetaInfo::AddVertexVectorIndex(std::shared_ptr<VertexVectorIndex> vvi) {
  uint32_t lid = vvi->lid();
  uint32_t pid = vvi->pid();
  uint64_t index_key =
      (static_cast<uint64_t>(lid) << 32) | static_cast<uint64_t>(pid);
  std::unique_lock lock(mutex_);
  vertex_vector_indexes.emplace(index_key, std::move(vvi));
}

void MetaInfo::Init(rocksdb::TransactionDB* db,
                    boost::asio::io_service& service, GraphCF* graph_cf,
                    uint16_t server_id, size_t ft_commit_interval,
                    size_t ft_apply_batch_size, size_t ft_apply_max_delay_ms,
                    size_t vt_commit_interval) {
  id_generator_.Bind(db, graph_cf, server_id);
  uint32_t max_lid = 0;
  uint32_t max_pid = 0;
  uint32_t max_tid = 0;
  uint32_t max_index_id = 0;
  rocksdb::ReadOptions ro;
  std::unique_ptr<rocksdb::Iterator> iter(
      db->NewIterator(ro, graph_cf->meta_info));
  for (iter->SeekToFirst(); iter->Valid(); iter->Next()) {
    auto key = iter->key();
    if (key.empty()) {
      continue;
    }
    auto val = iter->value();
    auto prefix = static_cast<MetaDataType>(key.data()[0]);
    if (prefix == MetaDataType::VertexLabel ||
        prefix == MetaDataType::EdgeType || prefix == MetaDataType::Property) {
      std::string name(key.data() + 1, key.size() - 1);
      uint32_t id = ReadValue<uint32_t>(val.data());
      id_generator_.LoadToken(prefix, name, id);
      uint32_t native_id = big_to_native(id);
      if (prefix == MetaDataType::VertexLabel) {
        max_lid = std::max(max_lid, native_id);
      } else if (prefix == MetaDataType::EdgeType) {
        max_tid = std::max(max_tid, native_id);
      } else {
        max_pid = std::max(max_pid, native_id);
      }
      continue;
    }
    if (prefix == MetaDataType::VertexPropertyIndex) {
      meta::VertexPropertyIndex meta;
      bool ret = meta.ParseFromString(val.ToString());
      assert(ret);
      LOG_INFO("vertex property index: [{}]", meta.ShortDebugString());
      max_index_id = std::max(max_index_id, meta.index_id());
      uint32_t lid = native_to_big(meta.label_id());
      std::vector<uint32_t> pids;
      pids.reserve(meta.property_ids_size());
      for (auto pid : meta.property_ids()) {
        pids.push_back(native_to_big(pid));
      }
      uint32_t index_id = native_to_big(meta.index_id());
      auto vi = std::make_shared<VertexPropertyIndex>(
          meta, graph_cf->index, index_id, lid, std::move(pids));
      AddVertexPropertyIndex(std::move(vi));
      continue;
    }
    if (prefix == MetaDataType::VertexFullTextIndex) {
      meta::VertexFullTextIndex meta;
      bool ret = meta.ParseFromString(val.ToString());
      assert(ret);
      LOG_INFO("vertex fulltext index: [{}]", meta.ShortDebugString());
      max_index_id = std::max(max_index_id, meta.index_id());
      std::unordered_set<uint32_t> lids, pids;
      for (auto id : meta.label_ids()) {
        lids.insert(native_to_big(id));
      }
      for (auto id : meta.property_ids()) {
        pids.insert(native_to_big(id));
      }
      auto v_ft_index = std::make_shared<VertexFullTextIndex>(
          db, service, graph_cf, &id_generator_, meta,
          native_to_big(meta.index_id()), ft_apply_batch_size,
          ft_apply_max_delay_ms, lids, pids, ft_commit_interval);
      AddVertexFullTextIndex(v_ft_index);
      v_ft_index->Start();
      continue;
    }
    if (prefix == MetaDataType::VertexVectorIndex) {
      meta::VertexVectorIndex meta;
      bool ret = meta.ParseFromString(val.ToString());
      assert(ret);
      LOG_INFO("vertex vector index: [{}]", meta.ShortDebugString());
      max_index_id = std::max(max_index_id, meta.index_id());
      auto index = std::make_shared<VertexVectorIndex>(
          db, service, graph_cf, native_to_big(meta.index_id()),
          native_to_big(meta.label_id()), native_to_big(meta.property_id()),
          meta, vt_commit_interval);
      AddVertexVectorIndex(index);
      index->Start();
    }
  }
  id_generator_.SetMaxIds(max_lid, max_pid, max_tid, max_index_id);
}
}  // namespace graphdb
