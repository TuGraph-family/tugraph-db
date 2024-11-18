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
#include <vector>

#include "lgraph/lgraph_types.h"

namespace lgraph {
class AccessControlledDB;
class CppPluginManagerImpl;
}  // namespace lgraph

namespace lgraph_api {

//------------------------------------------------------------------------------
// The following functions are used to determine if a plugin is being killed,
// either due to timeout or due to user request.
//
// Since C++ plugins run in the same process as the server, they cannot be forcely
// killed. We rely on the plugins to check if they are being killed and to stop
// running if they are.
//
// The lgraph_api calls always check for the kill flag when they are called and
// throw an TaskKilled if the flag is set, so the plugin can exit if the
// exception is not caught. However, if the plugin is running a busy loop, the
// plugin writer is responsible for checking the kill flag and exiting if it is set.

/**
 * @brief Determine if we should kill current task.
 *
 * @returns True if a KillTask command was issued for this task, either due to user request or
 *          timeout.
 */
bool ShouldKillThisTask();

/** @brief Defines an alias representing the thread context pointer */
typedef const void *ThreadContextPtr;

/**
 * @brief Gets thread context pointer, which can then be used in ShouldKillThisTask(ctx). Calling
 *        ShouldKillThisTask() is equivalent to ShouldKillThisTask(GetThreadContext()). In
 *        order to save the cost of GetThreadContext(), you can store the context and use it in
 *        ShouldKillThisTask(ctx).
 *
 *        Example: ```c++ ThreadContextPtr ctx = GetThreadContext();
 *        while (HasMoreWorkToDo()) {
 *            if (ShouldKillThisTask(ctx)) {
 *                break;
 *            }
 *            DoWork();
 *        }
 *        ```
 *
 * @returns The thread context.
 */
ThreadContextPtr GetThreadContext();

/**
 * @brief   Determine if we should kill the task currently running in the thread identified by
 *          ctx.
 *
 * @param   ctx The context, as obtained with GetThreadContext.
 *
 * @returns True if a KillTask command was issued for this task.
 */
bool ShouldKillThisTask(ThreadContextPtr ctx);

// End of functions used to determine if a plugin is being killed.
//------------------------------------------------------------------------------

class Transaction;

/**
 * @brief GraphDB represents a graph instance. In TuGraph, each graph instance has its own schema
 *        and access control settings. Accessing a GraphDB without appropriate access rights
 *        yields WriteNotAllowed.
 *
 *        A GraphDB becomes invalid if Close() is called, in which case all transactions and
 *        iterators associated with that GraphDB become invalid. Further operation on that
 *        GraphDB yields InvalidGraphDB.
 */
class GraphDB {
    lgraph::AccessControlledDB *db_;
    bool should_delete_db_ = false;
    bool read_only_ = false;

    /** Copying is disabled. */
    GraphDB(const GraphDB &) = delete;
    GraphDB &operator=(const GraphDB &) = delete;

 public:
    /** For internal use only. Users should use Galaxy::OpenGraph() to get GraphDB. */
    explicit GraphDB(lgraph::AccessControlledDB *db_with_access_control, bool read_only,
                     bool owns_db = false);
    GraphDB(GraphDB &&);
    GraphDB &operator=(GraphDB &&);
    ~GraphDB();

    /**
     * @brief   Close the graph. This will close the graph and release all transactions,
     *          iterators associated with the graph. After calling Close(), the graph becomes
     *          invalid, and cannot be used anymore.
     */
    void Close();

    /**
     * @brief   Creates a read transaction.
     *
     * @exception   InvalidGraphDB Thrown when currently GraphDB is invalid.
     *
     * @returns The new read transaction.
     */
    Transaction CreateReadTxn();

    /**
     * @brief   Creates a write transaction. Write operations can only be performed in write
     *          transactions, otherwise exceptions will be thrown. A write transaction can be
     *          optimistic. Optimistic transactions can run in parallel and any conflict
     *          will be detected during commit. If a transaction conflicts with an ealier one, a
     *          TxnConflict will be thrown during commit.
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                      level.
     *
     * @param   optimistic  (Optional) True to create an optimistic transaction.
     *
     * @returns The new write transaction.
     */
    Transaction CreateWriteTxn(bool optimistic = false);

