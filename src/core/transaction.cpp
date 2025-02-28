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

#include "core/blob_manager.h"
#include "core/data_type.h"
#include "core/field_data_helper.h"
#include "core/index_manager.h"
#include "core/lightning_graph.h"
#include "core/transaction.h"

namespace lgraph {
#define FMA_FMT fma_common::StringFormatter::Format

std::vector<IndexSpec> Transaction::ListVertexIndexes() {
    return curr_schema_->v_schema_manager.ListVertexIndexes();
}

std::vector<IndexSpec> Transaction::ListEdgeIndexes() {
    return curr_schema_->e_schema_manager.ListEdgeIndexes();
}

std::vector<CompositeIndexSpec> Transaction::ListVertexCompositeIndexes() {
    return curr_schema_->v_schema_manager.ListVertexCompositeIndexes();
}

std::vector<IndexSpec> Transaction::ListVertexIndexByLabel(const std::string& label) {
    return curr_schema_->v_schema_manager.ListIndexByLabel(label);
}

std::vector<IndexSpec> Transaction::ListEdgeIndexByLabel(const std::string& label) {
    return curr_schema_->e_schema_manager.ListEdgeIndexByLabel(label);
}

std::vector<CompositeIndexSpec> Transaction::ListVertexCompositeIndexByLabel(
                                const std::string& label) {
    return curr_schema_->v_schema_manager.ListVertexCompositeIndexByLabel(label);
}

std::vector<std::tuple<bool, std::string, std::string>> Transaction::ListFullTextIndexes() {
    std::vector<std::tuple<bool, std::string, std::string>> ret;
    const auto& v_labels = curr_schema_->v_schema_manager.GetAllLabels();
    for (const auto& label : v_labels) {
        auto schema = curr_schema_->v_schema_manager.GetSchema(label);
        FMA_ASSERT(schema);
        const auto& ft_fields = schema->GetFullTextFields();
        for (auto field : ft_fields) {
            ret.emplace_back(true, label, schema->GetFieldExtractor(field)->Name());
        }
    }
    const auto& e_labels = curr_schema_->e_schema_manager.GetAllLabels();
    for (const auto& label : e_labels) {
        auto schema = curr_schema_->e_schema_manager.GetSchema(label);
        FMA_ASSERT(schema);
        const auto& ft_fields = schema->GetFullTextFields();
        for (auto field : ft_fields) {
            ret.emplace_back(false, label, schema->GetFieldExtractor(field)->Name());
        }
    }
    return ret;
}

std::vector<std::tuple<bool, std::string, int64_t>> Transaction::countDetail() {
    std::vector<std::tuple<bool, std::string, int64_t>> ret;
    const auto& v_labels = curr_schema_->v_schema_manager.GetAllLabels();
    for (const auto& label : v_labels) {
        auto schema = curr_schema_->v_schema_manager.GetSchema(label);
        ret.emplace_back(true, label, graph_->GetCount(*txn_, true, schema->GetLabelId()));
    }
    const auto& e_labels = curr_schema_->e_schema_manager.GetAllLabels();
    for (const auto& label : e_labels) {
        auto schema = curr_schema_->e_schema_manager.GetSchema(label);
        ret.emplace_back(false, label, graph_->GetCount(*txn_, false, schema->GetLabelId()));
    }
    return ret;
}

VectorIndex* Transaction::GetVertexVectorIndex(const std::string& label, const std::string& field) {
    auto s = curr_schema_->v_schema_manager.GetSchema(label);
    if (!s) {
        THROW_CODE(LabelNotExist, "No such vertex label:{}", label);
    }
    auto f = s->GetFieldExtractor(field);
    auto ret = f->GetVectorIndex();
    if (!ret) {
        THROW_CODE(VectorIndexException, "No such vertex vector index, {}:{}", label, field);
    }
    return ret;
}

/**
 * Check if index is ready.
 *
 * \param   label   The label of index.
 * \param   field   The field of index.
 *
 * \return  True if the index is ready.
 */

bool Transaction::IsIndexed(const std::string& label, const std::string& field) {
    const Schema* s = curr_schema_->v_schema_manager.GetSchema(label);
    if (!s) return false;
    auto fe = s->TryGetFieldExtractor(field);
    if (!fe) return false;
    auto index = fe->GetVertexIndex();
    return (index && index->IsReady());
}

void Transaction::EnterTxn() {
    if (LightningGraph::InTransaction()) {
        THROW_CODE(InternalError,
            "Nested transaction is forbidden. "
            "Note that db.AddLabel/AddVertexIndex should NOT be used inside a "
            "transaction.");
    }
    LightningGraph::InTransaction() = true;
}

void Transaction::LeaveTxn() {
    FMA_DBG_ASSERT(LightningGraph::InTransaction());
    LightningGraph::InTransaction() = false;
}

Transaction::Transaction(bool read_only, bool optimistic, LightningGraph* db)
    : read_only_(read_only),
      db_(db),
      managed_schema_ptr_(db->schema_.GetScopedRef()),
      graph_(db->graph_.get()),
      index_manager_(db->index_manager_.get()),
      blob_manager_(db->blob_manager_.get()),
      fulltext_index_(db->fulltext_index_.get()) {
    EnterTxn();
    if (read_only) {
        txn_ = db->store_->CreateReadTxn();
    } else {
        txn_ = db->store_->CreateWriteTxn(optimistic);
    }
    curr_schema_ = managed_schema_ptr_.Get();
}

Transaction::Transaction(LightningGraph* db, KvTransaction& txn)
    : txn_(txn.Fork()),
      read_only_(true),
      db_(db),
      managed_schema_ptr_(db->schema_.GetScopedRef()),
      graph_(db->graph_.get()),
      index_manager_(db->index_manager_.get()),
      blob_manager_(db->blob_manager_.get()),
      fulltext_index_(db->fulltext_index_.get()) {
    EnterTxn();
    curr_schema_ = managed_schema_ptr_.Get();
}

typedef BlobManager::BlobKey BlobKey;

template <typename FT>
inline FieldData GetField(const Schema* s, const Value& v, const FT& field, BlobManager* bm,
                          KvTransaction& txn) {
    return s->GetField(v, field, [&](const BlobKey& bk) { return bm->Get(txn, bk); });
}

void DeleteBlobs(const Value& prop, Schema* schema, BlobManager* bm, KvTransaction& txn) {
    // delete blobs
    for (size_t i = 0; i < schema->GetNumFields(); i++) {
        auto fe = schema->GetFieldExtractor(i);
        if (fe->Type() == FieldType::BLOB) {
            Value v = fe->GetConstRef(prop);
            if (BlobManager::IsLargeBlob(v)) {
                // delete blob
                bm->Delete(txn, BlobManager::GetLargeBlobKey(v));
            }
        }
    }
}

Transaction::Transaction(Transaction&& rhs)
    : txn_(std::move(rhs.txn_)),
      read_only_(rhs.read_only_),
      db_(rhs.db_),
      managed_schema_ptr_(std::move(rhs.managed_schema_ptr_)),
      curr_schema_(rhs.curr_schema_),
      graph_(rhs.graph_),
      index_manager_(rhs.index_manager_),
      blob_manager_(rhs.blob_manager_),
      fulltext_index_(rhs.fulltext_index_),
      fulltext_buffers_(std::move(rhs.fulltext_buffers_)),
      vertex_delta_count_(std::move(rhs.vertex_delta_count_)),
      edge_delta_count_(std::move(rhs.edge_delta_count_)) {
    // Non-empty transactions should not be moved.
    FMA_DBG_ASSERT(rhs.iterators_.empty());
    rhs.read_only_ = true;
}

Transaction& Transaction::operator=(Transaction&& rhs) {
    if (this == &rhs) return *this;
    // Non-empty transactions should not be moved.
    FMA_DBG_ASSERT(rhs.iterators_.empty());
    if (IsValid()) Abort();
    read_only_ = rhs.read_only_;
    rhs.read_only_ = true;
    txn_ = std::move(rhs.txn_);
    db_ = rhs.db_;
    rhs.db_ = nullptr;
    managed_schema_ptr_ = std::move(rhs.managed_schema_ptr_);
    curr_schema_ = rhs.curr_schema_;
    graph_ = std::move(rhs.graph_);
    index_manager_ = rhs.index_manager_;
    blob_manager_ = rhs.blob_manager_;
    fulltext_index_ = rhs.fulltext_index_;
    fulltext_buffers_ = std::move(rhs.fulltext_buffers_);
    vertex_delta_count_ = std::move(rhs.vertex_delta_count_);
    edge_delta_count_ = std::move(rhs.edge_delta_count_);
    return *this;
}

Transaction::~Transaction() { Abort(); }

std::vector<std::pair<std::string, FieldData>> Transaction::GetVertexFields(
    const VertexIterator& it) {
    Value prop = it.GetProperty();
    FMA_DBG_ASSERT(prop.IsSlice());
    auto schema = curr_schema_->v_schema_manager.GetSchema(prop);
    FMA_DBG_ASSERT(schema);
    if (schema->DetachProperty()) {
        prop = schema->GetDetachedVertexProperty(*txn_, it.GetId());
    }
    std::vector<std::pair<std::string, FieldData>> values;
    for (size_t i = 0; i < schema->GetNumFields(); i++) {
        auto fe = schema->GetFieldExtractor(i);
        if (fe->IsDeleted()) continue;
        values.emplace_back(
            fe->Name(), GetField(schema, prop, i, blob_manager_, *txn_));
    }
    return values;
}

/**
 * Gets field identifiers
 *
 * \param   is_vertex   True to indicate this is vertex label, otherwise edge
 * label. \param   label_id    Identifier for the label. \param   field_names
 * List of names of the fields.
 *
 * \return  The field identifiers.
 */

/**
 * Gets field identifier
 *
 * \param   is_vertex   True to indicate this is vertex label, otherwise edge
 * label. \param   label       The label. \param   field       The field.
 *
 * \return  The field identifier.
 */

size_t Transaction::GetFieldId(bool is_vertex, const std::string& label, const std::string& field) {
    SchemaManager& sm = is_vertex ? curr_schema_->v_schema_manager : curr_schema_->e_schema_manager;
    const Schema* schema = sm.GetSchema(label);
    if (!schema) THROW_CODE(InputError, "Label with id={} does not exist.", label);
    return schema->GetFieldId(field);
}

std::vector<size_t> Transaction::GetFieldIds(bool is_vertex, size_t label_id,
                                             const std::vector<std::string>& field_names) {
    std::vector<size_t> fids(field_names.size());
    SchemaManager& sm = is_vertex ? curr_schema_->v_schema_manager : curr_schema_->e_schema_manager;
    const Schema* schema = sm.GetSchema(label_id);
    if (!schema) THROW_CODE(InputError, "Label with id={} does not exist.", label_id);
    for (size_t i = 0; i < field_names.size(); i++) {
        fids[i] = schema->GetFieldId(field_names[i]);
    }
    return fids;
}

size_t Transaction::GetFieldId(bool is_vertex, size_t label_id, const std::string& field_name) {
    SchemaManager& sm = is_vertex ? curr_schema_->v_schema_manager : curr_schema_->e_schema_manager;
    const Schema* schema = sm.GetSchema(label_id);
    if (!schema) THROW_CODE(InputError, "Label with id={} does not exist.", label_id);
    return schema->GetFieldId(field_name);
}

/**
 * List out edges from src
 *
 * \param   src     The vid.
 *
 * \return  A vector of [dst, eid] pairs.
 */
std::vector<EdgeUid> Transaction::ListOutEdges(VertexId src) {
    _detail::CheckVid(src);
    std::vector<EdgeUid> edges;
    auto eit = graph_->GetUnmanagedOutEdgeIterator(txn_.get(), EdgeUid(src, 0, 0, 0, 0), true);
    while (eit.IsValid()) {
        edges.emplace_back(eit.GetUid());
        eit.Next();
    }
    return edges;
}

/**
 * List in-coming edges of dst.
 *
 * \param   dst     The vid.
 *
 * \return  A vector of [src, eid] pairs.
 */
std::vector<EdgeUid> Transaction::ListInEdges(VertexId dst) {
    _detail::CheckVid(dst);
    std::vector<EdgeUid> edges;
    auto eit = graph_->GetUnmanagedInEdgeIterator(txn_.get(), EdgeUid(0, dst, 0, 0, 0), true);
    while (eit.IsValid()) {
        edges.emplace_back(eit.GetUid());
        eit.Next();
    }
    return edges;
}

void Transaction::CommitFullTextIndex() {
    if (!fulltext_index_) {
        return;
    }
    for (const auto& entry : fulltext_buffers_) {
        switch (entry.type) {
        case FTIndexEntryType::ADD_VERTEX:
            fulltext_index_->AddVertex(entry.vid1, entry.lid, entry.kvs);
            break;
        case FTIndexEntryType::DELETE_VERTEX:
            fulltext_index_->DeleteVertex(entry.vid1);
            break;
        case FTIndexEntryType::ADD_EDGE:
            fulltext_index_->AddEdge({entry.vid1, entry.vid2, entry.lid, entry.tid, entry.eid},
                                     entry.kvs);
            break;
        case FTIndexEntryType::DELETE_EDGE:
            fulltext_index_->DeleteEdge({entry.vid1, entry.vid2, entry.lid, entry.tid, entry.eid});
            break;
        default:
            break;
        }
    }
    if (!fulltext_buffers_.empty()) {
        if (fulltext_index_->commit_interval() <= 0) {
            fulltext_index_->Commit();
        }
        if (fulltext_index_->refresh_interval() <= 0) {
            fulltext_index_->Refresh();
        }
        fulltext_buffers_.clear();
    }
}

void Transaction::Commit() {
    if (!IsValid()) return;
    CloseAllIterators();
    if (db_->GetConfig().enable_realtime_count && !txn_->IsOptimistic()) {
        for (const auto& pair : vertex_delta_count_) {
            graph_->IncreaseCount(*txn_, true, pair.first, pair.second);
        }
        for (const auto& pair : edge_delta_count_) {
            graph_->IncreaseCount(*txn_, false, pair.first, pair.second);
        }
        for (LabelId id : vertex_label_delete_) {
            graph_->DeleteCount(*txn_, true, id);
        }
        for (LabelId id : edge_label_delete_) {
            graph_->DeleteCount(*txn_, false, id);
        }
        vertex_delta_count_.clear();
        edge_delta_count_.clear();
        vertex_label_delete_.clear();
        edge_label_delete_.clear();
    }
    txn_->Commit();
    txn_.reset();
    if (fulltext_index_) {
        CommitFullTextIndex();
    }
    managed_schema_ptr_.Release();
    LeaveTxn();
    if (!read_only_) {
        read_only_ = true;
    }
}

void Transaction::Abort() {
    if (!IsValid()) return;
    CloseAllIterators();
    txn_->Abort();
    txn_.reset();
    managed_schema_ptr_.Release();
    LeaveTxn();
    if (!read_only_) {
        read_only_ = true;
    }
}

void Transaction::DeleteVertex(graph::VertexIterator& it, size_t* n_in, size_t* n_out) {
    if (n_in) *n_in = 0;
    if (n_out) *n_out = 0;
    // check if there is blob
    Value prop = it.GetProperty();
    FMA_DBG_ASSERT(prop.IsSlice());
    auto vid = it.GetId();
    auto schema = curr_schema_->v_schema_manager.GetSchema(prop);
    if (schema->DetachProperty()) {
        prop = schema->GetDetachedVertexProperty(*txn_, vid);
    }
    if (schema->HasBlob()) DeleteBlobs(prop, schema, blob_manager_, *txn_);
    schema->DeleteVertexIndex(*txn_, vid, prop);
    schema->DeleteVertexCompositeIndex(*txn_, vid, prop);
    schema->DeleteVectorIndex(*txn_, vid, prop);
    auto on_edge_deleted = [&](bool is_out_edge, const graph::EdgeValue& edge_value){
        if (is_out_edge) {
            if (n_out) {
                *n_out += edge_value.GetEdgeCount();
            }
        } else {
            if (n_in) {
                *n_in += edge_value.GetEdgeCount();
            }
        }

        for (size_t i = 0; i < edge_value.GetEdgeCount(); i++) {
            const auto& data = edge_value.GetNthEdgeData(i);
            auto edge_schema = curr_schema_->e_schema_manager.GetSchema(data.lid);
            FMA_ASSERT(edge_schema);
            if (is_out_edge) {
                Value property(data.prop, data.psize);
                if (edge_schema->DetachProperty()) {
                    property = edge_schema->GetDetachedEdgeProperty(
                        *txn_, {vid, data.vid, data.lid, data.tid, data.eid});
                }
                EdgeUid euid{vid, data.vid, data.lid, data.tid, data.eid};
                edge_schema->DeleteEdgeIndex(*txn_, euid, property);
                if (edge_schema->DetachProperty()) {
                    edge_schema->DeleteDetachedEdgeProperty(*txn_, euid);
                }
                edge_delta_count_[data.lid]--;
                if (fulltext_index_) {
                    edge_schema->DeleteEdgeFullTextIndex(
                        {vid, data.vid, data.lid, data.tid, data.eid}, fulltext_buffers_);
                }
            } else {
                if (vid == data.vid) {
                    // The in edge directing to self is already included in the out edges
                    // skip to avoid double deleting
                    continue;
                }
                Value property(data.prop, data.psize);
                if (edge_schema->DetachProperty()) {
                    property = edge_schema->GetDetachedEdgeProperty(
                        *txn_, {data.vid, vid, data.lid, data.tid, data.eid});
                }
                EdgeUid euid{data.vid, vid, data.lid, data.tid, data.eid};
                edge_schema->DeleteEdgeIndex(*txn_, euid, property);
                if (edge_schema->DetachProperty()) {
                    edge_schema->DeleteDetachedEdgeProperty(*txn_, euid);
                }
                edge_delta_count_[data.lid]--;
                if (fulltext_index_) {
                    edge_schema->DeleteEdgeFullTextIndex(
                        {data.vid, vid, data.lid, data.tid, data.eid}, fulltext_buffers_);
                }
            }
        }
    };
    graph_->DeleteVertex(*txn_, it, on_edge_deleted);
    if (schema->DetachProperty()) {
        schema->DeleteDetachedVertexProperty(*txn_, vid);
    }
    vertex_delta_count_[schema->GetLabelId()]--;
    // delete vertex fulltext index
    if (fulltext_index_) {
        schema->DeleteVertexFullTextIndex(vid, fulltext_buffers_);
    }
}

/**
 * Deletes the vertex specified by vid.
 *
 * \param           vid     The vid.
 * \param [in,out]  n_in    Returns the number of in-coming edges.
 * \param [in,out]  n_out   Returns the number of out-going edges.
 *
 * \return  True if it succeeds, false if the vertex does not exist.
 */

bool Transaction::DeleteVertex(VertexId vid, size_t* n_in, size_t* n_out) {
    _detail::CheckVid(vid);
    auto vit = graph_->GetVertexIterator(this, vid);
    if (!vit.IsValid()) return false;
    DeleteVertex(vit, n_in, n_out);
    return true;
}

ENABLE_IF_EIT(EIT, void)
Transaction::DeleteEdge(EIT& eit) {
    ThrowIfReadOnlyTxn();
    Value prop = eit.GetProperty();
    FMA_DBG_ASSERT(prop.IsSlice());
    auto euid = eit.GetUid();
    auto schema = curr_schema_->e_schema_manager.GetSchema(euid.lid);
    if (schema->DetachProperty()) {
        prop = schema->GetDetachedEdgeProperty(*txn_, euid);
    }
    if (schema->HasBlob()) DeleteBlobs(prop, schema, blob_manager_, *txn_);
    schema->DeleteEdgeIndex(*txn_, eit.GetUid(), prop);
    graph_->DeleteEdge(*txn_, eit);
    if (schema->DetachProperty()) {
        schema->DeleteDetachedEdgeProperty(*txn_, euid);
    }
    edge_delta_count_[euid.lid]--;
    if (fulltext_index_) {
        schema->DeleteEdgeFullTextIndex(euid, fulltext_buffers_);
    }
}

bool Transaction::DeleteEdge(const EdgeUid& uid) {
    _detail::CheckEdgeUid(uid);
    ThrowIfReadOnlyTxn();
    auto eit = GetOutEdgeIterator(uid, false);
    if (!eit.IsValid()) return false;
    DeleteEdge(eit);
    return true;
}

template void Transaction::DeleteEdge<graph::InEdgeIterator>(graph::InEdgeIterator&);
template void Transaction::DeleteEdge<graph::OutEdgeIterator>(graph::OutEdgeIterator&);

void Transaction::GetStartAndEndVid(VertexId& start, VertexId& end) {
    auto vit = graph_->GetUnmanagedVertexIterator(txn_.get());
    if (!vit.IsValid()) {
        start = 0;
        end = 0;
    } else {
        start = vit.GetId();
        end = graph_->GetNextVid(*txn_);
    }
}

VertexIndex* Transaction::GetVertexIndex(const std::string& label, const std::string& field) {
    const Schema* s = curr_schema_->v_schema_manager.GetSchema(label);
    if (!s) THROW_CODE(InputError, "Label \"{}\" does not exist.", label);
    auto fe = s->GetFieldExtractor(field);
    return fe->GetVertexIndex();
}

VertexIndex* Transaction::GetVertexIndex(size_t label, size_t field) {
    const Schema* s = curr_schema_->v_schema_manager.GetSchema(label);
    if (!s) THROW_CODE(InputError, "Label \"{}\" does not exist.", label);
    auto fe = s->GetFieldExtractor(field);
    return fe->GetVertexIndex();
}

EdgeIndex* Transaction::GetEdgeIndex(const std::string& label, const std::string& field) {
    const Schema* s = curr_schema_->e_schema_manager.GetSchema(label);
    if (!s) THROW_CODE(InputError, "Label \"{}\" does not exist.", label);
    auto fe = s->GetFieldExtractor(field);
    return fe->GetEdgeIndex();
}

EdgeIndex* Transaction::GetEdgeIndex(size_t label, size_t field) {
    const Schema* s = curr_schema_->e_schema_manager.GetSchema(label);
    if (!s) THROW_CODE(InputError, "Label \"{}\" does not exist.", label);
    auto fe = s->GetFieldExtractor(field);
    return fe->GetEdgeIndex();
}

CompositeIndex* Transaction::GetVertexCompositeIndex(const std::string& label,
                                                     const std::vector<std::string>& fields) {
    Schema* s = curr_schema_->v_schema_manager.GetSchema(label);
    if (!s) THROW_CODE(InputError, "Label \"{}\" does not exist.", label);
    return s->GetCompositeIndex(fields);
}

CompositeIndex* Transaction::GetVertexCompositeIndex(const size_t& label,
                                                     const std::vector<size_t>& field_ids) {
    Schema* s = curr_schema_->v_schema_manager.GetSchema(label);
    if (!s) THROW_CODE(InputError, "Label \"{}\" does not exist.", label);
    return s->GetCompositeIndex(field_ids);
}

VertexIndexIterator Transaction::GetVertexIndexIterator(const std::string& label,
                                                        const std::string& field,
                                                        const FieldData& key_start,
                                                        const FieldData& key_end) {
    VertexIndex* index = GetVertexIndex(label, field);
    if (!index || !index->IsReady()) {
        THROW_CODE(InputError, "VertexIndex is not created for {}:{}", label, field);
    }
    Value ks, ke;
    if (!key_start.IsNull()) {
        ks = field_data_helper::FieldDataToValueOfFieldType(key_start, index->KeyType());
    }
    if (!key_end.IsNull()) {
        ke = field_data_helper::FieldDataToValueOfFieldType(key_end, index->KeyType());
    }
    return index->GetIterator(this, std::move(ks), std::move(ke), 0);
}

VertexIndexIterator Transaction::GetVertexIndexIterator(size_t label_id, size_t field_id,
                                                        const FieldData& key_start,
                                                        const FieldData& key_end) {
    VertexIndex* index = GetVertexIndex(label_id, field_id);
    if (!index || !index->IsReady()) {
        THROW_CODE(InputError, "VertexIndex is not created for this field");
    }
    Value ks, ke;
    if (!key_start.IsNull()) {
        ks = field_data_helper::FieldDataToValueOfFieldType(key_start, index->KeyType());
    }
    if (!key_end.IsNull()) {
        ke = field_data_helper::FieldDataToValueOfFieldType(key_end, index->KeyType());
    }
    return index->GetIterator(this, std::move(ks), std::move(ke), 0);
}

VertexIndexIterator Transaction::GetVertexIndexIterator(const std::string& label,
                                                        const std::string& field,
                                                        const std::string& key_start = "",
                                                        const std::string& key_end = "") {
    VertexIndex* index = GetVertexIndex(label, field);
    if (!index || !index->IsReady()) {
        THROW_CODE(InputError, "VertexIndex is not created for {}:{}", label, field);
    }
    Value ks, ke;
    if (!key_start.empty()) {
        ks = field_data_helper::ParseStringToValueOfFieldType(key_start, index->KeyType());
    }
    if (!key_end.empty()) {
        ke = field_data_helper::ParseStringToValueOfFieldType(key_end, index->KeyType());
    }
    return index->GetIterator(this, std::move(ks), std::move(ke), 0);
}

EdgeIndexIterator Transaction::GetEdgeIndexIterator(const std::string& label,
                                                    const std::string& field,
                                                    const FieldData& key_start,
                                                    const FieldData& key_end) {
    EdgeIndex* index = GetEdgeIndex(label, field);
    if (!index || !index->IsReady()) {
        THROW_CODE(InputError, "EdgeIndex is not created for {}:{}", label, field);
    }
    Value ks, ke;
    if (!key_start.IsNull()) {
        ks = field_data_helper::FieldDataToValueOfFieldType(key_start, index->KeyType());
    }
    if (!key_end.IsNull()) {
        ke = field_data_helper::FieldDataToValueOfFieldType(key_end, index->KeyType());
    }
    return index->GetIterator(this, std::move(ks), std::move(ke), 0);
}

EdgeIndexIterator Transaction::GetEdgeIndexIterator(size_t label_id, size_t field_id,
                                                    const FieldData& key_start,
                                                    const FieldData& key_end) {
    EdgeIndex* index = GetEdgeIndex(label_id, field_id);
    if (!index || !index->IsReady()) {
        THROW_CODE(InputError, "EdgeIndex is not created for this field");
    }
    Value ks, ke;
    if (!key_start.IsNull()) {
        ks = field_data_helper::FieldDataToValueOfFieldType(key_start, index->KeyType());
    }
    if (!key_end.IsNull()) {
        ke = field_data_helper::FieldDataToValueOfFieldType(key_end, index->KeyType());
    }
    return index->GetIterator(this, std::move(ks), std::move(ke), 0);
}

EdgeIndexIterator Transaction::GetEdgePairUniqueIndexIterator(
    size_t label_id, size_t field_id, VertexId src_vid, VertexId dst_vid,
    const FieldData& key_start, const FieldData& key_end) {
    EdgeIndex* index = GetEdgeIndex(label_id, field_id);
    if (!index || !index->IsReady() || index->GetType() != IndexType::PairUniqueIndex) {
        THROW_CODE(InputError, "Edge pair unique index is not created for this field");
    }
    Value ks, ke;
    if (!key_start.IsNull()) {
        ks = field_data_helper::FieldDataToValueOfFieldType(key_start, index->KeyType());
    }
    if (!key_end.IsNull()) {
        ke = field_data_helper::FieldDataToValueOfFieldType(key_end, index->KeyType());
    }
    return index->GetIterator(this, std::move(ks), std::move(ke), src_vid, dst_vid);
}

EdgeIndexIterator Transaction::GetEdgeIndexIterator(const std::string& label,
                                                    const std::string& field,
                                                    const std::string& key_start = "",
                                                    const std::string& key_end = "") {
    EdgeIndex* index = GetEdgeIndex(label, field);
    if (!index || !index->IsReady()) {
        THROW_CODE(InputError, "EdgeIndex is not created for {}:{}", label, field);
    }
    Value ks, ke;
    if (!key_start.empty()) {
        ks = field_data_helper::ParseStringToValueOfFieldType(key_start, index->KeyType());
    }
    if (!key_end.empty()) {
        ke = field_data_helper::ParseStringToValueOfFieldType(key_end, index->KeyType());
    }
    return index->GetIterator(this, std::move(ks), std::move(ke), 0);
}

CompositeIndexIterator Transaction::GetVertexCompositeIndexIterator(const std::string& label,
                                    const std::vector<std::string>& fields,
                                    const std::vector<FieldData>& key_start,
                                    const std::vector<FieldData>& key_end) {
    std::string fields_name = curr_schema_->v_schema_manager.GetSchema(label)
                                  ->GetCompositeIndexMapKey(fields);
    CompositeIndex* index = GetVertexCompositeIndex(label, fields);
    if (!index || !index->IsReady()) {
        THROW_CODE(InputError, "VertexIndex is not created for {}:{}", label, fields_name);
    }
    std::vector<Value> key_start_values, key_end_values;
    int num = fields.size();
    if (!key_start.empty()) {
        for (int i = 0; i < num; ++i) {
            key_start_values.push_back(field_data_helper::FieldDataToValueOfFieldType(
                key_start[i], index->key_types[i]));
        }
    }
    if (!key_end.empty()) {
        for (int i = 0; i < num; ++i) {
            key_end_values.push_back(field_data_helper::FieldDataToValueOfFieldType(
                key_end[i], index->key_types[i]));
        }
    }
    return index->GetIterator(this,
                              composite_index_helper::GenerateCompositeIndexKey(key_start_values),
                              composite_index_helper::GenerateCompositeIndexKey(key_end_values));
}

CompositeIndexIterator Transaction::GetVertexCompositeIndexIterator(const size_t& label_id,
                                    const std::vector<size_t>& field_ids,
                                    const std::vector<FieldData>& key_start,
                                    const std::vector<FieldData>& key_end) {
    std::string fields_name = curr_schema_->v_schema_manager.GetSchema(label_id)
                                  ->GetCompositeIndexMapKey(field_ids);
    CompositeIndex* index = GetVertexCompositeIndex(label_id, field_ids);
    if (!index || !index->IsReady()) {
        THROW_CODE(InputError, "VertexIndex is not created for {}:{}", label_id, fields_name);
    }
    std::vector<Value> key_start_values, key_end_values;
    int num = field_ids.size();
    if (!key_start.empty()) {
        for (int i = 0; i < num; ++i) {
            key_start_values.push_back(field_data_helper::FieldDataToValueOfFieldType(
                key_start[i], index->key_types[i]));
        }
    }
    if (!key_end.empty()) {
        for (int i = 0; i < num; ++i) {
            key_end_values.push_back(field_data_helper::FieldDataToValueOfFieldType(
                key_end[i], index->key_types[i]));
        }
    }
    return index->GetIterator(this,
                              composite_index_helper::GenerateCompositeIndexKey(key_start_values),
                              composite_index_helper::GenerateCompositeIndexKey(key_end_values));
}

CompositeIndexIterator Transaction::GetVertexCompositeIndexIterator(const std::string& label,
                                    const std::vector<std::string>& fields,
                                    const std::vector<std::string>& key_start,
                                    const std::vector<std::string>& key_end) {
    std::string fields_name = curr_schema_->v_schema_manager.GetSchema(label)
                                  ->GetCompositeIndexMapKey(fields);
    CompositeIndex* index = GetVertexCompositeIndex(label, fields);
    if (!index || !index->IsReady()) {
        THROW_CODE(InputError, "VertexIndex is not created for {}:{}", label, fields_name);
    }
    std::vector<Value> key_start_values, key_end_values;
    int num = fields.size();
    if (!key_start.empty()) {
        for (int i = 0; i < num; ++i) {
            key_start_values.push_back(field_data_helper::ParseStringToValueOfFieldType(
                key_start[i], index->key_types[i]));
        }
    }
    if (!key_end.empty()) {
        for (int i = 0; i < num; ++i) {
            key_end_values.push_back(field_data_helper::ParseStringToValueOfFieldType(
                key_end[i], index->key_types[i]));
        }
    }
    return index->GetIterator(this,
                  composite_index_helper::GenerateCompositeIndexKey(key_start_values),
                  composite_index_helper::GenerateCompositeIndexKey(key_end_values));
}

std::string Transaction::VertexToString(const VertexIterator& vit) {
    Value prop = vit.GetProperty();
    FMA_DBG_ASSERT(prop.IsSlice());
    auto schema = curr_schema_->v_schema_manager.GetSchema(prop);
    FMA_DBG_ASSERT(schema);
    if (schema->DetachProperty()) {
        prop = schema->GetDetachedVertexProperty(*txn_, vit.GetId());
    }
    std::string line;
    fma_common::StringFormatter::Append(
        line, "V[{}]:{} {}\n", vit.GetId(), schema->GetLabel(),
        curr_schema_->v_schema_manager.DumpRecord(prop));
    for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
        auto s = curr_schema_->e_schema_manager.GetSchema(eit.GetLabelId());
        FMA_DBG_ASSERT(s);
        auto edge_prop = eit.GetProperty();
        FMA_DBG_ASSERT(edge_prop.IsSlice());
        if (s->DetachProperty()) {
            edge_prop = s->GetDetachedEdgeProperty(*txn_, eit.GetUid());
        }
        fma_common::StringFormatter::Append(
            line, "\t -> E[{}]:{}, DST = {}, EP = {}", eit.GetEdgeId(), s->GetLabel(), eit.GetDst(),
            curr_schema_->e_schema_manager.DumpRecord(edge_prop, eit.GetLabelId()));
    }
    line.append("\n\t <- [");
    for (auto iit = vit.GetInEdgeIterator(); iit.IsValid(); iit.Next()) {
        fma_common::StringFormatter::Append(line, "{} ", iit.GetSrc());
    }
    line.append("]");
    return line;
}

