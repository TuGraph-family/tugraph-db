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

#include "lgraph/lgraph_vertex_index_iterator.h"

namespace lgraph_api {
#define ThrowIfInvalid()                                                        \
    do {                                                                        \
        if (!txn_->IsValid()) throw std::runtime_error("Invalid transaction."); \
        if (!it_->IsValid()) throw std::runtime_error("Invalid iterator.");     \
    } while (0)

VertexIndexIterator::VertexIndexIterator(lgraph::VertexIndexIterator&& it,
                                         const std::shared_ptr<lgraph::Transaction>& txn)
    : it_(new lgraph::VertexIndexIterator(std::move(it))), txn_(txn) {}

VertexIndexIterator::VertexIndexIterator(VertexIndexIterator&& rhs)
    : it_(std::move(rhs.it_)), txn_(std::move(rhs.txn_)) {}

VertexIndexIterator& VertexIndexIterator::operator=(VertexIndexIterator&& rhs) {
    it_ = std::move(rhs.it_);
    txn_ = std::move(rhs.txn_);
    return *this;
}

VertexIndexIterator::~VertexIndexIterator() {}

void VertexIndexIterator::Close() { it_->Close(); }

bool VertexIndexIterator::IsValid() const { return it_->IsValid(); }

bool VertexIndexIterator::Next() {
    ThrowIfInvalid();
    return it_->Next();
}

FieldData VertexIndexIterator::GetIndexValue() const {
    ThrowIfInvalid();
    return lgraph::field_data_helper::ValueToFieldData(it_->GetKey(), it_->KeyType());
}

int64_t VertexIndexIterator::GetVid() const {
    ThrowIfInvalid();
    return it_->GetVid();
}
}  // namespace lgraph_api