    /**
     * @brief   Forks a read transaction. The resulting read transaction will share the same view
     *          as the forked one, meaning that when reads are performed on the same vertex/edge,
     *          the results will always be identical, whether they are performed in the original
     *          transaction or the forked one. Only read transactions can be forked. Calling
     *          ForkTxn() on a write txn causes an InvalidFork to be thrown.
     *
     * @exception   InvalidGraphDB Thrown when currently GraphDB is invalid.
     * @exception   InvalidFork    Thrown when txn is a write transaction.
     *
     * @param [in]  txn The read transaction to be forked.
     *
     * @returns A new read Transaction that shares the same view with txn.
     */
    Transaction ForkTxn(Transaction &txn);

    /**
     * @brief   Flushes buffered data to disk. If there have been some async transactions, there
     *          could be data that are written to this graph, but not persisted to disk yet.
     *          Calling Flush() will persist the data and prevent data loss in case of system
     *          crash.
     *
     * @exception   InvalidGraphDB Thrown when currently GraphDB is invalid.
     */
    void Flush();

    /**
     * @brief   Drop all the data in the graph, including labels, indexes and vertexes/edges..
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                      level.
     */
    void DropAllData();

    /**
     * @brief   Drop all vertex and edges but keep the labels and indexes.
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                      level.
     */
    void DropAllVertex();

    /**
     * @brief   Estimate number of vertices. We don't maintain the exact number of vertices, but
     *          only the next vid. This function actually returns the next vid to be used. So if
     *          you have deleted a lot of vertices, the result can be quite different from actual
     *          number of vertices.
     *
     * @exception   InvalidGraphDB Thrown when currently GraphDB is invalid.
     *
     * @returns Estimated number of vertices.
     */
    size_t EstimateNumVertices();

    //********************************
    // Schema modification.
    //********************************

    /**
     * @brief   Adds a vertex label.
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                      level.
     * @exception   InputError              Thrown if the schema is illegal.
     *
     * @param   label           The label name.
     * @param   fds             The field specifications.
     * @param   primary_field   The primary field.
     *
     * @returns True if it succeeds, false if the label already exists.
     */
    bool AddVertexLabel(const std::string &label, const std::vector<FieldSpec> &fds,
                        const VertexOptions& options);

    /**
     * @brief   Deletes a vertex label and all the vertices with this label.
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                      level.
     *
     * @param       label       The label name.
     * @param [out] n_modified  (Optional) If non-null, return the number of deleted vertices.
     *
     * @returns True if it succeeds, false if the label does not exist.
     */
    bool DeleteVertexLabel(const std::string &label, size_t *n_modified = nullptr);

    /**
     * @brief   Deletes fields in a vertex label. This function also updates the vertex data and
     *          indices accordingly to make sure the database remains in consistent state.
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                      level.
     * @exception   InputError              Thrown if field not found, or some fields cannot be
     *                                      deleted.
     *
     * @param       label       The label name.
     * @param       del_fields  Labels of the fields to be deleted.
     * @param [out] n_modified  (Optional) If non-null, return the number of modified vertices.
     *
     * @returns True if it succeeds, false if the label does not exist.
     */
    bool AlterVertexLabelDelFields(const std::string &label,
                                   const std::vector<std::string> &del_fields,
                                   size_t *n_modified = nullptr);

    /**
     * @brief   Add fields to a vertex label. The new fields in existing vertices will be filled
     *          with default values.
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                      level.
     * @exception   InputError              Thrown if field already exists.
     *
     * @param       label           The label name.
     * @param       add_fields      The add fields.
     * @param       default_values  The default values of the newly added fields.
     * @param [out] n_modified      (Optional) If non-null, return the number of modified
     *                               vertices.
     *
     * @returns True if it succeeds, false if the label does not exist.
     */
    bool AlterVertexLabelAddFields(const std::string &label,
                                   const std::vector<FieldSpec> &add_fields,
                                   const std::vector<FieldData> &default_values,
                                   size_t *n_modified = nullptr);

