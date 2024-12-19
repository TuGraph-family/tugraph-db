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

#pragma once

#include "fma-common/type_traits.h"

#include "core/blob_manager.h"
#include "core/data_type.h"
#include "core/graph.h"
#include "core/kv_store.h"
#include "core/managed_object.h"
#include "core/schema_manager.h"
#include "core/type_convert.h"
#include "core/value.h"

namespace lgraph_api {
class Transaction;
}

namespace lgraph {
class IndexManager;
class PluginManager;
class LibraDB;
class LightningGraph;
class BlobManager;

namespace import_v2 {
class Importer;
class ImportOnline;
}  // namespace import_v2

namespace import_v3 {
class Importer;
}  // namespace import_v3

struct SchemaInfo {
    SchemaManager v_schema_manager;
    SchemaManager e_schema_manager;
};

#define ENABLE_IF_EIT(EIT, RT)                                                   \
    template <typename EIT>                                                      \
    typename std::enable_if<std::is_same<EIT, graph::OutEdgeIterator>::value ||  \
                                std::is_same<EIT, graph::InEdgeIterator>::value, \
                            RT>::type

#define IS_EIT_TYPE(T) \
    fma_common::IsType<T, ::lgraph::graph::InEdgeIterator, ::lgraph::graph::OutEdgeIterator>::value

class Transaction {
    friend class IndexManager;
    friend class PluginManager;
    friend class LightningGraph;
    friend class lgraph_api::Transaction;
    friend class ::lgraph::import_v2::Importer;
    friend class ::lgraph::import_v2::ImportOnline;
    friend class ::lgraph::import_v3::Importer;

    std::unique_ptr<KvTransaction> txn_;
    bool read_only_ = false;

    LightningGraph* db_ = nullptr;
    ScopedRef<SchemaInfo> managed_schema_ptr_;
    SchemaInfo* curr_schema_ = nullptr;
    graph::Graph* graph_ = nullptr;

    IndexManager* index_manager_ = nullptr;
    BlobManager* blob_manager_ = nullptr;
    std::vector<IteratorBase*> iterators_;
    FullTextIndex* fulltext_index_;
    std::vector<FTIndexEntry> fulltext_buffers_;
    std::vector<VectorIndexEntry> vector_buffers_;
    std::unordered_map<LabelId, int64_t> vertex_delta_count_;
    std::unordered_map<LabelId, int64_t> edge_delta_count_;
    std::set<LabelId> vertex_label_delete_;
    std::set<LabelId> edge_label_delete_;
    void ThrowIfReadOnlyTxn() const {
        if (read_only_)
            THROW_CODE(WriteNotAllowed,
                "Write operation not allowed in read-only transaction.");
    }

    DISABLE_COPY(Transaction);

    /**
     * Constructor
     *
     * @param  read_only        True if this is a read-only transaction.
     * @param  optimistic       True to begin an optimistic write transaction.
     * @param  get_lock         Whether to acquire db write lock in transaction.
     * @param  db               Pointer to the db.
     */
    Transaction(bool read_only, bool optimistic, LightningGraph* db);

    // used in ForkTxn
    Transaction(LightningGraph* db, KvTransaction& txn);

 public:
    typedef graph::InEdgeIterator InEdgeIterator;
    typedef graph::OutEdgeIterator OutEdgeIterator;
    typedef graph::VertexIterator VertexIterator;

    Transaction(Transaction&& rhs);

    Transaction& operator=(Transaction&& rhs);

    ~Transaction();

    /**
     * Get a vertex iterator pointing to the first vertex. If there is no vertex,
     * the iterator is invalid.
     *
     * \return  The vertex iterator.
     */
    VertexIterator GetVertexIterator() { return graph_->GetVertexIterator(this); }

    /**
     * Gets a vertex iterator pointing to the Vertex with vid. If the vertex does
     * not exist, the iterator is invalid.
     *
     * \param   vid The vid.
     *
     * \return  The vertex iterator.
     */
    VertexIterator GetVertexIterator(VertexId vid, bool nearest = false) {
        _detail::CheckVid(vid);
        return graph_->GetVertexIterator(this, vid, nearest);
    }

    /**
     * Gets an out edge iterator pointing to [src->dst | eid]. If (dst == -1),
     * points to the first out edge from src. If (dst != -1 &amp;&amp; eid == -1),
     * points to the first edge from src- >dst.
     *
     * \param   src Source for the.
     * \param   dst (Optional) Destination for the.
     * \param   eid (Optional) The eid.
     *
     * \return  The out edge iterator.
     */

    OutEdgeIterator GetOutEdgeIterator(const EdgeUid& euid, bool nearest) {
        _detail::CheckEdgeUid(euid);
        return graph_->GetOutEdgeIterator(this, euid, nearest);
    }

    OutEdgeIterator GetOutEdgeIterator(VertexId src) {
        _detail::CheckVid(src);
        return graph_->GetOutEdgeIterator(this, EdgeUid(src, 0, 0, 0, 0), true);
    }

    InEdgeIterator GetInEdgeIterator(const EdgeUid& euid, bool nearest) {
        _detail::CheckEdgeUid(euid);
        return graph_->GetInEdgeIterator(this, euid, nearest);
    }

