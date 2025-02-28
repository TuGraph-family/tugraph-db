/**
 * Copyright 2022 AntGroup CO., Ltd.
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
#include <memory>
#include <boost/algorithm/string.hpp>
#include "db/galaxy.h"
#include "core/index_manager.h"
#include "core/lightning_graph.h"
#include "import/import_config_parser.h"
#include "fma-common/hardware_info.h"

namespace lgraph {
thread_local bool LightningGraph::in_transaction_ = false;

LightningGraph::LightningGraph(const DBConfig& conf) : config_(conf) { Open(); }

LightningGraph::~LightningGraph() { Close(); }

void LightningGraph::Close() {
    _HoldWriteLock(meta_lock_);
    fulltext_index_.reset();
    index_manager_.reset();
    graph_.reset();
    store_.reset();
}

Transaction LightningGraph::CreateWriteTxn(bool optimistic) {
    return Transaction(false, optimistic, this);
}

void LightningGraph::DropAllData() {
    _HoldWriteLock(meta_lock_);
    {
        Transaction txn = CreateWriteTxn();
        auto s = schema_.GetScopedRef();
        s->v_schema_manager.Clear(txn.GetTxn());
        s->e_schema_manager.Clear(txn.GetTxn());
        graph_->Drop(txn.GetTxn());
        store_->DropAll(txn.GetTxn());
        txn.Commit();
    }
    if (fulltext_index_) {
        fulltext_index_->Clear();
        fulltext_index_.reset();
    }
    /* state reset */
    index_manager_.reset();
    graph_.reset();
    store_.reset();
    Open();
}

void LightningGraph::DropAllVertex() {
    try {
        _HoldWriteLock(meta_lock_);
        Transaction txn = CreateWriteTxn(false);
        ScopedRef<SchemaInfo> curr_schema = schema_.GetScopedRef();
        // clear indexes
        auto [indexes, composite_indexes, vector_indexes]
            = index_manager_->ListAllIndexes(txn.GetTxn());
        for (auto& idx : indexes) {
            auto v_schema = curr_schema->v_schema_manager.GetSchema(idx.label);
            auto e_schema = curr_schema->e_schema_manager.GetSchema(idx.label);
            FMA_DBG_ASSERT(v_schema || e_schema);
            if (v_schema) {
                auto ext = v_schema->GetFieldExtractor(idx.field);
                FMA_DBG_ASSERT(ext);
                ext->GetVertexIndex()->Clear(txn.GetTxn());
            }
            if (e_schema) {
                auto ext = e_schema->GetFieldExtractor(idx.field);
                FMA_DBG_ASSERT(ext);
                ext->GetEdgeIndex()->Clear(txn.GetTxn());
            }
        }
        for (auto& idx : composite_indexes) {
            auto v_schema = curr_schema->v_schema_manager.GetSchema(idx.label);
            FMA_DBG_ASSERT(v_schema);
            if (v_schema) {
                v_schema->GetCompositeIndex(idx.fields)->Clear(txn.GetTxn());
            }
        }
        for (auto& idx : vector_indexes) {
            auto v_schema = curr_schema->v_schema_manager.GetSchema(idx.label);
            FMA_DBG_ASSERT(v_schema);
            if (v_schema) {
                auto ext = v_schema->GetFieldExtractor(idx.field);
                FMA_DBG_ASSERT(ext);
                ext->GetVectorIndex()->Clear();
            }
        }
        // clear detached property data
        for (auto& name : curr_schema->v_schema_manager.GetAllLabels()) {
            auto s = curr_schema->v_schema_manager.GetSchema(name);
            if (s->DetachProperty()) {
                s->GetPropertyTable().Drop(txn.GetTxn());
            }
        }
        for (auto& name : curr_schema->e_schema_manager.GetAllLabels()) {
            auto s = curr_schema->e_schema_manager.GetSchema(name);
            if (s->DetachProperty()) {
                s->GetPropertyTable().Drop(txn.GetTxn());
            }
        }
        // clear graph data
        graph_->Drop(txn.GetTxn());
        // clear count data
        graph_->DeleteAllCount(txn.GetTxn());
        txn.Commit();
    } catch (std::exception& e) {
        LOG_WARN() << "Failed to drop all vertex : " << e.what();
    }
}

void LightningGraph::GetDBStat(size_t& msize, size_t& next_vid) {
    Transaction txn = CreateReadTxn();
    size_t height = 0;
    store_->DumpStat(txn.GetTxn(), msize, height);
    next_vid = graph_->GetNextVid(txn.GetTxn());
}

size_t LightningGraph::GetNumVertices() {
    Transaction txn = CreateReadTxn();
    return graph_->GetLooseNumVertex(txn.GetTxn());
}

/**
 * Adds a label
 *
 * \param   label       The label.
 * \param   n_fields    Number of fields for this label.
 * \param   fds         The FieldDefs.
 * \param   is_vertex   True if this is vertex label, otherwise it is edge label.
 * \param   options     Cast to VertexOptions when is_vertex is true, else cast to EdgeOptions.
 *
 * \return  True if it succeeds, false if the label already exists. Throws exception on error.
 */
bool LightningGraph::AddLabel(const std::string& label, size_t n_fields, const FieldSpec* fds,
                              bool is_vertex, const LabelOptions& options) {
    // check that label and field names are legal
    lgraph::CheckValidLabelName(label);
    lgraph::CheckValidFieldNum(n_fields);

    std::set<std::string> unique_fds;
    for (size_t i = 0; i < n_fields; i++) {
        using namespace import_v2;
        if (fds[i].name == KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP) ||
            fds[i].name == KeyWordFunc::GetStrFromKeyWord(KeyWord::SRC_ID) ||
            fds[i].name == KeyWordFunc::GetStrFromKeyWord(KeyWord::DST_ID)) {
            THROW_CODE(InputError,
                R"(Label[{}]: Property name cannot be "SKIP" or "SRC_ID" or "DST_ID")", label);
        }
        auto ret = unique_fds.insert(fds[i].name);
        if (!ret.second)
            THROW_CODE(InputError,
                "Label[{}]: Duplicate property definition: [{}]", label, fds[i].name);
    }

    // check constraints
    if (is_vertex) {
        const auto& primary_field = dynamic_cast<const VertexOptions&>(options).primary_field;
        if (n_fields == 0)
            THROW_CODE(InputError, "Vertex[{}]: Schema must have properties", label);
        if (primary_field.empty())
            THROW_CODE(InputError, "Vertex[{}]: Schema must specify the primary property", label);
        bool found = false;
        for (size_t i = 0; i < n_fields; i++) {
            if (fds[i].name != primary_field) continue;
            found = true;
            if (fds[i].optional)
                THROW_CODE(InputError, "Vertex[{}]: Primary property can not be optional", label);
        }
        if (!found) THROW_CODE(InputError, "Vertex[{}]: No primary property found", label);
    } else {
        const auto& edge_constraints = dynamic_cast<const EdgeOptions&>(options).edge_constraints;
        auto temp = edge_constraints;
        std::sort(temp.begin(), temp.end());
        auto iter = std::unique(temp.begin(), temp.end());
        if (iter != temp.end()) {
            THROW_CODE(InputError, "Duplicate constraints: [{}, {}]", iter->first, iter->second);
        }
        ScopedRef<SchemaInfo> schema = schema_.GetScopedRef();
        auto schema_info = schema.Get();
        for (auto& constraint : edge_constraints) {
            for (auto& vertex_label : {constraint.first, constraint.second}) {
                if (!schema_info->v_schema_manager.GetSchema(vertex_label)) {
                    THROW_CODE(InputError, "No such vertex label: {}", vertex_label);
                }
            }
        }
    }
    for (size_t i = 0; i < n_fields; i++) {
        auto& fn = fds[i].name;
        lgraph::CheckValidFieldName(fn);
    }
    // Need to hold a write lock here since if two threads tries to add label concurrently, data
    // race to schema_ may happen.
    _HoldWriteLock(meta_lock_);
    Transaction txn = CreateWriteTxn(false);
    ScopedRef<SchemaInfo> curr_schema = schema_.GetScopedRef();
    std::unique_ptr<SchemaInfo> new_schema(new SchemaInfo(*curr_schema.Get()));
    SchemaManager* sm = is_vertex ? &new_schema->v_schema_manager : &new_schema->e_schema_manager;
    lgraph::CheckValidLabelNum(new_schema->v_schema_manager.GetAllLabels().size() +
                               new_schema->e_schema_manager.GetAllLabels().size() + 1);
    bool r = sm->AddLabel(txn.GetTxn(), is_vertex, label, n_fields, fds, options);
    if (r && is_vertex) {
        // add vertex primary index
        Schema* schema = sm->GetSchema(label);
        FMA_DBG_ASSERT(schema);
        const auto& primary_field = dynamic_cast<const VertexOptions&>(options).primary_field;
        const _detail::FieldExtractorBase* extractor = schema->GetFieldExtractor(primary_field);
        FMA_DBG_ASSERT(extractor);
        std::unique_ptr<VertexIndex> index;
        index_manager_->AddVertexIndex(txn.GetTxn(), label, primary_field, extractor->Type(),
                                       IndexType::GlobalUniqueIndex, index);
        index->SetReady();
        schema->MarkVertexIndexed(extractor->GetFieldId(), index.release());
    }
    if (r) {
        Schema* schema = sm->GetSchema(label);
        FMA_DBG_ASSERT(schema);
        if (schema->DetachProperty()) {
            std::unique_ptr<KvTable> table;
            if (is_vertex) {
                std::string prefix = _detail::VERTEX_PROPERTY_TABLE_PREFIX;
                table = store_->OpenTable(txn.GetTxn(), prefix + label, true,
                                          ComparatorDesc::DefaultComparator());
            } else {
                std::string prefix = _detail::EDGE_PROPERTY_TABLE_PREFIX;
                table = store_->OpenTable(txn.GetTxn(), prefix + label, true,
                                          ComparatorDesc::DefaultComparator());
            }
            schema->SetPropertyTable(std::move(table));
        }

        // refill `EdgeConstraintsLids` with right vertex label id.
        new_schema->e_schema_manager.RefreshEdgeConstraintsLids(
            new_schema->v_schema_manager);
        txn.Commit();
        schema_.Assign(new_schema.release());
    }
    return r;
}

/**
 * Adds a label
 *
 * \param   label       The label name.
 * \param   fds         The FieldDefs.
 * \param   is_vertex   True if this is vertex label, otherwise it is edge label.
 * \param   options     Cast to VertexOptions when is_vertex is true, else cast to EdgeOptions.
 *
 * \return  True if it succeeds, false if the label already exists. Throws exception on error.
 */
bool LightningGraph::AddLabel(const std::string& label, const std::vector<FieldSpec>& fds,
                              bool is_vertex, const LabelOptions& options) {
    return AddLabel(label, fds.size(), fds.data(), is_vertex, options);
}

bool LightningGraph::DelLabel(const std::string& label, bool is_vertex, size_t* n_modified) {
    LOG_INFO() << "Deleting " << (is_vertex ? "vertex" : "edge") << " label ["
                             << label << "]";
    _HoldWriteLock(meta_lock_);
    size_t commit_size = 4096;
    // check that label and field names are legal
    lgraph::CheckValidLabelName(label);
    Transaction txn = CreateWriteTxn(false);
    ScopedRef<SchemaInfo> curr_schema_info = schema_.GetScopedRef();
    SchemaManager* curr_sm =
        is_vertex ? &curr_schema_info->v_schema_manager : &curr_schema_info->e_schema_manager;
    Schema* schema = curr_sm->GetSchema(label);
    if (!schema) return false;
    std::unique_ptr<SchemaInfo> new_schema(new SchemaInfo(*curr_schema_info.Get()));
    LabelId lid = schema->GetLabelId();
    size_t modified = 0;
    if (schema->DetachProperty()) {
        auto table_name = schema->GetPropertyTable().Name();
        LOG_INFO() << FMA_FMT("begin to scan detached table: {}", table_name);
        auto kv_iter = schema->GetPropertyTable().GetIterator(txn.GetTxn());
        for (kv_iter->GotoFirstKey(); kv_iter->IsValid(); kv_iter->Next()) {
            if (is_vertex) {
                auto vid = graph::KeyPacker::GetVidFromPropertyTableKey(kv_iter->GetKey());
                auto on_edge_deleted = [&curr_schema_info, &txn, vid]
                    (bool is_out_edge, const graph::EdgeValue& edge_value){
                    for (size_t i = 0; i < edge_value.GetEdgeCount(); i++) {
                        const auto& data = edge_value.GetNthEdgeData(i);
                        auto edge_schema = curr_schema_info->e_schema_manager.GetSchema(data.lid);
                        FMA_ASSERT(edge_schema);
                        if (is_out_edge) {
                            Value property(data.prop, data.psize);
                            if (edge_schema->DetachProperty()) {
                                property = edge_schema->GetDetachedEdgeProperty(
                                    txn.GetTxn(), {vid, data.vid, data.lid, data.tid, data.eid});
                            }
                            EdgeUid euid{vid, data.vid, data.lid, data.tid, data.eid};
                            edge_schema->DeleteEdgeIndex(txn.GetTxn(), euid, property);
                            if (edge_schema->DetachProperty()) {
                                edge_schema->DeleteDetachedEdgeProperty(txn.GetTxn(), euid);
                            }
                            txn.GetEdgeDeltaCount()[data.lid]--;
                        } else {
                            if (vid == data.vid) {
                                // The in edge directing to self is already included
                                // in the out edges skip to avoid double deleting
                                continue;
                            }
                            Value property(data.prop, data.psize);
                            if (edge_schema->DetachProperty()) {
                                property = edge_schema->GetDetachedEdgeProperty(
                                    txn.GetTxn(), {data.vid, vid, data.lid, data.tid, data.eid});
                            }
                            EdgeUid euid{data.vid, vid, data.lid, data.tid, data.eid};
                            edge_schema->DeleteEdgeIndex(txn.GetTxn(), euid, property);
                            if (edge_schema->DetachProperty()) {
                                edge_schema->DeleteDetachedEdgeProperty(txn.GetTxn(), euid);
                            }
                            txn.GetEdgeDeltaCount()[data.lid]--;
                        }
                    }
                };
                bool r = graph_->DeleteVertex(txn.GetTxn(), vid, on_edge_deleted);
                FMA_DBG_ASSERT(r);
            } else {
                auto euid = graph::KeyPacker::GetEuidFromPropertyTableKey(
                    kv_iter->GetKey(), schema->GetLabelId());
                bool r = graph_->DeleteEdge(txn.GetTxn(), euid);
                FMA_DBG_ASSERT(r);
            }
            modified++;
            if (modified % 1000000 == 0) {
                LOG_INFO() << "modified: " << modified;
            }
        }
        LOG_INFO() << "modified: " << modified;
        kv_iter.reset();
        LOG_INFO() << FMA_FMT("end to scan detached table: {}", table_name);

        // delete index table
        auto indexed_fids = schema->GetIndexedFields();
        for (auto& fid : indexed_fids) {
            if (is_vertex) {
                index_manager_->DeleteVertexIndex(txn.GetTxn(), label,
                                                  schema->GetFieldExtractor(fid)->Name());
            } else {
                index_manager_->DeleteEdgeIndex(txn.GetTxn(), label,
                                                schema->GetFieldExtractor(fid)->Name());
            }
        }
        // delete detached property table
        schema->GetPropertyTable().Delete(txn.GetTxn());
    } else if (is_vertex) {  // now delete every node/edge that has this label
        std::vector<VertexIndex*> indexes;
        auto indexed_fids = schema->GetIndexedFields();
        for (auto fid : indexed_fids) {
            indexes.push_back(schema->GetFieldExtractor(fid)->GetVertexIndex());
        }
        // get unique index
        VertexIndex* unique_idx = nullptr;
        for (auto& idx : indexes)
            if (idx->IsUnique()) {
                unique_idx = idx;
                break;
            }
        // try and see if we should use bit set
        size_t mem_size = fma_common::HardwareInfo::GetAvailableMemory();
        VertexId start_vid, end_vid;
        txn.GetStartAndEndVid(start_vid, end_vid);
        size_t nv = (size_t)(end_vid - start_vid);
        bool use_bitset = (mem_size > nv / 4);
        if (use_bitset) {
            // if mem_size larger than 2 times size of bit set size, we can use a bit set
            std::vector<bool> match_label(nv, false);
            // if has unique index, check size
            size_t n_labeled = 0;
            if (unique_idx) {
                for (auto iit = unique_idx->GetUnmanagedIterator(txn.GetTxn(), Value(), Value());
                     iit.IsValid(); iit.Next()) {
                    match_label[iit.GetVid() - start_vid] = true;
                    n_labeled++;
                }
            } else {
                // otherwise, do a full scan
                for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
                    if (txn.GetVertexLabelId(vit) == lid) {
                        match_label[vit.GetId() - start_vid] = true;
                        n_labeled++;
                    }
                }
            }
            // if this label comprises a big fraction of all vertexes, use a scan-and-delete
            if (n_labeled * 10 >= nv) {
                graph_->_ScanAndDelete(
                    *store_, txn.GetTxn(),
                    [&](graph::VertexIterator& vit) {
                        return match_label[vit.GetId() - start_vid];
                    },  // should delete this node
                    [&](graph::InEdgeIterator& vit) {
                        return match_label[vit.GetSrc() - start_vid];
                    },  // should delete this in-edge
                    [&](graph::OutEdgeIterator& vit) {
                        return match_label[vit.GetDst() - start_vid];
                    },  // should delete this out-edge
                    modified);
            } else {
                // otherwise, delete the vertexes one by one
                for (size_t i = 0; i < match_label.size(); i++) {
                    if (match_label[i]) {
                        bool r = graph_->DeleteVertex(txn.GetTxn(), start_vid + (VertexId)i);
                        FMA_ASSERT(r);
                        modified++;
                        if (modified != 0 && modified % commit_size == 0) {
                            txn.Commit();
                            txn = CreateWriteTxn(false);
                        }
                    }
                }
            }
            FMA_DBG_CHECK_EQ(modified, n_labeled);
        } else {  // if (use_bitset)
            if (unique_idx && unique_idx->GetNumKeys(txn.GetTxn()) * 10 < nv) {
                // use unique_idx and delete vertexes one by one
                for (auto iit = unique_idx->GetUnmanagedIterator(txn.GetTxn(), Value(), Value());
                     iit.IsValid(); iit.Next()) {
                    bool r = graph_->DeleteVertex(txn.GetTxn(), iit.GetVid());
                    FMA_DBG_ASSERT(r);
                    modified++;
                    if (modified != 0 && modified % commit_size == 0) {
                        txn.Commit();
                        txn = CreateWriteTxn(false);
                    }
                }
                FMA_DBG_CHECK_EQ(modified, unique_idx->GetNumKeys(txn.GetTxn()));
            } else {
                // no bitset, no index
                graph_->_ScanAndDelete(
                    *store_, txn.GetTxn(),
                    [lid, &txn](graph::VertexIterator& vit) {
                        return txn.GetVertexLabelId(vit) == lid;
                    },
                    [lid, &txn](graph::InEdgeIterator& eit) {
                        // src might have just been deleted
                        auto vit = txn.GetVertexIterator(eit.GetSrc());
                        return !vit.IsValid() || txn.GetVertexLabelId(vit) == lid;
                    },
                    [lid, &txn](graph::OutEdgeIterator& eit) {
                        // dst might have just been deleted
                        auto vit = txn.GetVertexIterator(eit.GetDst());
                        return !vit.IsValid() || txn.GetVertexLabelId(vit) == lid;
                    },
                    modified);
            }
        }
        // delete indexes
        std::unique_ptr<SchemaInfo> schema_with_no_index(new SchemaInfo(*curr_schema_info.Get()));
        Schema* to_unindex = schema_with_no_index->v_schema_manager.GetSchema(lid);
        for (auto& fid : indexed_fids) {
            index_manager_->DeleteVertexIndex(txn.GetTxn(), label,
                                              schema->GetFieldExtractor(fid)->Name());
            to_unindex->UnVertexIndex(fid);
        }
    } else {
        // this is edge label, just scan and delete edges
        graph_->_ScanAndDelete(
            *store_, txn.GetTxn(), nullptr,
            [lid, &txn](graph::InEdgeIterator& eit) { return txn.GetEdgeLabelId(eit) == lid; },
            [lid, &txn](graph::OutEdgeIterator& eit) { return txn.GetEdgeLabelId(eit) == lid; },
            modified);

        auto indexed_fids = schema->GetIndexedFields();
        std::unique_ptr<SchemaInfo> schema_with_no_index(new SchemaInfo(*curr_schema_info.Get()));
        Schema* to_unedgeindex = schema_with_no_index->e_schema_manager.GetSchema(lid);
        for (auto& fid : indexed_fids) {
            index_manager_->DeleteEdgeIndex(txn.GetTxn(), label,
                                            schema->GetFieldExtractor(fid)->Name());
            to_unedgeindex->UnEdgeIndex(fid);
        }
    }
    // finally delete this label
    SchemaManager* new_sm =
        is_vertex ? &new_schema->v_schema_manager : &new_schema->e_schema_manager;
    bool r = new_sm->DeleteLabel(txn.GetTxn(), label);
    if (is_vertex) {
        // refill `EdgeConstraintsLids` with right vertex label id.
        new_schema->e_schema_manager.RefreshEdgeConstraintsLids(
            new_schema->v_schema_manager);
    }
    if (is_vertex) {
        txn.GetVertexLabelDelete().emplace(lid);
    } else {
        txn.GetEdgeLabelDelete().emplace(lid);
    }
    txn.Commit();
    // delete fulltext index if has any
    if (fulltext_index_) {
        fulltext_index_->DeleteLabel(is_vertex, schema->GetLabelId());
        fulltext_index_->Commit();
    }
    // assign new schema before commit, so that
    schema_.Assign(new_schema.release());
    store_->Flush();
    if (n_modified) *n_modified = modified;
    return r;
}

