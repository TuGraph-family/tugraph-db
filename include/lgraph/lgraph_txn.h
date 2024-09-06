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
#include <string>
#include <type_traits>
#include <vector>

#include "lgraph/lgraph_types.h"

namespace lgraph {

class Transaction;

}  // namespace lgraph

namespace lgraph_api {
class GraphDB;
class VertexIterator;
class OutEdgeIterator;
class InEdgeIterator;
class VertexIndexIterator;
class EdgeIndexIterator;
class VertexCompositeIndexIterator;

/**
 * @brief   TuGraph operations happen in transactions. A transaction is sequence of operations
 *          that is carried out atomically on the GraphDB. TuGraph transactions provides full
 *          ACID guarantees.
 *
 *          Transactions are created using GraphDB::CreateReadTxn() and
 *          GraphDB::CreateWriteTxn(). A read transaction can only perform read operations,
 *          otherwise an exception is thrown. A write transaction can perform reads as well as
 *          writes. There are performance differences between read and write operations. So if
 *          you only need read in a transaction, you should create a read transaction.
 *
 *          Each transaction must be used in one thread only, and they should not be passed from
 *          one thread to another unless it is a forked transaction.
 *
 *          Read transactions can be forked. The new copy of the transaction will have the same
 *          view as the forked one, and it can be used in a separate thread. By forking from one
 *          read transaction and using the forked copies in different threads, we can parallelize
 *          the execution of specific operations. For example, you can implement a parallel BFS
 *          with this capability. Also, you can dump a snapshot of the whole graph using
 */
class Transaction {
    friend class GraphDB;
    std::shared_ptr<lgraph::Transaction> txn_;
    explicit Transaction(lgraph::Transaction&&);

 public:
    Transaction(Transaction&& rhs) = default;
    Transaction& operator=(Transaction&& rhs) = default;
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    /**
     * @brief   Commits this transaction. Note that optimistic write transactions may fail to
     *          commit (an TxnConflict would be thrown).
     */
    void Commit();

    /** @brief   Aborts this transaction. */
    void Abort();

    /**
     * @brief   Query if this transaction is valid. Transaction becomes invalid after calling
     *          Abort() or Commit(). Operations on invalid transaction yields exceptions.
     *
     * @returns True if valid, false if not.
     */
    bool IsValid() const;

    /**
     * @brief   Query if this txn is read only
     *
     * @returns True if read only, false if not.
     */
    bool IsReadOnly() const;

    /**
     * @brief   Get Transaction
     *
     * @returns Transaction
     */
    const std::shared_ptr<lgraph::Transaction> GetTxn();

    /**
     * @brief   Get a vertex iterator pointing to the first vertex. If there is no vertex, the
     *          iterator is invalid.
     *
     * @returns The vertex iterator.
     */
    VertexIterator GetVertexIterator();

    /**
     * @brief   Gets a vertex iterator pointing to the Vertex with vid. If the vertex does not
     *          exist, the iterator is invalid. If nearest==true, the iterator points to the
     *          first vertex sorted by vid, with id>=vid.
     *
     * @param   vid     The vid.
     * @param   nearest (Optional) True to point to the nearest vertex sorted by vid.
     *
     * @returns The vertex iterator.
     */
    VertexIterator GetVertexIterator(int64_t vid, bool nearest = false);

    /**
     * @brief   Gets an out edge iterator pointing to the edge specified by euid. If
     *          nearest==true, and the specified edge does not exist, return the first edge that
     *          sorts after the specified one.
     *
     * @param   euid    Edge Unique Id.
     * @param   nearest (Optional) If true, get the first edge that sorts after the specified one
     *                  if the specified one does not exist.
     *
     * @returns The out edge iterator.
     */
    OutEdgeIterator GetOutEdgeIterator(const EdgeUid& euid, bool nearest = false);

    OutEdgeIterator GetOutEdgeIterator(const int64_t src, const int64_t dst, const int16_t lid);

