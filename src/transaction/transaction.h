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

#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

#include "common/value.h"
#include "graphdb/edge_direction.h"
#include "graphdb/edge_iterator.h"
#include "graphdb/graph_cf.h"
#include "graphdb/id_generator.h"
#include "graphdb/meta_info.h"
#include "graphdb/vertex_iterator.h"
namespace graphdb {
class GraphDB;
}
namespace bolt {
class BoltConnection;
}
class ResultIterator;
namespace txn {
class Transaction {
 public:
  // No copying allowed
  Transaction(const Transaction&) = delete;
  void operator=(const Transaction&) = delete;

  Transaction(rocksdb::Transaction* txn, graphdb::GraphDB* graph_db)
      : txn_(txn), db_(graph_db) {}
  ~Transaction() { delete txn_; }
  graphdb::Vertex CreateVertex(
      const std::unordered_set<std::string>& labels,
      const std::unordered_map<std::string, Value>& values);
  graphdb::Edge CreateEdge(
      const graphdb::Vertex& start, const graphdb::Vertex& end,
      const std::string& type,
      const std::unordered_map<std::string, Value>& values);
  graphdb::Vertex GetVertexById(int64_t vid);
  graphdb::Edge GetEdgeById(uint32_t etid, int64_t eid);
  std::unique_ptr<graphdb::VertexIterator> NewVertexIterator();
  std::unique_ptr<graphdb::VertexIterator> NewVertexIterator(
      const std::string& label);
  std::unique_ptr<graphdb::VertexIterator> NewVertexIterator(
      const std::optional<std::string>& label,
      const std::optional<std::unordered_map<std::string, Value>>& props);
  // for debug
  std::string GetVertexIteratorInfo(
      const std::optional<std::string>& label,
      const std::optional<std::unordered_set<std::string>>& props);
  std::unique_ptr<graphdb::VertexIterator> QueryVertexByPropertyIndex(
      const std::string& index_name, const Value& query);
  std::unique_ptr<graphdb::VertexIterator> QueryVertexByPropertyRange(
      const std::string& index_name, const std::optional<Value>& lower,
      const std::optional<Value>& upper, bool left_closed, bool right_closed);
  std::unique_ptr<graphdb::VertexScoreIterator> QueryVertexByFTIndex(
      const std::string& index_name, const std::string& query, size_t top_n);
  std::unique_ptr<graphdb::VertexScoreIterator> QueryVertexByKnnSearch(
      const std::string& index_name, const std::vector<float>& query, int top_k,
      int ef_search);
  void AppendFullTextIndexWAL(
      std::shared_ptr<graphdb::VertexFullTextIndex> index,
      const meta::FullTextIndexUpdate& update);
  std::unique_ptr<ResultIterator> Execute(void* ctx, const std::string& cypher);
  void AppendVectorIndexWAL(std::shared_ptr<graphdb::VertexVectorIndex> index,
                            std::string payload) {
    pending_vector_wals_.push_back({std::move(index), std::move(payload)});
  }
  void Commit();
  void Rollback();
  graphdb::GraphDB* db() { return db_; }
  rocksdb::Transaction* dbtxn() { return txn_; };
  void SetConn(const std::shared_ptr<bolt::BoltConnection>& conn) {
    conn_ = conn;
  }
  std::shared_ptr<bolt::BoltConnection>& conn() { return conn_; }

 private:
  struct PendingFullTextWALKey {
    graphdb::VertexFullTextIndex* index = nullptr;
    int64_t vid = 0;

    bool operator==(const PendingFullTextWALKey& other) const {
      return index == other.index && vid == other.vid;
    }
  };

  struct PendingFullTextWALKeyHash {
    size_t operator()(const PendingFullTextWALKey& key) const {
      size_t hash = std::hash<graphdb::VertexFullTextIndex*>{}(key.index);
      hash ^= std::hash<int64_t>{}(key.vid) + 0x9e3779b9 + (hash << 6) +
              (hash >> 2);
      return hash;
    }
  };

  struct PendingFullTextWAL {
    std::shared_ptr<graphdb::VertexFullTextIndex> index;
    meta::FullTextIndexUpdate update;
  };

  struct PendingVectorWAL {
    std::shared_ptr<graphdb::VertexVectorIndex> index;
    std::string payload;
  };

  rocksdb::Transaction* txn_;
  graphdb::GraphDB* db_;
  std::shared_ptr<bolt::BoltConnection> conn_;
  std::vector<PendingFullTextWAL> pending_fulltext_wals_;
  std::unordered_map<PendingFullTextWALKey, size_t, PendingFullTextWALKeyHash>
      pending_fulltext_wal_positions_;
  std::vector<PendingVectorWAL> pending_vector_wals_;
};

}  // namespace txn