#define PERIODIC_COMMIT 0

template <typename GenNewSchema, typename MakeNewProp, typename ModifyIndex>
bool LightningGraph::_AlterLabel(
    bool is_vertex, const std::string& label,
    const GenNewSchema& gen_new_schema,                // std::function<Schema(Schema*)>
    const MakeNewProp& make_new_prop_and_destroy_old,  // std::function<Value(const Value&, Schema*,
                                                       // Schema*, Transaction&)>
    // std::function<void(Schema*, Schema*, CleanupActions&, Transaction&)>
    const ModifyIndex& modify_index,
    size_t* n_modified, size_t commit_size) {
    LOG_DEBUG() << "_AlterLabel(batch_size=" << commit_size << ")";
    _HoldWriteLock(meta_lock_);
    Transaction txn = CreateWriteTxn(false);
    ScopedRef<SchemaInfo> curr_schema_info = schema_.GetScopedRef();
    SchemaManager* curr_sm =
        is_vertex ? &curr_schema_info->v_schema_manager : &curr_schema_info->e_schema_manager;
    Schema* curr_schema = curr_sm->GetSchema(label);
    if (!curr_schema) return false;
    // copy schema
    std::unique_ptr<SchemaInfo> new_schema_info(new SchemaInfo(*curr_schema_info.Get()));
    std::unique_ptr<SchemaInfo> backup_schema(new SchemaInfo(*curr_schema_info.Get()));
    CleanupActions rollback_actions;

    SchemaManager* new_sm =
        is_vertex ? &new_schema_info->v_schema_manager : &new_schema_info->e_schema_manager;
    LabelId new_lid = new_sm->AlterLabel(txn.GetTxn(), label, gen_new_schema(curr_schema));
    Schema* new_schema = new_sm->GetSchema(new_lid);

    // TODO(hct): commit periodically to avoid too large transaction
    // Problem: If an exception occurs during vertex/edge update, we cannot rollback the committed
    // changes. We need a way to guarantee data consistency.

    // modify vertexes and edges if not fast alter schema
    size_t modified = 0;
    if (!new_schema->GetFastAlterSchema()) {
        // modify vertexes and edges
        size_t n_committed = 0;
        LabelId curr_lid = curr_schema->GetLabelId();
        if (curr_schema->DetachProperty()) {
            auto table_name = curr_schema->GetPropertyTable().Name();
            LOG_INFO() << FMA_FMT("begin to scan detached table: {}", table_name);
            auto kv_iter = curr_schema->GetPropertyTable().GetIterator(txn.GetTxn());
            for (kv_iter->GotoFirstKey(); kv_iter->IsValid(); kv_iter->Next()) {
                auto prop = kv_iter->GetValue();
                Value new_prop = make_new_prop_and_destroy_old(prop, curr_schema, new_schema, txn);
                kv_iter->SetValue(new_prop);
                modified++;
                if (modified % 1000000 == 0) {
                    LOG_INFO() << "modified: " << modified;
                }
            }
            LOG_INFO() << "modified: " << modified;
            kv_iter.reset();
            LOG_INFO() << FMA_FMT("end to scan detached table: {}", table_name);
        } else if (is_vertex) {
            // scan and modify the vertexes
            std::unique_ptr<lgraph::graph::VertexIterator> vit(
                new graph::VertexIterator(graph_->GetUnmanagedVertexIterator(&txn.GetTxn())));
            while (vit->IsValid()) {
                Value prop = vit->GetProperty();
                if (curr_sm->GetRecordLabelId(prop) == curr_lid) {
                    modified++;
                    Value new_prop =
                        make_new_prop_and_destroy_old(prop, curr_schema, new_schema, txn);
                    vit->RefreshContentIfKvIteratorModified();
                    vit->SetProperty(new_prop);
                    if (modified - n_committed >= commit_size) {
#if PERIODIC_COMMIT
                        VertexId vid = vit->GetId();
                        vit.reset();
                        txn.Commit();
                        n_committed = modified;
                        FMA_LOG() << "Committed " << n_committed << " changes.";
                        txn = CreateWriteTxn(false, false, false);
                        vit.reset(new lgraph::graph::VertexIterator(
                            graph_->GetUnmanagedVertexIterator(&txn.GetTxn(), vid, true)));
#else
                        n_committed = modified;
                        LOG_INFO() << "Made " << n_committed << " changes.";
#endif
                    }
                }
                vit->Next();
            }
        } else {
            // scan and modify
            std::unique_ptr<lgraph::graph::VertexIterator> vit(new lgraph::graph::VertexIterator(
                graph_->GetUnmanagedVertexIterator(&txn.GetTxn())));
            while (vit->IsValid()) {
                for (auto eit = vit->GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                    if (eit.GetLabelId() == curr_lid) {
                        modified++;
                        Value property = eit.GetProperty();
                        Value new_prop =
                            make_new_prop_and_destroy_old(property, curr_schema, new_schema, txn);
                        eit.RefreshContentIfKvIteratorModified();
                        eit.SetProperty(new_prop);
                    }
                }
                vit->RefreshContentIfKvIteratorModified();
                for (auto eit = vit->GetInEdgeIterator(); eit.IsValid(); eit.Next()) {
                    if (eit.GetLabelId() == curr_lid) {
                        Value property = eit.GetProperty();
                        Value new_prop =
                            make_new_prop_and_destroy_old(property, curr_schema, new_schema, txn);
                        eit.RefreshContentIfKvIteratorModified();
                        eit.SetProperty(new_prop);
                    }
                }
                vit->RefreshContentIfKvIteratorModified();
                if (modified - n_committed >= commit_size) {
#if PERIODIC_COMMIT
                    VertexId vid = vit->GetId();
                    vit.reset();
                    txn.Commit();
                    n_committed = modified;
                    FMA_LOG() << "Committed " << n_committed << " changes.";
                    txn = CreateWriteTxn(false, false, false);
                    vit.reset(new lgraph::graph::VertexIterator(
                        graph_->GetUnmanagedVertexIterator(&txn.GetTxn(), vid, true)));
#else
                    n_committed = modified;
                    LOG_INFO() << "Made " << n_committed << " changes.";
#endif
                }
                vit->Next();
            }
        }
    }
    modify_index(curr_schema, new_schema, rollback_actions, txn);

    // assign new schema and commit
    schema_.Assign(new_schema_info.release());
    rollback_actions.Emplace([&]() { schema_.Assign(backup_schema.release()); });
    txn.Commit();
    if (n_modified) *n_modified = modified;
    rollback_actions.CancelAll();
    return true;
}

/* edge_constraints can only append, no delete allowed */
bool LightningGraph::ClearEdgeConstraints(const std::string& edge_label) {
    _HoldWriteLock(meta_lock_);
    Transaction txn = CreateWriteTxn(false);
    ScopedRef<SchemaInfo> curr_schema_info = schema_.GetScopedRef();
    std::unique_ptr<SchemaInfo> new_schema_info(new SchemaInfo(*curr_schema_info.Get()));
    Schema* e_schema = new_schema_info->e_schema_manager.GetSchema(edge_label);
    if (!e_schema) {
        THROW_CODE(InputError, "No such edge label: " + edge_label);
    }
    Schema new_e_schema(*e_schema);
    new_e_schema.SetEdgeConstraints(EdgeConstraints());
    new_schema_info->e_schema_manager.AlterLabel(txn.GetTxn(), edge_label, new_e_schema);
    new_schema_info->e_schema_manager.RefreshEdgeConstraintsLids(new_schema_info->v_schema_manager);
    txn.Commit();
    // assign new schema
    schema_.Assign(new_schema_info.release());
    return true;
}

bool LightningGraph::AddEdgeConstraints(const std::string& edge_label,
                                             const EdgeConstraints& constraints) {
    {
        // check empty
        if (constraints.empty()) {
            THROW_CODE(InputError, "Constraints are empty");
        }
        // check duplicate
        auto temp = constraints;
        std::sort(temp.begin(), temp.end());
        auto iter = std::unique(temp.begin(), temp.end());
        if (iter != temp.end()) {
            THROW_CODE(InputError, "Duplicate constraints: [{}, {}]", iter->first, iter->second);
        }
    }
    _HoldWriteLock(meta_lock_);
    Transaction txn = CreateWriteTxn(false);
    ScopedRef<SchemaInfo> curr_schema_info = schema_.GetScopedRef();
    std::unique_ptr<SchemaInfo> new_schema_info(new SchemaInfo(*curr_schema_info.Get()));
    Schema* e_schema = new_schema_info->e_schema_manager.GetSchema(edge_label);
    if (!e_schema) {
        THROW_CODE(InputError, "No such edge label: " + edge_label);
    }
    Schema new_e_schema(*e_schema);
    EdgeConstraints ecs = new_e_schema.GetEdgeConstraints();
    if (ecs.empty()) {
        THROW_CODE(InputError, "Failed to add constraint: "
            "edge[{}] constraints are empty", edge_label);
    }
    for (auto& ec : constraints) {
        for (auto& vertex_label : {ec.first, ec.second}) {
            if (!new_schema_info->v_schema_manager.GetSchema(vertex_label)) {
                THROW_CODE(InputError, "No such vertex label: {}", vertex_label);
            }
        }
        auto iter = std::find_if(ecs.begin(), ecs.end(),
                                 [&ec](auto &item){return ec == item;});
        if (iter != ecs.end()) {
            THROW_CODE(InputError, "Failed to add constraint: "
                "constraint [{},{}] already exist", ec.first, ec.second);
        }
    }
    ecs.insert(ecs.end(), constraints.begin(), constraints.end());
    new_e_schema.SetEdgeConstraints(ecs);
    new_schema_info->e_schema_manager.AlterLabel(txn.GetTxn(), edge_label, new_e_schema);
    new_schema_info->e_schema_manager.RefreshEdgeConstraintsLids(new_schema_info->v_schema_manager);
    txn.Commit();
    // assign new schema
    schema_.Assign(new_schema_info.release());
    return true;
}

bool LightningGraph::AlterLabelModEdgeConstraints(const std::string& label,
                                                  const EdgeConstraints& edge_constraints) {
    _HoldReadLock(meta_lock_);
    Transaction txn = CreateWriteTxn(false);
    ScopedRef<SchemaInfo> curr_schema_info = schema_.GetScopedRef();
    std::unique_ptr<SchemaInfo> new_schema_info(new SchemaInfo(*curr_schema_info.Get()));
    std::unique_ptr<SchemaInfo> backup_schema(new SchemaInfo(*curr_schema_info.Get()));
    Schema* curr_schema = curr_schema_info->e_schema_manager.GetSchema(label);
    if (!curr_schema) return false;
    {
        // Check if new edge_constraints contain old ones
        EdgeConstraints old_ec = curr_schema->GetEdgeConstraints();
        EdgeConstraints new_ec = edge_constraints;
        std::sort(old_ec.begin(), old_ec.end());
        std::sort(new_ec.begin(), new_ec.end());
        auto old_p = old_ec.begin();
        auto new_p = new_ec.begin();
        while (old_p != old_ec.end() && new_p != new_ec.end()) {
            if ((*old_p) == (*new_p)) {
                old_p++;
            } else {
                new_p++;
            }
        }
        if (old_p != old_ec.end()) return false;
    }
    Schema new_schema(*curr_schema);
    new_schema.SetEdgeConstraints(edge_constraints);
    new_schema_info->e_schema_manager.AlterLabel(txn.GetTxn(), label, new_schema);

    CleanupActions rollback_actions;
    // assign new schema and commit
    schema_.Assign(new_schema_info.release());
    rollback_actions.Emplace([&]() { schema_.Assign(backup_schema.release()); });
    txn.Commit();
    rollback_actions.CancelAll();
    return true;
}

