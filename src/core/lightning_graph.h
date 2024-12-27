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
#include <iostream>
#include <memory>
#include <mutex>

#include "tools/lgraph_log.h"
#include "fma-common/rw_lock.h"

#include "core/blob_manager.h"
#include "core/data_type.h"
#include "core/defs.h"
#include "core/index_manager.h"
#include "core/graph.h"
#include "core/killable_rw_lock.h"
#include "core/transaction.h"
#include "core/value.h"
#include "core/full_text_index.h"
#include "plugin/plugin_manager.h"

namespace lgraph {
namespace import_v2 {
class Importer;
class ImportOnline;
}  // namespace import_v2

class Galaxy;

class LightningGraph {
    friend class IndexManager;
    friend class Transaction;
    friend class import_v2::Importer;
    friend class import_v2::ImportOnline;

    DBConfig config_;

    std::unique_ptr<KvStore> store_ = nullptr;
    std::shared_ptr<KvTable> meta_table_;
    std::unique_ptr<KvTable> blob_table_;
    std::unique_ptr<graph::Graph> graph_ = nullptr;
    std::unique_ptr<IndexManager> index_manager_ = nullptr;
    std::unique_ptr<PluginManager> plugin_manager_ = nullptr;
    std::unique_ptr<BlobManager> blob_manager_ = nullptr;
    std::unique_ptr<FullTextIndex> fulltext_index_ = nullptr;
    GCRefCountedPtr<SchemaInfo> schema_;
    KillableRWLock meta_lock_;  // lock to hold when doing meta update, especially when AlterLabel

    static thread_local bool in_transaction_;
    static inline bool& InTransaction() { return in_transaction_; }

    DISABLE_COPY(LightningGraph);
    DISABLE_MOVE(LightningGraph);

    std::string db_secret;

 public:
    explicit LightningGraph(const DBConfig& conf);

    ~LightningGraph();

    /**
     * Creates a read transaction
     *
     * \return  The new read transaction.
     */
    Transaction CreateReadTxn();

    /**
     * Forks a read transaction
     */
    Transaction ForkTxn(Transaction& txn);

    /**
     * @brief  Creates a read-write transaction
     * @param   optimistic  Create an optimistic txn. Multiple optimistic txns can run in parallel.
     *
     * @returns  The new write transaction.
     */
    Transaction CreateWriteTxn(bool optimistic = false);

    /** Flush any buffered data. Use this when opening the DB with durable = false. */
    void Persist() {
        if (store_) store_->Flush();
        if (fulltext_index_) fulltext_index_->Commit();
    }

    std::vector<VectorIndexSpec> ListVectorIndex(KvTransaction& txn);

    /** Drop all the data in the graph. */
    void DropAllData();

    /** Drop all vertex and edges but keep the labels and indexes */
    void DropAllVertex();

    /**
     * Gets database stats, including database size and the next VertexId.
     *
     * \param [in,out]  msize       The msize.
     * \param [in,out]  next_vid    The next vid.
     */
    void GetDBStat(size_t& msize, size_t& next_vid);

    size_t GetNumVertices();

    //********************************
    // Schema modification
    //********************************

    /**
     * Adds a label
     *
     * \param   label       The label.
     * \param   n_fields    Number of fields for this label.
     * \param   fds         The FieldDefs.
     * \param   is_vertex   True if this is vertex label, otherwise it is edge label.
     * \param   options     Cast to VertexOptions when is_vertex is true, else cast to EdgeOptions.

     * \return  True if it succeeds, false if the label already exists. Throws exception on error.
     */
    bool AddLabel(const std::string& label, size_t n_fields, const FieldSpec* fds, bool is_vertex,
                  const LabelOptions& options);

