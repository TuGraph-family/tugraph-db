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
#include "core/vsag_hnsw.h"

namespace lgraph {
IndexManager::IndexManager(KvTransaction& txn, SchemaManager* v_schema_manager,
                           SchemaManager* e_schema_manager,
                           std::unique_ptr<KvTable> index_list_table,
                           LightningGraph* db)
    : db_(db), index_list_table_(std::move(index_list_table)) {
    size_t v_index_len = strlen(_detail::VERTEX_INDEX);
    size_t e_index_len = strlen(_detail::EDGE_INDEX);
    size_t c_index_len = strlen(_detail::COMPOSITE_INDEX);
    size_t v_ft_index_len = strlen(_detail::VERTEX_FULLTEXT_INDEX);
    size_t e_ft_index_len = strlen(_detail::EDGE_FULLTEXT_INDEX);
    size_t vector_index_len = strlen(_detail::VECTOR_INDEX);
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
        } else if (index_name.size() > c_index_len &&
                   index_name.substr(index_name.size() - c_index_len) == _detail::COMPOSITE_INDEX) {
            _detail::CompositeIndexEntry idx = LoadCompositeIndex(it->GetValue());
            FMA_DBG_CHECK_EQ(idx.table_name, it->GetKey().AsString());
            Schema* schema = v_schema_manager->GetSchema(idx.label);
            FMA_DBG_ASSERT(schema);
            auto tbl = CompositeIndex::OpenTable(
                txn, db_->GetStore(), idx.table_name, idx.field_types, idx.index_type);
            auto index = std::make_unique<CompositeIndex>(
                std::move(tbl), idx.field_types, idx.index_type);  // creates index table
            index->SetReady();
            schema->SetCompositeIndex(idx.field_names, index.release());
        } else if (index_name.size() > vector_index_len &&
                   index_name.substr(index_name.size() - vector_index_len) ==
                       _detail::VECTOR_INDEX) {
            _detail::IndexEntry idx = LoadIndex(it->GetValue());
            FMA_DBG_CHECK_EQ(idx.table_name, it->GetKey().AsString());
            Schema* schema = v_schema_manager->GetSchema(idx.label);
            FMA_DBG_ASSERT(schema);
            std::vector<std::string> vector_index;
            std::regex re(R"(_@lgraph@_|vector_index)");
            auto words_begin = std::sregex_token_iterator(index_name.begin(),
                                index_name.end(), re, -1,
                                std::regex_constants::match_not_bol |
                                std::regex_constants::match_not_eol);
            auto words_end = std::sregex_token_iterator();
            for (std::sregex_token_iterator i = words_begin; i != words_end; ++i) {
                if (!i->str().empty()) {
                    vector_index.emplace_back(i->str());
                }
            }
            auto label = vector_index[0];
            auto field = vector_index[1];
            auto index_type = vector_index[2];
            auto distance_type = vector_index[4];
            int vec_dimension = std::stoi(vector_index[3]);
            std::vector<int> index_spec;
            std::regex pattern("-?[0-9]+\\.?[0-9]*");
            std::sregex_iterator begin_it(vector_index[5].begin(),
                                 vector_index[5].end(), pattern), end_it;
            while (begin_it != end_it) {
                std::smatch match = *begin_it;
                index_spec.push_back(std::stof(match.str()));
                ++begin_it;
            }
            FMA_DBG_ASSERT(index_type == "HNSW");
            FMA_DBG_ASSERT(schema->DetachProperty());
            LOG_INFO() << FMA_FMT("start building vertex index for {}:{} in detached model",
                                  label, field);
            const _detail::FieldExtractor* extractor = schema->GetFieldExtractor(idx.field);
            FMA_DBG_ASSERT(extractor);
            std::unique_ptr<VectorIndex> vsag_index;
            vsag_index.reset(dynamic_cast<lgraph::VectorIndex*> (
                new HNSW(label, field, distance_type, index_type, vec_dimension, index_spec)));
            uint64_t count = 0;
            std::vector<std::vector<float>> floatvector;
            std::vector<int64_t> vids;
            auto kv_iter = schema->GetPropertyTable().GetIterator(txn);
            for (kv_iter->GotoFirstKey(); kv_iter->IsValid(); kv_iter->Next()) {
                auto prop = kv_iter->GetValue();
                if (extractor->GetIsNull(prop)) {
                    continue;
                }
                auto vid = graph::KeyPacker::GetVidFromPropertyTableKey(kv_iter->GetKey());
                auto vector = (extractor->GetConstRef(prop)).AsType<std::vector<float>>();
                floatvector.emplace_back(vector);
                vids.emplace_back(vid);
                count++;
            }
            vsag_index->Build();
            vsag_index->Add(floatvector, vids, count);
            kv_iter.reset();
            LOG_DEBUG() << "index count: " << count;

            std::unique_ptr<VertexIndex> vertex_index;
            vertex_index = std::make_unique<VertexIndex>(
                nullptr, extractor->Type(), IndexType::NonuniqueIndex);
            vertex_index->SetReady();
            schema->MarkVertexIndexed(extractor->GetFieldId(), vertex_index.release());
            schema->MarkVectorIndexed(extractor->GetFieldId(), vsag_index.release());

            LOG_INFO() << FMA_FMT("end building vector index for {}:{} in detached model",
                                  label, field);
        } else {
            LOG_ERROR() << "Unknown index type: " << index_name;
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
    if (dt == FieldType::BLOB) THROW_CODE(InputError, "BLOB fields cannot be indexed.");
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

bool IndexManager::AddVectorIndex(KvTransaction& txn, const std::string& label,
                                  const std::string& field, const std::string& index_type,
                                  int vec_dimension, const std::string& distance_type,
                                  std::vector<int>& index_spec, FieldType dt, IndexType type,
                                  std::unique_ptr<VertexIndex>& index,
                                  std::unique_ptr<VectorIndex>& vector_index) {
    _detail::IndexEntry idx;
    idx.label = label;
    idx.field = field;
    idx.table_name = GetVectorIndexTableName(label, field, index_type,
                                vec_dimension, distance_type, index_spec);
    idx.type = type;

    auto it = index_list_table_->GetIterator(txn, Value::ConstRef(idx.table_name));
    if (it->IsValid()) return false;  // already exist

    Value idxv;
    StoreIndex(idx, idxv);
    it->AddKeyValue(Value::ConstRef(idx.table_name), idxv);

    index = std::make_unique<VertexIndex>(nullptr, dt, type);  // no need to creates index table

    if (index_type == "HNSW") {
        vector_index.reset(dynamic_cast<lgraph::VectorIndex*> (new HNSW(label, field, distance_type,
                            index_type, vec_dimension, index_spec)));
    }
    return true;
}

bool IndexManager::AddVertexCompositeIndex(KvTransaction& txn, const std::string& label,
                                           const std::vector<std::string>& fields,
                                           const std::vector<FieldType>& types,
                                           CompositeIndexType type,
                                           std::shared_ptr<CompositeIndex>& index) {
    for (auto &dt : types) {
        if (dt == FieldType::BLOB) THROW_CODE(InputError, "BLOB fields cannot be indexed.");
    }
    _detail::CompositeIndexEntry cidx;
    cidx.label = label;
    cidx.n = fields.size();
    cidx.field_names = fields;
    cidx.field_types = types;
    cidx.table_name = GetVertexCompositeIndexTableName(label, fields);
    cidx.index_type = type;
    auto it = index_list_table_->GetIterator(txn, Value::ConstRef(cidx.table_name));
    if (it->IsValid()) return false;
    Value idxv;
    StoreCompositeIndex(cidx, idxv);
    it->AddKeyValue(Value::ConstRef(cidx.table_name), idxv);

    auto tbl = CompositeIndex::OpenTable(txn, db_->GetStore(), cidx.table_name, types, type);
    index.reset(new CompositeIndex(std::move(tbl), types, type));  // creates index table
    return true;
}

bool IndexManager::AddEdgeIndex(KvTransaction& txn, const std::string& label,
                                const std::string& field, FieldType dt, IndexType type,
                                std::unique_ptr<EdgeIndex>& index) {
    if (dt == FieldType::BLOB) THROW_CODE(InputError, "BLOB fields cannot be indexed.");
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

bool IndexManager::DeleteVertexCompositeIndex(lgraph::KvTransaction& txn,
                                              const std::string& label,
                                              const std::vector<std::string>& fields) {
    std::string table_name = GetVertexCompositeIndexTableName(label, fields);
    // delete the entry from index list table
    if (!index_list_table_->DeleteKey(txn, Value::ConstRef(table_name)))
        return false;  // does not exist
                       // now delete the index table
    bool r = db_->GetStore().DeleteTable(txn, table_name);
    FMA_DBG_ASSERT(r);
    return true;
}

bool IndexManager::DeleteVectorIndex(KvTransaction& txn, const std::string& label,
                                     const std::string& field, const std::string& index_type,
                                     int vec_dimension, const std::string& distance_type) {
    std::string closest_table_name = label + _detail::NAME_SEPARATOR + field +
               _detail::NAME_SEPARATOR + index_type + _detail::NAME_SEPARATOR +
               std::to_string(vec_dimension) + _detail::NAME_SEPARATOR;
    auto table_name = (index_list_table_->GetClosestIterator(txn,
                        Value::ConstRef(closest_table_name))->GetKey()).AsString();
    // delete the entry from index list table
    if (!index_list_table_->DeleteKey(txn, Value::ConstRef(table_name)))
        return false;  // does not exist
                       // now delete the index table
    return true;
}

bool IndexManager::GetVectorIndexListTableName(KvTransaction& txn,
                                               std::vector<std::string>& table_name) {
    auto it = index_list_table_->GetIterator(txn);
    for (it->GotoFirstKey(); it->IsValid(); it->Next()) {
        auto key = it->GetKey();
        auto name = key.AsString();
        auto find = name.find(_detail::VECTOR_INDEX);
        if (find != std::string::npos) {
            table_name.emplace_back(name);
        }
    }
    return !table_name.empty();
}
}  // namespace lgraph
