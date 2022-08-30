/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
