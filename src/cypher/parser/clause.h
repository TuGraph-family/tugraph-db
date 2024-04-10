/**
 * Copyright 2024 AntGroup CO., Ltd.
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
#include "cypher/parser/expression.h"
#include "cypher/parser/data_typedef.h"
#include "cypher/parser/symbol_table.h"

namespace parser {

static std::string Serialize(const TUP_PROPERTIES &properties,
                             const size_t &cur_indent = 0, const size_t &indent_size = 2);
static std::string Serialize(const TUP_NODE_PATTERN &node_pattern,
                             const size_t &cur_indent = 0, const size_t &indent_size = 2);
static std::string Serialize(const TUP_RELATIONSHIP_PATTERN &relationship_pattern,
                             const size_t &cur_indent = 0, const size_t &indent_size = 2);
static std::string Serialize(const TUP_PATTERN_ELEMENT &pattern_element,
                             const size_t &cur_indent = 0, const size_t &indent_size = 2);
static std::string Serialize(const TUP_PATTERN_PART &pattern_part,
                             const size_t &cur_indent = 0, const size_t &indent_size = 2);
static std::string Serialize(const VEC_PATTERN &pattern,
                             const size_t &cur_indent = 0, const size_t &indent_size = 2);
static std::string Serialize(const TUP_RETURN &tup_return,
                             const size_t &cur_indent = 0, const size_t &indent_size = 2);
static std::string Serialize(const TUP_RETURN_BODY &return_body,
                             const size_t &cur_indent = 0, const size_t &indent_size = 2);
static std::string Serialize_1(const TUP_RETURN_ITEM &return_item,
                               const size_t &cur_indent = 0, const size_t &indent_size = 2);
static std::string Serialize(const TUP_CALL &call,
                             const size_t &cur_indent = 0, const size_t &indent_size = 2);
static std::string Serialize(const TUP_SET_ITEM &set_item,
                             const size_t &cur_indent = 0, const size_t &indent_size = 2);
static std::string Serialize(const VEC_SET &set,
                             const size_t &cur_indent = 0, const size_t &indent_size = 2);

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

    std::string ToString(const size_t &cur_indent, const size_t &indent_size = 2) const {
        size_t next_indent = cur_indent + indent_size;
        switch (type) {
        case MATCH:
            {
                auto match = GetMatch();
                auto pattern = std::get<0>(match);
                auto hints = std::get<1>(match);
                auto where = std::get<2>(match);
                auto optional = std::get<3>(match);
                return fma_common::StringFormatter::Format(
                    "{}MATCH = \\{\n"                   \
                    "{},\n"                             \
                    "{}HINTS = {},\n"                   \
                    "{}WHERE = {}"                      \
                    "{}\n"                              \
                    "{}\\}",
                    std::string(cur_indent, ' '),
                    Serialize(pattern, next_indent),
                    std::string(next_indent, ' '), fma_common::ToString(hints),
                    std::string(next_indent, ' '), where.ToString(),
                    optional ? fma_common::StringFormatter::Format(
                        ",\n{}OPTIONAL = true", std::string(next_indent, ' ')) : "",
                    std::string(cur_indent, ' '));
            }
        case UNWIND:
            {
                auto &e = std::get<0>(GetUnwind());
                auto &var = std::get<1>(GetUnwind());
                return fma_common::StringFormatter::Format(
                    "{}UNWIND = \\{{},{}\\}",
                    std::string(cur_indent, ' '),
                    e.ToString(), var);
            }
        case RETURN:
            {
                return Serialize(GetReturn(), cur_indent);
            }
        case WITH:
            {
                auto distinct = std::get<0>(GetWith());
                auto &body = std::get<1>(GetWith());
                auto &where = std::get<2>(GetWith());
                return fma_common::StringFormatter::Format(
                    "{}WITH = \\{\n"              \
                    "{},\n"                       \
                    "{}WHERE = {}\n"              \
                    "{}\\}",
                    std::string(cur_indent, ' '),
                    Serialize(std::make_tuple(distinct, body), next_indent),
                    std::string(cur_indent, ' '), where.ToString(),
                    std::string(cur_indent, ' '));
            }
        case CREATE:
            {
                return fma_common::StringFormatter::Format(
                    "{}CREATE = \\{\n"           \
                    "{}\n"                       \
                    "{}\\}",
                    std::string(cur_indent, ' '),
                    Serialize(GetCreate(), next_indent),
                    std::string(cur_indent, ' '));
            }
        case DELETE_:
            {
                auto& delete_clause = GetDelete();
                std::string type_delete;
                for (size_t i = 0; i < delete_clause.size(); i++) {
                    type_delete.append(delete_clause[i].ToString());
                    if (i < delete_clause.size() - 1) {
                        type_delete.append(",");
                    }
                }
                return fma_common::StringFormatter::Format(
                    "{}DELETE = \\{{}\\}",
                    std::string(cur_indent, ' '), type_delete);
            }
        case SET:
            {
                auto& set_clause = GetSet();
                return fma_common::StringFormatter::Format(
                    "{}SET = \\{\n"             \
                    "{}\n"                      \
                    "{}\\}",
                    std::string(cur_indent, ' '),
                    Serialize(set_clause, next_indent),
                    std::string(cur_indent, ' '));
            }
        case REMOVE:
            {
                auto& remove_clause = GetRemove();
                std::string type_remove;
                for (size_t i = 0; i < remove_clause.size(); i++) {
                    type_remove.append(remove_clause[i].ToString());
                    if (i < remove_clause.size() - 1) {
                        type_remove.append(",");
                    }
                }
                return fma_common::StringFormatter::Format(
                    "{}REMOVE = \\{{}\\}",
                    std::string(cur_indent, ' '), type_remove);
            }
        case MERGE:
            {
                auto merge = GetMerge();
                auto merge_pattern_part = std::get<0>(merge);
                auto merge_on_match_set = std::get<1>(merge);  // VEC_SET
                auto merge_on_create_set =
                    std::get<2>(merge);  // need to serialise set_items to string
                return fma_common::StringFormatter::Format(
                    "{}MERGE = \\{\n"                         \
                    "{},\n"                                   \
                    "{}ON_MATCH_SET = \\{\n"                  \
                    "{}\n"                                    \
                    "{}\\}\n"                                 \
                    "{}ON_CREATE_SET = \\{\n"                 \
                    "{}\n"                                    \
                    "{}\\}\n"                                 \
                    "{}\\}",
                    std::string(cur_indent, ' '),
                    Serialize(merge_pattern_part, next_indent),
                    std::string(next_indent, ' '),
                    Serialize(merge_on_match_set, next_indent + indent_size),
                    std::string(next_indent, ' '),
                    std::string(next_indent, ' '),
                    Serialize(merge_on_create_set, next_indent + indent_size),
                    std::string(next_indent, ' '),
                    std::string(cur_indent, ' '));
            }
        case STANDALONECALL:
        case INQUERYCALL:
            {
                return Serialize(GetCall(), cur_indent);
            }
        default:
            return fma_common::StringFormatter::Format(
                "{}unknown clause type: {}",
                std::string(cur_indent, ' '), type);
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
        /* create_clause && set_clause && delete_clause must set write txn */
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
        // TODO(anyone) handle annotation in WITH/RETURN, UNWIND, DEL, etc.
    }

    std::string ToString(const size_t &cur_indent, const size_t &indent_size = 2) const {
        size_t next_indent = cur_indent + indent_size;
        std::string s = std::string(cur_indent, ' ') + "[";
        for (size_t i = 0; i < clauses.size(); i++) {
            const auto& clause = clauses[i];
            s.append("\n").append(clause.ToString(next_indent));
            if (i < clauses.size() - 1) {
                s.append(",");
            } else {
                s.append("\n").append(std::string(cur_indent, ' '));
            }
        }
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

    std::string ToString(const size_t &cur_indent = 0, const size_t &indent_size = 2) const {
        size_t next_indent = cur_indent + indent_size;
        std::string s = std::string(cur_indent, ' ') + "[";
        for (size_t i = 0; i < parts.size(); i++) {
            const auto& part = parts[i];
            s.append("\n").append(part.ToString(next_indent));
            if (i < parts.size() - 1) {
                s.append(",");
            } else {
                s.append("\n").append(std::string(cur_indent, ' '));
            }
        }
        s.append("]");
        return s;
    }
};

