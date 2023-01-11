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

#include <boost/any.hpp>
#include "procedure/procedure.h"
#include "expression.h"
#include "data_typedef.h"
#include "symbol_table.h"

namespace parser {

struct Clause {
    // pattern, hints(JOIN_ON), where-clause, optional
    typedef std::tuple<VEC_PATTERN, VEC_STR, Expression, bool> TYPE_MATCH;
    // distinct, return body
    typedef TUP_RETURN TYPE_RETURN;
    // distinct, return body, where
    typedef std::tuple<bool, TUP_RETURN_BODY, Expression> TYPE_WITH;
    typedef TUP_UNWIND TYPE_UNWIND;
    typedef TUP_CALL TYPE_CALL;
    typedef VEC_PATTERN TYPE_CREATE;
    typedef std::vector<Expression> TYPE_DELETE;
    typedef VEC_REMOVE TYPE_REMOVE;
    typedef VEC_SET TYPE_SET;
    typedef std::tuple<TUP_PATTERN_PART, VEC_SET, VEC_SET> TYPE_MERGE;

    enum ClauseType {
        MATCH,
        UNWIND,
        RETURN,
        WITH,
        WHERE,
        ORDERBY,
        CREATE,
        DELETE_,
        SET,
        REMOVE,
        MERGE,
        STANDALONECALL,
        INQUERYCALL,
        NA
    } type;

    boost::any data;

    Clause() : type(NA), data(0) {}

    const TYPE_MATCH &GetMatch() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<TYPE_MATCH>));
        return *boost::any_cast<std::shared_ptr<TYPE_MATCH>>(data);
    }

    const TYPE_UNWIND &GetUnwind() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<TYPE_UNWIND>));
        return *boost::any_cast<std::shared_ptr<TYPE_UNWIND>>(data);
    }

    const TYPE_CREATE &GetCreate() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<TYPE_CREATE>));
        return *boost::any_cast<std::shared_ptr<TYPE_CREATE>>(data);
    }

    const TYPE_MERGE &GetMerge() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<TYPE_MERGE>));
        return *boost::any_cast<std::shared_ptr<TYPE_MERGE>>(data);
    }

    const TYPE_RETURN &GetReturn() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<TYPE_RETURN>));
        return *boost::any_cast<std::shared_ptr<TYPE_RETURN>>(data);
    }

    const TYPE_WITH &GetWith() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<TYPE_WITH>));
        return *boost::any_cast<std::shared_ptr<TYPE_WITH>>(data);
    }

    const TYPE_SET &GetSet() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<TYPE_SET>));
        return *boost::any_cast<std::shared_ptr<TYPE_SET>>(data);
    }

    const TYPE_DELETE &GetDelete() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<TYPE_DELETE>));
        return *boost::any_cast<std::shared_ptr<TYPE_DELETE>>(data);
    }

    const TYPE_REMOVE &GetRemove() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<TYPE_REMOVE>));
        return *boost::any_cast<std::shared_ptr<TYPE_REMOVE>>(data);
    }

    const TYPE_CALL &GetCall() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<TYPE_CALL>));
        return *boost::any_cast<std::shared_ptr<TYPE_CALL>>(data);
    }

    bool Empty() const { return (type == NA); }

    std::string ToString() const {
        switch (type) {
        case MATCH:
            {
                std::string str("MATCH = {");
                auto match = GetMatch();
                auto pattern = std::get<0>(match);
                auto hints = std::get<1>(match);
                auto where = std::get<2>(match);
                auto optional = std::get<3>(match);
                str.append(Serialize(pattern))
                    .append(", HINTS = ")
                    .append(fma_common::ToString(hints))
                    .append(", WHERE = {")
                    .append(where.ToString())
                    .append("}");
                if (optional) str.append(", OPTIONAL");
                str.append("}");
                return str;
            }
        case UNWIND:
            {
                std::string str("UNWIND = {");
                auto &e = std::get<0>(GetUnwind());
                auto &var = std::get<1>(GetUnwind());
                str.append(e.ToString()).append(",").append(var).append("}");
                return str;
            }
        case CREATE:
            {
                std::string str("CREATE = {");
                str.append(Serialize(GetCreate())).append("}");
                return str;
            }
        case MERGE:
            {
                auto merge = GetMerge();
                auto merge_pattern_part = std::get<0>(merge);
                auto merge_on_match_set = std::get<1>(merge);  // VEC_SET
                auto merge_on_create_set =
                    std::get<2>(merge);  // need to serialise set_items to string
                std::string str("MERGE = {");
                str.append(Serialize(merge_pattern_part)).append(",");
                str.append("ON MATCH SET = {");
                str.append(Serialize(merge_on_match_set)).append("},");
                str.append("ON CREATE SET = {");
                str.append(Serialize(merge_on_create_set)).append("}");
                str.append("}");
                return str;
            }
        case RETURN:
            {
                std::string str("RETURN = {");
                str.append(Serialize(GetReturn())).append("}");
                return str;
            }
        case WITH:
            {
                std::string str("WITH = {");
                auto distinct = std::get<0>(GetWith());
                auto &body = std::get<1>(GetWith());
                auto &where = std::get<2>(GetWith());
                str.append(Serialize(std::make_tuple(distinct, body)))
                    .append(", where ")
                    .append(where.ToString())
                    .append("}");
                return str;
            }
        case REMOVE:
            {
                std::string str("REMOVE = {");
                for (auto &item : GetRemove()) {
                    str.append(item.ToString()).append(",");
                }
                str.pop_back();
                str.append("}");
                return str;
            }
        case STANDALONECALL:
        case INQUERYCALL:
            {
                std::string str("CALL = {");
                str.append(Serialize(GetCall())).append("}");
                return str;
            }
        default:
            return "unknown type:" + std::to_string(type);
        }
    }
};

