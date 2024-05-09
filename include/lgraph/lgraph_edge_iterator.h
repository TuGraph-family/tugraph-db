//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "lgraph/lgraph_types.h"

namespace lgraph {

class Transaction;

namespace graph {

class InEdgeIterator;
class OutEdgeIterator;

}  // namespace graph

}  // namespace lgraph

namespace lgraph_api {

class Transaction;
class VertexIterator;

/**
 * @brief   An OutEdgeIterator can be used to iterate through the out-going edges of a vertex.
 *          Edges are sorted in (lid, dst, eid) order, and each (src, lid, tid, dst, eid) tuple
 *          is guaranteed to uniquely identify an edge.
 *
 *          An OutEdgeIterator is valid iff it points to a valid out-going edge, otherwise it is
 *          invalid. Calling member function on an invalid OutEdgeIterator throws an
 *          InvalidIterator, except for the IsValid() and Goto() functions.
 *
 *          The following operations invalidates an OutEdgeIterator:
 *          1. Constructing an OutEdgeIterator for non-existing edge.
 *          2. Calling Goto() with the id of a non-existing edge.
 *          3. Calling Next() on the last out-going edge.
 *          4. Calling Delete() on the last out-going edge.
 *          5. Calling Close() on the iterator.
 *
 *          In TuGraph, every iterator belongs to a transaction, and can only be used when the
 *          transaction is valid. Calling member functions on an iterator inside an invalid
 *          transaction yields InvalidTxn, except for Invalid().
 */
class OutEdgeIterator {
    friend class Transaction;
    friend class VertexIterator;

    std::unique_ptr<lgraph::graph::OutEdgeIterator> it_;
    std::shared_ptr<lgraph::Transaction> txn_;

    /**
     * @brief   Constructors are private, use Transaction::GetInEdgeIterator() or
     *          VertexIterator::GetInEdgeIterator() instead.
     */
    OutEdgeIterator(lgraph::graph::OutEdgeIterator&&, const std::shared_ptr<lgraph::Transaction>&);
    OutEdgeIterator(const OutEdgeIterator&) = delete;
    OutEdgeIterator& operator=(const OutEdgeIterator&) = delete;

 public:
    OutEdgeIterator(OutEdgeIterator&& rhs);
    OutEdgeIterator& operator=(OutEdgeIterator&& rhs);
    ~OutEdgeIterator();

    /** @brief    Closes this iterator. The iterator turns invalid after being closed. */
    void Close() noexcept;

    /**
     * @brief   Go to the edge specified by euid. That is, an edge from Vertex euid.src to
     *          euid.dst, with LabelId==euid.label, Tid==euid.tid, and EdgeId==euid.eid. If
     *          neareast==true and the exact edge was not found, the iterator tries to get the
     *          next edge that sorts after the specified edge. The edges are sorted in (label,
     *          tid, dst, eid) order. The iterator becomes invalid if there is no outgoing edge
     *          from euid.src that sorts after euid.
     *
     * @exception   InvalidTxn Thrown when called inside an invalid transaction.
     *
     * @param   euid    Edge Unique ID.
     * @param   nearest (Optional) True to get the nearest edge if the specified one cannot be
     *                  found.
     *
     * @returns True if it succeeds, false if there is no such edge.
     */
    bool Goto(EdgeUid euid, bool nearest = false);

    /**
     * @brief   Query if this iterator is valid.
     *
     * @returns True if valid, false if not.
     */
    bool IsValid() const noexcept;

    /**
     * @brief   Move to the next edge. Invalidates iterator if there is no more out edges from
     *          current source vertex.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     *
     * @returns True if it succeeds, false if it fails (no more out edge from current source).
     */
    bool Next();

    /**
     * @brief	Gets the Edge Unique Id
     *
     * @exception	InvalidTxn	    Thrown when called inside an invalid transaction.
     * @exception	InvalidIterator	Thrown when current iterator is invalid.
     *
     * @returns	The UID.
     */
    EdgeUid GetUid() const;