    /**
     * @brief   Modify fields in a vertex label, either chage the data type or optional, or both.
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                      level.
     * @exception   InputError              Thrown if field not found, or is of incompatible data
     *                                      type.
     *
     * @param       label       The label name.
     * @param       mod_fields  The new specification of the modified fields.
     * @param [out] n_modified  (Optional) If non-null, return the number of modified vertices.
     *
     * @returns True if it succeeds, false if the label does not exist.
     */
    bool AlterVertexLabelModFields(const std::string &label,
                                   const std::vector<FieldSpec> &mod_fields,
                                   size_t *n_modified = nullptr);

    /**
     * @brief   Add a edge label, specifying its schema. It is allowed to specify edge constrains,
     *          too. An edge can be bound to several (source_label, destination_label) pairs,
     *          which makes sure this type of edges will only be added between these types of
     *          vertices. By default, the constain is empty, meaning that the edge is not
     *          restricted.
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                      level.
     * @exception   InputError              if invalid schema (invalid specification, re-
     *                                      definition of the same field, etc.).
     *
     * @param   label               The label name.
     * @param   fds                 The field specifications.
     * @param   temporal_field      The temporal field for tid.
     * @param   edge_constraints    (Optional) The edge constraints. An empty constrain means no
     *                              restriction.
     *
     * @returns True if it succeeds, false if label already exists.
     */
    bool AddEdgeLabel(
        const std::string &label, const std::vector<FieldSpec> &fds,
        const EdgeOptions& options);

    /**
     * @brief   Deletes an edge label and all the edges of this type.
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                      level.
     *
     * @param       label       The label.
     * @param [out] n_modified  (Optional) If non-null, return the number of deleted edges.
     *
     * @returns True if it succeeds, false if label does not exist.
     */
    bool DeleteEdgeLabel(const std::string &label, size_t *n_modified = nullptr);

    /**
     * @brief   Modify edge constraint. Existing edges that violate the new constrain will be
     *          removed.
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                      level.
     * @exception   InputError              Thrown if any vertex label does not exist.
     *
     * @param   label       The label name.
     * @param   constraints The new edge constraint.
     *
     * @returns True if it succeeds, false if the edge label does not exist.
     */
    bool AlterLabelModEdgeConstraints(
        const std::string &label,
        const std::vector<std::pair<std::string, std::string>> &constraints);

    /**
     * @brief   Deletes fields in an edge label.
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                      level.
     * @exception   InputError              Thrown if any field does not exist, or cannot be
     *                                      deleted.
     *
     * @param       label       The label name.
     * @param       del_fields  The fields to be deleted.
     * @param [out] n_modified  (Optional) If non-null, return the number of edges modified.
     *
     * @returns True if it succeeds, false if it fails.
     */
    bool AlterEdgeLabelDelFields(const std::string &label,
                                 const std::vector<std::string> &del_fields,
                                 size_t *n_modified = nullptr);

    /**
     * @brief   Add fields to an edge label. The new fields in existing edges will be set to
     *          default values.
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                      level.
     * @exception   InputError              Thrown if any field already exists, or the default
     *                                      value is of incompatible type.
     *
     * @param           label           The label name.
     * @param           add_fields      The fields to add.
     * @param           default_values  The default values.
     * @param [in,out]  n_modified      (Optional) If non-null, return the number of modified
     *                                  edges.
     *
     * @returns True if it succeeds, false if the edge label does not exist.
     */
    bool AlterEdgeLabelAddFields(const std::string &label, const std::vector<FieldSpec> &add_fields,
                                 const std::vector<FieldData> &default_values,
                                 size_t *n_modified = nullptr);

    /**
     * @brief   Modify fields in an edge label. Data type and OPTIONAL can be modified.
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                      level.
     * @exception   InputError              Thrown if any field does not exist, or is of
     *                                      incompatible data type.
     *
     * @param           label       The label name.
     * @param           mod_fields  The modifier fields.
     * @param [in,out]  n_modified  (Optional) If non-null, return the number of modified edges.
     *
     * @returns True if it succeeds, false if the label does not exist.
     */
    bool AlterEdgeLabelModFields(const std::string &label, const std::vector<FieldSpec> &mod_fields,
                                 size_t *n_modified = nullptr);

    //********************************
    // VertexIndex related
    //********************************