// helper methods
[[maybe_unused]]
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
    case Expression::NULL_:
        break;
    default:
        LOG_WARN() << "Unhandled expression type " << expr.type;
    }
    return ld;
}

// serialize for debug
static std::string Serialize(const TUP_PROPERTIES &properties,
                             const size_t &cur_indent, const size_t &indent_size) {
    std::string properties_str;
    auto &map_literal = std::get<0>(properties);
    auto &parameter = std::get<1>(properties);
    if (!parameter.empty()) {
        properties_str.append(parameter);
    } else if (map_literal.type != Expression::NA) {
        properties_str.append(map_literal.ToString());
    }
    return fma_common::StringFormatter::Format(
        "{}PROPERTIES = \\{{}\\}",
        std::string(cur_indent, ' '),
        properties_str);
}

static std::string Serialize(const TUP_NODE_PATTERN &node_pattern,
                             const size_t &cur_indent, const size_t &indent_size) {
    size_t next_indent = cur_indent + indent_size;
    auto &variable = std::get<0>(node_pattern);
    auto &labels = std::get<1>(node_pattern);
    auto &properties = std::get<2>(node_pattern);
    return fma_common::StringFormatter::Format(
        "{}NODE_PATTERN = \\{\n"                \
        "{}VARIABLE = {},\n"                    \
        "{}LABEL = {},\n"                       \
        "{}\n"                                  \
        "{}\\}",
        std::string(cur_indent, ' '),
        std::string(next_indent, ' '), variable,
        std::string(next_indent, ' '), fma_common::ToString(labels),
        Serialize(properties, next_indent),
        std::string(cur_indent, ' '));
}