bool LightningGraph::AlterLabelDelFields(const std::string& label,
                                         const std::vector<std::string>& del_fields_,
                                         bool is_vertex, size_t* n_modified) {
    LOG_INFO() << FMA_FMT("Deleting fields {} from {} label [{}].", del_fields_,
                                        is_vertex ? "vertex" : "edge", label);
    _HoldReadLock(meta_lock_);
    // make unique
    std::vector<std::string> del_fields(del_fields_);
    std::sort(del_fields.begin(), del_fields.end());
    del_fields.erase(std::unique(del_fields.begin(), del_fields.end()), del_fields.end());
    if (del_fields.empty()) THROW_CODE(InputError, "No fields specified.");

    // get fids of the fields in new schema
    std::vector<size_t> new_fids;
    std::vector<size_t> old_field_pos;
    std::vector<const _detail::FieldExtractorBase*> blob_deleted_fes;

    // make new schema
    auto setup_and_gen_new_schema = [&](Schema* curr_schema) -> Schema {
        Schema new_schema(*curr_schema);
        if (curr_schema->GetFastAlterSchema()) {
            new_schema.DelFields(del_fields);
            return new_schema;
        }
        new_schema.DelFields(del_fields);
        size_t n_new_fields = new_schema.GetNumFields();
        for (size_t i = 0; i < n_new_fields; i++) new_fids.push_back(i);
        // setup auxiliary data
        for (size_t i = 0; i < n_new_fields; i++)
            old_field_pos.push_back(
                curr_schema->GetFieldId(new_schema.GetFieldExtractor(i)->Name()));
        for (auto& f : del_fields) {
            auto* extractor = curr_schema->GetFieldExtractor(f);
            if (extractor->Type() == FieldType::BLOB) {
                blob_deleted_fes.push_back(extractor);
            }
        }
        return new_schema;
    };

    // modify vertexes and edges
    auto make_new_prop_and_destroy_old = [&](const Value& old_prop, Schema* curr_schema,
                                             Schema* new_schema, Transaction& txn) {
        // recreate property
        Value new_prop = new_schema->CreateEmptyRecord(old_prop.Size());
        new_schema->CopyFieldsRaw(new_prop, new_fids, curr_schema, old_prop, old_field_pos);
        if (blob_deleted_fes.empty()) return new_prop;
        // delete large blobs if necessary
        Value old_prop_copy;  // copy the old property in case write ops invalidates pointer
        if (!blob_deleted_fes.empty()) old_prop_copy.Copy(old_prop.Data(), old_prop.Size());
        for (auto& fe : blob_deleted_fes) {
            const Value& v = fe->GetConstRef(old_prop_copy);
            if (BlobManager::IsLargeBlob(v)) {
                BlobManager::BlobKey key = BlobManager::GetLargeBlobKey(v);
                blob_manager_->Delete(txn.GetTxn(), key);
            }
        }
        return new_prop;
    };

    auto delete_indexes = [&](Schema* curr_schema, Schema* new_schema,
                              CleanupActions& rollback_actions, Transaction& txn) {
        // delete the indexes
        auto fids = curr_schema->GetFieldIds(del_fields);
        // delete indexes if necessary
        for (auto& f : fids) {
            auto* extractor = curr_schema->GetFieldExtractor(f);
            if (extractor->GetVertexIndex()) {
                // delete vertex index
                index_manager_->DeleteVertexIndex(txn.GetTxn(), label, extractor->Name());
            } else if (extractor->GetEdgeIndex()) {
                // delete edge index
                index_manager_->DeleteEdgeIndex(txn.GetTxn(), label, extractor->Name());
            } else if (extractor->FullTextIndexed()) {
                // delete fulltext index
                index_manager_->DeleteFullTextIndex(txn.GetTxn(), is_vertex, label,
                                                    extractor->Name());
            } else if (extractor->GetVectorIndex()) {
                index_manager_->DeleteVectorIndex(txn.GetTxn(), label, extractor->Name());
            }
        }
        auto composite_index_key = curr_schema->GetRelationalCompositeIndexKey(fids);
        for (const auto &cidx : composite_index_key) {
            index_manager_->DeleteVertexCompositeIndex(txn.GetTxn(), label, cidx);
        }
    };

    return _AlterLabel(is_vertex, label, setup_and_gen_new_schema, make_new_prop_and_destroy_old,
                       delete_indexes, n_modified, 100000);
}

bool LightningGraph::AlterLabelAddFields(const std::string& label,
                                         const std::vector<FieldSpec>& to_add,
                                         const std::vector<FieldData>& default_values,
                                         bool is_vertex, size_t* n_modified) {
    LOG_INFO() << FMA_FMT("Adding fields {} with values {} to {} label [{}].", to_add,
                                        default_values, is_vertex ? "vertex" : "edge", label);
    _HoldReadLock(meta_lock_);
    if (to_add.empty()) THROW_CODE(InputError, "No fields specified.");
    if (to_add.size() != default_values.size())
        THROW_CODE(InputError, "Number of fields and default values are not equal.");
    // de-duplicate
    {
        std::unordered_set<std::string> s;
        for (auto& f : to_add) {
            if (s.find(f.name) != s.end())
                THROW_CODE(InputError, "Field [{}] is defined more than once.", f.name);
            s.insert(f.name);
        }
    }
    // check input
    {
        for (size_t i = 0; i < to_add.size(); i++) {
            auto& fs = to_add[i];
            if (!fs.optional && default_values[i].IsNull())
                THROW_CODE(InputError,
                           "Field [{}] is declared as non-optional but the default value is NULL.",
                           fs.name);
        }
    }

    std::vector<size_t> dst_fids;  // field ids of old fields in new record
    std::vector<size_t> src_fids;  // field ids of old fields in old record
    std::vector<size_t> new_fids;  // ids of newly added fields
    // make new schema
    auto setup_and_gen_new_schema = [&](Schema* curr_schema) -> Schema {
        if (curr_schema->GetFastAlterSchema()) {
            Schema new_schema(*curr_schema);
            // check type complatible
            for (size_t i = 0; i < to_add.size(); i++) {
                if (!FieldTypeComplatible(default_values[i].GetType(), to_add[i].type)) {
                    throw ParseIncompatibleTypeException(to_add[i].name, to_add[i].type,
                                                         default_values[i].type);
                }
            }
            new_schema.AddFields(to_add);
            for (size_t i = 0; i < to_add.size(); i++) {
                auto extractor = new_schema.GetFieldExtractor(to_add[i].name);
                extractor->SetDefaultValue(default_values[i]);
            }
            return new_schema;
        }
        Schema new_schema(*curr_schema);
        new_schema.AddFields(to_add);
        // setup auxiliary data

        // data to copy
        dst_fids.reserve(curr_schema->GetNumFields());
        src_fids.reserve(curr_schema->GetNumFields());
        for (size_t i = 0; i < new_schema.GetNumFields(); i++) {
            size_t fid = 0;
            if (curr_schema->TryGetFieldId(new_schema.GetFieldExtractor(i)->Name(), fid)) {
                dst_fids.push_back(i);
                src_fids.push_back(fid);
            }
        }
        // new data
        new_fids.reserve(to_add.size());
        for (size_t i = 0; i < to_add.size(); i++) {
            new_fids.push_back(new_schema.GetFieldId(to_add[i].name));
        }
        return new_schema;
    };

    // modify vertexes and edges
    auto make_new_prop_and_destroy_old = [&](const Value& old_prop, Schema* curr_schema,
                                             Schema* new_schema, Transaction& txn) {
        // recreate property
        Value new_prop = new_schema->CreateEmptyRecord(old_prop.Size());
        new_schema->CopyFieldsRaw(new_prop, dst_fids, curr_schema, old_prop, src_fids);
        for (size_t i = 0; i < new_fids.size(); i++) {
            size_t fid = new_fids[i];
            auto* extr =
                Schema::GetFieldExtractorV1(new_schema->GetFieldExtractor(fid));
            if (extr->Type() == FieldType::BLOB) {
                extr->ParseAndSetBlob(new_prop, default_values[i], [&](const Value& v) {
                    return blob_manager_->Add(txn.GetTxn(), v);
                });
            } else {
                extr->ParseAndSet(new_prop, default_values[i]);
            }
        }
        return new_prop;
    };

    auto delete_indexes = [](Schema* curr_schema, Schema* new_schema,
                             CleanupActions& rollback_actions, Transaction& txn) {};

    return _AlterLabel(is_vertex, label, setup_and_gen_new_schema, make_new_prop_and_destroy_old,
                       delete_indexes, n_modified, 100000);
}

bool LightningGraph::FieldTypeComplatible(FieldType default_value, FieldType b) {
    if (default_value == b) return true;
    if (default_value == FieldType::NUL) return true;

    if ((lgraph_api::is_float_type(default_value)
        && lgraph_api::is_float_type(b))
        || (lgraph_api::is_integer_type(default_value)
        && lgraph_api::is_integer_type(b))) {
        return true;
    }
    return false;
}

bool LightningGraph::AlterLabelModFields(const std::string& label,
                                         const std::vector<FieldSpec>& to_mod, bool is_vertex,
                                         size_t* n_modified) {
    LOG_INFO() << FMA_FMT("Modifying fields {} in {} label [{}].", to_mod,
                                        is_vertex ? "vertex" : "edge", label);
    _HoldReadLock(meta_lock_);
    if (to_mod.empty()) THROW_CODE(InputError, "No fields specified.");
    // de-duplicate
    {
        std::unordered_set<std::string> s;
        for (auto& f : to_mod) {
            if (s.find(f.name) != s.end())
                THROW_CODE(InputError, "Field [{}] is specified more than once.", f.name);
            s.insert(f.name);
        }
    }

    // make new schema
    std::vector<size_t> direct_copy_dst_fids;
    std::vector<size_t> direct_copy_src_fids;
    std::vector<size_t> mod_dst_fids;
    std::vector<size_t> mod_src_fids;
    auto setup_and_gen_new_schema = [&](Schema* curr_schema) -> Schema {
        if (curr_schema->GetFastAlterSchema()) {
            // check field types
            for (auto& f : to_mod) {
                auto* extractor = curr_schema->GetFieldExtractor(f.name);
                if (extractor->Type() == f.type) {
                    continue;
                }

                if (!FieldTypeComplatible(extractor->Type(), f.type)) {
                    THROW_CODE(InputError,
                               "Enabled fast alter schema, only support convert from float_type to "
                               "float_type or"
                               "integer_type to integer_type");
                }

                if (extractor->FullTextIndexed()) {
                    THROW_CODE(InputError,
                               "Field [{}] has fulltext index, which cannot be converted to other "
                               "non-STRING types.",
                               f.name);
                }
            }
            Schema new_schema(*curr_schema);
            new_schema.ModFields(to_mod);
            FMA_DBG_ASSERT(new_schema.GetNumFields() == curr_schema->GetNumFields());
            return new_schema;
        }
        // check field types
        for (auto& f : to_mod) {
            auto* extractor = curr_schema->GetFieldExtractor(f.name);
            if (extractor->Type() == FieldType::BLOB && f.type != FieldType::BLOB) {
                THROW_CODE(InputError,
                           "Field [{}] is of type BLOB, which cannot be converted to other types.",
                            f.name);
            }
            if (extractor->FullTextIndexed()) {
                THROW_CODE(InputError,
                    "Field [{}] has fulltext index, which cannot be converted to other "
                            "non-STRING types.",
                            f.name);
            }
        }
        Schema new_schema(*curr_schema);
        new_schema.ModFields(to_mod);
        FMA_DBG_ASSERT(new_schema.GetNumFields() == curr_schema->GetNumFields());
        for (size_t i = 0; i < new_schema.GetNumFields(); i++) {
            const _detail::FieldExtractorV1* dst_fe =
                Schema::GetFieldExtractorV1(new_schema.GetFieldExtractor(i));
            const std::string& fname = dst_fe->Name();
            const _detail::FieldExtractorV1* src_fe =
                Schema::GetFieldExtractorV1(curr_schema->GetFieldExtractor(i));
            size_t src_fid = curr_schema->GetFieldId(fname);
            if (dst_fe->Type() == src_fe->Type()) {
                direct_copy_dst_fids.push_back(i);
                direct_copy_src_fids.push_back(src_fid);
            } else {
                mod_dst_fids.push_back(i);
                mod_src_fids.push_back(src_fid);
            }
        }
        return new_schema;
    };

    // modify vertexes and edges
    auto make_new_prop_and_destroy_old = [&](const Value& old_prop, Schema* curr_schema,
                                             Schema* new_schema, Transaction& txn) {
        // recreate property
        Value new_prop = new_schema->CreateEmptyRecord(old_prop.Size());
        new_schema->CopyFieldsRaw(new_prop, direct_copy_dst_fids, curr_schema, old_prop,
                                  direct_copy_src_fids);
        for (size_t i = 0; i < mod_dst_fids.size(); i++) {
            const _detail::FieldExtractorV1* dst_fe =
                Schema::GetFieldExtractorV1(new_schema->GetFieldExtractor(mod_dst_fids[i]));
            FieldData data = curr_schema->GetField(old_prop, mod_src_fids[i],
                                                   [&](const BlobManager::BlobKey& key) {
                                                       return blob_manager_->Get(txn.GetTxn(), key);
                                                   });
            if (dst_fe->Type() == FieldType::BLOB) {
                // strings are copied directly to blob, other types will fail
                if (data.IsString()) data.type = FieldType::BLOB;
                dst_fe->ParseAndSetBlob(new_prop, data, [&](const Value& blob) {
                    return blob_manager_->Add(txn.GetTxn(), blob);
                });
            } else {
                dst_fe->ParseAndSet(new_prop, data);
            }
        }
        return new_prop;
    };

    // delete indexes of modified fields
    auto delete_indexes = [&](Schema* curr_schema, Schema* new_schema,
                              CleanupActions& rollback_actions, Transaction& txn) {
        std::vector<size_t> mod_fids;
        for (auto& f : to_mod) mod_fids.push_back(curr_schema->GetFieldId(f.name));
        // delete indexes if necessary
        for (auto& f : mod_fids) {
            auto* extractor = curr_schema->GetFieldExtractor(f);
            if (extractor->GetVertexIndex()) {
                new_schema->UnVertexIndex(f);
                index_manager_->DeleteVertexIndex(txn.GetTxn(), label, extractor->Name());
            } else if (extractor->GetEdgeIndex()) {
                new_schema->UnEdgeIndex(f);
                index_manager_->DeleteEdgeIndex(txn.GetTxn(), label, extractor->Name());
            }
        }
        auto composite_index_key = curr_schema->GetRelationalCompositeIndexKey(mod_fids);
        for (const auto &cidx : composite_index_key) {
            index_manager_->DeleteVertexCompositeIndex(txn.GetTxn(), label, cidx);
        }
    };

    return _AlterLabel(
        is_vertex, label, setup_and_gen_new_schema, make_new_prop_and_destroy_old, delete_indexes,
        n_modified,
#if PERIODIC_COMMIT
        std::numeric_limits<size_t>::max());  // there could be data conversion error during
                                              // convert, so we cannot do periodic commit
#else
        100000);
#endif
}

/**
 * Adds an index to 'label:field'
 *
 * \param   label       The label.
 * \param   field       The field.
 * \param   type        The index type.
 * \param   is_vertex   True if this is vertex label, otherwise it is edge label.
 *
 * \return  True if it succeeds, false if the index already exists. Throws exception on error.
 */
bool LightningGraph::_AddEmptyIndex(const std::string& label, const std::string& field,
                                    IndexType type, bool is_vertex) {
    _HoldWriteLock(meta_lock_);
    Transaction txn = CreateWriteTxn(false);
    std::unique_ptr<SchemaInfo> new_schema(new SchemaInfo(*schema_.GetScopedRef().Get()));
    Schema* schema = is_vertex ? new_schema->v_schema_manager.GetSchema(label)
                               : new_schema->e_schema_manager.GetSchema(label);
    if (!schema) throw LabelNotExistException(label);
    const _detail::FieldExtractorBase* extractor = schema->GetFieldExtractor(field);
    if ((extractor->GetVertexIndex() && is_vertex) || (extractor->GetEdgeIndex() && !is_vertex))
        return false;  // index already exist
    if (is_vertex) {
        std::unique_ptr<VertexIndex> index;
        index_manager_->AddVertexIndex(txn.GetTxn(), label, field, extractor->Type(), type,
                                       index);
        index->SetReady();
        schema->MarkVertexIndexed(extractor->GetFieldId(), index.release());
    } else {
        std::unique_ptr<EdgeIndex> edge_index;
        index_manager_->AddEdgeIndex(txn.GetTxn(), label, field, extractor->Type(),
                                     type, edge_index);
        edge_index->SetReady();
        schema->MarkEdgeIndexed(extractor->GetFieldId(), edge_index.release());
    }
    // add index before commit
    txn.Commit();
    // install the new index
    schema_.Assign(new_schema.release());
    return true;
}

class ConstStringRef {
    struct {
        uint64_t size : 16;
        uint64_t ptr : 48;
    } size_ptr_;

