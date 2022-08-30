/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
