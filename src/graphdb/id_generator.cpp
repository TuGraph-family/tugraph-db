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
#include <boost/endian/conversion.hpp>
#include <iostream>
#include "common/exceptions.h"
#include "common/logger.h"
using namespace boost::endian;
namespace graphdb {
void IdGenerator::Init(rocksdb::TransactionDB *db, GraphCF *graph_cf) {
    int64_t max_vid = 0;
    int64_t max_eid = 0;
    uint32_t max_lid = 0;
    uint32_t max_tid = 0;
    uint32_t max_pid = 0;
    uint32_t max_index_id = 0;
    {
        auto iter = db->NewIterator({}, graph_cf->graph_topology);
        iter->SeekToLast();
        if (iter->Valid()) {
            max_vid = big_to_native((*(int64_t *)iter->key().data_));
        }
        delete iter;
    }
    {
        auto iter = db->NewIterator({}, graph_cf->name_id);
        for (iter->SeekToFirst(); iter->Valid(); iter->Next()) {
            auto key = iter->key();
            auto value = iter->value();
            std::string name(key.data() + 1, key.size() - 1);
            auto id = *(uint32_t *)(value.data());
            if (*key.data_ == static_cast<char>(TokenNameType::VertexLabel)) {
                vertex_labels_name_to_id_[name] = id;
                vertex_labels_id_to_name_[id] = name;
                max_lid = std::max(max_lid, big_to_native(id));
            } else if (*key.data_ ==
                       static_cast<char>(TokenNameType::EdgeType)) {
                edge_types_name_to_id_[name] = id;
                edge_types_id_to_name_[id] = name;
                max_tid = std::max(max_tid, big_to_native(id));
            } else if (*key.data_ ==
                       static_cast<char>(TokenNameType::Property)) {
                properties_name_to_id_[name] = id;
                properties_id_to_name_[id] = name;
                max_pid = std::max(max_pid, big_to_native(id));
            }
        }
        delete iter;
    }
    {
        std::vector<uint32_t> tids;
        tids.reserve(edge_types_name_to_id_.size());
        for (const auto &[name, id] : edge_types_name_to_id_) {
            tids.push_back(big_to_native(id));
        }
        std::sort(tids.begin(), tids.end());
        std::vector<int64_t> eids;
        auto iter = db->NewIterator({}, graph_cf->edge_type_eid);
        for (auto tid : tids) {
            auto next_tid = tid + 1;
            native_to_big_inplace(next_tid);
            iter->SeekForPrev({(const char *)(&next_tid), sizeof(next_tid)});
            if (iter->Valid()) {
                auto key = iter->key();
                if (big_to_native(*(uint32_t *)(key.data())) ==
                    tid) {
                    eids.push_back(big_to_native(*(int64_t *)(key.data() + sizeof(tid))));
                }
            }
        }
        delete iter;
        std::sort(eids.begin(), eids.end());
        if (!eids.empty()) {
            max_eid = eids.back();
        }
    }
    {
        auto iter = db->NewIterator({}, graph_cf->index);
        iter->SeekToLast();
        if (iter->Valid()) {
            max_index_id = big_to_native((*(uint32_t *)iter->key().data_));
        }
        delete iter;
    }
    LOG_INFO("max_vid:{}, max_eid:{}, max_lid:{}, max_pid:{}, max_tid:{}, max_index_id:{}",
                 max_vid, max_eid, max_lid, max_pid, max_tid, max_index_id);
    vertex_next_vid_ = max_vid + 1;
    edge_next_eid_ = max_eid + 1;
    label_next_lid_ = max_lid + 1;
    label_next_pid_ = max_pid + 1;
    label_next_tid_ = max_tid + 1;
    index_next_id_ = max_index_id + 1;

    db_ = db;
    graph_cf_ = graph_cf;
}

int64_t IdGenerator::GetNextVid() {
    return native_to_big(vertex_next_vid_++);
}

int64_t IdGenerator::GetNextEid() {
    return native_to_big(edge_next_eid_++);
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
        std::string key, val;
        char flag = static_cast<char>(TokenNameType::VertexLabel);
        key.append(1, flag);
        uint32_t bigendian_lid = native_to_big(label_next_lid_++);
        key.append(name);
        val.append((const char *)&bigendian_lid, sizeof(bigendian_lid));
        rocksdb::WriteOptions options;
        auto s = db_->Put(options, graph_cf_->name_id, key, val);
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
        std::string key, val;
        char flag = static_cast<char>(TokenNameType::EdgeType);
        key.append(1, flag);
        uint32_t bigendian_tid = native_to_big(label_next_tid_++);
        key.append(name);
        val.append((const char *)&bigendian_tid, sizeof(bigendian_tid));
        rocksdb::WriteOptions options;
        auto s = db_->Put(options, graph_cf_->name_id, key, val);
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
        std::string key, val;
        char flag = static_cast<char>(TokenNameType::Property);
        key.append(1, flag);
        uint32_t bigendian_pid = native_to_big(label_next_pid_++);
        key.append(name);
        val.append((const char *)&bigendian_pid, sizeof(bigendian_pid));
        rocksdb::WriteOptions options;
        auto s = db_->Put(options, graph_cf_->name_id, key, val);
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
}