struct QueryPart {
    std::vector<Clause> clauses;
    cypher::SymbolTable symbol_table;
    const Clause::TYPE_MATCH *match_clause = nullptr;
    const Clause::TYPE_UNWIND *unwind_clause = nullptr;
    const Clause::TYPE_RETURN *return_clause = nullptr;
    const Clause::TYPE_WITH *with_clause = nullptr;
    const Clause::TYPE_CALL *sa_call_clause = nullptr;
    const Clause::TYPE_CALL *iq_call_clause = nullptr;
    std::vector<const Clause::TYPE_CREATE *> create_clause;
    const Clause::TYPE_DELETE *delete_clause = nullptr;
    const Clause::TYPE_REMOVE *remove_clause = nullptr;
    std::vector<const Clause::TYPE_SET *> set_clause;
    std::vector<const Clause::TYPE_MERGE *> merge_clause;

    void AddClause(const Clause &clause) {
        clauses.emplace_back(clause);
        switch (clause.type) {
        case Clause::MATCH:
            if (match_clause) CYPHER_TODO();
            match_clause = &clauses.back().GetMatch();
            break;
        case Clause::UNWIND:
            if (unwind_clause) CYPHER_TODO();
            unwind_clause = &clauses.back().GetUnwind();
            break;
        case Clause::RETURN:
            if (return_clause) CYPHER_TODO();
            return_clause = &clauses.back().GetReturn();
            break;
        case Clause::WITH:
            if (with_clause) CYPHER_TODO();
            with_clause = &clauses.back().GetWith();
            break;
        case Clause::CREATE:
            create_clause.emplace_back(&clauses.back().GetCreate());
            break;
        case Clause::SET:
            set_clause.emplace_back(&clauses.back().GetSet()); 
            break;
        case Clause::DELETE_:
            delete_clause = &clauses.back().GetDelete();
            break;
        case Clause::REMOVE:
            remove_clause = &clauses.back().GetRemove();
            break;
        case Clause::MERGE:
            merge_clause.emplace_back(&clauses.back().GetMerge());
            break;
        case Clause::STANDALONECALL:
            if (sa_call_clause) CYPHER_TODO();
            sa_call_clause = &clauses.back().GetCall();
            break;
        case Clause::INQUERYCALL:
            if (iq_call_clause) CYPHER_TODO();
            iq_call_clause = &clauses.back().GetCall();
            break;
        default:
            CYPHER_TODO();
        }
    }

    bool ReadOnly(std::string& name, std::string& type) const {
        if (sa_call_clause) {
            auto &func_name = std::get<0>(*sa_call_clause);
            if (func_name == "db.plugin.callPlugin") {
                auto &plugins = std::get<1>(*sa_call_clause);
                if (plugins.size() > 2) {
                    name = std::move(plugins[1].ToString());
                    type = std::move(plugins[0].ToString());
                }
                return false;
            } else {
                auto pp = cypher::global_ptable.GetProcedure(func_name);
                if (pp && !pp->read_only) return false;
            }
        }
        /* create_clause && set_clause && delete_clause must set write txn */;
        return create_clause.empty() && set_clause.empty() && !delete_clause && !remove_clause &&
               merge_clause.empty();
    }