 public:
    ConstStringRef() {
        size_ptr_.size = 0;
        size_ptr_.ptr = 0;
    }

    ConstStringRef(const char* p, size_t s) {
        if (s > std::numeric_limits<uint16_t>::max())
            throw std::runtime_error(FMA_FMT("String size too large: {}", s));
        size_ptr_.size = s;
        uint64_t up = (uint64_t)p;
        if ((up & ((uint64_t)0xFFFF << 48)) != 0)
            throw std::runtime_error(FMA_FMT(
                "Pointer larger than 48 bit is not supported: {}", (void*)p));
        size_ptr_.ptr = (uint64_t)p;
    }

    const char* Ptr() const { return reinterpret_cast<const char*>(size_ptr_.ptr); }
    size_t Size() const { return size_ptr_.size; }

    bool operator<(const ConstStringRef& rhs) const {
        size_t sa = size_ptr_.size;
        size_t sb = rhs.size_ptr_.size;
        const char* pa = reinterpret_cast<const char*>(size_ptr_.ptr);
        const char* pb = reinterpret_cast<const char*>(rhs.size_ptr_.ptr);
        int r = strncmp(pa, pb, std::min(sa, sb));
        if (r < 0) return true;
        if (r == 0) return sa < sb;
        return false;
    }

    bool operator<=(const ConstStringRef& rhs) const {
        size_t sa = size_ptr_.size;
        size_t sb = rhs.size_ptr_.size;
        const char* pa = reinterpret_cast<const char*>(size_ptr_.ptr);
        const char* pb = reinterpret_cast<const char*>(rhs.size_ptr_.ptr);
        int r = strncmp(pa, pb, std::min(sa, sb));
        if (r < 0) return true;
        if (r == 0) return sa <= sb;
        return false;
    }

    bool operator==(const ConstStringRef& rhs) const {
        size_t sa = size_ptr_.size;
        size_t sb = rhs.size_ptr_.size;
        const char* pa = reinterpret_cast<const char*>(size_ptr_.ptr);
        const char* pb = reinterpret_cast<const char*>(rhs.size_ptr_.ptr);
        return sa == sb && (strncmp(pa, pb, sa) == 0);
    }

    bool operator!=(const ConstStringRef& rhs) const { return !(*this == rhs); }

    std::string ToString() const { return std::string(Ptr(), Size()); }
};

static_assert(sizeof(ConstStringRef) == 8, "Assuming ConstStringRef to take 8 types");

template <typename T>
struct KeyVid {
    T key;
    VertexId vid;

    KeyVid(const T& k, VertexId v) : key(k), vid(v) {}
    KeyVid() : key(T()), vid(0) {}

    bool operator<(const KeyVid& rhs) const {
        return key < rhs.key || (key == rhs.key && vid < rhs.vid);
    }
};

struct CompositeKeyVid {
    std::vector<Value> keys;
    std::vector<FieldType> types;
    VertexId vid;

    CompositeKeyVid(const std::vector<Value>& k, const std::vector<FieldType>& t,
                    VertexId v) : keys(k), types(t), vid(v) {}
    CompositeKeyVid() : keys(std::vector<Value>()), types(std::vector<FieldType>()), vid(0) {}

    bool operator<(const CompositeKeyVid& rhs) const {
        int n = keys.size();
        for (int i = 0; i < n; ++i) {
            switch (types[i]) {
            case FieldType::BOOL:
                if (keys[i].AsType<bool>() == rhs.keys[i].AsType<bool>()) continue;
                return keys[i].AsType<bool>() < rhs.keys[i].AsType<bool>();
            case FieldType::INT8:
                if (keys[i].AsType<int8_t>() == rhs.keys[i].AsType<int8_t>()) continue;
                return keys[i].AsType<int8_t>() < rhs.keys[i].AsType<int8_t>();
            case FieldType::INT16:
                if (keys[i].AsType<int16_t>() == rhs.keys[i].AsType<int16_t>()) continue;
                return keys[i].AsType<int16_t>() < rhs.keys[i].AsType<int16_t>();
            case FieldType::INT32:
                if (keys[i].AsType<int32_t>() == rhs.keys[i].AsType<int32_t>()) continue;
                return keys[i].AsType<int32_t>() < rhs.keys[i].AsType<int32_t>();
            case FieldType::DATE:
                if (keys[i].AsType<int32_t>() == rhs.keys[i].AsType<int32_t>()) continue;
                return keys[i].AsType<int32_t>() < rhs.keys[i].AsType<int32_t>();
            case FieldType::INT64:
                if (keys[i].AsType<int64_t>() == rhs.keys[i].AsType<int64_t>()) continue;
                return keys[i].AsType<int64_t>() < rhs.keys[i].AsType<int64_t>();
            case FieldType::DATETIME:
                if (keys[i].AsType<int64_t>() == rhs.keys[i].AsType<int64_t>()) continue;
                return keys[i].AsType<int64_t>() < rhs.keys[i].AsType<int64_t>();
            case FieldType::FLOAT:
                if (keys[i].AsType<float>() == rhs.keys[i].AsType<float>()) continue;
                return keys[i].AsType<float>() < rhs.keys[i].AsType<float>();
            case FieldType::DOUBLE:
                if (keys[i].AsType<double>() == rhs.keys[i].AsType<double>()) continue;
                return keys[i].AsType<double>() < rhs.keys[i].AsType<double>();
            case FieldType::STRING:
                if (keys[i].AsType<std::string>() == rhs.keys[i].AsType<std::string>()) continue;
                return keys[i].AsType<std::string>() < rhs.keys[i].AsType<std::string>();
            case FieldType::BLOB:
                THROW_CODE(KvException, "Blob fields cannot act as key.");
            default:
                THROW_CODE(KvException, "Unknown data type: {}", types[i]);
            }
        }
        return vid < rhs.vid;
    }

    bool operator==(const CompositeKeyVid& rhs) const {
        int n = keys.size();
        for (int i = 0; i < n; ++i) {
            switch (types[i]) {
            case FieldType::BOOL:
                if (keys[i].AsType<bool>() == rhs.keys[i].AsType<bool>()) continue;
                return false;
            case FieldType::INT8:
                if (keys[i].AsType<int8_t>() == rhs.keys[i].AsType<int8_t>()) continue;
                return false;
            case FieldType::INT16:
                if (keys[i].AsType<int16_t>() == rhs.keys[i].AsType<int16_t>()) continue;
                return false;
            case FieldType::INT32:
                if (keys[i].AsType<int32_t>() == rhs.keys[i].AsType<int32_t>()) continue;
                return false;
            case FieldType::DATE:
                if (keys[i].AsType<int32_t>() == rhs.keys[i].AsType<int32_t>()) continue;
                return false;
            case FieldType::INT64:
                if (keys[i].AsType<int64_t>() == rhs.keys[i].AsType<int64_t>()) continue;
                return false;
            case FieldType::DATETIME:
                if (keys[i].AsType<int64_t>() == rhs.keys[i].AsType<int64_t>()) continue;
                return false;
            case FieldType::FLOAT:
                if (keys[i].AsType<float>() == rhs.keys[i].AsType<float>()) continue;
                return false;
            case FieldType::DOUBLE:
                if (keys[i].AsType<double>() == rhs.keys[i].AsType<double>()) continue;
                return false;
            case FieldType::STRING:
                if (keys[i].AsType<std::string>() == rhs.keys[i].AsType<std::string>()) continue;
                return false;
            case FieldType::BLOB:
                THROW_CODE(KvException, "Blob fields cannot act as key.");
            default:
                THROW_CODE(KvException, "Unknown data type: {}", types[i]);
            }
        }
        return true;
    }
};

template <typename T>
struct KeyEUid {
    T key;
    EdgeUid euid;

    KeyEUid(const T& k, EdgeUid u) : key(k), euid(u) {}
    KeyEUid() : key(T()), euid() {}

    bool operator<(const KeyEUid& rhs) const {
        return key < rhs.key || (key == rhs.key && euid.src < rhs.euid.src) ||
               (key == rhs.key && euid.src == rhs.euid.src && euid.dst < rhs.euid.dst) ||
               (key == rhs.key && euid.src == rhs.euid.src && euid.dst == rhs.euid.dst &&
                euid.lid < rhs.euid.lid) ||
               (key == rhs.key && euid.src == rhs.euid.src && euid.dst == rhs.euid.dst &&
                euid.lid == rhs.euid.lid && euid.tid < rhs.euid.tid) ||
               (key == rhs.key && euid.src == rhs.euid.src && euid.dst == rhs.euid.dst &&
                euid.lid == rhs.euid.lid && euid.tid == rhs.euid.tid && euid.eid < rhs.euid.eid);
    }
};
template <typename T>
T GetIndexKeyFromValue(const Value& v) {
    return v.AsType<T>();
}

template <>
ConstStringRef GetIndexKeyFromValue<ConstStringRef>(const Value& v) {
    return ConstStringRef(v.Data(), v.Size());
}

template <>
std::string GetIndexKeyFromValue<std::string>(const Value& v) {
    return v.AsString();
}

template <typename T>
inline Value GetKeyConstRef(const T& key) {
    return Value::ConstRef(key);
}

inline Value GetKeyConstRef(const ConstStringRef& key) { return Value(key.Ptr(), key.Size()); }

template <typename T>
void LightningGraph::BatchBuildIndex(Transaction& txn, SchemaInfo* new_schema_info,
                                     LabelId label_id, size_t field_id, IndexType type,
                                     VertexId start_vid, VertexId end_vid, bool is_vertex) {
    if (is_vertex) {
        SchemaManager* schema_manager = &new_schema_info->v_schema_manager;
        auto v_schema = schema_manager->GetSchema(label_id);
        auto* field_extractor = v_schema->GetFieldExtractor(field_id);
        FMA_DBG_ASSERT(field_extractor);
        VertexIndex* index = field_extractor->GetVertexIndex();
        FMA_DBG_ASSERT(index);
        static const size_t max_block_size = 1 << 28;
        for (VertexId vid = start_vid; vid < end_vid; vid += max_block_size) {
            std::vector<KeyVid<T>> key_vids;
            VertexId curr_end = std::min<VertexId>(end_vid, vid + max_block_size);
            key_vids.reserve(curr_end - vid);
            for (auto it = txn.GetVertexIterator(vid, true); it.IsValid() && it.GetId() < curr_end;
                 it.Next()) {
                Value prop = it.GetProperty();
                if (lgraph::SchemaManager::GetRecordLabelId(prop) != label_id) continue;
                if (v_schema->DetachProperty()) {
                    prop = v_schema->GetDetachedVertexProperty(txn.GetTxn(), it.GetId());
                }
                if (field_extractor->GetIsNull(prop)) {
                    continue;
                }
                const T& key = GetIndexKeyFromValue<T>(field_extractor->GetConstRef(prop));
                key_vids.emplace_back(key, it.GetId());
            }
            LGRAPH_PSORT(key_vids.begin(), key_vids.end());
            // now insert into index table
            if (max_block_size >= (size_t)(end_vid - start_vid)) {
                // block size large enough, so there is only one pass, use AppendKv
                switch (type) {
                case IndexType::GlobalUniqueIndex:
                    {
                        // if there is only one block, we use AppendKv,
                        // so checking for duplicate is required
                        // if there are multiple blocks,
                        // then uniqueness will be checked when we insert the
                        // keys into index, and this is not required,
                        // but still good to find duplicates early
                        for (size_t i = 1; i < key_vids.size(); i++) {
                            if (key_vids[i].key == key_vids[i - 1].key)
                                THROW_CODE(InputError,
                                    "Duplicate vertex keys [{}] found for vids {} and {}.",
                                    key_vids[i].key, key_vids[i - 1].vid, key_vids[i].vid);
                        }
                        for (auto& kv : key_vids)
                            index->_AppendVertexIndexEntry(txn.GetTxn(), GetKeyConstRef(kv.key),
                                                           (VertexId)kv.vid);
                        break;
                    }
                case IndexType::NonuniqueIndex:
                    {
                        T key = key_vids.empty() ? T() : key_vids.front().key;
                        std::vector<VertexId> vids;
                        for (size_t i = 0; i < key_vids.size(); ++i) {
                            auto& kv = key_vids[i];
                            if (key != kv.key) {
                                // write out a bunch of vids
                                index->_AppendNonUniqueVertexIndexEntry(txn.GetTxn(),
                                                                        GetKeyConstRef(key), vids);
                                key = kv.key;
                                vids.clear();
                            }
                            vids.push_back(kv.vid);
                        }
                        if (!vids.empty()) {
                            index->_AppendNonUniqueVertexIndexEntry(txn.GetTxn(),
                                                                    GetKeyConstRef(key), vids);
                        }
                        break;
                    }
                case IndexType::PairUniqueIndex:
                    THROW_CODE(InputError, "vertex index do not support pair-unique attributes");
                }
            } else {
                // multiple blocks, use regular index calls
                for (auto& kv : key_vids) {
                    index->Add(txn.GetTxn(), GetKeyConstRef(kv.key), kv.vid);
                }
            }
        }
    } else {
        SchemaManager* schema_manager = &new_schema_info->e_schema_manager;
        auto e_schema = schema_manager->GetSchema(label_id);
        auto* field_extractor = e_schema->GetFieldExtractor(field_id);
        FMA_DBG_ASSERT(field_extractor);
        EdgeIndex* edge_index = field_extractor->GetEdgeIndex();
        FMA_DBG_ASSERT(edge_index);
        //
        // see issue #406 https://github.com/TuGraph-family/tugraph-db/issues/406
        // Reduce max_block_size from 1<<28 to 1<<24 to fix issues caused by overly large block
        // sizes.
        //
        static const size_t max_block_size = 1 << 24;
        for (VertexId vid = start_vid; vid < end_vid; vid += max_block_size) {
            std::vector<KeyEUid<T>> key_euids;
            VertexId curr_end = std::min<VertexId>(end_vid, vid + max_block_size);
            key_euids.reserve(curr_end - vid);
            for (auto it = txn.GetVertexIterator(vid, true); it.IsValid() && it.GetId() < curr_end;
                 it.Next()) {
                auto et = it.GetOutEdgeIterator();
                while (et.IsValid()) {
                    Value prop = et.GetProperty();
                    if (et.GetLabelId() != label_id) {
                        et.Next();
                        continue;
                    }
                    if (e_schema->DetachProperty()) {
                        prop = e_schema->GetDetachedEdgeProperty(txn.GetTxn(), et.GetUid());
                    }
                    if (!field_extractor->GetIsNull(prop)) {
                        const T& key = GetIndexKeyFromValue<T>(field_extractor->GetConstRef(prop));
                        key_euids.emplace_back(key, et.GetUid());
                    }
                    et.Next();
                }
            }
            LGRAPH_PSORT(key_euids.begin(), key_euids.end());
            // now insert into index table
            if (max_block_size >= (size_t)(end_vid - start_vid)) {
                // block size large enough, so there is only one pass, use AppendKv
                switch (type) {
                case IndexType::GlobalUniqueIndex:
                    {
                        // if there is only one block, we use AppendKv,
                        // so checking for duplicate is required
                        // if there are multiple blocks,
                        // then uniqueness will be checked when we insert the
                        // keys into index, and this is not required,
                        // but still good to find duplicates early
                        for (size_t i = 1; i < key_euids.size(); i++) {
                            if (key_euids[i].key == key_euids[i - 1].key)
                                THROW_CODE(InputError,
                                    "Duplicate edge index keys [{}] found for vid {} dst {} eid {},"
                                    "and {} {} {}.",
                                    key_euids[i].key, key_euids[i].euid.src, key_euids[i].euid.dst,
                                    key_euids[i].euid.eid, key_euids[i - 1].euid.src,
                                    key_euids[i - 1].euid.dst, key_euids[i - 1].euid.eid);
                        }
                        for (auto& kv : key_euids)
                            edge_index->_AppendIndexEntry(txn.GetTxn(), GetKeyConstRef(kv.key),
                                                          (EdgeUid)kv.euid);
                        break;
                    }
                case IndexType::PairUniqueIndex:
                    {
                        for (size_t i = 1; i < key_euids.size(); i++) {
                            if (key_euids[i].key == key_euids[i - 1].key &&
                                key_euids[i].euid.src == key_euids[i - 1].euid.src &&
                                key_euids[i].euid.dst == key_euids[i - 1].euid.dst)
                                THROW_CODE(InputError,
                                    "Duplicate edge index keys-vid [{}] found for vid {} "
                                    "dst{} eid {}, and {} {} {}.",
                                    key_euids[i].key, key_euids[i].euid.src, key_euids[i].euid.dst,
                                    key_euids[i].euid.eid, key_euids[i - 1].euid.src,
                                    key_euids[i - 1].euid.dst, key_euids[i - 1].euid.eid);
                        }
                        for (auto& kv : key_euids)
                            edge_index->_AppendIndexEntry(txn.GetTxn(), GetKeyConstRef(kv.key),
                                                          (EdgeUid)kv.euid);
                        break;
                    }
                case IndexType::NonuniqueIndex:
                    {
                        T key = key_euids.empty() ? T() : key_euids.front().key;
                        std::vector<EdgeUid> euids;
                        for (size_t i = 0; i < key_euids.size(); ++i) {
                            auto& kv = key_euids[i];
                            if (key != kv.key) {
                                // write out a bunch of vids
                                edge_index->_AppendNonUniqueIndexEntry(txn.GetTxn(),
                                                                       GetKeyConstRef(key), euids);
                                key = kv.key;
                                euids.clear();
                            }
                            euids.push_back(kv.euid);
                        }
                        if (!euids.empty()) {
                            edge_index->_AppendNonUniqueIndexEntry(txn.GetTxn(),
                                                                   GetKeyConstRef(key), euids);
                        }
                        break;
                    }
                }
            } else {
                // multiple blocks, use regular index calls
                for (auto& kv : key_euids) {
                    edge_index->Add(txn.GetTxn(), GetKeyConstRef(kv.key), kv.euid);
                }
            }
        }
    }
}