void Transaction::ImportAppendDataRaw(const Value& key, const Value& value) {
    graph_->AppendDataRaw(*txn_, key, value);
}

void Transaction::RefreshNextVid() { graph_->RefreshNextVid(*txn_); }

BlobManager::BlobKey Transaction::_GetNextBlobKey() { return blob_manager_->GetNextBlobKey(*txn_); }

std::string Transaction::_OnlineImportBatchAddVertexes(
    Schema* schema, const std::vector<Value>& vprops,
    const std::vector<std::pair<BlobManager::BlobKey, Value>>& blobs, bool continue_on_error) {
    std::string error;
    int64_t count = 0;
    for (auto& v : vprops) {
        if (v.Empty()) {
            continue;
        }
        if (schema->VertexUniqueIndexConflict(*txn_, v)) {
            if (continue_on_error) {
                continue;
            } else {
                THROW_CODE(InputError, "vertex unique index conflict");
            }
        }
        // add the new vertex without index
        VertexId newvid;
        if (schema->DetachProperty()) {
            newvid = graph_->AddVertex(*txn_, schema->CreateEmptyRecord());
            schema->AddDetachedVertexProperty(*txn_, newvid, v);
        } else {
            newvid = graph_->AddVertex(*txn_, v);
        }
        // do index
        std::vector<size_t> created_index;
        schema->AddVertexToIndex(*txn_, newvid, v, created_index);
        count++;
        // add fulltext index
        if (fulltext_index_) {
            schema->AddVertexToFullTextIndex(newvid, v, fulltext_buffers_);
        }
    }
    vertex_delta_count_[schema->GetLabelId()] += count;
    blob_manager_->_BatchAddBlobs(*txn_, blobs);
    return error;
}