    /**
     * @brief   Adds an index to 'label:field'. This function blocks until the index is fully
     *          created.
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                      level.
     * @exception   InputError              Thrown if label:field does not exist, or not
     *                                      indexable.
     *
     * @param   label       The label name.
     * @param   field       The field name.
     * @param   is_unique   True if is unique index, false if not.
     *
     * @returns True if it succeeds, false if the index already exists.
     */
    bool AddVertexIndex(const std::string &label, const std::string &field, IndexType type);

    /**
     * @brief   Adds an index to 'label:field'. This function blocks until the index is fully
     *          created.
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                      level.
     * @exception   InputError              Thrown if label:field does not exist, or not
     *                                      indexable.
     *
     * @param   label       The label.
     * @param   field       The field.
     * @param   is_unique   True if the field content is unique for each vertex.
     *
     * @returns True if it succeeds, false if the index already exists.
     */
    bool AddEdgeIndex(const std::string &label, const std::string &field, IndexType type);

    bool AddVertexCompositeIndex(const std::string& label,
                                 const std::vector<std::string>& fields,
                                 CompositeIndexType type);
     /**
     * @brief   Adds a vector index to 'label:field'. This function blocks until the index is fully
     *          created.
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                      level.
     * @exception   InputError              Thrown if label:field does not exist, or not
     *                                      indexable.
     * @param   is_vertex       vertex or edge.
     * @param   label           The label.
     * @param   field           The field.
     * @param   is_unique       True if the field content is unique for each vertex.
     * @param   index_type      Type of the index
     * @param   vec_dimension   Dimension of the vector
     * @param   distance_type   Type of the distance
     * @param   index_spec      Specification of the index
     * 
     * @returns True if it succeeds, false if the index already exists.
     */
    bool AddVectorIndex(bool is_vertex, const std::string& label, const std::string& field,
                        const std::string& index_type, int vec_dimension,
                        const std::string& distance_type, std::vector<int>& index_spec);

    /**
     * @brief   Check if this vertex_label:field is indexed.
     *
     * @exception   InvalidGraphDB Thrown when currently GraphDB is invalid.
     * @exception   InputError          Thrown if label:field does not exist.
     *
     * @param   label   The label.
     * @param   field   The field.
     *
     * @returns True if index exists, false if label:field exists but not indexed.
     */
    bool IsVertexIndexed(const std::string &label, const std::string &field);

    /**
     * @brief   Check if this edge_label:field is indexed.
     *
     * @exception   InvalidGraphDB Thrown when currently GraphDB is invalid.
     * @exception   InputError          Thrown if label:field does not exist.
     *
     * @param   label   The label.
     * @param   field   The field.
     *
     * @returns True if index exists, false if label:field exists but not indexed.
     */
    bool IsEdgeIndexed(const std::string &label, const std::string &field);

    bool IsVertexCompositeIndexed(const std::string &label,
                                  const std::vector<std::string> &field);

    /**
     * @brief   Deletes the index to 'vertex_label:field'
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                 level.
     * @exception   InputError         Thrown if label or field does not exist.
     *
     * @param   label   The label.
     * @param   field   The field.
     *
     * @returns True if it succeeds, false if the index does not exists.
     */
    bool DeleteVertexIndex(const std::string &label, const std::string &field);

    bool DeleteVertexCompositeIndex(const std::string& label,
                                    const std::vector<std::string>& fields);

    /**
     * @brief   Deletes the index to 'edge_label:field'
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                 level.
     * @exception   InputError         Thrown if label or field does not exist.
     *
     * @param   label   The label.
     * @param   field   The field.
     *
     * @returns True if it succeeds, false if the index does not exists.
     */
    bool DeleteEdgeIndex(const std::string &label, const std::string &field);

    /**
     * @brief   Deletes the vector index to 'vertex_label:field'
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access level.
     * @exception   InputError         Thrown if label or field does not exist.
     *
     * @param   is_vertex       vertex or edge.
     * @param   label           The label.
     * @param   field           The field.
     * 
     * @returns True if it succeeds, false if the index does not exists.
     */
    bool DeleteVectorIndex(bool is_vertex, const std::string& label, const std::string& field);

