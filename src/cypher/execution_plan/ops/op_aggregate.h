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
// Created by wt on 19-2-13.
//
#pragma once

#include "cypher/grouping/group.h"
#include "cypher/execution_plan/ops/op.h"
#include "cypher/arithmetic/arithmetic_expression.h"

namespace cypher {

class Aggregate : public OpBase {
    friend class PassReduceCount;

    const SymbolTable &sym_tab_;
    std::unordered_map<std::string, Group> group_cache_;
    std::unordered_map<std::string, Group>::iterator group_iter_;
    /* return terms which are not aggregated. */
    std::vector<ArithExprNode> noneaggregated_expressions_;
    std::vector<std::string> noneaggr_item_names_;
    /* array of values composing an aggregated group. */
    std::vector<Entry> group_keys_;
    std::vector<ArithExprNode> aggregated_expressions_;
    std::vector<std::string> aggr_item_names_;
    std::vector<std::string> item_names_;
    /* result_set_headers in parts (return or with clause) are different, so we need
     * to copy them. */
    ResultSetHeader result_set_header_;

    enum {
        Initialized,
        RefreshAfterPass,
        Resetted,
        Consuming,
    } state_;

    std::string _ComputeGroupKey(RTContext *ctx, const Record &r);

    void _AggregateRecord(RTContext *ctx, const Record &r);

    /* Returns a record populated with group data. */
    OpResult HandOff(RTContext *ctx);

 public:
    Aggregate(const std::vector<ArithExprNode> &aggregated_expressions,
              const std::vector<std::string> &aggr_item_names,
              const std::vector<ArithExprNode> &noneaggregated_expressions,
              const std::vector<std::string> &noneaggr_item_names,
              std::vector<std::string> &item_names, const SymbolTable *sym_tab,
              const ResultSetHeader &header);

    OpResult Initialize(RTContext *ctx) override;

    OpResult RealConsume(RTContext *ctx) override;

    ResultSetHeader GetResultSetHeader();

    std::vector<ArithExprNode> GetNoneAggregatedExpressions();

    std::vector<ArithExprNode> GetAggregatedExpressions();

    /* Restart */
    OpResult ResetImpl(bool complete) override;

    std::string ToString() const override;

    const std::vector<ArithExprNode>& NoneAggregatedExpressions() const;

    const std::vector<ArithExprNode>& AggregatedExpressions() const;

    const std::vector<std::string>& NoneAggrItemNames() const;

    const std::vector<std::string>& AggrItemNames() const;

    const std::vector<std::string>& ItemNames() const;

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};

}  // namespace cypher
