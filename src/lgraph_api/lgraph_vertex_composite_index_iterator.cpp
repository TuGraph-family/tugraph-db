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

#include "core/transaction.h"

#include "lgraph/lgraph_vertex_composite_index_iterator.h"

namespace lgraph_api {
#define ThrowIfInvalid()                                                        \
    do {                                                                        \
        if (!txn_->IsValid()) throw std::runtime_error("Invalid transaction."); \
        if (!it_->IsValid()) throw std::runtime_error("Invalid iterator.");     \
    } while (0)

VertexCompositeIndexIterator::VertexCompositeIndexIterator(lgraph::CompositeIndexIterator&& it,
                                        const std::shared_ptr<lgraph::Transaction>& txn)
    : it_(new lgraph::CompositeIndexIterator(std::move(it))), txn_(txn) {}

VertexCompositeIndexIterator::VertexCompositeIndexIterator(VertexCompositeIndexIterator&& rhs)
    : it_(std::move(rhs.it_)), txn_(std::move(rhs.txn_)) {}

VertexCompositeIndexIterator& VertexCompositeIndexIterator::operator=(
    VertexCompositeIndexIterator&& rhs) {
    it_ = std::move(rhs.it_);
    txn_ = std::move(rhs.txn_);
    return *this;
}

VertexCompositeIndexIterator::~VertexCompositeIndexIterator() {}

void VertexCompositeIndexIterator::Close() { it_->Close(); }

bool VertexCompositeIndexIterator::IsValid() const { return it_->IsValid(); }

bool VertexCompositeIndexIterator::Next() {
    ThrowIfInvalid();
    return it_->Next();
}

std::vector<FieldData> VertexCompositeIndexIterator::GetIndexValue() const {
    ThrowIfInvalid();
    return it_->GetKeyData();
}

std::vector<FieldType> VertexCompositeIndexIterator::GetIndexType() const {
    ThrowIfInvalid();
    return it_->KeyType();
}

int64_t VertexCompositeIndexIterator::GetVid() const {
    ThrowIfInvalid();
    return it_->GetVid();
}
}  // namespace lgraph_api