    InEdgeIterator GetInEdgeIterator(VertexId dst) {
        _detail::CheckVid(dst);
        return graph_->GetInEdgeIterator(this, EdgeUid(0, dst, 0, 0, 0), true);
    }

    /**
     * Gets vertex label
     *
     * \param [in,out]  it  The iterator.
     *
     * \return  The vertex label.
     */
    const std::string& GetVertexLabel(const VertexIterator& it) const {
        return curr_schema_->v_schema_manager.GetRecordLabel(it.GetProperty());
    }

    /**
     * Gets vertex label identifier
     *
     * \param [in,out]  it  The iterator.
     *
     * \return  The vertex label identifier.
     */
    LabelId GetVertexLabelId(const VertexIterator& it) const {
        return SchemaManager::GetRecordLabelId(it.GetProperty());
    }

    /**
     * Gets edge label.
     *
     * \param   it  The edge iterator.
     *
     * \return  The edge label.
     */
    ENABLE_IF_EIT(EIT, const std::string&)
    GetEdgeLabel(const EIT& it) const {
        return curr_schema_->e_schema_manager.GetSchema(it.GetLabelId())->GetLabel();
    }

    /**
     * Gets edge label id.
     *
     * \param   it  The edge iterator.
     *
     * \return  The edge label.
     */
    ENABLE_IF_EIT(EIT, LabelId)
    GetEdgeLabelId(const EIT& it) const { return it.GetLabelId(); }

    /**
     * Gets vertex schema
     *
     * \param [in,out]  it  The iterator.
     *
     * \return  vector of FieldDef
     */
    std::vector<FieldSpec> GetVertexSchema(const graph::VertexIterator& it) const {
        const Schema* schema = curr_schema_->v_schema_manager.GetSchema(it.GetProperty());
        FMA_DBG_ASSERT(schema);
        return schema->GetFieldSpecs();
    }

    /**
     * Gets schema
     *
     * \param   label   The label name.
     * \param   is_vertex   Vertex or Edge label.
     *
     * \return  Schema
     */
    const Schema* GetSchema(const std::string& label, bool is_vertex) const {
        const Schema* schema = is_vertex ? curr_schema_->v_schema_manager.GetSchema(label)
                                         : curr_schema_->e_schema_manager.GetSchema(label);
        return schema;
    }

    /**
     * Gets schema
     *
     * \param   lid   The label id.
     * \param   is_vertex   Vertex or Edge label.
     *
     * \return  Schema
     */
    const Schema* GetSchema(const LabelId& lid, bool is_vertex) const {
        const Schema* schema = is_vertex ? curr_schema_->v_schema_manager.GetSchema(lid)
                                         : curr_schema_->e_schema_manager.GetSchema(lid);
        return schema;
    }

    /**
     * Gets edge schema
     *
     * \param   it  The iterator.
     *
     * \return  The edge schema.
     */
    ENABLE_IF_EIT(EIT, std::vector<FieldSpec>)
    GetEdgeSchema(const EIT& it) {
        Schema* schema = curr_schema_->e_schema_manager.GetSchema(it.GetLabelId());
        FMA_DBG_ASSERT(schema);
        return schema->GetFieldSpecs();
    }

    /**
     * Query if this transaction is read only
     *
     * \return  True if read only, false if not.
     */
    bool IsReadOnly() const { return read_only_; }

    /**
     * Sets vertex property (exclude label)
     *
     * \tparam  FieldT  Type of the field specifier, can be size_t or std::string.
     * \tparam  DataT   Type of the data, can be std::string or FieldData.
     *
     * \param [in,out]  it          The vertex iterator.
     * \param           n_fields    The number of fields.
     * \param           fields      The field specifiers.
     * \param           values      The values.
     */
    template <typename FieldT, typename DataT>
    typename std::enable_if<IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT), void>::type
    SetVertexProperty(VertexIterator& it, size_t n_fields, const FieldT* fields,
                      const DataT* values);

