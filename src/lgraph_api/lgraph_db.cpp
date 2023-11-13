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

#include "core/lightning_graph.h"
#include "core/task_tracker.h"
#include "db/db.h"
#include "lgraph/lgraph_db.h"
#include "lgraph/lgraph_txn.h"

namespace lgraph_api {
bool ShouldKillThisTask() { return lgraph::TaskTracker::GetInstance().ShouldKillCurrentTask(); }

ThreadContextPtr GetThreadContext() {
    return lgraph::TaskTracker::GetInstance().GetThreadContext();
}

bool ShouldKillThisTask(ThreadContextPtr ctx) {
    return lgraph::TaskTracker::ShouldKillTask((lgraph::TaskTracker::ThreadContext*)ctx);
}

#define THROW_IF_RO()                                        \
    if (read_only_)                                          \
        throw WriteNotAllowedError(                          \
            "Write transaction is not allowed in read-only " \
            "DB.");
#define THROW_IF_INVALID() \
    if (!db_) throw InvalidGraphDBError();

GraphDB::GraphDB(lgraph::AccessControlledDB* db_with_access_control, bool read_only, bool owns_db)
    : db_(db_with_access_control), should_delete_db_(owns_db), read_only_(read_only) {}

GraphDB::GraphDB(GraphDB&& rhs)
    : db_(std::move(rhs.db_)),
      should_delete_db_(rhs.should_delete_db_),
      read_only_(rhs.read_only_) {
    rhs.should_delete_db_ = false;
    rhs.db_ = nullptr;
}

GraphDB& GraphDB::operator=(GraphDB&& rhs) {
    if (should_delete_db_) delete db_;
    db_ = std::move(rhs.db_);
    should_delete_db_ = rhs.should_delete_db_;
    read_only_ = rhs.read_only_;
    rhs.should_delete_db_ = false;
    rhs.db_ = nullptr;
    return *this;
}

GraphDB::~GraphDB() { Close(); }

void GraphDB::Close() {
    if (should_delete_db_ && db_) {
        delete db_;
        db_ = nullptr;
    }
}

Transaction GraphDB::CreateReadTxn() {
    THROW_IF_INVALID();
    return Transaction(db_->CreateReadTxn());
}

Transaction GraphDB::CreateWriteTxn(bool optimistic) {
    THROW_IF_INVALID();
    THROW_IF_RO();
    return Transaction(db_->CreateWriteTxn(optimistic));
}

Transaction GraphDB::ForkTxn(Transaction& txn) {
    THROW_IF_INVALID();
    return Transaction(db_->ForkTxn(*(txn.txn_)));
}

void GraphDB::Flush() {
    THROW_IF_INVALID();
    db_->Flush();
}

void GraphDB::DropAllData() {
    THROW_IF_INVALID();
    THROW_IF_RO();
    db_->DropAllData();
}

void GraphDB::DropAllVertex() {
    THROW_IF_INVALID();
    THROW_IF_RO();
    db_->DropAllVertex();
}

size_t GraphDB::EstimateNumVertices() {
    THROW_IF_INVALID();
    return db_->EstimateNumVertices();
}

bool GraphDB::AddVertexLabel(const std::string& label, const std::vector<FieldSpec>& fds,
                             const VertexOptions& options) {
    THROW_IF_INVALID();
    THROW_IF_RO();
    return db_->AddLabel(true, label, fds, options);
}

bool GraphDB::DeleteVertexLabel(const std::string& label, size_t* n_modified) {
    THROW_IF_INVALID();
    THROW_IF_RO();
    return db_->DeleteLabel(true, label, n_modified);
}

bool GraphDB::AlterLabelModEdgeConstraints(
    const std::string& label, const std::vector<std::pair<std::string, std::string>>& constraints) {
    THROW_IF_INVALID();
    THROW_IF_RO();
    return db_->AlterLabelModEdgeConstraints(label, constraints);
}

bool GraphDB::AlterVertexLabelDelFields(const std::string& label,
                                        const std::vector<std::string>& del_fields,
                                        size_t* n_modified) {
    THROW_IF_INVALID();
    THROW_IF_RO();
    return db_->AlterLabelDelFields(label, del_fields, true, n_modified);
}

bool GraphDB::AlterVertexLabelAddFields(const std::string& label,
                                        const std::vector<FieldSpec>& add_fields,
                                        const std::vector<FieldData>& default_values,
                                        size_t* n_modified) {
    THROW_IF_INVALID();
    THROW_IF_RO();
    return db_->AlterLabelAddFields(label, add_fields, default_values, true, n_modified);
}

bool GraphDB::AlterVertexLabelModFields(const std::string& label,
                                        const std::vector<FieldSpec>& mod_fields,
                                        size_t* n_modified) {
    THROW_IF_INVALID();
    THROW_IF_RO();
    return db_->AlterLabelModFields(label, mod_fields, true, n_modified);
}

bool GraphDB::AddEdgeLabel(const std::string& label, const std::vector<FieldSpec>& fds,
                           const EdgeOptions& options) {
    THROW_IF_INVALID();
    THROW_IF_RO();
    return db_->AddLabel(false, label, fds, options);
}

bool GraphDB::DeleteEdgeLabel(const std::string& label, size_t* n_modified) {
    THROW_IF_INVALID();
    THROW_IF_RO();
    return db_->DeleteLabel(false, label, n_modified);
}

bool GraphDB::AlterEdgeLabelDelFields(const std::string& label,
                                      const std::vector<std::string>& del_fields,
                                      size_t* n_modified) {
    THROW_IF_INVALID();
    THROW_IF_RO();
    return db_->AlterLabelDelFields(label, del_fields, false, n_modified);
}

bool GraphDB::AlterEdgeLabelAddFields(const std::string& label,
                                      const std::vector<FieldSpec>& add_fields,
                                      const std::vector<FieldData>& default_values,
                                      size_t* n_modified) {
    THROW_IF_INVALID();
    THROW_IF_RO();
    return db_->AlterLabelAddFields(label, add_fields, default_values, false, n_modified);
}

bool GraphDB::AlterEdgeLabelModFields(const std::string& label,
                                      const std::vector<FieldSpec>& mod_fields,
                                      size_t* n_modified) {
    THROW_IF_INVALID();
    THROW_IF_RO();
    return db_->AlterLabelModFields(label, mod_fields, false, n_modified);
}

bool GraphDB::AddVertexIndex(const std::string& label, const std::string& field, IndexType type) {
    THROW_IF_INVALID();
    THROW_IF_RO();
    return db_->AddVertexIndex(label, field, type);
}

bool GraphDB::AddEdgeIndex(const std::string& label, const std::string& field, IndexType type) {
    THROW_IF_INVALID();
    THROW_IF_RO();
    return db_->AddEdgeIndex(label, field, type);
}

bool GraphDB::IsVertexIndexed(const std::string& label, const std::string& field) {
    THROW_IF_INVALID();
    return db_->IsVertexIndexed(label, field);
}

bool GraphDB::IsEdgeIndexed(const std::string& label, const std::string& field) {
    THROW_IF_INVALID();
    return db_->IsEdgeIndexed(label, field);
}

bool GraphDB::DeleteVertexIndex(const std::string& label, const std::string& field) {
    THROW_IF_INVALID();
    THROW_IF_RO();
    return db_->DeleteVertexIndex(label, field);
}

bool GraphDB::DeleteEdgeIndex(const std::string& label, const std::string& field) {
    THROW_IF_INVALID();
    THROW_IF_RO();
    return db_->DeleteEdgeIndex(label, field);
}

std::string GraphDB::GetDescription() const {
    THROW_IF_INVALID();
    return db_->GetConfig().desc;
}

size_t GraphDB::GetMaxSize() const {
    THROW_IF_INVALID();
    return db_->GetConfig().db_size;
}

bool GraphDB::AddVertexFullTextIndex(const std::string& vertex_label, const std::string& field) {
    THROW_IF_INVALID();
    return db_->AddFullTextIndex(true, vertex_label, field);
}

bool GraphDB::AddEdgeFullTextIndex(const std::string& edge_label, const std::string& field) {
    THROW_IF_INVALID();
    return db_->AddFullTextIndex(false, edge_label, field);
}

bool GraphDB::DeleteVertexFullTextIndex(const std::string& vertex_label, const std::string& field) {
    THROW_IF_INVALID();
    return db_->DeleteFullTextIndex(true, vertex_label, field);
}

bool GraphDB::DeleteEdgeFullTextIndex(const std::string& edge_label, const std::string& field) {
    THROW_IF_INVALID();
    return db_->DeleteFullTextIndex(false, edge_label, field);
}

void GraphDB::RebuildFullTextIndex(const std::set<std::string>& vertex_labels,
                                   const std::set<std::string>& edge_labels) {
    THROW_IF_INVALID();
    db_->RebuildFullTextIndex(vertex_labels, edge_labels);
}

std::vector<std::tuple<bool, std::string, std::string>> GraphDB::ListFullTextIndexes() {
    THROW_IF_INVALID();
    return db_->ListFullTextIndexes();
}

std::vector<std::pair<int64_t, float>> GraphDB::QueryVertexByFullTextIndex(const std::string& label,
                                                                           const std::string& query,
                                                                           int top_n) {
    THROW_IF_INVALID();
    return db_->QueryVertexByFullTextIndex(label, query, top_n);
}

std::vector<std::pair<EdgeUid, float>> GraphDB::QueryEdgeByFullTextIndex(const std::string& label,
                                                                         const std::string& query,
                                                                         int top_n) {
    THROW_IF_INVALID();
    return db_->QueryEdgeByFullTextIndex(label, query, top_n);
}

void GraphDB::RefreshCount() {
    THROW_IF_INVALID();
    THROW_IF_RO();
    db_->RefreshCount();
}

}  // namespace lgraph_api