static std::string Serialize(const TUP_RELATIONSHIP_PATTERN &relationship_pattern,
                             const size_t &cur_indent, const size_t &indent_size) {
    size_t next_indent = cur_indent + indent_size;
    auto direction = std::get<0>(relationship_pattern);
    auto &detail = std::get<1>(relationship_pattern);
    auto &variable = std::get<0>(detail);
    auto &relationship_types = std::get<1>(detail);
    auto &range_literal = std::get<2>(detail);
    auto &properties = std::get<3>(detail);
    std::string edge_str;
    if (direction == LinkDirection::RIGHT_TO_LEFT) edge_str.append("<");
    edge_str.append("- {").append(variable).append(", ");
    if (!relationship_types.empty()) edge_str.append(relationship_types[0]);
    edge_str.append(", ");
    if (range_literal[0] >= 0) {
        edge_str.append("(")
            .append(std::to_string(range_literal[0]))
            .append(", ")
            .append(std::to_string(range_literal[1]))
            .append(")");
    }
    edge_str.append(", ").append(Serialize(properties)).append("} -");
    if (direction == LinkDirection::LEFT_TO_RIGHT) edge_str.append(">");
    return fma_common::StringFormatter::Format(
        "{}EDGE_PATTERN = \\{\n"            \
        "{}{}\n"                            \
        "{}\\}",
        std::string(cur_indent, ' '),
        std::string(next_indent, ' '), edge_str,
        std::string(cur_indent, ' '));
}

