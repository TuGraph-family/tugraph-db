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
// Created by ljr on 22-7-30.
//
#pragma once

#include "cypher/execution_plan/ops/op.h"
#include "filter/filter.h"
#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/expr/BAnd.h"
#include "geax-front-end/ast/expr/BEqual.h"
#include "geax-front-end/ast/expr/BGreaterThan.h"
#include "geax-front-end/ast/expr/BNotEqual.h"
#include "geax-front-end/ast/expr/BNotGreaterThan.h"
#include "geax-front-end/ast/expr/BNotSmallerThan.h"
#include "geax-front-end/ast/expr/BOr.h"
#include "geax-front-end/ast/expr/BSmallerThan.h"
#include "geax-front-end/ast/expr/BXor.h"
#include "geax-front-end/ast/expr/Not.h"
#include "geax-front-end/ast/expr/VBool.h"
#include "geax-front-end/ast/expr/VDouble.h"
#include "geax-front-end/ast/expr/VInt.h"
#include "geax-front-end/ast/expr/VString.h"
#include "lgraph/lgraph.h"
#include "lgraph/lgraph_traversal.h"
#include "libcuckoo/cuckoohash_map.hh"

namespace cypher {

/* TraversalOp
 * Traversal entire graph Parallel
 * basic plan : Node Scan - Argument
 * more: Node Scan - Filter - Expand - Expand - Argument
 */

class OpGqlTraversal : public OpBase {
    /** about node scan */
    std::string start_label_;
    std::string start_alias_;
    geax::frontend::Expr *filter_;

    /** about aggregate */
    ResultSetHeader result_set_header_;
    ArithExprNode agg_expr_;
    ArithExprNode noneagg_expr_;
    std::string noneagg_property_;

    /** about expand */
    std::vector<
        std::tuple<cypher::Node *, cypher::Node *, cypher::Relationship *, ExpandTowards, bool>>
        expands_;  // <start, neighbor, relp, towards, has_filter>

    /** result */
    cuckoohash_map<lgraph::FieldData, int, lgraph::FieldData::Hash> map_;
    std::vector<lgraph::FieldData> result_buffer_;
    size_t result_idx_;

    enum {
        Initialized,
        Consuming,
    } state_;

    bool FieldValue(geax::frontend::Expr *expr, lgraph::FieldData &data) {
        if (expr->type() == geax::frontend::AstNodeType::kVBool) {
            auto e = (geax::frontend::VBool *)expr;
            data = lgraph::FieldData(e->val());
            return true;
        } else if (expr->type() == geax::frontend::AstNodeType::kVInt) {
            auto e = (geax::frontend::VInt *)expr;
            data = lgraph::FieldData(e->val());
            return true;
        } else if (expr->type() == geax::frontend::AstNodeType::kVDouble) {
            auto e = (geax::frontend::VDouble *)expr;
            data = lgraph::FieldData(e->val());
            return true;
        } else if (expr->type() == geax::frontend::AstNodeType::kVString) {
            auto e = (geax::frontend::VString *)expr;
            data = lgraph::FieldData(e->val());
            return true;
        }
        return false;
    }