    /**
     * @brief	Gets destination of the edge.
     *
     * @exception	InvalidTxn		Thrown when called inside an invalid transaction.
     * @exception	InvalidIterator	Thrown when current iterator is invalid.
     *
     * @returns	The destination vertex id.
     */
    int64_t GetDst() const;

    /**
     * @brief	Gets edge id.
     *
     * @exception	InvalidTxn		Thrown when called inside an invalid transaction.
     * @exception	InvalidIterator	Thrown when current iterator is invalid.
     *
     * @returns	The edge identifier.
     */
    int64_t GetEdgeId() const;

    /**
     * @brief	Gets primary id.
     *
     * @exception	InvalidTxn		Thrown when called inside an invalid transaction.
     * @exception	InvalidIterator	Thrown when current iterator is invalid.
     *
     * @returns	The primary id of the edge.
     */
    int64_t GetTemporalId() const;

    /**
     * @brief	Gets the source vertex id.
     *
     * @exception	InvalidTxn		Thrown when called inside an invalid transaction.
     * @exception	InvalidIterator	Thrown when current iterator is invalid.
     *
     * @returns	The source vertex id.
     */
    int64_t GetSrc() const;

    /**
     * @brief	Gets the label of this edge.
     *
     * @exception	InvalidTxn		Thrown when called inside an invalid transaction.
     * @exception	InvalidIterator	Thrown when current iterator is invalid.
     *
     * @returns	The label.
     */
    const std::string& GetLabel() const;

    /**
     * @brief	Gets label id of this edge.
     *
     * @exception	InvalidTxn		Thrown when called inside an invalid transaction.
     * @exception	InvalidIterator	Thrown when current iterator is invalid.
     *
     * @returns	The label identifier.
     */
    int16_t GetLabelId() const;

    /**
     * @brief	Gets the fields specified.
     *
     * @exception	InvalidTxn		Thrown when called inside an invalid transaction.
     * @exception	InvalidIterator	Thrown when current iterator is invalid.
     * @exception	InputError				Thrown on other input errors (field not exist, etc.).
     *
     * @param 	field_names	List of names of the fields.
     *
     * @returns	The fields.
     */
    std::vector<FieldData> GetFields(const std::vector<std::string>& field_names) const;

    /**
     * @brief	Gets the field specified.
     *
     * @exception	InvalidTxn		Thrown when called inside an invalid transaction.
     * @exception	InvalidIterator	Thrown when current iterator is invalid.
     * @exception	InputError				Thrown on other input errors (field not exist, etc.).
     *
     * @param 	field_name	Field name.
     *
     * @returns	Field value.
     */
    FieldData GetField(const std::string& field_name) const;

    /**
     * @brief	Gets the fields specified.
     *
     * @exception	InvalidTxn		Thrown when called inside an invalid transaction.
     * @exception	InvalidIterator	Thrown when current iterator is invalid.
     * @exception	InputError				Thrown on other input errors (field not exist, etc.).
     *
     * @param 	field_ids	List of ids for the fields.
     *
     * @returns	The fields.
     */
    std::vector<FieldData> GetFields(const std::vector<size_t>& field_ids) const;

    /**
     * @brief	Gets the field specified.
     *
     * @exception	InvalidTxn		Thrown when called inside an invalid transaction.
     * @exception	InvalidIterator	Thrown when current iterator is invalid.
     * @exception	InputError				Thrown on other input errors (field not exist, etc.).
     *
     * @param 	field_id	Field ID.
     *
     * @returns	Field value.
     */
    FieldData GetField(size_t field_id) const;

    /**
     * @brief	Get field identified by field_name.
     *
     * @exception	InvalidTxn		Thrown when called inside an invalid transaction.
     * @exception	InvalidIterator	Thrown when current iterator is invalid.
     * @exception	InputError				Thrown on other input errors (field not exist, etc.).
     *
     * @param 	field_name	The name of the field to get.
     *
     * @returns	Field value.
     */
    FieldData operator[](const std::string& field_name) const { return GetField(field_name); }