    /**
     * Adds a label
     *
     * \param   label       The label name.
     * \param   fds         The FieldDefs.
     * \param   is_vertex   True if this is vertex label, otherwise it is edge label.
     * \param   options     Cast to VertexOptions when is_vertex is true, else cast to EdgeOptions.

     * \return  True if it succeeds, false if the label already exists. Throws exception on error.
     */
    bool AddLabel(const std::string& label, const std::vector<FieldSpec>& fds, bool is_vertex,
                  const LabelOptions& options);

    // delete a label
    bool DelLabel(const std::string& label, bool is_vertex, size_t* n_modified);

    // alter label
    template <typename GenNewSchema, typename MakeNewProp, typename ModifyIndex>
    bool _AlterLabel(
        bool is_vertex, const std::string& label,
        const GenNewSchema& gen_new_schema,                // std::function<Schema(Schema*)>
        const MakeNewProp& make_new_prop_and_destroy_old,  // std::function<Value(const Value&,
                                                           // Schema*, Schema*, Transaction&)>
        const ModifyIndex& modify_index,
        size_t* n_modified, size_t commit_size);

    bool AlterLabelModEdgeConstraints(const std::string& label,
                                      const EdgeConstraints& edge_constraints);
    bool AlterLabelDelFields(const std::string& label, const std::vector<std::string>& del_fields,
                             bool is_vertex, size_t* n_modified);
    bool AlterLabelAddFields(const std::string& label, const std::vector<FieldSpec>& add_fields,
                             const std::vector<FieldData>& default_values, bool is_vertex,
                             size_t* n_modified);
    bool AlterLabelModFields(const std::string& label, const std::vector<FieldSpec>& mod_fields,
                             bool is_vertex, size_t* n_modified);
    bool AddEdgeConstraints(const std::string& edge_label, const EdgeConstraints& constraints);
    bool ClearEdgeConstraints(const std::string& edge_label);

    /**
     * Adds an index to 'label:field', but do not actually build it. Used internally
     *
     * \param   label       The label.
     * \param   field       The field.
     * \param   type        The index type.
     * \param   is_vertex   True if this is vertex label, otherwise it is edge label.
     *
     * \return  True if it succeeds, false if the index already exists. Throws exception on error.
     */
    bool _AddEmptyIndex(const std::string& label, const std::string& field,
                        IndexType type, bool is_vertex);

    // adds an index, blocks until the index is ready
    // returns true if success, false if index already exists.
    bool BlockingAddIndex(const std::string& label, const std::string& field,
                          IndexType type, bool is_vertex, bool known_vid_range = false,
                          VertexId start_vid = 0, VertexId end_vid = 0);

    // adds a vector index, blocks until the index is ready
    // returns true if success, false if index already exists.
    bool BlockingAddVectorIndex(bool is_vertex, const std::string& label, const std::string& field,
                                const std::string& index_type, int vec_dimension,
                                const std::string& distance_type, std::vector<int>& index_spec);

    // adds an index, blocks until the index is ready
    // returns true if success, false if index already exists.
    bool BlockingAddCompositeIndex(const std::string& label, const std::vector<std::string>& fields,
                          CompositeIndexType type, bool is_vertex, bool known_vid_range = false,
                          VertexId start_vid = 0, VertexId end_vid = 0);

    bool AddFullTextIndex(bool is_vertex, const std::string& label, const std::string& field);

    bool DeleteFullTextIndex(bool is_vertex, const std::string& label, const std::string& field);

    void RebuildFullTextIndex(const std::set<LabelId>& v_lids, const std::set<LabelId>& e_lids);

    void RebuildFullTextIndex(const std::set<std::string>& v_labels,
                              const std::set<std::string>& e_labels);

    void RebuildAllFullTextIndex();

    void FullTextIndexRefresh();

    void RefreshCount();

    std::vector<std::tuple<bool, std::string, std::string>> ListFullTextIndexes();

    std::vector<std::pair<int64_t, float>> QueryVertexByFullTextIndex(const std::string& label,
                                                    const std::string& query, int top_n);