    /**
     * @brief   Get graph description
     *
     * @exception   InvalidGraphDB Thrown when currently GraphDB is invalid.
     *
     * @returns The description.
     */
    std::string GetDescription() const;

    /**
     * @brief   Get maximum graph size
     *
     * @exception   InvalidGraphDB Thrown when currently GraphDB is invalid.
     *
     * @returns The maximum size.
     */
    size_t GetMaxSize() const;

    /**
     * @brief   Add fulltext index to 'vertex_label:field'
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                 level.
     * @exception   InputError         Thrown if vertex label or field does not exist.
     *
     * @param   vertex_label    The vertex label.
     * @param   field           The field.
     *
     * @returns True if it succeeds, false if the fulltext index already exists.
     */

    bool AddVertexFullTextIndex(const std::string &vertex_label, const std::string &field);

    /**
     * @brief   Add fulltext index to 'edge_label:field'
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                 level.
     * @exception   InputError         Thrown if edge label or field does not exist.
     *
     * @param   edge_label  The edge label.
     * @param   field       The field.
     *
     * @returns True if it succeeds, false if the fulltext index already exists.
     */
    bool AddEdgeFullTextIndex(const std::string &edge_label, const std::string &field);

    /**
     * @brief   Delete the fulltext index of 'vertex_label:field'
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                 level.
     * @exception   InputError         Thrown if vertex label or field does not exist.
     *
     * @param   vertex_label    The vertex label.
     * @param   field           The field.
     *
     * @returns True if it succeeds, false if the fulltext index does not exists.
     */
    bool DeleteVertexFullTextIndex(const std::string &vertex_label, const std::string &field);

    /**
     * @brief   Delete the fulltext index of 'edge_label:field'
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                 level.
     * @exception   InputError         Thrown if edge label or field does not exist.
     *
     * @param   edge_label  The edge label.
     * @param   field       The field.
     *
     * @returns True if it succeeds, false if the fulltext index does not exists.
     */
    bool DeleteEdgeFullTextIndex(const std::string &edge_label, const std::string &field);

    /**
     * @brief   Rebuild the fulltext index of `vertex_labels` and `edge_labels`.
     *
     * @param   vertex_labels   The vertex labels whose fulltext index need to rebuild.
     * @param   edge_labels     The edge labels whose fulltext index need to rebuild.
     */
    void RebuildFullTextIndex(const std::set<std::string> &vertex_labels,
                              const std::set<std::string> &edge_labels);

    /**
     * @brief   List fulltext indexes of vertex and edge
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                 level.
     *
     * @returns Format of returned data : (is_vertex, label_name, property_name)
     */
    std::vector<std::tuple<bool, std::string, std::string>> ListFullTextIndexes();

    /**
     * @brief   Query vertex by fulltext index.
     *
     * @exception   InvalidGraphDB Thrown when currently GraphDB is invalid.
     * @exception   InputError          Thrown if label does not exist.
     *
     * @param   label   The vertex label.
     * @param   query   Lucene query language.
     * @param   top_n   return top n data.
     *
     * @returns Vertex vids and score. Throws exception on error.
     */
    std::vector<std::pair<int64_t, float>> QueryVertexByFullTextIndex(const std::string &label,
                                                                      const std::string &query,
                                                                      int top_n);

    /**
     * @brief   Query edge by fulltext index
     *
     * @exception   InvalidGraphDB Thrown when currently GraphDB is invalid.
     * @exception   InputError          Thrown if label does not exist.
     *
     * @param   label   The edge label.
     * @param   query   Lucene query language.
     * @param   top_n   return top n data.
     *
     * @returns Edge uids and score. Throws exception on error.
     */
    std::vector<std::pair<EdgeUid, float>> QueryEdgeByFullTextIndex(const std::string &label,
                                                                    const std::string &query,
                                                                    int top_n);

    /**
     * @brief   Recount the total number of vertex and edge, stop writing during the count
     *
     * @exception   InvalidGraphDB     Thrown when currently GraphDB is invalid.
     * @exception   WriteNotAllowed    Thrown when called on a GraphDB with read-only access
     *                                 level.
     */
    void RefreshCount();
};

}  // namespace lgraph_api
