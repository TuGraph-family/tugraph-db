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
#include <memory>
#include "core/lightning_graph.h"
#include "core/transaction.h"
#include "db/acl.h"
#include "plugin/plugin_desc.h"

namespace lgraph {
// interface for a graphDB, which has its own LightningGraph and plugin managers
class AccessControlledDB {
    ScopedRef<LightningGraph> graph_ref_;
    LightningGraph* graph_;
    AutoReadLock graph_ref_lock_;
    AccessLevel access_level_;
    std::string user_;
    static bool enable_plugin;

    DISABLE_COPY(AccessControlledDB);

 public:
    AccessControlledDB(ScopedRef<LightningGraph>&& g, AccessLevel access_level,
                       const std::string& user);

    // unmanaged db instance with full access, just for testing
    explicit AccessControlledDB(LightningGraph* graph);

    AccessControlledDB(AccessControlledDB&& rhs);

    AccessControlledDB& operator=(AccessControlledDB&&) = delete;

    const DBConfig& GetConfig() const;

    Transaction CreateReadTxn();

    Transaction CreateWriteTxn(bool optimistic = false);

    Transaction ForkTxn(Transaction& txn);

    bool LoadPlugin(plugin::Type plugin_type, const std::string& token, const std::string& name,
                    const std::vector<std::string>& code, const std::vector<std::string>& filename,
                    plugin::CodeType code_type, const std::string& desc, bool is_read_only,
                    const std::string& version);

    bool DelPlugin(plugin::Type plugin_type, const std::string& token, const std::string& name);

    bool CallPlugin(lgraph_api::Transaction* txn,
                    plugin::Type plugin_type, const std::string& token, const std::string& name,
                    const std::string& request, double timeout_seconds, bool in_process,
                    std::string& output);

    bool CallV2Plugin(lgraph_api::Transaction* txn, plugin::Type plugin_type,
                      const std::string& user, const std::string& name, const std::string& request,
                      double timeout_seconds, bool in_process, Result& output);

    std::vector<PluginDesc> ListPlugins(plugin::Type plugin_type, const std::string& token);

    bool GetPluginCode(plugin::Type plugin_type, const std::string& token, const std::string& name,
                       PluginCode& ret);

    bool IsReadOnlyPlugin(plugin::Type type, const std::string& token, const std::string& name);

    void DropAllData();

    void DropAllVertex();

    void Flush();

    size_t EstimateNumVertices();

    bool AddLabel(bool is_vertex, const std::string& label, const std::vector<FieldSpec>& fds,
                  const LabelOptions& options);

    bool DeleteLabel(bool is_vertex, const std::string& label, size_t* n_modified);

    bool AlterLabelModEdgeConstraints(
        const std::string& label,
        const std::vector<std::pair<std::string, std::string>>& constraints);

    bool AlterLabelDelFields(const std::string& label, const std::vector<std::string>& del_fields,
                             bool is_vertex, size_t* n_modified);

    bool AlterLabelAddFields(const std::string& label, const std::vector<FieldSpec>& add_fields,
                             const std::vector<FieldData>& default_values, bool is_vertex,
                             size_t* n_modified);

    bool AlterLabelModFields(const std::string& label, const std::vector<FieldSpec>& mod_fields,
                             bool is_vertex, size_t* n_modified);
    bool AddEdgeConstraints(
        const std::string& label,
        const std::vector<std::pair<std::string, std::string>>& constraints);
    bool ClearEdgeConstraints(const std::string& label);

    bool AddVertexIndex(const std::string& label, const std::string& field, IndexType type);

    bool AddVertexCompositeIndex(const std::string& label,
                                 const std::vector<std::string>& fields, CompositeIndexType type);

    bool AddEdgeIndex(const std::string& label, const std::string& field, IndexType type);

    bool AddVectorIndex(bool is_vertex, const std::string& label, const std::string& field,
                        const std::string& index_type, int vec_dimension,
                        const std::string& distance_type, std::vector<int>& index_spec);

    bool DeleteVertexIndex(const std::string& label, const std::string& field);

    bool DeleteEdgeIndex(const std::string& label, const std::string& field);

    bool DeleteVertexCompositeIndex(const std::string& label,
                                    const std::vector<std::string>& fields);

    bool DeleteVectorIndex(bool is_vertex, const std::string& label, const std::string& field);

    bool IsVertexIndexed(const std::string& label, const std::string& field);

    bool IsEdgeIndexed(const std::string& label, const std::string& field);

    bool IsVertexCompositeIndexed(const std::string& label,
                                  const std::vector<std::string>& fields);

    bool AddFullTextIndex(bool is_vertex, const std::string& label, const std::string& field);

    bool DeleteFullTextIndex(bool is_vertex, const std::string& label, const std::string& field);

    void RebuildFullTextIndex(const std::set<std::string>& vertex_labels,
                              const std::set<std::string>& edge_labels);
    std::vector<std::tuple<bool, std::string, std::string>> ListFullTextIndexes();

    std::vector<std::pair<int64_t, float>> QueryVertexByFullTextIndex(const std::string& label,
                                                    const std::string& query, int top_n);

    std::vector<std::pair<EdgeUid, float>> QueryEdgeByFullTextIndex(const std::string& label,
                                                  const std::string& query, int top_n);
    void RefreshCount();

    void WarmUp() const;

    size_t Backup(const std::string& path, bool compact) const;

    inline AccessLevel GetAccessLevel() const { return access_level_; }

    inline LightningGraph* GetLightningGraph() const { return graph_; }

    inline static void SetEnablePlugin(bool enable_plugin_) {
        AccessControlledDB::enable_plugin = enable_plugin_;
    }

    inline static bool GetEnablePlugin() {
        return enable_plugin;
    }

 private:
    inline void CheckReadAccess() const {
        if (access_level_ < AccessLevel::READ) THROW_CODE(Unauthorized, "No read permission.");
    }

    inline void CheckWriteAccess() const {
        if (access_level_ < AccessLevel::WRITE) THROW_CODE(Unauthorized, "No write permission.");
    }

    inline void CheckFullAccess() const {
        if (access_level_ < AccessLevel::FULL) THROW_CODE(Unauthorized, "No full permission.");
    }

    inline void CheckAdmin() const {
        if (user_ != _detail::DEFAULT_ADMIN_NAME) THROW_CODE(Unauthorized, "Not the admin user.");
    }

    inline void CheckLoadOrDeletePlugin() const {
        if (!enable_plugin) THROW_CODE(PluginDisabled, "No permission to load or delete plugin, "
                                       "please use correct config and restart server!\n"
                                       "This function has security risks, please enable "
                                       "it with caution!");
    }
};
}  // namespace lgraph
