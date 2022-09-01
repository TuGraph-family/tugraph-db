/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

//
// Created by wt on 19-2-15.
//
#pragma once

#include "resultset/record.h"
#include "arithmetic/arithmetic_expression.h"

namespace cypher {
struct Group {
    std::vector<Entry> keys;
    std::vector<ArithExprNode> aggregation_functions;
};
}  // namespace cypher
