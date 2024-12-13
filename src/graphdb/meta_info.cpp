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
#include <boost/endian/conversion.hpp>
#include <filesystem>
#include "common/exceptions.h"
#include "proto/meta.pb.h"
#include "common/logger.h"
using namespace boost::endian;
namespace graphdb {
std::unordered_map<std::string, std::unique_ptr<VertexFullTextIndex>> &
MetaInfo::GetVertexFullTextIndex() {
    return vertex_ft_indexes;
}

VertexFullTextIndex *MetaInfo::GetVertexFullTextIndex(const std::string &name) {
    auto iter = vertex_ft_indexes.find(name);
    if (iter != vertex_ft_indexes.end()) {
        return iter->second.get();
    } else {
        return nullptr;
    }
}

void MetaInfo::DeleteVertexFullTextIndex(const std::string &name) {
    vertex_ft_indexes.erase(name);
}

VertexPropertyIndex *MetaInfo::GetVertexPropertyIndex(uint32_t lid,
                                                      uint32_t pid) {
    uint64_t index_key =
        (static_cast<uint64_t>(lid) << 32) | static_cast<uint64_t>(pid);
    auto iter = vertex_property_indexes.find(index_key);
    if (iter != vertex_property_indexes.end()) {
        return &iter->second;
    } else {
        return nullptr;
    }
}

VertexPropertyIndex *MetaInfo::GetVertexPropertyIndex(
    const std::string& index_name) {
    for (auto& [_, index] : vertex_property_indexes) {
        if (index.meta().name() == index_name) {
            return &index;
        }
    }
    return nullptr;
}

std::unordered_map<uint64_t, VertexPropertyIndex> &
    MetaInfo::GetVertexPropertyIndex() {
    return vertex_property_indexes;
}

VertexVectorIndex *MetaInfo::GetVertexVectorIndex(uint32_t lid,
                                                      uint32_t pid) {
    uint64_t index_key =
        (static_cast<uint64_t>(lid) << 32) | static_cast<uint64_t>(pid);
    auto iter = vertex_vector_indexes.find(index_key);
    if (iter != vertex_vector_indexes.end()) {
        return iter->second.get();
    } else {
        return nullptr;
    }
}

std::unordered_map<uint64_t, std::unique_ptr<VertexVectorIndex>> &MetaInfo::GetVertexVectorIndex() {
    return vertex_vector_indexes;
}

VertexVectorIndex *MetaInfo::GetVertexVectorIndex(const std::string& name) {
    for (auto& [_, index] : vertex_vector_indexes) {
        if (index->meta().name() == name) {
            return index.get();
        }
    }
    return nullptr;
}

void MetaInfo::DeleteVertexVectorIndex(const std::string &name) {
    for (auto iter = vertex_vector_indexes.begin(); iter != vertex_vector_indexes.end(); iter++) {
        if (iter->second->meta().name() == name) {
            vertex_vector_indexes.erase(iter);
            break;
        }
    }
}

bool MetaInfo::AddVertexPropertyIndex(graphdb::VertexPropertyIndex &&vpi) {
    uint64_t index_key =
        (static_cast<uint64_t>(vpi.lid()) << 32) | static_cast<uint64_t>(vpi.pid());
    if (vertex_property_indexes.count(index_key)) {
        return false;
    } else {
        vertex_property_indexes.emplace(index_key, std::move(vpi));
        return true;
    }
}

void MetaInfo::DeleteVertexPropertyIndex(const std::string &index_name) {
    for (auto iter = vertex_property_indexes.begin();
         iter != vertex_property_indexes.end(); iter++) {
        if (iter->second.meta().name() == index_name) {
            vertex_property_indexes.erase(iter);
            break;
        }
    }
}

bool MetaInfo::AddVertexFullTextIndex(std::unique_ptr<VertexFullTextIndex> ft) {
    std::string name = ft->Name();
    if (vertex_ft_indexes.count(name)) {
        return false;
    } else {
        vertex_ft_indexes.emplace(name, std::move(ft));
        return true;
    }
}

void MetaInfo::AddVertexVectorIndex(std::unique_ptr<VertexVectorIndex> vvi) {
    uint32_t lid = vvi->lid();
    uint32_t pid = vvi->pid();
    uint64_t index_key =
        (static_cast<uint64_t>(lid) << 32) | static_cast<uint64_t>(pid);
    vertex_vector_indexes.emplace(index_key, std::move(vvi));
}

void MetaInfo::Init(rocksdb::TransactionDB *db,
                    boost::asio::io_service &service,
                    GraphCF *graph_cf,
                    IdGenerator* id_generator,
                    size_t ft_commit_interval,
                    size_t vt_commit_interval) {
    rocksdb::ReadOptions ro;
    std::unique_ptr<rocksdb::Iterator> iter(
        db->NewIterator(ro, graph_cf->meta_info));
    std::string prefix;
    prefix.append(1, static_cast<char>(MetaDataType::VertexPropertyIndex));
    for (iter->Seek(prefix); iter->Valid() && iter->key().starts_with(prefix);
         iter->Next()) {
        auto val = iter->value();
        meta::VertexPropertyIndex meta;
        bool ret = meta.ParseFromString(val.ToString());
        assert(ret);
        LOG_INFO("vertex property index: [{}]", meta.ShortDebugString());
        uint32_t lid = native_to_big(meta.label_id());
        uint32_t pid = native_to_big(meta.property_id());
        uint32_t index_id = native_to_big(meta.index_id());
        VertexPropertyIndex vi(meta, graph_cf->index, index_id, lid, pid);
        uint64_t index_key = (static_cast<uint64_t>(lid) << 32) | static_cast<uint64_t>(pid);
        vertex_property_indexes.emplace(index_key, std::move(vi));
    }
    prefix.clear();
    prefix.append(1, static_cast<char>(MetaDataType::VertexFullTextIndex));
    for (iter->Seek(prefix); iter->Valid() && iter->key().starts_with(prefix);
         iter->Next()) {
        auto val = iter->value();
        meta::VertexFullTextIndex meta;
        bool ret = meta.ParseFromString(val.ToString());
        assert(ret);
        LOG_INFO("vertex fulltext index: [{}]", meta.ShortDebugString());
        std::unordered_set<uint32_t> lids, pids;
        for(auto id : meta.label_ids()) {
            lids.insert(native_to_big(id));
        }
        for(auto id : meta.property_ids()) {
            pids.insert(native_to_big(id));
        }
        auto v_ft_index = std::make_unique<VertexFullTextIndex>(
            db, service, graph_cf, id_generator, meta,
            native_to_big(meta.index_id()), lids, pids, ft_commit_interval);
        vertex_ft_indexes.emplace(meta.name(), std::move(v_ft_index));
    }
    prefix.clear();
    prefix.append(1, static_cast<char>(MetaDataType::VertexVectorIndex));
    for (iter->Seek(prefix); iter->Valid() && iter->key().starts_with(prefix);
         iter->Next()) {
        auto val = iter->value();
        meta::VertexVectorIndex meta;
        bool ret = meta.ParseFromString(val.ToString());
        assert(ret);
        LOG_INFO("vertex vector index: [{}]", meta.ShortDebugString());
        auto index = std::make_unique<VertexVectorIndex>(
            db, service, graph_cf, native_to_big(meta.index_id()), native_to_big(meta.label_id()),
            native_to_big(meta.property_id()), meta, vt_commit_interval);
        AddVertexVectorIndex(std::move(index));
    }
}
}