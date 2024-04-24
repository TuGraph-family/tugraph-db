//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "lgraph/lgraph_types.h"

namespace lgraph {
class Transaction;

namespace graph {

class VertexIterator;

}  // namespace graph

}  // namespace lgraph

namespace lgraph_api {
class Transaction;
class OutEdgeIterator;
class InEdgeIterator;

/**
 * @brief   VertexIterator can be used to iterate through vertices in the DB. Vertices are sorted
 *          according to vertex id in the DB.
 *
 *          A VertexIterator is valid iff it points to a valid vertex. Calling method functions
 *          on an invalid VertexIterator throws an InvalidIterator, except for the IsValid()
 *          and Goto() functions.
 *
 *          The following operations invalidates a VertexIterator:
 *          1. Constructing an VertexIterator for non-existing vertex.
 *          2. Calling Goto() with the id of a non-existing vertex.
 *          3. Calling Next() on the last vertex.
 *          4. Calling Delete() on the last vertex.
 */
class VertexIterator {
    friend class Transaction;

    std::unique_ptr<lgraph::graph::VertexIterator> it_;
    std::shared_ptr<lgraph::Transaction> txn_;

    VertexIterator(lgraph::graph::VertexIterator&&, const std::shared_ptr<lgraph::Transaction>&);
    VertexIterator(const VertexIterator&) = delete;
    VertexIterator& operator=(const VertexIterator&) = delete;

 public:
    VertexIterator(VertexIterator&& rhs);
    VertexIterator& operator=(VertexIterator&& rhs);
    ~VertexIterator();

    /**
     * @brief Closes this iterator
     */
    void Close();

    /**
     * @brief   Move to the next vertex.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     *
     * @returns True if it succeeds, otherwise return false (no more vertex)
     *          and invalidate the iterator.
     */
    bool Next();

    /**
     * @brief   Goto the vertex with id src. If there is no vertex with exactly the same vid, and
     *          nearest==true, go to the next vertex with id>=vid, otherwise return false and
     *          invalidate the iterator.
     *
     * @exception   InvalidTxn Thrown when called inside an invalid transaction.
     *
     * @param   vid     Vertex id of the vertex to go.
     * @param   nearest (Optional) True to go to the closest vertex with id>=vid.
     *
     * @returns True if it succeeds, otherwise false (no such vertex).
     */
    bool Goto(int64_t vid, bool nearest = false);

    /**
     * @brief   Gets the vertex id.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     *
     * @returns The id.
     */
    int64_t GetId() const;

    /**
     * @brief   Gets an OutEdgeIterator pointing to the first out-going edge.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     *
     * @returns The OutEdgeIterator.
     */
    OutEdgeIterator GetOutEdgeIterator() const;

    /**
     * @brief   Returns an OutEdgeIterator pointing to the edge specified by euid. If there is no
     *          such edge, and nearest==false an invalid iterator is returned. If the specified
     *          out-edge does not exist, and nearest==true, get the first out-edge that sorts
     *          after the specified one.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     *
     * @param   euid    The Edge Unique Id.
     * @param   nearest (Optional) If set to true and the specified edge does not exist, get the
     *                  edge that sorts right after it.
     *
     * @returns The out edge iterator.
     */
    OutEdgeIterator GetOutEdgeIterator(const EdgeUid& euid, bool nearest = false) const;

    /**
     * @brief   Gets an InEdgeIterator pointing to the first in-coming edge.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     *
     * @returns The InEdgeIterator.
     */
    InEdgeIterator GetInEdgeIterator() const;

    /**
     * @brief   Returns an InEdgeIterator pointing to the edge specified by euid. If there is no
     *          such edge and nearest==false, an invalid iterator is returned. If the specified
     *          edge does not exist and nearest==true,
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     *
     * @param   euid    The Edge Unique Id.
     * @param   nearest (Optional) If set to true and the specified edge does not exist, get the
     *                  edge that sorts right after it.
     *
     * @returns The InEdgeIterator.
     */
    InEdgeIterator GetInEdgeIterator(const EdgeUid& euid, bool nearest = false) const;

    /**
     * @brief Query if this iterator is valid.
     *
     * @returns    True if valid, false if not.
     */
    bool IsValid() const;

    /**
     * @brief   Gets the label of this vertex.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     *
     * @returns The label.
     */
    const std::string& GetLabel() const;

    /**
     * @brief   Gets label id of this vertex.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     *
     * @returns The label identifier.
     */
    size_t GetLabelId() const;

    /**
     * @brief   Gets the fields specified.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     * @exception   InputError              Thrown on other input errors (field not exist, etc.).
     *
     * @param   field_names List of names of the fields.
     *
     * @returns The fields.
     */
    std::vector<FieldData> GetFields(const std::vector<std::string>& field_names) const;

    /**
     * @brief   Gets the field specified.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     * @exception	InputError				Thrown on other input errors (field not exist, etc.).
     *
     * @param   field_name  Field name.
     *
     * @returns Field value.
     */
    FieldData GetField(const std::string& field_name) const;

    /**
     * @brief   Gets the fields specified.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     * @exception	InputError				Thrown on other input errors (field not exist, etc.).
     *
     * @param   field_ids   List of ids for the fields.
     *
     * @returns The fields.
     */
    std::vector<FieldData> GetFields(const std::vector<size_t>& field_ids) const;

    /**
     * @brief   Gets the field specified.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     * @exception	InputError				Thrown on other input errors (field not exist, etc.).
     *
     * @param   field_id    Field ID.
     *
     * @returns Field value.
     */
    FieldData GetField(size_t field_id) const;

