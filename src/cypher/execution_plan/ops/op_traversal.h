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
#include "lgraph/lgraph.h"
#include "lgraph/lgraph_traversal.h"
#include "libcuckoo/cuckoohash_map.hh"

namespace cypher {

/* TraversalOp
 * Traversal entire graph Parallel
 * basic plan : Node Scan - Argument
 * more: Node Scan - Filter - Expand - Expand - Argument
 */

class Traversal : public OpBase {
    /** about node scan */
    std::string start_label_;
    std::string start_alias_;
    std::shared_ptr<lgraph::Filter> filter_;

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

    bool DoFilter(lgraph_api::VertexIterator &vit, std::shared_ptr<lgraph::Filter> f) {
        if (!f) {
            return true;
        }
        switch (f->LogicalOp()) {
        case lgraph::LBR_EMPTY:
            {
                lgraph::RangeFilter *rf = dynamic_cast<lgraph::RangeFilter *>(f.get());
                lgraph::CompareOp compare_op = rf->GetCompareOp();
                cypher::ArithExprNode ae_left = rf->GetAeLeft();
                cypher::ArithExprNode ae_right = rf->GetAeRight();
                std::string prop_name = ae_left.operand.variadic.entity_prop;
                if (prop_name.empty()) {
                    CYPHER_TODO();
                }
                lgraph::FieldData prop_value = ae_right.operand.constant.scalar;
                switch (compare_op) {
                case lgraph::LBR_EQ:
                    {
                        return vit[prop_name] == prop_value;
                    }
                case lgraph::LBR_NEQ:
                    {
                        return vit[prop_name] != prop_value;
                    }
                case lgraph::LBR_LT:
                    {
                        return vit[prop_name] < prop_value;
                    }
                case lgraph::LBR_GT:
                    {
                        return vit[prop_name] > prop_value;
                    }
                case lgraph::LBR_LE:
                    {
                        return vit[prop_name] <= prop_value;
                    }
                case lgraph::LBR_GE:
                    {
                        return vit[prop_name] >= prop_value;
                    }
                default:
                    return false;
                }
            }
        case lgraph::LBR_NOT:
            return f->Left() && !DoFilter(vit, f->Left());
        case lgraph::LBR_AND:
            return f->Left() && f->Right() &&
                   (DoFilter(vit, f->Left()) && DoFilter(vit, f->Right()));
        case lgraph::LBR_OR:
            return f->Left() && f->Right() &&
                   (DoFilter(vit, f->Left()) || DoFilter(vit, f->Right()));
        case lgraph::LBR_XOR:
            return f->Left() && f->Right() &&
                   (DoFilter(vit, f->Left()) != DoFilter(vit, f->Right()));
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
                        map_.upsert(
                            group_by_key, [](int &n) { ++n; }, 1);
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
                                map_.upsert(
                                    group_by_key, [](int &n) { ++n; }, 1);
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
                                map_.upsert(
                                    group_by_key, [](int &n) { ++n; }, 1);
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
                                map_.upsert(
                                    group_by_key, [](int &n) { ++n; }, 1);
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
                                map_.upsert(
                                    group_by_key, [](int &n) { ++n; }, 1);
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
    Traversal(std::string label, std::string alias, ResultSetHeader result_set_header,
              ArithExprNode agg_expr, ArithExprNode noneagg_expr,
              std::vector<std::tuple<cypher::Node *, cypher::Node *, cypher::Relationship *,
                                     ExpandTowards, bool>>
                  expands = std::vector<std::tuple<cypher::Node *, cypher::Node *,
                                                   cypher::Relationship *, ExpandTowards, bool>>{})
        : OpBase(OpType::TRAVERSAl, "Traversal"),
          start_label_(label),
          start_alias_(alias),
          result_set_header_(result_set_header),
          agg_expr_(agg_expr),
          noneagg_expr_(noneagg_expr),
          expands_(expands) {
        noneagg_property_ = noneagg_expr_.operand.variadic.entity_prop;
        filter_ = nullptr;
    }

    void SetFilter(std::shared_ptr<lgraph::Filter> filter) { filter_ = filter; }

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
