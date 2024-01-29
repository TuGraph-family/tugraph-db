/**
 * Copyright 2024 AntGroup CO., Ltd.
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

#pragma once

namespace lgraph {
class Transaction;

class IteratorBase {
    IteratorBase(const IteratorBase&) = delete;
    IteratorBase& operator=(const IteratorBase&) = delete;
    IteratorBase& operator=(IteratorBase&&) = delete;

 protected:
    Transaction* txn_;

    virtual void CloseImpl() = 0;

 public:
    explicit IteratorBase(Transaction* txn);
    IteratorBase(IteratorBase&& rhs);
    virtual ~IteratorBase();
    virtual void Close();
    Transaction* GetTxn() const { return txn_; }

    /**
     * Refresh the contents of this iterator if the underlying KvIterator was modified by other
     * write operations.
     */
    virtual void RefreshContentIfKvIteratorModified() = 0;
};
}  // namespace lgraph