    void Enrich() {
        if (match_clause) {
            auto &pattern = std::get<0>(*match_clause);
            for (auto &pattern_part : pattern) {
                auto &var = std::get<0>(pattern_part);
                auto &pattern_element = std::get<1>(pattern_part);
                if (!var.empty()) {
                    /* copy content of anot_collection other than pointor,
                     * for the symbol_table will copied into execution plan. */
                    symbol_table.anot_collection.named_paths.emplace(
                        var, std::make_shared<parser::TUP_PATTERN_ELEMENT>(pattern_element));
                }
            }
        }
        // TODO: handle annotation in WITH/RETURN, UNWIND, DEL, etc. // NOLINT
    }

    std::string ToString() const {
        std::string s = "[";
        for (auto &clause : clauses) {
            s.append(clause.ToString()).append("\n");
        }
        if (!clauses.empty()) s.pop_back();
        s.append("]");
        return s;
    }
};

struct SglQuery {
    std::vector<QueryPart> parts;

    bool ReadOnly(std::string& name, std::string& type) const {
        for (auto &part : parts) {
            if (!part.ReadOnly(name, type)) return false;
        }
        return true;
    }

    std::string ToString() const {
        std::string s = "[\n";
        for (auto &part : parts) {
            s.append(part.ToString()).append("\n");
        }
        s.append("]\n");
        return s;
    }
};

// helper methods
static lgraph::FieldData MakeFieldData(const Expression &expr) {
    lgraph::FieldData ld;
    switch (expr.type) {
    case Expression::BOOL:
        ld = lgraph::FieldData(expr.Bool());
        break;
    case Expression::INT:
        ld = lgraph::FieldData(expr.Int());
        break;
    case Expression::DOUBLE:
        ld = lgraph::FieldData(expr.Double());
        break;
    case Expression::STRING:
        {
            ld = lgraph::FieldData(expr.String());
            break;
        }
    case Expression::PARAMETER:
        break;
    default:
        FMA_WARN() << "[Warning] " << __func__ << ": Error Type";
    }
    return ld;
}

// serialize for debug
static std::string Serialize(const TUP_PROPERTIES &properties) {
    std::string str("properties = {");
    auto &map_literal = std::get<0>(properties);
    auto &parameter = std::get<1>(properties);
    if (!parameter.empty()) {
        str.append(parameter);
    } else if (map_literal.type != Expression::NA) {
        str.append(map_literal.ToString());
    } else {
        str.clear();
        return str;
    }
    str.append("}");
    return str;
}

static std::string Serialize(const TUP_NODE_PATTERN &node_pattern) {
    std::string str("node_pattern = {");
    auto &variable = std::get<0>(node_pattern);
    auto &labels = std::get<1>(node_pattern);
    auto &properties = std::get<2>(node_pattern);
    str.append("variable = ").append(variable).append(", ");
    str.append("label = ").append(fma_common::ToString(labels));
    str.append(", ").append(Serialize(properties)).append("}");
    return str;
}

static std::string Serialize(const TUP_RELATIONSHIP_PATTERN &relationship_pattern) {
    auto direction = std::get<0>(relationship_pattern);
    auto &detail = std::get<1>(relationship_pattern);
    auto &variable = std::get<0>(detail);
    auto &relationship_types = std::get<1>(detail);
    auto &range_literal = std::get<2>(detail);
    auto &properties = std::get<3>(detail);
    std::string str("relationship_pattern = { ");
    if (direction == LinkDirection::RIGHT_TO_LEFT) str.append("<");
    str.append("- {").append(variable).append(", ");
    if (!relationship_types.empty()) str.append(relationship_types[0]);
    str.append(", ");
    if (range_literal[0] >= 0) {
        str.append("(")
            .append(std::to_string(range_literal[0]))
            .append(", ")
            .append(std::to_string(range_literal[1]))
            .append(")");
    }
    str.append(", ").append(Serialize(properties)).append("} -");
    if (direction == LinkDirection::LEFT_TO_RIGHT) str.append(">");
    str.append(" }");
    return str;
}

