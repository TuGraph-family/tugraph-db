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
#include <boost/endian/conversion.hpp>
#include <unordered_set>
#include "common/value.h"
#include "edge_direction.h"

namespace txn {
class Transaction;
}
namespace graphdb {
class Property {
   public:
    virtual Value GetProperty(const std::string&) = 0;
    virtual Value GetProperty(uint32_t) = 0;
    virtual std::unordered_map<std::string, Value> GetAllProperty() = 0;

    virtual void SetProperties(const std::unordered_map<std::string, Value>&) = 0;

    virtual void RemoveProperty(const std::string&) = 0;
    virtual void RemoveAllProperty() = 0;
};

class EdgeIterator;

class Vertex : Property {
   public:
    Vertex(txn::Transaction* txn, int64_t id) : txn_(txn), id_(id) {}
    [[nodiscard]] int64_t GetId() const { return id_; };
    [[nodiscard]] int64_t GetNativeId() const {
        return boost::endian::big_to_native(id_);
    };
    [[nodiscard]] std::string_view GetIdView() const {
        return {(const char*)&id_, sizeof(id_)};
    };

    std::unordered_set<uint32_t> GetLabelIds();
    std::unordered_set<std::string> GetLabels();
    void AddLabels(const std::unordered_set<std::string>& labels);
    void DeleteLabels(const std::unordered_set<std::string>& labels);
    int Delete();
    std::unique_ptr<EdgeIterator> NewEdgeIterator(
        EdgeDirection direction, const std::unordered_set<std::string>& types,
        const std::unordered_map<std::string, Value>& props);
    std::unique_ptr<EdgeIterator> NewEdgeIterator(
        EdgeDirection direction, const std::unordered_set<std::string>& types,
        const std::unordered_map<std::string, Value>& props,
        const std::unordered_set<std::string>& other_node_labels,
        const std::unordered_map<std::string, Value>& other_node_props);
    std::unique_ptr<EdgeIterator> NewEdgeIterator(
        EdgeDirection direction, const std::string& type,
        const std::unordered_map<std::string, Value>& props,
        const Vertex& other_node);
    std::unique_ptr<EdgeIterator> NewEdgeIterator(
        EdgeDirection direction, const std::unordered_set<std::string>& types,
        const std::unordered_map<std::string, Value>& props,const Vertex& other_node);
    bool operator==(const Vertex& v) const { return id_ == v.id_; }

    Value GetProperty(const std::string&) override;
    Value GetProperty(uint32_t) override;
    std::unordered_map<std::string, Value> GetAllProperty() override;
    int GetDegree(EdgeDirection direction);
    void SetProperties(const std::unordered_map<std::string, Value>& properties) override;
    void RemoveProperty(const std::string&) override;
    void RemoveAllProperty() override;
    virtual ~Vertex() = default;

   private:
    void Lock();
    txn::Transaction* txn_ = nullptr;
    int64_t id_;
};

class Edge : public Property {
   public:
    Edge(txn::Transaction* txn, int64_t id, int64_t startId, int64_t endId,
         uint32_t typeId)
        : txn_(txn),
          id_(id),
          startId_(startId),
          endId_(endId),
          typeId_(typeId) {}

    [[nodiscard]] int64_t GetId() const { return id_; };
    [[nodiscard]] int64_t GetNativeId() const {
        return boost::endian::big_to_native(id_);
    };
    [[nodiscard]] Vertex GetStart() const { return {txn_, startId_}; };
    [[nodiscard]] int64_t GetStartId() const { return startId_; };
    [[nodiscard]] int64_t GetNativeStartId() const {
        return boost::endian::big_to_native(startId_);
    };
    [[nodiscard]] Vertex GetEnd() const { return {txn_, endId_}; };
    [[nodiscard]] int64_t GetEndId() const { return endId_; };
    [[nodiscard]] int64_t GetNativeEndId() const {
        return boost::endian::big_to_native(endId_);
    };
    [[nodiscard]] uint32_t GetTypeId() const { return typeId_; };
    [[nodiscard]] Vertex GetOtherEnd(int64_t vid) const;
    std::string GetType();
    void Delete();

    bool operator==(const Edge& e) const {
        return (id_ == e.id_) && (startId_ == e.startId_) &&
               (endId_ == e.endId_) && (typeId_ == e.typeId_);
    }

    Value GetProperty(const std::string&) override;
    Value GetProperty(uint32_t) override;
    std::unordered_map<std::string, Value> GetAllProperty() override;
    void SetProperties(const std::unordered_map<std::string, Value>& properties) override;
    void RemoveProperty(const std::string&) override;
    void RemoveAllProperty() override;
    virtual ~Edge() = default;

   private:
    void Lock();
    txn::Transaction* txn_ = nullptr;
    int64_t id_;
    int64_t startId_;
    int64_t endId_;
    uint32_t typeId_;
};
}
