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

#include "db/db.h"
#include "lgraph/lgraph_txn.h"

bool lgraph::AccessControlledDB::enable_plugin = true;

lgraph::AccessControlledDB::AccessControlledDB(ScopedRef<LightningGraph>&& ref,
                                               AccessLevel access_level,
                                               const std::string& user)
    : graph_ref_(std::move(ref)),
      graph_(graph_ref_.Get()),
      graph_ref_lock_(graph_->GetReloadLock(), GetMyThreadId()),
      access_level_(access_level),
      user_(user) {}

lgraph::AccessControlledDB::AccessControlledDB(lgraph::LightningGraph* db)
    : graph_ref_(nullptr, GetMyThreadId()),
      graph_(db),
      graph_ref_lock_(db->GetReloadLock(), GetMyThreadId()),
      access_level_(AccessLevel::FULL),
      user_(_detail::DEFAULT_ADMIN_NAME) {}

lgraph::AccessControlledDB::AccessControlledDB(AccessControlledDB&& rhs)
    : graph_ref_(std::move(rhs.graph_ref_)),
      graph_(rhs.graph_),
      graph_ref_lock_(graph_->GetReloadLock(), GetMyThreadId()),
      access_level_(rhs.access_level_),
      user_(std::move(rhs.user_)) {}

const lgraph::DBConfig& lgraph::AccessControlledDB::GetConfig() const {
    CheckReadAccess();
    return graph_->GetConfig();
}

lgraph::Transaction lgraph::AccessControlledDB::CreateReadTxn() {
    CheckReadAccess();
    return graph_->CreateReadTxn();
}

lgraph::Transaction lgraph::AccessControlledDB::CreateWriteTxn(bool optimistic) {
    CheckWriteAccess();
    return graph_->CreateWriteTxn(optimistic);
}

lgraph::Transaction lgraph::AccessControlledDB::ForkTxn(Transaction& txn) {
    return graph_->ForkTxn(txn);
}

bool lgraph::AccessControlledDB::LoadPlugin(plugin::Type plugin_type, const std::string& user,
                                            const std::string& name,
                                            const std::vector<std::string>& code,
                                            const std::vector<std::string>& filename,
                                            plugin::CodeType code_type, const std::string& desc,
                                            bool is_read_only, const std::string& version) {
    CheckAdmin();
    CheckLoadOrDeletePlugin();
    return graph_->GetPluginManager()->LoadPluginFromCode(plugin_type, user, name, code, filename,
                                                          code_type, desc, is_read_only, version);
}

bool lgraph::AccessControlledDB::DelPlugin(plugin::Type plugin_type, const std::string& user,
                                           const std::string& name) {
    CheckAdmin();
    CheckLoadOrDeletePlugin();
    return graph_->GetPluginManager()->DelPlugin(plugin_type, user, name);
}

bool lgraph::AccessControlledDB::CallPlugin(lgraph_api::Transaction* txn,
                                            plugin::Type plugin_type, const std::string& user,
                                            const std::string& name, const std::string& request,
                                            double timeout_seconds, bool in_process,
                                            std::string& output) {
    auto pm = graph_->GetPluginManager();
    bool is_readonly = pm->IsReadOnlyPlugin(plugin_type, user, name);
    if (access_level_ < AccessLevel::WRITE && !is_readonly)
        THROW_CODE(Unauthorized, "Write permission needed to call this plugin.");
    return pm->Call(txn,
                    plugin_type,
                    user,
                    this,
                    name,
                    request,
                    timeout_seconds,
                    in_process,
                    output);
}

bool lgraph::AccessControlledDB::CallV2Plugin(lgraph_api::Transaction* txn,
                                              plugin::Type plugin_type, const std::string& user,
                                              const std::string& name, const std::string& request,
                                              double timeout_seconds, bool in_process,
                                              Result& output) {
    auto pm = graph_->GetPluginManager();
    bool is_readonly = pm->IsReadOnlyPlugin(plugin_type, user, name);
    if (access_level_ < AccessLevel::WRITE && !is_readonly)
        THROW_CODE(Unauthorized, "Write permission needed to call this plugin.");
    return pm->CallV2(txn,
                      plugin_type,
                      user,
                      this,
                      name,
                      request,
                      timeout_seconds,
                      in_process,
                      output);
}

