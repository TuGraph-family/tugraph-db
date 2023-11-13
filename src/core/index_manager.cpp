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

#include "core/index_manager.h"

#include <utility>
#include "core/kv_store.h"
#include "core/lightning_graph.h"
#include "core/transaction.h"

namespace lgraph {
IndexManager::IndexManager(KvTransaction& txn, SchemaManager* v_schema_manager,
                           SchemaManager* e_schema_manager,
                           std::unique_ptr<KvTable> index_list_table,
                           LightningGraph* db)
    : db_(db), index_list_table_(std::move(index_list_table)) {
    size_t v_index_len = strlen(_detail::VERTEX_INDEX);
    size_t e_index_len = strlen(_detail::EDGE_INDEX);
    size_t v_ft_index_len = strlen(_detail::VERTEX_FULLTEXT_INDEX);
    size_t e_ft_index_len = strlen(_detail::EDGE_FULLTEXT_INDEX);
    auto it = index_list_table_->GetIterator(txn);
    for (it->GotoFirstKey(); it->IsValid(); it->Next()) {
        std::string index_name = it->GetKey().AsString();
        if (index_name.size() > v_index_len &&
            index_name.substr(index_name.size() - v_index_len) == _detail::VERTEX_INDEX) {
            _detail::IndexEntry idx = LoadIndex(it->GetValue());
            FMA_DBG_CHECK_EQ(idx.table_name, it->GetKey().AsString());
            Schema* schema = v_schema_manager->GetSchema(idx.label);
            FMA_DBG_ASSERT(schema);
            const _detail::FieldExtractor* fe = schema->GetFieldExtractor(idx.field);
            FMA_DBG_ASSERT(fe);
            auto tbl =
                VertexIndex::OpenTable(txn, db_->GetStore(), index_name, fe->Type(), idx.type);
            VertexIndex* index = new VertexIndex(std::move(tbl), fe->Type(), idx.type);
            index->SetReady();
            schema->MarkVertexIndexed(fe->GetFieldId(), index);
        } else if (index_name.size() > e_index_len &&
                   index_name.substr(index_name.size() - e_index_len) == _detail::EDGE_INDEX) {
            _detail::IndexEntry idx = LoadIndex(it->GetValue());
            FMA_DBG_CHECK_EQ(idx.table_name, it->GetKey().AsString());
            Schema* schema = e_schema_manager->GetSchema(idx.label);
            FMA_DBG_ASSERT(schema);
            const _detail::FieldExtractor* fe = schema->GetFieldExtractor(idx.field);
            FMA_DBG_ASSERT(fe);
            auto tbl =
                EdgeIndex::OpenTable(txn, db_->GetStore(), index_name, fe->Type(), idx.type);
            EdgeIndex* index = new EdgeIndex(std::move(tbl), fe->Type(), idx.type);
            index->SetReady();
            schema->MarkEdgeIndexed(fe->GetFieldId(), index);
        } else if (index_name.size() > v_ft_index_len &&
                   index_name.substr(index_name.size() - v_ft_index_len) ==
                       _detail::VERTEX_FULLTEXT_INDEX) {
            auto ft_idx = LoadIndex(it->GetValue());
            FMA_DBG_CHECK_EQ(ft_idx.table_name, it->GetKey().AsString());
            Schema* schema = v_schema_manager->GetSchema(ft_idx.label);
            FMA_DBG_ASSERT(schema);
            const _detail::FieldExtractor* fe = schema->GetFieldExtractor(ft_idx.field);
            FMA_DBG_ASSERT(fe);
            schema->MarkFullTextIndexed(fe->GetFieldId(), true);
        } else if (index_name.size() > e_ft_index_len &&
                   index_name.substr(index_name.size() - e_ft_index_len) ==
                       _detail::EDGE_FULLTEXT_INDEX) {
            auto ft_idx = LoadIndex(it->GetValue());
            FMA_DBG_CHECK_EQ(ft_idx.table_name, it->GetKey().AsString());
            Schema* schema = e_schema_manager->GetSchema(ft_idx.label);
            FMA_DBG_ASSERT(schema);
            const _detail::FieldExtractor* fe = schema->GetFieldExtractor(ft_idx.field);
            FMA_DBG_ASSERT(fe);
            schema->MarkFullTextIndexed(fe->GetFieldId(), true);
        } else {
            FMA_ERR() << "Unknown index type: " << index_name;
        }
    }
}

IndexManager::~IndexManager() {}

bool IndexManager::AddFullTextIndex(KvTransaction& txn, bool is_vertex, const std::string& label,
                                    const std::string& field) {
    _detail::IndexEntry ft_idx;
    ft_idx.label = label;
    ft_idx.field = field;
    ft_idx.table_name = GetFullTextIndexTableName(is_vertex, label, field);

    auto it = index_list_table_->GetIterator(txn, Value::ConstRef(ft_idx.table_name));
    if (it->IsValid()) return false;  // already exist
    Value idxv;
    StoreIndex(ft_idx, idxv);
    it->AddKeyValue(Value::ConstRef(ft_idx.table_name), idxv);
    return true;
}

bool IndexManager::AddVertexIndex(KvTransaction& txn, const std::string& label,
                                  const std::string& field, FieldType dt, IndexType type,
                                  std::unique_ptr<VertexIndex>& index) {
    if (dt == FieldType::BLOB) throw InputError("BLOB fields cannot be indexed.");
    _detail::IndexEntry idx;
    idx.label = label;
    idx.field = field;
    idx.table_name = GetVertexIndexTableName(label, field);
    idx.type = type;

    auto it = index_list_table_->GetIterator(txn, Value::ConstRef(idx.table_name));
    if (it->IsValid()) return false;  // already exist
    Value idxv;
    StoreIndex(idx, idxv);
    it->AddKeyValue(Value::ConstRef(idx.table_name), idxv);

    auto tbl = VertexIndex::OpenTable(txn, db_->GetStore(), idx.table_name, dt, type);
    index.reset(new VertexIndex(std::move(tbl), dt, type));  // creates index table
    return true;
}

bool IndexManager::AddEdgeIndex(KvTransaction& txn, const std::string& label,
                                const std::string& field, FieldType dt, IndexType type,
                                std::unique_ptr<EdgeIndex>& index) {
    if (dt == FieldType::BLOB) throw InputError("BLOB fields cannot be indexed.");
    _detail::IndexEntry idx;
    idx.label = label;
    idx.field = field;
    idx.table_name = GetEdgeIndexTableName(label, field);
    idx.type = type;

    auto it = index_list_table_->GetIterator(txn, Value::ConstRef(idx.table_name));
    if (it->IsValid()) return false;  // already exist
    Value idxv;
    StoreIndex(idx, idxv);
    it->AddKeyValue(Value::ConstRef(idx.table_name), idxv);

    auto tbl = EdgeIndex::OpenTable(txn, db_->GetStore(), idx.table_name, dt, type);
    index.reset(new EdgeIndex(std::move(tbl), dt, type));  // creates index table
    return true;
}

bool IndexManager::DeleteFullTextIndex(KvTransaction& txn, bool is_vertex, const std::string& label,
                                       const std::string& field) {
    std::string table_name = GetFullTextIndexTableName(is_vertex, label, field);
    if (!index_list_table_->DeleteKey(txn, Value::ConstRef(table_name)))
        return false;  // does not exist
    return true;
}

bool IndexManager::DeleteVertexIndex(KvTransaction& txn, const std::string& label,
                                     const std::string& field) {
    std::string table_name = GetVertexIndexTableName(label, field);
    // delete the entry from index list table
    if (!index_list_table_->DeleteKey(txn, Value::ConstRef(table_name)))
        return false;  // does not exist
                       // now delete the index table
    bool r = db_->GetStore().DeleteTable(txn, table_name);
    FMA_DBG_ASSERT(r);
    return true;
}

bool IndexManager::DeleteEdgeIndex(KvTransaction& txn, const std::string& label,
                                   const std::string& field) {
    std::string table_name = GetEdgeIndexTableName(label, field);
    // delete the entry from index list table
    if (!index_list_table_->DeleteKey(txn, Value::ConstRef(table_name)))
        return false;  // does not exist
    // now delete the index table
    bool r = db_->GetStore().DeleteTable(txn, table_name);
    FMA_DBG_ASSERT(r);
    return true;
}

}  // namespace lgraph
