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
#include "cypher/parser/cypher_base_visitor.h"

namespace parser {
using namespace cypher;
using ArithExprType = ArithExprNode::ArithExprType;
using ArithOperandType = ArithOperandNode::ArithOperandType;
void RewriteFilterAst(std::shared_ptr<lgraph::Filter>& filter,
                      std::vector<TUP_PATTERN_PART>& pattern,
                      const lgraph::SchemaInfo& si) {
    if (filter->Type() == lgraph::Filter::Type::RANGE_FILTER) {
        bool rewrite = false;
        auto& range_filter = dynamic_cast<lgraph::RangeFilter&>(*filter);
        if (range_filter.GetCompareOp() != lgraph::CompareOp::LBR_EQ) {
            return;
        }
        auto& rf_left = range_filter.GetAeLeft();
        auto& rf_right = range_filter.GetAeRight();
        if (!(rf_left.type == ArithExprType::AR_EXP_OPERAND &&
            rf_left.operand.type == ArithOperandType::AR_OPERAND_VARIADIC &&
            rf_right.type == ArithExprType::AR_EXP_OPERAND &&
              rf_right.operand.type == ArithOperandType::AR_OPERAND_CONSTANT)) {
            return;
        }
        for (auto &pattern_part : pattern) {
            auto &pattern_element = std::get<1>(pattern_part);
            std::vector<TUP_NODE_PATTERN*> node_patterns;
            node_patterns.push_back(&std::get<0>(pattern_element));
            for (auto& item : std::get<1>(pattern_element)) {
                node_patterns.push_back(&std::get<1>(item));
            }
            for (auto p : node_patterns) {
                auto& node_pattern = *p;
                const std::string &node_variable = std::get<0>(node_pattern);
                if (node_variable != rf_left.operand.variadic.alias) {
                    continue;
                }
                auto &node_labels = std::get<1>(node_pattern);
                if (!node_labels.empty()) {
                    auto label = node_labels[0];
                    auto &properties = std::get<2>(node_pattern);
                    auto &expr_prop = std::get<0>(properties);
                    if (expr_prop.type == parser::Expression::NA ||
                        (expr_prop.type == parser::Expression::MAP &&
                         expr_prop.Map().empty())) {
                        auto s = si.v_schema_manager.GetSchema(label);
                        if (!s)
                            THROW_CODE(CypherException, "No such vertex label: {}", label);
                        if (s->GetPrimaryField() == rf_left.operand.variadic.entity_prop) {
                            Expression::EXPR_TYPE_MAP map_exp;
                            map_exp[rf_left.operand.variadic.entity_prop] = rf_right.expression_;
                            // rewrite ast
                            rewrite = true;
                            expr_prop.type = Expression::MAP;
                            expr_prop.data = std::make_shared<Expression::EXPR_TYPE_MAP>(
                                std::move(map_exp));
                        }
                    }
                }
            }
        }
        if (rewrite) {
            filter.reset();
        }
        return;
    }
    if (filter->Type() == lgraph::Filter::Type::BINARY) {
        if (filter->LogicalOp() == lgraph::LogicalOp::LBR_AND) {
            RewriteFilterAst(filter->Left(), pattern, si);
            RewriteFilterAst(filter->Right(), pattern, si);
            if (!filter->Left() && filter->Right()) {
                filter = filter->Right();
            } else if (filter->Left() && !filter->Right()) {
                filter = filter->Left();
            } else if (!filter->Left() && !filter->Right()) {
                filter.reset();
            }
        }
    }
}

/**
 * before:
 *     MATCH (n:person),(m:movie) where n.id = 1 and m.id = 1 ...
 * after:
 *     MATCH (n:person {id:1}),(m:movie {id:1}) ...
 */

/**
 * before:
 *     MATCH (n:person)-[r]-(m:movie)-[r1]-(m1:user) where n.id = 10 and m.id = 1 and m1.id > 10 ...
 * after:
 *     MATCH (n:person {id:10})-[r]-(m:movie {id:1})-[r1]-(m1:user) where m1.id > 10 ...
 */

void PushDownFilterAst(std::vector<SglQuery>& sql_query, const lgraph::SchemaInfo& si) {
    for (auto& query : sql_query) {
        for (auto& part : query.parts) {
            if (part.match_clause) {
                auto match_clause = const_cast<Clause::TYPE_MATCH*>(part.match_clause);
                auto &pattern = std::get<0>(*match_clause);
                auto &where = std::get<2>(*match_clause);
                if (where.type == Expression::FILTER) {
                    auto filter = where.Filter();
                    RewriteFilterAst(filter, pattern, si);
                    if (!filter) {
                        Expression empty;
                        where = std::move(empty);
                    } else {
                        where.data = filter;
                    }
                }
            }
        }
    }
}

void CypherBaseVisitor::AstRewrite(cypher::RTContext *ctx) {
    if (ctx->graph_.empty()) {
        return;
    }
    auto graph = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    auto txn = graph.CreateReadTxn();
    const auto &si = txn.GetSchemaInfo();
    PushDownFilterAst(_query, si);
}

}  // namespace parser

