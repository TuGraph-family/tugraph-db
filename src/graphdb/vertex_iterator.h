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
#include "graph_entity.h"
#include "ftindex/include/lib.rs.h"
namespace graphdb {
class VertexIterator : public Iterator {
   public:
    explicit VertexIterator(txn::Transaction* txn) : Iterator(txn) {}
    virtual Vertex& GetVertex() = 0;
};

class NoVertexFound : public VertexIterator {
   public:
    explicit NoVertexFound(txn::Transaction* txn) : VertexIterator(txn) {
        valid_ = false;  // always false
    }
    Vertex& GetVertex() override {
        assert(valid_);
        return *ve_;
    };
    void Next() override {}

   private:
    std::unique_ptr<Vertex> ve_;
};

class ScanVertexBylabel : public VertexIterator {
   public:
    ScanVertexBylabel(txn::Transaction* txn, uint32_t lid);
    void Next() override;
    Vertex& GetVertex() override {
        assert(valid_);
        return *ve_;
    }

   private:
    uint32_t lid_ = 0;
    std::unique_ptr<rocksdb::Iterator> iter_;
    std::unique_ptr<Vertex> ve_;
};

class ScanVertexBylabelProperties : public VertexIterator {
   public:
    ScanVertexBylabelProperties(txn::Transaction* txn, uint32_t lid,
                                std::unordered_map<uint32_t, Value> properties);
    void Next() override;
    Vertex& GetVertex() override { return iter_->GetVertex(); }

   private:
    bool MatchProperties();
    std::unique_ptr<ScanVertexBylabel> iter_;
    std::unordered_map<uint32_t, Value> properties_;
};

class ScanAllVertex : public VertexIterator {
   public:
    explicit ScanAllVertex(txn::Transaction* txn);
    void Next() override;
    Vertex& GetVertex() override {
        assert(valid_);
        return *ve_;
    }

   private:
    std::unique_ptr<rocksdb::Iterator> iter_;
    std::unique_ptr<Vertex> ve_;
};

class ScanVertexByProperties : public VertexIterator {
   public:
    ScanVertexByProperties(txn::Transaction* txn,
                           std::unordered_map<uint32_t, Value> properties);
    void Next() override;
    Vertex& GetVertex() override { return iter_->GetVertex(); }

   private:
    bool MatchProperties();
    std::unique_ptr<ScanAllVertex> iter_;
    std::unordered_map<uint32_t, Value> properties_;
};

class GetVertexByUniqueIndex : public VertexIterator {
   public:
    GetVertexByUniqueIndex(
        txn::Transaction* txn, uint32_t lid, uint32_t pid, const Value& value,
        const std::unordered_map<uint32_t, Value>& other_props);
    void Next() override {valid_ = false;};
    Vertex& GetVertex() override {
        assert(valid_);
        return *ve_;
    }

   private:
    std::unique_ptr<Vertex> ve_;
};

struct VertexScore {
    Vertex vertex;
    float score;
    VertexScore(Vertex v, float s) : vertex(std::move(v)), score(s) {}
};

class VertexScoreIterator : public Iterator {
   public:
    explicit VertexScoreIterator(txn::Transaction* txn) : Iterator(txn) {}
    virtual VertexScore& GetVertexScore() = 0;
};

class GetVertexByFullTextIndex : public VertexScoreIterator {
   public:
    GetVertexByFullTextIndex(txn::Transaction* txn,
                             const std::string& ft_index_name,
                             const std::string& query, size_t top_n);
    void Next() override;
    VertexScore& GetVertexScore() override {
        assert(valid_);
        return *ve_;
    }

   private:
    ::rust::Vec<::IdScore> result_;
    size_t iter_index_ = 0;
    std::unique_ptr<VertexScore> ve_;
};

class GetVertexByKnnSearch : public VertexScoreIterator {
   public:
    GetVertexByKnnSearch(txn::Transaction* txn,
                         const std::string& vector_index,
                         const std::vector<float>& query,
                         int top_k, int ef_search);
    void Next() override;
    VertexScore& GetVertexScore() override {
        assert(valid_);
        return *ve_;
    }

   private:
    std::vector<std::pair<int64_t, float>> result_;
    size_t iter_index_ = 0;
    std::unique_ptr<VertexScore> ve_;
};


}