std::string Transaction::_OnlineImportBatchAddEdges(
    const std::vector<EdgeDataForTheSameVertex>& data,
    const std::vector<std::pair<BlobManager::BlobKey, Value>>& blobs, bool continue_on_error,
    Schema* schema) {
    std::string error;
    auto it = graph_->_GetKvTable().GetIterator(*txn_);
    std::function<void(int64_t, int64_t, uint16_t, int64_t, int64_t, const Value&)>
        add_fulltext_index;

    int64_t count = 0;
    auto extra_work = [&](const EdgeUid& euid, const Value& record) {
        std::vector<size_t> created_index;
        schema->AddEdgeToIndex(*txn_, euid, record, created_index);
        if (fulltext_index_) {
            schema->AddEdgeToFullTextIndex(euid, record, fulltext_buffers_);
        }
        if (schema->DetachProperty()) {
            schema->AddDetachedEdgeProperty(*txn_, euid, record);
        }
        ++count;
    };

    for (auto& v : data) {
        graph::Graph::OutIteratorImpl::InsertEdges(v.vid, v.outs.begin(), v.outs.end(), *it,
                                                   extra_work, schema->DetachProperty());
        graph::Graph::InIteratorImpl::InsertEdges(v.vid, v.ins.begin(), v.ins.end(), *it,
                                                  extra_work, schema->DetachProperty());
    }
    edge_delta_count_[schema->GetLabelId()] += count;
    blob_manager_->_BatchAddBlobs(*txn_, blobs);
    return error;
}