    /**
     * @brief   Gets an in edge iterator pointing to the edge specified by euid. If nearest==true,
     *          and the specified edge does not exist, return the first edge that sorts after the
     *          specified one.
     *
     * @param   euid    Edge Unique Id.
     * @param   nearest (Optional) If true, get the first edge that sorts after the specified one
     *                  if the specified one does not exist.
     *
     * @returns The out edge iterator.
     */
    InEdgeIterator GetInEdgeIterator(const EdgeUid& euid, bool nearest = false);

    InEdgeIterator GetInEdgeIterator(const int64_t src, const int64_t dst, const int16_t lid);

    /**
     * @brief   Gets number of vertex labels
     *
     * @returns The number of vertex labels.
     */
    size_t GetNumVertexLabels();

    /**
     * @brief   Gets number of edge labels.
     *
     * @returns The number of edge labels.
     */
    size_t GetNumEdgeLabels();

    /**
     * @brief   Lists all vertex labels.
     *
     * @returns Label names.
     */
    std::vector<std::string> ListVertexLabels();

    /**
     * @brief   List all edge labels
     *
     * @returns Label names.
     */
    std::vector<std::string> ListEdgeLabels();

    /**
     * @brief   Gets vertex label id corresponding to the label name.
     *
     * @param   label   The label name.
     *
     * @returns The label id.
     */
    size_t GetVertexLabelId(const std::string& label);

    /**
     * @brief   Gets edge label id corresponding to the label name.
     *
     * @param   label   The label.
     *
     * @returns The edge label id.
     */
    size_t GetEdgeLabelId(const std::string& label);

    /**
     * @brief   Gets edge schema definition corresponding to the vertex label.
     *
     * @param   label   The label.
     *
     * @returns The schema.
     */
    std::vector<FieldSpec> GetVertexSchema(const std::string& label);

    /**
     * @brief   Gets edge schema definition corresponding to the edge label.
     *
     * @param   label   The label.
     *
     * @returns The edge schema.
     */
    std::vector<FieldSpec> GetEdgeSchema(const std::string& label);

    /**
     * @brief   Gets vertex field id.
     *
     * @param   label_id    Identifier for the label.
     * @param   field_name  Field name.
     *
     * @returns The vertex field identifiers.
     */
    size_t GetVertexFieldId(size_t label_id, const std::string& field_name);

    /**
     * @brief   Gets vertex field ids.
     *
     * @param   label_id    Identifier for the label.
     * @param   field_names Field names.
     *
     * @returns The vertex field identifiers.
     */
    std::vector<size_t> GetVertexFieldIds(size_t label_id,
                                          const std::vector<std::string>& field_names);

    /**
     * @brief   Gets edge field id.
     *
     * @param   label_id    Identifier for the label.
     * @param   field_name  Field name.
     *
     * @returns The edge field identifier.
     */
    size_t GetEdgeFieldId(size_t label_id, const std::string& field_name);

    /**
     * @brief   Gets edge field ids.
     *
     * @param   label_id    Identifier for the label.
     * @param   field_names Field names.
     *
     * @returns The edge field identifier.
     */
    std::vector<size_t> GetEdgeFieldIds(size_t label_id,
                                        const std::vector<std::string>& field_names);

    /**
     * @brief   Adds a vertex. All non-nullable fields must be specified. VertexIndex is also
     *          updated. If a unique_id is indexed for the vertex, and the same unique_id exists,
     *          an exception is thrown.
     *
     * @param   label_name          Name of the label.
     * @param   field_names         List of names of the fields.
     * @param   field_value_strings The field values in string representation.
     *
     * @returns Vertex id of the new vertex.
     */
    int64_t AddVertex(const std::string& label_name, const std::vector<std::string>& field_names,
                      const std::vector<std::string>& field_value_strings);

    /**
     * @brief   Adds a vertex. All non-nullable fields must be specified. VertexIndex is also
     *          updated. If a unique_id is indexed for the vertex, and the same unique_id exists,
     *          an exception is thrown.
     *
     * @param   label_name      Name of the label.
     * @param   field_names     List of names of the fields.
     * @param   field_values    The field values.
     *
     * @returns Vertex id of the new vertex.
     */
    int64_t AddVertex(const std::string& label_name, const std::vector<std::string>& field_names,
                      const std::vector<FieldData>& field_values);

