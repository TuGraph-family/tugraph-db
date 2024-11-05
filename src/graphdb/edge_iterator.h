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
#include <rocksdb/utilities/transaction.h>
#include "iterator.h"
#include "edge_direction.h"
#include "graph_entity.h"
#include <queue>
namespace graphdb {
class EdgeIterator : public Iterator {
   public:
    explicit EdgeIterator(txn::Transaction* txn) : Iterator(txn) {}
    virtual Edge& GetEdge() = 0;
};

class NoEdgeFound : public EdgeIterator {
   public:
    explicit NoEdgeFound(txn::Transaction* txn) : EdgeIterator(txn) {
        valid_ = false;  // always false
    }
    void Next() override {}
    Edge& GetEdge() override {
        assert(valid_);
        return *ee_;
    };

   private:
    std::unique_ptr<Edge> ee_;
};

class ScanEdgeByVidDirectionTypes : public EdgeIterator {
   public:
    ScanEdgeByVidDirectionTypes(txn::Transaction* txn, int64_t vid,
                                EdgeDirection direction,
                                std::unordered_set<uint32_t> types);
    void Next() override;
    Edge& GetEdge() override {
        assert(valid_);
        return *ee_;
    };

   private:
    void Load();
    void SeekToNextPrefix();
    int64_t vid_;
    EdgeDirection direction_;
    std::unordered_set<uint32_t> types_;
    std::unique_ptr<rocksdb::Iterator> iter_;
    std::string prefix_;
    std::queue<std::string> prefixes_;
    std::unique_ptr<Edge> ee_;
};

class ScanEdgeByVidDirectionTypesProperties : public EdgeIterator {
   public:
    ScanEdgeByVidDirectionTypesProperties(
        txn::Transaction* txn, int64_t vid, EdgeDirection direction,
        std::unordered_set<uint32_t> types,
        std::unordered_map<uint32_t, Value> properties);
    void Next() override;
    Edge& GetEdge() override { return iter_->GetEdge(); };

   private:
    bool MatchProperties();
    std::unique_ptr<ScanEdgeByVidDirectionTypes> iter_;
    std::unordered_map<uint32_t, Value> properties_;
};

class ScanEdgeByVidDirectionTypesPropertiesOtherNode : public EdgeIterator {
   public:
    ScanEdgeByVidDirectionTypesPropertiesOtherNode(
        txn::Transaction* txn, int64_t vid, EdgeDirection direction,
        std::unordered_set<uint32_t> types,
        std::unordered_map<uint32_t, Value> properties,
        std::unordered_set<uint32_t> other_node_labels,
        std::unordered_map<uint32_t, Value> other_node_properties);
    void Next() override;
    Edge& GetEdge() override { return iter_->GetEdge(); };

   private:
    bool MatchOtherEnd(Vertex& other_end);
    int64_t vid_;
    std::unique_ptr<ScanEdgeByVidDirectionTypesProperties> iter_;
    std::unordered_set<uint32_t> other_node_labels_;
    std::unordered_map<uint32_t, Value> other_node_properties_;
};

class ScanEdgeByVidDirectionTypesPropertiesOtherVid : public EdgeIterator {
   public:
    ScanEdgeByVidDirectionTypesPropertiesOtherVid(
        txn::Transaction* txn, int64_t vid, EdgeDirection direction,
        std::unordered_set<uint32_t> types,
        std::unordered_map<uint32_t, Value> properties, const Vertex& other);
    void Next() override;
    Edge& GetEdge() override { return iter_->GetEdge(); };

   private:
    int64_t vid_;
    std::unique_ptr<ScanEdgeByVidDirectionTypesProperties> iter_;
    const Vertex& other_node_;
};

class ScanEdgeByVidDirectionTypePropertiesOtherNode : public EdgeIterator {
   public:
    ScanEdgeByVidDirectionTypePropertiesOtherNode(
        txn::Transaction* txn, int64_t vid, EdgeDirection direction,
        uint32_t type, std::unordered_map<uint32_t, Value> properties,
        const Vertex& other_node);
    void Next() override;
    Edge& GetEdge() override { return iter_->GetEdge(); };

   private:
    bool MatchProperties();
    int64_t vid_;
    std::unordered_map<uint32_t, Value> properties_;
    const Vertex& other_node_;
    std::unique_ptr<ScanEdgeByVidDirectionTypes> iter_;
};
}