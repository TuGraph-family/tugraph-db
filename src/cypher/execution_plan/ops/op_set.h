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
// Created by dcy on 19-8-22.
//
#pragma once

#include "op.h"
#include "parser/clause.h"

// todo: get resources then set all
namespace cypher {

class OpSet : public OpBase {
    std::vector<parser::Clause::TYPE_SET> set_data_;
    PatternGraph *pattern_graph_ = nullptr;
    std::vector<lgraph::VertexId> vertices_to_delete_;
    std::vector<lgraph::EdgeUid> edges_to_delete_;
    bool summary_ = false;

    size_t GetRecordIdx(const std::string &var) {
        auto it = record->symbol_table->symbols.find(var);
        CYPHER_THROW_ASSERT(it != record->symbol_table->symbols.end());
        return it->second.id;
    }

    void GetVertexFields(RTContext *ctx, lgraph::VertexId vid, VEC_STR &k,
                         std::vector<lgraph_api::FieldData> &v) {
        auto vit = ctx->txn_->GetVertexIterator(vid);
        auto fields = ctx->txn_->GetVertexFields(vit);
        for (auto &f : fields) {
            k.emplace_back(f.first);
            v.emplace_back(f.second);
        }
    }

    void GetEdgeFields(RTContext *ctx, lgraph::EdgeUid uid, VEC_STR &k,
                       std::vector<lgraph_api::FieldData> &v) {
        auto eit = ctx->txn_->GetOutEdgeIterator(uid, false);
        auto fields = ctx->txn_->GetEdgeFields(eit);
        for (auto &f : fields) {
            k.emplace_back(f.first);
            v.emplace_back(f.second);
        }
    }

    void ExtractProperties(RTContext *ctx, const parser::Expression &p,
                           std::vector<lgraph::FieldData> &values) {
        ArithExprNode ae(p, *record->symbol_table);
        auto val = ae.Evaluate(ctx, *record);
        if (!val.IsScalar()) CYPHER_TODO();
        values.emplace_back(val.constant.scalar);
    }

    void SetVertex(RTContext *ctx,
                   const parser::Expression::EXPR_TYPE_PROPERTY &property_expression,
                   const std::string &sign, const parser::Expression &value) {
        auto &node_variable = property_expression.first.String();
        VEC_STR fields;
        std::vector<lgraph_api::FieldData> values;
        if (sign != "=") CYPHER_INTL_ERR();
        fields.emplace_back(property_expression.second);
        ExtractProperties(ctx, value, values);
        auto idx = GetRecordIdx(node_variable);
        auto vid = record->values[idx].node->PullVid();
        ctx->txn_->SetVertexProperty(vid, fields, values);
        ctx->txn_->RefreshIterators();
        ctx->result_info_->statistics.properties_set += fields.size();
    }

    void SetVertex(RTContext *ctx, const std::string &lhs, const std::string &sign,
                   const parser::Expression &rhs) {
        using namespace parser;
        VEC_STR fields;
        std::vector<lgraph_api::FieldData> values;
        if (rhs.type == parser::Expression::DataType::MAP) {  // SET n += {hungry: TRUE,
                                                              // position: 'Entrepreneur'}
            if ((sign == "=") || (sign == "+=")) {
                auto &map = rhs.Map();
                for (auto &m : map) {
                    fields.emplace_back(m.first);
                    ExtractProperties(ctx, m.second, values);
                }
                auto idx = GetRecordIdx(lhs);
                auto vid = record->values[idx].node->PullVid();
                ctx->txn_->SetVertexProperty(vid, fields,
                                             values);
                ctx->txn_->RefreshIterators();
                ctx->result_info_->statistics.properties_set += fields.size();
            } else {
                CYPHER_TODO();
            }
        } else if (rhs.type == parser::Expression::DataType::VARIABLE) {  // SET n=m
            if (sign != "=") CYPHER_TODO();
            auto src = record->values[GetRecordIdx(rhs.String())];
            auto dst = record->values[GetRecordIdx(lhs)];
            GetVertexFields(ctx, src.node->ItRef()->GetId(), fields, values);
            auto vid = dst.node->PullVid();
            ctx->txn_->SetVertexProperty(vid, fields, values);
            ctx->txn_->RefreshIterators();
            ctx->result_info_->statistics.properties_set += fields.size();
        } else if (rhs.type == parser::Expression::DataType::NULL_) {
            if (sign != "=") CYPHER_TODO();
            auto idx = GetRecordIdx(lhs);
            if (record->values[idx].node && record->values[idx].node->ItRef() &&
                record->values[idx].node->ItRef()->IsValid()) {
                auto vid = record->values[idx].node->PullVid();
                vertices_to_delete_.emplace_back(vid);
                ctx->result_info_->statistics.properties_set++;
            }
        } else {
            CYPHER_TODO();
        }
    }