static std::string Serialize(const TUP_PATTERN_ELEMENT &pattern_element,
                             const size_t &cur_indent, const size_t &indent_size) {
    size_t next_indent = cur_indent + indent_size;
    auto &node_pattern = std::get<0>(pattern_element);
    auto &chains = std::get<1>(pattern_element);
    std::string chains_str;
    if (!chains.empty()) {
        chains_str.append(",");
    }
    for (size_t i = 0; i < chains.size(); i++) {
        const auto& chain = chains[i];
        auto &relp_patn = std::get<0>(chain);
        auto &node_patn = std::get<1>(chain);
        chains_str.append("\n")
            .append(Serialize(relp_patn, next_indent))
            .append(",\n")
            .append(Serialize(node_patn, next_indent));
        if (i < chains.size() - 1) {
            chains_str.append(",");
        }
    }
    return fma_common::StringFormatter::Format(
        "{}PATTERN_ELEMENT = \\{\n"             \
        "{}"                                    \
        "{}\n"                                  \
        "{}\\}",
        std::string(cur_indent, ' '),
        Serialize(node_pattern, next_indent),
        chains_str,
        std::string(cur_indent, ' '));
}

static std::string Serialize(const TUP_PATTERN_PART &pattern_part,
                             const size_t &cur_indent, const size_t &indent_size) {
    size_t next_indent = cur_indent + indent_size;
    auto &variable = std::get<0>(pattern_part);
    auto &element = std::get<1>(pattern_part);
    return fma_common::StringFormatter::Format(
        "{}PATTERN_PART = \\{\n"             \
        "{}VARIABLE = {}\n"                  \
        "{}\n"                               \
        "{}\\}",
        std::string(cur_indent, ' '),
        std::string(next_indent, ' '), variable,
        Serialize(element, next_indent),
        std::string(cur_indent, ' '));
}

static std::string Serialize(const VEC_PATTERN &pattern,
                             const size_t &cur_indent, const size_t &indent_size) {
    size_t next_indent = cur_indent + indent_size;
    std::string path_list_str;
    for (size_t i = 0; i < pattern.size(); i++) {
        const auto& part = pattern[i];
        path_list_str.append("\n")
            .append(Serialize(part, next_indent));
        if (i < pattern.size() - 1) {
            path_list_str.append(",");
        } else {
            path_list_str.append("\n").append(std::string(cur_indent, ' '));
        }
    }
    return fma_common::StringFormatter::Format(
        "{}PATH_PATTERN_LIST = [{}]",
        std::string(cur_indent, ' '),
        path_list_str);
}

static std::string Serialize(const TUP_RETURN &tup_return,
                             const size_t &cur_indent, const size_t &indent_size) {
    size_t next_indent = cur_indent + indent_size;
    bool distinct = std::get<0>(tup_return);
    auto &return_body = std::get<1>(tup_return);
    return fma_common::StringFormatter::Format(
        "{}RETURN = \\{\n"          \
        "{}"                        \
        "{}\n"                      \
        "{}\\}",
        std::string(cur_indent, ' '),
        distinct ? std::string(next_indent, ' ') + "DISTINCT = true,\n" : "",
        Serialize(return_body, next_indent),
        std::string(cur_indent, ' '));
}

static std::string Serialize(const TUP_RETURN_BODY &return_body,
                             const size_t &cur_indent, const size_t &indent_size) {
    size_t next_indent = cur_indent + indent_size;
    auto &items = std::get<0>(return_body);
    auto &sort_items = std::get<1>(return_body);
    auto &skip = std::get<2>(return_body);
    auto &limit = std::get<3>(return_body);
    std::string items_str;
    for (size_t i = 0; i < items.size(); i++) {
        const auto& item = items[i];
        items_str.append("\n")
            .append(Serialize_1(item, next_indent+indent_size));
        if (i < items.size() - 1) {
            items_str.append(",");
        } else {
            items_str.append("\n")
                .append(std::string(next_indent, ' '));
        }
    }
    std::string sort_str;
    for (size_t i = 0; i < sort_items.size(); i++) {
        const auto& sort_item = sort_items[i];
        sort_str.append(std::to_string(sort_item.first)).append(sort_item.second ? "ASC" : "DESC");
        if (i < sort_items.size() - 1) { sort_str.append(","); }
    }
    return fma_common::StringFormatter::Format(
        "{}RETURN_BODY = \\{\n"                \
        "{}ITEMS = [{}],\n"                    \
        "{}ORDER_BY = \\{{}\\}"                \
        "{}"                                   \
        "{}"                                   \
        "\n{}\\}",
        std::string(cur_indent, ' '),
        std::string(next_indent, ' '), items_str,
        std::string(next_indent, ' '), sort_str,
        skip.type != Expression::NA ? fma_common::StringFormatter::Format(
            ",\n{}SKIP = {}",
            std::string(next_indent, ' '),
            skip.ToString()) : "",
        limit.type != Expression::NA ? fma_common::StringFormatter::Format(
            ",\n{}LIMIT = {}",
            std::string(next_indent, ' '),
            limit.ToString()) : "",
        std::string(cur_indent, ' '));
}

