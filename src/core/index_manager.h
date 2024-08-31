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

#include <atomic>
#include <mutex>
#include <map>

#include "fma-common/binary_buffer.h"
#include "fma-common/binary_read_write_helper.h"

#include "core/data_type.h"
#include "core/defs.h"
#include "core/vertex_index.h"
#include "core/edge_index.h"
#include "core/kv_store.h"
#include "core/lightning_graph.h"
#include "core/schema.h"
#include "core/schema_manager.h"
#include "core/transaction.h"

namespace lgraph {
namespace _detail {
struct IndexEntry {
    IndexEntry() {}
    IndexEntry(IndexEntry&& rhs)
        : label(std::move(rhs.label)),
          field(std::move(rhs.field)),
          table_name(std::move(rhs.table_name)),
          type(rhs.type) {}

    std::string label;
    std::string field;
    std::string table_name;
    IndexType type;

    template <typename StreamT>
    size_t Serialize(StreamT& buf) const {
        return BinaryWrite(buf, label) + BinaryWrite(buf, field) + BinaryWrite(buf, table_name) +
               BinaryWrite(buf, type);
    }

    template <typename StreamT>
    size_t Deserialize(StreamT& buf) {
        return BinaryRead(buf, label) + BinaryRead(buf, field) + BinaryRead(buf, table_name) +
               BinaryRead(buf, type);
    }
};

struct CompositeIndexEntry {
    CompositeIndexEntry() {}
    CompositeIndexEntry(CompositeIndexEntry&& rhs)
        : label(std::move(rhs.label)),
          n(rhs.n),
          field_names(std::move(rhs.field_names)),
          field_types(std::move(rhs.field_types)),
          table_name(std::move(rhs.table_name)),
          index_type(rhs.index_type) {}

    std::string label;
    int16_t n;
    std::vector<std::string> field_names;
    std::vector<FieldType> field_types;
    std::string table_name;
    CompositeIndexType index_type;

    template <typename StreamT>
    size_t Serialize(StreamT& buf) const {
        size_t res = BinaryWrite(buf, label) + BinaryWrite(buf, n);
        for (auto &f : field_names) {
            res += BinaryWrite(buf, f);
        }
        for (auto &f : field_types) {
            res += BinaryWrite(buf, f);
        }
        return res + BinaryWrite(buf, table_name) + BinaryWrite(buf, index_type);
    }

    template <typename StreamT>
    size_t Deserialize(StreamT& buf) {
        size_t res = BinaryRead(buf, label) + BinaryRead(buf, n);
        for (int i = 0; i < n; ++i) {
            std::string f;
            res += BinaryRead(buf, f);
            field_names.push_back(f);
        }
        for (int i = 0; i < n; ++i) {
            FieldType f;
            res += BinaryRead(buf, f);
            field_types.push_back(f);
        }
        return res + BinaryRead(buf, table_name) + BinaryRead(buf, index_type);
    }
};

}  // namespace _detail
}  // namespace lgraph

namespace lgraph {
class LightningGraph;

/** Manager for indexes.
\note This class must be created AFTER schema manager. It depends on schema
manager to get the information on data fields.
*/
class IndexManager {
 private:
    static std::string GetVertexIndexTableName(const std::string& label, const std::string& field) {
        return label + _detail::NAME_SEPARATOR + field + _detail::NAME_SEPARATOR +
               _detail::VERTEX_INDEX;
    }

    static std::string GetVertexCompositeIndexTableName(const std::string& label,
                                                        const std::vector<std::string>& fields) {
        std::string res = label + _detail::NAME_SEPARATOR;
        for (auto &s : fields) {
            res += s + _detail::NAME_SEPARATOR;
        }
        return res + _detail::COMPOSITE_INDEX;
    }

    static std::string GetEdgeIndexTableName(const std::string& label, const std::string& field) {
        return label + _detail::NAME_SEPARATOR + field + _detail::NAME_SEPARATOR +
               _detail::EDGE_INDEX;
    }

