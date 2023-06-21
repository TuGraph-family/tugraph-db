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

#include "core/kv_store.h"
#include "core/transaction.h"

#include "lgraph/lgraph_txn.h"
#include "lgraph/lgraph_edge_iterator.h"
#include "lgraph/lgraph_vertex_index_iterator.h"
#include "lgraph/lgraph_edge_index_iterator.h"
#include "lgraph/lgraph_vertex_iterator.h"

namespace lgraph_api {
#define ThrowIfInvalid()                                                        \
    do {                                                                        \
        if (!txn_->IsValid()) throw std::runtime_error("Invalid transaction."); \
    } while (0)

#define RefreshAndReturn(stmt)    \
    do {                          \
        auto r = stmt;            \
        txn_->RefreshIterators(); \
        return r;                 \
    } while (0)

Transaction::Transaction(lgraph::Transaction&& impl)
    : txn_(new lgraph::Transaction(std::move(impl))) {}

VertexIterator Transaction::GetVertexIterator() {
    ThrowIfInvalid();
    return VertexIterator(txn_->GetVertexIterator(), txn_);
}

void Transaction::Commit() { txn_->Commit(); }

void Transaction::Abort() { txn_->Abort(); }

bool Transaction::IsValid() const { return txn_->IsValid(); }

const std::shared_ptr<lgraph::Transaction> Transaction::GetTxn() { return txn_; }

bool Transaction::IsReadOnly() const { return txn_->IsReadOnly(); }

VertexIterator Transaction::GetVertexIterator(int64_t vid, bool nearest) {
    ThrowIfInvalid();
    return VertexIterator(txn_->GetVertexIterator(vid, nearest), txn_);
}

OutEdgeIterator Transaction::GetOutEdgeIterator(const EdgeUid& euid, bool nearest) {
    ThrowIfInvalid();
    return OutEdgeIterator(txn_->GetOutEdgeIterator(euid, nearest), txn_);
}

OutEdgeIterator Transaction::GetOutEdgeIterator(const int64_t src, const int64_t dst,
                                                const int16_t lid) {
    ThrowIfInvalid();
    auto it = OutEdgeIterator(txn_->GetOutEdgeIterator(EdgeUid(src, 0, lid, 0, 0), true), txn_);
    while (it.IsValid()) {
        if (it.GetLabelId() != lid) {
            // set iterator invalid
            it.Close();
            break;
        }
        if (it.GetDst() == dst) break;
        it.Next();
    }
    return it;
}

InEdgeIterator Transaction::GetInEdgeIterator(const EdgeUid& euid, bool nearest) {
    ThrowIfInvalid();
    return InEdgeIterator(txn_->GetInEdgeIterator(euid, nearest), txn_);
}

InEdgeIterator Transaction::GetInEdgeIterator(const int64_t src, const int64_t dst,
                                              const int16_t lid) {
    ThrowIfInvalid();
    auto it = InEdgeIterator(txn_->GetInEdgeIterator(EdgeUid(0, dst, lid, 0, 0), true), txn_);
    while (it.IsValid()) {
        if (it.GetLabelId() != lid) {
            // set iterator invalid
            it.Close();
            break;
        }
        if (it.GetSrc() == src) break;
        it.Next();
    }
    return it;
}

size_t Transaction::GetNumVertexLabels() {
    ThrowIfInvalid();
    return txn_->GetNumLabels(true);
}

size_t Transaction::GetNumEdgeLabels() {
    ThrowIfInvalid();
    return txn_->GetNumLabels(false);
}

std::vector<std::string> Transaction::ListVertexLabels() {
    ThrowIfInvalid();
    return txn_->GetAllLabels(true);
}

std::vector<std::string> Transaction::ListEdgeLabels() {
    ThrowIfInvalid();
    return txn_->GetAllLabels(false);
}

size_t Transaction::GetVertexLabelId(const std::string& label) {
    ThrowIfInvalid();
    return txn_->GetLabelId(true, label);
}

size_t Transaction::GetEdgeLabelId(const std::string& label) {
    ThrowIfInvalid();
    return txn_->GetLabelId(false, label);
}

std::vector<FieldSpec> Transaction::GetVertexSchema(const std::string& label) {
    ThrowIfInvalid();
    return txn_->GetSchema(true, label);
}

std::vector<FieldSpec> Transaction::GetEdgeSchema(const std::string& label) {
    ThrowIfInvalid();
    return txn_->GetSchema(false, label);
}

size_t Transaction::GetVertexFieldId(size_t label_id, const std::string& field_name) {
    ThrowIfInvalid();
    return txn_->GetFieldId(true, label_id, field_name);
}

std::vector<size_t> Transaction::GetVertexFieldIds(size_t label_id,
                                                   const std::vector<std::string>& field_names) {
    ThrowIfInvalid();
    return txn_->GetFieldIds(true, label_id, field_names);
}

size_t Transaction::GetEdgeFieldId(size_t label_id, const std::string& field_name) {
    ThrowIfInvalid();
    return txn_->GetFieldId(false, label_id, field_name);
}

std::vector<size_t> Transaction::GetEdgeFieldIds(size_t label_id,
                                                 const std::vector<std::string>& field_names) {
    ThrowIfInvalid();
    return txn_->GetFieldIds(false, label_id, field_names);
}

int64_t Transaction::AddVertex(const std::string& label_name,
                               const std::vector<std::string>& field_names,
                               const std::vector<std::string>& field_value_strings) {
    ThrowIfInvalid();
    RefreshAndReturn(txn_->AddVertex(label_name, field_names, field_value_strings));
}

int64_t Transaction::AddVertex(const std::string& label_name,
                               const std::vector<std::string>& field_names,
                               const std::vector<FieldData>& field_values) {
    ThrowIfInvalid();
    RefreshAndReturn(txn_->AddVertex(label_name, field_names, field_values));
}

int64_t Transaction::AddVertex(size_t label_id, const std::vector<size_t>& field_ids,
                               const std::vector<FieldData>& field_values) {
    ThrowIfInvalid();
    RefreshAndReturn(txn_->AddVertex(label_id, field_ids, field_values));
}

EdgeUid Transaction::AddEdge(int64_t src, int64_t dst, const std::string& label,
                             const std::vector<std::string>& field_names,
                             const std::vector<std::string>& field_value_strings) {
    ThrowIfInvalid();
    RefreshAndReturn(txn_->AddEdge(src, dst, label, field_names, field_value_strings));
}

EdgeUid Transaction::AddEdge(int64_t src, int64_t dst, const std::string& label,
                             const std::vector<std::string>& field_names,
                             const std::vector<FieldData>& field_values) {
    ThrowIfInvalid();
    RefreshAndReturn(txn_->AddEdge(src, dst, label, field_names, field_values));
}

EdgeUid Transaction::AddEdge(int64_t src, int64_t dst, size_t label_id,
                             const std::vector<size_t>& field_ids,
                             const std::vector<FieldData>& field_values) {
    ThrowIfInvalid();
    RefreshAndReturn(txn_->AddEdge(src, dst, label_id, field_ids, field_values));
}

bool Transaction::UpsertEdge(int64_t src, int64_t dst, const std::string& label,
                             const std::vector<std::string>& field_names,
                             const std::vector<std::string>& field_value_strings) {
    ThrowIfInvalid();
    RefreshAndReturn(txn_->UpsertEdge(src, dst, label, field_names, field_value_strings));
}

bool Transaction::UpsertEdge(int64_t src, int64_t dst, const std::string& label,
                             const std::vector<std::string>& field_names,
                             const std::vector<FieldData>& field_values) {
    ThrowIfInvalid();
    RefreshAndReturn(txn_->UpsertEdge(src, dst, label, field_names, field_values));
}

bool Transaction::UpsertEdge(int64_t src, int64_t dst, size_t label_id,
                             const std::vector<size_t>& field_ids,
                             const std::vector<FieldData>& field_values) {
    ThrowIfInvalid();
    RefreshAndReturn(txn_->UpsertEdge(src, dst, label_id, field_ids, field_values));
}

std::vector<IndexSpec> Transaction::ListVertexIndexes() {
    ThrowIfInvalid();
    return txn_->ListVertexIndexes();
}

std::vector<IndexSpec> Transaction::ListEdgeIndexes() {
    ThrowIfInvalid();
    return txn_->ListEdgeIndexes();
}

VertexIndexIterator Transaction::GetVertexIndexIterator(size_t label_id, size_t field_id,
                                                        const FieldData& key_start,
                                                        const FieldData& key_end) {
    ThrowIfInvalid();
    return VertexIndexIterator(txn_->GetVertexIndexIterator(label_id, field_id, key_start, key_end),
                               txn_);
}

EdgeIndexIterator Transaction::GetEdgeIndexIterator(size_t label_id, size_t field_id,
                                                    const FieldData& key_start,
                                                    const FieldData& key_end) {
    ThrowIfInvalid();
    return EdgeIndexIterator(txn_->GetEdgeIndexIterator(label_id, field_id, key_start, key_end),
                             txn_);
}

VertexIndexIterator Transaction::GetVertexIndexIterator(const std::string& label,
                                                        const std::string& field,
                                                        const FieldData& key_start,
                                                        const FieldData& key_end) {
    ThrowIfInvalid();
    return VertexIndexIterator(txn_->GetVertexIndexIterator(label, field, key_start, key_end),
                               txn_);
}

EdgeIndexIterator Transaction::GetEdgeIndexIterator(const std::string& label,
                                                    const std::string& field,
                                                    const FieldData& key_start,
                                                    const FieldData& key_end) {
    ThrowIfInvalid();
    return EdgeIndexIterator(txn_->GetEdgeIndexIterator(label, field, key_start, key_end), txn_);
}

VertexIndexIterator Transaction::GetVertexIndexIterator(const std::string& label,
                                                        const std::string& field,
                                                        const std::string& key_start,
                                                        const std::string& key_end) {
    ThrowIfInvalid();
    return VertexIndexIterator(txn_->GetVertexIndexIterator(label, field, key_start, key_end),
                               txn_);
}

EdgeIndexIterator Transaction::GetEdgeIndexIterator(const std::string& label,
                                                    const std::string& field,
                                                    const std::string& key_start,
                                                    const std::string& key_end) {
    ThrowIfInvalid();
    return EdgeIndexIterator(txn_->GetEdgeIndexIterator(label, field, key_start, key_end), txn_);
}

bool Transaction::IsVertexIndexed(const std::string& label, const std::string& field) {
    ThrowIfInvalid();
    lgraph::VertexIndex* idx = txn_->GetVertexIndex(label, field);
    if (!idx || !idx->IsReady()) return false;
    return true;
}

bool Transaction::IsEdgeIndexed(const std::string& label, const std::string& field) {
    ThrowIfInvalid();
    lgraph::EdgeIndex* idx = txn_->GetEdgeIndex(label, field);
    if (!idx || !idx->IsReady()) return false;
    return true;
}

VertexIterator Transaction::GetVertexByUniqueIndex(const std::string& label_name,
                                                   const std::string& field_name,
                                                   const std::string& field_value_string) {
    ThrowIfInvalid();
    lgraph::VertexIndexIterator iit = txn_->GetVertexIndexIterator(
        label_name, field_name, field_value_string, field_value_string);
    if (!iit.IsValid()) throw std::runtime_error("No vertex found with specified index value.");
    return VertexIterator(txn_->GetVertexIterator(iit.GetVid()), txn_);
}

OutEdgeIterator Transaction::GetEdgeByUniqueIndex(const std::string& label_name,
                                                  const std::string& field_name,
                                                  const std::string& field_value_string) {
    ThrowIfInvalid();
    lgraph::EdgeIndexIterator eit =
        txn_->GetEdgeIndexIterator(label_name, field_name, field_value_string, field_value_string);
    if (!eit.IsValid()) throw std::runtime_error("No Edge found with specified index value.");
    EdgeUid euid;
    euid = eit.GetUid();
    return GetOutEdgeIterator(euid, false);
}

VertexIterator Transaction::GetVertexByUniqueIndex(const std::string& label_name,
                                                   const std::string& field_name,
                                                   const FieldData& field_value) {
    ThrowIfInvalid();
    lgraph::VertexIndexIterator iit =
        txn_->GetVertexIndexIterator(label_name, field_name, field_value, field_value);
    if (!iit.IsValid()) throw std::runtime_error("No vertex found with specified index value.");
    return VertexIterator(txn_->GetVertexIterator(iit.GetVid()), txn_);
}

OutEdgeIterator Transaction::GetEdgeByUniqueIndex(const std::string& label_name,
                                                  const std::string& field_name,
                                                  const FieldData& field_value) {
    ThrowIfInvalid();
    lgraph::EdgeIndexIterator eit =
        txn_->GetEdgeIndexIterator(label_name, field_name, field_value, field_value);
    if (!eit.IsValid()) throw std::runtime_error("No Edge found with specified index value.");
    EdgeUid euid;
    euid = eit.GetUid();
    return GetOutEdgeIterator(euid, false);
}

VertexIterator Transaction::GetVertexByUniqueIndex(size_t label_id, size_t field_id,
                                                   const FieldData& field_value) {
    ThrowIfInvalid();
    lgraph::VertexIndexIterator iit =
        txn_->GetVertexIndexIterator(label_id, field_id, field_value, field_value);
    if (!iit.IsValid()) throw std::runtime_error("No vertex found with specified index value.");

    return VertexIterator(txn_->GetVertexIterator(iit.GetVid()), txn_);
}

OutEdgeIterator Transaction::GetEdgeByUniqueIndex(size_t label_id, size_t field_id,
                                                  const FieldData& field_value) {
    ThrowIfInvalid();
    lgraph::EdgeIndexIterator eit =
        txn_->GetEdgeIndexIterator(label_id, field_id, field_value, field_value);
    if (!eit.IsValid()) throw std::runtime_error("No Edge found with specified index value.");

    EdgeUid euid;
    euid = eit.GetUid();
    return GetOutEdgeIterator(euid, false);
}
size_t Transaction::GetNumVertices() {
    ThrowIfInvalid();
    return txn_->graph_->GetLooseNumVertex(txn_->GetTxn());
}

const std::string& Transaction::GetVertexPrimaryField(const std::string& label) {
    ThrowIfInvalid();
    return txn_->GetVertexPrimaryField(label);
}

std::pair<uint64_t, uint64_t> Transaction::Count() {
    ThrowIfInvalid();
    const auto& counts = txn_->countDetail();
    uint64_t vertex_num = 0;
    uint64_t edge_num = 0;
    for (const auto& count : counts) {
        if (std::get<0>(count)) {
            vertex_num += std::get<2>(count);
        } else {
            edge_num += std::get<2>(count);
        }
    }
    return {vertex_num, edge_num};
}

std::vector<std::tuple<bool, std::string, int64_t>> Transaction::CountDetail() {
    ThrowIfInvalid();
    return txn_->countDetail();
}

}  // namespace lgraph_api