    /**
     * Sets vertex property (exclude label)
     *
     * \tparam  FieldT  Type of the field specifier.
     * \tparam  DataT   Type of the data.
     *
     * \param [in,out]  it      The vertex iterator.
     * \param           fields  The field specifiers.
     * \param           data    The values.
     */
    template <typename FieldT, typename DataT>
    typename std::enable_if<IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT), void>::type
    SetVertexProperty(graph::VertexIterator& it, const std::vector<FieldT>& fields,
                      const std::vector<DataT>& data) {
        if (fields.size() != data.size())
            THROW_CODE(InputError, "Number of fields and number of data values does not match");
        SetVertexProperty(it, fields.size(), fields.data(), data.data());
    }

    /**
     * Gets vertex field
     *
     * \tparam  FieldT  Type of the field specifier, std::string or size_t.
     * \param   it  The vertex iterator.
     * \param   fd  The field specifier. i.e. name or index of the field.
     *
     * \return  The vertex field.
     */
    template <typename FieldT>
    typename std::enable_if<IS_FIELD_TYPE(FieldT), FieldData>::type GetVertexField(
        const VertexIterator& it, const FieldT& fd);

    /**
     * Gets vertex fields implementation
     *
     * \tparam  FieldT  Type of the field specifier, can be std::string or size_t.
     * \param [in,out]  it          The iterator.
     * \param           n_fields    The fields.
     * \param           fds         The fds.
     * \param [in,out]  fields      If non-null, the fields.
     */
    template <typename FieldT>
    typename std::enable_if<IS_FIELD_TYPE(FieldT), void>::type GetVertexFields(
        const graph::VertexIterator& it, size_t n_fields, const FieldT* fds, FieldData* fields);

    /**
     * Gets vertex fields
     *
     * \tparam  FieldT  Type of the field speicifier, can be std::string or
     * size_t. \param   it  The iterator. \param   fds The fds.
     *
     * \return  A vector of vertex fields.
     */
    template <typename FieldT>
    typename std::enable_if<IS_FIELD_TYPE(FieldT), std::vector<FieldData>>::type GetVertexFields(
        const VertexIterator& it, const std::vector<FieldT>& fds) {
        std::vector<FieldData> values(fds.size());
        GetVertexFields(it, fds.size(), &fds[0], &values[0]);
        return values;
    }

    // get all the fields of a vertex
    std::vector<std::pair<std::string, FieldData>> GetVertexFields(const VertexIterator& it);

    /**
     * Gets edge fields
     *
     * \tparam  E   Edge iterator type, InEdgeIterator or OutEdgeIterator.
     * \tparam  T   Field specifier type, std::string or size_t.
     * \param [in,out]  it          The iterator.
     * \param           n_fields    The fields.
     * \param           fds         The fds.
     * \param [in,out]  fields      If non-null, the fields.
     */
    template <typename E, typename T>
    typename std::enable_if<IS_EIT_TYPE(E) && IS_FIELD_TYPE(T), void>::type GetEdgeFields(
        const E& it, size_t n_fields, const T* fds, FieldData* fields);

    template <typename E, typename T>
    typename std::enable_if<IS_EIT_TYPE(E) && IS_FIELD_TYPE(T), std::vector<FieldData>>::type
    GetEdgeFields(const E& it, const std::vector<T>& fds) {
        std::vector<FieldData> fields(fds.size());
        GetEdgeFields(it, fds.size(), fds.data(), &fields[0]);
        return fields;
    }

    // get all the fields of the edge
    template <typename EIT>
    typename std::enable_if<IS_EIT_TYPE(EIT), std::vector<std::pair<std::string, FieldData>>>::type
    GetEdgeFields(const EIT& it);

    /**
     * Gets edge field
     *
     * \tparam  E   Edge iterator type, InEdgeIterator or OutEdgeIterator.
     * \tparam  T   Field specifier type, std::string or size_t.
     * \param [in,out]  it  The iterator.
     * \param           fd  The fd.
     *
     * \return  The edge field.
     */
    template <typename E, typename T>
    typename std::enable_if<IS_EIT_TYPE(E) && IS_FIELD_TYPE(T), FieldData>::type GetEdgeField(
        const E& it, const T& fd);

    /**
     * Sets edge property
     *
     * \tparam  EIT     Type of the edge iterator, InEdgeIterator or
     * OutEdgeIterator. \tparam  FieldT  Type of the field specifier, std::string
     * or size_t. \tparam  DataT   Type of the data t, std::string or FieldData
     * \param [in,out]  it          The iterator.
     * \param           n_fields    The fields.
     * \param           fields      The fields.
     * \param           values      The values.
     */
    template <typename EIT, typename FieldT, typename DataT>
    typename std::enable_if<IS_EIT_TYPE(EIT) && IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT),
                            void>::type
    SetEdgeProperty(EIT& it, size_t n_fields, const FieldT* fields, const DataT* values);

    /**
     * Sets edge property (excluing label)
     *
     * \tparam  EIT     Type of the edge iterator, InEdgeIterator or
     * OutEdgeIterator. \tparam  FieldT  Type of the field, std::string or size_t.
     * \tparam  DataT   Type of the data, std::string or FieldData.
     * \param [in,out]  it      The vertex iterator.
     * \param           fields  The field specifiers.
     * \param           values  The values.
     */
    template <typename EIT, typename FieldT, typename DataT>
    typename std::enable_if<IS_EIT_TYPE(EIT) && IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT),
                            void>::type
    SetEdgeProperty(EIT& it, const std::vector<FieldT>& fields, const std::vector<DataT>& values) {
        if (fields.size() != values.size())
            THROW_CODE(InputError, "Number of fields and number of data values does not match");
        return SetEdgeProperty(it, fields.size(), fields.data(), values.data());
    }

    /**
     * Gets number labels
     *
     * \param   is_vertex   True to return number of vertex labels, otherwise
     * returns edge labels.
     *
     * \return  The number labels.
     */
    size_t GetNumLabels(bool is_vertex) {
        return is_vertex ? curr_schema_->v_schema_manager.GetNumLabels()
                         : curr_schema_->e_schema_manager.GetNumLabels();
    }

    /**
     * Gets all labels
     *
     * \param   is_vertex   True to return number of vertex labels, otherwise
     * returns edge labels.
     *
     * \return  Label names.
     */
    std::vector<std::string> GetAllLabels(bool is_vertex) {
        SchemaManager& sm =
            is_vertex ? curr_schema_->v_schema_manager : curr_schema_->e_schema_manager;
        return sm.GetAllLabels();
    }

    /**
     * Gets LabelId.
     *
     * \param   is_vertex   True to return number of vertex labels, otherwise
     * returns edge labels. \param   label       The label name.
     *
     * \return  The LabelId.
     */
    LabelId GetLabelId(bool is_vertex, const std::string& label) {
        SchemaManager* sm =
            is_vertex ? &curr_schema_->v_schema_manager : &curr_schema_->e_schema_manager;
        return sm->GetLabelId(label);
    }

    LabelId GetLabelId(bool is_vertex, size_t label) {
        _detail::CheckLid(label);
        return (LabelId)label;
    }

    /**
     * Gets schema
     *
     * \tparam  T   Label specifier type, std::string or size_t
     * \param   is_vertex   True if this object is vertex.
     * \param   label       The label.
     *
     * \return  The schema implementation.
     */
    template <typename T>
    typename std::enable_if<IS_LABEL_TYPE(T), std::vector<FieldSpec>>::type GetSchema(
        bool is_vertex, const T& label) {
        SchemaManager& sm =
            is_vertex ? curr_schema_->v_schema_manager : curr_schema_->e_schema_manager;
        Schema* schema = sm.GetSchema(label);
        if (!schema)
            THROW_CODE(InputError, "{} Label \"{}\" does not exist.",
                                  is_vertex ? "vertex" : "edge", label);
        return schema->GetAliveFieldSpecs();
    }

    std::map<std::string, FieldSpec> GetSchemaAsMap(bool is_vertex, const std::string& label) {
        SchemaManager& sm =
            is_vertex ? curr_schema_->v_schema_manager : curr_schema_->e_schema_manager;
        Schema* schema = sm.GetSchema(label);
        if (!schema)
            THROW_CODE(InputError, "Label \"{}\" does not exist.", label);
        return schema->GetAliveFieldSpecsAsMap();
    }

    const std::string& GetVertexPrimaryField(const std::string& label) {
        Schema* schema = curr_schema_->v_schema_manager.GetSchema(label);
        if (!schema)
            THROW_CODE(InputError, "Vertex label \"{}\" does not exist.", label);
        return schema->GetPrimaryField();
    }

    bool HasTemporalField(const std::string& label) {
        Schema* schema = curr_schema_->e_schema_manager.GetSchema(label);
        if (!schema)
            THROW_CODE(InputError, "Edge label \"{}\" does not exist.", label);
        return schema->HasTemporalField();
    }

    const std::string& GetEdgeTemporalField(const std::string& label) {
        Schema* schema = curr_schema_->e_schema_manager.GetSchema(label);
        if (!schema)
            THROW_CODE(InputError, "Edge label \"{}\" does not exist.", label);
        return schema->GetTemporalField();
    }

    const EdgeConstraints& GetEdgeConstraints(const std::string& label) {
        Schema* schema = curr_schema_->e_schema_manager.GetSchema(label);
        if (!schema)
            THROW_CODE(InputError, "Edge Label \"{}\" does not exist.", label);
        return schema->GetEdgeConstraints();
    }

    /**
     * Gets field identifier
     *
     * \param   is_vertex   True to indicate this is vertex label, otherwise edge
     * label. \param   label       The label. \param   field       The field.
     *
     * \return  The field identifier.
     */
    size_t GetFieldId(bool is_vertex, const std::string& label, const std::string& field);

    /**
     * Gets field identifiers
     *
     * \param   is_vertex   True to indicate this is vertex label, otherwise edge
     * label. \param   label_id    Identifier for the label. \param   field_names
     * List of names of the fields.
     *
     * \return  The field identifiers.
     */
    std::vector<size_t> GetFieldIds(bool is_vertex, size_t label_id,
                                    const std::vector<std::string>& field_names);

    size_t GetFieldId(bool is_vertex, size_t label_id, const std::string& field_name);

    /**
     * Gets vertex field
     *
     * \tparam  FieldT  Type of the field specifier, std::string or size_t.
     * \param   vid The vid.
     * \param   fd  The fd.
     *
     * \return  The vertex field.
     */
    template <typename FieldT>
    typename std::enable_if<IS_FIELD_TYPE(FieldT), FieldData>::type GetVertexField(
        VertexId vid, const FieldT& fd);

    /**
     * Gets vertex fields
     *
     * \tparam  FieldT  Type of the field specifier, std::string or size_t.
     * \param   vid The vid.
     * \param   fds The fds.
     *
     * \return  A vector of vertex fields.
     */
    template <typename FieldT>
    typename std::enable_if<IS_FIELD_TYPE(FieldT), std::vector<FieldData>>::type GetVertexFields(
        VertexId vid, const std::vector<FieldT>& fds);

    /**
     * List out edges from src
     *
     * \param   src     The vid.
     *
     * \return  A vector of [dst, eid] pairs.
     */
    std::vector<EdgeUid> ListOutEdges(VertexId src);

    /**
     * List in-coming edges of dst.
     *
     * \param   dst     The vid.
     *
     * \return  A vector of [src, eid] pairs.
     */
    std::vector<EdgeUid> ListInEdges(VertexId dst);

    /**
     * Gets edge field
     *
     * \tparam  FieldT  Type of the field specifier, std::string or size_t.
     * \param   src Source for the.
     * \param   dst Destination for the.
     * \param   eid The eid.
     * \param   fd  The fd.
     *
     * \return  The edge field.
     */
    template <typename FieldT>
    typename std::enable_if<IS_FIELD_TYPE(FieldT), FieldData>::type GetEdgeField(const EdgeUid& uid,
                                                                                 const FieldT& fd);

    template <typename FieldT>
    typename std::enable_if<IS_FIELD_TYPE(FieldT), void>::type GetEdgeFields(const EdgeUid& uid,
                                                                             const size_t n_fields,
                                                                             const FieldT* fds,
                                                                             FieldData* fields);

    template <typename FieldT>
    typename std::enable_if<IS_FIELD_TYPE(FieldT), std::vector<FieldData>>::type GetEdgeFields(
        const EdgeUid& uid, const std::vector<FieldT>& fds) {
        _detail::CheckEdgeUid(uid);
        auto eit = graph_->GetUnmanagedOutEdgeIterator(txn_.get(), uid, false);
        if (!eit.IsValid()) THROW_CODE(InputError, "Edge does not exist");
        return GetEdgeFields(eit, fds);
    }

    /** Commits this transaction. */
    void Commit();

    /** Aborts this transaction. */
    void Abort();

    /**
     * Query if this object is valid. Transaction becomes invalid after calling
     * Abort() or Commit(). Operations on invalid transaction yields exceptions.
     *
     * \return  True if valid, false if not.
     */
    bool IsValid() const { return txn_ != nullptr; }

    /**
     * Adds a vertex to the DB.
     *
     * \tparam  LabelT  Type of the label specifier, could be size_t or
     * std::string. \tparam  FieldT  Type of the field specifier, could be size_t
     * or std::string. \tparam  DataT   Type of the data, could be std::string or
     * FieldData
     *
     * \param   label       The label.
     * \param   fields      The field specifiers.
     * \param   values      The values.
     *
     * \return  VertexId of the new vertex.
     */
    template <typename LabelT, typename FieldT, typename DataT>
    typename std::enable_if<IS_LABEL_TYPE(LabelT) && IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT),
                            VertexId>::type
    AddVertex(const LabelT& label, size_t n_fields, const FieldT* fields, const DataT* values);

    template <typename LabelT, typename FieldT, typename DataT>
    typename std::enable_if<IS_LABEL_TYPE(LabelT) && IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT),
                            VertexId>::type
    AddVertex(const LabelT& label, const std::vector<FieldT>& fields,
              const std::vector<DataT>& values) {
        if (fields.size() != values.size())
            THROW_CODE(InputError, "Number of fields and data values do not match");
        return AddVertex(label, fields.size(), fields.data(), values.data());
    }

    /**
     * Deletes the vertex pointed to by the iterator.
     *
     * \param [in,out]  it      The vertex iterator.
     * \param [in,out]  n_in    Returns the number of in-coming edges.
     * \param [in,out]  n_out   Returns the number of out-going edges.
     */
    void DeleteVertex(graph::VertexIterator& it, size_t* n_in = nullptr, size_t* n_out = nullptr);

    /**
     * Deletes the vertex specified by vid.
     *
     * \param           vid     The vid.
     * \param [in,out]  n_in    Returns the number of in-coming edges.
     * \param [in,out]  n_out   Returns the number of out-going edges.
     *
     * \return  True if it succeeds, false if the vertex does not exist.
     */
    bool DeleteVertex(VertexId vid, size_t* n_in = nullptr, size_t* n_out = nullptr);

    /**
     * Adds an edge
     *
     * \tparam  LabelT  Type of the label t.
     * \tparam  FieldT  Type of the field t.
     * \tparam  DataT   Type of the data t.
     * \param   src         Source for the.
     * \param   dst         Destination for the.
     * \param   label       The label.
     * \param   n_fields    The fields.
     * \param   fields      The fields.
     * \param   values      The values.
     *
     * \return  EdgeId of the new edge.
     */
    template <typename LabelT, typename FieldT, typename DataT>
    typename std::enable_if<IS_LABEL_TYPE(LabelT) && IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT),
                            EdgeUid>::type
    AddEdge(VertexId src, VertexId dst, const LabelT& label, size_t n_fields, const FieldT* fields,
            const DataT* values);

    /**
     * Adds an edge to the DB.
     *
     * \tparam  LabelT  Type of the label specifier, could be size_t or
     * std::string. \tparam  FieldT  Type of the field specifier, could be size_t
     * or std::string. \tparam  DataT   Type of the data, could be std::string or
     * FieldData
     *
     * \param   src         Source vertex id.
     * \param   dst         Destination vertex id.
     * \param   label       The label.
     * \param   fields      The field specifiers. i.e. name or index of the
     * fields. \param   values      The values.
     *
     * \return  EdgeId of the new edge.
     */
    template <typename LabelT, typename FieldT, typename DataT>
    typename std::enable_if<IS_LABEL_TYPE(LabelT) && IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT),
                            EdgeUid>::type
    AddEdge(VertexId src, VertexId dst, const LabelT& label, const std::vector<FieldT>& fields,
            const std::vector<DataT>& values) {
        _detail::CheckVid(src);
        _detail::CheckVid(dst);
        if (fields.size() != values.size())
            THROW_CODE(InputError, "Number of fields and data values do not match");
        return AddEdge(src, dst, label, fields.size(), fields.data(), values.data());
    }

    template <typename LabelT, typename FieldT, typename DataT>
    typename std::enable_if<IS_LABEL_TYPE(LabelT) && IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT),
                            bool>::type
    UpsertEdge(VertexId src, VertexId dst, const LabelT& label, size_t n_fields,
               const FieldT* fields, const DataT* values);

    template <typename LabelT, typename FieldT, typename DataT>
    typename std::enable_if<IS_LABEL_TYPE(LabelT) && IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT),
                            bool>::type
    UpsertEdge(VertexId src, VertexId dst, const LabelT& label, const std::vector<FieldT>& fields,
               const std::vector<DataT>& values) {
        if (fields.size() != values.size())
            THROW_CODE(InputError, "Number of fields and data values do not match");
        return UpsertEdge(src, dst, label, fields.size(), fields.data(), values.data());
    }

    ENABLE_IF_EIT(EIT, void)
    DeleteEdge(EIT& eit);

    /**
     * Deltes the edge [src->dst | eid]
     *
     * \param   src Source for the edge.
     * \param   dst Destination for the edge.
     * \param   eid The EdgeId of the edge.
     *
     * \return  True if it succeeds, false if the edge does not exist.
     */
    bool DeleteEdge(const EdgeUid& uid);

    /**
     * Sets vertex property (excluding label)
     *
     * \param   id          The Vertex id.
     * \param   n_fields    The number of fields.
     * \param   fields      The field specifiers. i.e. name or index of the
     * fields. \param   values      The values.
     *
     * \return True if it succeeds, false if it fails.
     */
    template <typename FieldT, typename DataT>
    typename std::enable_if<IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT), bool>::type
    SetVertexProperty(VertexId id, size_t n_fields, const FieldT* fields, const DataT* values) {
        _detail::CheckVid(id);
        ThrowIfReadOnlyTxn();
        VertexIterator it = GetVertexIterator(id);
        if (!it.IsValid()) return false;
        SetVertexProperty(it, n_fields, fields, values);
        return true;
    }

    /**
     * Sets vertex property (excluding label)
     *
     * \param   id          The Vertex id.
     * \param   fields      The field specifiers. i.e. name or index of the
     * fields. \param   values      The values.
     *
     * \return True if it succeeds, false if it fails.
     */
    template <typename FieldT, typename DataT>
    typename std::enable_if<IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT), bool>::type
    SetVertexProperty(VertexId id, const std::vector<FieldT>& fields,
                      const std::vector<DataT>& values) {
        _detail::CheckVid(id);
        if (fields.size() != values.size())
            THROW_CODE(InputError, "Number of fields and data values do not match");
        return SetVertexProperty(id, fields.size(), fields.data(), values.data());
    }

    /**
     * Sets edge property (excluding label)
     *
     * \param   src         Source of the edge.
     * \param   dst         Destination of the edge.
     * \param   eid         Edge identifier.
     * \param   n_fields    The number of fields.
     * \param   fields      The field specifiers. i.e. name or index of the
     * fields. \param   values      The values.
     *
     * \return True if it succeeds, false if it fails.
     */
    template <typename FieldT, typename DataT>
    typename std::enable_if<IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT), bool>::type
    SetEdgeProperty(const EdgeUid& uid, size_t n_fields, const FieldT* fields,
                    const DataT* values) {
        _detail::CheckEdgeUid(uid);
        ThrowIfReadOnlyTxn();
        OutEdgeIterator eit = GetOutEdgeIterator(uid, false);
        if (!eit.IsValid()) return false;
        SetEdgeProperty(eit, n_fields, fields, values);
        return true;
    }

    /**
     * Sets edge property (excluding label)
     *
     * \param   src         Source of the edge.
     * \param   dst         Destination of the edge.
     * \param   eid         Edge identifier.
     * \param   fields      The field specifiers. i.e. name or index of the
     * fields. \param   values      The values.
     *
     * \return True if it succeeds, false if it fails.
     */
    template <typename FieldT, typename DataT>
    typename std::enable_if<IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT), bool>::type
    SetEdgeProperty(const EdgeUid& uid, const std::vector<FieldT>& fields,
                    const std::vector<DataT>& values) {
        _detail::CheckEdgeUid(uid);
        if (fields.size() != values.size())
            THROW_CODE(InputError, "Number of fields and data values do not match");
        return SetEdgeProperty(uid, fields.size(), fields.data(), values.data());
    }

    /**
     * List indexes
     *
     * \return  A vector of IndexSpec
     */
    std::vector<IndexSpec> ListVertexIndexes();

    std::vector<IndexSpec> ListEdgeIndexes();

    std::vector<CompositeIndexSpec> ListVertexCompositeIndexes();

    // list index by label
    std::vector<IndexSpec> ListVertexIndexByLabel(const std::string& label);

    std::vector<IndexSpec> ListEdgeIndexByLabel(const std::string& label);

    std::vector<CompositeIndexSpec> ListVertexCompositeIndexByLabel(const std::string& label);

    std::vector<std::tuple<bool, std::string, std::string>> ListFullTextIndexes();

    std::vector<std::tuple<bool, std::string, int64_t>> countDetail();

    VectorIndex* GetVertexVectorIndex(const std::string& label, const std::string& field);

    /**
     * Check if index is ready.
     *
     * \param   label   The label of index.
     * \param   field   The field of index.
     *
     * \return  True if the index is ready.
     */
    bool IsIndexed(const std::string& label, const std::string& field);

    /**
     * Gets index iterator. The iterator has field value [key_start, key_end].
     * So key_start=key_end=v returns an iterator pointing to all vertexes
     * that has field value v.
     *
     * \param   label       The label.
     * \param   field       The field.
     * \param   key_start   The key start.
     * \param   key_end     The key end, inclusive.
     *
     * \return  The index iterator.
     */
    VertexIndexIterator GetVertexIndexIterator(const std::string& label, const std::string& field,
                                               const FieldData& key_start = FieldData(),
                                               const FieldData& key_end = FieldData());

    VertexIndexIterator GetVertexIndexIterator(size_t label_id, size_t field_id,
                                               const FieldData& key_start = FieldData(),
                                               const FieldData& key_end = FieldData());

    /**
     * Gets index iterator. The iterator has field value [key_start, key_end].
     * So key_start=key_end=v returns an iterator pointing to all vertexes
     * that has field value v.
     *
     * \param   label       The label.
     * \param   field       The field.
     * \param   key_start   The key start.
     * \param   key_end     The key end.
     *
     * \return  The index iterator.
     */
    VertexIndexIterator GetVertexIndexIterator(const std::string& label, const std::string& field,
                                               const std::string& key_start,
                                               const std::string& key_end);

    EdgeIndexIterator GetEdgeIndexIterator(const std::string& label, const std::string& field,
                                   const FieldData& key_start = FieldData(),
                                   const FieldData& key_end = FieldData());

    EdgeIndexIterator GetEdgeIndexIterator(size_t label_id, size_t field_id,
                                   const FieldData& key_start = FieldData(),
                                   const FieldData& key_end = FieldData());

    EdgeIndexIterator GetEdgePairUniqueIndexIterator(
        size_t label_id, size_t field_id, VertexId src_vid, VertexId dst_vid,
        const FieldData& key_start, const FieldData& key_end);

    EdgeIndexIterator GetEdgeIndexIterator(const std::string& label, const std::string& field,
                                   const std::string& key_start, const std::string& key_end);

    CompositeIndexIterator GetVertexCompositeIndexIterator(const std::string& label,
                                   const std::vector<std::string>& fields,
                                   const std::vector<FieldData>& key_start,
                                   const std::vector<FieldData>& key_end);

    CompositeIndexIterator GetVertexCompositeIndexIterator(const size_t& label,
                                   const std::vector<size_t>& field_ids,
                                   const std::vector<FieldData>& key_start,
                                   const std::vector<FieldData>& key_end);

    CompositeIndexIterator GetVertexCompositeIndexIterator(const std::string& label,
                                   const std::vector<std::string>& fields,
                                   const std::vector<std::string>& key_start,
                                   const std::vector<std::string>& key_end);


    /**
     * Gets the string representation of the vertex.
     *
     * \param vid  Vertex id.
     *
     * \return  String representation of the vertex.
     */
    std::string VertexToString(const VertexIterator& vit);

    ENABLE_IF_EIT(EIT, std::string)
    EdgeToString(const EIT& eit) {
        Value prop = eit.GetProperty();
        auto schema = curr_schema_->e_schema_manager.GetSchema(eit.GetLabelId());
        if (schema->DetachProperty()) {
            prop = schema->GetDetachedEdgeProperty(*txn_, eit.GetUid());
        }
        return fma_common::StringFormatter::Format(
            "E[{}]: DST = {}, EP = {}", eit.GetEdgeId(), eit.GetDst(),
            curr_schema_->e_schema_manager.DumpRecord(prop, eit.GetLabelId()));
    }

