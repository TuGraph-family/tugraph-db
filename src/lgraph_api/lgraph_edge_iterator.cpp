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

#include "core/graph_edge_iterator.h"
#include "core/kv_store.h"
#include "core/transaction.h"

#include "lgraph/lgraph_edge_iterator.h"

namespace lgraph_api {
#define ThrowIfInvalid()                                    \
    do {                                                    \
        if (!txn_->IsValid()) THROW_CODE(InvalidTxn);      \
        if (!it_->IsValid())  THROW_CODE(InvalidIterator);  \
    } while (0)

OutEdgeIterator::OutEdgeIterator(lgraph::graph::OutEdgeIterator&& impl,
                                 const std::shared_ptr<lgraph::Transaction>& txn)
    : it_(new lgraph::graph::OutEdgeIterator(std::move(impl))), txn_(txn) {}

OutEdgeIterator::OutEdgeIterator(OutEdgeIterator&& rhs)
    : it_(std::move(rhs.it_)), txn_(std::move(rhs.txn_)) {}

OutEdgeIterator& OutEdgeIterator::operator=(OutEdgeIterator&& rhs) {
    it_ = std::move(rhs.it_);
    txn_ = std::move(rhs.txn_);
    return *this;
}

OutEdgeIterator::~OutEdgeIterator() {}

void OutEdgeIterator::Close() noexcept { it_->Close(); }

bool OutEdgeIterator::Goto(EdgeUid euid, bool nearest) {
    if (!txn_->IsValid()) THROW_CODE(InvalidTxn);
    return it_->Goto(euid, nearest);
}

bool OutEdgeIterator::Next() {
    ThrowIfInvalid();
    return it_->Next();
}

EdgeUid OutEdgeIterator::GetUid() const {
    ThrowIfInvalid();
    return it_->GetUid();
}

int64_t OutEdgeIterator::GetDst() const {
    ThrowIfInvalid();
    return it_->GetDst();
}

int64_t OutEdgeIterator::GetEdgeId() const {
    ThrowIfInvalid();
    return it_->GetEdgeId();
}

int64_t OutEdgeIterator::GetSrc() const {
    ThrowIfInvalid();
    return it_->GetSrc();
}

int64_t OutEdgeIterator::GetTemporalId() const {
    ThrowIfInvalid();
    return it_->GetTemporalId();
}

bool OutEdgeIterator::IsValid() const noexcept { return it_->IsValid(); }

const std::string& OutEdgeIterator::GetLabel() const {
    ThrowIfInvalid();
    return txn_->GetEdgeLabel(*it_);
}

int16_t OutEdgeIterator::GetLabelId() const {
    ThrowIfInvalid();
    return txn_->GetEdgeLabelId(*it_);
}

FieldData OutEdgeIterator::GetField(const std::string& field_name) const {
    ThrowIfInvalid();
    return txn_->GetEdgeField(*it_, field_name);
}

std::vector<FieldData> OutEdgeIterator::GetFields(
    const std::vector<std::string>& field_names) const {
    ThrowIfInvalid();
    return txn_->GetEdgeFields(*it_, field_names);
}

FieldData OutEdgeIterator::GetField(size_t field_id) const {
    ThrowIfInvalid();
    return txn_->GetEdgeField(*it_, field_id);
}

std::vector<FieldData> OutEdgeIterator::GetFields(const std::vector<size_t>& field_ids) const {
    ThrowIfInvalid();
    return txn_->GetEdgeFields(*it_, field_ids);
}

std::map<std::string, FieldData> OutEdgeIterator::GetAllFields() const {
    ThrowIfInvalid();
    auto t = txn_->GetEdgeFields(*it_);
    return std::map<std::string, FieldData>(t.begin(), t.end());
}

void OutEdgeIterator::SetField(const std::string& field_name, const FieldData& field_value) {
    ThrowIfInvalid();
    std::vector<std::string> field_names = {field_name};
    std::vector<FieldData> field_values = {field_value};
    txn_->SetEdgeProperty(*it_, field_names, field_values);
    txn_->RefreshIterators();
}

void OutEdgeIterator::SetField(size_t field_id, const FieldData& field_value) {
    ThrowIfInvalid();
    std::vector<size_t> field_ids = {field_id};
    std::vector<FieldData> field_values = {field_value};
    txn_->SetEdgeProperty(*it_, field_ids, field_values);
    txn_->RefreshIterators();
}

void OutEdgeIterator::SetFields(const std::vector<std::string>& field_names,
                                const std::vector<std::string>& field_value_strings) {
    ThrowIfInvalid();
    txn_->SetEdgeProperty(*it_, field_names, field_value_strings);
    txn_->RefreshIterators();
}

void OutEdgeIterator::SetFields(const std::vector<std::string>& field_names,
                                const std::vector<FieldData>& field_values) {
    ThrowIfInvalid();
    txn_->SetEdgeProperty(*it_, field_names, field_values);
    txn_->RefreshIterators();
}

void OutEdgeIterator::SetFields(const std::vector<size_t>& field_ids,
                                const std::vector<FieldData>& field_values) {
    ThrowIfInvalid();
    txn_->SetEdgeProperty(*it_, field_ids, field_values);
    txn_->RefreshIterators();
}

void OutEdgeIterator::Delete() {
    ThrowIfInvalid();
    txn_->DeleteEdge(*it_);
    txn_->RefreshIterators();
}

std::string OutEdgeIterator::ToString() const {
    ThrowIfInvalid();
    return txn_->EdgeToString(*it_);
}

InEdgeIterator::InEdgeIterator(lgraph::graph::InEdgeIterator&& impl,
                               const std::shared_ptr<lgraph::Transaction>& txn)
    : it_(new lgraph::graph::InEdgeIterator(std::move(impl))), txn_(txn) {}

InEdgeIterator::InEdgeIterator(InEdgeIterator&& rhs)
    : it_(std::move(rhs.it_)), txn_(std::move(rhs.txn_)) {}

InEdgeIterator& InEdgeIterator::operator=(InEdgeIterator&& rhs) {
    it_ = std::move(rhs.it_);
    txn_ = std::move(rhs.txn_);
    return *this;
}

InEdgeIterator::~InEdgeIterator() {}

void InEdgeIterator::Close() noexcept { it_->Close(); }

bool InEdgeIterator::Next() {
    ThrowIfInvalid();
    return it_->Next();
}

bool InEdgeIterator::Goto(EdgeUid euid, bool nearest) {
    if (!txn_->IsValid()) THROW_CODE(InvalidTxn);
    return it_->Goto(euid, nearest);
}

EdgeUid InEdgeIterator::GetUid() const {
    ThrowIfInvalid();
    return it_->GetUid();
}

int64_t InEdgeIterator::GetSrc() const {
    ThrowIfInvalid();
    return it_->GetSrc();
}

int64_t InEdgeIterator::GetDst() const {
    ThrowIfInvalid();
    return it_->GetDst();
}

int64_t InEdgeIterator::GetEdgeId() const {
    ThrowIfInvalid();
    return it_->GetEdgeId();
}

int64_t InEdgeIterator::GetTemporalId() const {
    ThrowIfInvalid();
    return it_->GetTemporalId();
}

bool InEdgeIterator::IsValid() const { return it_->IsValid(); }

const std::string& InEdgeIterator::GetLabel() const {
    ThrowIfInvalid();
    return txn_->GetEdgeLabel(*it_);
}

int16_t InEdgeIterator::GetLabelId() const {
    ThrowIfInvalid();
    return txn_->GetEdgeLabelId(*it_);
}

FieldData InEdgeIterator::GetField(const std::string& field_name) const {
    ThrowIfInvalid();
    return txn_->GetEdgeField(*it_, field_name);
}

FieldData InEdgeIterator::GetField(size_t field_id) const {
    ThrowIfInvalid();
    return txn_->GetEdgeField(*it_, field_id);
}

std::vector<FieldData> InEdgeIterator::GetFields(
    const std::vector<std::string>& field_names) const {
    ThrowIfInvalid();
    return txn_->GetEdgeFields(*it_, field_names);
}

std::vector<FieldData> InEdgeIterator::GetFields(const std::vector<size_t>& field_ids) const {
    ThrowIfInvalid();
    return txn_->GetEdgeFields(*it_, field_ids);
}

std::map<std::string, FieldData> InEdgeIterator::GetAllFields() const {
    ThrowIfInvalid();
    auto props = txn_->GetEdgeFields(*it_);
    return std::map<std::string, FieldData>(props.begin(), props.end());
}

void InEdgeIterator::SetField(const std::string& field_name, const FieldData& field_value) {
    ThrowIfInvalid();
    std::vector<std::string> field_names = {field_name};
    std::vector<FieldData> field_values = {field_value};
    txn_->SetEdgeProperty(*it_, field_names, field_values);
    txn_->RefreshIterators();
}

void InEdgeIterator::SetField(size_t field_id, const FieldData& field_value) {
    ThrowIfInvalid();
    std::vector<size_t> field_ids = {field_id};
    std::vector<FieldData> field_values = {field_value};
    txn_->SetEdgeProperty(*it_, field_ids, field_values);
    txn_->RefreshIterators();
}

void InEdgeIterator::SetFields(const std::vector<std::string>& field_names,
                               const std::vector<std::string>& field_value_strings) {
    ThrowIfInvalid();
    txn_->SetEdgeProperty(*it_, field_names, field_value_strings);
    txn_->RefreshIterators();
}

void InEdgeIterator::SetFields(const std::vector<std::string>& field_names,
                               const std::vector<FieldData>& field_values) {
    ThrowIfInvalid();
    txn_->SetEdgeProperty(*it_, field_names, field_values);
    txn_->RefreshIterators();
}

void InEdgeIterator::SetFields(const std::vector<size_t>& field_ids,
                               const std::vector<FieldData>& field_values) {
    ThrowIfInvalid();
    txn_->SetEdgeProperty(*it_, field_ids, field_values);
    txn_->RefreshIterators();
}

void InEdgeIterator::Delete() {
    ThrowIfInvalid();
    txn_->DeleteEdge(*it_);
    txn_->RefreshIterators();
}

std::string InEdgeIterator::ToString() const {
    ThrowIfInvalid();
    return txn_->EdgeToString(*it_);
}

bool operator==(const OutEdgeIterator& lhs, const OutEdgeIterator& rhs) {
    return lhs.GetSrc() == rhs.GetSrc() && lhs.GetDst() == rhs.GetDst() &&
           lhs.GetLabelId() == rhs.GetLabelId() && lhs.GetEdgeId() == rhs.GetEdgeId() &&
           lhs.GetTemporalId() == rhs.GetTemporalId();
}

bool operator==(const OutEdgeIterator& lhs, const InEdgeIterator& rhs) {
    return lhs.GetSrc() == rhs.GetSrc() && lhs.GetDst() == rhs.GetDst() &&
           lhs.GetLabelId() == rhs.GetLabelId() && lhs.GetEdgeId() == rhs.GetEdgeId() &&
           lhs.GetTemporalId() == rhs.GetTemporalId();
}

bool operator==(const InEdgeIterator& lhs, const OutEdgeIterator& rhs) {
    return lhs.GetSrc() == rhs.GetSrc() && lhs.GetDst() == rhs.GetDst() &&
           lhs.GetLabelId() == rhs.GetLabelId() && lhs.GetEdgeId() == rhs.GetEdgeId() &&
           lhs.GetTemporalId() == rhs.GetTemporalId();
}

bool operator==(const InEdgeIterator& lhs, const InEdgeIterator& rhs) {
    return lhs.GetSrc() == rhs.GetSrc() && lhs.GetDst() == rhs.GetDst() &&
           lhs.GetLabelId() == rhs.GetLabelId() && lhs.GetEdgeId() == rhs.GetEdgeId() &&
           lhs.GetTemporalId() == rhs.GetTemporalId();
}

bool operator!=(const OutEdgeIterator& lhs, const OutEdgeIterator& rhs) { return !(lhs == rhs); }

bool operator!=(const OutEdgeIterator& lhs, const InEdgeIterator& rhs) { return !(lhs == rhs); }

bool operator!=(const InEdgeIterator& lhs, const OutEdgeIterator& rhs) { return !(lhs == rhs); }

bool operator!=(const InEdgeIterator& lhs, const InEdgeIterator& rhs) { return !(lhs == rhs); }

}  // namespace lgraph_api
