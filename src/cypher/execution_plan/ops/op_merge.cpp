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

#include "cypher/execution_plan/ops/op_merge.h"
#include "cypher/arithmetic/arithmetic_expression.h"
#include "geax-front-end/ast/clause/Edge.h"
#include "geax-front-end/ast/clause/LabelOr.h"
#include "geax-front-end/ast/clause/PathPattern.h"
#include "geax-front-end/ast/clause/SingleLabel.h"
#include "geax-front-end/ast/clause/UpdateProperties.h"
#include "geax-front-end/ast/stmt/SetStatement.h"

namespace cypher {

void static ExtractLabelTree(std::vector<std::string> &labels, const geax::frontend::LabelTree *root) {
    if (typeid(*root) == typeid(geax::frontend::SingleLabel)) {
        labels.emplace_back(((geax::frontend::SingleLabel *)root)->label());
    } else if (typeid(*root) == typeid(geax::frontend::LabelOr)) {
        auto *label_or = (geax::frontend::LabelOr *)root;
        ExtractLabelTree(labels, label_or->left());
        ExtractLabelTree(labels, label_or->right());
    } else {
        CYPHER_TODO();
    }
}

void OpGqlMerge::MergeNode(RTContext *ctx, const geax::frontend::Node *node_pattern) {
    std::unordered_map<std::string, Value> properties_merge, properties_on_match, properties_on_create;
    if (!node_pattern->filler()->v().has_value()) CYPHER_TODO();
    auto node_variable = node_pattern->filler()->v().value();  // string
    std::vector<std::string> labels;
    if (node_pattern->filler()->label().has_value()) {
        ExtractLabelTree(labels, node_pattern->filler()->label().value());
    }
    if (labels.size() > 1) {
        CYPHER_TODO();
    }
    std::vector<std::string> fields;
    std::vector<Value> values;
    ExtractProperties(ctx, properties_merge, node_pattern->filler());
    ExtractProperties(ctx, node_variable, properties_on_match, on_match_items_);
    ExtractProperties(ctx, node_variable, properties_on_create, on_create_items_);
    std::optional<std::string> merge_label;
    if (labels.size() == 1) {
        merge_label = labels[0];
    }
    std::optional<std::unordered_map<std::string, Value>> merge_props;
    if (!properties_merge.empty()) {
        merge_props = properties_merge;
    }
    std::optional<graphdb::Vertex> v;
    bool on_match = false;
    for (auto iter = ctx->txn_->NewVertexIterator(merge_label, merge_props); iter->Valid(); iter->Next()) {
        /* ON MATCH */
        on_match = true;
        iter->GetVertex().SetProperties(properties_on_match);
        v = iter->GetVertex();
    }
    if (!on_match) {
        /* ON CREATE */
        for (const auto& pair : properties_on_create) {
            properties_merge.insert(pair);
        }
        v = ctx->txn_->CreateVertex({labels.begin(), labels.end()}, properties_merge);
        ctx->result_info_->statistics.vertices_created++;
    }
    // TODO(anyone) When multiple nodes are matched, all data should be processed
    if (!node_variable.empty()) {
        auto node = &pattern_graph_->GetNode(node_variable);
        if (node->Empty()) CYPHER_TODO();
        node->Visited() = true;
        node->vertex_ = v;
        if (!summary_) {
            auto it = sym_tab_.symbols.find(node_variable);
            CYPHER_THROW_ASSERT(it != sym_tab_.symbols.end());
            record->values[it->second.id].type = Entry::NODE;
            record->values[it->second.id].node = node;
        }
    }
}

void OpGqlMerge::MergeVE(RTContext* ctx) {
    if (path_pattern_->prefix().has_value()) CYPHER_TODO();
    auto pattern_element = path_pattern_->chains();  // TUP_PATTERN_ELEMENT
    for (auto chain : path_pattern_->chains()) {
        auto start_node = chain->head();
        auto pattern_element_chains = chain->tails();
        if (pattern_element_chains.empty()) {  // create vertex
            MergeNode(ctx, start_node);
        } else {  // create chains
            for (auto &element_chainchain : pattern_element_chains) {
                auto rel = (geax::frontend::Edge *)std::get<0>(element_chainchain);
                auto end_node = std::get<1>(element_chainchain);
                if (!pattern_graph_->GetNode(start_node->filler()->v().value()).Visited()) {
                    // for eg: match(node1),(node2) or
                    // merge(m:person{name:""})
                    // -[r:konws{}]->(n:person{name:""})
                    MergeNode(ctx, start_node);
                }
                if (!pattern_graph_->GetNode(end_node->filler()->v().value()).Visited()) {
                    // for eg: merge(m:person{name:""})
                    // -[r:konws{}]->(n:person{name:""})
                    MergeNode(ctx, end_node);
                }
                MergeChains(ctx, start_node, end_node, rel);
                start_node = end_node;
            }
        }  // end else
    }
}

void OpGqlMerge::ExtractProperties(RTContext *ctx, std::unordered_map<std::string, Value>& ext_props,
                       const geax::frontend::ElementFiller *filler) {
    auto &props = filler->predicates();
    for (auto p : props) {
        geax::frontend::PropStruct *prop = nullptr;
        checkedCast(p, prop);
        if (!prop) CYPHER_TODO();
        for (auto &property : prop->properties()) {
            ArithExprNode ae(std::get<1>(property), *record->symbol_table);
            auto value = ae.Evaluate(ctx, *record);
            CYPHER_THROW_ASSERT(value.IsScalar());
            ext_props.emplace(std::get<0>(property), std::move(value.constant));
        }
    }
}

void OpGqlMerge::ExtractProperties(RTContext *ctx, const std::string &var, std::unordered_map<std::string, Value>& ex_props,
                       const std::vector<geax::frontend::SetStatement *>& set_items) {
    for (auto set_item : set_items) {
        for (auto &item : set_item->items()) {
            if (typeid(*item) != typeid(geax::frontend::UpdateProperties)) CYPHER_TODO();
            geax::frontend::UpdateProperties *props = nullptr;
            checkedCast(item, props);
            if (props->v() != var) {
                THROW_CODE(CypherException, "Variable `{}` not found", props->v());
            }
            auto properties = props->structs()->properties();
            for (auto property : properties) {
                ArithExprNode ae(std::get<1>(property), *record->symbol_table);
                auto val = ae.Evaluate(ctx, *record);
                if (!val.IsScalar()) CYPHER_TODO();
                ex_props.emplace(std::get<0>(property), std::move(val.constant));
            }
        }
    }
}

void OpGqlMerge::MergeChains(RTContext *ctx, const geax::frontend::Node *node_patt,
                             const geax::frontend::Node *rls_node_pattern, const geax::frontend::Edge *rls_patt) {
    using namespace parser;
    auto src_node_var = node_patt->filler()->v().value();
    auto dst_node_var = rls_node_pattern->filler()->v().value();
    auto direction = rls_patt->direction();
    if (direction == geax::frontend::EdgeDirection::kPointLeft) {
        std::swap(src_node_var, dst_node_var);
    } else if (direction != geax::frontend::EdgeDirection::kPointRight) {
        THROW_CODE(CypherException, "Only directed relationships are supported in merge");
    }
    auto &edge_variable = rls_patt->filler()->v().value();
    std::vector<std::string> edge_labels;
    if (rls_patt->filler()->label().has_value()) {
        ExtractLabelTree(edge_labels, rls_patt->filler()->label().value());
    }
    if (edge_labels.empty()) THROW_CODE(CypherException, "edge label missing in merge");
    auto &src_node = pattern_graph_->GetNode(src_node_var);
    auto &dst_node = pattern_graph_->GetNode(dst_node_var);  // get node.Lable() get node.Prop()
    auto &label = edge_labels[0];
    if (!src_node.vertex_ || !dst_node.vertex_) {
        std::string err = src_node.vertex_ ? dst_node_var : src_node_var;
        THROW_CODE(CypherException, "Invalid vertex when create edge: " + err);
    }
    // TODO(anyone) Currently only one field can be obtained
    std::unordered_map<std::string, Value> properties_merge, properties_on_match, properties_on_create;
    std::unordered_map<std::string, Value> edge_properties;
    ExtractProperties(ctx, edge_properties, rls_patt->filler());
    ExtractProperties(ctx, edge_variable, properties_on_match,on_match_items_);
    ExtractProperties(ctx, edge_variable, properties_on_create,on_create_items_);
    std::optional<graphdb::Edge> e;
    bool matched = false;
    for (auto iter = src_node.vertex_->NewEdgeIterator(graphdb::EdgeDirection::OUTGOING, label, edge_properties, dst_node.vertex_.value());
         iter->Valid(); iter->Next()) {
        matched = true;
        e = iter->GetEdge();
    }
    if (!matched) {
        e = ctx->txn_->CreateEdge(src_node.vertex_.value(), dst_node.vertex_.value(), label, edge_properties);
        ctx->result_info_->statistics.edges_created++;
    }
    if (!edge_variable.empty()) {
        auto relp = &pattern_graph_->GetRelationship(edge_variable);
        if (relp->Empty()) CYPHER_TODO();
        relp->edge_ = e;
        if (!summary_) {
            auto it = sym_tab_.symbols.find(edge_variable);
            CYPHER_THROW_ASSERT(it != sym_tab_.symbols.end());
            record->values[it->second.id].type = Entry::RELATIONSHIP;
            record->values[it->second.id].relationship = relp;
        }
    }
}


}