    bool DoFilter(lgraph_api::VertexIterator &vit, geax::frontend::Expr *expr) {
        if (expr == nullptr) {
            return true;
        }
        switch (expr->type()) {
        case geax::frontend::AstNodeType::kBEqual:
            {
                geax::frontend::BEqual *op = (geax::frontend::BEqual *)expr;
                auto left = op->left();
                if (left->type() != geax::frontend::AstNodeType::kGetField) CYPHER_TODO();
                std::string prop_name = ((geax::frontend::GetField *)left)->fieldName();
                if (prop_name.empty()) {
                    CYPHER_TODO();
                }
                lgraph::FieldData prop_value;
                if (!FieldValue(op->right(), prop_value)) CYPHER_TODO();
                return vit[prop_name] == prop_value;
            }
        case geax::frontend::AstNodeType::kBNotEqual:
            {
                geax::frontend::BNotEqual *op = (geax::frontend::BNotEqual *)expr;
                auto left = op->left();
                if (left->type() != geax::frontend::AstNodeType::kGetField) CYPHER_TODO();
                std::string prop_name = ((geax::frontend::GetField *)left)->fieldName();
                if (prop_name.empty()) {
                    CYPHER_TODO();
                }
                lgraph::FieldData prop_value;
                if (!FieldValue(op->right(), prop_value)) CYPHER_TODO();
                return vit[prop_name] != prop_value;
            }
        case geax::frontend::AstNodeType::kBGreaterThan:
            {
                geax::frontend::BGreaterThan *op = (geax::frontend::BGreaterThan *)expr;
                auto left = op->left();
                if (left->type() != geax::frontend::AstNodeType::kGetField) CYPHER_TODO();
                std::string prop_name = ((geax::frontend::GetField *)left)->fieldName();
                if (prop_name.empty()) {
                    CYPHER_TODO();
                }
                lgraph::FieldData prop_value;
                if (!FieldValue(op->right(), prop_value)) CYPHER_TODO();
                return vit[prop_name] > prop_value;
            }
        case geax::frontend::AstNodeType::kBSmallerThan:
            {
                geax::frontend::BSmallerThan *op = (geax::frontend::BSmallerThan *)expr;
                auto left = op->left();
                if (left->type() != geax::frontend::AstNodeType::kGetField) CYPHER_TODO();
                std::string prop_name = ((geax::frontend::GetField *)left)->fieldName();
                if (prop_name.empty()) {
                    CYPHER_TODO();
                }
                lgraph::FieldData prop_value;
                if (!FieldValue(op->right(), prop_value)) CYPHER_TODO();
                return vit[prop_name] < prop_value;
            }
        case geax::frontend::AstNodeType::kBNotGreaterThan:
            {
                geax::frontend::BNotGreaterThan *op = (geax::frontend::BNotGreaterThan *)expr;
                auto left = op->left();
                if (left->type() != geax::frontend::AstNodeType::kGetField) CYPHER_TODO();
                std::string prop_name = ((geax::frontend::GetField *)left)->fieldName();
                if (prop_name.empty()) {
                    CYPHER_TODO();
                }
                lgraph::FieldData prop_value;
                if (!FieldValue(op->right(), prop_value)) CYPHER_TODO();
                return vit[prop_name] <= prop_value;
            }
        case geax::frontend::AstNodeType::kBNotSmallerThan:
            {
                geax::frontend::BNotSmallerThan *op = (geax::frontend::BNotSmallerThan *)expr;
                auto left = op->left();
                if (left->type() != geax::frontend::AstNodeType::kGetField) CYPHER_TODO();
                std::string prop_name = ((geax::frontend::GetField *)left)->fieldName();
                if (prop_name.empty()) {
                    CYPHER_TODO();
                }
                lgraph::FieldData prop_value;
                if (!FieldValue(op->right(), prop_value)) CYPHER_TODO();
                return vit[prop_name] >= prop_value;
            }
        case geax::frontend::AstNodeType::kNot:
            {
                geax::frontend::Not *op = (geax::frontend::Not *)expr;
                return op->expr() && !DoFilter(vit, op->expr());
            }
        case geax::frontend::AstNodeType::kBAnd:
            {
                geax::frontend::BAnd *op = (geax::frontend::BAnd *)expr;
                return op->left() && op->right() &&
                       (DoFilter(vit, op->left()) && DoFilter(vit, op->right()));
            }
        case geax::frontend::AstNodeType::kBOr:
            {
                geax::frontend::BOr *op = (geax::frontend::BOr *)expr;
                return op->left() && op->right() &&
                       (DoFilter(vit, op->left()) || DoFilter(vit, op->right()));
            }
        case geax::frontend::AstNodeType::kBXor:
            {
                geax::frontend::BXor *op = (geax::frontend::BXor *)expr;
                return op->left() && op->right() &&
                       (DoFilter(vit, op->left()) && DoFilter(vit, op->right()));
            }
        default:
            return false;
        }
    }