void LightningGraph::BatchBuildCompositeIndex(Transaction& txn, SchemaInfo* new_schema_info,
                                              LabelId label_id,
                                              const std::vector<std::string> &fields,
                                              CompositeIndexType type, VertexId start_vid,
                                              VertexId end_vid, bool is_vertex) {
    if (is_vertex) {
        SchemaManager* schema_manager = &new_schema_info->v_schema_manager;
        auto v_schema = schema_manager->GetSchema(label_id);
        CompositeIndex* index = v_schema->GetCompositeIndex(fields);
        FMA_DBG_ASSERT(index);
        static const size_t max_block_size = 1 << 28;
        for (VertexId vid = start_vid; vid < end_vid; vid += max_block_size) {
            std::vector<CompositeKeyVid> key_vids;
            VertexId curr_end = std::min<VertexId>(end_vid, vid + max_block_size);
            key_vids.reserve(curr_end - vid);
            for (auto it = txn.GetVertexIterator(vid, true); it.IsValid() && it.GetId() < curr_end;
                 it.Next()) {
                Value prop = it.GetProperty();
                if (lgraph::SchemaManager::GetRecordLabelId(prop) != label_id) continue;
                if (v_schema->DetachProperty()) {
                    prop = v_schema->GetDetachedVertexProperty(txn.GetTxn(), it.GetId());
                }
                bool can_index = true;
                for (const std::string& field : fields) {
                    const _detail::FieldExtractorBase* extractor =
                        v_schema->GetFieldExtractor(field);
                    if (extractor->GetIsNull(prop)) {
                        can_index = false;
                        break;
                    }
                }
                if (!can_index) {
                    continue;
                }
                std::vector<Value> values;
                std::vector<FieldType> types;
                for (auto& field : fields) {
                    values.emplace_back(v_schema->GetFieldExtractor(field)->GetConstRef(prop));
                    types.emplace_back(v_schema->GetFieldExtractor(field)->Type());
                }
                key_vids.emplace_back(values, types, it.GetId());
            }
            LGRAPH_PSORT(key_vids.begin(), key_vids.end());
            // now insert into index table
            if (max_block_size >= (size_t)(end_vid - start_vid)) {
                // block size large enough, so there is only one pass, use AppendKv
                switch (type) {
                case CompositeIndexType::UniqueIndex:
                    {
                        // if there is only one block, we use AppendKv,
                        // so checking for duplicate is required
                        // if there are multiple blocks,
                        // then uniqueness will be checked when we insert the
                        // keys into index, and this is not required,
                        // but still good to find duplicates early
                        for (size_t i = 1; i < key_vids.size(); i++) {
                            if (key_vids[i].keys == key_vids[i - 1].keys)
                                THROW_CODE(InputError,
                                           "Duplicate composite vertex keys [{}] found "
                                           "for vids {} and {}.",
                                           key_vids[i].keys[0].AsString(), key_vids[i - 1].vid,
                                           key_vids[i].vid);
                        }
                        for (auto& kv : key_vids)
                            index->_AppendCompositeIndexEntry(txn.GetTxn(),
                                   composite_index_helper::GenerateCompositeIndexKey(kv.keys),
                                   (VertexId)kv.vid);
                        break;
                    }
                case CompositeIndexType::NonUniqueIndex:
                    {
                        std::vector<Value> key;
                        if (!key_vids.empty())
                            key = key_vids.front().keys;
                        std::vector<VertexId> vids;
                        for (size_t i = 0; i < key_vids.size(); ++i) {
                            auto& kv = key_vids[i];
                            if (!(key == kv.keys)) {
                                // write out a bunch of vids
                                index->_AppendNonUniqueCompositeIndexEntry(txn.GetTxn(),
                                composite_index_helper::GenerateCompositeIndexKey(key), vids);
                                key = kv.keys;
                                vids.clear();
                            }
                            vids.push_back(kv.vid);
                        }
                        if (!vids.empty()) {
                            index->_AppendNonUniqueCompositeIndexEntry(txn.GetTxn(),
                                     composite_index_helper::GenerateCompositeIndexKey(key), vids);
                        }
                        break;
                    }
                }
            } else {
                // multiple blocks, use regular index calls
                for (auto& kv : key_vids) {
                    index->Add(txn.GetTxn(),
                               composite_index_helper::GenerateCompositeIndexKey(kv.keys), kv.vid);
                }
            }
        }
    }
}

std::vector<std::pair<int64_t, float>> LightningGraph::QueryVertexByFullTextIndex(
    const std::string& label, const std::string& query, int top_n) {
    ScopedRef<SchemaInfo> curr_schema_info = schema_.GetScopedRef();
    auto schema = curr_schema_info->v_schema_manager.GetSchema(label);
    if (!schema) {
        THROW_CODE(InputError, "Vertex Label [{}] does not exist.", label);
    }
    if (fulltext_index_) {
        return fulltext_index_->QueryVertex(schema->GetLabelId(), query, top_n);
    } else {
        return {};
    }
}

std::vector<std::pair<EdgeUid, float>> LightningGraph::QueryEdgeByFullTextIndex(
    const std::string& label, const std::string& query, int top_n) {
    ScopedRef<SchemaInfo> curr_schema_info = schema_.GetScopedRef();
    auto schema = curr_schema_info->e_schema_manager.GetSchema(label);
    if (!schema) {
        THROW_CODE(InputError, "Edge Label [{}] does not exist.", label);
    }
    if (fulltext_index_) {
        return fulltext_index_->QueryEdge(schema->GetLabelId(), query, top_n);
    } else {
        return {};
    }
}

void LightningGraph::FullTextIndexRefresh() {
    if (fulltext_index_) {
        fulltext_index_->Refresh();
    }
}

std::vector<std::tuple<bool, std::string, std::string>> LightningGraph::ListFullTextIndexes() {
    Transaction txn = CreateReadTxn();
    return txn.ListFullTextIndexes();
}

void LightningGraph::RebuildAllFullTextIndex() {
    if (!fulltext_index_) {
        return;
    }
    ScopedRef<SchemaInfo> curr_schema_info = schema_.GetScopedRef();
    const auto& all_vertex_labels = curr_schema_info->v_schema_manager.GetAllLabels();
    const auto& all_edge_labels = curr_schema_info->e_schema_manager.GetAllLabels();
    std::set<LabelId> v_lids, e_lids;
    for (const auto& label : all_vertex_labels) {
        auto schema = curr_schema_info->v_schema_manager.GetSchema(label);
        const auto& ft = schema->GetFullTextFields();
        if (!ft.empty()) {
            v_lids.emplace(schema->GetLabelId());
        }
    }
    for (const auto& label : all_edge_labels) {
        auto schema = curr_schema_info->e_schema_manager.GetSchema(label);
        const auto& ft = schema->GetFullTextFields();
        if (!ft.empty()) {
            e_lids.emplace(schema->GetLabelId());
        }
    }
    RebuildFullTextIndex(v_lids, e_lids);
}

void LightningGraph::RebuildFullTextIndex(const std::set<std::string>& v_labels,
                                          const std::set<std::string>& e_labels) {
    if (!fulltext_index_) {
        return;
    }
    LOG_INFO() <<
        FMA_FMT("start rebuilding fulltext index, v_labels:[{}], e_labels:[{}]",
                boost::algorithm::join(v_labels, ","),
                boost::algorithm::join(e_labels, ","));
    std::set<LabelId> v_lids, e_lids;
    ScopedRef<SchemaInfo> curr_schema_info = schema_.GetScopedRef();
    for (const auto& label : v_labels) {
        auto* schema = curr_schema_info->v_schema_manager.GetSchema(label);
        if (!schema) {
            THROW_CODE(InputError, "Vertex Label [{}] does not exist.", label);
        }
        if (schema->GetFullTextFields().empty()) {
            THROW_CODE(InputError, "Vertex Label [{}] has no fulltext index.", label);
        }
        v_lids.emplace(schema->GetLabelId());
    }
    for (const auto& label : e_labels) {
        auto* schema = curr_schema_info->e_schema_manager.GetSchema(label);
        if (!schema) {
            THROW_CODE(InputError, "Edge Label [{}] does not exist.", label);
        }
        if (schema->GetFullTextFields().empty()) {
            THROW_CODE(InputError, "Edge Label [{}] has no fulltext index.", label);
        }
        e_lids.emplace(schema->GetLabelId());
    }
    RebuildFullTextIndex(v_lids, e_lids);
    LOG_INFO() <<
        FMA_FMT("end rebuilding fulltext index, v_labels:[{}], e_labels:[{}]",
                boost::algorithm::join(v_labels, ","),
                boost::algorithm::join(e_labels, ","));
}

void LightningGraph::RebuildFullTextIndex(const std::set<LabelId>& v_lids,
                                          const std::set<LabelId>& e_lids) {
    if (!fulltext_index_) {
        return;
    }
    Transaction txn = CreateWriteTxn(false);
    ScopedRef<SchemaInfo> curr_schema_info = schema_.GetScopedRef();
    for (auto id : v_lids) {
        fulltext_index_->DeleteLabel(true, id);
    }
    for (auto id : e_lids) {
        fulltext_index_->DeleteLabel(false, id);
    }
    uint64_t count = 0;
    for (LabelId id : v_lids) {
        Schema* schema = curr_schema_info->v_schema_manager.GetSchema(id);
        const auto& fulltext_filelds = schema->GetFullTextFields();
        LabelId lid = schema->GetLabelId();
        for (auto iit = txn.GetVertexIndexIterator(schema->GetLabel(), schema->GetPrimaryField());
             iit.IsValid(); iit.Next()) {
            VertexId vid = iit.GetVid();
            Value properties;
            if (schema->DetachProperty()) {
                properties = schema->GetDetachedVertexProperty(txn.GetTxn(), vid);
            } else {
                properties = txn.GetVertexIterator(vid).GetProperty();
            }
            std::vector<std::pair<std::string, std::string>> kvs;
            for (auto& idx : fulltext_filelds) {
                auto fe = schema->GetFieldExtractor(idx);
                if (fe->GetIsNull(properties)) continue;
                kvs.emplace_back(fe->Name(), fe->FieldToString(properties));
            }
            fulltext_index_->AddVertex(vid, lid, kvs);
            if (++count % 100000 == 0) {
                LOG_DEBUG() << std::to_string(count) +
                                 " vertex FT index entries have been added" << count;
            }
        }
    }
    LOG_DEBUG() << std::to_string(count) + " vertex FT index entries have been added" << count;
    count = 0;
    if (!e_lids.empty()) {
        for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
            for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                LabelId lid = eit.GetLabelId();
                if (e_lids.find(lid) != e_lids.end()) {
                    std::vector<std::pair<std::string, std::string>> kvs;
                    const auto& euid = eit.GetUid();
                    Schema* schema = curr_schema_info->e_schema_manager.GetSchema(lid);
                    const auto& fulltext_filelds = schema->GetFullTextFields();
                    for (auto& idx : fulltext_filelds) {
                        auto fe = schema->GetFieldExtractor(idx);
                        Value properties;
                        if (schema->DetachProperty()) {
                            properties = schema->GetDetachedEdgeProperty(txn.GetTxn(), euid);
                        } else {
                            properties = eit.GetProperty();
                        }
                        if (fe->GetIsNull(properties)) continue;
                        kvs.emplace_back(fe->Name(), fe->FieldToString(properties));
                    }
                    fulltext_index_->AddEdge({euid.src, euid.dst, euid.lid, euid.tid, euid.eid},
                                             kvs);
                    if (++count % 100000 == 0) {
                        LOG_DEBUG() << std::to_string(count) +
                                         " edge FT index entries have been added" << count;
                    }
                }
            }
        }
    }
    LOG_DEBUG() << std::to_string(count) + " edge FT index entries have been added" << count;
    fulltext_index_->Commit();
}

bool LightningGraph::AddFullTextIndex(bool is_vertex, const std::string& label,
                                      const std::string& field) {
    if (!fulltext_index_) {
        THROW_CODE(InputError, "Fulltext index is not enabled");
    }
    _HoldWriteLock(meta_lock_);
    Transaction txn = CreateWriteTxn(false);
    std::unique_ptr<SchemaInfo> new_schema(new SchemaInfo(*schema_.GetScopedRef().Get()));
    Schema* schema = nullptr;
    if (is_vertex) {
        schema = new_schema->v_schema_manager.GetSchema(label);
    } else {
        schema = new_schema->e_schema_manager.GetSchema(label);
    }
    if (!schema) {
        THROW_CODE(InputError, "label \"{}\" does not exist.", label);
    }
    const _detail::FieldExtractorBase* extractor = schema->GetFieldExtractor(field);
    if (!extractor) {
        THROW_CODE(InputError, "field \"{}\":\"{}\" does not exist.", label, field);
    }
    if (extractor->FullTextIndexed()) {
        return false;
    }
    schema->MarkFullTextIndexed(extractor->GetFieldId(), true);
    index_manager_->AddFullTextIndex(txn.GetTxn(), is_vertex, label, field);
    txn.Commit();
    // install the new index
    schema_.Assign(new_schema.release());
    return true;
}


void LightningGraph::RefreshCount() {
    auto txn = CreateWriteTxn();
    auto num = txn.GetLooseNumVertex();
    const auto processor = std::thread::hardware_concurrency();
    auto batch = num / processor + 1;
    std::vector<std::thread> threads;
    std::vector<std::unordered_map<LabelId, int64_t>>
        count_vertex(processor), count_edge(processor);
    auto count = [this](VertexId startId, VertexId endId,
                        std::unordered_map<LabelId, int64_t>& vertex,
                        std::unordered_map<LabelId, int64_t>& edge) {
        auto txn = CreateReadTxn();
        for (auto vit = txn.GetVertexIterator(startId, true);
             vit.IsValid() && vit.GetId() < endId; vit.Next()) {
            auto vlid = txn.GetVertexLabelId(vit);
            vertex[vlid]++;
            for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                auto elid = eit.GetLabelId();
                edge[elid]++;
            }
        }
    };
    for (uint16_t i = 0; i < processor; i++) {
        threads.emplace_back(count, i*batch, (i+1)*batch,
                             std::ref(count_vertex[i]),
                             std::ref(count_edge[i]));
    }
    for (auto& t : threads) t.join();
    graph_->DeleteAllCount(txn.GetTxn());
    for (auto& item : count_vertex) {
        for (auto& pair : item) {
            graph_->IncreaseCount(txn.GetTxn(), true, pair.first, pair.second);
        }
    }
    for (auto& item : count_edge) {
        for (auto& pair : item) {
            graph_->IncreaseCount(txn.GetTxn(), false, pair.first, pair.second);
        }
    }
    txn.Commit();
}