    /**
     * @brief   Adds a vertex. All non-nullable fields must be specified. VertexIndex is also
     *          updated. If a unique_id is indexed for the vertex, and the same unique_id exists,
     *          an exception is thrown.
     *
     * @param   label_id        Label id.
     * @param   field_ids       List of field ids.
     * @param   field_values    The field values.
     *
     * @returns Vertex id of the new vertex.
     */
    int64_t AddVertex(size_t label_id, const std::vector<size_t>& field_ids,
                      const std::vector<FieldData>& field_values);

    /**
     * @brief   Upsert a vertex.
     *
     * @param   label_id        Label id.
     * @param   primary_pos     The location of the primary field in field_ids.
     * @param   unique_pos      The locations of the unique index field in field_ids, can be empty.
     * @param   field_ids       List of field ids.
     * @param   field_values    The field values.
     *
     * @returns  0: nothing happened because of index conflict
     *           1: the vertex is inserted
     *           2: the vertex is updated
     */
    int UpsertVertex(size_t label_id, size_t primary_pos,
                      const std::vector<size_t>& unique_pos,
                      const std::vector<size_t>& field_ids,
                      const std::vector<FieldData>& field_values);

    /**
     * @brief   Adds an edge. All non-nullable fields must be specified. An exception is thrown
     *          if src or dst does not exist.
     *
     * @param   src                 Source vertex id.
     * @param   dst                 Destination vertex id.
     * @param   label               The label name.
     * @param   field_names         List of field names.
     * @param   field_value_strings List of field values in string representation.
     *
     * @returns EdgeUid of the new edge.
     */
    EdgeUid AddEdge(int64_t src, int64_t dst, const std::string& label,
                    const std::vector<std::string>& field_names,
                    const std::vector<std::string>& field_value_strings);

    /**
     * @brief   Adds an edge. All non-nullable fields must be specified. An exception is thrown
     *          if src or dst does not exist.
     *
     * @param   src             Source vertex id.
     * @param   dst             Destination vertex id.
     * @param   label           The label name.
     * @param   field_names     List of field names.
     * @param   field_values    List of field values.
     *
     * @returns EdgeUid of the new edge.
     */
    EdgeUid AddEdge(int64_t src, int64_t dst, const std::string& label,
                    const std::vector<std::string>& field_names,
                    const std::vector<FieldData>& field_values);

    /**
     * @brief   Adds an edge. All non-nullable fields must be specified. An exception is thrown
     *          if src or dst does not exist.
     *
     * @param   src             Source vertex id.
     * @param   dst             Destination vertex id.
     * @param   label_id        The label id.
     * @param   field_ids       List of field ids.
     * @param   field_values    List of field values.
     *
     * @returns EdgeUid of the new edge.
     */
    EdgeUid AddEdge(int64_t src, int64_t dst, size_t label_id, const std::vector<size_t>& field_ids,
                    const std::vector<FieldData>& field_values);

    /**
     * @brief   Upsert edge. If there is no src->dst edge, insert it. Otherwise, try to update
     *          the edge's property. If the edge exists and the label differs from specified
     *          label, an exception is thrown.
     *
     * @param   src                 Source vertex id.
     * @param   dst                 Destination vertex id.
     * @param   label               The label name.
     * @param   field_names         List of field names.
     * @param   field_value_strings List of field values in string representation.
     *
     * @returns True if the edge is inserted, false if the edge is updated.
     */
    bool UpsertEdge(int64_t src, int64_t dst, const std::string& label,
                    const std::vector<std::string>& field_names,
                    const std::vector<std::string>& field_value_strings);

