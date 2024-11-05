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

#include "cypher/execution_plan/ops/op_create.h"
#include "cypher/execution_plan/runtime_context.h"
#include "cypher/utils/geax_util.h"
#include "cypher/cypher_exception.h"
#include "cypher/arithmetic/arithmetic_expression.h"

namespace cypher {

void OpGqlCreate::ExtractProperties(RTContext *ctx, std::unordered_map<std::string, Value>& properties,
                       const geax::frontend::ElementFiller* filler) {
    auto& props = filler->predicates();
    //if (props.size() > 1) CYPHER_TODO();
    for (auto p : props) {
        geax::frontend::PropStruct* prop = nullptr;
        checkedCast(p, prop);
        if (!prop) CYPHER_TODO();
        for (auto& property : prop->properties()) {
            ArithExprNode ae(std::get<1>(property), *record->symbol_table);
            auto ent = ae.Evaluate(ctx, *record);
            properties.emplace(std::get<0>(property), std::move(ent.constant));
        }
    }
}

void OpGqlCreate::CreateVertex(RTContext *ctx, geax::frontend::Node* node) {
    //if (!node->filler()->label().has_value()) CYPHER_TODO();
    auto & variable = node->filler()->v().value();
    std::vector<std::string> labels;
    if (node->filler()->label().has_value()) {
        ExtractLabelTree(labels, node->filler()->label().value());
    }
    std::unordered_map<std::string, Value> properties;
    ExtractProperties(ctx, properties, node->filler());
    auto v = ctx->txn_->CreateVertex(std::unordered_set<std::string>{labels.begin(), labels.end()}, properties);
    //auto vid = ctx->txn_->AddVertex(labels[0], fields, values);
    ctx->result_info_->statistics.vertices_created++;
    // update pattern graph
    // add isolated node
    if (!variable.empty()) {
        /* There could be multiple match,
         * e.g. MATCH (a:Film) CREATE (b)  */
        auto pattern_node = &pattern_graph_->GetNode(variable);
        if (pattern_node->Empty()) CYPHER_TODO();
        pattern_node->Visited() = true;
        pattern_node->vertex_ = v;
        // fill the record
        if (!summary_) {
            auto it = pattern_graph_->symbol_table.symbols.find(variable);
            CYPHER_THROW_ASSERT(it != pattern_graph_->symbol_table.symbols.end());
            record->values[it->second.id].type = Entry::NODE;
            record->values[it->second.id].node = pattern_node;
        }
    }
}

void OpGqlCreate::CreateEdge(RTContext *ctx, geax::frontend::Node* start,
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
    if (!src_node.vertex_ || !dst_node.vertex_) {
        THROW_CODE(CypherException,
                           "Invalid vertex when create edge [{}]: {}", edge_variable,
                           !src_node.vertex_ ? src_variable : dst_variable);
    }
    if (!edge->filler()->label().has_value()) CYPHER_TODO();
    std::vector<std::string> labels;
    ExtractLabelTree(labels, edge->filler()->label().value());
    std::unordered_map<std::string, Value> props;
    ExtractProperties(ctx, props, edge->filler());
    auto e = ctx->txn_->CreateEdge(src_node.vertex_.value(), dst_node.vertex_.value(), labels[0], props);
    ctx->result_info_->statistics.edges_created++;
    if (!edge_variable.empty()) {
        /* There could be multiple match,
         * e.g. MATCH (a:Film),(b:City) CREATE (a)-[r:BORN_IN]->(b)  */
        auto relp = &pattern_graph_->GetRelationship(edge_variable);
        if (relp->Empty()) CYPHER_TODO();
        relp->edge_ = e;
        // fill the record
        if (!summary_) {
            auto it = pattern_graph_->symbol_table.symbols.find(edge_variable);
            CYPHER_THROW_ASSERT(it != pattern_graph_->symbol_table.symbols.end());
            record->values[it->second.id].type = Entry::RELATIONSHIP;
            record->values[it->second.id].relationship = relp;
        }
    }
}

void OpGqlCreate::CreateVE(RTContext *ctx) {
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
}

void OpGqlCreate::ResultSummary(RTContext *ctx) {
    if (summary_) {
        std::string summary;
        summary.append("created ")
                .append(std::to_string(ctx->result_info_->statistics.vertices_created))
                .append(" vertices, created ")
                .append(std::to_string(ctx->result_info_->statistics.edges_created))
                .append(" edges.");
        /*auto header = ctx->result_->Header();
        header.clear();
        header.emplace_back(std::make_pair("<SUMMARY>", lgraph_api::LGraphType::STRING));
        ctx->result_->ResetHeader(header);*/
        CYPHER_THROW_ASSERT(record);
        record->values.clear();
        record->AddConstant(Value(summary));
    } else {
        /* There should be a "Project" operation on top of this
         * "Create", so leave result set to it. */
    }
}

}
