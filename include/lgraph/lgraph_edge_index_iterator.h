//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once
#include <memory>
#include "lgraph/lgraph_types.h"

namespace lgraph {

class Transaction;
class EdgeIndexIterator;

}  // namespace lgraph

namespace lgraph_api {
class Transaction;

/**
 * \brief   EdgeIndexIterator can be used to access a set of edges that has
 *          the same indexed value. If the index is unique (that is, each
 *            Edge has a unique index value), then each   EdgeIndexIterator will
 *          only have one edge unique id, and will become invalid after Next()
 *          is called.
 *
 *          An   EdgeIndexIterator is valid iff it points to a valid (index_value,
 *          euid) pair, otherwise it is invalid. Calling member function on an
 *          invalid   EdgeIndexIterator throws an exception, except for the
 *          IsValid() function.
 */
class EdgeIndexIterator {
    friend class Transaction;

    std::unique_ptr<lgraph::EdgeIndexIterator> it_;
    std::shared_ptr<lgraph::Transaction> txn_;

    EdgeIndexIterator(lgraph::EdgeIndexIterator&& it,
                      const std::shared_ptr<lgraph::Transaction>& txn);
    EdgeIndexIterator(const EdgeIndexIterator&) = delete;
    EdgeIndexIterator& operator=(const EdgeIndexIterator&) = delete;

 public:
    EdgeIndexIterator(EdgeIndexIterator&& rhs);
    EdgeIndexIterator& operator=(EdgeIndexIterator&&);
    ~EdgeIndexIterator();

    /**
     * \brief Closes this iterator
     */
    void Close();

    /**
     * \brief   Query if this iterator is valid, i.e. the Key and Vid can be
     *          queried.
     *
     * \return  True if valid, false if not.
     */
    bool IsValid() const;

    /**
     * \brief   Move to the next edge unique id in the list, which consists of all the
     *          valid edge unique ids of the iterator and is sorted from small to
     *          large. If we hit the end of the list, iterator will become
     *          invalid and false is returned.
     *
     * \return  True if it succeeds, otherwise false.
     */
    bool Next();

    /**
     * \brief   Gets the current index value. The euids are sorted in (  EdgeIndexValue,
     *          euid) order. When Next() is called, the iterator moves from one
     *          euid to next, possibly moving from one EdgeIndexValue to another. This
     *          function tells the EdgeIndexValue currently pointed to.
     *
     * \return  The key.
     */
    FieldData GetIndexValue() const;

    /**
     * \brief   Gets the Edge Unique Id
     *
     * \return  The UID.
     */
    EdgeUid GetUid() const;

    /**
     * \brief   Gets the source vertex id.
     *
     * \return  The source vertex id.
     */
    int64_t GetSrc() const;

    /**
     * \brief   Gets destination of the edge.
     *
     * \return  The destination vertex id.
     */
    int64_t GetDst() const;

    /**
     * \brief   Gets label id of this edge.
     *
     * \return  The label identifier.
     */
    size_t GetLabelId() const;

    /**
     * \brief   Gets edge id.
     *
     * \return  The edge identifier.
     */
    int64_t GetEdgeId() const;
};

}  // namespace lgraph_api