    /**
     * @brief   Upsert edge. If there is no src->dst edge, insert it. Otherwise, try to update
     *          the edge's property. If the edge exists and the label differs from specified
     *          label, an exception is thrown.
     *
     * @param   src             Source vertex id.
     * @param   dst             Destination vertex id.
     * @param   label           The label name.
     * @param   field_names     List of field names.
     * @param   field_values    List of field values.
     *
     * @returns True if the edge is inserted, false if the edge is updated.
     */
    bool UpsertEdge(int64_t src, int64_t dst, const std::string& label,
                    const std::vector<std::string>& field_names,
                    const std::vector<FieldData>& field_values);

    /**
     * @brief   Upsert edge. If there is no src->dst edge, insert it. Otherwise, try to update
     *          the edge's property. If the edge exists and the label differs from specified
     *          label, an exception is thrown.
     *
     * @param   src             Source vertex id.
     * @param   dst             Destination vertex id.
     * @param   label_id        The label id.
     * @param   field_ids       List of field ids.
     * @param   field_values    List of field values.
     *
     * @returns True if the edge is inserted, false if the edge is updated.
     */
    bool UpsertEdge(int64_t src, int64_t dst, size_t label_id, const std::vector<size_t>& field_ids,
                    const std::vector<FieldData>& field_values);

    /**
     * @brief   Upsert edge. If there is no src->dst edge, insert it. Otherwise, try to update
     *          the edge's property.
     *
     * @param   src             Source vertex id.
     * @param   dst             Destination vertex id.
     * @param   label_id        The label id.
     * @param   unique_pos      The locations of the unique index field in field_ids, can be empty.
     * @param   field_ids       List of field ids.
     * @param   field_values    List of field values.
     *
     * @returns  0: nothing happened because of index conflict
     *           1: the vertex is inserted
     *           2: the vertex is updated
     */
    int UpsertEdge(int64_t src, int64_t dst, size_t label_id,
                   const std::vector<size_t>& unique_pos,
                   const std::vector<size_t>& field_ids,
                   const std::vector<FieldData>& field_values,
                   std::optional<size_t> pair_unique_pos);

    /**
     * @brief   List indexes
     *
     * @returns A vector of vertex index specs.
     */
    std::vector<IndexSpec> ListVertexIndexes();

    /**
     * @brief   List indexes
     *
     * @returns A vector of vertex composite index specs.
     */
    std::vector<CompositeIndexSpec> ListVertexCompositeIndexes();

    /**
     * @brief   List indexes
     *
     * @returns A vector of edge index specs.
     */
    std::vector<IndexSpec> ListEdgeIndexes();

    /**
     * @brief   Gets vertex index iterator. The iterator has field value [key_start, key_end]. So
     *          key_start=key_end=v returns an iterator pointing to all vertexes that has field
     *          value v.
     *
     * @param   label_id    The label id.
     * @param   field_id    The field id.
     * @param   key_start   The key start.
     * @param   key_end     The key end, inclusive.
     *
     * @returns The index iterator.
     */
    VertexIndexIterator GetVertexIndexIterator(size_t label_id, size_t field_id,
                                               const FieldData& key_start,
                                               const FieldData& key_end);

    VertexCompositeIndexIterator GetVertexCompositeIndexIterator(size_t label_id,
                                               const std::vector<size_t>& field_id,
                                               const std::vector<FieldData>& key_start,
                                               const std::vector<FieldData>& key_end);

    /**
     * @brief   Gets edge index iterator. The iterator has field value [key_start, key_end]. So
     *          key_start=key_end=v returns an iterator pointing to all edges that has field
     *          value v.
     *
     * @param   label_id    The label id.
     * @param   field_id    The field id.
     * @param   key_start   The key start.
     * @param   key_end     The key end, inclusive.
     *
     * @returns The index iterator.
     */
    EdgeIndexIterator GetEdgeIndexIterator(size_t label_id, size_t field_id,
                                           const FieldData& key_start, const FieldData& key_end);

    EdgeIndexIterator GetEdgePairUniqueIndexIterator(size_t label_id, size_t field_id,
                                                     int64_t src_vid, int64_t dst_vid,
                                                     const FieldData& key_start,
                                                     const FieldData& key_end);

