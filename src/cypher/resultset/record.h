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

//
// Created by wt on 6/14/18.
//
#pragma once

#include <utility>
#include <unordered_map>
#include "core/data_type.h"  // lgraph::FieldData
#include "cypher/cypher_types.h"
#include "parser/data_typedef.h"
#include "graph/node.h"
#include "graph/relationship.h"
#include "cypher/resultset/column_vector.h"

namespace cypher {

struct SymbolTable;
class RTContext;

struct Entry {
    cypher::FieldData constant;
    union {
        Node *node = nullptr;
        Relationship *relationship;
    };

    enum RecordEntryType {
        UNKNOWN = 0,
        CONSTANT,
        NODE,
        RELATIONSHIP,
        VAR_LEN_RELP,
        HEADER,  // TODO(anyone) useless?
        NODE_SNAPSHOT,
        RELP_SNAPSHOT,
    } type;

    Entry() = default;

    explicit Entry(const cypher::FieldData &v) : constant(v), type(CONSTANT) {}

    explicit Entry(cypher::FieldData &&v) : constant(std::move(v)), type(CONSTANT) {}

    explicit Entry(const lgraph::FieldData &v) : constant(v), type(CONSTANT) {}

    explicit Entry(lgraph::FieldData &&v) : constant(std::move(v)), type(CONSTANT) {}

    explicit Entry(Node *v) : node(v), type(NODE) {}

    explicit Entry(Relationship *v)
        : relationship(v), type(v->VarLen() ? VAR_LEN_RELP : RELATIONSHIP) {}

    Entry(const Entry &rhs) = default;

    Entry(Entry &&rhs) = default;

    Entry &operator=(const Entry &rhs) = default;

    Entry &operator=(Entry &&rhs) = default;

    bool EqualNull() const {
        switch (type) {
        case CONSTANT:
            return constant.EqualNull();
        case NODE:
            return !node || node->PullVid() < 0;
        case RELATIONSHIP:
            return !relationship || !relationship->ItRef()->IsValid();
        case VAR_LEN_RELP:
            return !relationship || relationship->path_.Empty();
        case NODE_SNAPSHOT:
        case RELP_SNAPSHOT:
            CYPHER_TODO();
        default:
            return false;
        }
    }

    bool IsNull() const { return type == CONSTANT && constant.IsNull(); }

    bool IsArray() const { return type == CONSTANT && constant.type == cypher::FieldData::ARRAY; }

    bool IsMap() const { return type == CONSTANT && constant.type == cypher::FieldData::MAP; }

    bool IsScalar() const { return type == CONSTANT && constant.type == cypher::FieldData::SCALAR; }

    bool IsConstant() const { return IsNull() || IsScalar() || IsArray() || IsMap(); }

    bool IsBool() const { return type == CONSTANT && constant.IsBool(); }

    bool IsInteger() const { return type == CONSTANT && constant.IsInteger(); }

    bool IsReal() const { return type == CONSTANT && constant.IsReal(); }

    bool IsString() const { return type == CONSTANT && constant.IsString(); }

    bool IsNode() const { return type == NODE && node; }

    bool IsPoint() const { return type == CONSTANT && constant.IsPoint(); }

    bool IsLineString() const { return type == CONSTANT && constant.IsLineString(); }

    bool IsPolygon() const { return type == CONSTANT && constant.IsPolygon(); }

    bool IsSpatial() const { return type == CONSTANT && constant.IsSpatial(); }

    bool IsRelationship() const { return type == RELATIONSHIP && relationship; }

    bool operator==(const Entry &rhs) const {
        /* Handle null specially, e.g.:
         * CASE r WHEN null THEN false ELSE true END  */
        switch (type) {
        case CONSTANT:
            return (type == rhs.type && constant == rhs.constant) ||
                   (EqualNull() && rhs.EqualNull());
        case NODE:
            return (EqualNull() && rhs.EqualNull()) ||
                   (type == rhs.type && node && rhs.node && node->PullVid() == rhs.node->PullVid());
        case RELATIONSHIP:
            return (EqualNull() && rhs.EqualNull()) ||
                   (type == rhs.type && relationship && rhs.relationship && relationship->ItRef() &&
                    rhs.relationship->ItRef() &&
                    relationship->ItRef()->GetUid() == rhs.relationship->ItRef()->GetUid());
        case VAR_LEN_RELP:
        case NODE_SNAPSHOT:
        case RELP_SNAPSHOT:
            CYPHER_TODO();
        default:
            return false;
        }
    }

