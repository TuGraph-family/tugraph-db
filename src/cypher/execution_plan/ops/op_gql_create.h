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

#include "cypher/execution_plan/ops/op.h"
#include "cypher/utils/geax_util.h"
#include "cypher/arithmetic//arithmetic_expression.h"
#include "cypher/cypher_exception.h"

namespace cypher {

class OpGqlCreate : public OpBase {
    const std::vector<geax::frontend::PathChain*>& paths_;
    PatternGraph* pattern_graph_;
    bool standalone_ = false;
    bool summary_ = false;

    void ExtractProperties(RTContext *ctx, std::vector<std::string> &fields,
                           std::vector<lgraph::FieldData> &values,
                           const geax::frontend::ElementFiller* filler) {
        auto& props = filler->predicates();
        if (props.size() > 1) CYPHER_TODO();
        for (auto p : props) {
            geax::frontend::PropStruct* prop = nullptr;
            checkedCast(p, prop);
            if (!prop) CYPHER_TODO();
            for (auto& property : prop->properties()) {
                fields.emplace_back(std::get<0>(property));
                ArithExprNode ae(std::get<1>(property), *record->symbol_table);
                auto value = ae.Evaluate(ctx, *record);
                CYPHER_THROW_ASSERT(value.IsScalar() || value.IsArray());
                if (value.IsScalar()) {
                    values.emplace_back(value.constant.scalar);
                } else {
                    std::vector<float> float_vector;
                    for (const auto &item : *value.constant.array) {
                        if (item.IsReal()) {
                            float_vector.push_back(item.AsDouble());
                        } else if (item.IsInteger()) {
                            float_vector.push_back(item.AsInt64());
                        } else {
                            THROW_CODE(CypherParameterTypeError, "vector type "
                                       "only support real & int");
                        }
                    }
                    values.emplace_back(float_vector);
                }
            }
        }
    }

    void ExtractLabelTree(std::vector<std::string>& labels,
                           const geax::frontend::LabelTree* root) {
        if (typeid(*root) == typeid(geax::frontend::SingleLabel)) {
            labels.emplace_back(((geax::frontend::SingleLabel*)root)->label());
        } else if (typeid(*root) == typeid(geax::frontend::LabelOr)) {
            geax::frontend::LabelOr* label_or = (geax::frontend::LabelOr*)root;
            ExtractLabelTree(labels, label_or->left());
            ExtractLabelTree(labels, label_or->right());
        } else {
            CYPHER_TODO();
        }
    }

    void CreateVertex(RTContext *ctx, geax::frontend::Node* node) {
        if (!node->filler()->label().has_value()) CYPHER_TODO();
        auto & variable = node->filler()->v().value();
        std::vector<std::string> labels;
        ExtractLabelTree(labels, node->filler()->label().value());
        std::vector<std::string> fields;
        std::vector<lgraph::FieldData> values;
        ExtractProperties(ctx, fields, values, node->filler());
        auto vid = ctx->txn_->AddVertex(labels[0], fields, values);
        ctx->result_info_->statistics.vertices_created++;
        // update pattern graph
        // add isolated node
        if (!variable.empty()) {
            /* There could be multiple match,
             * e.g. MATCH (a:Film) CREATE (b)  */
            auto node = &pattern_graph_->GetNode(variable);
            if (node->Empty()) CYPHER_TODO();
            node->Visited() = true;
            node->PushVid(vid);
            // fill the record
            if (!summary_) {
                auto it = pattern_graph_->symbol_table.symbols.find(variable);
                CYPHER_THROW_ASSERT(it != pattern_graph_->symbol_table.symbols.end());
                record->values[it->second.id].type = Entry::NODE;
                record->values[it->second.id].node = node;
            }
        }
    }

    void CreateEdge(RTContext *ctx, geax::frontend::Node* start,
                    geax::frontend::Edge* edge, geax::frontend::Node* end) {
        auto src_variable = start->filler()->v().value();
        auto dst_variable = end->filler()->v().value();
        auto direction = edge->direction();
        if (direction ==  geax::frontend::EdgeDirection::kPointLeft) {
            std::swap(src_variable, dst_variable);
        } else if (direction !=  geax::frontend::EdgeDirection::kPointRight) {
            CYPHER_TODO();
        }
        auto &src_node = pattern_graph_->GetNode(src_variable);
        auto &dst_node = pattern_graph_->GetNode(dst_variable);
        auto &edge_variable = edge->filler()->v().value();     // variable
        if (src_node.PullVid() < 0 || dst_node.PullVid() < 0) {
            throw lgraph::CypherException(
                FMA_FMT("Invalid vertex when create edge [{}]: {}", edge_variable,
                        src_node.PullVid() < 0 ? src_variable : dst_variable));
        }
        if (!edge->filler()->label().has_value()) CYPHER_TODO();
        std::vector<std::string> labels;
        ExtractLabelTree(labels, edge->filler()->label().value());
        std::vector<std::string> fields;
        std::vector<lgraph::FieldData> values;
        ExtractProperties(ctx, fields, values, edge->filler());
        auto euid =
            ctx->txn_->AddEdge(src_node.PullVid(), dst_node.PullVid(), labels[0], fields, values);
        ctx->result_info_->statistics.edges_created++;
        if (!edge_variable.empty()) {
            /* There could be multiple match,
             * e.g. MATCH (a:Film),(b:City) CREATE (a)-[r:BORN_IN]->(b)  */
            auto relp = &pattern_graph_->GetRelationship(edge_variable);
            if (relp->Empty()) CYPHER_TODO();
            relp->ItRef()->Initialize(ctx->txn_->GetTxn().get(), euid);
            // fill the record
            if (!summary_) {
                auto it = pattern_graph_->symbol_table.symbols.find(edge_variable);
                CYPHER_THROW_ASSERT(it != pattern_graph_->symbol_table.symbols.end());
                record->values[it->second.id].type = Entry::RELATIONSHIP;
                record->values[it->second.id].relationship = relp;
            }
        }
    }