    /**
     * @brief   Gets vertex index iterator. The iterator has field value [key_start, key_end]. So
     *          key_start=key_end=v returns an iterator pointing to all vertexes that has field
     *          value v.
     *
     * @param   label       The label.
     * @param   field       The field.
     * @param   key_start   The key start.
     * @param   key_end     The key end, inclusive.
     *
     * @returns The index iterator.
     */
    VertexIndexIterator GetVertexIndexIterator(const std::string& label, const std::string& field,
                                               const FieldData& key_start,
                                               const FieldData& key_end);

    VertexCompositeIndexIterator GetVertexCompositeIndexIterator(const std::string& label,
                                               const std::vector<std::string>& field,
                                               const std::vector<FieldData>& key_start,
                                               const std::vector<FieldData>& key_end);

    /**
     * @brief   Gets index iterator. The iterator has field value [key_start, key_end]. So
     *          key_start=key_end=v returns an iterator pointing to all edges that has field
     *          value v.
     *
     * @param   label       The label.
     * @param   field       The field.
     * @param   key_start   The key start.
     * @param   key_end     The key end, inclusive.
     *
     * @returns The index iterator.
     */
    EdgeIndexIterator GetEdgeIndexIterator(const std::string& label, const std::string& field,
                                           const FieldData& key_start, const FieldData& key_end);

    /**
     * @brief   Gets index iterator. The iterator has field value [key_start, key_end]. So
     *          key_start=key_end=v returns an iterator pointing to all vertexes that has field
     *          value v.
     *
     * @param   label       The label.
     * @param   field       The field.
     * @param   key_start   The key start.
     * @param   key_end     The key end.
     *
     * @returns The index iterator.
     */
    VertexIndexIterator GetVertexIndexIterator(const std::string& label, const std::string& field,
                                               const std::string& key_start,
                                               const std::string& key_end);

    VertexCompositeIndexIterator GetVertexCompositeIndexIterator(const std::string& label,
                                               const std::vector<std::string>& field,
                                               const std::vector<std::string>& key_start,
                                               const std::vector<std::string>& key_end);

    /**
     * @brief   Gets index iterator. The iterator has field value [key_start, key_end]. So
     *          key_start=key_end=v returns an iterator pointing to all edges that has field
     *          value v.
     *
     * @param   label       The label.
     * @param   field       The field.
     * @param   key_start   The key start.
     * @param   key_end     The key end.
     *
     * @returns The index iterator.
     */
    EdgeIndexIterator GetEdgeIndexIterator(const std::string& label, const std::string& field,
                                           const std::string& key_start,
                                           const std::string& key_end);

    /**
     * @brief   Query if index is ready for use. This should be used only to decide whether to
     *          use an index. To wait for an index to be ready, use lgraphDB::WaitIndexReady().
     *
     *          VertexIndex building is async, especially when added for a (label, field) that
     *          already has a lot of vertices. This function tells us if the index building is
     *          finished.
     *
     *          DO NOT wait for index building in a transaction. Write transactions block other
     *          write transactions, so blocking in a write transaction is always a bad idea. And
     *          long-living read transactions interfere with GC, making the DB grow unexpectly.
     *
     * @param   label   The label.
     * @param   field   The field.
     *
     * @returns True if index ready, false if not.
     */
    bool IsVertexIndexed(const std::string& label, const std::string& field);

    /**
     * @brief   Query if index is ready for use. This should be used only to decide whether to
     *          use an index. To wait for an index to be ready, use lgraphDB::WaitIndexReady().
     *
     *          VertexIndex building is async, especially when added for a (label, field) that
     *          already has a lot of edges. This function tells us if the index building is
     *          finished.
     *
     *          DO NOT wait for index building in a transaction. Write transactions block other
     *          write transactions, so blocking in a write transaction is always a bad idea. And
     *          long-living read transactions interfere with GC, making the DB grow unexpectly.
     *
     * @param   label   The label.
     * @param   field   The field.
     *
     * @returns True if index ready, false if not.
     */
    bool IsEdgeIndexed(const std::string& label, const std::string& field);