    /**
     * @brief	Get field identified by field id. FieldId can be obtained with
     * 			txn.GetEdgeFieldId()
     *
     * @exception	InvalidTxn		Thrown when called inside an invalid transaction.
     * @exception	InvalidIterator	Thrown when current iterator is invalid.
     * @exception	InputError				Thrown on other input errors (field not exist, etc.).
     *
     * @param 	fid	fid  The field id.
     *
     * @returns	Field value.
     */
    FieldData operator[](size_t fid) const { return GetField(fid); }

    /**
     * @brief	Gets all fields of current vertex.
     *
     * @exception	InvalidTxn		Thrown when called inside an invalid transaction.
     * @exception	InvalidIterator	Thrown when current iterator is invalid.
     *
     * @returns	All fields in a dictionary of {(field_name, field_value),...}.
     */
    std::map<std::string, FieldData> GetAllFields() const;

    /**
     * @brief   Sets the specified field.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     * @exception   WriteNotAllowed    Thrown when called in a read-only transaction.
     * @exception   InputError              Thrown on other input errors (field not exist, etc.).
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
     * @exception   InputError              Thrown on other input errors (field not exist, etc.).
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
     * @exception   InputError              Thrown on other input errors (field not exist, etc.).
     *
     * @param   field_names         List of names of the fields.
     * @param   field_value_strings The field value in string representation.
     */
    void SetFields(const std::vector<std::string>& field_names,
                   const std::vector<std::string>& field_value_strings);

    /**
     * @brief   Sets the fields specified.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     * @exception   WriteNotAllowed    Thrown when called in a read-only transaction.
     * @exception   InputError              Thrown on other input errors (field not exist, etc.).
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
     * @exception   InputError              Thrown on other input errors (field not exist, etc.).
     *
     * @param   field_ids       List of identifiers for the fields.
     * @param   field_values    The field values.
     */
    void SetFields(const std::vector<size_t>& field_ids,
                   const std::vector<FieldData>& field_values);

    /**
     * @brief	Deletes this edge. The iterator will point to the next out-going edge sorted by
     * 			(label, tid, dst, eid) if there is any. If there is no more out-going edges for
     * 			this source vertex, the iterator becomes invalid.
     *
     * @exception	InvalidTxn		Thrown when called inside an invalid transaction.
     * @exception	InvalidIterator	Thrown when current iterator is invalid.
     * @exception   WriteNotAllowed Thrown when called in a read-only transaction.
     */
    void Delete();

    /**
     * @brief   Get string representation of the edge.
     *
     * @exception   InvalidTxn         Thrown when called inside an invalid transaction.
     * @exception   InvalidIterator    Thrown when current iterator is invalid.
     *
     * @returns A std::string that represents this object.
     */
    std::string ToString() const;
};

/**
 * @brief   An InEdgeIterator can be used to iterate through the in-coming edges of a vertex.
 *          Edges are sorted in (lid, tid, src, eid) order, and each (dst, lid, tid, src, eid)
 *          tuple is guaranteed to uniquely identify an edge.
 *
 *          An InEdgeIterator is valid iff it points to a valid in-coming edge, otherwise it is
 *          invalid. Calling member function on an invalid InEdgeIterator throws an exception,
 *          except for the IsValid() and Goto() functions.
 *
 *          The following operations invalidates an InEdgeIterator:
 *          1. Constructing an InEdgeIterator for non-existing edge.
 *          2. Calling Goto() with the id of a non-existing edge.
 *          3. Calling Next() on the last in-coming edge.
 *          4. Calling Delete() on the last in-coming edge.
 *          5. Calling Close() on the iterator.
 *
 *          In TuGraph, every iterator belongs to a transaction, and can only be used when the
 *          transaction is valid. Calling member functions on an iterator inside an invalid
 *          transaction yields InvalidTxn, except for Invalid().
 */
class InEdgeIterator {
    friend class Transaction;
    friend class VertexIterator;

    std::unique_ptr<lgraph::graph::InEdgeIterator> it_;
    std::shared_ptr<lgraph::Transaction> txn_;