    bool operator!=(const Entry &rhs) const { return !(*this == rhs); }

    bool operator>(const Entry &rhs) const {
        switch (type) {
        case CONSTANT:
            return constant > rhs.constant;
        default:
            return false;
        }
    }

    bool operator<(const Entry &rhs) const {
        switch (type) {
        case CONSTANT:
            return constant < rhs.constant;
        default:
            return false;
        }
    }

    /* Get field value of node or relationship. */
    lgraph::FieldData GetEntityField(RTContext *ctx, const std::string &fd) const;

    bool CheckEntityEfficient(RTContext *ctx) const;

    static std::string ToString(const RecordEntryType &type) {
        switch (type) {
        case UNKNOWN:
            return "UNKNOWN";
        case CONSTANT:
            return "CONSTANT";
        case NODE:
            return "NODE";
        case RELATIONSHIP:
            return "RELATIONSHIP";
        case VAR_LEN_RELP:
            return "VAR_LEN_RELP";
        case HEADER:
            return "HEADER";
        case NODE_SNAPSHOT:
            return "NODE_SNAPSHOT";
        case RELP_SNAPSHOT:
            return "RELP_SNAPSHOT";
        default:
            throw lgraph::CypherException("unknown RecordEntryType");
        }
    }

    std::string ToString(const std::string &null_value = "__null__") const {
        switch (type) {
        case CONSTANT:
            return constant.ToString(null_value);
        case NODE_SNAPSHOT:
        case RELP_SNAPSHOT:
            // TODO(anyone) use integers
            return constant.scalar.string();
        case NODE:
            {
                CYPHER_THROW_ASSERT(node);
                auto vid = node->PullVid();
                if (vid < 0) return null_value;
                // V[{id}]
                std::string str("V[");
                str.append(std::to_string(vid)).append("]");
                return str;
            }
        case RELATIONSHIP:
            {
                CYPHER_THROW_ASSERT(relationship);
                auto eit = relationship->ItRef();
                if (!eit || !eit->IsValid()) return null_value;
                // E[src,dst,lid,eid]
                std::string str("E[");
                str.append(cypher::_detail::EdgeUid2String(eit->GetUid())).append("]");
                return str;
            }
        case VAR_LEN_RELP:
            return relationship->path_.ToString();
        default:
            return null_value;
        }
    }

    Entry &Snapshot() {
        switch (type) {
        case CONSTANT:
            break;
        case NODE:
            /* TODO(anyone) save VID in integer constant */
            constant = ::lgraph::FieldData(ToString());
            type = NODE_SNAPSHOT;
            break;
        case RELATIONSHIP:
            /* TODO(anyone) save <EID> in integer list */
            constant = ::lgraph::FieldData(ToString());
            type = RELP_SNAPSHOT;
            break;
        case VAR_LEN_RELP:
            constant = ::lgraph::FieldData(ToString());
            CYPHER_TODO();
        default:
            break;
        }
        return *this;
    }
};

struct Record {
    std::vector<Entry> values;
    const SymbolTable *symbol_table = nullptr;

    Record() = default;

    explicit Record(size_t size) { values.resize(size); }

    Record(size_t size, const SymbolTable *sym_tab, const PARAM_TAB &ptab)
        : symbol_table(sym_tab) {
        values.resize(size);
        SetParameter(ptab);
    }

    Record(const Record &rhs) {
        values = rhs.values;
        symbol_table = rhs.symbol_table;
    }

    void AddConstant(const cypher::FieldData &value) {
        // equivalent to: values.push_back(Entry(value));
        values.emplace_back(value);
    }