    /**
     * @brief   Gets vertex by unique index. Throws exception if there is no such vertex.
     *
     * @param   label_name          Name of the label.
     * @param   field_name          Name of the field.
     * @param   field_value_string  The field value string.
     *
     * @returns The vertex by unique index.
     */
    VertexIterator GetVertexByUniqueIndex(const std::string& label_name,
                                          const std::string& field_name,
                                          const std::string& field_value_string);

    VertexIterator GetVertexByUniqueCompositeIndex(const std::string& label_name,
                                          const std::vector<std::string>& field_name,
                                          const std::vector<std::string>& field_value_string);

    /**
     * @brief   Gets edge by unique index. Throws exception if there is no such vertex.
     *
     * @param   label_name          Name of the label.
     * @param   field_name          Name of the field.
     * @param   field_value_string  The field value string.
     *
     * @returns The vertex by unique index.
     */
    OutEdgeIterator GetEdgeByUniqueIndex(const std::string& label_name,
                                         const std::string& field_name,
                                         const std::string& field_value_string);

    /**
     * @brief   Gets vertex by unique index. Throws exception if there is no such vertex.
     *
     * @param   label_name  Name of the label.
     * @param   field_name  Name of the field.
     * @param   field_value The field value.
     *
     * @returns The vertex by unique index.
     */
    VertexIterator GetVertexByUniqueIndex(const std::string& label_name,
                                          const std::string& field_name,
                                          const FieldData& field_value);

    VertexIterator GetVertexByUniqueCompositeIndex(const std::string& label_name,
                                          const std::vector<std::string>& field_name,
                                          const std::vector<FieldData>& field_value);

    /**
     * @brief   Gets edge by unique index. Throws exception if there is no such vertex.
     *
     * @param   label_name  Name of the label.
     * @param   field_name  Name of the field.
     * @param   field_value The field value.
     *
     * @returns The vertex by unique index.
     */
    OutEdgeIterator GetEdgeByUniqueIndex(const std::string& label_name,
                                         const std::string& field_name,
                                         const FieldData& field_value);

    /**
     * @brief   Gets vertex by unique index. Throws exception if there is no such vertex.
     *
     * @param   label_id    Identifier for the label.
     * @param   field_id    Identifier for the field.
     * @param   field_value The field value.
     *
     * @returns The vertex by unique index.
     */
    VertexIterator GetVertexByUniqueIndex(size_t label_id, size_t field_id,
                                          const FieldData& field_value);

    VertexIterator GetVertexByUniqueCompositeIndex(size_t label_id,
                                          const std::vector<size_t>& field_id,
                                          const std::vector<FieldData>& field_value);

    /**
     * @brief   Gets edge by unique index. Throws exception if there is no such vertex.
     *
     * @param   label_id    Identifier for the label.
     * @param   field_id    Identifier for the field.
     * @param   field_value The field value.
     *
     * @returns The vertex by unique index.
     */
    OutEdgeIterator GetEdgeByUniqueIndex(size_t label_id, size_t field_id,
                                         const FieldData& field_value);

    /**
     * @brief   Gets the number of vertices.
     *
     * @returns The nubmer of vertices.
     */
    size_t GetNumVertices();

    /**
     * @brief   Gets vertex primary field
     *
     * @returns The primary field.
     */
    const std::string& GetVertexPrimaryField(const std::string& label);


    /**
     * @brief   Get the total number of vertex and edge
     *
     * @returns std::pair object, first element is vertex number, second is edge number.
     */
    std::pair<uint64_t, uint64_t> Count();

    /**
     * @brief   Get the total number of vertex or edge for each label
     *
     * @returns std::tuple object list, first element indicates whether it is VERTEX or EDGE,
     *          second is label name, third is number.
     */
    std::vector<std::tuple<bool, std::string, int64_t>> CountDetail();
};

}  // namespace lgraph_api
