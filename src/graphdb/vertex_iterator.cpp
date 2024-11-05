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
#include "transaction/transaction.h"
#include "common/logger.h"
#include <boost/endian/conversion.hpp>
#include "graph_db.h"
#include "bolt/connection.h"
using namespace txn;
namespace graphdb {
ScanVertexBylabel::ScanVertexBylabel(Transaction *txn, uint32_t lid)
    : VertexIterator(txn), lid_(lid) {
    rocksdb::ReadOptions ro;
    iter_.reset(
        txn->dbtxn()->GetIterator(ro, txn->db()->graph_cf().vertex_label_vid));
    iter_->Seek({(const char *)&lid_, sizeof(lid_)});
    if (iter_->Valid()) {
        auto key = iter_->key();
        if (lid_ == *(uint32_t *)key.data()) {
            key.remove_prefix(sizeof(uint32_t));
            valid_ = true;
            ve_ = std::make_unique<Vertex>(txn, *(int64_t *)(key.data()));
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
            if (lid_ == *(uint32_t *)key.data()) {
                key.remove_prefix(sizeof(uint32_t));
                valid_ = true;
                ve_ = std::make_unique<Vertex>(txn_, *(int64_t *)(key.data()));
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
                                           *(int64_t *)(iter_->key().data()));
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
                                           *(int64_t *)(iter_->key().data()));
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
    Transaction *txn, uint32_t lid, uint32_t pid, const Value &value,
    const std::unordered_map<uint32_t, Value> &other_props)
    : VertexIterator(txn) {
    std::string index_key = value.Serialize();
    auto vi = txn_->db()->meta_info().GetVertexPropertyIndex(lid, pid);
    if (!vi) {
        return;
    }
    rocksdb::ReadOptions ro;
    std::string index_val;
    auto s = txn_->dbtxn()->Get(ro, vi->cf(), index_key, &index_val);
    if (s.ok()) {
        int64_t vid = *(int64_t *)index_val.data();
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

GetVertexByFullTextIndex::GetVertexByFullTextIndex(
    Transaction *txn, const std::string &ft_index_name,
    const std::string &query, size_t top_n)
    : VertexScoreIterator(txn) {
    auto ft = txn->db()->meta_info().GetVertexFullTextIndex(ft_index_name);
    if (!ft) THROW_CODE(FullTextIndexNotFound, "No such fulltext index: {}", ft_index_name);
    result_ = ft->Query(query, top_n);
    if (!result_.empty()) {
        ve_ = std::make_unique<VertexScore>(
            Vertex(txn_, result_[iter_index_].id), result_[iter_index_].score);
        valid_ = true;
    }
}

void GetVertexByFullTextIndex::Next() {
    assert(valid_);
    valid_ = false;
    iter_index_++;
    if (iter_index_ < result_.size()) {
        ve_ = std::make_unique<VertexScore>(
            Vertex(txn_, result_[iter_index_].id), result_[iter_index_].score);
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

}