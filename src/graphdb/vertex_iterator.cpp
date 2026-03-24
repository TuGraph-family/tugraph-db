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

#include "vertex_iterator.h"

#include <boost/endian/conversion.hpp>

#include "bolt/connection.h"
#include "common/byte_utils.h"
#include "common/logger.h"
#include "graph_db.h"
#include "transaction/transaction.h"
using namespace txn;
using common::AsChars;
using common::ReadValue;
namespace graphdb {

namespace {

int CompareKeyWithBoundPrefix(rocksdb::Slice key, const std::string &bound) {
  size_t common_size = std::min(key.size(), bound.size());
  int cmp = std::memcmp(key.data(), bound.data(), common_size);
  if (cmp < 0) {
    return -1;
  }
  if (cmp > 0) {
    return 1;
  }
  if (key.size() < bound.size()) {
    return -1;
  }
  return 0;
}

int64_t ReadPropertyIndexVid(const std::shared_ptr<VertexPropertyIndex> &index,
                             rocksdb::Slice key, rocksdb::Slice value) {
  if (index->is_unique()) {
    if (value.size() != sizeof(int64_t)) {
      THROW_CODE(StorageEngineError,
                 "vertex unique index stores invalid vid size");
    }
    return ReadValue<int64_t>(value.data());
  }
  if (key.size() < sizeof(uint32_t) + sizeof(int64_t)) {
    THROW_CODE(StorageEngineError,
               "vertex non-unique index stores invalid key size");
  }
  return ReadValue<int64_t>(key.data() + key.size() - sizeof(int64_t));
}

}  // namespace

ScanVertexBylabel::ScanVertexBylabel(Transaction *txn, uint32_t lid)
    : VertexIterator(txn), lid_(lid) {
  rocksdb::ReadOptions ro;
  iter_.reset(
      txn->dbtxn()->GetIterator(ro, txn->db()->graph_cf().vertex_label_vid));
  iter_->Seek({AsChars(lid_), sizeof(lid_)});
  if (iter_->Valid()) {
    auto key = iter_->key();
    if (lid_ == ReadValue<uint32_t>(key.data())) {
      key.remove_prefix(sizeof(uint32_t));
      valid_ = true;
      ve_ = std::make_unique<Vertex>(txn, ReadValue<int64_t>(key.data()));
    }
  }
}

void ScanVertexBylabel::Next() {
  if (txn_->conn() && txn_->conn()->has_closed()) {
    THROW_CODE(ConnectionDisconnected);
  }
  assert(valid_);
  valid_ = false;
  if (iter_->Valid()) {
    iter_->Next();
    if (iter_->Valid()) {
      auto key = iter_->key();
      if (lid_ == ReadValue<uint32_t>(key.data())) {
        key.remove_prefix(sizeof(uint32_t));
        valid_ = true;
        ve_ = std::make_unique<Vertex>(txn_, ReadValue<int64_t>(key.data()));
      }
    }
  }
}

bool ScanVertexBylabelProperties::MatchProperties() {
  bool match = true;
  for (auto &pair : properties_) {
    if (pair.second != iter_->GetVertex().GetProperty(pair.first)) {
      match = false;
      break;
    }
  }
  return match;
}

ScanVertexBylabelProperties::ScanVertexBylabelProperties(
    Transaction *txn, uint32_t lid,
    std::unordered_map<uint32_t, Value> properties)
    : VertexIterator(txn), properties_(std::move(properties)) {
  for (iter_ = std::make_unique<ScanVertexBylabel>(txn, lid); iter_->Valid();
       iter_->Next()) {
    if (MatchProperties()) {
      valid_ = true;
      break;
    }
  }
}

void ScanVertexBylabelProperties::Next() {
  assert(valid_);
  valid_ = false;
  while (iter_->Valid()) {
    iter_->Next();
    if (!iter_->Valid()) {
      break;
    }
    if (MatchProperties()) {
      valid_ = true;
      break;
    }
  }
}

ScanAllVertex::ScanAllVertex(Transaction *txn) : VertexIterator(txn) {
  rocksdb::ReadOptions ro;
  iter_.reset(
      txn->dbtxn()->GetIterator(ro, txn->db()->graph_cf().graph_topology));
  valid_ = false;
  for (iter_->SeekToFirst(); iter_->Valid(); iter_->Next()) {
    if (iter_->key().size() == sizeof(int64_t)) {
      valid_ = true;
      ve_ = std::make_unique<Vertex>(txn,
                                     ReadValue<int64_t>(iter_->key().data()));
      break;
    }
  }
}

void ScanAllVertex::Next() {
  if (txn_->conn() && txn_->conn()->has_closed()) {
    THROW_CODE(ConnectionDisconnected);
  }
  assert(valid_);
  valid_ = false;
  if (!iter_) {
    return;
  }
  while (iter_->Valid()) {
    iter_->Next();
    if (iter_->Valid() && iter_->key().size() == sizeof(int64_t)) {
      valid_ = true;
      ve_ = std::make_unique<Vertex>(txn_,
                                     ReadValue<int64_t>(iter_->key().data()));
      break;
    }
  }
}

bool ScanVertexByProperties::MatchProperties() {
  bool match = true;
  for (auto &pair : properties_) {
    if (pair.second != iter_->GetVertex().GetProperty(pair.first)) {
      match = false;
      break;
    }
  }
  return match;
}

ScanVertexByProperties::ScanVertexByProperties(
    Transaction *txn, std::unordered_map<uint32_t, Value> properties)
    : VertexIterator(txn), properties_(std::move(properties)) {
  for (iter_ = std::make_unique<ScanAllVertex>(txn); iter_->Valid();
       iter_->Next()) {
    if (MatchProperties()) {
      valid_ = true;
      break;
    }
  }
}

void ScanVertexByProperties::Next() {
  assert(valid_);
  valid_ = false;
  while (iter_->Valid()) {
    iter_->Next();
    if (!iter_->Valid()) {
      break;
    }
    if (MatchProperties()) {
      valid_ = true;
      break;
    }
  }
}

GetVertexByUniqueIndex::GetVertexByUniqueIndex(
    Transaction *txn, std::shared_ptr<VertexPropertyIndex> index,
    std::vector<Value> values, std::unordered_map<uint32_t, Value> other_props)
    : VertexIterator(txn), index_(std::move(index)) {
  if (!index_ || !index_->is_unique()) {
    return;
  }
  rocksdb::ReadOptions ro;
  std::string index_val;
  std::string index_key = index_->IndexKey(values);
  auto s = txn_->dbtxn()->Get(ro, index_->cf(), index_key, &index_val);
  if (s.ok()) {
    int64_t vid = ReadValue<int64_t>(index_val.data());
    ve_ = std::make_unique<Vertex>(txn_, vid);
    valid_ = true;
    for (auto &[id, val] : other_props) {
      if (ve_->GetProperty(id) != val) {
        valid_ = false;
        break;
      }
    }
  } else if (!s.IsNotFound()) {
    THROW_CODE(StorageEngineError, s.ToString());
  }
}

GetVertexByPropertyIndex::GetVertexByPropertyIndex(
    txn::Transaction *txn, std::shared_ptr<VertexPropertyIndex> index,
    std::string prefix)
    : VertexIterator(txn),
      index_(std::move(index)),
      prefix_(std::move(prefix)) {
  rocksdb::ReadOptions ro;
  if (index_->is_unique()) {
    std::string index_val;
    auto s = txn_->dbtxn()->Get(ro, index_->cf(), prefix_, &index_val);
    if (s.ok()) {
      ve_ = std::make_unique<Vertex>(
          txn_, ReadPropertyIndexVid(index_, rocksdb::Slice(prefix_),
                                     rocksdb::Slice(index_val)));
      valid_ = true;
    } else if (!s.IsNotFound()) {
      THROW_CODE(StorageEngineError, s.ToString());
    }
    return;
  }
  iter_.reset(txn->dbtxn()->GetIterator(ro, index_->cf()));
  iter_->Seek(prefix_);
  SeekToNextValid();
}

void GetVertexByPropertyIndex::SeekToNextValid() {
  valid_ = false;
  while (iter_ && iter_->Valid() && iter_->key().starts_with(prefix_)) {
    ve_ = std::make_unique<Vertex>(
        txn_, ReadPropertyIndexVid(index_, iter_->key(), iter_->value()));
    valid_ = true;
    return;
  }
  if (iter_ && !iter_->status().ok()) {
    THROW_CODE(StorageEngineError, iter_->status().ToString());
  }
}

void GetVertexByPropertyIndex::Next() {
  if (txn_->conn() && txn_->conn()->has_closed()) {
    THROW_CODE(ConnectionDisconnected);
  }
  assert(valid_);
  valid_ = false;
  if (index_->is_unique()) {
    return;
  }
  iter_->Next();
  SeekToNextValid();
}

GetVertexByPropertyRange::GetVertexByPropertyRange(
    txn::Transaction *txn, std::shared_ptr<VertexPropertyIndex> index,
    std::optional<std::string> lower_key, std::optional<std::string> upper_key,
    bool left_closed, bool right_closed)
    : VertexIterator(txn),
      index_(std::move(index)),
      lower_key_(std::move(lower_key)),
      upper_key_(std::move(upper_key)),
      left_closed_(left_closed),
      right_closed_(right_closed) {
  index_prefix_.assign(AsChars(index_->index_id()), sizeof(index_->index_id()));
  rocksdb::ReadOptions ro;
  iter_.reset(txn->dbtxn()->GetIterator(ro, index_->cf()));
  if (lower_key_.has_value()) {
    iter_->Seek(*lower_key_);
  } else {
    iter_->Seek(index_prefix_);
  }
  SeekToNextValid();
}

void GetVertexByPropertyRange::SeekToNextValid() {
  valid_ = false;
  while (iter_ && iter_->Valid() && iter_->key().starts_with(index_prefix_)) {
    if (lower_key_.has_value()) {
      int cmp = CompareKeyWithBoundPrefix(iter_->key(), *lower_key_);
      if (cmp < 0 || (cmp == 0 && !left_closed_)) {
        iter_->Next();
        continue;
      }
    }
    if (upper_key_.has_value()) {
      int cmp = CompareKeyWithBoundPrefix(iter_->key(), *upper_key_);
      if (cmp > 0 || (cmp == 0 && !right_closed_)) {
        break;
      }
    }
    ve_ = std::make_unique<Vertex>(
        txn_, ReadPropertyIndexVid(index_, iter_->key(), iter_->value()));
    valid_ = true;
    return;
  }
  if (iter_ && !iter_->status().ok()) {
    THROW_CODE(StorageEngineError, iter_->status().ToString());
  }
}

void GetVertexByPropertyRange::Next() {
  if (txn_->conn() && txn_->conn()->has_closed()) {
    THROW_CODE(ConnectionDisconnected);
  }
  assert(valid_);
  valid_ = false;
  iter_->Next();
  SeekToNextValid();
}

GetVertexByFullTextIndex::GetVertexByFullTextIndex(
    Transaction *txn, const std::string &ft_index_name,
    const std::string &query, size_t top_n)
    : VertexScoreIterator(txn) {
  auto ft = txn->db()->meta_info().GetVertexFullTextIndex(ft_index_name);
  if (!ft)
    THROW_CODE(FullTextIndexNotFound, "No such fulltext index: {}",
               ft_index_name);
  result_ = ft->Query(query, top_n);
  if (!result_.empty()) {
    ve_ = std::make_unique<VertexScore>(Vertex(txn_, result_[iter_index_].id),
                                        result_[iter_index_].score);
    valid_ = true;
  }
}

void GetVertexByFullTextIndex::Next() {
  assert(valid_);
  valid_ = false;
  iter_index_++;
  if (iter_index_ < result_.size()) {
    ve_ = std::make_unique<VertexScore>(Vertex(txn_, result_[iter_index_].id),
                                        result_[iter_index_].score);
    valid_ = true;
  }
}

GetVertexByKnnSearch::GetVertexByKnnSearch(txn::Transaction *txn,
                                           const std::string &vector_index,
                                           const std::vector<float> &query,
                                           int top_k, int ef_search)
    : VertexScoreIterator(txn) {
  auto index = txn->db()->meta_info().GetVertexVectorIndex(vector_index);
  if (!index) {
    THROW_CODE(VectorIndexException, "No such vector index:{}", vector_index);
  }
  result_ = index->KnnSearch(query.data(), top_k, ef_search);
  if (!result_.empty()) {
    ve_ = std::make_unique<VertexScore>(
        Vertex(txn_, result_[iter_index_].first), result_[iter_index_].second);
    valid_ = true;
  }
}

void GetVertexByKnnSearch::Next() {
  assert(valid_);
  valid_ = false;
  iter_index_++;
  if (iter_index_ < result_.size()) {
    ve_ = std::make_unique<VertexScore>(
        Vertex(txn_, result_[iter_index_].first), result_[iter_index_].second);
    valid_ = true;
  }
}

}  // namespace graphdb
