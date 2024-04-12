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