    bool ParallelTraversal(RTContext *ctx) {
        lgraph_api::GraphDB db_(ctx->ac_db_.get(), true, false);
        ctx->txn_->Abort();
        auto txn = db_.CreateReadTxn();
        // 1. FindVertices
        auto vs = lgraph_api::traversal::FindVertices(
            db_, txn,
            [&](lgraph_api::VertexIterator &vit) {
                if (vit.GetLabel() == start_label_ && DoFilter(vit, filter_)) {
                    if (expands_.empty()) {
                        auto group_by_key = noneagg_property_.empty() ? lgraph::FieldData()
                                                                      : vit[noneagg_property_];
                        map_.upsert(group_by_key, [](int &n) { ++n; }, 1);
                    }
                    return true;
                }
                return false;
            },
            true);
        // 2. ExpandEdges
        auto ft = lgraph_api::traversal::FrontierTraversal(
            db_, txn,
            lgraph_api::traversal::TRAVERSAL_PARALLEL |
                lgraph_api::traversal::TRAVERSAL_ALLOW_REVISITS);
        ft.SetFrontier(vs);
        for (size_t i = 0; i < expands_.size(); i++) {
            switch (std::get<3>(expands_[i])) {
            case ExpandTowards::FORWARD:  // out
                ft.ExpandOutEdges(
                    [&](lgraph_api::OutEdgeIterator &eit) {
                        return std::get<2>(expands_[i])->Types().find(eit.GetLabel()) !=
                               std::get<2>(expands_[i])->Types().end();
                    },
                    [&](lgraph_api::VertexIterator &vit) {
                        if (vit.GetLabel() == std::get<1>(expands_[i])->Label()) {
                            if (i == expands_.size() - 1) {
                                auto group_by_key = noneagg_property_.empty()
                                                        ? lgraph::FieldData()
                                                        : vit[noneagg_property_];
                                map_.upsert(group_by_key, [](int &n) { ++n; }, 1);
                            }
                            return true;
                        }
                        return false;
                    });
                break;
            case ExpandTowards::REVERSED:  // in
                ft.ExpandInEdges(
                    [&](lgraph_api::InEdgeIterator &eit) {
                        return std::get<2>(expands_[i])->Types().find(eit.GetLabel()) !=
                               std::get<2>(expands_[i])->Types().end();
                    },
                    [&](lgraph_api::VertexIterator &vit) {
                        if (vit.GetLabel() == std::get<1>(expands_[i])->Label()) {
                            if (i == expands_.size() - 1) {
                                auto group_by_key = noneagg_property_.empty()
                                                        ? lgraph::FieldData()
                                                        : vit[noneagg_property_];
                                map_.upsert(group_by_key, [](int &n) { ++n; }, 1);
                            }
                            return true;
                        }
                        return false;
                    });
                break;
            case ExpandTowards::BIDIRECTIONAL:  //  bi
                ft.ExpandEdges(
                    [&](lgraph_api::OutEdgeIterator &eit) {
                        return std::get<2>(expands_[i])->Types().find(eit.GetLabel()) !=
                               std::get<2>(expands_[i])->Types().end();
                    },
                    [&](lgraph_api::InEdgeIterator &eit) {
                        return std::get<2>(expands_[i])->Types().find(eit.GetLabel()) !=
                               std::get<2>(expands_[i])->Types().end();
                    },
                    [&](lgraph_api::VertexIterator &vit) {
                        if (vit.GetLabel() == std::get<1>(expands_[i])->Label()) {
                            if (i == expands_.size() - 1) {
                                auto group_by_key = noneagg_property_.empty()
                                                        ? lgraph::FieldData()
                                                        : vit[noneagg_property_];
                                map_.upsert(group_by_key, [](int &n) { ++n; }, 1);
                            }
                            return true;
                        }
                        return false;
                    },
                    [&](lgraph_api::VertexIterator &vit) {
                        if (vit.GetLabel() == std::get<1>(expands_[i])->Label()) {
                            if (i == expands_.size() - 1) {
                                auto group_by_key = noneagg_property_.empty()
                                                        ? lgraph::FieldData()
                                                        : vit[noneagg_property_];
                                map_.upsert(group_by_key, [](int &n) { ++n; }, 1);
                            }
                            return true;
                        }
                        return false;
                    });
                break;
            }
        }
        for (auto &num : map_.lock_table()) {
            if (noneagg_property_.empty()) {
                result_buffer_.emplace_back(lgraph::FieldData(num.second));
            } else {  // result_buffer_ format：[agg, key, agg, key, agg, key, ...]
                result_buffer_.emplace_back(lgraph::FieldData(num.second));
                result_buffer_.emplace_back(num.first);
            }
        }
        return true;
    }

