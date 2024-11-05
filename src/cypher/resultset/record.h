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
#include "cypher/cypher_types.h"
#include "cypher/parser/data_typedef.h"
#include "cypher/graph/node.h"
#include "cypher/graph/relationship.h"

namespace cypher {

struct SymbolTable;
class RTContext;

struct PathElement {
    bool is_node;
    std::any element;
};

struct Entry {
    Value constant;
    std::vector<PathElement> path;
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
        PATH,
    } type;

    Entry() = default;

    explicit Entry(const Value &v) : constant(v), type(CONSTANT) {}

    explicit Entry(Value &&v) : constant(std::move(v)), type(CONSTANT) {}

    explicit Entry(Node *v) : node(v), type(NODE) {}

    explicit Entry(Relationship *v)
        : relationship(v), type(v->VarLen() ? VAR_LEN_RELP : RELATIONSHIP) {}

    Entry(const Entry &rhs) = default;

    Entry(Entry &&rhs) = default;

    bool EqualNull() const {
        switch (type) {
            case CONSTANT:
                return constant.IsNull();
            case NODE:
                return !node || !node->vertex_;
            case RELATIONSHIP:
                return !relationship || !relationship->edge_;
            case VAR_LEN_RELP:
                return !relationship || relationship->path_.Empty();
            case NODE_SNAPSHOT:
            case RELP_SNAPSHOT:
                CYPHER_TODO();
            default:
                return false;
        }
    }

    std::string UUID();

    Entry &operator=(const Entry &rhs) = default;

    Entry &operator=(Entry &&rhs) = default;

    bool IsNull() const { return type == CONSTANT && constant.IsNull(); }

    bool IsArray() const { return type == CONSTANT && constant.IsArray(); }

    bool IsMap() const { return type == CONSTANT && constant.IsMap(); }

    bool IsBool() const { return type == CONSTANT && constant.IsBool(); }

    bool IsInteger() const { return type == CONSTANT && constant.IsInteger(); }

    bool IsReal() const { return type == CONSTANT && constant.IsDouble(); }

    bool IsString() const { return type == CONSTANT && constant.IsString(); }

    bool IsScalar() const { return type == CONSTANT && constant.IsScalar();};

    bool IsConstant() const { return type == CONSTANT;}

    bool IsNode() const { return type == NODE && node; }

    bool IsPath() const { return type == PATH; }

    bool IsRelationship() const { return type == RELATIONSHIP && relationship; }

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

    bool operator==(const Entry &rhs) const {
        /* Handle null specially, e.g.:
         * CASE r WHEN null THEN false ELSE true END  */
        switch (type) {
        case CONSTANT:
            return (type == rhs.type && constant == rhs.constant) ||
                (EqualNull() && rhs.EqualNull());
        case NODE:
            return (EqualNull() && rhs.EqualNull()) ||
                (type == rhs.type && node && rhs.node && node->vertex_ == rhs.node->vertex_);
        case RELATIONSHIP:
            return (EqualNull() && rhs.EqualNull()) ||
                (type == rhs.type && relationship && rhs.relationship && relationship->edge_ ==  rhs.relationship->edge_);
        case VAR_LEN_RELP:
        case NODE_SNAPSHOT:
        case RELP_SNAPSHOT:
            CYPHER_TODO();
        default:
            return false;
        }
    }

    bool operator!=(const Entry &rhs) const { return !(*this == rhs); }

    /* Get field value of node or relationship. */
    Value GetEntityField(RTContext *ctx, const std::string &fd);

    //bool CheckEntityEfficient(RTContext *ctx) const;


    [[nodiscard]] std::string ToString() const {
        switch (type) {
            case CONSTANT:
                return constant.ToString();
            case NODE_SNAPSHOT:
            case RELP_SNAPSHOT:
                CYPHER_TODO();
                // TODO(anyone) use integers
                //return constant.scalar.string();
            case NODE:
            {
                CYPHER_THROW_ASSERT(node);
                if (!node->vertex_) {
                    CYPHER_TODO();
                }
                return std::to_string(node->vertex_->GetId());
                /*auto vid = node->PullVid();
                if (vid < 0) return null_value;
                // V[{id}]
                std::string str("V[");
                str.append(std::to_string(vid)).append("]");
                return str;*/
            }
            case RELATIONSHIP:
            {
                CYPHER_THROW_ASSERT(relationship);
                if (!relationship->edge_) {
                    CYPHER_TODO();
                }
                return std::to_string(relationship->edge_->GetId());
                /*auto eit = relationship->ItRef();
                if (!eit || !eit->IsValid()) return null_value;
                // E[src,dst,lid,eid]
                std::string str("E[");
                str.append(cypher::_detail::EdgeUid2String(eit->GetUid())).append("]");
                return str;*/
            }
            case VAR_LEN_RELP:
                CYPHER_TODO();
                //return relationship->path_.ToString();
            default:
                CYPHER_TODO();
        }
    }

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
            THROW_CODE(CypherException, "unknown RecordEntryType");
        }
    }
};

struct Record {
    std::vector<Entry> values;
    const SymbolTable *symbol_table = nullptr;

    Record() = default;

    explicit Record(size_t size) { values.resize(size); }

    Record(size_t size, const SymbolTable *sym_tab)
            : symbol_table(sym_tab) {
        values.resize(size);
    }

    void AddConstant(const Value &value) {
        values.emplace_back(value);
    }

    bool Null() const {
        for (auto &v : values)
            if (!v.EqualNull()) return false;
        return true;
    }

    std::string UUID();

    void AddNode(Node *value) { values.emplace_back(value); }

    void AddRelationship(Relationship *value) { values.emplace_back(value); }

    void AddEntry(const Entry &value) { values.emplace_back(value); }

    void Merge(const Record &rhs) {
        size_t len = rhs.values.size();
        if (values.size() < len) values.resize(len);
        for (size_t i = 0; i < len; i++) {
            if (rhs.values[i].type != Entry::UNKNOWN) {
                values[i] = rhs.values[i];
            }
        }
    }
};
}  // namespace cypher