bool LightningGraph::BlockingAddCompositeIndex(const std::string& label,
                                               const std::vector<std::string>& fields,
                                               CompositeIndexType type, bool is_vertex,
                                               bool known_vid_range, VertexId start_vid,
                                               VertexId end_vid) {
    _HoldWriteLock(meta_lock_);
    std::string field_names = boost::algorithm::join(fields, ",");
    if (fields.size() > _detail::MAX_COMPOSITE_FILED_SIZE || fields.size() < 2)
        THROW_CODE(InputError, "The number of fields({}) in the combined index "
                   "exceeds the maximum limit.", field_names);
    Transaction txn = CreateWriteTxn(false);
    std::unique_ptr<SchemaInfo> new_schema(new SchemaInfo(*schema_.GetScopedRef().Get()));
    Schema* schema = is_vertex ? new_schema->v_schema_manager.GetSchema(label)
                               : new_schema->e_schema_manager.GetSchema(label);
    if (!schema) {
        if (is_vertex)
            THROW_CODE(InputError, "Vertex label \"{}\" does not exist.", label);
        else
            THROW_CODE(InputError, "Edge label \"{}\" does not exist.", label);
    }
    std::vector<FieldType> field_types;
    for (const std::string &field : fields) {
        const _detail::FieldExtractorBase* extractor = schema->GetFieldExtractor(field);
        if (!extractor) {
            if (is_vertex)
                THROW_CODE(InputError, "Vertex field \"{}\":\"{}\" does not exist.", label, field);
            else
                THROW_CODE(InputError, "Edge field \"{}\":\"{}\" does not exist.", label, field);
        }
        /* if (extractor->IsOptional() && type == CompositeIndexType::UniqueIndex) {
            THROW_CODE(InputError, "Unique index cannot be added to an optional field [{}:{}]",
                       label, field);
        } */
        if (extractor->Type() == FieldType::BLOB) {
            THROW_CODE(InputError, "Field with type BLOB cannot be indexed");
        }
        field_types.emplace_back(extractor->Type());
    }
    if (schema->GetCompositeIndex(fields) != nullptr)
        return false;
    if (is_vertex) {
        std::shared_ptr<CompositeIndex> composite_index;
        bool success = index_manager_->AddVertexCompositeIndex(txn.GetTxn(), label, fields,
                                                               field_types, type, composite_index);
        if (!success)
            THROW_CODE(InputError, "build index {}-{} failed", label, field_names);

        composite_index->SetReady();
        schema->SetCompositeIndex(fields, composite_index.get());
        if (schema->DetachProperty()) {
            LOG_INFO() <<
                FMA_FMT("start building vertex index for {}:{} in detached model",
                        label, field_names);

            CompositeIndex* index = schema->GetCompositeIndex(fields);
            uint64_t count = 0;
            auto kv_iter = schema->GetPropertyTable().GetIterator(txn.GetTxn());
            for (kv_iter->GotoFirstKey(); kv_iter->IsValid(); kv_iter->Next()) {
                auto vid = graph::KeyPacker::GetVidFromPropertyTableKey(kv_iter->GetKey());
                auto prop = kv_iter->GetValue();
                std::vector<Value> values;
                std::vector<FieldType> types;
                for (auto &field : fields) {
                    values.emplace_back(schema->GetFieldExtractor(field)->GetConstRef(prop));
                    types.emplace_back(schema->GetFieldExtractor(field)->Type());
                }
                index->Add(txn.GetTxn(),
                           composite_index_helper::GenerateCompositeIndexKey(values), vid);
                count++;
                if (count % 100000 == 0) {
                    LOG_DEBUG() << "index count: " << count;
                }
            }
            kv_iter.reset();
            LOG_DEBUG() << "index count: " << count;
            txn.Commit();
            schema_.Assign(new_schema.release());
            LOG_INFO() <<
                FMA_FMT("end building vertex index for {}:{} in "
                    "detached model", label, field_names);
            return true;
        }

        // now build index
        if (!known_vid_range) {
            start_vid = 0;
            end_vid = txn.GetLooseNumVertex();
            // vid range not known, try getting from index
            VertexIndex* idx =
                schema->GetFieldExtractor(schema->GetPrimaryField())->GetVertexIndex();
            FMA_DBG_ASSERT(idx);
            VertexId beg = std::numeric_limits<VertexId>::max();
            VertexId end = 0;
            for (auto it = idx->GetUnmanagedIterator(txn.GetTxn(), Value(), Value());
                 it.IsValid(); it.Next()) {
                VertexId vid = it.GetVid();
                beg = std::min(beg, vid);
                end = std::max(end, vid);
            }
            if (beg != std::numeric_limits<VertexId>::max()) start_vid = beg;
            if (end != 0) end_vid = end + 1;
        }
    }
    LabelId lid = schema->GetLabelId();
    BatchBuildCompositeIndex(txn, new_schema.get(), lid,
                             fields, type, start_vid, end_vid, is_vertex);
    txn.Commit();
    // install the new index
    schema_.Assign(new_schema.release());
    return true;
}

bool LightningGraph::BlockingAddIndex(const std::string& label, const std::string& field,
                                      IndexType type, bool is_vertex, bool known_vid_range,
                                      VertexId start_vid, VertexId end_vid) {
    _HoldWriteLock(meta_lock_);
    Transaction txn = CreateWriteTxn(false);
    std::unique_ptr<SchemaInfo> new_schema(new SchemaInfo(*schema_.GetScopedRef().Get()));
    Schema* schema = is_vertex ? new_schema->v_schema_manager.GetSchema(label)
                               : new_schema->e_schema_manager.GetSchema(label);
    if (!schema) {
        if (is_vertex)
            THROW_CODE(InputError, "Vertex label \"{}\" does not exist.", label);
        else
            THROW_CODE(InputError, "Edge label \"{}\" does not exist.", label);
    }
    const _detail::FieldExtractorBase* extractor = schema->GetFieldExtractor(field);
    if (!extractor) {
        if (is_vertex)
            THROW_CODE(InputError, "Vertex field \"{}\":\"{}\" does not exist.", label, field);
        else
            THROW_CODE(InputError, "Edge field \"{}\":\"{}\" does not exist.", label, field);
    }
    if ((extractor->GetVertexIndex() && is_vertex) || (extractor->GetEdgeIndex() && !is_vertex))
        return false;  // index already exist

    /*if (extractor->IsOptional() && (type == IndexType::GlobalUniqueIndex ||
                                    type == IndexType::PairUniqueIndex)) {
        THROW_CODE(InputError, "Unique index cannot be added to an optional field [{}:{}]",
                   label, field);
    }*/

    if (extractor->Type() == FieldType::BLOB) {
        THROW_CODE(InputError, "Field with type BLOB cannot be indexed");
    }
    if (is_vertex) {
        std::unique_ptr<VertexIndex> vertex_index;
        bool success = index_manager_->AddVertexIndex(txn.GetTxn(), label, field,
                                   extractor->Type(), type, vertex_index);
        if (!success)
            THROW_CODE(InputError, "build index {}-{} failed", label, field);

        vertex_index->SetReady();
        schema->MarkVertexIndexed(extractor->GetFieldId(), vertex_index.release());
        if (schema->DetachProperty()) {
            LOG_INFO() <<
                FMA_FMT("start building vertex index for {}:{} in detached model", label, field);
            VertexIndex* index = extractor->GetVertexIndex();
            uint64_t count = 0;
            uint64_t filter = 0;
            auto kv_iter = schema->GetPropertyTable().GetIterator(txn.GetTxn());
            for (kv_iter->GotoFirstKey(); kv_iter->IsValid(); kv_iter->Next()) {
                auto vid = graph::KeyPacker::GetVidFromPropertyTableKey(kv_iter->GetKey());
                auto props = kv_iter->GetValue();
                auto prop = extractor->GetConstRef(props);
                if (prop.Empty()) {
                    filter++;
                    continue;
                }
                if (!index->Add(txn.GetTxn(), prop, vid)) {
                    THROW_CODE(InternalError,
                        "Failed to index vertex [{}] with field value [{}:{}]",
                        vid, extractor->Name(), extractor->FieldToString(props));
                }
                count++;
                if (count % 100000 == 0) {
                    LOG_INFO() << "index count: " << count;
                }
            }
            kv_iter.reset();
            LOG_INFO() << "index count: " << count;
            LOG_INFO() << FMA_FMT("{} records are skipped during adding index.", filter);
            txn.Commit();
            schema_.Assign(new_schema.release());
            LOG_INFO() <<
                FMA_FMT("end building vertex index for {}:{} in detached model", label, field);
            return true;
        }

        // now build index
        if (!known_vid_range) {
            start_vid = 0;
            end_vid = txn.GetLooseNumVertex();
            // vid range not known, try getting from index
            VertexIndex* idx =
                schema->GetFieldExtractor(schema->GetPrimaryField())->GetVertexIndex();
            FMA_DBG_ASSERT(idx);
            VertexId beg = std::numeric_limits<VertexId>::max();
            VertexId end = 0;
            for (auto it = idx->GetUnmanagedIterator(txn.GetTxn(), Value(), Value());
                 it.IsValid(); it.Next()) {
                VertexId vid = it.GetVid();
                beg = std::min(beg, vid);
                end = std::max(end, vid);
            }
            if (beg != std::numeric_limits<VertexId>::max()) start_vid = beg;
            if (end != 0) end_vid = end + 1;
        }
    } else {
        std::unique_ptr<EdgeIndex> edge_index;
        bool success = index_manager_->AddEdgeIndex(txn.GetTxn(), label, field,
                                       extractor->Type(), type, edge_index);
        if (!success)
            THROW_CODE(InputError, "build index {}-{} failed", label, field);

        edge_index->SetReady();
        schema->MarkEdgeIndexed(extractor->GetFieldId(), edge_index.release());
        if (schema->DetachProperty()) {
            LOG_INFO() <<
                FMA_FMT("start building edge index for {}:{} in detached model", label, field);
            uint64_t count = 0;
            EdgeIndex* index = extractor->GetEdgeIndex();
            auto kv_iter = schema->GetPropertyTable().GetIterator(txn.GetTxn());
            for (kv_iter->GotoFirstKey(); kv_iter->IsValid(); kv_iter->Next()) {
                auto euid = graph::KeyPacker::GetEuidFromPropertyTableKey(
                    kv_iter->GetKey(), schema->GetLabelId());
                auto prop = kv_iter->GetValue();
                if (extractor->GetIsNull(prop)) {
                    continue;
                }
                if (!index->Add(txn.GetTxn(), extractor->GetConstRef(prop),
                                {euid.src, euid.dst, euid.lid, euid.tid, euid.eid})) {
                    THROW_CODE(InternalError,
                        "Failed to index edge [{}] with field value [{}:{}]",
                        euid.ToString(), extractor->Name(), extractor->FieldToString(prop));
                }
                count++;
                if (count % 100000 == 0) {
                    LOG_DEBUG() << "index count: " << count;
                }
            }
            kv_iter.reset();
            LOG_DEBUG() << "index count: " << count;
            txn.Commit();
            schema_.Assign(new_schema.release());
            LOG_INFO() <<
                FMA_FMT("end building edge index for {}:{} in detached model", label, field);
            return true;
        }
        // now build index
        if (!known_vid_range) {
            start_vid = 0;
            end_vid = txn.GetLooseNumVertex();
            // vid range not known, try getting from index
            auto& indexed_fields = schema->GetIndexedFields();
            for (size_t pos : indexed_fields) {
                auto fe = schema->GetFieldExtractor(pos);
                if (!fe->IsOptional()) {
                    EdgeIndex* idx = fe->GetEdgeIndex();
                    FMA_DBG_ASSERT(idx);
                    VertexId beg = std::numeric_limits<VertexId>::max();
                    VertexId end = 0;
                    for (auto it = idx->GetUnmanagedIterator(txn.GetTxn(), Value(), Value());
                         it.IsValid(); it.Next()) {
                        VertexId vid = it.GetSrcVid();
                        beg = std::min(beg, vid);
                        end = std::max(end, vid);
                    }
                    if (beg != std::numeric_limits<VertexId>::max()) start_vid = beg;
                    if (end != 0) end_vid = end + 1;
                    break;
                }
            }
        }
    }
    // now we know the start and end vid of this label, start building
    // try building index
    LabelId lid = schema->GetLabelId();
    size_t fid = schema->GetFieldId(field);
    switch (extractor->Type()) {
    case FieldType::BOOL:
        BatchBuildIndex<int8_t>(txn, new_schema.get(), lid, fid, type, start_vid, end_vid,
                                is_vertex);
        break;
    case FieldType::INT8:
        BatchBuildIndex<int8_t>(txn, new_schema.get(), lid, fid, type, start_vid, end_vid,
                                is_vertex);
        break;
    case FieldType::INT16:
        BatchBuildIndex<int16_t>(txn, new_schema.get(), lid, fid, type, start_vid, end_vid,
                                 is_vertex);
        break;
    case FieldType::INT32:
        BatchBuildIndex<int32_t>(txn, new_schema.get(), lid, fid, type, start_vid, end_vid,
                                 is_vertex);
        break;
    case FieldType::INT64:
        BatchBuildIndex<int64_t>(txn, new_schema.get(), lid, fid, type, start_vid, end_vid,
                                 is_vertex);
        break;
    case FieldType::DATE:
        BatchBuildIndex<int32_t>(txn, new_schema.get(), lid, fid, type, start_vid, end_vid,
                                 is_vertex);
        break;
    case FieldType::DATETIME:
        BatchBuildIndex<int64_t>(txn, new_schema.get(), lid, fid, type, start_vid, end_vid,
                                 is_vertex);
        break;
    case FieldType::FLOAT:
        BatchBuildIndex<float>(txn, new_schema.get(), lid, fid, type, start_vid, end_vid,
                               is_vertex);
        break;
    case FieldType::DOUBLE:
        BatchBuildIndex<double>(txn, new_schema.get(), lid, fid, type, start_vid, end_vid,
                                is_vertex);
        break;
    case FieldType::STRING:
        BatchBuildIndex<ConstStringRef>(txn, new_schema.get(), lid, fid, type, start_vid,
                                        end_vid, is_vertex);
        break;
    case FieldType::BLOB:
        THROW_CODE(InputError, std::string("Field of type ") +
                         field_data_helper::FieldTypeName(extractor->Type()) +
                         " cannot be indexed.");
    default:
        throw std::runtime_error(std::string("Unhandled field type: ") +
                                 field_data_helper::FieldTypeName(extractor->Type()));
    }
    txn.Commit();
    // install the new index
    schema_.Assign(new_schema.release());
    return true;
}

bool LightningGraph::BlockingAddVectorIndex(bool is_vertex, const std::string& label,
                                            const std::string& field,
                                            const std::string& index_type, int vec_dimension,
                                            const std::string& distance_type,
                                            std::vector<int>& index_spec) {
    if (!is_vertex) {
        THROW_CODE(VectorIndexException, "Only vertex supports vector index");
    }
    _HoldWriteLock(meta_lock_);
    Transaction txn = CreateWriteTxn(false);
    std::unique_ptr<SchemaInfo> new_schema(new SchemaInfo(*schema_.GetScopedRef().Get()));
    Schema* schema = new_schema->v_schema_manager.GetSchema(label);
    if (!schema) {
        THROW_CODE(InputError, "Vertex label \"{}\" does not exist.", label);
    }
    const _detail::FieldExtractorBase* extractor = schema->GetFieldExtractor(field);
    if (!extractor) {
        THROW_CODE(InputError, "Vertex field \"{}\":\"{}\" does not exist.", label, field);
    }
    if (extractor->GetVertexIndex() || extractor->GetVectorIndex())
        return false;  // index already exist
    if (extractor->Type() != FieldType::FLOAT_VECTOR) {
        THROW_CODE(VectorIndexException, "Only FLOAT_VECTOR type supports vector index");
    }
    std::unique_ptr<VectorIndex> vector_index;
    bool success = index_manager_->AddVectorIndex(txn.GetTxn(), label, field, index_type,
                               vec_dimension, distance_type, index_spec, vector_index);
    if (!success)
        THROW_CODE(VectorIndexException, "failed to add vector index {}-{}", label, field);
    schema->MarkVectorIndexed(extractor->GetFieldId(), vector_index.release());
    if (!schema->DetachProperty()) {
        THROW_CODE(VectorIndexException, "vector index only support detached model");
    }
    LOG_INFO() << FMA_FMT("start building vertex vector index for {}:{} in detached model",
                          label, field);
    VectorIndex* index = extractor->GetVectorIndex();
    uint64_t count = 0;
    std::vector<std::vector<float>> floatvector;
    std::vector<lgraph::VertexId> vids;
    auto dim = index->GetVecDimension();
    auto kv_iter = schema->GetPropertyTable().GetIterator(txn.GetTxn());
    for (kv_iter->GotoFirstKey(); kv_iter->IsValid(); kv_iter->Next()) {
        auto prop = kv_iter->GetValue();
        if (extractor->GetIsNull(prop)) {
            continue;
        }
        auto vid = graph::KeyPacker::GetVidFromPropertyTableKey(kv_iter->GetKey());
        auto vector = (extractor->GetConstRef(prop)).AsType<std::vector<float>>();
        if (vector.size() != (size_t)dim) {
            THROW_CODE(VectorIndexException,
                       "vector size error, size:{}, dim:{}", vector.size(), dim);
        }
        if (index->GetIndexType() != "hnsw") {
            floatvector.emplace_back(std::move(vector));
            vids.emplace_back(vid);
        } else {
            index->Add({std::move(vector)}, {vid});
        }
        count++;
        if ((count % 10000) == 0) {
            LOG_INFO() << "vector index count: " << count;
        }
    }
    if (index->GetIndexType() != "hnsw") index->Add(floatvector, vids);
    LOG_INFO() << "vector index count: " << count;
    LOG_INFO() << FMA_FMT("end building vertex vector index for {}:{} in detached model",
                          label, field);
    kv_iter.reset();
    txn.Commit();
    schema_.Assign(new_schema.release());
    return true;
}