static std::string Serialize_1(const TUP_RETURN_ITEM &return_item,
                               const size_t &cur_indent, const size_t &indent_size) {
    auto &expr = std::get<0>(return_item);
    auto &variable = std::get<1>(return_item);
    return fma_common::StringFormatter::Format(
        "{}RETURN_ITEM = \\{{}, {}\\}",
        std::string(cur_indent, ' '),
        expr.ToString(), variable);
}

static std::string Serialize(const TUP_CALL &call,
                             const size_t &cur_indent, const size_t &indent_size) {
    std::string input_str;
    std::string output_str;
    auto &parameters = std::get<1>(call);
    auto &yield_items = std::get<2>(call);
    size_t i = 0;
    for (auto &p : parameters) {
        input_str.append(p.ToString());
        if (++i < parameters.size()) input_str.append(", ");
    }
    i = 0;
    for (auto &y : yield_items) {
        output_str.append(y.first);
        if (++i < yield_items.size()) output_str.append(", ");
    }
    return fma_common::StringFormatter::Format(
        "{}CALL = \\{{}({})::({})\\}",
        std::string(cur_indent, ' '),
        std::get<0>(call), input_str, output_str);
}

static std::string Serialize(const TUP_SET_ITEM &set_item,
                             const size_t &cur_indent, const size_t &indent_size) {
    size_t next_indent = cur_indent + indent_size;
    auto &variable = std::get<0>(set_item);
    auto &property_expr = std::get<1>(set_item);
    auto &symbol = std::get<2>(set_item);
    auto &properties = std::get<3>(set_item);
    auto &labels = std::get<4>(set_item);
    return fma_common::StringFormatter::Format(
        "{}SET_ITEM = \\{\n"                    \
        "{}VARIABLE = {},\n"                    \
        "{}PROPERTY_EXPR = {},\n"               \
        "{}SYMBOL = {},\n"                      \
        "{}PROPERTIES = {},\n"                  \
        "{}LABEL = {}\n"                        \
        "{}\\}",
        std::string(cur_indent, ' '),
        std::string(next_indent, ' '), variable,
        std::string(next_indent, ' '), property_expr.ToString(),
        std::string(next_indent, ' '), symbol,
        std::string(next_indent, ' '), properties.ToString(),
        std::string(next_indent, ' '), labels,
        std::string(cur_indent, ' '));
}

static std::string Serialize(const VEC_SET &set,
                             const size_t &cur_indent, const size_t &indent_size) {
    size_t next_indent = cur_indent + indent_size;
    std::string set_items;
    for (size_t i = 0; i < set.size(); i++) {
        const auto& item = set[i];
        set_items.append("\n")
            .append(Serialize(item, next_indent));
        if (i < set.size() - 1) {
            set_items.append(",");
        } else {
            set_items.append("\n").append(std::string(cur_indent, ' '));
        }
    }
    return fma_common::StringFormatter::Format(
        "{}SET_ITEMS = \\{{}\\}",
        std::string(cur_indent, ' '), set_items);
}

}  // namespace parser