    OpResult HandOff() {
        if (state_ != Consuming) return OP_DEPLETED;
        if (result_idx_ == result_buffer_.size()) return OP_DEPLETED;
        int key_idx = 0, agg_idx = 0;
        for (auto &col : result_set_header_.colums) {
            if (col.aggregated) {
                record->values[key_idx + agg_idx] =
                    Entry(cypher::FieldData(result_buffer_.at(result_idx_)));
                agg_idx++;
            } else {
                record->values[key_idx + agg_idx] =
                    Entry(cypher::FieldData(result_buffer_.at(result_idx_ + 1)));
                key_idx++;
            }
        }
        result_idx_ += result_set_header_.colums.size();
        return OP_OK;
    }

 public:
    OpGqlTraversal(
        std::string label, std::string alias, ResultSetHeader result_set_header,
        ArithExprNode agg_expr, ArithExprNode noneagg_expr,
        std::vector<
            std::tuple<cypher::Node *, cypher::Node *, cypher::Relationship *, ExpandTowards, bool>>
            expands = std::vector<std::tuple<cypher::Node *, cypher::Node *, cypher::Relationship *,
                                             ExpandTowards, bool>>{})
        : OpBase(OpType::GQL_TRAVERSAl, "Gql Traversal"),
          start_label_(label),
          start_alias_(alias),
          result_set_header_(result_set_header),
          agg_expr_(agg_expr),
          noneagg_expr_(noneagg_expr),
          expands_(expands) {
        if (noneagg_expr.type == ArithExprNode::ArithExprType::AR_AST_EXP) {
            if (noneagg_expr.evaluator->GetExpression()->type() !=
                geax::frontend::AstNodeType::kGetField) {
                CYPHER_TODO();
            }
            noneagg_property_ =
                ((geax::frontend::GetField *)(noneagg_expr.evaluator->GetExpression()))
                    ->fieldName();
        }
        filter_ = nullptr;
    }

    void SetFilter(geax::frontend::Expr *filter) { filter_ = filter; }

    OpResult Initialize(RTContext *ctx) override {
        record = std::make_shared<Record>(result_set_header_.colums.size());
        result_idx_ = 0;
        state_ = Initialized;
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        if (state_ == Initialized) {
            ParallelTraversal(ctx);
            result_idx_ = 0;
            state_ = Consuming;
        }
        return HandOff();
    }

    OpResult ResetImpl(bool complete) override {
        if (complete) {
            record = nullptr;
            state_ = Initialized;
            result_idx_ = 0;
        }
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [").append(start_alias_);
        for (size_t i = 0; i < expands_.size(); i++) {
            auto towards = std::get<3>(expands_[i]) == ExpandTowards::FORWARD    ? "-->"
                           : std::get<3>(expands_[i]) == ExpandTowards::REVERSED ? "<--"
                                                                                 : "--";
            str.append(towards).append(std::get<1>(expands_[i])->Alias());
        }
        str.append("]");
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
