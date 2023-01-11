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
        auto indexes = index_manager_->ListAllIndexes(txn.GetTxn());
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
        // clear graph data
        graph_->Drop(txn.GetTxn());
        txn.Commit();
    } catch (std::exception& e) {
        FMA_WARN_STREAM(logger_) << "Failed to drop all indexes: " << e.what();
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

template <bool IS_LABEL>
inline void CheckIsValidLabelFieldName(const std::string& lof) {
    if (lof.empty() || lof.size() > 64) {
        throw InputError(std::string((IS_LABEL ? "Label" : "Field name")) +
                         " is invalid: must be between 1 and 64 bytes.");
    }
    if (fma_common::TextParserUtils::IsDigits(lof.front())) {
        throw InputError(std::string((IS_LABEL ? "Label" : "Field name")) +
                         " cannot begin with a digit.");
    }
    for (char c : lof) {
        if ((uint8_t)c < 128 && !fma_common::TextParserUtils::IsValidNameCharacter(c))
            throw InputError(std::string((IS_LABEL ? "Label" : "Field name")) +
                             " can only contain alphabetic and numeric characters and underscore.");
    }
}

/**
 * Adds a label
 *
 * \param   label       The label.
 * \param   n_fields    Number of fields for this label.
 * \param   fds         The FieldDefs.
 * \param   is_vertex   True if this is vertex label, otherwise
 *          it is edge label.
 * \param   primary_field The vertex primary property, must be
 *          set when is_vertex is true
 * \param   edge_constraints The edge constraints, can be set
 *          when is_vertex is false
 *
 * \return  True if it succeeds, false if the label already exists. Throws exception on error.
 */
bool LightningGraph::AddLabel(const std::string& label, size_t n_fields, const FieldSpec* fds,
                              bool is_vertex, const std::string& primary_field,
                              const EdgeConstraints& edge_constraints) {
    // check that label and field names are legal
    CheckIsValidLabelFieldName<true>(label);
    if (_F_UNLIKELY(n_fields > 1024)) {
        throw InputError(
            FMA_FMT("Label[{}]: Number of fields cannot exceed 1024, given [{}]", label, n_fields));
    }

    std::set<std::string> unique_fds;
    for (size_t i = 0; i < n_fields; i++) {
        using namespace import_v2;
        if (fds[i].name == KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP) ||
            fds[i].name == KeyWordFunc::GetStrFromKeyWord(KeyWord::SRC_ID) ||
            fds[i].name == KeyWordFunc::GetStrFromKeyWord(KeyWord::DST_ID)) {
            throw InputError(FMA_FMT(
                R"(Label[{}]: Property name cannot be "SKIP" or "SRC_ID" or "DST_ID")", label));
        }
        auto ret = unique_fds.insert(fds[i].name);
        if (!ret.second)
            throw InputError(
                FMA_FMT("Label[{}]: Duplicate property definition: [{}]", label, fds[i].name));
    }

    // check constraints
    if (is_vertex) {
        if (n_fields == 0)
            throw InputError(FMA_FMT("Vertex[{}]: Schema must have properties", label));
        if (primary_field.empty())
            throw InputError(
                FMA_FMT("Vertex[{}]: Schema must specify the primary property", label));
        bool found = false;
        for (size_t i = 0; i < n_fields; i++) {
            if (fds[i].name != primary_field) continue;
            found = true;
            if (fds[i].optional)
                throw InputError(
                    FMA_FMT("Vertex[{}]: Primary property can not be optional", label));
        }
        if (!found) throw InputError(FMA_FMT("Vertex[{}]: No primary property found", label));
    } else {
        auto temp = edge_constraints;
        std::sort(temp.begin(), temp.end());
        auto iter = std::unique(temp.begin(), temp.end());
        if (iter != temp.end()) {
            throw InputError(
                FMA_FMT("Duplicate constraints: [{}, {}]", iter->first, iter->second));
        }
        ScopedRef<SchemaInfo> schema = schema_.GetScopedRef();
        auto schema_info = schema.Get();
        for (auto& constraint : edge_constraints) {
            for (auto& vertex_label : {constraint.first, constraint.second}) {
                if (!schema_info->v_schema_manager.GetSchema(vertex_label)) {
                    throw InputError(FMA_FMT("No such vertex label: {}", vertex_label));
                }
            }
        }
    }
    for (size_t i = 0; i < n_fields; i++) {
        auto& fn = fds[i].name;
        CheckIsValidLabelFieldName<false>(fn);
    }
    // Need to hold a write lock here since if two threads tries to add label concurrently, data
    // race to schema_ may happen.
    _HoldWriteLock(meta_lock_);
    Transaction txn = CreateWriteTxn(false);
    ScopedRef<SchemaInfo> curr_schema = schema_.GetScopedRef();
    std::unique_ptr<SchemaInfo> new_schema(new SchemaInfo(*curr_schema.Get()));
    SchemaManager* sm = is_vertex ? &new_schema->v_schema_manager : &new_schema->e_schema_manager;
    bool r = sm->AddLabel(txn.GetTxn(), is_vertex, label, n_fields, fds, primary_field,
                          edge_constraints);
    if (r && is_vertex) {
        // add vertex primary index
        Schema* schema = sm->GetSchema(label);
        FMA_DBG_ASSERT(schema);
        const _detail::FieldExtractor* extractor = schema->GetFieldExtractor(primary_field);
        FMA_DBG_ASSERT(extractor);
        std::unique_ptr<VertexIndex> index;
        index_manager_->AddVertexIndex(txn.GetTxn(), label, primary_field, extractor->Type(), true,
                                       index);
        index->SetReady();
        schema->MarkVertexIndexed(extractor->GetFieldId(), index.release());
    }
    if (r) {
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
 * \param   is_vertex   True if this is vertex label, otherwise
 *          it is edge label.
 * \param   primary_field The vertex primary property, must be
 *          set when is_vertex is true
 * \param   edge_constraints The edge constraints, can be set
 *          when is_vertex is false
 *
 * \return  True if it succeeds, false if the label already exists. Throws exception on error.
 */
bool LightningGraph::AddLabel(const std::string& label, const std::vector<FieldSpec>& fds,
                              bool is_vertex, const std::string& primary_field,
                              const EdgeConstraints& edge_constraints) {
    return AddLabel(label, fds.size(), fds.data(), is_vertex, primary_field, edge_constraints);
}

bool LightningGraph::DelLabel(const std::string& label, bool is_vertex, size_t* n_modified) {
    FMA_INFO_STREAM(logger_) << "Deleting " << (is_vertex ? "vertex" : "edge") << " label ["
                             << label << "]";
    _HoldWriteLock(meta_lock_);
    size_t commit_size = 4096;
    // check that label and field names are legal
    CheckIsValidLabelFieldName<true>(label);
    Transaction txn = CreateWriteTxn(false);
    ScopedRef<SchemaInfo> curr_schema_info = schema_.GetScopedRef();
    SchemaManager* curr_sm =
        is_vertex ? &curr_schema_info->v_schema_manager : &curr_schema_info->e_schema_manager;
    Schema* schema = curr_sm->GetSchema(label);
    if (!schema) return false;
    std::unique_ptr<SchemaInfo> new_schema(new SchemaInfo(*curr_schema_info.Get()));
    LabelId lid = schema->GetLabelId();
    size_t modified = 0;
    // now delete every node/edge that has this label
    if (is_vertex) {
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
                    bool r = graph_->DeleteVertex(txn.GetTxn(), iit.GetVid(), nullptr, nullptr);
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

template <typename GenNewSchema, typename MakeNewProp, typename ModifyIndex,
          typename ModifyEdgeIndex>
bool LightningGraph::_AlterLabel(
    bool is_vertex, const std::string& label,
    const GenNewSchema& gen_new_schema,                // std::function<Schema(Schema*)>
    const MakeNewProp& make_new_prop_and_destroy_old,  // std::function<Value(const Value&, Schema*,
                                                       // Schema*, Transaction&)>
    const ModifyIndex&
        modify_index,  // std::function<void(Schema*, Schema*, CleanupActions&, Transaction&)>
    const ModifyEdgeIndex& modify_edge_index, size_t* n_modified, size_t commit_size) {
    FMA_DBG_STREAM(logger_) << "_AlterLabel(batch_size=" << commit_size << ")";
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

    // modify vertexes and edges
    size_t modified = 0;
    size_t n_committed = 0;
    LabelId curr_lid = curr_schema->GetLabelId();
    if (is_vertex) {
        // scan and modify the vertexes
        std::unique_ptr<lgraph::graph::VertexIterator> vit(
            new lgraph::graph::VertexIterator(graph_->GetUnmanagedVertexIterator(&txn.GetTxn())));
        while (vit->IsValid()) {
            const Value& prop = vit->GetProperty();
            if (curr_sm->GetRecordLabelId(prop) == curr_lid) {
                modified++;
                Value new_prop = make_new_prop_and_destroy_old(prop, curr_schema, new_schema, txn);
                vit->RefreshContentIfKvIteratorModified();
                vit->SetProperty(new_prop);
                if (modified - n_committed >= commit_size) {
#if PERIODIC_COMMIT
                    VertexId vid = vit->GetId();
                    vit.reset();
                    txn.Commit();
                    n_committed = modified;
                    FMA_INFO_STREAM(logger_) << "Committed " << n_committed << " changes.";
                    txn = CreateWriteTxn(false, false, false);
                    vit.reset(new lgraph::graph::VertexIterator(
                        graph_->GetUnmanagedVertexIterator(&txn.GetTxn(), vid, true)));
#else
                    n_committed = modified;
                    FMA_INFO_STREAM(logger_) << "Made " << n_committed << " changes.";
#endif
                }
            }
            vit->Next();
        }
        modify_index(curr_schema, new_schema, rollback_actions, txn);
    } else {
        // scan and modify
        std::unique_ptr<lgraph::graph::VertexIterator> vit(
            new lgraph::graph::VertexIterator(graph_->GetUnmanagedVertexIterator(&txn.GetTxn())));
        while (vit->IsValid()) {
            for (auto eit = vit->GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                if (eit.GetLabelId() == curr_lid) {
                    modified++;
                    Value new_prop = make_new_prop_and_destroy_old(eit.GetProperty(), curr_schema,
                                                                   new_schema, txn);
                    eit.RefreshContentIfKvIteratorModified();
                    eit.SetProperty(new_prop);
                }
            }
            vit->RefreshContentIfKvIteratorModified();
            for (auto eit = vit->GetInEdgeIterator(); eit.IsValid(); eit.Next()) {
                if (eit.GetLabelId() == curr_lid) {
                    Value new_prop = make_new_prop_and_destroy_old(eit.GetProperty(), curr_schema,
                                                                   new_schema, txn);
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
                FMA_INFO_STREAM(logger_) << "Committed " << n_committed << " changes.";
                txn = CreateWriteTxn(false, false, false);
                vit.reset(new lgraph::graph::VertexIterator(
                    graph_->GetUnmanagedVertexIterator(&txn.GetTxn(), vid, true)));
#else
                n_committed = modified;
                FMA_INFO_STREAM(logger_) << "Made " << n_committed << " changes.";
#endif
            }
            vit->Next();
        }
        modify_edge_index(curr_schema, new_schema, rollback_actions, txn);
    }
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
        throw InputError("No such edge label: " + edge_label);
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
            throw InputError(FMA_FMT("Constraints are empty"));
        }
        // check duplicate
        auto temp = constraints;
        std::sort(temp.begin(), temp.end());
        auto iter = std::unique(temp.begin(), temp.end());
        if (iter != temp.end()) {
            throw InputError(
                FMA_FMT("Duplicate constraints: [{}, {}]", iter->first, iter->second));
        }
    }
    _HoldWriteLock(meta_lock_);
    Transaction txn = CreateWriteTxn(false);
    ScopedRef<SchemaInfo> curr_schema_info = schema_.GetScopedRef();
    std::unique_ptr<SchemaInfo> new_schema_info(new SchemaInfo(*curr_schema_info.Get()));
    Schema* e_schema = new_schema_info->e_schema_manager.GetSchema(edge_label);
    if (!e_schema) {
        throw InputError("No such edge label: " + edge_label);
    }
    Schema new_e_schema(*e_schema);
    EdgeConstraints ecs = new_e_schema.GetEdgeConstraints();
    if (ecs.empty()) {
        throw InputError(FMA_FMT("Failed to add constraint: "
            "edge[{}] constraints are empty", edge_label));
    }
    for (auto& ec : constraints) {
        for (auto& vertex_label : {ec.first, ec.second}) {
            if (!new_schema_info->v_schema_manager.GetSchema(vertex_label)) {
                throw InputError(FMA_FMT("No such vertex label: {}", vertex_label));
            }
        }
        auto iter = std::find_if(ecs.begin(), ecs.end(),
                                 [&ec](auto &item){return ec == item;});
        if (iter != ecs.end()) {
            throw InputError(FMA_FMT("Failed to add constraint: "
                "constraint [{},{}] already exist", ec.first, ec.second));
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
    FMA_INFO_STREAM(logger_) << FMA_FMT("Deleting fields {} from {} label [{}].", del_fields_,
                                        is_vertex ? "vertex" : "edge", label);
    _HoldReadLock(meta_lock_);
    // make unique
    std::vector<std::string> del_fields(del_fields_);
    std::sort(del_fields.begin(), del_fields.end());
    del_fields.erase(std::unique(del_fields.begin(), del_fields.end()), del_fields.end());
    if (del_fields.empty()) throw InputError("No fields specified.");

    // get fids of the fields in new schema
    std::vector<size_t> new_fids;
    std::vector<size_t> old_field_pos;
    std::vector<const _detail::FieldExtractor*> blob_deleted_fes;

    // make new schema
    auto setup_and_gen_new_schema = [&](Schema* curr_schema) -> Schema {
        Schema new_schema(*curr_schema);
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
                index_manager_->DeleteVertexIndex(txn.GetTxn(), label, extractor->Name());
            } else if (extractor->FullTextIndexed()) {
                index_manager_->DeleteFullTextIndex(txn.GetTxn(), is_vertex, label,
                                                    extractor->Name());
            }
        }
    };

    auto delete_edge_indexes = [&](Schema* curr_schema, Schema* new_schema,
                                   CleanupActions& rollback_actions, Transaction& txn) {
        // delete the indexes
        auto fids = curr_schema->GetFieldIds(del_fields);
        // delete indexes if necessary
        for (auto& f : fids) {
            auto* extractor = curr_schema->GetFieldExtractor(f);
            if (extractor->GetEdgeIndex()) {
                index_manager_->DeleteEdgeIndex(txn.GetTxn(), label, extractor->Name());
            }
        }
    };
    return _AlterLabel(is_vertex, label, setup_and_gen_new_schema, make_new_prop_and_destroy_old,
                       delete_indexes, delete_edge_indexes, n_modified, 100000);
}

bool LightningGraph::AlterLabelAddFields(const std::string& label,
                                         const std::vector<FieldSpec>& to_add,
                                         const std::vector<FieldData>& default_values,
                                         bool is_vertex, size_t* n_modified) {
    FMA_INFO_STREAM(logger_) << FMA_FMT("Adding fields {} with values {} to {} label [{}].", to_add,
                                        default_values, is_vertex ? "vertex" : "edge", label);
    _HoldReadLock(meta_lock_);
    if (to_add.empty()) throw InputError("No fields specified.");
    if (to_add.size() != default_values.size())
        throw InputError("Number of fields and default values are not equal.");
    // de-duplicate
    {
        std::unordered_set<std::string> s;
        for (auto& f : to_add) {
            if (s.find(f.name) != s.end())
                throw InputError(FMA_FMT("Field [{}] is defined more than once.", f.name));
            s.insert(f.name);
        }
    }
    // check input
    {
        for (size_t i = 0; i < to_add.size(); i++) {
            auto& fs = to_add[i];
            if (!fs.optional && default_values[i].IsNull())
                throw InputError(
                    FMA_FMT("Field [{}] is declared as non-optional but the default value is NULL.",
                            fs.name));
        }
    }

    std::vector<size_t> dst_fids;  // field ids of old fields in new record
    std::vector<size_t> src_fids;  // field ids of old fields in old record
    std::vector<size_t> new_fids;  // ids of newly added fields
    // make new schema
    auto setup_and_gen_new_schema = [&](Schema* curr_schema) -> Schema {
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
            auto* extr = new_schema->GetFieldExtractor(fid);
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

    auto delete_edge_indexes = [](Schema* curr_schema, Schema* new_schema,
                                  CleanupActions& rollback_actions, Transaction& txn) {};

    return _AlterLabel(is_vertex, label, setup_and_gen_new_schema, make_new_prop_and_destroy_old,
                       delete_indexes, delete_edge_indexes, n_modified, 100000);
}

bool LightningGraph::AlterLabelModFields(const std::string& label,
                                         const std::vector<FieldSpec>& to_mod, bool is_vertex,
                                         size_t* n_modified) {
    FMA_INFO_STREAM(logger_) << FMA_FMT("Modifying fields {} in {} label [{}].", to_mod,
                                        is_vertex ? "vertex" : "edge", label);
    _HoldReadLock(meta_lock_);
    if (to_mod.empty()) throw InputError("No fields specified.");
    // de-duplicate
    {
        std::unordered_set<std::string> s;
        for (auto& f : to_mod) {
            if (s.find(f.name) != s.end())
                throw InputError(FMA_FMT("Field [{}] is specified more than once.", f.name));
            s.insert(f.name);
        }
    }

    // make new schema
    std::vector<size_t> direct_copy_dst_fids;
    std::vector<size_t> direct_copy_src_fids;
    std::vector<size_t> mod_dst_fids;
    std::vector<size_t> mod_src_fids;
    auto setup_and_gen_new_schema = [&](Schema* curr_schema) -> Schema {
        // check field types
        for (auto& f : to_mod) {
            auto* extractor = curr_schema->GetFieldExtractor(f.name);
            if (extractor->Type() == FieldType::BLOB && f.type != FieldType::BLOB) {
                throw InputError(
                    FMA_FMT("Field [{}] is of type BLOB, which cannot be converted to other types.",
                            f.name));
            }
            if (extractor->FullTextIndexed()) {
                throw InputError(
                    FMA_FMT("Field [{}] has fulltext index, which cannot be converted to other "
                            "non-STRING types.",
                            f.name));
            }
        }
        Schema new_schema(*curr_schema);
        new_schema.ModFields(to_mod);
        FMA_DBG_ASSERT(new_schema.GetNumFields() == curr_schema->GetNumFields());
        for (size_t i = 0; i < new_schema.GetNumFields(); i++) {
            const _detail::FieldExtractor* dst_fe = new_schema.GetFieldExtractor(i);
            const std::string& fname = dst_fe->Name();
            const _detail::FieldExtractor* src_fe = curr_schema->GetFieldExtractor(i);
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
            const _detail::FieldExtractor* dst_fe = new_schema->GetFieldExtractor(mod_dst_fids[i]);
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
            }
        }
    };

    auto delete_edge_indexes = [&](Schema* curr_schema, Schema* new_schema,
                                   CleanupActions& rollback_actions, Transaction& txn) {
        std::vector<size_t> mod_fids;
        for (auto& f : to_mod) mod_fids.push_back(curr_schema->GetFieldId(f.name));
        // delete indexes if necessary
        for (auto& f : mod_fids) {
            auto* extractor = curr_schema->GetFieldExtractor(f);
            if (extractor->GetEdgeIndex()) {
                new_schema->UnEdgeIndex(f);
                index_manager_->DeleteEdgeIndex(txn.GetTxn(), label, extractor->Name());
            }
        }
    };
    return _AlterLabel(
        is_vertex, label, setup_and_gen_new_schema, make_new_prop_and_destroy_old, delete_indexes,
        delete_edge_indexes, n_modified,
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
 * \param   is_unique   True if the field content is unique for each vertex.
 *
 * \return  True if it succeeds, false if the index already exists. Throws exception on error.
 */
bool LightningGraph::_AddEmptyIndex(const std::string& label, const std::string& field,
                                    bool is_unique, bool is_vertex) {
    _HoldWriteLock(meta_lock_);
    Transaction txn = CreateWriteTxn(false);
    std::unique_ptr<SchemaInfo> new_schema(new SchemaInfo(*schema_.GetScopedRef().Get()));
    Schema* schema = is_vertex ? new_schema->v_schema_manager.GetSchema(label)
                               : new_schema->e_schema_manager.GetSchema(label);
    if (!schema) throw LabelNotExistException(label);
    const _detail::FieldExtractor* extractor = schema->GetFieldExtractor(field);
    if ((extractor->GetVertexIndex() && is_vertex) || (extractor->GetEdgeIndex() && !is_vertex))
        return false;  // index already exist
    if (is_vertex) {
        std::unique_ptr<VertexIndex> index;
        index_manager_->AddVertexIndex(txn.GetTxn(), label, field, extractor->Type(), is_unique,
                                       index);
        index->SetReady();
        schema->MarkVertexIndexed(extractor->GetFieldId(), index.release());
    } else {
        std::unique_ptr<EdgeIndex> edge_index;
        index_manager_->AddEdgeIndex(txn.GetTxn(), label, field, extractor->Type(), is_unique,
                                     edge_index);
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
            throw std::runtime_error(
                fma_common::StringFormatter::Format("String size too large: {}", s));
        size_ptr_.size = s;
        uint64_t up = (uint64_t)p;
        if ((up & ((uint64_t)0xFFFF << 48)) != 0)
            throw std::runtime_error(fma_common::StringFormatter::Format(
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
                euid.lid == rhs.euid.lid && euid.eid < rhs.euid.eid);
    }
};
template <typename T>
T GetIndexKeyFromValue(const Value& v) {
    return v.AsType<T>();
}

template <>
ConstStringRef GetIndexKeyFromValue<ConstStringRef>(const Value& v) {
    if (v.Size() > _detail::MAX_KEY_SIZE)
        throw InputError(fma_common::StringFormatter::Format(
            "This string is too long to be indexed, max indexable size={}, string size={}, "
            "content: {}...",
            _detail::MAX_KEY_SIZE, v.Size(), v.AsString().substr(0, 100)));
    return ConstStringRef(v.Data(), v.Size());
}

template <>
std::string GetIndexKeyFromValue<std::string>(const Value& v) {
    if (v.Size() > _detail::MAX_KEY_SIZE)
        throw InputError(fma_common::StringFormatter::Format(
            "This string is too long to be indexed, max indexable size={}, string size={}, "
            "content: {}...",
            _detail::MAX_KEY_SIZE, v.Size(), v.AsString().substr(0, 100)));
    return v.AsString();
}

template <typename T>
inline Value GetKeyConstRef(const T& key) {
    return Value::ConstRef(key);
}

inline Value GetKeyConstRef(const ConstStringRef& key) { return Value(key.Ptr(), key.Size()); }

template <typename T>
inline void CheckKeySizeForIndex(const T&) {}

template <>
inline void CheckKeySizeForIndex<std::string>(const std::string& str) {
    if (str.size() >= _detail::MAX_KEY_SIZE) {
        throw InputError(FMA_FMT("String key too long for index: {}{}", str.substr(0, 1024),
                                 str.size() > 1024 ? "..." : ""));
    }
}

template <typename T>
void LightningGraph::BatchBuildIndex(Transaction& txn, SchemaInfo* new_schema_info,
                                     LabelId label_id, size_t field_id, bool is_unique,
                                     VertexId start_vid, VertexId end_vid, bool is_vertex) {
    if (is_vertex) {
        SchemaManager* schema_manager = &new_schema_info->v_schema_manager;
        auto* field_extractor = schema_manager->GetSchema(label_id)->GetFieldExtractor(field_id);
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
                const Value& prop = it.GetProperty();
                if (schema_manager->GetRecordLabelId(prop) != label_id) continue;
                const T& key = GetIndexKeyFromValue<T>(field_extractor->GetConstRef(prop));
                CheckKeySizeForIndex(key);
                key_vids.emplace_back(key, it.GetId());
            }
            LGRAPH_PSORT(key_vids.begin(), key_vids.end());
            // check uniqueness
            if (is_unique) {
                // if there is only one block, we use AppendKv,
                // so checking for duplicate is required
                // if there are multiple blocks,
                // then uniqueness will be checked when we insert the
                // keys into index, and this is not required,
                // but still good to find duplicates early
                for (size_t i = 1; i < key_vids.size(); i++) {
                    if (key_vids[i].key == key_vids[i - 1].key)
                        throw InputError(fma_common::StringFormatter::Format(
                            "Duplicate keys [{}] found for vids {} and {}.", key_vids[i].key,
                            key_vids[i - 1].vid, key_vids[i].vid));
                }
            }
            // now insert into index table
            if (max_block_size >= (size_t)(end_vid - start_vid)) {
                // block size large enough, so there is only one pass, use AppendKv
                if (is_unique) {
                    for (auto& kv : key_vids)
                        index->_AppendVertexIndexEntry(txn.GetTxn(), GetKeyConstRef(kv.key),
                                                       (VertexId)kv.vid);
                } else {
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
                    if (!key_vids.empty()) {
                        index->_AppendNonUniqueVertexIndexEntry(txn.GetTxn(), GetKeyConstRef(key),
                                                                vids);
                    }
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
        auto* field_extractor = schema_manager->GetSchema(label_id)->GetFieldExtractor(field_id);
        FMA_DBG_ASSERT(field_extractor);
        EdgeIndex* edge_index = field_extractor->GetEdgeIndex();
        FMA_DBG_ASSERT(edge_index);
        static const size_t max_block_size = 1 << 28;
        for (VertexId vid = start_vid; vid < end_vid; vid += max_block_size) {
            std::vector<KeyEUid<T>> key_euids;
            VertexId curr_end = std::min<VertexId>(end_vid, vid + max_block_size);
            key_euids.reserve(curr_end - vid);
            for (auto it = txn.GetVertexIterator(vid, true); it.IsValid() && it.GetId() < curr_end;
                 it.Next()) {
                auto et = it.GetOutEdgeIterator();
                while (et.IsValid()) {
                    const Value& prop = et.GetProperty();
                    if (et.GetLabelId() != label_id) {
                        et.Next();
                        continue;
                    }
                    const T& key = GetIndexKeyFromValue<T>(field_extractor->GetConstRef(prop));
                    CheckKeySizeForIndex(key);
                    key_euids.emplace_back(key, et.GetUid());
                    et.Next();
                }
            }
            LGRAPH_PSORT(key_euids.begin(), key_euids.end());
            // check uniqueness
            if (is_unique) {
                // if there is only one block, we use AppendKv,
                // so checking for duplicate is required
                // if there are multiple blocks,
                // then uniqueness will be checked when we insert the
                // keys into index, and this is not required,
                // but still good to find duplicates early
                for (size_t i = 1; i < key_euids.size(); i++) {
                    if (key_euids[i].key == key_euids[i - 1].key)
                        throw InputError(fma_common::StringFormatter::Format(
                            "Duplicate keys [{}] found for vid {} dst{} eid (),"
                            "and {} {} {}.",
                            key_euids[i].key, key_euids[i].euid.src, key_euids[i].euid.dst,
                            key_euids[i].euid.eid, key_euids[i - 1].euid.src,
                            key_euids[i - 1].euid.dst, key_euids[i - 1].euid.eid));
                }
            }
            // now insert into index table
            if (max_block_size >= (size_t)(end_vid - start_vid)) {
                // block size large enough, so there is only one pass, use AppendKv
                if (is_unique) {
                    for (auto& kv : key_euids)
                        edge_index->_AppendIndexEntry(txn.GetTxn(), GetKeyConstRef(kv.key),
                                                      (EdgeUid)kv.euid);
                } else {
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
                    if (!key_euids.empty()) {
                        edge_index->_AppendNonUniqueIndexEntry(txn.GetTxn(), GetKeyConstRef(key),
                                                               euids);
                    }
                }
            } else {
                // multiple blocks, use regular index calls
                for (auto& kv : key_euids) {
                    edge_index->Add(txn.GetTxn(), GetKeyConstRef(kv.key), kv.euid.src, kv.euid.dst,
                                    kv.euid.lid, kv.euid.tid, kv.euid.eid);
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
        throw InputError(
            fma_common::StringFormatter::Format("Vertex Label [{}] does not exist.", label));
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
        throw InputError(
            fma_common::StringFormatter::Format("Edge Label [{}] does not exist.", label));
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
    std::set<LabelId> v_lids, e_lids;
    ScopedRef<SchemaInfo> curr_schema_info = schema_.GetScopedRef();
    const auto& all_vertex_labels = curr_schema_info->v_schema_manager.GetAllLabels();
    const auto& all_edge_labels = curr_schema_info->e_schema_manager.GetAllLabels();
    for (const auto& label : v_labels) {
        auto* schema = curr_schema_info->v_schema_manager.GetSchema(label);
        if (!schema) {
            throw InputError(
                fma_common::StringFormatter::Format("Vertex Label [{}] does not exist.", label));
        }
        if (schema->GetFullTextFields().empty()) {
            throw InputError(fma_common::StringFormatter::Format(
                "Vertex Label [{}] has no fulltext index.", label));
        }
        v_lids.emplace(schema->GetLabelId());
    }
    for (const auto& label : e_labels) {
        auto* schema = curr_schema_info->e_schema_manager.GetSchema(label);
        if (!schema) {
            throw InputError(
                fma_common::StringFormatter::Format("Edge Label [{}] does not exist.", label));
        }
        if (schema->GetFullTextFields().empty()) {
            throw InputError(fma_common::StringFormatter::Format(
                "Edge Label [{}] has no fulltext index.", label));
        }
        e_lids.emplace(schema->GetLabelId());
    }
    RebuildFullTextIndex(v_lids, e_lids);
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
    for (LabelId id : v_lids) {
        Schema* schema = curr_schema_info->v_schema_manager.GetSchema(id);
        const auto& fulltext_filelds = schema->GetFullTextFields();
        LabelId lid = schema->GetLabelId();
        for (auto iit = txn.GetVertexIndexIterator(schema->GetLabel(), schema->GetTemporalField());
             iit.IsValid(); iit.Next()) {
            VertexId vid = iit.GetVid();
            const auto& properties = txn.GetVertexIterator(vid).GetProperty();
            std::vector<std::pair<std::string, std::string>> kvs;
            for (auto& idx : fulltext_filelds) {
                auto fe = schema->GetFieldExtractor(idx);
                if (fe->GetIsNull(properties)) continue;
                kvs.emplace_back(fe->Name(), fe->FieldToString(properties));
            }
            fulltext_index_->AddVertex(vid, lid, kvs);
        }
    }
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
                        const auto& properties = eit.GetProperty();
                        if (fe->GetIsNull(properties)) continue;
                        kvs.emplace_back(fe->Name(), fe->FieldToString(properties));
                    }
                    fulltext_index_->AddEdge({euid.src, euid.dst, euid.lid, euid.tid, euid.eid},
                                             kvs);
                }
            }
        }
    }
    fulltext_index_->Commit();
}

bool LightningGraph::AddFullTextIndex(bool is_vertex, const std::string& label,
                                      const std::string& field) {
    if (!fulltext_index_) {
        throw InputError("Fulltext index is not enabled");
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
        throw InputError(
            fma_common::StringFormatter::Format("label \"{}\" does not exist.", label));
    }
    const _detail::FieldExtractor* extractor = schema->GetFieldExtractor(field);
    if (!extractor) {
        throw InputError(fma_common::StringFormatter::Format("field \"{}\":\"{}\" does not exist.",
                                                             label, field));
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

bool LightningGraph::BlockingAddIndex(const std::string& label, const std::string& field,
                                      bool is_unique, bool is_vertex, bool known_vid_range,
                                      VertexId start_vid, VertexId end_vid) {
    _HoldWriteLock(meta_lock_);
    Transaction txn = CreateWriteTxn(false);
    std::unique_ptr<SchemaInfo> new_schema(new SchemaInfo(*schema_.GetScopedRef().Get()));
    Schema* schema = is_vertex ? new_schema->v_schema_manager.GetSchema(label)
                               : new_schema->e_schema_manager.GetSchema(label);
    if (!schema) {
        if (is_vertex)
            throw InputError(
                fma_common::StringFormatter::Format("Vertex label \"{}\" "
                                                    "does not exist.",
                                                    label));
        else
            throw InputError(
                fma_common::StringFormatter::Format("Edge label \"{}\" "
                                                    "does not exist.",
                                                    label));
    }
    const _detail::FieldExtractor* extractor = schema->GetFieldExtractor(field);
    if (!extractor) {
        if (is_vertex)
            throw InputError(fma_common::StringFormatter::Format(
                "Vertex field \"{}\":\"{}\" does not exist.", label, field));
        else
            throw InputError(fma_common::StringFormatter::Format(
                "Edge field \"{}\":\"{}\" does not exist.", label, field));
    }
    if ((extractor->GetVertexIndex() && is_vertex) || (extractor->GetEdgeIndex() && !is_vertex))
        return false;  // index already exist

    if (extractor->IsOptional() && is_unique) {
        throw InputError(
            FMA_FMT("Unique index cannot be added to an optional field [{}:{}]", label, field));
    }
    if (is_vertex) {
        std::unique_ptr<VertexIndex> index;
        index_manager_->AddVertexIndex(txn.GetTxn(), label, field, extractor->Type(), is_unique,
                                       index);
        index->SetReady();
        schema->MarkVertexIndexed(extractor->GetFieldId(), index.release());
        // now build index
        if (!known_vid_range) {
            start_vid = 0;
            end_vid = txn.GetLooseNumVertex();
            // vid range not known, try getting from index
            auto& indexed_fields = schema->GetIndexedFields();
            if (!indexed_fields.empty()) {
                VertexIndex* idx =
                    schema->GetFieldExtractor(*indexed_fields.begin())->GetVertexIndex();
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
    } else {
        std::unique_ptr<EdgeIndex> edge_index;
        index_manager_->AddEdgeIndex(txn.GetTxn(), label, field, extractor->Type(), is_unique,
                                     edge_index);
        edge_index->SetReady();
        schema->MarkEdgeIndexed(extractor->GetFieldId(), edge_index.release());
        // now build index
        if (!known_vid_range) {
            start_vid = 0;
            end_vid = txn.GetLooseNumVertex();
            // vid range not known, try getting from index
            auto& indexed_fields = schema->GetIndexedFields();
            if (!indexed_fields.empty()) {
                EdgeIndex* idx = schema->GetFieldExtractor(*indexed_fields.begin())->GetEdgeIndex();
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
            }
        }
    }
    // now we know the start and end vid of this label, start building
    // try building index
    LabelId lid = schema->GetLabelId();
    size_t fid = schema->GetFieldId(field);
    switch (extractor->Type()) {
    case FieldType::BOOL:
        BatchBuildIndex<int8_t>(txn, new_schema.get(), lid, fid, is_unique, start_vid, end_vid,
                                is_vertex);
        break;
    case FieldType::INT8:
        BatchBuildIndex<int8_t>(txn, new_schema.get(), lid, fid, is_unique, start_vid, end_vid,
                                is_vertex);
        break;
    case FieldType::INT16:
        BatchBuildIndex<int16_t>(txn, new_schema.get(), lid, fid, is_unique, start_vid, end_vid,
                                 is_vertex);
        break;
    case FieldType::INT32:
        BatchBuildIndex<int32_t>(txn, new_schema.get(), lid, fid, is_unique, start_vid, end_vid,
                                 is_vertex);
        break;
    case FieldType::INT64:
        BatchBuildIndex<int64_t>(txn, new_schema.get(), lid, fid, is_unique, start_vid, end_vid,
                                 is_vertex);
        break;
    case FieldType::DATE:
        BatchBuildIndex<int32_t>(txn, new_schema.get(), lid, fid, is_unique, start_vid, end_vid,
                                 is_vertex);
        break;
    case FieldType::DATETIME:
        BatchBuildIndex<int64_t>(txn, new_schema.get(), lid, fid, is_unique, start_vid, end_vid,
                                 is_vertex);
        break;
    case FieldType::FLOAT:
        BatchBuildIndex<float>(txn, new_schema.get(), lid, fid, is_unique, start_vid, end_vid,
                               is_vertex);
        break;
    case FieldType::DOUBLE:
        BatchBuildIndex<double>(txn, new_schema.get(), lid, fid, is_unique, start_vid, end_vid,
                                is_vertex);
        break;
    case FieldType::STRING:
        BatchBuildIndex<ConstStringRef>(txn, new_schema.get(), lid, fid, is_unique, start_vid,
                                        end_vid, is_vertex);
        break;
    case FieldType::BLOB:
        throw InputError(std::string("Field of type ") +
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
    FMA_LOG() << "Creating batch index for " << spec.label << ":" << spec.field;
    next_vertex_id = first_vertex;
    std::deque<KeyVid<T>> key_vids;
    std::deque<KeyEUid<T>> key_euids;
    if (!_AddEmptyIndex(spec.label, spec.field, spec.unique, is_vertex) && is_vertex) {
        throw InputError(fma_common::StringFormatter::Format(
            "Failed to create index {}:{}: index already exists", spec.label, spec.field));
    }
    if (is_vertex) {
        auto txn = CreateReadTxn();
        {
            LabelId lid = txn.GetLabelId(true, spec.label);
            FMA_LOG() << "Scanning vertexes for keys";
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
                key_vids.emplace_back(GetIndexKeyFromValue<T>(extractor->GetConstRef(v)),
                                      vit.GetId());
            }
            if (!vit.IsValid()) {
                txn.Abort();
                next_vertex_id = GetNumVertices();
            }
            txn.Abort();
        }
        FMA_LOG() << "Sorting by unique id";
        LGRAPH_PSORT(key_vids.begin(), key_vids.end());
        FMA_LOG() << "Dumping index";
        txn = CreateWriteTxn();
        auto index = txn.GetVertexIndex(spec.label, spec.field);
        if (spec.unique) {
            for (size_t i = 0; i < key_vids.size(); i++) {
                if (i != 0 && i % batch_commit_size == 0) {
                    txn.Commit();
                    FMA_LOG() << "committed " << i << " keys";
                    txn = CreateWriteTxn();
                }
                auto& kv = key_vids[i];
                index->_AppendVertexIndexEntry(txn.GetTxn(), GetKeyConstRef(kv.key),
                                               (VertexId)kv.vid);
            }
        } else {
            T key = key_vids.empty() ? T() : key_vids.front().key;
            std::vector<VertexId> vids;
            for (size_t i = 0; i < key_vids.size(); ++i) {
                if (i != 0 && i % batch_commit_size == 0) {
                    txn.Commit();
                    FMA_LOG() << "committed " << i << " keys";
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
            if (!key_vids.empty()) {
                index->_AppendNonUniqueVertexIndexEntry(txn.GetTxn(), GetKeyConstRef(key), vids);
            }
        }
        txn.Commit();
        index->SetReady();
    } else {
        auto txn = CreateReadTxn();
        {
            LabelId lid = txn.GetLabelId(false, spec.label);
            FMA_LOG() << "Scanning edges for keys";
            auto vit = txn.GetVertexIterator(first_vertex, true);
            Value v = vit.GetProperty();
            auto start_lid = SchemaManager::GetRecordLabelId(v);
            auto schema = txn.curr_schema_->e_schema_manager.GetSchema(lid);
            FMA_DBG_ASSERT(schema);
            auto extractor = schema->GetFieldExtractor(spec.field);
            FMA_DBG_ASSERT(extractor);
            for (; vit.IsValid(); vit.Next()) {
                Value v = vit.GetProperty();
                auto eit = vit.GetOutEdgeIterator();
                for (; eit.IsValid(); eit.Next()) {
                    if (eit.GetLabelId() == lid) {
                        EdgeUid euid = eit.GetUid();
                        key_euids.emplace_back(GetIndexKeyFromValue<T>(extractor->GetConstRef(v)),
                                               euid);
                    }
                }
                auto v_lid = SchemaManager::GetRecordLabelId(v);
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
        FMA_LOG() << "Sorting by unique id";
        LGRAPH_PSORT(key_euids.begin(), key_euids.end());
        FMA_LOG() << "Dumping index";
        txn = CreateWriteTxn();
        auto index = txn.GetEdgeIndex(spec.label, spec.field);
        if (spec.unique) {
            for (size_t i = 0; i < key_euids.size(); i++) {
                if (i != 0 && i % batch_commit_size == 0) {
                    txn.Commit();
                    FMA_LOG() << "committed " << i << " keys";
                    txn = CreateWriteTxn();
                }
                auto& kv = key_euids[i];
                index->_AppendIndexEntry(txn.GetTxn(), GetKeyConstRef(kv.key), (EdgeUid)kv.euid);
            }
        } else {
            T key = key_euids.empty() ? T() : key_euids.front().key;
            std::vector<EdgeUid> euids;
            for (size_t i = 0; i < key_euids.size(); ++i) {
                if (i != 0 && i % batch_commit_size == 0) {
                    txn.Commit();
                    FMA_LOG() << "committed " << i << " keys";
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
                throw InputError(fma_common::StringFormatter::Format(
                    "Duplicate index specified for {}:{}", kv.first, v[i].spec.field));
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
                throw InputError(fma_common::StringFormatter::Format(
                    "Field {} does not exist for label {}", st.spec.field, st.spec.label));
            }
            st.type = it->second;
        }
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
                throw InternalError(fma_common::StringFormatter::Format(
                    "Vertex Ids are not totally ordered: "
                    "found vertex vid={} with label {} after scanning the last range. "
                    "Please delete the indexes of this label and retry.",
                    vit.GetId(), txn.GetVertexLabel(vit)));
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
                    throw InputError(std::string("Field of type ") +
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
                FMA_WARN() << "Label " << label << " specified, but no vertex of that type exists.";
                for (auto& spec : specs) {
                    _AddEmptyIndex(spec.spec.label, spec.spec.field, spec.spec.unique, is_vertex);
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
        throw InputError(
            fma_common::StringFormatter::Format("label \"{}\" does not exist.", label));
    }
    const _detail::FieldExtractor* extractor = schema->GetFieldExtractor(field);
    if (!extractor) {
        throw InputError(fma_common::StringFormatter::Format("field \"{}\":\"{}\" does not exist.",
                                                             label, field));
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
    if (field == schema->GetTemporalField()) {
        throw PrimaryIndexCannotBeDeletedException(field);
    }
    const _detail::FieldExtractor* extractor = schema->GetFieldExtractor(field);
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

void LightningGraph::DropAllIndex() {
    try {
        _HoldWriteLock(meta_lock_);
        Transaction txn = CreateWriteTxn(false);
        ScopedRef<SchemaInfo> curr_schema = schema_.GetScopedRef();
        std::unique_ptr<SchemaInfo> new_schema(new SchemaInfo(*curr_schema.Get()));
        std::unique_ptr<SchemaInfo> backup_schema(new SchemaInfo(*curr_schema.Get()));
        auto indexes = index_manager_->ListAllIndexes(txn.GetTxn());

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
        FMA_WARN_STREAM(logger_) << "Failed to drop all indexes: " << e.what();
    }
}

KvStore& LightningGraph::GetStore() { return *store_; }

const DBConfig& LightningGraph::GetConfig() const { return config_; }

/**
 * Backups the current DB to the path specified.
 *
 * \param path  Full pathname of the destination.
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
        if (!fs.Mkdir(path)) throw InternalError("Failed to create dir " + path + " for snapshot.");
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
#if __APPLE__
    int r = fma_common::ExecCmd(
        fma_common::StringFormatter::Format("dd if={}/data.mdb of=/dev/null", config_.dir));
#else
    int r = fma_common::ExecCmd(
        fma_common::StringFormatter::Format("dd if={}/data.mdb of=/dev/null bs=20M", config_.dir));
#endif
    if (r) store_->WarmUp(nullptr);
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
    store_.reset(
        new KvStore(config_.dir, config_.db_size, config_.durable, config_.create_if_not_exist));
    KvTransaction txn = store_->CreateWriteTxn();
    // load meta info
    meta_table_ =
        store_->OpenTable(txn, _detail::META_TABLE, true, ComparatorDesc::DefaultComparator());
    // load schema info
    KvTable v_s_tbl = SchemaManager::OpenTable(txn, *store_, _detail::V_SCHEMA_TABLE);
    SchemaManager vs(txn, v_s_tbl, true);
    KvTable e_s_tbl = SchemaManager::OpenTable(txn, *store_, _detail::E_SCHEMA_TABLE);
    SchemaManager es(txn, e_s_tbl, false);
    es.RefreshEdgeConstraintsLids(vs);
    schema_.Assign(new SchemaInfo{std::move(vs), std::move(es)});
    // open graph
    KvTable g_tbl = graph::Graph::OpenTable(txn, *store_, _detail::GRAPH_TABLE);
    graph_.reset(new graph::Graph(txn, g_tbl, &meta_table_));
    // load index
    KvTable i_tbl = IndexManager::OpenIndexListTable(txn, *store_, _detail::INDEX_TABLE);
    index_manager_.reset(new IndexManager(txn, &schema_.GetScopedRef()->v_schema_manager,
                                          &schema_.GetScopedRef()->e_schema_manager, i_tbl, this));
    // blob manager
    KvTable b_tbl = BlobManager::OpenTable(txn, *store_, _detail::BLOB_TABLE);
    blob_manager_.reset(new BlobManager(txn, b_tbl));
    txn.Commit();
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
            FMA_WARN() << "The following fulltext indexes will not work because the configuration "
                          "item `enable_fulltext_index` is not true)";
            for (auto ft_index : ft_indexes) {
                FMA_WARN() << FMA_FMT("{} label: {} field: {}",
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
    if (!txn.read_only_) throw lgraph_api::InvalidForkError();
    return Transaction(this, txn.GetTxn());
}

bool LightningGraph::CheckDbSecret(const std::string& expected) {
    auto txn = store_->CreateWriteTxn(false);
    Value key = Value::ConstRef(lgraph::_detail::DB_SECRET_KEY);
    Value v = meta_table_.GetValue(txn, key);
    if (v.Empty()) {
        // no such value, this is a newly created DB
        meta_table_.SetValue(txn, key, Value::ConstRef(expected));
        txn.Commit();
        return true;
    } else {
        return v.AsString() == expected;
    }
}
}  // namespace lgraph