    void SetEdge(RTContext *ctx, const parser::Expression::EXPR_TYPE_PROPERTY &lhs,
                 const std::string &sign, const parser::Expression &rhs) {
        using namespace parser;
        auto &edge_variable = lhs.first.String();
        VEC_STR fields;
        std::vector<lgraph_api::FieldData> values;
        if (sign != "=") CYPHER_TODO();
        fields.emplace_back(lhs.second);
        ExtractProperties(ctx, rhs, values);
        auto idx = GetRecordIdx(edge_variable);
        ctx->txn_->SetEdgeProperty(record->values[idx].relationship->ItRef()->GetUid(), fields,
                                   values);
        ctx->txn_->RefreshIterators();
        ctx->result_info_->statistics.properties_set++;
    }

    void SetEdge(RTContext *ctx, const std::string &lhs, const std::string &sign,
                 const parser::Expression &rhs) {
        VEC_STR fields;
        std::vector<lgraph_api::FieldData> values;
        auto &edge_variable = lhs;
        if (rhs.type == parser::Expression::DataType::MAP) {
            // SET n += {hungry: TRUE, position: 'Entrepreneur'}
            auto &map = rhs.Map();
            for (auto &m : map) {
                fields.emplace_back(m.first);
                ExtractProperties(ctx, m.second, values);
            }
            if (sign == "+=") {
                auto idx = GetRecordIdx(edge_variable);
                ctx->txn_->SetEdgeProperty(record->values[idx].relationship->ItRef()->GetUid(),
                                           fields, values);
                ctx->txn_->RefreshIterators();
                ctx->result_info_->statistics.properties_set++;
            } else {
                CYPHER_TODO();
            }
        } else if (rhs.type == parser::Expression::DataType::VARIABLE) {
            // SET n=m
            auto src = record->values[GetRecordIdx(rhs.String())];
            auto dst = record->values[GetRecordIdx(lhs)];
            if (sign != "=") CYPHER_TODO();
            GetEdgeFields(ctx, src.relationship->ItRef()->GetUid(), fields, values);
            ctx->txn_->SetEdgeProperty(dst.relationship->ItRef()->GetUid(), fields, values);
            ctx->txn_->RefreshIterators();
            ctx->result_info_->statistics.properties_set++;
        } else if (rhs.type == parser::Expression::DataType::NULL_) {
            auto idx = GetRecordIdx(edge_variable);
            if (record->values[idx].relationship && record->values[idx].relationship->ItRef() &&
                record->values[idx].relationship->ItRef()->IsValid()) {
                edges_to_delete_.emplace_back(record->values[idx].relationship->ItRef()->GetUid());
                ctx->result_info_->statistics.properties_set++;
            }
        } else {
            CYPHER_TODO();
        }
    }

    void SetVE(RTContext *ctx) {
        OpBase *child = children[0];
        for (auto &s : set_data_) {   
            for (auto &set_item : s) {
                auto &lhs_var = std::get<0>(set_item);
                auto &lhs_prop_expr = std::get<1>(set_item);
                std::string sign = std::get<2>(set_item);
                auto &rhs_expr = std::get<3>(set_item);
                auto &rhs_node_labels = std::get<4>(set_item);
                if (lhs_var.empty()) {
                    auto &property = lhs_prop_expr.Property();
                    auto &key_expr = property.first;
                    if (key_expr.type != parser::Expression::VARIABLE) CYPHER_TODO();
                    std::string alias = key_expr.String();
                    auto it = record->symbol_table->symbols.find(alias);
                    CYPHER_THROW_ASSERT(it != record->symbol_table->symbols.end());
                    if (it->second.type == SymbolNode::NODE) {
                        /* set n.name = 'AB' */
                        SetVertex(ctx, property, sign, rhs_expr);
                    } else if (it->second.type == SymbolNode::RELATIONSHIP) {
                        SetEdge(ctx, property, sign, rhs_expr);
                    } else {
                        CYPHER_TODO();
                    }
                } else {
                    /* set n = m */
                    auto it = record->symbol_table->symbols.find(lhs_var);
                    if (it == record->symbol_table->symbols.end()) CYPHER_TODO();
                    if (it->second.type == SymbolNode::NODE) {
                        SetVertex(ctx, lhs_var, sign, rhs_expr);
                    } else if (it->second.type == SymbolNode::RELATIONSHIP) {
                        SetEdge(ctx, lhs_var, sign, rhs_expr);
                    } else {
                        CYPHER_TODO();
                    }
                }
            }
        }
    }