void Transaction::_BatchAddBlobs(const std::vector<std::pair<BlobManager::BlobKey, Value>>& blobs) {
    blob_manager_->_BatchAddBlobs(*txn_, blobs);
}

/**
 * Sets vertex property (exclude label)
 *
 * \tparam  FieldT  Type of the field specifier, can be size_t or std::string.
 * \tparam  DataT   Type of the data, can be std::string or FieldData.
 *
 * \param [in,out]  it          The vertex iterator.
 * \param           n_fields    The number of fields.
 * \param           fields      The field specifiers.
 * \param           values      The values.
 */
template <typename FieldT, typename DataT>
typename std::enable_if<IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT), void>::type
Transaction::SetVertexProperty(VertexIterator& it, size_t n_fields, const FieldT* fields,
                               const DataT* values) {
    if (n_fields == 0) {
        return;
    }
    ThrowIfReadOnlyTxn();
    Value old_prop = it.GetProperty();
    FMA_DBG_ASSERT(old_prop.IsSlice());
    VertexId vid = it.GetId();
    Schema* schema = curr_schema_->v_schema_manager.GetSchema(old_prop);
    FMA_DBG_ASSERT(schema);
    if (schema->DetachProperty()) {
        old_prop = schema->GetDetachedVertexProperty(*txn_, vid);
    }
    Value new_prop;
    new_prop.Copy(old_prop);
    for (size_t i = 0; i < n_fields; i++) {
        // TODO: use SetField like SetEdgeProperty // NOLINT
        _detail::FieldExtractorBase* fe = schema->GetFieldExtractor(fields[i]);
        if (fe->Type() == FieldType::BLOB) {
            Value oldv = fe->GetConstRef(new_prop);
            if (BlobManager::IsLargeBlob(oldv)) {
                BlobKey bk = BlobManager::GetLargeBlobKey(oldv);
                blob_manager_->Delete(*txn_, bk);
            }
            schema->ParseAndSetBlob(
                new_prop, values[i], [&](const Value& v) { return blob_manager_->Add(*txn_, v); },
                fe);
        } else if (fe->Type() == FieldType::FLOAT_VECTOR) {
            schema->ParseAndSet(new_prop, values[i], fe);
            VectorIndex* index = fe->GetVectorIndex();
            if (index) {
                auto old_v = fe->GetConstRef(old_prop);
                auto new_v = fe->GetConstRef(new_prop);
                std::vector<int64_t> vids {vid};
                if (!old_v.Empty() && !new_v.Empty()) {
                    if (old_v == new_v) {
                        continue;
                    }
                    // delete
                    index->Remove(vids);
                    // add
                    auto dim = index->GetVecDimension();
                    std::vector<std::vector<float>> floatvector;
                    floatvector.emplace_back(new_v.AsFloatVector());
                    if (floatvector.back().size() != (size_t)dim) {
                        THROW_CODE(InputError,
                                   "vector index dimension mismatch, vector size:{}, dim:{}",
                                   floatvector.back().size(), dim);
                    }
                    index->Add(floatvector, vids);
                } else if (old_v.Empty() && !new_v.Empty()) {
                    // add
                    auto dim = index->GetVecDimension();
                    std::vector<std::vector<float>> floatvector;
                    floatvector.emplace_back(new_v.AsFloatVector());
                    if (floatvector.back().size() != (size_t)dim) {
                        THROW_CODE(InputError,
                                   "vector index dimension mismatch, vector size:{}, dim:{}",
                                   floatvector.back().size(), dim);
                    }
                    index->Add(floatvector, vids);
                } else if (!old_v.Empty() && new_v.Empty()) {
                    // delete
                    index->Remove(vids);
                }
            }
        } else {
            schema->ParseAndSet(new_prop, values[i], fe);
            // update index if there is no error
            VertexIndex* index = fe->GetVertexIndex();
            if (index && index->IsReady()) {
                auto old_v = fe->GetConstRef(old_prop);
                auto new_v = fe->GetConstRef(new_prop);
                if (!old_v.Empty() && !new_v.Empty()) {
                    // update
                    if (old_v == new_v) {
                        // If the values are equal, there is no need to update the index.
                        continue;
                    }
                    bool r = index->Update(*txn_, old_v, new_v, vid);
                    if (!r)
                        THROW_CODE(InputError,
                            "failed to update vertex index, {}:[{}] already exists", fe->Name(),
                                                 fe->FieldToString(new_prop));
                } else if (old_v.Empty() && !new_v.Empty()) {
                    // set to non-null, add index
                    bool r = index->Add(*txn_, new_v, vid);
                    if (!r)
                        THROW_CODE(InputError,
                            "failed to add vertex index, {}:[{}] already exists", fe->Name(),
                                                 fe->FieldToString(new_prop));
                } else if (!old_v.Empty() && new_v.Empty()) {
                    // set to null, delete index
                    bool r = index->Delete(*txn_, old_v, vid);
                    FMA_DBG_ASSERT(r);
                } else {
                    // both null, nothing to do
                }
            }
        }
    }
    if (fulltext_index_) {
        // fulltext
        schema->DeleteVertexFullTextIndex(vid, fulltext_buffers_);
        schema->AddVertexToFullTextIndex(vid, new_prop, fulltext_buffers_);
    }
    if (schema->DetachProperty()) {
        schema->SetDetachedVertexProperty(*txn_, vid, new_prop);
    } else {
        it.SetProperty(std::move(new_prop));
    }
}

