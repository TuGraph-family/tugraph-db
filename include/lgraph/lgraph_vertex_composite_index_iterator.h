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
class CompositeIndexIterator;

}  // namespace lgraph

namespace lgraph_api {
class Transaction;

/**
 * @brief   VertexIndexIterator can be used to access a set of vertices that has the same indexed
 *          value. If the index is unique (that is, each vertex has a unique index value), then
 *          each VertexIndexIterator will only have one VertexId, and will become invalid after
 *          Next()
 *          is called.
 *
 *          An VertexIndexIterator is valid iff it points to a valid (index_value, vid) pair,
 *          otherwise it is invalid. Calling member function on an invalid VertexIndexIterator
 *          throws an exception, except for the IsValid() function.
 */
class VertexCompositeIndexIterator {
    friend class Transaction;

    std::unique_ptr<lgraph::CompositeIndexIterator> it_;
    std::shared_ptr<lgraph::Transaction> txn_;

    VertexCompositeIndexIterator(lgraph::CompositeIndexIterator &&it,
                        const std::shared_ptr<lgraph::Transaction> &txn);

    VertexCompositeIndexIterator(const VertexCompositeIndexIterator &) = delete;
    VertexCompositeIndexIterator &operator=(const VertexCompositeIndexIterator &) = delete;

 public:
    VertexCompositeIndexIterator(VertexCompositeIndexIterator &&rhs);
    VertexCompositeIndexIterator &operator=(VertexCompositeIndexIterator &&);
    ~VertexCompositeIndexIterator();

    /**
     * @brief Closes this iterator
     */
    void Close();

    /**
     * @brief   Query if this iterator is valid, i.e. the Key and Vid can be queried.
     *
     * @returns True if valid, false if not.
     */
    bool IsValid() const;

    /**
     * @brief   Move to the next vertex id in the list, which consists of all the valid vertex
     *          ids of the iterator and is sorted from small to large. If we hit the end of the
     *          list, iterator will become invalid and false is returned.
     *
     * @returns True if it succeeds, otherwise false.
     */
    bool Next();

    /**
     * @brief   Gets the current index value. The vids are sorted in (IndexValue, Vid) order.
     *          When Next() is called, the iterator moves from one vid to next, possibly moving
     *          from one IndexValue to another. This function tells the IndexValue currently
     *          pointed to.
     *
     * @returns The key.
     */
    std::vector<FieldData> GetIndexValue() const;

    std::vector<FieldType> GetIndexType() const;

    /**
     * @brief   Gets the current vertex id.
     *
     * @returns The current vertex id.
     */
    int64_t GetVid() const;
};

}  // namespace lgraph_api