    static std::string GetFullTextIndexTableName(bool is_vertex, const std::string& label,
                                                 const std::string& field) {
        return label + _detail::NAME_SEPARATOR + field + _detail::NAME_SEPARATOR +
               (is_vertex ? _detail::VERTEX_FULLTEXT_INDEX : _detail::EDGE_FULLTEXT_INDEX);
    }

    static std::string GetVectorIndexTableName(const std::string& label, const std::string& field,
                                               const std::string& index_type, int vec_dimension,
                                               const std::string& distance_type,
                                               std::vector<int>& index_spec) {
        std::string INDEX_SPEC;
        INDEX_SPEC += '[';
        for (auto spec : index_spec) {
            INDEX_SPEC += std::to_string(spec) + ',';
        }
        INDEX_SPEC.pop_back();
        INDEX_SPEC += ']';
        return label + _detail::NAME_SEPARATOR + field + _detail::NAME_SEPARATOR + index_type +
               _detail::NAME_SEPARATOR + std::to_string(vec_dimension) +
               _detail::NAME_SEPARATOR + distance_type +
               _detail::NAME_SEPARATOR + INDEX_SPEC +
               _detail::NAME_SEPARATOR + _detail::VECTOR_INDEX;
    }

    static void GetLabelAndFieldFromTableName(const std::string& table_name, std::string& label,
                                              std::string& field) {
        size_t sep_len = strlen(_detail::NAME_SEPARATOR);
        size_t pos = table_name.find(_detail::NAME_SEPARATOR);
        FMA_ASSERT(pos != table_name.npos);
        label = table_name.substr(0, pos);
        pos += sep_len;
        size_t next_pos = table_name.find(_detail::NAME_SEPARATOR, pos);
        FMA_ASSERT(next_pos != table_name.npos);
        field = table_name.substr(pos, next_pos - pos);
    }

    static _detail::IndexEntry LoadIndex(const Value& v) {
        fma_common::BinaryBuffer buf(v.Data(), v.Size());
        _detail::IndexEntry idx;
        size_t r = fma_common::BinaryRead(buf, idx);
        if (r != v.Size()) THROW_CODE(InternalError, "Failed to load index meta info from buffer");
        return idx;
    }

    static _detail::CompositeIndexEntry LoadCompositeIndex(const Value& v) {
        fma_common::BinaryBuffer buf(v.Data(), v.Size());
        _detail::CompositeIndexEntry idx;
        size_t r = fma_common::BinaryRead(buf, idx);
        if (r != v.Size()) THROW_CODE(InternalError, "Failed to load composite "
                                      "index meta info from buffer");
        return idx;
    }

    static void StoreIndex(const _detail::IndexEntry& idx, Value& v) {
        fma_common::BinaryBuffer buf;
        fma_common::BinaryWrite(buf, idx);
        v.Copy(Value(buf.GetBuf(), buf.GetSize()));
    }

    static void StoreCompositeIndex(const _detail::CompositeIndexEntry& idx, Value& v) {
        fma_common::BinaryBuffer buf;
        fma_common::BinaryWrite(buf, idx);
        v.Copy(Value(buf.GetBuf(), buf.GetSize()));
    }

    LightningGraph* db_;
    std::unique_ptr<KvTable> index_list_table_;

 public:
    DISABLE_COPY(IndexManager);
    DISABLE_MOVE(IndexManager);

    /**
     * Load index information and set it in the schema_info. The new schema info
     * will be stored as the new schema info so later transactions can see it.
     *
     * \param [in,out]  txn         The transaction.
     * \param [in,out]  v_schema_manager vertex schema info
     * \param [in,out]  e_schema_manager edge schema info
     * \param           table       The table.
     * \param [in,out]  db If non-null, the database.
     */
    IndexManager(KvTransaction& txn, SchemaManager* v_schema_info, SchemaManager* e_schema_info,
                 std::unique_ptr<KvTable> index_list_table, LightningGraph* db);

    ~IndexManager();