template <typename FieldT>
typename std::enable_if<IS_FIELD_TYPE(FieldT), FieldData>::type Transaction::GetVertexField(
    const VertexIterator& it, const FieldT& fd) {
    Value prop = it.GetProperty();
    FMA_DBG_ASSERT(prop.IsSlice());
    auto schema = curr_schema_->v_schema_manager.GetSchema(prop);
    FMA_DBG_ASSERT(schema);
    if (schema->DetachProperty()) {
        prop = schema->GetDetachedVertexProperty(*txn_, it.GetId());
    }
    return GetField(schema, prop, fd, blob_manager_, *txn_);
}

/**
 * Gets vertex fields implementation
 *
 * \tparam  FieldT  Type of the field specifier, can be std::string or size_t.
 * \param [in,out]  it          The iterator.
 * \param           n_fields    The fields.
 * \param           fds         The fds.
 * \param [in,out]  fields      If non-null, the fields.
 */
template <typename FieldT>
typename std::enable_if<IS_FIELD_TYPE(FieldT), void>::type Transaction::GetVertexFields(
    const graph::VertexIterator& it, size_t n_fields, const FieldT* fds, FieldData* fields) {
    Value prop = it.GetProperty();
    FMA_DBG_ASSERT(prop.IsSlice());
    auto schema = curr_schema_->v_schema_manager.GetSchema(prop);
    FMA_DBG_ASSERT(schema);
    if (schema->DetachProperty()) {
        prop = schema->GetDetachedVertexProperty(*txn_, it.GetId());
    }
    for (size_t i = 0; i < n_fields; i++) {
        fields[i] = GetField(schema, prop, fds[i], blob_manager_, *txn_);
    }
}

