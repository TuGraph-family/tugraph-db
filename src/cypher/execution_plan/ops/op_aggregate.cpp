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

#include "cypher/execution_plan/ops/op_aggregate.h"

namespace cypher {

void Aggregate::_BuildArithmeticExpressions(const parser::QueryPart *stmt) {
    const auto &return_body =
        stmt->return_clause ? std::get<1>(*stmt->return_clause) : std::get<1>(*stmt->with_clause);
    const auto &return_items = std::get<0>(return_body);
    for (auto &item : return_items) {
        auto &expr = std::get<0>(item);
        auto &var = std::get<1>(item);
        ArithExprNode ae;
        ae.Set(expr, sym_tab_);
        if (ae.ContainsAggregation()) {
            aggregated_parser_expressions_.emplace_back(expr);
            aggregated_expressions_.emplace_back(ae);
            if (!var.empty()) modifies.emplace_back(var);
            aggr_item_names_.emplace_back(var.empty() ? expr.ToString(false) : var);
        } else {
            noneaggregated_expressions_.emplace_back(ae);
            noneaggr_item_names_.emplace_back(var.empty() ? expr.ToString(false) : var);
        }
        item_names_.emplace_back(var.empty() ? expr.ToString(false) : var);
    }
    group_keys_.resize(noneaggregated_expressions_.size());
}

std::string Aggregate::_ComputeGroupKey(RTContext *ctx, const Record &r) {
    CYPHER_THROW_ASSERT(noneaggregated_expressions_.size() == group_keys_.size());
    std::string group_key;
    int i = 0;
    for (auto &e : noneaggregated_expressions_) {
        group_keys_[i] = e.Evaluate(ctx, r);
        /* When the key is NODE/RELATIONSHIP, the ITERATOR will be invalid
         * after the aggregation, so we should take a snapshot of it.  */
        group_keys_[i].Snapshot();
        group_key.append(group_keys_[i].ToString()).append(",");
        i++;
    }
    /* Discard last delimiter. */
    if (!group_key.empty()) group_key.pop_back();
    return group_key;
}

void Aggregate::_AggregateRecord(RTContext *ctx, const Record &r) {
    Group *group = nullptr;
    auto group_key = _ComputeGroupKey(ctx, r);
    auto it = group_cache_.find(group_key);
    if (it == group_cache_.end()) {
        Group new_group;
        new_group.keys = group_keys_;
        /* Use CONSTRUCT instead of COPY to create new AggCtx for a new
         * group. */
        bool ast_expr = aggregated_parser_expressions_.empty();
        if (ast_expr) {
            for (auto &ae : aggregated_expressions_) {
                ArithExprNode new_ae(ae.expr_, sym_tab_);
                new_group.aggregation_functions.emplace_back(new_ae);
            }
        } else {
            for (auto &ae : aggregated_parser_expressions_) {
                ArithExprNode new_ae(ae, sym_tab_);
                new_group.aggregation_functions.emplace_back(new_ae);
            }
        }
        auto ret = group_cache_.emplace(group_key, new_group);
        CYPHER_THROW_ASSERT(ret.second);
        group = &ret.first->second;
    } else {
        group = &it->second;
    }
    for (auto &agg : group->aggregation_functions) {
        agg.Aggregate(ctx, r);
    }
}

/* Returns a record populated with group data. */
OpBase::OpResult Aggregate::HandOff(RTContext *ctx) {
    if (state_ != Consuming) return OP_DEPLETED;
    if (group_iter_ == group_cache_.end()) return OP_DEPLETED;
    int key_idx = 0, agg_idx = 0;
    /* Add group elements according to specified return order. */
    for (auto &col : result_set_header_.colums) {
        if (col.aggregated) {
            auto &agg_exp = group_iter_->second.aggregation_functions[agg_idx];
            agg_exp.Reduce();
            /* Generally AR_OP_AGGREGATE do not need record in evaluate,
             * only when in a compound expression like `collect(a) + x`.  */
            if (agg_exp.type == ArithExprNode::AR_EXP_OPERAND) CYPHER_TODO();
            if (agg_exp.type == ArithExprNode::AR_EXP_OP &&
                agg_exp.op.type != ArithOpNode::AR_OP_AGGREGATE) {
                // throw todo exception when `collect(a) + a.amt`
                if (agg_exp.op.type != ArithOpNode::AR_OP_MATH || agg_exp.op.children.size() != 3) {
                    CYPHER_TODO();
                }
                int opd = agg_exp.op.children[0].op.type == ArithOpNode::AR_OP_AGGREGATE   ? 1
                          : agg_exp.op.children[1].op.type == ArithOpNode::AR_OP_AGGREGATE ? 0
                                                                                           : -1;
                if (opd < 0) CYPHER_TODO();
                if (agg_exp.op.children[opd].operand.type ==
                    ArithOperandNode::AR_OPERAND_VARIADIC) {
                    auto it =
                        sym_tab_.symbols.find(agg_exp.op.children[opd].operand.variadic.alias);
                    if (it != sym_tab_.symbols.end() &&
                        it->second.type != SymbolNode::Type::CONSTANT) {
                        CYPHER_TODO();
                    }
                }
            }
            if (children.empty() || !children[0]->record) CYPHER_INTL_ERR();
            record->values[key_idx + agg_idx] = agg_exp.Evaluate(ctx, *children[0]->record);
            agg_idx++;
        } else {
            record->values[key_idx + agg_idx] = group_iter_->second.keys[key_idx];
            key_idx++;
        }
    }
    group_iter_++;
    return OP_OK;
}

Aggregate::Aggregate(const parser::QueryPart *stmt, const SymbolTable *sym_tab,
                     const ResultSetHeader &header)
    : OpBase(OpType::AGGREGATE, "Aggregate"), sym_tab_(*sym_tab), result_set_header_(header) {
    _BuildArithmeticExpressions(stmt);
}

Aggregate::Aggregate(const std::vector<ArithExprNode> &aggregated_expressions,
                     const std::vector<std::string> &aggr_item_names,
                     const std::vector<ArithExprNode> &noneaggregated_expressions,
                     const std::vector<std::string> &noneaggr_item_names,
                     std::vector<std::string> &item_names, const SymbolTable *sym_tab,
                     const ResultSetHeader &header)
    : OpBase(OpType::AGGREGATE, "Aggregate"), sym_tab_(*sym_tab), result_set_header_(header) {
    aggregated_expressions_ = aggregated_expressions;
    aggr_item_names_ = aggr_item_names;
    noneaggregated_expressions_ = noneaggregated_expressions;
    noneaggr_item_names_ = noneaggr_item_names;
    item_names_ = item_names;
    group_keys_.resize(noneaggregated_expressions_.size());
}

OpBase::OpResult Aggregate::Initialize(RTContext *ctx) {
    CYPHER_THROW_ASSERT(!children.empty());
    auto &child = children[0];
    auto res = child->Initialize(ctx);
    if (res != OP_OK) return res;
    record = std::make_shared<Record>(item_names_.size());
    group_cache_.clear();
    state_ = Initialized;
    return OP_OK;
}

OpBase::OpResult Aggregate::RealConsume(RTContext *ctx) {
    if (state_ == Initialized) {
        auto child = children[0];
        while (child->Consume(ctx) == OP_OK) {
            _AggregateRecord(ctx, *child->record);
        }
        group_iter_ = group_cache_.begin();
        state_ = Consuming;
    }
    return HandOff(ctx);
}

ResultSetHeader Aggregate::GetResultSetHeader() { return result_set_header_; }

std::vector<ArithExprNode> Aggregate::GetNoneAggregatedExpressions() {
    return noneaggregated_expressions_;
}

std::vector<ArithExprNode> Aggregate::GetAggregatedExpressions() {
    std::vector<ArithExprNode> agg_exprs;
    for (auto &ae : aggregated_expressions_) {
        agg_exprs.emplace_back(ae);
    }
    return agg_exprs;
}

/* Restart */
OpBase::OpResult Aggregate::ResetImpl(bool complete) {
    if (complete) {
        record = nullptr;
        state_ = Initialized;
    }
    return OP_OK;
}

std::string Aggregate::ToString() const {
    std::string str(name);
    str.append(" [");
    for (auto &i : item_names_) {
        str.append(i).append(",");
    }
    if (!item_names_.empty()) str.pop_back();
    str.append("]");
    return str;
}

const std::vector<ArithExprNode> &Aggregate::NoneAggregatedExpressions() const {
    return noneaggregated_expressions_;
}

const std::vector<ArithExprNode> &Aggregate::AggregatedExpressions() const {
    return aggregated_expressions_;
}

const std::vector<std::string> &Aggregate::NoneAggrItemNames() const {
    return noneaggr_item_names_;
}

const std::vector<std::string> &Aggregate::AggrItemNames() const { return aggr_item_names_; }

const std::vector<std::string> &Aggregate::ItemNames() const { return item_names_; }

}  // namespace cypher