#ifdef _USELESS_CODE
    void ImportEdgesRaw(VertexId src, size_t label_id, const std::vector<VertexId>& dsts,
                        const std::vector<size_t> field_ids,
                        const std::vector<std::vector<FieldData>>& edge_data,
                        const std::vector<VertexId>& inrefs) {
        std::vector<Value> props;
        props.reserve(edge_data.size());
        auto& sm = curr_schema_->e_schema_manager;
        for (auto& fds : edge_data) {
            Value record;
            sm.CreateEmptyRecord(record, label_id, field_ids.size(), field_ids.data(), fds.data());
            props.emplace_back(std::move(record));
        }
        graph_->AddEdgesRaw(txn_, src, dsts, props, inrefs);
    }

    /**
     * Import vertex data. Raw interface, used by import_from_scratch only.
     *
     * \param vdata     Source for the.
     * \param id        Identifier for the label.
     * \param edge_data Information describing the edge.
     * \param inrefs    The inrefs.
     */
    void ImportVertexDataRaw(const std::string& vdata, VertexId id,
                             const std::vector<std::pair<VertexId, std::string>>& edge_data,
                             const std::vector<VertexId>& inrefs) {
        graph_->ImportVertexDataRaw(txn_, vdata, id, edge_data, inrefs);
    }