    /**
     * @brief   Get field identified by field_name
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     * @exception	InputError				Thrown on other input errors (field not exist, etc.).
     *
     * @param   field_name  Filename of the file.
     *
     * @returns The indexed value.
     */
    FieldData operator[](const std::string& field_name) const { return GetField(field_name); }

    /**
     * @brief   Get field identified by field id
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     * @exception	InputError				Thrown on other input errors (field not exist, etc.).
     *
     * @param   fid The field id.
     *
     * @returns The indexed value.
     */
    FieldData operator[](size_t fid) const { return GetField(fid); }

    /**
     * @brief   Gets all fields of current vertex.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     *
     * @returns all fields.
     */
    std::map<std::string, FieldData> GetAllFields() const;

    /**
     * @brief   Sets the specified field.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     * @exception   WriteNotAllowed    Thrown when called in a read-only transaction.
     * @exception   InputError         Thrown on other input errors (field not exist, etc.).
     *
     * @param   field_name  Field name.
     * @param   field_value Field value.
     */
    void SetField(const std::string& field_name, const FieldData& field_value);

    /**
     * @brief   Sets the specified field.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     * @exception   WriteNotAllowed    Thrown when called in a read-only transaction.
     * @exception   InputError         Thrown on other input errors (field not exist, etc.).
     *
     * @param   field_id    Field id.
     * @param   field_value Field value.
     */
    void SetField(size_t field_id, const FieldData& field_value);

    /**
     * @brief   Sets the fields specified.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     * @exception   WriteNotAllowed    Thrown when called in a read-only transaction.
     * @exception   InputError         Thrown on other input errors (field not exist, etc.).
     *
     * @param   field_names         List of names of the fields.
     * @param   field_value_strings The field value strings.
     */
    void SetFields(const std::vector<std::string>& field_names,
                   const std::vector<std::string>& field_value_strings);

    /**
     * @brief   Sets the fields specified.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     * @exception   WriteNotAllowed    Thrown when called in a read-only transaction.
     * @exception   InputError         Thrown on other input errors (field not exist, etc.).
     *
     * @param   field_names     List of names of the fields.
     * @param   field_values    The field values.
     */
    void SetFields(const std::vector<std::string>& field_names,
                   const std::vector<FieldData>& field_values);

    /**
     * @brief   Sets the fields specified.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     * @exception   WriteNotAllowed    Thrown when called in a read-only transaction.
     * @exception   InputError         Thrown on other input errors (field not exist, etc.).
     *
     * @param   field_ids       List of identifiers for the fields.
     * @param   field_values    The field values.
     */
    void SetFields(const std::vector<size_t>& field_ids,
                   const std::vector<FieldData>& field_values);

    /**
     * @brief   List source vids. Each source vid is stored only once in the result.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     *
     * @param       n_limit     (Optional) The limit on number of vids to return.
     * @param [out] more_to_go  (Optional) If non-null, returns whether the limit is exceeded.
     *
     * @returns List of source vids.
     */
    std::vector<int64_t> ListSrcVids(size_t n_limit = std::numeric_limits<size_t>::max(),
                                     bool* more_to_go = nullptr);

    /**
     * @brief   List destination vids. Each vid is stored only once in the result.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     *
     * @param       n_limit     (Optional) The limit of number of vids to return.
     * @param [out] more_to_go  (Optional) If non-null, returns whether the limit is exceeded.
     *
     * @returns List of desitnation vids.
     */
    std::vector<int64_t> ListDstVids(size_t n_limit = std::numeric_limits<size_t>::max(),
                                     bool* more_to_go = nullptr);

    /**
     * @brief   Gets number of incoming edges, stopping on limit. This function can come in handy
     *          if we need to filter on large vertexes.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     *
     * @param       n_limit     (Optional) The limit on number of in-coming edges to count. When
     *                          the limit is reached, n_limit is returned.
     * @param [out] more_to_go  (Optional) If non-null, return whether the limit is exceeded.
     *
     * @returns Number of incoming edges.
     */
    size_t GetNumInEdges(size_t n_limit = std::numeric_limits<size_t>::max(),
                         bool* more_to_go = nullptr);

    /**
     * @brief   Gets number of out-going edges, stopping on limit. This function can come in
     *          handy if we need to filter on large vertexes.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     *
     * @param       n_limit     (Optional) The limit on number of out-going edges to count. When
     *                          the limit is reached, n_limit is returned and limit_exceeded is
     *                          set to true.
     * @param [out] more_to_go  (Optional) If non-null, return whether the limit is exceeded.
     *
     * @returns Number of out-going edges.
     */
    size_t GetNumOutEdges(size_t n_limit = std::numeric_limits<size_t>::max(),
                          bool* more_to_go = nullptr);

    /**
     * @brief   Deletes this vertex, also deletes all incoming and outgoing edges of this vertex.
     *          The iterator will point to the next vertex by vid if there is any.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     * @exception   WriteNotAllowed    Thrown when called in a read-only transaction.
     *
     * @param [out] n_in_edges  (Optional) If non-null, the number of in edges the vertex had.
     * @param [out] n_out_edges (Optional) If non-null, the number of out edges the vertex
     *                             had.
     */
    void Delete(size_t* n_in_edges = nullptr, size_t* n_out_edges = nullptr);

    /** @brief Get the string representation of this vertex. */
    std::string ToString() const;
};
}  // namespace lgraph_api
