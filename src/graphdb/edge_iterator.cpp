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

#include "edge_iterator.h"
#include "transaction/transaction.h"
#include "common/logger.h"
#include <boost/endian/conversion.hpp>
#include "graph_db.h"
#include "bolt/connection.h"
namespace graphdb {
ScanEdgeByVidDirectionTypes::ScanEdgeByVidDirectionTypes(
    txn::Transaction *txn, int64_t vid, EdgeDirection direction,
    std::unordered_set<uint32_t> types)
    : EdgeIterator(txn),
      vid_(vid),
      direction_(direction),
      types_(std::move(types)) {
    rocksdb::ReadOptions ro;
    iter_.reset(
        txn_->dbtxn()->GetIterator(ro, txn->db()->graph_cf().graph_topology));
    if (direction_ == EdgeDirection::OUTGOING ||
        direction_ == EdgeDirection::BOTH) {
        std::string prefix;
        prefix.append((const char *)&vid_, sizeof(vid_)).append(1, 0);
        if (types_.empty()) {
            prefixes_.push(prefix);
        } else {
            for (auto type : types_) {
                std::string tmp = prefix;
                tmp.append(((const char *)&type), sizeof(type));
                prefixes_.push(std::move(tmp));
            }
        }
    }
    if (direction_ == EdgeDirection::INCOMING ||
        direction_ == EdgeDirection::BOTH) {
        std::string prefix;
        prefix.append((const char *)&vid_, sizeof(vid_)).append(1, 1);
        if (types_.empty()) {
            prefixes_.push(prefix);
        } else {
            for (auto type : types_) {
                std::string tmp = prefix;
                tmp.append(((const char *)&type), sizeof(type));
                prefixes_.push(std::move(tmp));
            }
        }
    }
    SeekToNextPrefix();
}

void ScanEdgeByVidDirectionTypes::Load() {
    auto p = iter_->key().data();
    int64_t vid1 = *(int64_t *)p;
    p += sizeof(int64_t);
    auto dir = static_cast<EdgeDirection>(*(p));
    p += sizeof(char);
    uint32_t etid = *(uint32_t *)p;
    p += sizeof(uint32_t);
    int64_t vid2 = *(int64_t *)p;
    p += sizeof(int64_t);
    int64_t eid = *(int64_t *)p;
    if (dir == EdgeDirection::OUTGOING) {
        ee_ = std::make_unique<Edge>(txn_, eid, vid1, vid2, etid);
    } else {
        ee_ = std::make_unique<Edge>(txn_, eid, vid2, vid1, etid);
    }
}

void ScanEdgeByVidDirectionTypes::SeekToNextPrefix() {
    while (!prefixes_.empty()) {
        prefix_ = prefixes_.front();
        prefixes_.pop();
        iter_->Seek(prefix_);
        if (iter_->Valid()) {
            auto key = iter_->key();
            if (key.starts_with(prefix_)) {
                valid_ = true;
                Load();
                return;
            }
        }
    }
}

void ScanEdgeByVidDirectionTypes::Next() {
    if (txn_->conn() && txn_->conn()->has_closed()) {
        THROW_CODE(ConnectionDisconnected);
    }
    assert(valid_);
    valid_ = false;
    iter_->Next();
    if (iter_->Valid()) {
        auto key = iter_->key();
        if (key.starts_with(prefix_)) {
            valid_ = true;
            Load();
            return;
        }
    }
    SeekToNextPrefix();
}

bool ScanEdgeByVidDirectionTypesProperties::MatchProperties() {
    bool match = true;
    for (auto &pair : properties_) {
        if (pair.second != iter_->GetEdge().GetProperty(pair.first)) {
            match = false;
            break;
        }
    }
    return match;
}

ScanEdgeByVidDirectionTypesProperties::ScanEdgeByVidDirectionTypesProperties(
    txn::Transaction *txn, int64_t vid, EdgeDirection direction,
    std::unordered_set<uint32_t> types,
    std::unordered_map<uint32_t, Value> properties)
    : EdgeIterator(txn), properties_(std::move(properties)) {
    for (iter_ = std::make_unique<ScanEdgeByVidDirectionTypes>(
             txn, vid, direction, std::move(types));
         iter_->Valid(); iter_->Next()) {
        if (MatchProperties()) {
            valid_ = true;
            break;
        }
    }
}

void ScanEdgeByVidDirectionTypesProperties::Next() {
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

ScanEdgeByVidDirectionTypesPropertiesOtherVid::ScanEdgeByVidDirectionTypesPropertiesOtherVid(
    txn::Transaction *txn, int64_t vid, graphdb::EdgeDirection direction,
    std::unordered_set<uint32_t> types,
    std::unordered_map<uint32_t, Value> properties,
    const graphdb::Vertex &other) : EdgeIterator(txn), vid_(vid), other_node_(other) {
    for (iter_ = std::make_unique<ScanEdgeByVidDirectionTypesProperties>(
             txn, vid, direction, std::move(types), std::move(properties));
         iter_->Valid(); iter_->Next()) {
        auto other_end = iter_->GetEdge().GetOtherEnd(vid_);
        if (other_end == other_node_) {
            valid_ = true;
            break;
        }
    }
}

void ScanEdgeByVidDirectionTypesPropertiesOtherVid::Next() {
    assert(valid_);
    valid_ = false;
    while (iter_->Valid()) {
        iter_->Next();
        if (!iter_->Valid()) {
            break;
        }
        auto other_end = iter_->GetEdge().GetOtherEnd(vid_);
        if (other_end == other_node_) {
            valid_ = true;
            break;
        }
    }
}

ScanEdgeByVidDirectionTypesPropertiesOtherNode::
    ScanEdgeByVidDirectionTypesPropertiesOtherNode(
        txn::Transaction *txn, int64_t vid, EdgeDirection direction,
        std::unordered_set<uint32_t> types,
        std::unordered_map<uint32_t, Value> properties,
        std::unordered_set<uint32_t> other_node_labels,
        std::unordered_map<uint32_t, Value> other_node_properties)
    : EdgeIterator(txn),
      vid_(vid),
      other_node_labels_(std::move(other_node_labels)),
      other_node_properties_(std::move(other_node_properties)) {
    for (iter_ = std::make_unique<ScanEdgeByVidDirectionTypesProperties>(
             txn, vid, direction, std::move(types), std::move(properties));
         iter_->Valid(); iter_->Next()) {
        auto other_end = iter_->GetEdge().GetOtherEnd(vid_);
        if (MatchOtherEnd(other_end)) {
            valid_ = true;
            break;
        }
    }
}

void ScanEdgeByVidDirectionTypesPropertiesOtherNode::Next() {
    assert(valid_);
    valid_ = false;
    while (iter_->Valid()) {
        iter_->Next();
        if (!iter_->Valid()) {
            break;
        }
        auto other_end = iter_->GetEdge().GetOtherEnd(vid_);
        if (MatchOtherEnd(other_end)) {
            valid_ = true;
            break;
        }
    }
}

bool ScanEdgeByVidDirectionTypesPropertiesOtherNode::MatchOtherEnd(
    Vertex &other_end) {
    if (!other_node_labels_.empty()) {
        auto lids = other_end.GetLabelIds();
        bool found = std::any_of(lids.begin(), lids.end(), [this](uint32_t id) {
            return other_node_labels_.count(id) > 0;
        });
        if (!found) {
            return false;
        }
    }
    if (!other_node_properties_.empty()) {
        bool match = true;
        for (auto &pair : other_node_properties_) {
            if (pair.second != other_end.GetProperty(pair.first)) {
                match = false;
                break;
            }
        }
        if (!match) {
            return false;
        }
    }
    return true;
}

bool ScanEdgeByVidDirectionTypePropertiesOtherNode::MatchProperties() {
    bool match = true;
    for (auto &pair : properties_) {
        if (pair.second != iter_->GetEdge().GetProperty(pair.first)) {
            match = false;
            break;
        }
    }
    return match;
}

ScanEdgeByVidDirectionTypePropertiesOtherNode::
    ScanEdgeByVidDirectionTypePropertiesOtherNode(
        txn::Transaction *txn, int64_t vid, EdgeDirection direction,
        uint32_t type, std::unordered_map<uint32_t, Value> properties,
        const Vertex &other_node)
    : EdgeIterator(txn),
      vid_(vid),
      properties_(std::move(properties)),
      other_node_(other_node) {
    for (iter_ = std::make_unique<ScanEdgeByVidDirectionTypes>(
             txn, vid, direction, std::unordered_set<uint32_t>{type});
         iter_->Valid(); iter_->Next()) {
        auto other = iter_->GetEdge().GetOtherEnd(vid_);
        if (other == other_node_ && MatchProperties()) {
            valid_ = true;
            break;
        }
    }
}

void ScanEdgeByVidDirectionTypePropertiesOtherNode::Next() {
    assert(valid_);
    valid_ = false;
    while (iter_->Valid()) {
        iter_->Next();
        if (!iter_->Valid()) {
            break;
        }
        auto other = iter_->GetEdge().GetOtherEnd(vid_);
        if (other == other_node_ && MatchProperties()) {
            valid_ = true;
            break;
        }
    }
}
}