    void ResultSummary(RTContext *ctx) {
        if (summary_) {
            std::string summary;
            summary.append("set ")
                .append(std::to_string(ctx->result_info_->statistics.properties_set))
                .append(" properties.");
            // ctx->result_info_->header.colums.emplace_back("<SUMMARY>");
            auto header = ctx->result_->Header();
            header.emplace_back(std::make_pair("<SUMMARY>", lgraph::ElementType::STRING));
            ctx->result_->ResetHeader(header);
            CYPHER_THROW_ASSERT(record);
            record->values.clear();
            record->AddConstant(lgraph::FieldData(summary));
        } else {
            /* There should be a "Project" operation on top of this "Set",
             * so leave result set to it. */
        }
    }

    void DoDeleteVE(RTContext *ctx) {
        FMA_DBG() << "vertices & edges to delete:";
        for (auto &v : vertices_to_delete_) FMA_DBG() << "V[" << v << "]";
        for (auto &e : edges_to_delete_) FMA_DBG() << "E[" << _detail::EdgeUid2String(e) << "]";

        for (auto &e : edges_to_delete_) {
            if (ctx->txn_->DeleteEdge(e)) {
                ctx->result_info_->statistics.edges_deleted++;
            }
        }
        for (auto &v : vertices_to_delete_) {
            size_t n_in, n_out;
            if (ctx->txn_->DeleteVertex(v, &n_in, &n_out)) {
                ctx->result_info_->statistics.vertices_deleted++;
                ctx->result_info_->statistics.edges_deleted += n_in + n_out;
            }
        }
        /* NOTE:
         * lgraph::EdgeIterator will refresh itself after calling
         * Delete(), and point to the next element.
         * While lgraph::Transaction::DeleteEdge() will not refresh
         * the iterator, after calling this method, the edge iterator
         * just becomes invalid.  */
        ctx->txn_->RefreshIterators();
    }

 public:
    OpSet(const parser::QueryPart *stmt, PatternGraph *pattern_graph)
        : OpBase(OpType::UPDATE, "Set"), pattern_graph_(pattern_graph) {
        for (auto s : stmt->set_clause) {
            set_data_.emplace_back(*s);
        }
        state = StreamUnInitialized;
    }

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(parent && children.size() < 2);
        summary_ = !parent->parent;
        for (auto child : children) {
            child->Initialize(ctx);
        }
        record = children[0]->record;
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        if (state == StreamDepleted) return OP_DEPLETED;
        if (children.size() > 1) CYPHER_TODO();
        auto child = children[0];
        if (summary_) {
            while (child->Consume(ctx) == OP_OK) SetVE(ctx);
            DoDeleteVE(ctx);
            ResultSummary(ctx);
            state = StreamDepleted;
            return OP_OK;
        } else {
            if (child->Consume(ctx) == OP_OK) {
                SetVE(ctx);
                return OP_OK;
            } else {
                DoDeleteVE(ctx);
                return OP_DEPLETED;
            }
        }
    }

    OpResult ResetImpl(bool complete) override {
        state = StreamUnInitialized;
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [");
        for (auto &m : modifies) str.append(m).append(",");
        if (!modifies.empty()) str.pop_back();
        str.append("]");
        return str;
    }

    const std::vector<parser::Clause::TYPE_SET>& GetSetData() const { return set_data_; }

    PatternGraph* GetPatternGraph() const { return pattern_graph_; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()

};
}  // namespace cypher