std::vector<lgraph::PluginDesc> lgraph::AccessControlledDB::ListPlugins(plugin::Type plugin_type,
                                                                        const std::string& user) {
    return graph_->GetPluginManager()->ListPlugins(plugin_type, user);
}

bool lgraph::AccessControlledDB::GetPluginCode(plugin::Type plugin_type, const std::string& user,
                                               const std::string& name, lgraph::PluginCode& ret) {
    return graph_->GetPluginManager()->GetPluginCode(plugin_type, user, name, ret);
}

bool lgraph::AccessControlledDB::IsReadOnlyPlugin(plugin::Type type, const std::string& user,
                                                  const std::string& name) {
    return graph_->GetPluginManager()->IsReadOnlyPlugin(type, user, name);
}

void lgraph::AccessControlledDB::DropAllData() {
    CheckFullAccess();
    graph_->DropAllData();
}

void lgraph::AccessControlledDB::DropAllVertex() {
    CheckFullAccess();
    graph_->DropAllVertex();
}

void lgraph::AccessControlledDB::Flush() {
    CheckWriteAccess();
    graph_->Persist();
}

size_t lgraph::AccessControlledDB::EstimateNumVertices() {
    CheckReadAccess();
    return graph_->GetNumVertices();
}

bool lgraph::AccessControlledDB::AddLabel(bool is_vertex, const std::string& label,
                                          const std::vector<FieldSpec>& fds,
                                          const LabelOptions& options) {
    CheckFullAccess();
    return graph_->AddLabel(label, fds, is_vertex, options);
}

bool lgraph::AccessControlledDB::DeleteLabel(bool is_vertex, const std::string& label,
                                             size_t* n_modified) {
    CheckFullAccess();
    return graph_->DelLabel(label, is_vertex, n_modified);
}

bool lgraph::AccessControlledDB::AlterLabelModEdgeConstraints(
    const std::string& label, const std::vector<std::pair<std::string, std::string>>& constraints) {
    CheckFullAccess();
    return graph_->AlterLabelModEdgeConstraints(label, constraints);
}

bool lgraph::AccessControlledDB::AlterLabelDelFields(const std::string& label,
                                                     const std::vector<std::string>& del_fields,
                                                     bool is_vertex, size_t* n_modified) {
    CheckFullAccess();
    return graph_->AlterLabelDelFields(label, del_fields, is_vertex, n_modified);
}

bool lgraph::AccessControlledDB::AlterLabelAddFields(const std::string& label,
                                                     const std::vector<FieldSpec>& add_fields,
                                                     const std::vector<FieldData>& default_values,
                                                     bool is_vertex, size_t* n_modified) {
    CheckFullAccess();
    return graph_->AlterLabelAddFields(label, add_fields, default_values, is_vertex, n_modified);
}

bool lgraph::AccessControlledDB::AlterLabelModFields(const std::string& label,
                                                     const std::vector<FieldSpec>& mod_fields,
                                                     bool is_vertex, size_t* n_modified) {
    CheckFullAccess();
    return graph_->AlterLabelModFields(label, mod_fields, is_vertex, n_modified);
}

bool lgraph::AccessControlledDB::AddEdgeConstraints(
    const std::string& label,
    const std::vector<std::pair<std::string, std::string>>& constraints) {
    CheckFullAccess();
    return graph_->AddEdgeConstraints(label, constraints);
}

bool lgraph::AccessControlledDB::ClearEdgeConstraints(const std::string& label) {
    CheckFullAccess();
    return graph_->ClearEdgeConstraints(label);
}

void lgraph::AccessControlledDB::RefreshCount() {
    CheckFullAccess();
    graph_->RefreshCount();
}


bool lgraph::AccessControlledDB::AddVertexIndex(const std::string& label, const std::string& field,
                                                IndexType type) {
    CheckFullAccess();
    return graph_->BlockingAddIndex(label, field, type, true);
}

bool lgraph::AccessControlledDB::AddVertexCompositeIndex(const std::string& label,
                                                         const std::vector<std::string>& fields,
                                                         CompositeIndexType type) {
    CheckFullAccess();
    return graph_->BlockingAddCompositeIndex(label, fields, type, true);
}