/**
 * Gets edge fields
 *
 * \tparam  E   Edge iterator type, InEdgeIterator or OutEdgeIterator.
 * \tparam  T   Field specifier type, std::string or size_t.
 * \param [in,out]  it          The iterator.
 * \param           n_fields    The fields.
 * \param           fds         The fds.
 * \param [in,out]  fields      If non-null, the fields.
 */
template <typename E, typename T>
typename std::enable_if<IS_EIT_TYPE(E) && IS_FIELD_TYPE(T), void>::type Transaction::GetEdgeFields(
    const E& it, size_t n_fields, const T* fds, FieldData* fields) {
    Value prop = it.GetProperty();
    FMA_DBG_ASSERT(prop.IsSlice());
    auto schema = curr_schema_->e_schema_manager.GetSchema(it.GetLabelId());
    FMA_DBG_ASSERT(schema);
    if (schema->DetachProperty()) {
        prop = schema->GetDetachedEdgeProperty(*txn_, it.GetUid());
    }
    for (size_t i = 0; i < n_fields; i++) {
        fields[i] = GetField(schema, prop, fds[i], blob_manager_, *txn_);
    }
}

// get all the fields of the edge
template <typename EIT>
typename std::enable_if<IS_EIT_TYPE(EIT), std::vector<std::pair<std::string, FieldData>>>::type
Transaction::GetEdgeFields(const EIT& it) {
    Value prop = it.GetProperty();
    FMA_DBG_ASSERT(prop.IsSlice());
    auto schema = curr_schema_->e_schema_manager.GetSchema(it.GetLabelId());
    FMA_DBG_ASSERT(schema);
    if (schema->DetachProperty()) {
        prop = schema->GetDetachedEdgeProperty(*txn_, it.GetUid());
    }
    std::vector<std::pair<std::string, FieldData>> values;
    for (size_t i = 0; i < schema->GetNumFields(); i++) {
        auto fe = schema->GetFieldExtractor(i);
        values.emplace_back(fe->Name(), GetField(schema, prop, i, blob_manager_, *txn_));
    }
    return values;
}

/**
 * Gets edge field
 *
 * \tparam  E   Edge iterator type, InEdgeIterator or OutEdgeIterator.
 * \tparam  T   Field specifier type, std::string or size_t.
 * \param [in,out]  it  The iterator.
 * \param           fd  The fd.
 *
 * \return  The edge field.
 */
template <typename E, typename T>
typename std::enable_if<IS_EIT_TYPE(E) && IS_FIELD_TYPE(T), FieldData>::type
Transaction::GetEdgeField(const E& it, const T& fd) {
    Value prop = it.GetProperty();
    FMA_DBG_ASSERT(prop.IsSlice());
    auto schema = curr_schema_->e_schema_manager.GetSchema(it.GetLabelId());
    FMA_DBG_ASSERT(schema);
    if (schema->DetachProperty()) {
        prop = schema->GetDetachedEdgeProperty(*txn_, it.GetUid());
    }
    return GetField(schema, prop, fd, blob_manager_, *txn_);
}

/**
 * Sets edge property
 *
 * \tparam  EIT     Type of the edge iterator, InEdgeIterator or
 * OutEdgeIterator. \tparam  FieldT  Type of the field specifier, std::string
 * or size_t. \tparam  DataT   Type of the data t, std::string or FieldData
 * \param [in,out]  it          The iterator.
 * \param           n_fields    The fields.
 * \param           fields      The fields.
 * \param           values      The values.
 */
template <typename EIT, typename FieldT, typename DataT>
typename std::enable_if<IS_EIT_TYPE(EIT) && IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT),
                        void>::type
Transaction::SetEdgeProperty(EIT& it, size_t n_fields, const FieldT* fields, const DataT* values) {
    if (n_fields == 0) {
        return;
    }
    ThrowIfReadOnlyTxn();
    auto euid = it.GetUid();
    Value old_prop = it.GetProperty();
    FMA_DBG_ASSERT(old_prop.IsSlice());
    auto schema = curr_schema_->e_schema_manager.GetSchema(it.GetLabelId());
    FMA_DBG_ASSERT(schema);
    if (schema->DetachProperty()) {
        old_prop = schema->GetDetachedEdgeProperty(*txn_, euid);
    }
    Value new_prop;
    new_prop.Copy(old_prop);
    for (size_t i = 0; i < n_fields; i++) {
        auto fe = schema->GetFieldExtractor(fields[i]);
        if (fe->Type() == FieldType::BLOB) {
            Value oldv = fe->GetConstRef(new_prop);
            if (BlobManager::IsLargeBlob(oldv)) {
                // existing blob is large, replace it
                BlobKey bk = BlobManager::GetLargeBlobKey(oldv);
                blob_manager_->Delete(*txn_, bk);
            }
            schema->ParseAndSetBlob(
                new_prop, values[i], [&](const Value& v) { return blob_manager_->Add(*txn_, v); },
                fe);
        } else {
            schema->ParseAndSet(new_prop, values[i], fe);
            // update index if there is no error
            EdgeIndex* index = fe->GetEdgeIndex();
            if (index && index->IsReady()) {
                bool oldnull = fe->GetIsNull(old_prop);
                bool newnull = fe->GetIsNull(new_prop);
                if (!oldnull && !newnull) {
                    const auto& old_v = fe->GetConstRef(old_prop);
                    const auto& new_v = fe->GetConstRef(new_prop);
                    if (old_v == new_v) {
                        // If the values are equal, there is no need to update the index.
                        continue;
                    }
                    // update
                    bool r = index->Update(*txn_, old_v, new_v, euid);
                    if (!r)
                        THROW_CODE(InputError,
                            "failed to update edge index, {}:[{}] already exists", fe->Name(),
                                                 fe->FieldToString(new_prop));
                } else if (oldnull && !newnull) {
                    // set to non-null, add index
                    bool r = index->Add(*txn_, fe->GetConstRef(new_prop), euid);
                    if (!r)
                        THROW_CODE(InputError,
                            "failed to add edge index, {}:[{}] already exists", fe->Name(),
                                                 fe->FieldToString(new_prop));
                } else if (!oldnull && newnull) {
                    // set to null, delete index
                    bool r = index->Delete(*txn_, fe->GetConstRef(old_prop), euid);
                    FMA_DBG_ASSERT(r);
                } else {
                    // both null, nothing to do
                }
            }
        }
    }
    if (schema->DetachProperty()) {
        schema->SetDetachedEdgeProperty(*txn_, euid, new_prop);
    } else {
        it.SetProperty(new_prop);
        auto ieit = graph_->GetUnmanagedInEdgeIterator(txn_.get(), it.GetUid(), false);
        FMA_DBG_ASSERT(ieit.IsValid());
        ieit.SetProperty(new_prop);
        it.RefreshContentIfKvIteratorModified();
    }
    if (fulltext_index_) {
        // fulltext
        schema->DeleteEdgeFullTextIndex(it.GetUid(), fulltext_buffers_);
        schema->AddEdgeToFullTextIndex(it.GetUid(), new_prop, fulltext_buffers_);
    }
}