    /**
     * Opens the table that stores index list. The table stores
     * index_name->IndexEntry
     *
     * \param [in,out]  txn         The transaction.
     * \param [in,out]  store       The store.
     * \param           table_name  Name of the table.
     *
     * \return  A KvTable.
     */
    static std::unique_ptr<KvTable> OpenIndexListTable(KvTransaction& txn, KvStore& store,
                                      const std::string& table_name) {
        return store.OpenTable(txn, table_name, true, ComparatorDesc::DefaultComparator());
    }

    bool AddVertexIndex(KvTransaction& txn, const std::string& label, const std::string& field,
                        FieldType dt, IndexType type, std::unique_ptr<VertexIndex>& index);

    bool AddVertexCompositeIndex(KvTransaction& txn, const std::string& label,
                                 const std::vector<std::string>& fields,
                                 const std::vector<FieldType>& types,
                                 CompositeIndexType type,
                                 std::shared_ptr<CompositeIndex>& index);

    bool AddVectorIndex(KvTransaction& txn, const std::string& label,
                        const std::string& field, const std::string& index_type,
                        int vec_dimension, const std::string& distance_type,
                        std::vector<int>& index_spec, FieldType dt, IndexType type,
                        std::unique_ptr<VertexIndex>& index,
                        std::unique_ptr<VectorIndex>& vector_index);

    bool AddEdgeIndex(KvTransaction& txn, const std::string& label, const std::string& field,
                      FieldType dt, IndexType type, std::unique_ptr<EdgeIndex>& index);

    bool AddFullTextIndex(KvTransaction& txn, bool is_vertex, const std::string& label,
                          const std::string& field);

    bool DeleteVertexIndex(KvTransaction& txn, const std::string& label, const std::string& field);

    bool DeleteEdgeIndex(KvTransaction& txn, const std::string& label, const std::string& field);

    bool DeleteVertexCompositeIndex(KvTransaction& txn, const std::string& label,
                                    const std::vector<std::string>& fields);

    bool DeleteVectorIndex(KvTransaction& txn, const std::string& label,
                           const std::string& field, const std::string& index_type,
                           int vec_dimension, const std::string& distance_type);

    bool DeleteFullTextIndex(KvTransaction& txn, bool is_vertex, const std::string& label,
                             const std::string& field);

    bool GetVectorIndexListTableName(KvTransaction& txn, std::vector<std::string>& table_name);

    // vertex index
    std::pair<std::vector<IndexSpec>, std::vector<CompositeIndexSpec>> ListAllIndexes(
        KvTransaction& txn) {
        std::vector<IndexSpec> indexes;
        std::vector<CompositeIndexSpec> compositeIndexes;
        IndexSpec is;
        CompositeIndexSpec cis;
        size_t v_index_len = strlen(_detail::VERTEX_INDEX);
        size_t e_index_len = strlen(_detail::EDGE_INDEX);
        size_t c_index_len = strlen(_detail::COMPOSITE_INDEX);
        auto it = index_list_table_->GetIterator(txn);
        for (it->GotoFirstKey(); it->IsValid(); it->Next()) {
            std::string index_name = it->GetKey().AsString();
            if (index_name.size() > v_index_len &&
                index_name.substr(index_name.size() - v_index_len) == _detail::VERTEX_INDEX) {
                _detail::IndexEntry ent = LoadIndex(it->GetValue());
                is.label = ent.label;
                is.field = ent.field;
                is.type = ent.type;
                indexes.emplace_back(std::move(is));
            } else if (index_name.size() > e_index_len &&
                index_name.substr(index_name.size() - e_index_len) == _detail::EDGE_INDEX) {
                _detail::IndexEntry ent = LoadIndex(it->GetValue());
                is.label = ent.label;
                is.field = ent.field;
                is.type = ent.type;
                indexes.emplace_back(std::move(is));
            } else if (index_name.size() > c_index_len &&
                       index_name.substr(index_name.size() - c_index_len) ==
                       _detail::COMPOSITE_INDEX) {
                _detail::CompositeIndexEntry idx = LoadCompositeIndex(it->GetValue());
                cis.label = idx.label;
                cis.fields = idx.field_names;
                cis.type = idx.index_type;
                compositeIndexes.emplace_back(std::move(cis));
            }
        }
        return {indexes, compositeIndexes};
    }
};
}  // namespace lgraph
