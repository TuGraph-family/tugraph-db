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

#include "graph_entity.h"

#include <rocksdb/utilities/write_batch_with_index.h>

#include "common/exceptions.h"
#include "graph_db.h"
#include "common/logger.h"
#include "transaction/transaction.h"
using namespace boost::endian;
namespace graphdb {
std::unique_ptr<EdgeIterator> Vertex::NewEdgeIterator(
    EdgeDirection direction, const std::unordered_set<std::string> &types,
    const std::unordered_map<std::string, Value> &props) {
    std::unordered_map<uint32_t, Value> prop_map;
    for (auto &[name, val] : props) {
        auto pid = txn_->db()->id_generator().GetPid(name);
        if (pid) {
            prop_map.emplace(pid.value(), val);
        } else {
            return std::make_unique<NoEdgeFound>(txn_);
        }
    }
    std::unordered_set<uint32_t> type_set;
    for (auto &type : types) {
        auto tid = txn_->db()->id_generator().GetTid(type);
        if (tid) {
            type_set.insert(tid.value());
        }
    }
    return std::make_unique<ScanEdgeByVidDirectionTypesProperties>(
        txn_, id_, direction, std::move(type_set), std::move(prop_map));
}

std::unique_ptr<EdgeIterator> Vertex::NewEdgeIterator(
    EdgeDirection direction, const std::unordered_set<std::string> &types,
    const std::unordered_map<std::string, Value> &props,
    const std::unordered_set<std::string> &other_node_labels,
    const std::unordered_map<std::string, Value> &other_node_props) {
    std::unordered_map<uint32_t, Value> prop_map;
    if (!props.empty()) {
        for (auto &[name, val] : props) {
            auto pid = txn_->db()->id_generator().GetPid(name);
            if (pid) {
                prop_map.emplace(pid.value(), val);
            } else {
                return std::make_unique<NoEdgeFound>(txn_);
            }
        }
    }
    std::unordered_set<uint32_t> type_set;
    if (!types.empty()) {
        for (auto &type : types) {
            auto tid = txn_->db()->id_generator().GetTid(type);
            if (tid) {
                type_set.insert(tid.value());
            }
        }
        if (type_set.empty()) {
            return std::make_unique<NoEdgeFound>(txn_);
        }
    }
    std::unordered_set<uint32_t> other_node_label_set;
    if (!other_node_labels.empty()) {
        for (auto &label : other_node_labels) {
            auto lid = txn_->db()->id_generator().GetLid(label);
            if (lid) {
                other_node_label_set.insert(lid.value());
            }
        }
        if (other_node_label_set.empty()) {
            return std::make_unique<NoEdgeFound>(txn_);
        }
    }
    std::unordered_map<uint32_t, Value> other_node_prop_map;
    if (!other_node_props.empty()) {
        for (auto &[name, val] : other_node_props) {
            auto pid = txn_->db()->id_generator().GetPid(name);
            if (pid) {
                other_node_prop_map.emplace(pid.value(), val);
            } else {
                return std::make_unique<NoEdgeFound>(txn_);
            }
        }
    }
    return std::make_unique<ScanEdgeByVidDirectionTypesPropertiesOtherNode>(
        txn_, id_, direction, std::move(type_set), std::move(prop_map),
        std::move(other_node_label_set), std::move(other_node_prop_map));
}

std::unique_ptr<EdgeIterator> Vertex::NewEdgeIterator(
    EdgeDirection direction, const std::unordered_set<std::string>& types,
    const std::unordered_map<std::string, Value>& props,const Vertex& other_node) {
    std::unordered_map<uint32_t, Value> prop_map;
    if (!props.empty()) {
        for (auto &[name, val] : props) {
            auto pid = txn_->db()->id_generator().GetPid(name);
            if (pid) {
                prop_map.emplace(pid.value(), val);
            } else {
                return std::make_unique<NoEdgeFound>(txn_);
            }
        }
    }
    std::unordered_set<uint32_t> type_set;
    if (!types.empty()) {
        for (auto &type : types) {
            auto tid = txn_->db()->id_generator().GetTid(type);
            if (tid) {
                type_set.insert(tid.value());
            }
        }
        if (type_set.empty()) {
            return std::make_unique<NoEdgeFound>(txn_);
        }
    }

    return std::make_unique<ScanEdgeByVidDirectionTypesPropertiesOtherVid>(
        txn_, id_, direction, std::move(type_set), std::move(prop_map),
        other_node);
}

std::unique_ptr<EdgeIterator> Vertex::NewEdgeIterator(
    EdgeDirection direction, const std::string &type,
    const std::unordered_map<std::string, Value> &props,
    const Vertex &other_node) {
    if (direction == EdgeDirection::BOTH) {
        THROW_CODE(InputError, "EdgeDirection can not be BOTH");
    }
    auto tid = txn_->db()->id_generator().GetTid(type);
    if (!tid) {
        return std::make_unique<NoEdgeFound>(txn_);
    }
    std::unordered_map<uint32_t, Value> prop_map;
    if (!props.empty()) {
        for (auto &[name, val] : props) {
            auto pid = txn_->db()->id_generator().GetPid(name);
            if (pid) {
                prop_map.emplace(pid.value(), val);
            } else {
                return std::make_unique<NoEdgeFound>(txn_);
            }
        }
    }
    return std::make_unique<ScanEdgeByVidDirectionTypePropertiesOtherNode>(
        txn_, id_, direction, tid.value(), prop_map, other_node);
}

std::unordered_set<uint32_t> Vertex::GetLabelIds() {
    rocksdb::ReadOptions ro;
    std::string val;
    auto s = txn_->dbtxn()->Get(
        ro, txn_->db()->graph_cf().graph_topology,
        rocksdb::Slice{(const char *)(&id_), sizeof(id_)}, &val);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    std::unordered_set<uint32_t> ret;
    for (size_t i = 0; i < val.size(); i += sizeof(uint32_t)) {
        ret.insert(*(uint32_t *)(val.data() + i));
    }
    return ret;
}

void Vertex::Lock() {
    rocksdb::ReadOptions ro;
    auto s =
        txn_->dbtxn()->GetForUpdate(ro, txn_->db()->graph_cf().graph_topology,
                                    GetIdView(), (std::string *)nullptr);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
}

int Vertex::Delete() {
    int deleted_edge = 0;
    rocksdb::ReadOptions ro;
    // lock vertex
    Lock();
    std::unique_ptr<rocksdb::Iterator> iter;
    rocksdb::Slice prefix((const char *)&id_, sizeof(id_));
    iter.reset(
        txn_->dbtxn()->GetIterator(ro, txn_->db()->graph_cf().graph_topology));
    std::vector<int64_t> eids;
    for (iter->Seek(prefix); iter->Valid() && iter->key().starts_with(prefix);
         iter->Next()) {
        auto key = iter->key().ToString();  // must copy
        auto val = iter->value();
        if (key.size() == sizeof(int64_t)) {
            std::unordered_set<uint32_t> labelIds;
            for (size_t i = 0; i < val.size(); i += sizeof(uint32_t)) {
                labelIds.insert(*(uint32_t *)(val.data() + i));
            }
            std::unordered_set<uint32_t> pids;
            // delete label vid
            for (auto labelId : labelIds) {
                std::string labelVid;
                labelVid.append((const char *)&labelId, sizeof(labelId));
                labelVid.append(key.data(), key.size());
                auto s = txn_->dbtxn()->GetWriteBatch()->SingleDelete(
                    txn_->db()->graph_cf().vertex_label_vid, labelVid);
                if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
            }

            // delete vertex properties
            std::unique_ptr<rocksdb::Iterator> vp_iter;
            rocksdb::Slice vp_prefix = key;
            vp_iter.reset(txn_->dbtxn()->GetIterator(
                ro, txn_->db()->graph_cf().vertex_property));
            for (vp_iter->Seek(vp_prefix);
                 vp_iter->Valid() && vp_iter->key().starts_with(vp_prefix);
                 vp_iter->Next()) {
                auto p_key = vp_iter->key().ToString();  // must copy
                auto pid = *(uint32_t *)(p_key.data() + sizeof(int64_t));
                pids.insert(pid);
                auto p_val = vp_iter->value().ToString();  // must copy
                for (auto lid : labelIds) {
                    auto vi = txn_->db()->meta_info().GetVertexPropertyIndex(
                        lid, pid);
                    if (vi) {
                        vi->DeleteIndex(txn_, p_val);
                    }
                }
                auto s = txn_->dbtxn()->GetWriteBatch()->Delete(
                    txn_->db()->graph_cf().vertex_property, p_key);
                if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
            }
            if (txn_->db()->busy_index().Busy(labelIds, pids)) {
                THROW_CODE(IndexBusy);
            }
            // fulltext index
            for (const auto &[name, ft] :
                 txn_->db()->meta_info().GetVertexFullTextIndex()) {
                if (!ft->MatchLabelIds(labelIds)) {
                    continue;
                }
                if (ft->IsIndexed(txn_, id_)) {
                    meta::FullTextIndexUpdate del;
                    del.set_type(meta::UpdateType::Delete);
                    del.set_id(id_);
                    ft->DeleteIndex(txn_, id_, del);
                }
            }
            // vector index
            for (auto& [index_name, vvi] : txn_->db()->meta_info().GetVertexVectorIndex()) {
                if (!labelIds.count(vvi.lid())) {
                    continue;
                }
                if (!pids.count(vvi.pid())) {
                    continue;
                }
                VectorIndexUpdate del;
                del.type = UpdateType::Delete;
                del.index_name = index_name;
                del.vid = id_;
                txn_->vector_updates().emplace_back(std::move(del));
            }
            auto s = txn_->dbtxn()->GetWriteBatch()->Delete(
                txn_->db()->graph_cf().graph_topology, key);
            if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
        } else {
            assert(key.size() == 29);
            auto p = key.data();
            int64_t vid1 = *(int64_t *)p;
            p += sizeof(int64_t);
            auto dir = static_cast<EdgeDirection>(*(p));
            p += sizeof(char);
            uint32_t etid = *(uint32_t *)p;
            p += sizeof(uint32_t);
            int64_t vid2 = *(int64_t *)p;
            p += sizeof(int64_t);
            int64_t eid = *(int64_t *)p;
            {
                // lock edge
                auto s = txn_->dbtxn()->GetForUpdate(
                    ro, txn_->db()->graph_cf().graph_topology,
                    rocksdb::Slice((const char *)&eid, sizeof(eid)),
                    (std::string *)nullptr);
                if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
            }
            eids.push_back(eid);
            // delete other edge key
            std::string other_edge_key;
            other_edge_key.append((const char *)&vid2, sizeof(vid2));
            other_edge_key.append(
                1, static_cast<char>(dir == EdgeDirection::OUTGOING
                                         ? EdgeDirection::INCOMING
                                         : EdgeDirection::OUTGOING));
            other_edge_key.append((const char *)&etid, sizeof(etid));
            other_edge_key.append((const char *)&vid1, sizeof(vid1));
            other_edge_key.append((const char *)&eid, sizeof(eid));
            auto s = txn_->dbtxn()->GetWriteBatch()->SingleDelete(
                txn_->db()->graph_cf().graph_topology, other_edge_key);
            if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
            deleted_edge++;
            // delete type eid
            std::string typeEid;
            typeEid.append((const char *)&etid, sizeof(etid));
            typeEid.append((const char *)&eid, sizeof(eid));
            s = txn_->dbtxn()->GetWriteBatch()->SingleDelete(
                txn_->db()->graph_cf().edge_type_eid, typeEid);
            if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
            // delete edge properties
            std::unique_ptr<rocksdb::Iterator> ep_iter;
            rocksdb::Slice ep_prefix((const char *)&eid, sizeof(eid));
            ep_iter.reset(txn_->dbtxn()->GetIterator(
                ro, txn_->db()->graph_cf().edge_property));
            for (ep_iter->Seek(ep_prefix);
                 ep_iter->Valid() && ep_iter->key().starts_with(ep_prefix);
                 ep_iter->Next()) {
                auto prop_key = ep_iter->key().ToString();  // must copy
                s = txn_->dbtxn()->GetWriteBatch()->Delete(
                    txn_->db()->graph_cf().edge_property, prop_key);
                if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
            }
            s = txn_->dbtxn()->GetWriteBatch()->SingleDelete(
                txn_->db()->graph_cf().graph_topology, key);
            if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
        }
    }
    return deleted_edge;
}

std::unordered_set<std::string> Vertex::GetLabels() {
    std::unordered_set<std::string> ret;
    for (auto id : GetLabelIds()) {
        auto optional = txn_->db()->id_generator().GetVertexLabelName(id);
        if (optional.has_value()) {
            ret.emplace(std::move(optional.value()));
        }
    }
    return ret;
}

void Vertex::AddLabels(const std::unordered_set<std::string> &labels) {
    if (labels.empty()) {
        return;
    }
    std::unordered_set<uint32_t> add_lids, new_lids;
    for (auto& label : labels) {
        auto lid = txn_->db()->id_generator().GetOrCreateLid(label);
        add_lids.insert(lid);
    }
    if (txn_->db()->busy_index().LabelBusy(add_lids)) {
        THROW_CODE(IndexBusy);
    }
    Lock();
    auto labelIds = GetLabelIds();
    for (auto& lid : add_lids) {
        if (labelIds.count(lid)) {
            continue;
        }
        new_lids.insert(lid);
    }
    // full text index
    for (const auto &[ft_name, ft] :
         txn_->db()->meta_info().GetVertexFullTextIndex()) {
        if (!ft->MatchLabelIds(new_lids)) {
            continue;
        }
        if (ft->IsIndexed(txn_, id_)) {
            continue;
        }
        meta::FullTextIndexUpdate add;
        add.set_id(id_);
        add.set_type(meta::UpdateType::Add);
        for (auto pid : ft->PropertyIds()) {
            auto prop_val = GetProperty(pid);
            if (!(prop_val.IsString() && !prop_val.AsString().empty())) {
                continue;
            }
            add.add_fields(txn_->db()->id_generator().GetPropertyName(pid).value());
            add.add_values(prop_val.AsString());
        }
        if (!add.fields().empty()) {
            ft->AddIndex(txn_, id_, add);
        }
    }
    // vector index
    for (auto& [index_name, vvi] : txn_->db()->meta_info().GetVertexVectorIndex()) {
        if (!new_lids.count(vvi.lid())) {
            continue;
        }
        auto p_val = GetProperty(vvi.pid());
        if (!p_val.IsArray()) {
            continue;
        }
        auto& array = p_val.AsArray();
        if (array.empty() || (!array[0].IsDouble() && !array[0].IsFloat())) {
            continue;
        }
        if (array.size() != vvi.meta().dimensions()) {
            continue;
        }
        VectorIndexUpdate add;
        add.type = UpdateType::Add;
        add.index_name = index_name;
        add.vid = id_;
        add.vectors = array;
        txn_->vector_updates().emplace_back(std::move(add));
    }

    labelIds.insert(new_lids.begin(), new_lids.end());
    std::string buffer;
    for (auto l : labelIds) {
        buffer.append((const char *)&l, sizeof(l));
    }
    auto s = txn_->dbtxn()->GetWriteBatch()->Put(
        txn_->db()->graph_cf().graph_topology, GetIdView(), buffer);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    for (auto& id : new_lids) {
        std::string key((const char *)&id, sizeof(id));
        key.append(GetIdView());
        s = txn_->dbtxn()->GetWriteBatch()->Put(
            txn_->db()->graph_cf().vertex_label_vid, key, {});
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }
}

void Vertex::DeleteLabels(const std::unordered_set<std::string> &labels) {
    std::unordered_set<uint32_t> lids, remove_lids;
    for (auto& label : labels) {
        auto optional = txn_->db()->id_generator().GetLid(label);
        if (optional.has_value()) {
            lids.insert(optional.value());
        }
    }
    if (lids.empty()) {
        return;
    }
    Lock();
    auto labelIds = GetLabelIds();
    for (auto id : lids) {
        if (labelIds.count(id)) {
            remove_lids.insert(id);
        }
    }
    if (remove_lids.empty()) {
        return;
    }
    if (txn_->db()->busy_index().LabelBusy(remove_lids)) {
        THROW_CODE(IndexBusy);
    }
    // full text index
    for (const auto &[ft_name, ft] :
         txn_->db()->meta_info().GetVertexFullTextIndex()) {
        if (!ft->MatchLabelIds(remove_lids)) {
            continue;
        }
        if (ft->IsIndexed(txn_, id_)) {
            meta::FullTextIndexUpdate del;
            del.set_type(meta::UpdateType::Delete);
            del.set_id(id_);
            ft->DeleteIndex(txn_, id_, del);
        }
    }
    // vector index
    for (auto& [index_name, vvi] : txn_->db()->meta_info().GetVertexVectorIndex()) {
        if (remove_lids.count(vvi.lid())) {
            continue;
        }
        auto p_val = GetProperty(vvi.pid());
        if (p_val.IsNull()) {
            continue;
        }
        VectorIndexUpdate del;
        del.type = UpdateType::Delete;
        del.index_name = index_name;
        del.vid = id_;
        txn_->vector_updates().emplace_back(std::move(del));
    }
    for (auto id : remove_lids) {
        labelIds.erase(id);
    }
    std::string buffer;
    for (auto l : labelIds) {
        buffer.append((const char *)&l, sizeof(l));
    }
    auto s = txn_->dbtxn()->GetWriteBatch()->Put(
        txn_->db()->graph_cf().graph_topology, GetIdView(), buffer);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    for (auto id : remove_lids) {
        std::string key((const char *)&id, sizeof(id));
        key.append(GetIdView());
        s = txn_->dbtxn()->GetWriteBatch()->SingleDelete(
            txn_->db()->graph_cf().vertex_label_vid, key);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }
}

Value Vertex::GetProperty(const std::string &name) {
    auto optional = txn_->db()->id_generator().GetPid(name);
    if (!optional.has_value()) {
        return {};
    }
    return GetProperty(optional.value());
}

Value Vertex::GetProperty(uint32_t pid) {
    rocksdb::ReadOptions ro;
    rocksdb::PinnableSlice pval;
    Value ret;
    std::string pkey((const char *)&id_, sizeof(id_));
    pkey.append((const char *)(&pid), sizeof(pid));
    auto s = txn_->dbtxn()->Get(ro, txn_->db()->graph_cf().vertex_property,
                                pkey, &pval);
    if (s.ok()) {
        ret.Deserialize(pval.data(), pval.size());
    } else if (!s.IsNotFound()) {
        THROW_CODE(StorageEngineError, s.ToString());
    }
    return ret;
}

std::unordered_map<std::string, Value> Vertex::GetAllProperty() {
    std::string prefix((const char *)&id_, sizeof(id_));
    rocksdb::ReadOptions ro;
    std::unordered_map<std::string, Value> ret;
    std::unique_ptr<rocksdb::Iterator> p_iter;
    p_iter.reset(
        txn_->dbtxn()->GetIterator(ro, txn_->db()->graph_cf().vertex_property));
    for (p_iter->Seek(prefix);
         p_iter->Valid() && p_iter->key().starts_with(prefix); p_iter->Next()) {
        auto key = p_iter->key();
        auto value = p_iter->value();
        key.remove_prefix(sizeof(int64_t));
        uint32_t pid = *(uint32_t *)key.data();
        auto optional = txn_->db()->id_generator().GetPropertyName(pid);
        if (optional.has_value()) {
            Value v;
            v.Deserialize(value.data(), value.size());
            ret.emplace(std::move(optional.value()), std::move(v));
        }
    }
    return ret;
}

int Vertex::GetDegree(graphdb::EdgeDirection direction) {
    int count = 0;
    for (auto eiter = NewEdgeIterator(direction, {}, {});
         eiter->Valid(); eiter->Next()) {
        count++;
    }
    return count;
}

void Vertex::SetProperties(const std::unordered_map<std::string, Value>& values) {
    if (values.empty()) {
        return;
    }
    std::unordered_map<uint32_t, const Value*> original;
    std::unordered_map<uint32_t, std::string> serialized;
    std::unordered_set<uint32_t> pids;
    for (const auto& [name, val] : values) {
        auto pid = txn_->db()->id_generator().GetOrCreatePid(name);
        original[pid] = &val;
        serialized[pid] = val.Serialize();
        pids.insert(pid);
    }
    Lock();
    auto lids = GetLabelIds();
    // property index
    for (auto &[name, index] :
         txn_->db()->meta_info().GetVertexPropertyIndex()) {
        if (!lids.count(index.lid())) {
            continue;
        }
        auto iter = serialized.find(index.pid());
        if (iter == serialized.end()) {
            continue;
        }
        auto pid = iter->first;
        std::string pkey((const char *)&id_, sizeof(id_));
        pkey.append((const char *)(&pid), sizeof(pid));
        rocksdb::ReadOptions ro;
        std::string val;
        const std::string* old = nullptr;
        auto s = txn_->dbtxn()->Get(ro, txn_->db()->graph_cf().vertex_property, pkey, &val);
        if (s.ok()) {
            old = &val;
        } else if (!s.IsNotFound()) {
            THROW_CODE(StorageEngineError, s.ToString());
        }
        index.UpdateIndex(txn_, id_, iter->second, old);
    }
    // full text index
    for (const auto &[name, index] :
         txn_->db()->meta_info().GetVertexFullTextIndex()) {
        if (!index->MatchLabelIds(lids) || !index->MatchPropertyIds(pids)) {
            continue;
        }
        if (index->IsIndexed(txn_, id_)) {
            meta::FullTextIndexUpdate del;
            del.set_type(meta::UpdateType::Delete);
            del.set_id(id_);
            index->DeleteIndex(txn_, id_, del);
        }
        meta::FullTextIndexUpdate add;
        add.set_id(id_);
        add.set_type(meta::UpdateType::Add);
        for (auto prop_id : index->PropertyIds()) {
            std::optional<std::string> text;
            auto iter = original.find(prop_id);
            if (iter != original.end()) {
                if (iter->second->IsString() && !iter->second->AsString().empty()) {
                    text = iter->second->AsString();
                }
            } else {
                auto prop = GetProperty(prop_id);
                if (prop.IsString() && !prop.AsString().empty()) {
                    text = prop.AsString();
                }
            }
            if (text) {
                add.add_fields(txn_->db()->id_generator().GetPropertyName(prop_id).value());
                add.add_values(std::move(text.value()));
            }
        }
        if (!add.fields().empty()) {
            index->AddIndex(txn_, id_, add);
        }
    }
    // vector index
    for (auto& [name, index] : txn_->db()->meta_info().GetVertexVectorIndex()) {
        if (!pids.count(index.pid()) || !lids.count(index.lid())) {
            continue;
        }
        VectorIndexUpdate del;
        del.type = UpdateType::Delete;
        del.index_name = name;
        del.vid = id_;
        txn_->vector_updates().emplace_back(std::move(del));
        auto value = original.at(index.pid());
        if (!value->IsArray()) {
            continue;
        }
        auto& array = value->AsArray();
        if (array.empty() || (!array[0].IsDouble() && !array[0].IsFloat())) {
            continue;
        }
        if (array.size() != index.meta().dimensions()) {
            continue;
        }
        VectorIndexUpdate add;
        add.type = UpdateType::Add;
        add.index_name = name;
        add.vid = id_;
        add.vectors = array;
        txn_->vector_updates().emplace_back(std::move(add));
    }
    for (auto& [pid, pval] : serialized) {
        std::string pkey((const char *)&id_, sizeof(id_));
        pkey.append((const char *)(&pid), sizeof(pid));
        auto s = txn_->dbtxn()->GetWriteBatch()->Put(
            txn_->db()->graph_cf().vertex_property, pkey, pval);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }
}

void Vertex::RemoveAllProperty() {
    Lock();
    auto lids = GetLabelIds();
    std::string prefix((const char *)&id_, sizeof(id_));
    rocksdb::ReadOptions ro;
    std::unordered_set<uint32_t> pids;
    std::unordered_map<uint32_t , std::string> props;
    std::vector<std::string> prop_keys;
    std::unique_ptr<rocksdb::Iterator> p_iter;
    p_iter.reset(
        txn_->dbtxn()->GetIterator(ro, txn_->db()->graph_cf().vertex_property));
    for (p_iter->Seek(prefix);
         p_iter->Valid() && p_iter->key().starts_with(prefix); p_iter->Next()) {
        auto key = p_iter->key();
        prop_keys.push_back(key.ToString());
        auto value = p_iter->value();
        key.remove_prefix(sizeof(int64_t));
        uint32_t pid = *(uint32_t *)key.data();
        props.emplace(pid, value.ToString());
        pids.insert(pid);
    }
    p_iter.reset();
    // property index
    for (auto lid : lids) {
        for (auto& [pid, prop] : props) {
            auto vi = txn_->db()->meta_info().GetVertexPropertyIndex(lid, pid);
            if (vi) {
                vi->DeleteIndex(txn_, prop);
            }
        }
    }
    // full text index
    for (const auto &[ft_name, ft] :
         txn_->db()->meta_info().GetVertexFullTextIndex()) {
        if (!ft->MatchLabelIds(lids)) {
            continue;
        }
        if (ft->IsIndexed(txn_, id_)) {
            meta::FullTextIndexUpdate del;
            del.set_id(id_);
            del.set_type(meta::UpdateType::Delete);
            ft->DeleteIndex(txn_, id_, del);
        }
    }
    // vector index
    for (auto& [index_name, vvi] : txn_->db()->meta_info().GetVertexVectorIndex()) {
        if (!pids.count(vvi.pid()) || !lids.count(vvi.lid())) {
            continue;
        }
        VectorIndexUpdate del;
        del.type = UpdateType::Delete;
        del.index_name = index_name;
        del.vid = id_;
        txn_->vector_updates().emplace_back(std::move(del));
    }
    for (auto& key : prop_keys) {
        auto s = txn_->dbtxn()->GetWriteBatch()->Delete(
            txn_->db()->graph_cf().vertex_property, key);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }
}

void Vertex::RemoveProperty(const std::string &name) {
    auto optional = txn_->db()->id_generator().GetPid(name);
    if (!optional.has_value()) {
        return;
    }
    auto pid = optional.value();
    std::string pkey((const char *)&id_, sizeof(id_));
    pkey.append((const char *)(&pid), sizeof(pid));
    Lock();
    auto lids = GetLabelIds();
    if (txn_->db()->busy_index().Busy(lids, pid)) {
        THROW_CODE(IndexBusy);
    }
    // property index
    for (auto lid : lids) {
        auto vi = txn_->db()->meta_info().GetVertexPropertyIndex(lid, pid);
        if (vi) {
            std::string old_val;
            rocksdb::ReadOptions ro;
            auto s =
                txn_->dbtxn()->Get(ro, txn_->db()->graph_cf().vertex_property,
                                   pkey, &old_val);
            if (s.ok()) {
                vi->DeleteIndex(txn_, old_val);
            } else if (s.IsNotFound()) {
                return;
            } else {
                THROW_CODE(StorageEngineError, s.ToString());
            }
        }
    }
    // full text index
    for (const auto &[ft_name, ft] :
         txn_->db()->meta_info().GetVertexFullTextIndex()) {
        if (!ft->MatchLabelIds(lids) || !ft->MatchPropertyIds({pid})) {
            continue;
        }
        if (ft->IsIndexed(txn_, id_)) {
            meta::FullTextIndexUpdate del;
            del.set_id(id_);
            del.set_type(meta::UpdateType::Delete);
            ft->DeleteIndex(txn_, id_, del);
        }

        meta::FullTextIndexUpdate add;
        add.set_type(meta::UpdateType::Add);
        add.set_id(id_);
        for (auto prop_id : ft->PropertyIds()) {
            if (prop_id == pid) {
                continue;
            }
            auto prop = GetProperty(prop_id);
            if (prop.IsString() && !prop.AsString().empty()) {
                add.add_fields(txn_->db()
                                   ->id_generator()
                                   .GetPropertyName(prop_id)
                                   .value());
                add.add_values(prop.AsString());
            }
        }
        if (!add.fields().empty()) {
            ft->AddIndex(txn_, id_, add);
        }
    }
    // vector index
    for (auto& [index_name, vvi] : txn_->db()->meta_info().GetVertexVectorIndex()) {
        if (pid != vvi.pid() || !lids.count(vvi.lid())) {
            continue;
        }
        VectorIndexUpdate del;
        del.type = UpdateType::Delete;
        del.index_name = index_name;
        del.vid = id_;
        txn_->vector_updates().emplace_back(std::move(del));
    }
    auto s = txn_->dbtxn()->GetWriteBatch()->Delete(
        txn_->db()->graph_cf().vertex_property, pkey);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
}

void Edge::Delete() {
    Lock();
    // delete out edge key
    std::string key;
    key.append((const char *)&startId_, sizeof(startId_));
    key.append(1, static_cast<char>(EdgeDirection::OUTGOING));
    key.append((const char *)&typeId_, sizeof(typeId_));
    key.append((const char *)&endId_, sizeof(endId_));
    key.append((const char *)&id_, sizeof(id_));
    auto s = txn_->dbtxn()->GetWriteBatch()->SingleDelete(
        txn_->db()->graph_cf().graph_topology, key);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    key.clear();
    // delete in edge key
    key.append((const char *)&endId_, sizeof(endId_));
    key.append(1, static_cast<char>(EdgeDirection::INCOMING));
    key.append((const char *)&typeId_, sizeof(typeId_));
    key.append((const char *)&startId_, sizeof(startId_));
    key.append((const char *)&id_, sizeof(id_));
    s = txn_->dbtxn()->GetWriteBatch()->SingleDelete(
        txn_->db()->graph_cf().graph_topology, key);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    // delete type eid
    key.clear();
    key.append((const char *)&typeId_, sizeof(typeId_));
    key.append((const char *)&id_, sizeof(id_));
    s = txn_->dbtxn()->GetWriteBatch()->SingleDelete(
        txn_->db()->graph_cf().edge_type_eid, key);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    // delete edge properties
    std::unique_ptr<rocksdb::Iterator> ep_iter;
    rocksdb::ReadOptions ro;
    rocksdb::Slice ep_prefix((const char *)&id_, sizeof(id_));
    ep_iter.reset(
        txn_->dbtxn()->GetIterator(ro, txn_->db()->graph_cf().edge_property));
    for (ep_iter->Seek(ep_prefix);
         ep_iter->Valid() && ep_iter->key().starts_with(ep_prefix);
         ep_iter->Next()) {
        auto prop_key = ep_iter->key().ToString();  // must copy
        s = txn_->dbtxn()->GetWriteBatch()->Delete(
            txn_->db()->graph_cf().edge_property, prop_key);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }
}

std::string Edge::GetType() {
    auto id = GetTypeId();
    auto ret = txn_->db()->id_generator().GetEdgeTypeName(id);
    assert(ret.has_value());
    return ret.value();
}

Value Edge::GetProperty(uint32_t pid) {
    rocksdb::ReadOptions ro;
    rocksdb::PinnableSlice pinnable_val;
    Value ret;
    std::string pkey((const char *)&id_, sizeof(id_));
    pkey.append((const char *)(&pid), sizeof(pid));
    auto s = txn_->dbtxn()->Get(ro, txn_->db()->graph_cf().edge_property,
                                pkey, &pinnable_val);
    if (s.ok()) {
        ret.Deserialize(pinnable_val.data(), pinnable_val.size());
    }
    return ret;
}

Vertex Edge::GetOtherEnd(int64_t vid) const {
    if (startId_ == vid) {
        return GetEnd();
    } else {
        if (vid != endId_) {
            THROW_CODE(UnknownError,
                       "GetOtherEnd error, startId:{}, endId:{}, vid:{}",
                       big_to_native(startId_),
                       big_to_native(endId_),
                       big_to_native(vid));
        }
        return GetStart();
    }
}

Value Edge::GetProperty(const std::string &name) {
    auto optional = txn_->db()->id_generator().GetPid(name);
    if (!optional.has_value()) {
        return {};
    }
    return GetProperty(optional.value());
}

std::unordered_map<std::string, Value> Edge::GetAllProperty() {
    std::string prefix((const char *)&id_, sizeof(id_));
    rocksdb::ReadOptions ro;
    std::unordered_map<std::string, Value> ret;
    std::unique_ptr<rocksdb::Iterator> p_iter;
    p_iter.reset(
        txn_->dbtxn()->GetIterator(ro, txn_->db()->graph_cf().edge_property));
    for (p_iter->Seek(prefix);
         p_iter->Valid() && p_iter->key().starts_with(prefix); p_iter->Next()) {
        auto key = p_iter->key();
        auto value = p_iter->value();
        key.remove_prefix(sizeof(int64_t));
        uint32_t pid = *(uint32_t *)key.data();
        auto optional = txn_->db()->id_generator().GetPropertyName(pid);
        if (optional.has_value()) {
            Value v;
            v.Deserialize(value.data(), value.size());
            ret.emplace(std::move(optional.value()), std::move(v));
        }
    }
    return ret;
}

void Edge::SetProperties(const std::unordered_map<std::string, Value> &properties) {
    if (properties.empty()) {
        return;
    }
    Lock();
    for (auto& [name, value] : properties) {
        auto pid = txn_->db()->id_generator().GetOrCreatePid(name);
        std::string pkey((const char *)&id_, sizeof(id_));
        pkey.append((const char *)(&pid), sizeof(pid));
        auto s = txn_->dbtxn()->GetWriteBatch()->Put(
            txn_->db()->graph_cf().edge_property, pkey, value.Serialize());
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }
}

void Edge::RemoveProperty(const std::string &name) {
    auto optional = txn_->db()->id_generator().GetPid(name);
    if (!optional.has_value()) {
        return;
    }
    Lock();
    auto pid = optional.value();
    std::string pkey((const char *)&id_, sizeof(id_));
    pkey.append((const char *)(&pid), sizeof(pid));
    auto s = txn_->dbtxn()->GetWriteBatch()->Delete(
        txn_->db()->graph_cf().edge_property, pkey);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
}

void Edge::RemoveAllProperty() {
    Lock();
    std::vector<std::string> prop_keys;
    std::string prefix((const char *)&id_, sizeof(id_));
    rocksdb::ReadOptions ro;
    std::unique_ptr<rocksdb::Iterator> p_iter;
    p_iter.reset(
        txn_->dbtxn()->GetIterator(ro, txn_->db()->graph_cf().edge_property));
    for (p_iter->Seek(prefix);
         p_iter->Valid() && p_iter->key().starts_with(prefix); p_iter->Next()) {
        auto key = p_iter->key();
        prop_keys.push_back(key.ToString());
    }
    p_iter.reset();
    for (auto& key : prop_keys) {
        auto s = txn_->dbtxn()->GetWriteBatch()->Delete(
            txn_->db()->graph_cf().edge_property, key);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }
}

void Edge::Lock() {
    rocksdb::ReadOptions ro;
    auto s = txn_->dbtxn()->GetForUpdate(
        ro, txn_->db()->graph_cf().graph_topology,
        rocksdb::Slice((const char *)&id_, sizeof(id_)),
        (std::string *)nullptr);
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
}
}