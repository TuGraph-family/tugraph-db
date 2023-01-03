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

#include <stdexcept>

#include "core/graph_vertex_iterator.h"
#include "core/kv_store.h"
#include "core/transaction.h"

#include "lgraph/lgraph_edge_iterator.h"
#include "lgraph/lgraph_vertex_iterator.h"

namespace lgraph_api {
#define ThrowIfInvalid()                                                        \
    do {                                                                        \
        if (!txn_->IsValid()) throw std::runtime_error("Invalid transaction."); \
        if (!it_->IsValid()) throw std::runtime_error("Invalid iterator.");     \
    } while (0)

#define RefreshAndReturn(stmt)    \
    do {                          \
        auto r = stmt;            \
        txn_->RefreshIterators(); \
        return r;                 \
    } while (0)

VertexIterator::VertexIterator(lgraph::graph::VertexIterator&& impl,
                               const std::shared_ptr<lgraph::Transaction>& txn)
    : it_(new lgraph::graph::VertexIterator(std::move(impl))), txn_(txn) {}

VertexIterator::VertexIterator(VertexIterator&& rhs)
    : it_(std::move(rhs.it_)), txn_(std::move(rhs.txn_)) {}

VertexIterator& VertexIterator::operator=(VertexIterator&& rhs) {
    it_ = std::move(rhs.it_);
    txn_ = std::move(rhs.txn_);
    return *this;
}

VertexIterator::~VertexIterator() {}

void VertexIterator::Close() { it_->Close(); }

bool VertexIterator::Next() {
    ThrowIfInvalid();
    return it_->Next();
}

bool VertexIterator::Goto(int64_t vid, bool nearest) {
    if (!txn_->IsValid()) throw std::runtime_error("Invalid transaction.");
    return it_->Goto(vid, nearest);
}

int64_t VertexIterator::GetId() const {
    ThrowIfInvalid();
    return it_->GetId();
}

OutEdgeIterator VertexIterator::GetOutEdgeIterator() const {
    ThrowIfInvalid();
    return OutEdgeIterator(it_->GetOutEdgeIterator(EdgeUid(), true), txn_);
}

OutEdgeIterator VertexIterator::GetOutEdgeIterator(const EdgeUid& euid, bool nearest) const {
    ThrowIfInvalid();
    return OutEdgeIterator(it_->GetOutEdgeIterator(euid, nearest), txn_);
}

InEdgeIterator VertexIterator::GetInEdgeIterator() const {
    ThrowIfInvalid();
    return InEdgeIterator(it_->GetInEdgeIterator(EdgeUid(), true), txn_);
}

InEdgeIterator VertexIterator::GetInEdgeIterator(const EdgeUid& euid, bool nearest) const {
    ThrowIfInvalid();
    return InEdgeIterator(it_->GetInEdgeIterator(euid, nearest), txn_);
}

bool VertexIterator::IsValid() const { return it_->IsValid(); }

const std::string& VertexIterator::GetLabel() const {
    ThrowIfInvalid();
    return txn_->GetVertexLabel(*it_);
}

size_t VertexIterator::GetLabelId() const {
    ThrowIfInvalid();
    return txn_->GetVertexLabelId(*it_);
}

FieldData VertexIterator::GetField(const std::string& field_name) const {
    ThrowIfInvalid();
    return txn_->GetVertexField(*it_, field_name);
}

std::vector<FieldData> VertexIterator::GetFields(
    const std::vector<std::string>& field_names) const {
    ThrowIfInvalid();
    return txn_->GetVertexFields(*it_, field_names);
}

std::vector<FieldData> VertexIterator::GetFields(const std::vector<size_t>& field_ids) const {
    ThrowIfInvalid();
    return txn_->GetVertexFields(*it_, field_ids);
}

FieldData VertexIterator::GetField(size_t field_id) const {
    ThrowIfInvalid();
    return txn_->GetVertexField(*it_, field_id);
}

std::map<std::string, FieldData> VertexIterator::GetAllFields() const {
    ThrowIfInvalid();
    auto props = txn_->GetVertexFields(*it_);
    return std::map<std::string, FieldData>(props.begin(), props.end());
}

void VertexIterator::SetField(const std::string& field_name, const FieldData& field_value) {
    ThrowIfInvalid();
    std::vector<std::string> field_names = {field_name};
    std::vector<FieldData> field_values = {field_value};
    txn_->SetVertexProperty(*it_, field_names, field_values);
    txn_->RefreshIterators();
}

void VertexIterator::SetField(size_t field_id, const FieldData& field_value) {
    ThrowIfInvalid();
    std::vector<size_t> field_ids = {field_id};
    std::vector<FieldData> field_values = {field_value};
    txn_->SetVertexProperty(*it_, field_ids, field_values);
    txn_->RefreshIterators();
}

void VertexIterator::SetFields(const std::vector<std::string>& field_names,
                               const std::vector<std::string>& field_value_strings) {
    ThrowIfInvalid();
    txn_->SetVertexProperty(*it_, field_names, field_value_strings);
    txn_->RefreshIterators();
}

void VertexIterator::SetFields(const std::vector<std::string>& field_names,
                               const std::vector<FieldData>& field_values) {
    ThrowIfInvalid();
    txn_->SetVertexProperty(*it_, field_names, field_values);
    txn_->RefreshIterators();
}

void VertexIterator::SetFields(const std::vector<size_t>& field_ids,
                               const std::vector<FieldData>& field_values) {
    ThrowIfInvalid();
    txn_->SetVertexProperty(*it_, field_ids, field_values);
    txn_->RefreshIterators();
}

std::vector<int64_t> VertexIterator::ListSrcVids(size_t n_limit, bool* more_to_go) {
    ThrowIfInvalid();
    return it_->ListSrcVids(nullptr, nullptr, n_limit, more_to_go);
}

std::vector<int64_t> VertexIterator::ListDstVids(size_t n_limit, bool* more_to_go) {
    ThrowIfInvalid();
    return it_->ListDstVids(nullptr, nullptr, n_limit, more_to_go);
}

size_t VertexIterator::GetNumInEdges(size_t n_limit, bool* more_to_go) {
    ThrowIfInvalid();
    return it_->GetNumInEdges(n_limit, more_to_go);
}

size_t VertexIterator::GetNumOutEdges(size_t n_limit, bool* more_to_go) {
    ThrowIfInvalid();
    return it_->GetNumOutEdges(n_limit, more_to_go);
}

void VertexIterator::Delete(size_t* n_in, size_t* n_out) {
    ThrowIfInvalid();
    txn_->DeleteVertex(*it_, n_in, n_out);
    txn_->RefreshIterators();
}

std::string VertexIterator::ToString() const {
    ThrowIfInvalid();
    return txn_->VertexToString(*it_);
}
}  // namespace lgraph_api
