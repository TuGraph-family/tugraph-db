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
#include "core/type_convert.h"

#include "lgraph/lgraph_edge_index_iterator.h"

namespace lgraph_api {
#define ThrowIfInvalid()                                                        \
    do {                                                                        \
        if (!txn_->IsValid()) throw std::runtime_error("Invalid transaction."); \
        if (!it_->IsValid()) throw std::runtime_error("Invalid iterator.");     \
    } while (0)

EdgeIndexIterator::EdgeIndexIterator(lgraph::EdgeIndexIterator&& it,
                                     const std::shared_ptr<lgraph::Transaction>& txn)
    : it_(new lgraph::EdgeIndexIterator(std::move(it))), txn_(txn) {}

EdgeIndexIterator::EdgeIndexIterator(EdgeIndexIterator&& rhs)
    : it_(std::move(rhs.it_)), txn_(std::move(rhs.txn_)) {}

EdgeIndexIterator& EdgeIndexIterator::operator=(EdgeIndexIterator&& rhs) {
    it_ = std::move(rhs.it_);
    txn_ = std::move(rhs.txn_);
    return *this;
}

EdgeIndexIterator::~EdgeIndexIterator() {}

void EdgeIndexIterator::Close() { it_->Close(); }

bool EdgeIndexIterator::IsValid() const { return it_->IsValid(); }

bool EdgeIndexIterator::Next() {
    ThrowIfInvalid();
    return it_->Next();
}

FieldData EdgeIndexIterator::GetIndexValue() const {
    ThrowIfInvalid();
    return lgraph::field_data_helper::ValueToFieldData(it_->GetKey(), it_->KeyType());
}

EdgeUid EdgeIndexIterator::GetUid() const {
    ThrowIfInvalid();
    return it_->GetUid();
}

int64_t EdgeIndexIterator::GetSrc() const {
    ThrowIfInvalid();
    return it_->GetSrcVid();
}

int64_t EdgeIndexIterator::GetDst() const {
    ThrowIfInvalid();
    return it_->GetDstVid();
}

int64_t EdgeIndexIterator::GetEdgeId() const {
    ThrowIfInvalid();
    return it_->GetEid();
}

size_t EdgeIndexIterator::GetLabelId() const {
    ThrowIfInvalid();
    return it_->GetLabelId();
}
}  // namespace lgraph_api