static std::string Serialize(const TUP_PATTERN_ELEMENT &pattern_element) {
    std::string str("pattern_element = {");
    auto &node_pattern = std::get<0>(pattern_element);
    auto &chains = std::get<1>(pattern_element);
    str.append(Serialize(node_pattern)).append(", ");
    int i = 0;
    for (auto &chain : chains) {
        auto &relp_patn = std::get<0>(chain);
        auto &node_patn = std::get<1>(chain);
        str.append("{")
            .append(Serialize(relp_patn))
            .append(", ")
            .append(Serialize(node_patn))
            .append("}");
        if (++i < (int)chains.size()) str.append(", ");
    }
    str.append("}");
    return str;
}

static std::string Serialize(const TUP_PATTERN_PART &pattern_part) {
    std::string str("pattern_part = {");
    auto &variable = std::get<0>(pattern_part);
    auto &element = std::get<1>(pattern_part);
    str.append(variable).append(", ").append(Serialize(element)).append("}");
    return str;
}

static std::string Serialize(const VEC_PATTERN &pattern) {
    std::string str("pattern of size ");
    str.append(std::to_string(pattern.size())).append(" = {");
    int i = 0;
    for (auto &part : pattern) {
        str.append(Serialize(part));
        if (++i < (int)pattern.size()) str.append(", ");
    }
    str.append("}");
    return str;
}

static std::string Serialize_1(const TUP_RETURN_ITEM &return_item) {
    std::string str("return_item = {");
    auto &expr = std::get<0>(return_item);
    auto &variable = std::get<1>(return_item);
    str.append(expr.ToString()).append(", ").append(variable).append("}");
    return str;
}

static std::string Serialize(const TUP_RETURN &return_c) {
    std::string str;
    bool distinct = std::get<0>(return_c);
    auto &return_body = std::get<1>(return_c);
    if (distinct) str.append("distinct, ");
    str.append("return_body = {");
    auto &items = std::get<0>(return_body);
    auto &sort_items = std::get<1>(return_body);
    auto &skip = std::get<2>(return_body);
    auto &limit = std::get<3>(return_body);
    str.append("items of size ").append(std::to_string(items.size())).append(" = {");
    int i = 0;
    for (auto &item : items) {
        str.append(Serialize_1(item));
        if (++i < (int)items.size()) str.append(", ");
    }
    str.append("}, order by {");
    i = 0;
    for (auto sort_item : sort_items) {
        str.append(std::to_string(sort_item.first)).append(sort_item.second ? "ASC" : "DESC");
        if (++i < (int)sort_items.size()) str.append(", ");
    }
    str.append("}");
    if (skip.type != Expression::NA) str.append(", skip ").append(skip.ToString());
    if (limit.type != Expression::NA) str.append(", limit ").append(limit.ToString());
    str.append("}");
    return str;
}

static std::string Serialize(const TUP_CALL &call) {
    auto str = std::get<0>(call);
    auto &parameters = std::get<1>(call);
    auto &yield_items = std::get<2>(call);
    str.append("(");
    int i = 0;
    for (auto &p : parameters) {
        str.append(p.ToString());
        if (++i < (int)parameters.size()) str.append(", ");
    }
    str.append(")::(");
    i = 0;
    for (auto &y : yield_items) {
        str.append(y);
        if (++i < (int)yield_items.size()) str.append(", ");
    }
    str.append(")");
    return str;
}

static std::string Serialize(const TUP_SET_ITEM &set_item) {
    std::string str("set_items ={");
    auto &variable = std::get<0>(set_item);
    auto &property_expr = std::get<1>(set_item);
    auto &symbol = std::get<2>(set_item);
    auto &properties = std::get<3>(set_item);
    auto &labels = std::get<4>(set_item);
    str.append("variable = ").append(variable).append(", ");
    str.append("property_expr = ").append(property_expr.ToString()).append(symbol);
    str.append("property = {").append(properties.ToString()).append("},");
    str.append("label = ").append(fma_common::ToString(labels)).append("}");

    return str;
}

static std::string Serialize(const VEC_SET &set) {
    std::string str("set_items of size ");
    str.append(std::to_string(set.size())).append(" = {");
    int i = 0;
    for (auto &item : set) {
        str.append(Serialize(item));
        if (++i < (int)set.size()) str.append(", ");
    }
    str.append("}");
    return str;
}

}  // namespace parser