bool lgraph::AccessControlledDB::AddEdgeIndex(const std::string& label, const std::string& field,
                                          IndexType type) {
    CheckFullAccess();
    return graph_->BlockingAddIndex(label, field, type, false);
}

bool lgraph::AccessControlledDB::AddVectorIndex(bool is_vertex, const std::string& label,
                                                const std::string& field,
                                                const std::string& index_type,
                                                int vec_dimension, const std::string& distance_type,
                                                std::vector<int>& index_spec) {
    CheckFullAccess();
    return graph_->BlockingAddVectorIndex(is_vertex, label, field, index_type, vec_dimension,
                                            distance_type, index_spec);
}

bool lgraph::AccessControlledDB::AddFullTextIndex(bool is_vertex, const std::string& label,
                                                  const std::string& field) {
    CheckFullAccess();
    return graph_->AddFullTextIndex(is_vertex, label, field);
}

bool lgraph::AccessControlledDB::DeleteFullTextIndex(bool is_vertex, const std::string& label,
                                                     const std::string& field) {
    CheckFullAccess();
    return graph_->DeleteFullTextIndex(is_vertex, label, field);
}

void lgraph::AccessControlledDB::RebuildFullTextIndex(const std::set<std::string>& vertex_labels,
                                                      const std::set<std::string>& edge_labels) {
    CheckFullAccess();
    return graph_->RebuildFullTextIndex(vertex_labels, edge_labels);
}

std::vector<std::tuple<bool, std::string, std::string>>
lgraph::AccessControlledDB::ListFullTextIndexes() {
    CheckFullAccess();
    return graph_->ListFullTextIndexes();
}

std::vector<std::pair<int64_t, float>> lgraph::AccessControlledDB::QueryVertexByFullTextIndex(
    const std::string& label, const std::string& query, int top_n) {
    CheckFullAccess();
    return graph_->QueryVertexByFullTextIndex(label, query, top_n);
}

std::vector<std::pair<lgraph_api::EdgeUid, float>>
lgraph::AccessControlledDB::QueryEdgeByFullTextIndex(const std::string& label,
                                                     const std::string& query, int top_n) {
    CheckFullAccess();
    return graph_->QueryEdgeByFullTextIndex(label, query, top_n);
}

bool lgraph::AccessControlledDB::DeleteVertexIndex(const std::string& label,
                                                   const std::string& field) {
    CheckFullAccess();
    return graph_->DeleteIndex(label, field, true);
}

bool lgraph::AccessControlledDB::DeleteEdgeIndex(const std::string& label,
                                                 const std::string& field) {
    CheckFullAccess();
    return graph_->DeleteIndex(label, field, false);
}

bool lgraph::AccessControlledDB::DeleteVertexCompositeIndex(const std::string& label,
                                 const std::vector<std::string>& fields) {
    CheckFullAccess();
    return graph_->DeleteCompositeIndex(label, fields, true);
}

bool lgraph::AccessControlledDB::DeleteVectorIndex(bool is_vertex, const std::string& label,
                                                   const std::string& field) {
    CheckFullAccess();
    return graph_->DeleteVectorIndex(is_vertex, label, field);
}

bool lgraph::AccessControlledDB::IsVertexIndexed(const std::string& label,
                                                 const std::string& field) {
    CheckReadAccess();
    return graph_->IsIndexed(label, field, true);
}

bool lgraph::AccessControlledDB::IsEdgeIndexed(const std::string& label,
                                               const std::string& field) {
    CheckReadAccess();
    return graph_->IsIndexed(label, field, false);
}

bool lgraph::AccessControlledDB::IsVertexCompositeIndexed(const std::string& label,
                                 const std::vector<std::string>& fields) {
    CheckReadAccess();
    return graph_->IsCompositeIndexed(label, fields);
}

void lgraph::AccessControlledDB::WarmUp() const { graph_->WarmUp(); }

size_t lgraph::AccessControlledDB::Backup(const std::string& path, bool compact) const {
    CheckReadAccess();
    return graph_->Backup(path, compact);
}
