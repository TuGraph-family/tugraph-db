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
// Created by wt on 18-7-27.
//
#pragma once

#include "parser/clause.h"
#include "cypher/execution_plan/ops/op.h"

namespace cypher {

class OpCreate : public OpBase {
    const SymbolTable &sym_tab_;
    std::vector<parser::Clause::TYPE_CREATE> create_data_;
    PatternGraph *pattern_graph_ = nullptr;
    bool standalone_ = false;
    bool summary_ = false;

    void ExtractProperties(RTContext *ctx, const parser::TUP_PROPERTIES &properties,
                           VEC_STR &fields, std::vector<lgraph::FieldData> &values);

    void CreateVertex(RTContext *ctx, const parser::TUP_NODE_PATTERN &node_pattern) {
        using namespace parser;
        auto &node_variable = std::get<0>(node_pattern);
        auto &node_labels = std::get<1>(node_pattern);
        auto &properties = std::get<2>(node_pattern);
        if (node_labels.empty())
            throw lgraph::EvaluationException("vertex label missing in create.");
        const std::string &label = *node_labels.begin();
        VEC_STR fields;
        std::vector<lgraph::FieldData> values;
        ExtractProperties(ctx, properties, fields, values);
        auto vid = ctx->txn_->AddVertex(label, fields, values);
        ctx->result_info_->statistics.vertices_created++;
        // update pattern graph
        // add isolated node
        if (!node_variable.empty()) {
            /* There could be multiple match,
             * e.g. MATCH (a:Film) CREATE (b)  */
            auto node = &pattern_graph_->GetNode(node_variable);
            if (node->Empty()) CYPHER_TODO();
            node->Visited() = true;
            node->PushVid(vid);
            // fill the record
            if (!summary_) {
                auto it = sym_tab_.symbols.find(node_variable);
                CYPHER_THROW_ASSERT(it != sym_tab_.symbols.end());
                record->values[it->second.id].type = Entry::NODE;
                record->values[it->second.id].node = node;
            }
        }
    }

    void CreateEdge(RTContext *ctx, const parser::TUP_NODE_PATTERN &lhs_patt,
                    const parser::TUP_NODE_PATTERN &rhs_patt,
                    const parser::TUP_RELATIONSHIP_PATTERN &relp_pattern) {
        using namespace parser;
        auto src_node_var = std::get<0>(lhs_patt);
        auto dst_node_var = std::get<0>(rhs_patt);
        auto direction = std::get<0>(relp_pattern);
        auto &relationship_detail = std::get<1>(relp_pattern);
        if (direction == LinkDirection::RIGHT_TO_LEFT) {
            std::swap(src_node_var, dst_node_var);
        } else if (direction != LinkDirection::LEFT_TO_RIGHT) {
            throw lgraph::CypherException("Only directed relationships are supported in CREATE");
        }
        auto &edge_variable = std::get<0>(relationship_detail);
        auto &relationship_types = std::get<1>(relationship_detail);
        auto &properties = std::get<3>(relationship_detail);
        // add edge
        if (relationship_types.empty())
            throw lgraph::CypherException("Edge label missing in create");
        // TODO(anyone) use children's record instead?
        auto &src_node = pattern_graph_->GetNode(src_node_var);
        auto &dst_node = pattern_graph_->GetNode(dst_node_var);
        auto &label = relationship_types[0];
        if (src_node.PullVid() < 0 || dst_node.PullVid() < 0) {
            throw lgraph::CypherException(
                FMA_FMT("Invalid vertex when create edge [{}]: {}", edge_variable,
                        src_node.PullVid() < 0 ? src_node_var : dst_node_var));
        }
        VEC_STR fields;
        std::vector<lgraph::FieldData> values;
        ExtractProperties(ctx, properties, fields, values);
        auto euid =
            ctx->txn_->AddEdge(src_node.PullVid(), dst_node.PullVid(), label, fields, values);
        ctx->result_info_->statistics.edges_created++;
        if (!edge_variable.empty()) {
            /* There could be multiple match,
             * e.g. MATCH (a:Film),(b:City) CREATE (a)-[r:BORN_IN]->(b)  */
            auto relp = &pattern_graph_->GetRelationship(edge_variable);
            if (relp->Empty()) CYPHER_TODO();
            relp->ItRef()->Initialize(ctx->txn_->GetTxn().get(), euid);
            // fill the record
            if (!summary_) {
                auto it = sym_tab_.symbols.find(edge_variable);
                CYPHER_THROW_ASSERT(it != sym_tab_.symbols.end());
                record->values[it->second.id].type = Entry::RELATIONSHIP;
                record->values[it->second.id].relationship = relp;
            }
        }
    }

    void CreateVE(RTContext *ctx) {
        for (auto &pattern : create_data_) {
            for (auto &pattern_part : pattern) {
                auto pp_variable = std::get<0>(pattern_part);
                if (!pp_variable.empty()) CYPHER_TODO();
                auto pattern_element = std::get<1>(pattern_part);
                auto lhs_patt = std::get<0>(pattern_element);
                auto pattern_element_chains = std::get<1>(pattern_element);
                if (pattern_element_chains.empty()) {  // create vertex
                    CreateVertex(ctx, lhs_patt);
                } else {  // create chains
                    for (auto &chain : pattern_element_chains) {
                        auto &rhs_patt = std::get<1>(chain);
                        auto &relp_pattern = std::get<0>(chain);
                        auto &lhs_node = pattern_graph_->GetNode(std::get<0>(lhs_patt));
                        auto &rhs_node = pattern_graph_->GetNode(std::get<0>(rhs_patt));
                        if (lhs_node.derivation_ == Node::CREATED && !lhs_node.Visited()) {
                            CreateVertex(ctx, lhs_patt);
                        }
                        if (rhs_node.derivation_ == Node::CREATED && !rhs_node.Visited()) {
                            CreateVertex(ctx, rhs_patt);
                        }
                        CreateEdge(ctx, lhs_patt, rhs_patt, relp_pattern);
                        lhs_patt = rhs_patt;
                    }
                }
            }  // for pattern_part
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
    OpCreate(const parser::QueryPart *stmt, PatternGraph *pattern_graph)
        : OpBase(OpType::CREATE, "Create"),
          sym_tab_(pattern_graph->symbol_table),
          pattern_graph_(pattern_graph) {
        for (auto &node : pattern_graph_->GetNodes()) {
            if (node.derivation_ == Node::CREATED) modifies.emplace_back(node.Alias());
            for (auto rr : node.RhsRelps()) {
                auto &r = pattern_graph_->GetRelationship(rr);
                if (r.derivation_ == Relationship::CREATED) modifies.emplace_back(r.Alias());
            }
        }
        for (auto c : stmt->create_clause) {
            create_data_.emplace_back(*c);
        }
    }

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(parent && children.size() < 2);
        summary_ = !parent->parent;
        standalone_ = children.empty();
        for (auto child : children) {
            child->Initialize(ctx);
        }
        record = children.empty() ?
            std::make_shared<Record>(sym_tab_.symbols.size(), &sym_tab_, ctx->param_tab_)
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

    const SymbolTable& SymTab() const { return sym_tab_; }

    PatternGraph* GetPatternGraph() const { return pattern_graph_; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
