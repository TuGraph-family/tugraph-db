/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "core/iterator_base.h"
#include "core/transaction.h"

namespace lgraph {
IteratorBase::IteratorBase(Transaction* txn) : txn_(txn) {
    if (txn_) txn_->RegisterIterator(this);
}

IteratorBase::IteratorBase(IteratorBase&& rhs) : txn_(rhs.txn_) {
    if (txn_) {
        txn_->DeregisterIterator(&rhs);
        rhs.txn_ = nullptr;
        txn_->RegisterIterator(this);
    }
}

IteratorBase::~IteratorBase() {
    if (txn_) {
        txn_->DeregisterIterator(this);
    }
}

void IteratorBase::Close() {
    CloseImpl();
    if (txn_) {
        txn_->DeregisterIterator(this);
        txn_ = nullptr;
    }
}
}  // namespace lgraph
