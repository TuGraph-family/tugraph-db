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
#include "core/data_type.h"  // lgraph::FieldData
#include "cypher/cypher_types.h"
#include "parser/data_typedef.h"
#include "graph/node.h"
#include "graph/relationship.h"

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
}  // namespace cypher