/**
 * reads a sequence of vertices and dump the index.
 *
 * \return Whether there are still vertex left.
 */
struct IndexSpecType {
    explicit IndexSpecType(const IndexSpec& s) : spec(s) {}
    IndexSpecType() {}

    IndexSpec spec;
    FieldType type = FieldType::STRING;
};

template <typename T>
void LightningGraph::_DumpIndex(const IndexSpec& spec, VertexId first_vertex,
                                size_t batch_commit_size, VertexId& next_vertex_id,
                                bool is_vertex) {
    LOG_DEBUG() << "Creating batch index for " << spec.label << ":" << spec.field;
    next_vertex_id = first_vertex;
    std::deque<KeyVid<T>> key_vids;
    std::deque<KeyEUid<T>> key_euids;
    if (!_AddEmptyIndex(spec.label, spec.field, spec.type, is_vertex) && is_vertex) {
        THROW_CODE(InputError, "Failed to create index {}:{}: index already exists",
                                 spec.label, spec.field);
    }
    if (is_vertex) {
        auto txn = CreateReadTxn();
        {
            LabelId lid = txn.GetLabelId(true, spec.label);
            LOG_DEBUG() << "Scanning vertexes for keys";
            auto vit = txn.GetVertexIterator(first_vertex, true);
            auto schema = txn.curr_schema_->v_schema_manager.GetSchema(lid);
            FMA_DBG_ASSERT(schema);
            auto extractor = schema->GetFieldExtractor(spec.field);
            FMA_DBG_ASSERT(extractor);
            for (; vit.IsValid(); vit.Next()) {
                Value v = vit.GetProperty();
                if (SchemaManager::GetRecordLabelId(v) != lid) {
                    next_vertex_id = vit.GetId();
                    break;
                }
                if (schema->DetachProperty()) {
                    v = schema->GetDetachedVertexProperty(txn.GetTxn(), vit.GetId());
                }
                key_vids.emplace_back(GetIndexKeyFromValue<T>(extractor->GetConstRef(v)),
                                      vit.GetId());
            }
            if (!vit.IsValid()) {
                txn.Abort();
                next_vertex_id = GetNumVertices();
            }
            txn.Abort();
        }
        LOG_DEBUG() << "Sorting by unique id";
        LGRAPH_PSORT(key_vids.begin(), key_vids.end());
        LOG_DEBUG() << "Dumping index";
        txn = CreateWriteTxn();
        auto index = txn.GetVertexIndex(spec.label, spec.field);
        switch (spec.type) {
        case IndexType::GlobalUniqueIndex:
            {
                for (size_t i = 1; i < key_vids.size(); i++) {
                    if (key_vids[i].key == key_vids[i - 1].key)
                        THROW_CODE(InputError,
                            "Duplicate vertex keys [{}] found for vids {} and {}.",
                            key_vids[i].key, key_vids[i - 1].vid, key_vids[i].vid);
                }

                for (size_t i = 0; i < key_vids.size(); i++) {
                    if (i != 0 && i % batch_commit_size == 0) {
                        txn.Commit();
                        LOG_DEBUG() << "committed " << i << " keys";
                        txn = CreateWriteTxn();
                    }
                    auto& kv = key_vids[i];
                    index->_AppendVertexIndexEntry(txn.GetTxn(), GetKeyConstRef(kv.key),
                                                   (VertexId)kv.vid);
                }
                break;
            }
        case IndexType::NonuniqueIndex:
            {
                T key = key_vids.empty() ? T() : key_vids.front().key;
                std::vector<VertexId> vids;
                for (size_t i = 0; i < key_vids.size(); ++i) {
                    if (i != 0 && i % batch_commit_size == 0) {
                        txn.Commit();
                        LOG_DEBUG() << "committed " << i << " keys";
                        txn = CreateWriteTxn();
                    }
                    auto& kv = key_vids[i];
                    if (key != kv.key) {
                        // write out a bunch of vids
                        index->_AppendNonUniqueVertexIndexEntry(txn.GetTxn(), GetKeyConstRef(key),
                                                                vids);
                        key = kv.key;
                        vids.clear();
                    }
                    vids.push_back(kv.vid);
                }
                if (!vids.empty()) {
                    index->_AppendNonUniqueVertexIndexEntry(txn.GetTxn(),
                                                            GetKeyConstRef(key), vids);
                }
                break;
            }
        case IndexType::PairUniqueIndex:
            THROW_CODE(InputError, "vertex index do not support pair-unique attributes");
        }
        txn.Commit();
        index->SetReady();
    } else {
        auto txn = CreateReadTxn();
        {
            LabelId lid = txn.GetLabelId(false, spec.label);
            LOG_DEBUG() << "Scanning edges for keys";
            auto vit = txn.GetVertexIterator(first_vertex, true);
            Value v = vit.GetProperty();
            auto start_lid = SchemaManager::GetRecordLabelId(v);
            auto schema = txn.curr_schema_->e_schema_manager.GetSchema(lid);
            FMA_DBG_ASSERT(schema);
            auto extractor = schema->GetFieldExtractor(spec.field);
            FMA_DBG_ASSERT(extractor);
            for (; vit.IsValid(); vit.Next()) {
                Value v = vit.GetProperty();
                auto v_lid = SchemaManager::GetRecordLabelId(v);
                auto v_schema = txn.curr_schema_->v_schema_manager.GetSchema(v_lid);
                if (v_schema->DetachProperty()) {
                    v = v_schema->GetDetachedVertexProperty(txn.GetTxn(), vit.GetId());
                }
                auto eit = vit.GetOutEdgeIterator();
                for (; eit.IsValid(); eit.Next()) {
                    if (eit.GetLabelId() == lid) {
                        EdgeUid euid = eit.GetUid();
                        Value e_property = eit.GetProperty();
                        if (schema->DetachProperty()) {
                            e_property = schema->GetDetachedEdgeProperty(txn.GetTxn(), euid);
                        }
                        key_euids.emplace_back(GetIndexKeyFromValue<T>(
                                        extractor->GetConstRef(e_property)), euid);
                    }
                }
                if (v_lid != start_lid) {
                    next_vertex_id = vit.GetId();
                    break;
                }
            }
            if (!vit.IsValid()) {
                txn.Abort();
                next_vertex_id = GetNumVertices();
            }
            txn.Abort();
        }
        LOG_DEBUG() << "Sorting by unique id";
        LGRAPH_PSORT(key_euids.begin(), key_euids.end());
        LOG_DEBUG() << "Dumping index";
        txn = CreateWriteTxn();
        auto index = txn.GetEdgeIndex(spec.label, spec.field);
        switch (spec.type) {
        case IndexType::GlobalUniqueIndex:
            {
                for (size_t i = 1; i < key_euids.size(); i++) {
                    if (key_euids[i].key == key_euids[i - 1].key)
                        THROW_CODE(InputError,
                            "Duplicate edge index keys [{}] found for vid {} dst{} eid {},"
                            "and {} {} {}.",
                            key_euids[i].key, key_euids[i].euid.src, key_euids[i].euid.dst,
                            key_euids[i].euid.eid, key_euids[i - 1].euid.src,
                            key_euids[i - 1].euid.dst, key_euids[i - 1].euid.eid);
                }
                LOG_DEBUG() << "add unique index";
                for (size_t i = 0; i < key_euids.size(); i++) {
                    if (i != 0 && i % batch_commit_size == 0) {
                        txn.Commit();
                        LOG_DEBUG() << "committed " << i << " keys";
                        txn = CreateWriteTxn();
                    }
                    auto& kv = key_euids[i];
                    index->_AppendIndexEntry(txn.GetTxn(), GetKeyConstRef(kv.key),
                                             (EdgeUid)kv.euid);
                }
                break;
            }
        case IndexType::PairUniqueIndex:
            {
                for (size_t i = 1; i < key_euids.size(); i++) {
                    if (key_euids[i].key == key_euids[i - 1].key &&
                        key_euids[i].euid.src == key_euids[i - 1].euid.src &&
                        key_euids[i].euid.dst == key_euids[i - 1].euid.dst)
                        THROW_CODE(InputError,
                            "Duplicate edge index keys-vid [{}] found for vid {} "
                            "dst{} eid {}, and {} {} {}.",
                            key_euids[i].key, key_euids[i].euid.src, key_euids[i].euid.dst,
                            key_euids[i].euid.eid, key_euids[i - 1].euid.src,
                            key_euids[i - 1].euid.dst, key_euids[i - 1].euid.eid);
                }
                LOG_DEBUG() << "add pair_unique index";
                for (size_t i = 0; i < key_euids.size(); i++) {
                    if (i != 0 && i % batch_commit_size == 0) {
                        txn.Commit();
                        LOG_DEBUG() << "committed " << i << " keys";
                        txn = CreateWriteTxn();
                    }
                    auto& kv = key_euids[i];
                    index->_AppendIndexEntry(txn.GetTxn(), GetKeyConstRef(kv.key),
                                             (EdgeUid)kv.euid);
                }
                break;
            }
        case IndexType::NonuniqueIndex:
            {
                T key = key_euids.empty() ? T() : key_euids.front().key;
                std::vector<EdgeUid> euids;
                for (size_t i = 0; i < key_euids.size(); ++i) {
                    if (i != 0 && i % batch_commit_size == 0) {
                        txn.Commit();
                        LOG_DEBUG() << "committed " << i << " keys";
                        txn = CreateWriteTxn();
                    }
                    auto& kv = key_euids[i];
                    if (key != kv.key) {
                        // write out a bunch of vids
                        index->_AppendNonUniqueIndexEntry(txn.GetTxn(), GetKeyConstRef(key), euids);
                        key = kv.key;
                        euids.clear();
                    }
                    euids.push_back(kv.euid);
                }
                if (!key_euids.empty()) {
                    index->_AppendNonUniqueIndexEntry(txn.GetTxn(), GetKeyConstRef(key), euids);
                }
                break;
            }
        }
        txn.Commit();
        index->SetReady();
    }
}

void LightningGraph::OfflineCreateBatchIndex(const std::vector<IndexSpec>& indexes,
                                             size_t commit_batch_size, bool is_vertex) {
    _HoldWriteLock(meta_lock_);
    std::map<std::string, std::vector<IndexSpecType>> label_indexes;
    std::vector<IndexSpecType> edge_label_indexes;
    for (auto& idx : indexes) {
        if (!is_vertex) {
            edge_label_indexes.emplace_back(idx);
        }
        label_indexes[idx.label].emplace_back(idx);
    }
    Transaction txn = CreateReadTxn();
    for (auto& kv : label_indexes) {
        // check for duplicate field
        auto& v = kv.second;
        std::sort(v.begin(), v.end(), [](const IndexSpecType& l, const IndexSpecType& r) {
            return l.spec.field < r.spec.field;
        });
        for (size_t i = 1; i < v.size(); i++) {
            if (v[i].spec.field == v[i - 1].spec.field) {
                THROW_CODE(InputError, "Duplicate index specified for {}:{}",
                                         kv.first, v[i].spec.field);
            }
        }
        // get field types
        std::vector<FieldSpec> fields = txn.GetSchema(is_vertex, kv.first);
        std::map<std::string, FieldType> fts;
        for (auto& fd : fields) fts[fd.name] = fd.type;
        // check for field types
        for (auto& st : v) {
            auto it = fts.find(st.spec.field);
            if (it == fts.end()) {
                THROW_CODE(InputError, "Field {} does not exist for label {}",
                                         st.spec.field, st.spec.label);
            }
            st.type = it->second;
        }
    }
    for (auto& label : edge_label_indexes) {
        std::vector<FieldSpec> fields = txn.GetSchema(is_vertex, label.spec.label);
        std::map<std::string, FieldType> fts;
        for (auto& fd : fields) fts[fd.name] = fd.type;
        auto it = fts.find(label.spec.field);
        if (it == fts.end()) {
            THROW_CODE(InputError, "Field {} does not exist for label {}",
                                     label.spec.field, label.spec.label);
        }
        label.type = it->second;
    }
    std::map<LabelId, bool> label_id_done;
    std::map<LabelId, std::string> label_id_name;
    for (auto& kv : label_indexes) {
        LabelId lid = txn.GetLabelId(is_vertex, kv.first);
        label_id_done[lid] = false;
        label_id_name[lid] = kv.first;
    }
    txn.Abort();

    // now start building the indexes
    VertexId start_vid = 0;
    while (true) {
        Transaction txn = CreateReadTxn();
        auto vit = txn.GetVertexIterator(start_vid, true);
        if (!vit.IsValid()) break;
        LabelId curr_lid = SchemaManager::GetRecordLabelId(vit.GetProperty());
        start_vid = vit.GetId();
        if (!is_vertex || label_id_done.find(curr_lid) != label_id_done.end()) {
            if (is_vertex && label_id_done[curr_lid]) {
                THROW_CODE(InternalError, "Vertex Ids are not totally ordered: "
                    "found vertex vid={} with label {} after scanning the last range. "
                    "Please delete the indexes of this label and retry.",
                    vit.GetId(), txn.GetVertexLabel(vit));
            }
            // need to build index for this label
            std::string label = txn.GetVertexLabel(vit);
            txn.Abort();
            VertexId next_vid = 0;
            auto& indexs = is_vertex ? label_indexes[label] : edge_label_indexes;
            for (auto& idx : indexs) {
                switch (idx.type) {
                case FieldType::BOOL:
                    _DumpIndex<int8_t>(idx.spec, start_vid, commit_batch_size, next_vid, is_vertex);
                    break;
                case FieldType::INT8:
                    _DumpIndex<int8_t>(idx.spec, start_vid, commit_batch_size, next_vid, is_vertex);
                    break;
                case FieldType::INT16:
                    _DumpIndex<int16_t>(idx.spec, start_vid, commit_batch_size, next_vid,
                                        is_vertex);
                    break;
                case FieldType::INT32:
                    _DumpIndex<int32_t>(idx.spec, start_vid, commit_batch_size, next_vid,
                                        is_vertex);
                    break;
                case FieldType::INT64:
                    _DumpIndex<int64_t>(idx.spec, start_vid, commit_batch_size, next_vid,
                                        is_vertex);
                    break;
                case FieldType::DATE:
                    _DumpIndex<int32_t>(idx.spec, start_vid, commit_batch_size, next_vid,
                                        is_vertex);
                    break;
                case FieldType::DATETIME:
                    _DumpIndex<int64_t>(idx.spec, start_vid, commit_batch_size, next_vid,
                                        is_vertex);
                    break;
                case FieldType::FLOAT:
                    _DumpIndex<float>(idx.spec, start_vid, commit_batch_size, next_vid, is_vertex);
                    break;
                case FieldType::DOUBLE:
                    _DumpIndex<double>(idx.spec, start_vid, commit_batch_size, next_vid, is_vertex);
                    break;
                case FieldType::STRING:
                    _DumpIndex<ConstStringRef>(idx.spec, start_vid, commit_batch_size, next_vid,
                                               is_vertex);
                    break;
                case FieldType::BLOB:
                    THROW_CODE(InputError, std::string("Field of type ") +
                                     field_data_helper::FieldTypeName(idx.type) +
                                     " cannot be indexed.");
                default:
                    break;
                }
            }
            start_vid = next_vid;
            label_id_done[curr_lid] = true;
        } else {
            // no need to build index, skip
            while (vit.IsValid() && SchemaManager::GetRecordLabelId(vit.GetProperty()) == curr_lid)
                vit.Next();
            if (vit.IsValid()) {
                start_vid = vit.GetId();
            } else {
                break;
            }
        }
    }
    if (is_vertex) {
        // check if there is still label left
        for (auto& kv : label_id_done) {
            if (!kv.second) {
                const std::string& label = label_id_name[kv.first];
                auto& specs = label_indexes[label];
                LOG_WARN() << "Label " << label << " specified, but no vertex of that type exists.";
                for (auto& spec : specs) {
                    _AddEmptyIndex(spec.spec.label, spec.spec.field, spec.spec.type, is_vertex);
                }
            }
        }
    }
}

bool LightningGraph::IsIndexed(const std::string& label, const std::string& field, bool is_vertex) {
    Transaction txn = CreateReadTxn();
    auto curr_schema = schema_.GetScopedRef();
    const Schema* s = is_vertex ? curr_schema->v_schema_manager.GetSchema(label)
                                : curr_schema->e_schema_manager.GetSchema(label);
    if (!s) throw LabelNotExistException(label);
    auto fe = s->GetFieldExtractor(field);

    if (is_vertex) {
        VertexIndex* idx = fe->GetVertexIndex();
        return idx && idx->IsReady();
    } else {
        EdgeIndex* idx = fe->GetEdgeIndex();
        return idx && idx->IsReady();
    }
}