/**
 * Gets vertex field
 *
 * \tparam  FieldT  Type of the field specifier, std::string or size_t.
 * \param   vid The vid.
 * \param   fd  The fd.
 *
 * \return  The vertex field.
 */
template <typename FieldT>
typename std::enable_if<IS_FIELD_TYPE(FieldT), FieldData>::type Transaction::GetVertexField(
    VertexId vid, const FieldT& fd) {
    _detail::CheckVid(vid);
    auto vit = graph_->GetUnmanagedVertexIterator(txn_.get(), vid);
    if (!vit.IsValid()) THROW_CODE(InputError, "Vertex {} does not exist.", vid);
    Value prop = vit.GetProperty();
    FMA_DBG_ASSERT(prop.IsSlice());
    auto schema = curr_schema_->v_schema_manager.GetSchema(prop);
    FMA_DBG_ASSERT(schema);
    if (schema->DetachProperty()) {
        prop = schema->GetDetachedVertexProperty(*txn_, vid);
    }
    return GetField(schema, prop, fd, blob_manager_, *txn_);
}

/**
 * Gets vertex fields
 *
 * \tparam  FieldT  Type of the field specifier, std::string or size_t.
 * \param   vid The vid.
 * \param   fds The fds.
 *
 * \return  A vector of vertex fields.
 */
template <typename FieldT>
typename std::enable_if<IS_FIELD_TYPE(FieldT), std::vector<FieldData>>::type
Transaction::GetVertexFields(VertexId vid, const std::vector<FieldT>& fds) {
    _detail::CheckVid(vid);
    std::vector<FieldData> fields(fds.size());
    auto vit = graph_->GetUnmanagedVertexIterator(txn_.get(), vid);
    if (!vit.IsValid()) THROW_CODE(InputError, "Vertex {} does not exist.", vid);
    Value prop = vit.GetProperty();
    FMA_DBG_ASSERT(prop.IsSlice());
    auto schema = curr_schema_->v_schema_manager.GetSchema(prop);
    FMA_DBG_ASSERT(schema);
    if (schema->DetachProperty()) {
        prop = schema->GetDetachedVertexProperty(*txn_, vid);
    }
    for (size_t i = 0; i < fds.size(); i++) {
        fields[i] = GetField(schema, prop, fds[i], blob_manager_, *txn_);
    }
    return fields;
}

/**
 * Gets edge field
 *
 * \tparam  FieldT  Type of the field specifier, std::string or size_t.
 * \param   src Source for the.
 * \param   dst Destination for the.
 * \param   eid The eid.
 * \param   fd  The fd.
 *
 * \return  The edge field.
 */
template <typename FieldT>
typename std::enable_if<IS_FIELD_TYPE(FieldT), FieldData>::type Transaction::GetEdgeField(
    const EdgeUid& uid, const FieldT& fd) {
    _detail::CheckEdgeUid(uid);
    auto eit = graph_->GetUnmanagedOutEdgeIterator(txn_.get(), uid, false);
    if (!eit.IsValid()) THROW_CODE(InputError, "Edge does not exist");
    auto schema = curr_schema_->e_schema_manager.GetSchema(eit.GetLabelId());
    FMA_DBG_ASSERT(schema);
    Value prop = eit.GetProperty();
    FMA_DBG_ASSERT(prop.IsSlice());
    if (schema->DetachProperty()) {
        prop = schema->GetDetachedEdgeProperty(*txn_, uid);
    }
    return GetField(schema, prop, fd, blob_manager_, *txn_);
}

template <typename FieldT>
typename std::enable_if<IS_FIELD_TYPE(FieldT), void>::type Transaction::GetEdgeFields(
    const EdgeUid& uid, const size_t n_fields, const FieldT* fds, FieldData* fields) {
    _detail::CheckEdgeUid(uid);
    auto eit = graph_->GetUnmanagedOutEdgeIterator(txn_.get(), uid, false);
    if (!eit.IsValid()) THROW_CODE(InputError, "Edge does not exist");
    auto schema = curr_schema_->e_schema_manager.GetSchema(eit.GetLabelId());
    FMA_DBG_ASSERT(schema);
    Value prop = eit.GetProperty();
    FMA_DBG_ASSERT(prop.IsSlice());
    if (schema->DetachProperty()) {
        prop = schema->GetDetachedEdgeProperty(*txn_, eit.GetUid());
    }
    for (size_t i = 0; i < n_fields; i++) {
        fields[i] = GetField(schema, prop, fds[i], blob_manager_, *txn_);
    }
}

template <typename LabelT, typename FieldT, typename DataT>
typename std::enable_if<IS_LABEL_TYPE(LabelT) && IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT),
                        VertexId>::type
Transaction::AddVertex(const LabelT& label, size_t n_fields, const FieldT* fields,
                       const DataT* values) {
    ThrowIfReadOnlyTxn();
    Schema* schema = curr_schema_->v_schema_manager.GetSchema(label);
    if (!schema) THROW_CODE(InputError, "Vertex label {} does not exist.", label);
    Value prop = schema->HasBlob()
                     ? schema->CreateRecordWithBlobs(
                           n_fields, fields, values,
                           [this](const Value& blob) {
                               return blob_manager_->Add(*txn_, blob); })
                     : schema->CreateRecord(n_fields, fields, values);
    VertexId newvid = graph_->AddVertex(
        *txn_, schema->DetachProperty() ? schema->CreateRecordWithLabelId() : prop);
    std::vector<size_t> created_index;
    std::vector<std::string> created_composite_index;
    schema->AddVertexToIndex(*txn_, newvid, prop, created_index);
    schema->AddVertexToCompositeIndex(*txn_, newvid, prop, created_composite_index);
    schema->AddVectorToVectorIndex(*txn_, newvid, prop);
    if (schema->DetachProperty()) {
        schema->AddDetachedVertexProperty(*txn_, newvid, prop);
    }
    if (fulltext_index_) {
        schema->AddVertexToFullTextIndex(newvid, prop, fulltext_buffers_);
    }
    vertex_delta_count_[schema->GetLabelId()]++;
    return newvid;
}

TemporalId Transaction::ParseTemporalId(const FieldData& fd,
                                        const TemporalFieldOrder& temporal_order) {
    if (temporal_order == TemporalFieldOrder::ASC) {
        return fd.AsInt64();
    } else {
        return std::numeric_limits<int64_t>::max() ^ fd.AsInt64();
    }
}

TemporalId Transaction::ParseTemporalId(const std::string& str,
                                        const TemporalFieldOrder& temporal_order) {
    TemporalId tid = 0;
    if (fma_common::TextParserUtils::ParseT(str, tid) != str.size())
        THROW_CODE(InputError, "Incorrect tid format: {}", str);
    if (temporal_order == TemporalFieldOrder::ASC) {
        return tid;
    } else {
        return std::numeric_limits<int64_t>::max() ^ tid;
    }
}

/**
 * Adds an edge
 *
 * \tparam  LabelT  Type of the label t.
 * \tparam  FieldT  Type of the field t.
 * \tparam  DataT   Type of the data t.
 * \param   src         Source for the.
 * \param   dst         Destination for the.
 * \param   label       The label.
 * \param   n_fields    The fields.
 * \param   fields      The fields.
 * \param   values      The values.
 *
 * \return  EdgeId of the new edge.
 */
template <typename LabelT, typename FieldT, typename DataT>
typename std::enable_if<IS_LABEL_TYPE(LabelT) && IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT),
                        EdgeUid>::type
Transaction::AddEdge(VertexId src, VertexId dst, const LabelT& label, size_t n_fields,
                     const FieldT* fields, const DataT* values) {
    _detail::CheckVid(src);
    _detail::CheckVid(dst);
    ThrowIfReadOnlyTxn();
    auto schema = curr_schema_->e_schema_manager.GetSchema(label);
    if (!schema) THROW_CODE(InputError, "Edge label {} does not exist", label);
    Value prop = schema->HasBlob()
                     ? schema->CreateRecordWithBlobs(
                           n_fields, fields, values,
                           [this](const Value& blob) { return blob_manager_->Add(*txn_, blob); })
                     : schema->CreateRecord(n_fields, fields, values);
    TemporalId tid = 0;
    if (schema->HasTemporalField()) {
        int pos = schema->GetTemporalPos(n_fields, fields);
        if (pos != -1) {
            tid = ParseTemporalId(values[pos], schema->GetTemporalOrder());
        }
    }
    const auto& constraints = schema->GetEdgeConstraintsLids();
    EdgeUid euid = graph_->AddEdge(
        *txn_, EdgeSid(src, dst, schema->GetLabelId(), tid),
        schema->DetachProperty() ? Value() : prop, constraints);
    if (schema->DetachProperty()) {
        schema->AddDetachedEdgeProperty(*txn_, euid, prop);
    }
    std::vector<size_t> created_index;
    schema->AddEdgeToIndex(*txn_, euid, prop, created_index);
    if (fulltext_index_) {
        schema->AddEdgeToFullTextIndex(euid, prop, fulltext_buffers_);
    }
    edge_delta_count_[schema->GetLabelId()]++;
    return euid;
}