    void AddConstant(const lgraph::FieldData &scalar) {
        values.emplace_back(cypher::FieldData(scalar));
    }

    void AddNode(Node *value) { values.emplace_back(value); }

    void AddRelationship(Relationship *value) { values.emplace_back(value); }

    void AddEntry(const Entry &value) { values.emplace_back(value); }

    /* Set parameters if param_tab not empty. Note this is
     * a runtime method.  */
    void SetParameter(const PARAM_TAB &ptab);

    void Merge(const Record &rhs) {
        size_t len = rhs.values.size();
        if (values.size() < len) values.resize(len);
        for (size_t i = 0; i < len; i++) {
            if (rhs.values[i].type != Entry::UNKNOWN) {
                values[i] = rhs.values[i];
            }
        }
    }

    bool Null() const {
        for (auto &v : values)
            if (!v.EqualNull()) return false;
        return true;
    }

    std::string ToString() const {
        std::string line;
        auto size = values.size();
        if (size == 0) return line;
        for (size_t i = 0; i < size - 1; i++) {
            line.append(values[i].ToString()).append(",");
        }
        line.append(values[size - 1].ToString());
        return line;
    }

    Record &Snapshot() {
        for (auto &v : values) {
            v.Snapshot();
        }
        return *this;
    }
};

struct DataChunk {
    std::unordered_map<std::string, std::unique_ptr<ColumnVector>> columnar_data_;
    std::unordered_map<std::string, std::unique_ptr<ColumnVector>> string_columns_;
    std::unordered_map<std::string, uint32_t> property_positions_;
    std::unordered_map<std::string, std::vector<uint32_t>> property_vids_;

    DataChunk() = default;

    void CopyColumn(const std::string& column_name, const DataChunk& source_record,
                    bool overwrite_existing = true) {
        if (source_record.columnar_data_.find(column_name) != source_record.columnar_data_.end()) {
            const auto& src_vector = source_record.columnar_data_.at(column_name);
            if (overwrite_existing || columnar_data_.find(column_name) == columnar_data_.end()) {
                auto dst_vector = std::make_unique<ColumnVector>(*src_vector);
                columnar_data_[column_name] = std::move(dst_vector);
                property_positions_[column_name] =
                    source_record.property_positions_.at(column_name);
                property_vids_[column_name] =
                    source_record.property_vids_.at(column_name);
            }
        } else if (source_record.string_columns_.find(column_name) !=
                   source_record.string_columns_.end()) {
            const auto& src_vector = source_record.string_columns_.at(column_name);
            if (overwrite_existing || string_columns_.find(column_name) ==
                string_columns_.end()) {
                auto dst_vector = std::make_unique<ColumnVector>(*src_vector);
                string_columns_[column_name] = std::move(dst_vector);
                property_positions_[column_name] =
                    source_record.property_positions_.at(column_name);
                property_vids_[column_name] = source_record.property_vids_.at(column_name);
            }
        }
    }

    void MergeColumn(const DataChunk& source_record, bool overwrite_existing = true) {
        for (const auto& pair : source_record.columnar_data_) {
            CopyColumn(pair.first, source_record, overwrite_existing);
        }
        for (const auto& pair : source_record.string_columns_) {
            CopyColumn(pair.first, source_record, overwrite_existing);
        }
    }