    void CreateVE(RTContext *ctx) {
        for (auto path : paths_) {
            auto start = path->head();
            if (!start->filler()->v().has_value()) CYPHER_TODO();
            auto& start_name = start->filler()->v().value();
            auto& lhs_node = pattern_graph_->GetNode(start_name);
            if (lhs_node.derivation_ == Node::CREATED && !lhs_node.Visited()) {
                CreateVertex(ctx, start);
            }
            const auto& tails = path->tails();
            for (auto& tail_tup : tails) {
                auto end = std::get<1>(tail_tup);
                if (!end->filler()->v().has_value()) CYPHER_TODO();
                auto& end_name = end->filler()->v().value();
                auto& rhs_node = pattern_graph_->GetNode(end_name);
                if (rhs_node.derivation_ == Node::CREATED && !rhs_node.Visited()) {
                    CreateVertex(ctx, end);
                }

                auto edge = (geax::frontend::Edge*)std::get<0>(tail_tup);
                if (!edge->filler()->v().has_value()) CYPHER_TODO();
                auto& edge_name = edge->filler()->v().value();
                auto& e = pattern_graph_->GetRelationship(edge_name);
                if (e.derivation_ == Relationship::CREATED) {
                    CreateEdge(ctx, start, edge, end);
                }
                start = end;
            }
        }
        ctx->txn_->GetTxn()->RefreshIterators();
    }

    void ResultSummary(RTContext *ctx) {
        if (summary_) {
            std::string summary;
            summary.append("created ")
                .append(std::to_string(ctx->result_info_->statistics.vertices_created))
                .append(" vertices, created ")
                .append(std::to_string(ctx->result_info_->statistics.edges_created))
                .append(" edges.");
            CYPHER_THROW_ASSERT(ctx->result_->Header().size() == 1);
            CYPHER_THROW_ASSERT(record);
            record->values.clear();
            record->AddConstant(lgraph::FieldData(summary));
        } else {
            /* There should be a "Project" operation on top of this
             * "Create", so leave result set to it. */
        }
    }

 public:
    OpGqlCreate(const std::vector<geax::frontend::PathChain*>& paths,
                PatternGraph* pattern_graph)
        : OpBase(OpType::GQL_CREATE, "Gql Create")
        , paths_(paths)
        , pattern_graph_(pattern_graph) {
        for (auto &node : pattern_graph_->GetNodes()) {
            if (node.derivation_ == Node::CREATED) modifies.emplace_back(node.Alias());
            for (auto rr : node.RhsRelps()) {
                auto &r = pattern_graph_->GetRelationship(rr);
                if (r.derivation_ == Relationship::CREATED) modifies.emplace_back(r.Alias());
            }
        }
    }

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(parent && children.size() < 2);
        summary_ = !parent->parent;
        standalone_ = children.empty();
        for (auto child : children) {
            child->Initialize(ctx);
        }
        auto &sym_tab = pattern_graph_->symbol_table;
        record = children.empty() ? std::make_shared<Record>(sym_tab.symbols.size(),
                                     &sym_tab, ctx->param_tab_)
                                  : children[0]->record;
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        if (state == StreamDepleted) return OP_DEPLETED;
        if (standalone_) {
            CreateVE(ctx);
            ResultSummary(ctx);
            state = StreamDepleted;
            return OP_OK;
        } else {
            if (children.size() > 1) CYPHER_TODO();
            auto child = children[0];
            if (summary_) {
                while (child->Consume(ctx) == OP_OK) CreateVE(ctx);
                ResultSummary(ctx);
                state = StreamDepleted;
                return OP_OK;
            } else {
                if (child->Consume(ctx) == OP_OK) {
                    CreateVE(ctx);
                    return OP_OK;
                } else {
                    return OP_DEPLETED;
                }
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

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};

}  // namespace cypher