template <typename LabelT, typename FieldT, typename DataT>
typename std::enable_if<IS_LABEL_TYPE(LabelT) && IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT),
                        bool>::type
Transaction::UpsertEdge(VertexId src, VertexId dst, const LabelT& label, size_t n_fields,
                        const FieldT* fields, const DataT* values) {
    _detail::CheckVid(src);
    _detail::CheckVid(dst);
    ThrowIfReadOnlyTxn();
    auto lid = GetLabelId(false, label);
    TemporalId tid = 0;
    auto schema = curr_schema_->e_schema_manager.GetSchema(label);
    if (!schema) THROW_CODE(InputError, "Edge label {} does not exist", label);
    if (schema->HasTemporalField()) {
        // NOTE: if one edge has primary id, different primary id will be insert rather than update.
        int pos = schema->GetTemporalPos(n_fields, fields);
        if (pos != -1) {
            tid = ParseTemporalId(values[pos], schema->GetTemporalOrder());
        }
    }
    graph::OutEdgeIterator it =
        graph_->GetOutEdgeIterator(this, EdgeUid(src, dst, lid, tid, 0), false);
    if (it.IsValid()) {
        SetEdgeProperty(it, n_fields, fields, values);
        return false;
    } else {
        it.Close();
        AddEdge(src, dst, label, n_fields, fields, values);
        return true;
    }
}

template FieldData Transaction::GetVertexField<size_t>(const VertexIterator& it, const size_t& fd);
template FieldData Transaction::GetVertexField<std::string>(const VertexIterator& it,
                                                            const std::string& fd);

template void Transaction::GetVertexFields<size_t>(const graph::VertexIterator& it, size_t n_fields,
                                                   const size_t* fds, FieldData* fields);
template void Transaction::GetVertexFields<std::string>(const graph::VertexIterator& it,
                                                        size_t n_fields, const std::string* fds,
                                                        FieldData* fields);

template std::vector<FieldData> Transaction::GetVertexFields<size_t>(
    VertexId vid, const std::vector<size_t>& fds);
template std::vector<FieldData> Transaction::GetVertexFields<std::string>(
    VertexId vid, const std::vector<std::string>& fds);

template std::vector<std::pair<std::string, FieldData>>
Transaction::GetEdgeFields<graph::InEdgeIterator>(const graph::InEdgeIterator& it);
template std::vector<std::pair<std::string, FieldData>>
Transaction::GetEdgeFields<graph::OutEdgeIterator>(const graph::OutEdgeIterator& it);

template FieldData Transaction::GetVertexField<size_t>(VertexId vid, const size_t& fd);
template FieldData Transaction::GetVertexField<std::string>(VertexId vid, const std::string& fd);

template FieldData Transaction::GetEdgeField<size_t>(const EdgeUid& uid, const size_t& fd);
template FieldData Transaction::GetEdgeField<std::string>(const EdgeUid& uid,
                                                          const std::string& fd);

template void Transaction::GetEdgeFields<size_t>(const EdgeUid& uid, const size_t n_fields,
                                                 const size_t* fds, FieldData* fields);
template void Transaction::GetEdgeFields<std::string>(const EdgeUid& uid, const size_t n_fields,
                                                      const std::string* fds, FieldData* fields);

#define _SET_VERTEX_PROPERTY_FUNC(FT, DT)                                                      \
    template void Transaction::SetVertexProperty<FT, DT>(VertexIterator & it, size_t n_fields, \
                                                         const FT* fields, const DT* values);
#define _GET_EDGE_FIELDS_FUNC(T1, T2)                                                              \
    template void Transaction::GetEdgeFields<T1, T2>(const T1& it, size_t n_fields, const T2* fds, \
                                                     FieldData* fields);
#define _GET_EDGE_FIELD_FUNC(T1, T2) \
    template FieldData Transaction::GetEdgeField<T1, T2>(const T1& it, const T2& fd);
#define _SET_EDGE_PROPERTY_FUNC(ET, FT, DT)                                          \
    template void Transaction::SetEdgeProperty<ET, FT, DT>(ET & it, size_t n_fields, \
                                                           const FT* fields, const DT* values);
#define _ADD_VERTEX_FUNC(LT, FT, DT)                                                       \
    template VertexId Transaction::AddVertex<LT, FT, DT>(const LT& label, size_t n_fields, \
                                                         const FT* fields, const DT* values);
#define _ADD_EDGE_FUNC(LT, FT, DT)                                                                 \
    template EdgeUid Transaction::AddEdge<LT, FT, DT>(VertexId src, VertexId dst, const LT& label, \
                                                      size_t n_fields, const FT* fields,           \
                                                      const DT* values);

#define _UPSERT_EDGE_FUNC(LT, FT, DT)                                                              \
    template bool Transaction::UpsertEdge<LT, FT, DT>(VertexId src, VertexId dst, const LT& label, \
                                                      size_t n_fields, const FT* fields,           \
                                                      const DT* values);

#define EXPAND(x) x
#define PAREN(...) (__VA_ARGS__)
#define EXPAND_F(m, ...) EXPAND(m PAREN(__VA_ARGS__))

#define _SET_ONE(F, a1, a2) EXPAND_F(F, a1) EXPAND_F(F, a2)
#define _SET_MT_ONE(F, a1, a2, ...) EXPAND_F(F, a1, __VA_ARGS__) EXPAND_F(F, a2, __VA_ARGS__)
#define _SET_TWO(F, a1, a2, b1, b2) _SET_MT_ONE(F, a1, a2, b1) _SET_MT_ONE(F, a1, a2, b2)
#define _SET_MT_TWO(F, a1, a2, b1, b2, ...) \
    _SET_MT_ONE(F, a1, a2, b1, __VA_ARGS__) _SET_MT_ONE(F, a1, a2, b2, __VA_ARGS__)
#define _SET_THREE(F, a1, a2, b1, b2, c1, c2) \
    _SET_MT_TWO(F, a1, a2, b1, b2, c1) _SET_MT_TWO(F, a1, a2, b1, b2, c2)
#define _SET_MT_THREE(F, a1, a2, b1, b2, c1, c2, ...) \
    _SET_MT_TWO(F, a1, a2, b1, b2, c1, __VA_ARGS__) _SET_MT_TWO(F, a1, a2, b1, b2, c2, __VA_ARGS__)
#define _SET_FOUR(F, a1, a2, b1, b2, c1, c2, d1, d2) \
    _SET_MT_THREE(F, a1, a2, b1, b2, c1, c2, d1) _SET_MT_THREE(F, a1, a2, b1, b2, c1, c2, d2)

#define _INSTANTIATE_FT_DT(F) _SET_TWO(F, size_t, std::string, FieldData, std::string)
#define _INSTANTIATE_ET_FT(F) \
    _SET_TWO(F, graph::InEdgeIterator, graph::OutEdgeIterator, size_t, std::string)
#define _INSTANTIATE_ET_FT_DT(F)                                                                 \
    _SET_THREE(F, graph::InEdgeIterator, graph::OutEdgeIterator, size_t, std::string, FieldData, \
               std::string)
#define _INSTANTIATE_LT_FT_DT(F) \
    _SET_THREE(F, size_t, std::string, size_t, std::string, FieldData, std::string)

_INSTANTIATE_FT_DT(_SET_VERTEX_PROPERTY_FUNC);
_INSTANTIATE_ET_FT(_GET_EDGE_FIELDS_FUNC);
_INSTANTIATE_ET_FT(_GET_EDGE_FIELD_FUNC);
_INSTANTIATE_ET_FT_DT(_SET_EDGE_PROPERTY_FUNC);
_INSTANTIATE_LT_FT_DT(_ADD_VERTEX_FUNC);
_INSTANTIATE_LT_FT_DT(_ADD_EDGE_FUNC);
_INSTANTIATE_LT_FT_DT(_UPSERT_EDGE_FUNC);
}  // namespace lgraph