    void TruncateData(int usable_r) {
        std::set<uint32_t> sorted_vids;
        for (const auto& pair : property_vids_) {
            const auto& vids = pair.second;
            sorted_vids.insert(vids.begin(), vids.end());
        }

        std::vector<uint32_t> selected_vids;
        for (auto it = sorted_vids.begin(); it != sorted_vids.end() &&
            selected_vids.size() < static_cast<size_t>(usable_r); ++it) {
            selected_vids.push_back(*it);
        }
        for (auto& pair : columnar_data_) {
            const std::string& column_name = pair.first;
            auto& column_vector = pair.second;
            const auto& vids = property_vids_.at(column_name);
            auto new_vector = std::make_unique<ColumnVector>(
                              column_vector->GetElementSize(), usable_r,
                              column_vector->GetFieldType());
            std::vector<uint32_t> new_vids;
            uint32_t new_pos = 0;
            for (uint32_t selected_vid : selected_vids) {
                auto it = std::find(vids.begin(), vids.end(), selected_vid);
                if (it != vids.end()) {
                    uint32_t original_pos = std::distance(vids.begin(), it);
                    if (!column_vector->IsNull(original_pos)) {
                        std::memcpy(new_vector->data() + new_pos * new_vector->GetElementSize(),
                                    column_vector->data() + original_pos *
                                        column_vector->GetElementSize(),
                                    column_vector->GetElementSize());
                        new_vids.push_back(selected_vid);
                    }
                    new_pos++;
                }
            }
            column_vector = std::move(new_vector);
            property_vids_[column_name] = new_vids;
            property_positions_[column_name] = new_pos;
        }

        for (auto& pair : string_columns_) {
            const std::string& column_name = pair.first;
            auto& column_vector = pair.second;
            const auto& vids = property_vids_.at(column_name);
            auto new_vector = std::make_unique<ColumnVector>(
                              sizeof(cypher_string_t), usable_r, column_vector->GetFieldType());
            std::vector<uint32_t> new_vids;
            uint32_t new_pos = 0;
            for (uint32_t selected_vid : selected_vids) {
                auto it = std::find(vids.begin(), vids.end(), selected_vid);
                if (it != vids.end()) {
                    uint32_t original_pos = std::distance(vids.begin(), it);
                    if (!column_vector->IsNull(original_pos)) {
                        StringColumn::AddString(new_vector.get(), new_pos,
                            column_vector->GetValue<cypher_string_t>(original_pos).GetAsString());
                        new_vids.push_back(selected_vid);
                    }
                    new_pos++;
                }
            }
            column_vector = std::move(new_vector);
            property_vids_[column_name] = new_vids;
            property_positions_[column_name] = new_pos;
        }
    }

    void Append(const DataChunk& source_record) {
        for (const auto& pair : source_record.string_columns_) {
            const std::string& column_name = pair.first;
            const auto& src_vector = pair.second;
            uint32_t size = source_record.property_positions_.at(column_name);
            // PrintStringColumnData(column_name, *src_vector);
            if (string_columns_.find(column_name) == string_columns_.end()) {
                string_columns_[column_name] = std::make_unique<ColumnVector>(*src_vector);
                property_positions_[column_name] =
                    source_record.property_positions_.at(column_name);
                property_vids_[column_name] = source_record.property_vids_.at(column_name);
            } else {
                auto& dst_vector = string_columns_[column_name];
                uint32_t old_size = property_vids_[column_name].size();
                uint32_t new_size = old_size + size;
                auto new_vector = std::make_unique<ColumnVector>(
                                  sizeof(cypher_string_t), new_size, dst_vector->GetFieldType());
                for (uint32_t i = 0; i < old_size; ++i) {
                    const cypher_string_t& dst_string = dst_vector->GetValue<cypher_string_t>(i);
                    StringColumn::AddString(new_vector.get(), i, dst_string.GetAsString());
                }
                for (uint32_t i = 0; i < size; ++i) {
                    const cypher_string_t& src_string = src_vector->GetValue<cypher_string_t>(i);
                    StringColumn::AddString(new_vector.get(),
                                            old_size + i, src_string.GetAsString());
                }
                string_columns_[column_name] = std::move(new_vector);
                property_positions_[column_name] +=
                    source_record.property_positions_.at(column_name);
                auto& dst_vids = property_vids_[column_name];
                const auto& src_vids = source_record.property_vids_.at(column_name);
                dst_vids.insert(dst_vids.end(), src_vids.begin(), src_vids.end());
            }
        }

        for (const auto& pair : source_record.columnar_data_) {
            const std::string& column_name = pair.first;
            const auto& src_vector = pair.second;
            auto size = source_record.property_positions_.at(column_name);
            if (columnar_data_.find(column_name) == columnar_data_.end()) {
                columnar_data_[column_name] = std::make_unique<ColumnVector>(*src_vector);
                property_positions_[column_name] =
                    source_record.property_positions_.at(column_name);
                property_vids_[column_name] = source_record.property_vids_.at(column_name);
            } else {
                auto& dst_vector = columnar_data_[column_name];
                uint32_t old_size = property_vids_[column_name].size();
                uint32_t new_size = old_size + size;
                auto new_vector = std::make_unique<ColumnVector>(
                                  dst_vector->GetElementSize(), new_size,
                                  dst_vector->GetFieldType());
                std::memcpy(new_vector->data(), dst_vector->data(),
                            dst_vector->GetElementSize() * old_size);
                std::memcpy(new_vector->data() + old_size * new_vector->GetElementSize(),
                            src_vector->data(), src_vector->GetElementSize() * size);
                columnar_data_[column_name] = std::move(new_vector);
                property_positions_[column_name] +=
                    source_record.property_positions_.at(column_name);
                auto& dst_vids = property_vids_[column_name];
                const auto& src_vids = source_record.property_vids_.at(column_name);
                dst_vids.insert(dst_vids.end(), src_vids.begin(), src_vids.end());
            }
        }
    }

