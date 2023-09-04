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
          is_unique(rhs.is_unique),
          is_global(rhs.is_global) {}

    std::string label;
    std::string field;
    std::string table_name;
    bool is_unique = false;
    bool is_global = false;

    template <typename StreamT>
    size_t Serialize(StreamT& buf) const {
        return BinaryWrite(buf, label) + BinaryWrite(buf, field) + BinaryWrite(buf, table_name) +
               BinaryWrite(buf, is_unique) + BinaryWrite(buf, is_global);
    }

    template <typename StreamT>
    size_t Deserialize(StreamT& buf) {
        return BinaryRead(buf, label) + BinaryRead(buf, field) + BinaryRead(buf, table_name) +
               BinaryRead(buf, is_unique) + BinaryRead(buf, is_global);
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
        return label + _detail::NAME_SEPERATOR + field + _detail::NAME_SEPERATOR +
               _detail::VERTEX_INDEX;
    }

    static std::string GetEdgeIndexTableName(const std::string& label, const std::string& field) {
        return label + _detail::NAME_SEPERATOR + field + _detail::NAME_SEPERATOR +
               _detail::EDGE_INDEX;
    }

    static std::string GetFullTextIndexTableName(bool is_vertex, const std::string& label,
                                                 const std::string& field) {
        return label + _detail::NAME_SEPERATOR + field + _detail::NAME_SEPERATOR +
               (is_vertex ? _detail::VERTEX_FULLTEXT_INDEX : _detail::EDGE_FULLTEXT_INDEX);
    }

    static void GetLabelAndFieldFromTableName(const std::string& table_name, std::string& label,
                                              std::string& field) {
        size_t sep_len = strlen(_detail::NAME_SEPERATOR);
        size_t pos = table_name.find(_detail::NAME_SEPERATOR);
        FMA_ASSERT(pos != table_name.npos);
        label = table_name.substr(0, pos);
        pos += sep_len;
        size_t next_pos = table_name.find(_detail::NAME_SEPERATOR, pos);
        FMA_ASSERT(next_pos != table_name.npos);
        field = table_name.substr(pos, next_pos - pos);
    }

    static _detail::IndexEntry LoadIndex(const Value& v) {
        fma_common::BinaryBuffer buf(v.Data(), v.Size());
        _detail::IndexEntry idx;
        size_t r = fma_common::BinaryRead(buf, idx);
        if (r != v.Size()) throw InternalError("Failed to load index meta info from buffer");
        return idx;
    }

    static void StoreIndex(const _detail::IndexEntry& idx, Value& v) {
        fma_common::BinaryBuffer buf;
        fma_common::BinaryWrite(buf, idx);
        v.Copy(Value(buf.GetBuf(), buf.GetSize()));
    }

    LightningGraph* db_;
    KvTable index_list_table_;

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
                 const KvTable& index_list_table, LightningGraph* db);

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
    static KvTable OpenIndexListTable(KvTransaction& txn, KvStore& store,
                                      const std::string& table_name) {
        return store.OpenTable(txn, table_name, true, ComparatorDesc::DefaultComparator());
    }

    bool AddVertexIndex(KvTransaction& txn, const std::string& label, const std::string& field,
                        FieldType dt, bool is_unique, std::unique_ptr<VertexIndex>& index);

    bool AddEdgeIndex(KvTransaction& txn, const std::string& label, const std::string& field,
                      FieldType dt, bool is_unique, bool is_global,
                      std::unique_ptr<EdgeIndex>& index);

    bool AddFullTextIndex(KvTransaction& txn, bool is_vertex, const std::string& label,
                          const std::string& field);

    bool DeleteVertexIndex(KvTransaction& txn, const std::string& label, const std::string& field);

    bool DeleteEdgeIndex(KvTransaction& txn, const std::string& label, const std::string& field);

    bool DeleteFullTextIndex(KvTransaction& txn, bool is_vertex, const std::string& label,
                             const std::string& field);

    // vertex index
    std::vector<IndexSpec> ListAllIndexes(KvTransaction& txn) {
        std::vector<IndexSpec> indexes;
        IndexSpec is;
        size_t v_index_len = strlen(_detail::VERTEX_INDEX);
        size_t e_index_len = strlen(_detail::EDGE_INDEX);
        for (auto it = index_list_table_.GetIterator(txn); it.IsValid(); it.Next()) {
            std::string index_name = it.GetKey().AsString();
            if (index_name.size() > v_index_len &&
                index_name.substr(index_name.size() - v_index_len) == _detail::VERTEX_INDEX) {
                _detail::IndexEntry ent = LoadIndex(it.GetValue());
                is.label = ent.label;
                is.field = ent.field;
                is.unique = ent.is_unique;
                indexes.emplace_back(std::move(is));
            }
            if (index_name.size() > e_index_len &&
                index_name.substr(index_name.size() - e_index_len) == _detail::EDGE_INDEX) {
                _detail::IndexEntry ent = LoadIndex(it.GetValue());
                is.label = ent.label;
                is.field = ent.field;
                is.unique = ent.is_unique;
                indexes.emplace_back(std::move(is));
            }
        }
        return indexes;
    }
};
}  // namespace lgraph
