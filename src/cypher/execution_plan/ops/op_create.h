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
#include "geax-front-end/ast/Ast.h"
#include "cypher/graph/graph.h"
#include "common/value.h"

namespace cypher {

class OpGqlCreate : public OpBase {
    const std::vector<geax::frontend::PathChain*>& paths_;
    PatternGraph* pattern_graph_;
    bool standalone_ = false;
    bool summary_ = false;

    void ExtractProperties(RTContext *ctx, std::unordered_map<std::string, Value>& properties,
                           const geax::frontend::ElementFiller* filler);

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

    void CreateVertex(RTContext *ctx, geax::frontend::Node* node, bool update_visited = true);

    void CreateEdge(RTContext *ctx, geax::frontend::Node* start,
                    geax::frontend::Edge* edge, geax::frontend::Node* end);

    void CreateVE(RTContext *ctx, bool update_visited = true);

    void ResultSummary(RTContext *ctx);

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
        const auto& sym_tab = pattern_graph_->symbol_table;
        record = children.empty() ? std::make_shared<Record>(sym_tab.symbols.size(),
                                     &sym_tab)
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
                while (child->Consume(ctx) == OP_OK) {
                    CreateVE(ctx, false);
                }
                for (auto path : paths_) {
                    auto start = path->head();
                    if (!start->filler()->v().has_value()) CYPHER_TODO();
                    auto& start_name = start->filler()->v().value();
                    auto& lhs_node = pattern_graph_->GetNode(start_name);
                    lhs_node.Visited() = true;
                    const auto& tails = path->tails();
                    for (auto& tail_tup : tails) {
                        auto end = std::get<1>(tail_tup);
                        if (!end->filler()->v().has_value()) CYPHER_TODO();
                        auto& end_name = end->filler()->v().value();
                        auto& rhs_node = pattern_graph_->GetNode(end_name);
                        rhs_node.Visited() = true;
                    }
                }
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

    OpResult ResetImpl(bool ) override {
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