    /**
     * @brief   Constructors are private, use Transaction::GetInEdgeIterator() or
     *          VertexIterator::GetInEdgeIterator() instead.
     */
    InEdgeIterator(lgraph::graph::InEdgeIterator&&, const std::shared_ptr<lgraph::Transaction>&);
    InEdgeIterator(const InEdgeIterator&) = delete;
    InEdgeIterator& operator=(const InEdgeIterator&) = delete;

 public:
    InEdgeIterator(InEdgeIterator&& rhs);
    InEdgeIterator& operator=(InEdgeIterator&&);
    ~InEdgeIterator();

    /** @brief   Closes this iterator. The iterator turns invalid after being closed. */
    void Close() noexcept;

    /**
     * @brief   Move to the next incoming edge to current destination vertex. If there is no more
     *          edge, the iterator becomes invalid and false is returned.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     *
     * @returns True if it succeeds, false if it fails.
     */
    bool Next();

    /**
     * @brief   Go to the edge specified by euid. If the specified edge cannot be found and
     *          nearest==true, then try to get the next in-coming edge to the vertex euid.dst,
     *          sorted by (label, tid, src, eid). If there is no such edge, iterator is
     *          invalidated and false is returned.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     *
     * @param   euid    Edge Unique Id.
     * @param   nearest (Optional) True to get the nearest edge if the specified one cannot be
     *                  found.
     *
     * @returns True if it succeeds, false if it fails.
     */
    bool Goto(EdgeUid euid, bool nearest = false);

    /**
     * @brief   Gets the Edge Unique Id
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     *
     * @returns The UID.
     */
    EdgeUid GetUid() const;

    /**
     * @brief   Gets the source vertex id.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     *
     * @return  The source vertex id.
     */
    int64_t GetSrc() const;

    /**
     * @brief   Gets destination vertex id.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     *
     * @returns The destination vertex id.
     */
    int64_t GetDst() const;

    /**
     * @brief   Gets edge id.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     *
     * @returns The edge id.
     */
    int64_t GetEdgeId() const;

    /**
     * @brief   Gets temporal id.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     *
     * @returns  The temporal id.
     */
    int64_t GetTemporalId() const;

    /**
     * @brief   Query if this iterator is valid.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     *
     * @returns True if valid, false if not.
     */
    bool IsValid() const;

    /**
     * @brief   Gets the label of this edge.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     *
     * @returns The label.
     */
    const std::string& GetLabel() const;

    /**
     * @brief   Gets label id of this edge.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     *
     * @returns The label identifier.
     */
    int16_t GetLabelId() const;

    /**
     * @brief   Gets the fields specified.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     * @exception   InputError              Thrown if any field does not exist.
     *
     * @param   field_names List of names of the fields.
     *
     * @returns The fields.
     */
    std::vector<FieldData> GetFields(const std::vector<std::string>& field_names) const;

    /**
     * @brief   Gets the field specified.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     * @exception   InputError              Thrown if field does not exist.
     *
     * @param   field_name  Field name.
     *
     * @return  Field value.
     */
    FieldData GetField(const std::string& field_name) const;

    /**
     * @brief   Gets the fields specified.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     * @exception   InputError              Thrown if any field does not exist.
     *
     * @param   field_ids   List of ids for the fields.
     *
     * @returns The fields.
     */
    std::vector<FieldData> GetFields(const std::vector<size_t>& field_ids) const;

    /**
     * @brief   Gets the field specified.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     * @exception   InputError              Thrown if field does not exist.
     *
     * @param   field_id    Field ID.
     *
     * @returns Field value.
     */
    FieldData GetField(size_t field_id) const;

    /**
     * @brief   Get field identified by field_name
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     * @exception   InputError              Thrown if field does not exist.
     *
     * @param   field_name  Filename of the file.
     *
     * @returns The indexed value.
     */
    FieldData operator[](const std::string& field_name) const { return GetField(field_name); }

    /**
     * @brief   Get field identified by field id
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     * @exception   InputError              Thrown if field does not exist.
     *
     * @param   fid The field id.
     *
     * @returns The indexed value.
     */
    FieldData operator[](size_t fid) const { return GetField(fid); }