    void Print() const {
        std::cout << "DataChunk contents:\n";
        std::cout << "Columnar Data:\n";
        for (const auto& pair : columnar_data_) {
            std::cout << "  Column Name: " << pair.first << "\n";
            std::cout << "  Element Size: " << pair.second->GetElementSize() << "\n";
            std::cout << "  Capacity: " << pair.second->GetCapacity() << "\n";
            PrintColumnData(pair.first, *pair.second);
        }

        std::cout << "String Columns:\n";
        for (const auto& pair : string_columns_) {
            std::cout << "  Column Name: " << pair.first << "\n";
            std::cout << "  Element Size: " << pair.second->GetElementSize() << "\n";
            std::cout << "  Capacity: " << pair.second->GetCapacity() << "\n";
            PrintStringColumnData(pair.first, *pair.second);
        }

        std::cout << "Property Positions:\n";
        for (const auto& pair : property_positions_) {
            std::cout << "  Property Name: " << pair.first
                      << ", End Position: " << pair.second << "\n";
        }

        std::cout << "Property VIDs:\n";
        for (const auto& pair : property_vids_) {
            std::cout << "  Property Name: " << pair.first << ", VIDs: [";
            const auto& vids = pair.second;
            for (size_t i = 0; i < vids.size(); ++i) {
                std::cout << vids[i];
                if (i < vids.size() - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << "]\n";
        }
    }

    template<typename T>
    void PrintColumnData(const std::string& column_name, const ColumnVector& column) const {
        for (uint32_t i = 0; i < column.GetCapacity(); ++i) {
            if (!column.IsNull(i)) {
                std::cout << "    Data[" << i << "]: " << column.GetValue<T>(i) << "\n";
            } else {
                std::cout << "    Data[" << i << "]: NULL\n";
            }
        }
    }

    void PrintColumnData(const std::string& column_name, const ColumnVector& column) const {
        lgraph_api::FieldType field_type = column.GetFieldType();
        switch (field_type) {
            case lgraph_api::FieldType::BOOL:
                PrintColumnData<bool>(column_name, column);
                break;
            case lgraph_api::FieldType::INT8:
                PrintColumnData<int8_t>(column_name, column);
                break;
            case lgraph_api::FieldType::INT16:
                PrintColumnData<int16_t>(column_name, column);
                break;
            case lgraph_api::FieldType::INT32:
                PrintColumnData<int32_t>(column_name, column);
                break;
            case lgraph_api::FieldType::INT64:
                PrintColumnData<int64_t>(column_name, column);
                break;
            case lgraph_api::FieldType::FLOAT:
                PrintColumnData<float>(column_name, column);
                break;
            case lgraph_api::FieldType::DOUBLE:
                PrintColumnData<double>(column_name, column);
                break;
            default:
                std::cout << "Unsupported field type for column: " << column_name << "\n";
                break;
        }
    }

    void PrintStringColumnData(const std::string& column_name, const ColumnVector& column) const {
        for (uint32_t i = 0; i < column.GetCapacity(); ++i) {
            if (!column.IsNull(i)) {
                const cypher_string_t& value = column.GetValue<cypher_string_t>(i);
                std::cout << "    Data[" << i << "]: " << value.GetAsString() << "\n";
            } else {
                std::cout << "    Data[" << i << "]: NULL\n";
            }
        }
    }

    std::string Dump(bool is_standard) const {
        json arr = json::array();
        std::vector<std::string> column_names;
        for (const auto& pair : columnar_data_) {
            column_names.push_back(pair.first);
        }
        for (const auto& pair : string_columns_) {
            if (std::find(column_names.begin(), column_names.end(),
                pair.first) == column_names.end()) {
                column_names.push_back(pair.first);
            }
        }

        std::set<uint32_t> common_vids;
        for (const auto& pair : property_vids_) {
            const auto& vids = pair.second;
            common_vids.insert(vids.begin(), vids.end());
        }
        // for (uint32_t vid : common_vids) {
        //     std::cout << "Common VID: " << vid << "\n";
        // }
        for (uint32_t vid : common_vids) {
            json j;
            j["vid"] = vid;
            for (const auto& column_name : column_names) {
                if (property_vids_.find(column_name) != property_vids_.end()) {
                    const auto& vids = property_vids_.at(column_name);
                    auto it = std::find(vids.begin(), vids.end(), vid);
                    if (it != vids.end()) {
                        uint32_t column_pos = std::distance(vids.begin(), it);
                        if (columnar_data_.find(column_name) != columnar_data_.end()) {
                            const auto& column_vector = columnar_data_.at(column_name);
                            if (!column_vector->IsNull(column_pos)) {
                                lgraph_api::FieldType field_type = column_vector->GetFieldType();
                                switch (field_type) {
                                    case lgraph_api::FieldType::BOOL:
                                        j[column_name] =
                                            column_vector->GetValue<bool>(column_pos);
                                        break;
                                    case lgraph_api::FieldType::INT8:
                                        j[column_name] =
                                            column_vector->GetValue<int8_t>(column_pos);
                                        break;
                                    case lgraph_api::FieldType::INT16:
                                        j[column_name] =
                                            column_vector->GetValue<int16_t>(column_pos);
                                        break;
                                    case lgraph_api::FieldType::INT32:
                                        j[column_name] =
                                            column_vector->GetValue<int32_t>(column_pos);
                                        break;
                                    case lgraph_api::FieldType::INT64:
                                        j[column_name] =
                                            column_vector->GetValue<int64_t>(column_pos);
                                        break;
                                    case lgraph_api::FieldType::FLOAT:
                                        j[column_name] =
                                            column_vector->GetValue<float>(column_pos);
                                        break;
                                    case lgraph_api::FieldType::DOUBLE:
                                        j[column_name] =
                                            column_vector->GetValue<double>(column_pos);
                                        break;
                                    default:
                                        throw std::runtime_error(
                                                "Unsupported data type in columnar_data_");
                                }
                            }
                        }
                        if (string_columns_.find(column_name) != string_columns_.end()) {
                            const auto& column_vector = string_columns_.at(column_name);
                            if (!column_vector->IsNull(column_pos)) {
                                j[column_name] =
                                column_vector->GetValue<cypher_string_t>(column_pos).GetAsString();
                            }
                        }
                    }
                }
            }
            if (j.is_null()) {
                throw std::runtime_error(
                    "DataChunk has a null row! Maybe your new record is not a reference.");
            }
            arr.emplace_back(j);
        }
        // std::cout << "Dump Result: " << arr.dump(4) << std::endl;
        if (is_standard) {
            json output;
            output["header"] = column_names;
            output["is_standard"] = true;
            output["data"] = arr;
            return output.dump();
        } else {
            return arr.dump();
        }
    }
};


}  // namespace cypher