#endif
    // append keys and values into the graph, used only when creating a graph from scratch
    // MUST call RefreshNextVid() after import, otherwise next vid will be incorrect
    // key: key
    // value: value
    // blobs: list of (BlobKey, BlobValue)
    void ImportAppendDataRaw(const Value& key, const Value& value);

    // Refresh next vid stored in graph::meta_table.
    // Should be called after ImportAppendDataRaw() since import does not call VertexAdd()
    // which correctly sets next vid.
    void RefreshNextVid();

    BlobManager::BlobKey _GetNextBlobKey();

    std::string _OnlineImportBatchAddVertexes(
        Schema* schema, const std::vector<Value>& vprops,
        const std::vector<std::pair<BlobManager::BlobKey, Value>>& blobs, bool continue_on_error);

    struct EdgeDataForTheSameVertex {
        VertexId vid;
        std::vector<std::tuple<LabelId, VertexId, TemporalId, Value>> outs;
        std::vector<std::tuple<LabelId, VertexId, TemporalId, Value>> ins;
    };

    std::string _OnlineImportBatchAddEdges(
        const std::vector<EdgeDataForTheSameVertex>& data,
        const std::vector<std::pair<BlobManager::BlobKey, Value>>& blobs, bool continue_on_error,
        Schema* schema);

    void _BatchAddBlobs(const std::vector<std::pair<BlobManager::BlobKey, Value>>& blobs);

    const SchemaInfo& GetSchemaInfo() const { return *curr_schema_; }

    KvTransaction& GetTxn() { return *txn_; }

    /**
     * Registers a new iterator.
     * Each transaction keep a list of all the iterators created inside it.
     * When the transaction is committed or aborted, it closes all the
     * iterators.
     * User can also call RefreshIterators() to refresh iterators after
     * write operations.
     *
     * @param [in,out]  it  If non-null, the iterator.
     */
    void RegisterIterator(IteratorBase* it) { iterators_.push_back(it); }

    void DeregisterIterator(IteratorBase* it) {
        for (int i = (int)iterators_.size() - 1; i >= 0; i--) {
            if (iterators_[i] == it) {
                iterators_.erase(iterators_.begin() + i);
                return;
            }
        }
    }

    void RefreshIterators() {
        for (auto& it : iterators_) {
            it->RefreshContentIfKvIteratorModified();
        }
    }

    std::unordered_map<LabelId, int64_t>& GetVertexDeltaCount() {
        return vertex_delta_count_;
    }

    std::unordered_map<LabelId, int64_t>& GetEdgeDeltaCount() {
        return edge_delta_count_;
    }

    std::set<LabelId>& GetVertexLabelDelete() {
        return vertex_label_delete_;
    }

    std::set<LabelId>& GetEdgeLabelDelete() {
        return edge_label_delete_;
    }

    size_t GetLooseNumVertex() { return graph_->GetLooseNumVertex(*txn_); }

    void GetStartAndEndVid(VertexId& start, VertexId& end);

    static TemporalId ParseTemporalId(const FieldData& fd,
                                      const TemporalFieldOrder& temporal_order);

    static TemporalId ParseTemporalId(const std::string& fd,
                                      const TemporalFieldOrder& temporal_order);

 private:
    void CloseAllIterators() {
        /** NOTE: it->Close() will remove the it from iterators_. So we
         *  need to make a copy of iterators_ before calling Close(). */
        auto iterators_copy = iterators_;
        // traversing backward to make it easier for iterators to
        // deregister
        for (int i = (int)iterators_copy.size() - 1; i >= 0; i--) iterators_copy[i]->Close();
        FMA_DBG_ASSERT(iterators_.empty());
    }

    VertexIndex* GetVertexIndex(const std::string& label, const std::string& field);
    VertexIndex* GetVertexIndex(size_t label, size_t field);
    EdgeIndex* GetEdgeIndex(const std::string& label, const std::string& field);
    EdgeIndex* GetEdgeIndex(size_t label, size_t field);

    CompositeIndex* GetVertexCompositeIndex(const std::string& label,
                                            const std::vector<std::string>& fields);

    CompositeIndex* GetVertexCompositeIndex(const size_t& label,
                                            const std::vector<size_t>& field_ids);

    void EnterTxn();
    void LeaveTxn();

    void CommitFullTextIndex();

    size_t GetTxnId() { return txn_->TxnId(); }
};

}  // namespace lgraph