bool LightningGraph::IsCompositeIndexed(const std::string& label,
                                        const std::vector<std::string>& fields) {
    Transaction txn = CreateReadTxn();
    auto curr_schema = schema_.GetScopedRef();
    Schema* s = curr_schema->v_schema_manager.GetSchema(label);
    auto index = s->GetCompositeIndex(fields);
    return index && index->IsReady();
}


bool LightningGraph::DeleteFullTextIndex(bool is_vertex, const std::string& label,
                                         const std::string& field) {
    _HoldWriteLock(meta_lock_);
    Transaction txn = CreateWriteTxn(false);
    std::unique_ptr<SchemaInfo> new_schema(new SchemaInfo(*schema_.GetScopedRef().Get()));
    Schema* schema = nullptr;
    if (is_vertex) {
        schema = new_schema->v_schema_manager.GetSchema(label);
    } else {
        schema = new_schema->e_schema_manager.GetSchema(label);
    }
    if (!schema) {
        THROW_CODE(InputError, "label \"{}\" does not exist.", label);
    }
    const _detail::FieldExtractorBase* extractor = schema->GetFieldExtractor(field);
    if (!extractor) {
        THROW_CODE(InputError, "field \"{}\":\"{}\" does not exist.", label, field);
    }
    if (!extractor->FullTextIndexed()) {
        return false;
    }
    schema->MarkFullTextIndexed(extractor->GetFieldId(), false);
    index_manager_->DeleteFullTextIndex(txn.GetTxn(), is_vertex, label, field);
    txn.Commit();
    // install the new index
    schema_.Assign(new_schema.release());
    return true;
}

bool LightningGraph::DeleteIndex(const std::string& label, const std::string& field,
                                 bool is_vertex) {
    _HoldWriteLock(meta_lock_);
    Transaction txn = CreateWriteTxn(false);
    ScopedRef<SchemaInfo> curr_schema = schema_.GetScopedRef();
    Schema* schema = is_vertex ? curr_schema->v_schema_manager.GetSchema(label)
                               : curr_schema->e_schema_manager.GetSchema(label);
    std::unique_ptr<SchemaInfo> old_schema_backup(new SchemaInfo(*curr_schema.Get()));
    if (!schema) throw LabelNotExistException(label);
    if (field == schema->GetPrimaryField()) {
        throw PrimaryIndexCannotBeDeletedException(field);
    }
    const _detail::FieldExtractorBase* extractor = schema->GetFieldExtractor(field);
    bool index_exist =
        (is_vertex && extractor->GetVertexIndex()) || (!is_vertex && extractor->GetEdgeIndex());
    if (!index_exist) return false;
    std::unique_ptr<SchemaInfo> new_schema(new SchemaInfo(*curr_schema.Get()));
    schema = is_vertex ? new_schema->v_schema_manager.GetSchema(label)
                       : new_schema->e_schema_manager.GetSchema(label);
    bool deleted = true;
    if (is_vertex) {
        schema->UnVertexIndex(extractor->GetFieldId());
        deleted = index_manager_->DeleteVertexIndex(txn.GetTxn(), label, field);
    } else {
        schema->UnEdgeIndex(extractor->GetFieldId());
        deleted = index_manager_->DeleteEdgeIndex(txn.GetTxn(), label, field);
    }
    if (deleted) {
        // install the new schema
        schema_.Assign(new_schema.release());
        AutoCleanupAction revert_assign_new_schema(
            [&]() { schema_.Assign(old_schema_backup.release()); });
        txn.Commit();
        // if success, cancel revert
        revert_assign_new_schema.Cancel();
        return true;
    }
    return false;
}

bool LightningGraph::DeleteCompositeIndex(const std::string& label,
                                          const std::vector<std::string>& fields,
                                          bool is_vertex) {
    _HoldWriteLock(meta_lock_);
    Transaction txn = CreateWriteTxn(false);
    ScopedRef<SchemaInfo> curr_schema = schema_.GetScopedRef();
    Schema* schema = is_vertex ? curr_schema->v_schema_manager.GetSchema(label)
                               : curr_schema->e_schema_manager.GetSchema(label);
    std::unique_ptr<SchemaInfo> old_schema_backup(new SchemaInfo(*curr_schema.Get()));
    if (!schema) throw LabelNotExistException(label);
    if (is_vertex) {
        if (!schema->GetCompositeIndex(fields)) return false;
        std::unique_ptr<SchemaInfo> new_schema(new SchemaInfo(*curr_schema.Get()));
        schema = new_schema->v_schema_manager.GetSchema(label);
        bool deleted = true;
        schema->UnVertexCompositeIndex(fields);
        deleted = index_manager_->DeleteVertexCompositeIndex(txn.GetTxn(), label, fields);
        if (deleted) {
            // install the new schema
            schema_.Assign(new_schema.release());
            AutoCleanupAction revert_assign_new_schema(
                [&]() { schema_.Assign(old_schema_backup.release()); });
            txn.Commit();
            // if success, cancel revert
            revert_assign_new_schema.Cancel();
            return true;
        }
    }
    return false;
}

bool LightningGraph::DeleteVectorIndex(
    bool is_vertex, const std::string& label, const std::string& field) {
    if (!is_vertex) {
        THROW_CODE(VectorIndexException, "Only vertex supports vector index");
    }
    _HoldWriteLock(meta_lock_);
    Transaction txn = CreateWriteTxn(false);
    ScopedRef<SchemaInfo> curr_schema = schema_.GetScopedRef();
    Schema* schema = curr_schema->v_schema_manager.GetSchema(label);
    if (!schema) throw LabelNotExistException(label);
    if (field == schema->GetPrimaryField()) {
        throw PrimaryIndexCannotBeDeletedException(field);
    }
    std::unique_ptr<SchemaInfo> old_schema_backup(new SchemaInfo(*curr_schema.Get()));
    const _detail::FieldExtractorBase* extractor = schema->GetFieldExtractor(field);
    if (!extractor->GetVectorIndex()) {
        return false;
    }
    std::unique_ptr<SchemaInfo> new_schema(new SchemaInfo(*curr_schema.Get()));
    schema = new_schema->v_schema_manager.GetSchema(label);
    auto deleted = index_manager_->DeleteVectorIndex(txn.GetTxn(), label, field);
    if (deleted) {
        schema->UnVectorIndex(extractor->GetFieldId());
        // install the new schema
        schema_.Assign(new_schema.release());
        AutoCleanupAction revert_assign_new_schema(
            [&]() { schema_.Assign(old_schema_backup.release()); });
        txn.Commit();
        // if success, cancel revert
        revert_assign_new_schema.Cancel();
        return true;
    }
    return false;
}

void LightningGraph::DropAllIndex() {
    try {
        _HoldWriteLock(meta_lock_);
        Transaction txn = CreateWriteTxn(false);
        ScopedRef<SchemaInfo> curr_schema = schema_.GetScopedRef();
        std::unique_ptr<SchemaInfo> new_schema(new SchemaInfo(*curr_schema.Get()));
        std::unique_ptr<SchemaInfo> backup_schema(new SchemaInfo(*curr_schema.Get()));
        auto [indexes, composite_indexes, vector_indexes]
            = index_manager_->ListAllIndexes(txn.GetTxn());

        bool success = true;
        for (auto& idx : indexes) {
            auto v_schema = new_schema->v_schema_manager.GetSchema(idx.label);
            auto e_schema = new_schema->e_schema_manager.GetSchema(idx.label);
            if (v_schema) {
                if (!index_manager_->DeleteVertexIndex(txn.GetTxn(), idx.label, idx.field)) {
                    success = false;
                    break;
                }
                auto ext = v_schema->GetFieldExtractor(idx.field);
                v_schema->UnVertexIndex(ext->GetFieldId());
            }
            if (e_schema) {
                if (!index_manager_->DeleteEdgeIndex(txn.GetTxn(), idx.label, idx.field)) {
                    success = false;
                    break;
                }
                auto ext = e_schema->GetFieldExtractor(idx.field);
                e_schema->UnEdgeIndex(ext->GetFieldId());
            }
        }
        for (auto& idx : composite_indexes) {
            auto v_schema = new_schema->v_schema_manager.GetSchema(idx.label);
            if (!index_manager_->DeleteVertexCompositeIndex(txn.GetTxn(), idx.label, idx.fields)) {
                success = false;
                break;
            }
            v_schema->UnVertexCompositeIndex(idx.fields);
        }
        for (auto& idx : vector_indexes) {
            auto v_schema = new_schema->v_schema_manager.GetSchema(idx.label);
            FMA_DBG_ASSERT(v_schema);
            auto ret = index_manager_->DeleteVectorIndex(txn.GetTxn(), idx.label, idx.field);
            FMA_DBG_ASSERT(ret);
            auto ext = v_schema->GetFieldExtractor(idx.field);
            v_schema->UnVectorIndex(ext->GetFieldId());
        }
        if (success) {
            schema_.Assign(new_schema.release());
            AutoCleanupAction revert_assign_new_schema(
                [&]() { schema_.Assign(backup_schema.release()); });
            txn.Commit();
            // install the new schema
            revert_assign_new_schema.Cancel();
        } else {
            txn.Abort();
        }
    } catch (std::exception& e) {
        LOG_WARN() << "Failed to drop all indexes: " << e.what();
    }
}

std::vector<VectorIndexSpec> LightningGraph::ListVectorIndex(KvTransaction& txn) {
    return index_manager_->ListVectorIndex(txn);
}

KvStore& LightningGraph::GetStore() { return *store_; }

const DBConfig& LightningGraph::GetConfig() const { return config_; }

/**
 * Backups the current DB to the path specified.
 *
 * \param path  Full pathname of the destination.
 * \param compact  True to enable compaction
 *
 * \return  Transaction ID of the last committed transaction.
 */
size_t LightningGraph::Backup(const std::string& path, bool compact) {
    auto ret = store_->Backup(path, compact);
    if (fulltext_index_) {
        fulltext_index_->Commit();
        fulltext_index_->Backup(path + "/" + _detail::FULLTEXT_INDEX_DIR);
    }
    return ret;
}

/**
 * Take a snapshot of the whole db using the read transaction txn.
 *
 * \param [in,out] txn  The transaction.
 * \param          path Full pathname of the snapshot file.
 */
void LightningGraph::Snapshot(Transaction& txn, const std::string& path) {
    // create parent dir if not exist
    auto& fs = fma_common::FileSystem::GetFileSystem(path);
    if (!fs.IsDir(path)) {
        if (!fs.Mkdir(path)) THROW_CODE(InternalError,
                                        "Failed to create dir " + path + " for snapshot.");
    } else {
        fs.Remove(path + fma_common::LocalFileSystem::PATH_SEPERATOR() + "data.mdb");
        fs.Remove(path + fma_common::LocalFileSystem::PATH_SEPERATOR() + "lock.mdb");
    }
    store_->Snapshot(txn.GetTxn(), path);
}

void LightningGraph::LoadSnapshot(const std::string& path) {
    _HoldWriteLock(meta_lock_);
    // DB must have already been locked, this is the only thread accessing this db
    plugin_manager_.reset();
    index_manager_.reset();
    graph_.reset();
    store_->LoadSnapshot(path);
    Open();
}

/** Warmups this DB */

void LightningGraph::WarmUp() const {
    store_->WarmUp(nullptr);
}

PluginManager* LightningGraph::GetPluginManager() const { return plugin_manager_.get(); }

void LightningGraph::ReloadFromDisk(const DBConfig& config) {
    _HoldWriteLock(meta_lock_);
    config_ = config;
    Close();
    Open();
}

size_t LightningGraph::GetCurrentVersion() {
    auto txn = CreateWriteTxn();
    return txn.GetTxnId();
}

#if USELESS_CODE
ScopedRef<SchemaInfo> LightningGraph::GetSchemaInfo() { return schema_.GetScopedRef(); }
#endif

void LightningGraph::Open() {
    Close();
    store_.reset(new LMDBKvStore(
        config_.dir, config_.db_size, config_.durable, config_.create_if_not_exist));
    auto txn = store_->CreateWriteTxn();
    // load meta info
    meta_table_ =
        store_->OpenTable(*txn, _detail::META_TABLE, true, ComparatorDesc::DefaultComparator());
    // load schema info
    auto v_s_tbl = SchemaManager::OpenTable(*txn, *store_, _detail::V_SCHEMA_TABLE);
    SchemaManager vs(*txn, std::move(v_s_tbl), true);
    for (auto& label : vs.GetAllLabels()) {
        auto s = vs.GetSchema(label);
        FMA_ASSERT(s);
        if (s->DetachProperty()) {
            std::string prefix = _detail::VERTEX_PROPERTY_TABLE_PREFIX;
            auto t = store_->OpenTable(*txn, prefix + label,
                                       true, ComparatorDesc::DefaultComparator());
            s->SetPropertyTable(std::move(t));
        }
    }
    auto e_s_tbl = SchemaManager::OpenTable(*txn, *store_, _detail::E_SCHEMA_TABLE);
    SchemaManager es(*txn, std::move(e_s_tbl), false);
    for (auto& label : es.GetAllLabels()) {
        auto s = es.GetSchema(label);
        FMA_ASSERT(s);
        if (s->DetachProperty()) {
            std::string prefix = _detail::EDGE_PROPERTY_TABLE_PREFIX;
            auto t = store_->OpenTable(*txn, prefix + label,
                                       true, ComparatorDesc::DefaultComparator());
            s->SetPropertyTable(std::move(t));
        }
    }
    es.RefreshEdgeConstraintsLids(vs);
    schema_.Assign(new SchemaInfo{std::move(vs), std::move(es)});
    // open graph
    auto g_tbl = graph::Graph::OpenTable(*txn, *store_, _detail::GRAPH_TABLE);
    graph_.reset(new graph::Graph(*txn, std::move(g_tbl), meta_table_));
    // load index
    auto i_tbl = IndexManager::OpenIndexListTable(*txn, *store_, _detail::INDEX_TABLE);
    index_manager_.reset(new IndexManager(
        *txn, &schema_.GetScopedRef()->v_schema_manager,
        &schema_.GetScopedRef()->e_schema_manager, std::move(i_tbl), this));
    // blob manager
    auto b_tbl = BlobManager::OpenTable(*txn, *store_, _detail::BLOB_TABLE);
    blob_manager_.reset(new BlobManager(*txn, std::move(b_tbl)));
    txn->Commit();
    if (config_.load_plugins) {
        plugin_manager_.reset(new PluginManager(
            this, config_.name, config_.dir + "/" + _detail::CPP_PLUGIN_DIR,
            _detail::CPP_PLUGIN_TABLE, config_.dir + "/" + _detail::PYTHON_PLUGIN_DIR,
            _detail::PYTHON_PLUGIN_TABLE, static_cast<int>(config_.subprocess_max_idle_seconds)));
    }
    if (config_.ft_index_options.enable_fulltext_index) {
        fulltext_index_.reset(
            new FullTextIndex(config_.dir + "/" + _detail::FULLTEXT_INDEX_DIR,
                              config_.ft_index_options.fulltext_analyzer,
                              config_.ft_index_options.fulltext_commit_interval,
                              config_.ft_index_options.fulltext_refresh_interval));
    } else {
        auto ft_indexes = CreateReadTxn().ListFullTextIndexes();
        if (!ft_indexes.empty()) {
            LOG_WARN() << "The following fulltext indexes will not work because the configuration "
                          "item `enable_fulltext_index` is not true)";
            for (auto ft_index : ft_indexes) {
                LOG_WARN() << FMA_FMT("{} label: {} field: {}",
                                      std::get<0>(ft_index) ? "Vertex" : "Edge",
                                      std::get<1>(ft_index), std::get<2>(ft_index));
            }
        }
    }
}

/**
 * Creates a read transaction
 *
 * \return  The new read transaction.
 */
Transaction LightningGraph::CreateReadTxn() {
    return Transaction(true,   // read_only
                       false,  // optimistic
                       this);  // db
}

Transaction LightningGraph::ForkTxn(Transaction& txn) {
    if (!txn.read_only_) THROW_CODE(InvalidFork);
    return Transaction(this, txn.GetTxn());
}

bool LightningGraph::CheckDbSecret(const std::string& expected) {
    auto txn = store_->CreateWriteTxn(false);
    Value key = Value::ConstRef(lgraph::_detail::DB_SECRET_KEY);
    Value v = meta_table_->GetValue(*txn, key);
    if (v.Empty()) {
        // no such value, this is a newly created DB
        meta_table_->SetValue(*txn, key, Value::ConstRef(expected));
        txn->Commit();
        db_secret = expected;
        return true;
    } else {
        return v.AsString() == expected;
    }
}

void LightningGraph::FlushDbSecret(const std::string& secret) {
    auto txn = store_->CreateWriteTxn(false);
    Value key = Value::ConstRef(lgraph::_detail::DB_SECRET_KEY);
    meta_table_->SetValue(*txn, key, Value::ConstRef(secret));
    txn->Commit();
    db_secret = secret;
}

std::string LightningGraph::GetSecret() {
    return db_secret;
}
}  // namespace lgraph