    std::vector<std::pair<EdgeUid, float>> QueryEdgeByFullTextIndex(const std::string& label,
                                                  const std::string& query, int top_n);

    /**
     * Builds a set of indexes at once.
     *  \notice: It is assumed that vertexes of the same labels are stored with cotinuous VID,
     *           so that we can build indexes for the labels one by one and avoid scanning
     *           the DB multiple times.
     */
    void OfflineCreateBatchIndex(const std::vector<IndexSpec>& indexes,
                                 size_t commit_batch_size = 1 << 20, bool is_vertex = true);

    /**
     * Is this index ready? When an index is added to a non-empty graph, it will require some time
     * to be built. This function returns the status of the index so user can choose to wait for
     * the index to be ready.
     *
     * \param   label   The label.
     * \param   field   The field.
     * \param   is_vertxt True for vertex index, False for edge index.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool IsIndexed(const std::string& label, const std::string& field, bool is_vertex);

    bool IsCompositeIndexed(const std::string& label, const std::vector<std::string>& fields);

    /**
     * Deletes the index to 'label:field'
     *
     * \param   label   The label.
     * \param   field   The field.
     * \param   is_vertxt True for vertex index, False for edge index.
     *
     * \return  True if it succeeds, false if the index does not exist. Throws exception on error.
     */
    bool DeleteIndex(const std::string& label, const std::string& field, bool is_vertex);

    bool DeleteVectorIndex(bool is_vertex, const std::string& label, const std::string& field);

    bool DeleteCompositeIndex(const std::string& label,
                              const std::vector<std::string>& fields, bool is_vertex);

    /** Drop all index */
    void DropAllIndex();

    KvStore& GetStore();

    const DBConfig& GetConfig() const;

    /**
     * Backups the current DB to the path specified.
     *
     * \param path  Full pathname of the destination.
     * \param compact  True to enable compaction
     *
     * \return  Transaction ID of the last committed transaction.
     */
    size_t Backup(const std::string& path, bool compact = true);

    /**
     * Take a snapshot of the whole db using the read transaction txn.
     *
     * \param [in,out] txn  The transaction.
     * \param          path Full pathname of the snapshot file.
     */
    void Snapshot(Transaction& txn, const std::string& path);

    void LoadSnapshot(const std::string& path);

    /** Warmups this DB */
    void WarmUp() const;

    PluginManager* GetPluginManager() const;

    // reload content from disk, including plugins
    void ReloadFromDisk(const DBConfig& config);

    // current version, which is actually lmdb txn id
    size_t GetCurrentVersion();

    // Check DB secret ID, used in galaxy to prevent copying LightningGraph dirs
    // Returns true if check success, otherwise false
    bool CheckDbSecret(const std::string& expected);

    // Flush db secret in online full importing
    void FlushDbSecret(const std::string& secret);

    // Get db secret
    std::string GetSecret();

    KillableRWLock& GetReloadLock() { return meta_lock_; }

    // close everything, used by Galaxy when deleting graph
    void Close();

 private:
#if USELESS_CODE
    ScopedRef<SchemaInfo> GetSchemaInfo();
#endif

    template <typename T>
    void _DumpIndex(const IndexSpec& spec, VertexId first_vertex, size_t batch_commit_size,
                    VertexId& next_vertex_id, bool is_vertex = true);

    template <typename T>
    void BatchBuildIndex(Transaction& txn, SchemaInfo* new_schema_info, LabelId label_id,
                         size_t field_id, IndexType type, VertexId start_vid, VertexId end_vid,
                         bool is_vertex = true);

    void BatchBuildCompositeIndex(Transaction& txn, SchemaInfo* new_schema_info, LabelId label_id,
                         const std::vector<std::string> &fields, CompositeIndexType type,
                         VertexId start_vid, VertexId end_vid, bool is_vertex = true);

    void Open();
    static bool FieldTypeComplatible(FieldType a, FieldType b);
};
}  // namespace lgraph
