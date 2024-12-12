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

#include "transaction/transaction.h"

#include <rocksdb/utilities/write_batch_with_index.h>

#include <boost/endian/conversion.hpp>

#include "common/exceptions.h"
#include "cypher/execution_plan/result_iterator.h"
#include "graphdb/graph_db.h"
#include "common/logger.h"
using namespace graphdb;
using namespace boost::endian;
namespace txn {
Vertex Transaction::CreateVertex(
    const std::unordered_set<std::string>& labels,
    const std::unordered_map<std::string, Value>& values) {
    rocksdb::Status s;
    int64_t vid = db_->id_generator().GetNextVid();
    std::unordered_set<uint32_t> lids;
    std::string buffer;
    for (const auto& label : labels) {
        auto lid = db_->id_generator().GetOrCreateLid(label);
        lids.emplace(lid);
        buffer.clear();
        buffer.append((const char*)&lid, sizeof(lid));
        buffer.append((const char*)&vid, sizeof(vid));
        s = txn_->GetWriteBatch()->Put(db_->graph_cf().vertex_label_vid, buffer,
                                       {});
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }
    buffer.clear();
    for (auto lid : lids) {
        buffer.append((const char*)&lid, sizeof(lid));
    }
    s = txn_->GetWriteBatch()->Put(
        db_->graph_cf().graph_topology,
        rocksdb::Slice((const char*)&vid, sizeof(vid)), buffer);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    std::unordered_map<uint32_t, const Value*> pid_values;
    std::unordered_set<uint32_t> pids;
    for (const auto& [name, value] : values) {
        uint32_t pid = db_->id_generator().GetOrCreatePid(name);
        pid_values[pid] = &value;
        pids.insert(pid);
        buffer.clear();
        buffer.append((const char*)&vid, sizeof(vid));
        buffer.append((const char*)&pid, sizeof(pid));
        std::string val = value.Serialize();
        for (auto lid : lids) {
            auto vi = db_->meta_info().GetVertexPropertyIndex(lid, pid);
            if (vi) {
                vi->AddIndex(this, vid, val);
            }
        }
        s = txn_->GetWriteBatch()->Put(db_->graph_cf().vertex_property, buffer,
                                       val);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }
    if (db_->busy_index().Busy(lids, pids)) {
        THROW_CODE(IndexBusy);
    }
    // full text index
    for (auto& [ft_name, ft] : db_->meta_info().GetVertexFullTextIndex()) {
        if (!ft->MatchLabelIds(lids)) {
            continue;
        }
        meta::FullTextIndexUpdate add;
        add.set_type(meta::UpdateType::Add);
        add.set_vid(vid);
        for (const auto& [pid, prop] : pid_values) {
            if (prop->IsString() && !prop->AsString().empty() && ft->PropertyIds().count(pid)) {
                add.add_fields(db_->id_generator().GetPropertyName(pid).value());
                add.add_values(prop->AsString());
            }
        }
        if (!add.fields().empty()) {
            ft->AddIndex(this, vid, add);
        }
    }
    // vector index
    for (auto& [index_name, vvi] : db_->meta_info().GetVertexVectorIndex()) {
        if (!lids.count(vvi->lid())) {
            continue;
        }
        auto iter = pid_values.find(vvi->pid());
        if (iter == pid_values.end()) {
            continue;
        }
        auto prop = iter->second;
        if (!prop->IsArray()) {
            continue;
        }
        auto& array = prop->AsArray();
        if (array.empty() || (!array[0].IsDouble() && !array[0].IsFloat())) {
            continue;
        }
        if (array.size() != vvi->meta().dimensions()) {
            continue;
        }
        meta::VectorIndexUpdate add;
        add.set_type(meta::UpdateType::Add);
        for (auto& item : array) {
            if (item.IsFloat()) {
                add.add_vector(item.AsFloat());
            } else {
                add.add_vector((float)item.AsDouble());
            }
        }
        vvi->AddIndex(this, vid, add);
    }
    return {this, vid};
}

Edge Transaction::CreateEdge(
    const Vertex& start, const Vertex& end, const std::string& type,
    const std::unordered_map<std::string, Value>& values) {
    rocksdb::Status s;
    {
        rocksdb::ReadOptions ro;
        s = txn_->GetForUpdate(ro, db_->graph_cf().graph_topology,
                               start.GetIdView(), (std::string*)nullptr);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
        s = txn_->GetForUpdate(ro, db_->graph_cf().graph_topology,
                               end.GetIdView(), (std::string*)nullptr);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }

    int64_t eid = db_->id_generator().GetNextEid();
    uint32_t tid = db_->id_generator().GetOrCreateTid(type);
    std::string key, val;
    // out key
    key.append(start.GetIdView());
    key.append(1, 0);
    key.append((const char*)&tid, sizeof(tid));
    key.append(end.GetIdView());
    key.append((const char*)&eid, sizeof(eid));
    s = txn_->GetWriteBatch()->Put(db_->graph_cf().graph_topology, key, {});
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    // in key
    key.clear();
    key.append(end.GetIdView());
    key.append(1, 1);
    key.append((const char*)&tid, sizeof(tid));
    key.append(start.GetIdView());
    key.append((const char*)&eid, sizeof(eid));
    s = txn_->GetWriteBatch()->Put(db_->graph_cf().graph_topology, key, {});
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    // type
    key.clear();
    key.append((const char*)&tid, sizeof(tid));
    key.append((const char*)&eid, sizeof(eid));
    val.append(start.GetIdView());
    val.append(end.GetIdView());
    s = txn_->GetWriteBatch()->Put(db_->graph_cf().edge_type_eid, key, val);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    // properties
    for (const auto& [name, value] : values) {
        uint32_t pid = db_->id_generator().GetOrCreatePid(name);
        key.clear();
        key.append((const char*)&eid, sizeof(eid));
        key.append((const char*)&pid, sizeof(pid));
        val = value.Serialize();
        s = txn_->GetWriteBatch()->Put(db_->graph_cf().edge_property, key, val);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }
    return {this, eid, start.GetId(), end.GetId(), tid};
}

Vertex Transaction::GetVertexById(int64_t vid) {
    rocksdb::ReadOptions ro;
    std::string val;
    auto s = txn_->Get(ro, db_->graph_cf().graph_topology,
                       rocksdb::Slice((const char*)&vid, sizeof(vid)), &val);
    if (s.ok()) {
        return {this, vid};
    } else if (s.IsNotFound()) {
        THROW_CODE(VertexIdNotFound, "Vertex id {} not found", big_to_native(vid));
    } else {
        THROW_CODE(StorageEngineError, s.ToString());
    }
}

Edge Transaction::GetEdgeById(uint32_t etid, int64_t eid) {
    std::string key;
    key.append((const char*)&etid, sizeof(etid));
    key.append((const char*)&eid, sizeof(eid));
    rocksdb::ReadOptions ro;
    std::string val;
    auto s = txn_->Get(ro, db_->graph_cf().edge_type_eid, key, &val);
    if (s.ok()) {
        auto p = val.data();
        int64_t startId = *(int64_t*)p;
        p += sizeof(int64_t);
        int64_t endId = *(int64_t*)p;
        return {this, eid, startId, endId, etid};
    } else if (s.IsNotFound()) {
        THROW_CODE(EdgeIdNotFound, "Edge [etid:{},eid:{}] not found",
                   big_to_native(etid), big_to_native(eid));
    } else {
        THROW_CODE(StorageEngineError, s.ToString());
    }
}

std::unique_ptr<VertexIterator> Transaction::NewVertexIterator() {
    return std::make_unique<ScanAllVertex>(this);
}

std::unique_ptr<VertexIterator> Transaction::NewVertexIterator(
    const std::string& label) {
    auto lid = db_->id_generator().GetLid(label);
    if (lid.has_value()) {
        return std::make_unique<ScanVertexBylabel>(this, lid.value());
    } else {
        return std::make_unique<NoVertexFound>(this);
    }
}

std::string Transaction::GetVertexIteratorInfo(
    const std::optional<std::string>& label,
    const std::optional<std::unordered_set<std::string>>& props) {
    if (!label && !props) {
        return "ScanAllVertex";
    } else if (label && !props) {
        auto lid = db_->id_generator().GetLid(label.value());
        if (!lid.has_value()) {
            return "NoVertexFound";
        } else {
            return "ScanVertexBylabel";
        }
    } else if (!label && props) {
        std::unordered_map<uint32_t, Value> map;
        for (auto& name: props.value()) {
            auto pid = db_->id_generator().GetPid(name);
            if (!pid.has_value()) {
                return "NoVertexFound";
            }
        }
        return "ScanVertexByProperties";
    } else {
        auto lid = db_->id_generator().GetLid(label.value());
        if (!lid.has_value()) {
            return "NoVertexFound";
        }
        std::unordered_map<uint32_t, Value> map;
        bool found_unique_index = false;
        Value unique_val;
        for (auto& name: props.value()) {
            auto pid = db_->id_generator().GetPid(name);
            if (!pid.has_value()) {
                return "NoVertexFound";
            } else {
                if (!found_unique_index) {
                    auto vi = db_->meta_info().GetVertexPropertyIndex(
                        lid.value(), pid.value());
                    if (vi) {
                        found_unique_index = true;
                        continue;
                    }
                }
            }
        }
        if (found_unique_index) {
            return "GetVertexByUniqueIndex";
        } else {
            return "ScanVertexBylabelProperties";
        }
    }
}

std::unique_ptr<VertexIterator> Transaction::NewVertexIterator(
    const std::optional<std::string>& label,
    const std::optional<std::unordered_map<std::string, Value>>& props) {
    if (!label && !props) {
        return std::make_unique<ScanAllVertex>(this);
    } else if (label && !props) {
        auto lid = db_->id_generator().GetLid(label.value());
        if (!lid.has_value()) {
            return std::make_unique<NoVertexFound>(this);
        } else {
            return std::make_unique<ScanVertexBylabel>(this, lid.value());
        }
    } else if (!label && props) {
        std::unordered_map<uint32_t, Value> map;
        for (auto& [name, val] : props.value()) {
            auto pid = db_->id_generator().GetPid(name);
            if (!pid.has_value()) {
                return std::make_unique<NoVertexFound>(this);
            } else {
                map.emplace(pid.value(), val);
            }
        }
        return std::make_unique<ScanVertexByProperties>(this, std::move(map));
    } else {
        auto lid = db_->id_generator().GetLid(label.value());
        if (!lid.has_value()) {
            return std::make_unique<NoVertexFound>(this);
        }
        std::unordered_map<uint32_t, Value> map;
        bool found_unique_index = false;
        uint32_t unique_pid;
        Value unique_val;
        for (auto& [name, val] : props.value()) {
            auto pid = db_->id_generator().GetPid(name);
            if (!pid.has_value()) {
                return std::make_unique<NoVertexFound>(this);
            } else {
                if (!found_unique_index) {
                    auto vi = db_->meta_info().GetVertexPropertyIndex(
                        lid.value(), pid.value());
                    if (vi) {
                        found_unique_index = true;
                        unique_pid = pid.value();
                        unique_val = val;
                        continue;
                    }
                }
                map.emplace(pid.value(), val);
            }
        }
        if (found_unique_index) {
            return std::make_unique<GetVertexByUniqueIndex>(
                this, lid.value(), unique_pid, unique_val, map);
        } else {
            return std::make_unique<ScanVertexBylabelProperties>(
                this, lid.value(), std::move(map));
        }
    }
}

void Transaction::Commit() {
    auto s = txn_->Commit();
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
}

void Transaction::Rollback() {
    auto s = txn_->Rollback();
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
}

std::unique_ptr<VertexScoreIterator> Transaction::QueryVertexByFTIndex(
    const std::string& index_name, const std::string& query, size_t top_n) {
    return std::make_unique<GetVertexByFullTextIndex>(this, index_name, query,
                                                      top_n);
}

std::unique_ptr<graphdb::VertexScoreIterator>
    Transaction::QueryVertexByKnnSearch(const std::string& index_name,
                                    const std::vector<float>& query,
                                    int top_k, int ef_search) {
    return std::make_unique<GetVertexByKnnSearch>(this, index_name, query, top_k, ef_search);
}

std::unique_ptr<ResultIterator> Transaction::Execute(void* ctx,
    const std::string& cypher) {
    return std::make_unique<ResultIterator>(ctx, this, cypher);
}
}