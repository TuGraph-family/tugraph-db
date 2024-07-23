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
// Created by wt on 18-10-31.
//
#pragma once

#include <string>
#include <vector>
#include <map>
#include "core/lightning_graph.h"
#include "cypher/cypher_types.h"

namespace lgraph {
class StateMachine;
}

namespace parser {
// class declarations
struct Expression;

enum LinkDirection {
    LEFT_TO_RIGHT,
    RIGHT_TO_LEFT,
    DIR_NOT_SPECIFIED,
    UNKNOWN,
};

enum CmdType {
    QUERY,
    EXPLAIN,
    PROFILE,
};

static const char *const ANONYMOUS = "@ANON_";
static const char *const INVISIBLE = "@INVIS_";
static const int VAR_LEN_EXPAND_MAX_HOP = 128;

// typedefs, TODO(anyone) simplify
typedef std::vector<std::string> VEC_STR;
typedef std::map<std::string, std::string> MAP_STR;
// property_key_name, expression
typedef std::map<std::string, Expression> MAP_MAP_LITERAL;
// map_literal | parameter
typedef std::tuple<Expression, std::string> TUP_PROPERTIES;
// variable, node_labels, properties
typedef std::tuple<std::string, VEC_STR, TUP_PROPERTIES> TUP_NODE_PATTERN;
// variable, relationship_types, range_literal, properties
typedef std::tuple<std::string, VEC_STR, std::array<int, 2>, TUP_PROPERTIES>
    TUP_RELATIONSHIP_DETAIL;
// link direction, relationship_detail
typedef std::tuple<LinkDirection, TUP_RELATIONSHIP_DETAIL> TUP_RELATIONSHIP_PATTERN;
// relationship_pattern, node_pattern
typedef std::tuple<TUP_RELATIONSHIP_PATTERN, TUP_NODE_PATTERN> TUP_PATTERN_ELEMENT_CHAIN;
// node_pattern, pattern_element_chain
typedef std::tuple<TUP_NODE_PATTERN, std::vector<TUP_PATTERN_ELEMENT_CHAIN>> TUP_PATTERN_ELEMENT;
// variable, [anonymous_]pattern_element
typedef std::tuple<std::string, TUP_PATTERN_ELEMENT> TUP_PATTERN_PART;
// pattern_parts
typedef std::vector<TUP_PATTERN_PART> VEC_PATTERN;
// expression, as_variable
typedef std::tuple<Expression, std::string, bool> TUP_RETURN_ITEM;
// return_items, sort_items(return_item_idx, asc), skip, limit
typedef std::tuple<std::vector<TUP_RETURN_ITEM>, std::vector<std::pair<int, bool>>, Expression,
                   Expression>
    TUP_RETURN_BODY;
// distinct, return_body
typedef std::tuple<bool, TUP_RETURN_BODY> TUP_RETURN;
// expression, as_variable
typedef TUP_RETURN_ITEM TUP_UNWIND;
// procedure_name, parameters, yield items
typedef std::tuple<
    std::string,
    std::vector<Expression>,
        // name, type(only used in signatured plugins)
    std::vector<std::pair<std::string, lgraph_api::LGraphType>>,
    Expression
    > TUP_CALL;
// variable, property_expression, sign(=/+=), expression, node_labels
typedef std::tuple<std::string, Expression, std::string, Expression, VEC_STR> TUP_SET_ITEM;
// vector of set items
typedef std::vector<TUP_SET_ITEM> VEC_SET;
// vector of delete
typedef std::vector<Expression> VEC_DEL;
// RemoveItem = (Variable, NodeLabels) | PropertyExpression
typedef std::vector<Expression> VEC_REMOVE;

}  // namespace parser

namespace cypher {
typedef std::unordered_map<std::string, cypher::FieldData> PARAM_TAB;
static const char *const CUSTOM_FUNCTION_PREFIX = "custom.";
static const int DELETE_BUFFER_SIZE = 1 << 26;
}  // namespace cypher