    /**
     * @brief   Gets all fields of current vertex.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     * @exception   InputError              Thrown if any field does not exist.
     *
     * @returns All field names and values stored as a {(field_name, field_value),...} map.
     */
    std::map<std::string, FieldData> GetAllFields() const;

    /**
     * @brief   Sets the specified field.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     * @exception   WriteNotAllowed    Thrown if called inside a read-only transaction.
     * @exception   InputError              Thrown if any field does not exist.
     *
     * @param   field_name  Field name.
     * @param   field_value Field value.
     */
    void SetField(const std::string& field_name, const FieldData& field_value);

    /**
     * @brief   Sets the specified field.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     * @exception   WriteNotAllowed    Thrown if called inside a read-only transaction.
     * @exception   InputError              Thrown if field does not exist.
     *
     * @param   field_id    Field id.
     * @param   field_value Field value.
     */
    void SetField(size_t field_id, const FieldData& field_value);

    /**
     * @brief   Sets the fields specified.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     * @exception   WriteNotAllowed    Thrown if called inside a read-only transaction.
     * @exception   InputError              Thrown if any field does not exist or field value
     *                                      type is incorrect.
     *
     * @param   field_names         List of names of the fields.
     * @param   field_value_strings The field value strings.
     */
    void SetFields(const std::vector<std::string>& field_names,
                   const std::vector<std::string>& field_value_strings);

    /**
     * @brief   Sets the fields specified.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     * @exception   WriteNotAllowed    Thrown if called inside a read-only transaction.
     * @exception   InputError              Thrown if any field does not exist or field value
     *                                      type is incorrect.
     *
     * @param   field_names     List of names of the fields.
     * @param   field_values    The field values.
     */
    void SetFields(const std::vector<std::string>& field_names,
                   const std::vector<FieldData>& field_values);

    /**
     * @brief   Sets the fields specified.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     * @exception   WriteNotAllowed    Thrown if called inside a read-only transaction.
     * @exception   InputError              Thrown if any field does not exist or field value
     *                                      type is incorrect.
     *
     * @param   field_ids       List of identifiers for the fields.
     * @param   field_values    The field values.
     */
    void SetFields(const std::vector<size_t>& field_ids,
                   const std::vector<FieldData>& field_values);

    /**
     * @brief   Deletes this edge. The iterator will point to the next incoming edge sorted by
     *          (lid, tid, src, eid) if there is any. If no in-coming edge is left for this
     *          vertex, the iterator becomes invalid.
     *
     * @exception   InvalidTxn         Thrown if the transaction is invalid.
     * @exception   InvalidIterator    Thrown if the iterator is invalid.
     * @exception   WriteNotAllowed    Thrown if called inside a read-only transaction.
     */
    void Delete();

    /** @brief   Get string representation of the edge. */
    std::string ToString() const;
};

/**
 * @brief   Check whether lhs and rhs points to the same edge.
 */
bool operator==(const OutEdgeIterator& lhs, const OutEdgeIterator& rhs);

/**
 * @brief   Check whether lhs and rhs points to the same edge.
 */
bool operator==(const OutEdgeIterator& lhs, const InEdgeIterator& rhs);

/**
 * @brief   Check whether lhs and rhs points to the same edge.
 */
bool operator==(const InEdgeIterator& lhs, const OutEdgeIterator& rhs);

/**
 * @brief   Check whether lhs and rhs points to the same edge.
 */
bool operator==(const InEdgeIterator& lhs, const InEdgeIterator& rhs);

/**
 * @brief   Check whether lhs and rhs points to the same edge.
 */
bool operator!=(const OutEdgeIterator& lhs, const OutEdgeIterator& rhs);

/**
 * @brief   Check whether lhs and rhs points to the same edge.
 */
bool operator!=(const OutEdgeIterator& lhs, const InEdgeIterator& rhs);

/**
 * @brief   Check whether lhs and rhs points to the same edge.
 */
bool operator!=(const InEdgeIterator& lhs, const OutEdgeIterator& rhs);

/**
 * @brief   Check whether lhs and rhs points to the same edge.
 */
bool operator!=(const InEdgeIterator& lhs, const InEdgeIterator& rhs);

}  // namespace lgraph_api
