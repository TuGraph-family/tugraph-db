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
#include "cypher/execution_plan/runtime_context.h"
#include "cypher/graph/graph.h"

namespace cypher {

class OpGqlMerge : public OpBase {
    const std::vector<geax::frontend::SetStatement *> &on_match_items_;
    const std::vector<geax::frontend::SetStatement *> &on_create_items_;
    const geax::frontend::PathPattern *path_pattern_;
    const SymbolTable &sym_tab_;
    PatternGraph *pattern_graph_ = nullptr;
    bool standalone_;
    bool summary_;
    void ResultSummary(RTContext *ctx) {
        if (summary_) {
            std::string summary;
            summary.append("merged ")
                .append(std::to_string(ctx->result_info_->statistics.vertices_created))
                .append(" vertices, merged ")
                .append(std::to_string(ctx->result_info_->statistics.edges_created))
                .append(" edges.");
            //CYPHER_THROW_ASSERT(ctx->result_->Header().size() == 1);
            CYPHER_THROW_ASSERT(record);
            record->values.clear();
            record->AddConstant(Value(std::move(summary)));
        } else {
            /* There should be a "Project" operation on top of this
             * "Create", so leave result set to it. */
        }
    }
 public:
    OpGqlMerge(const std::vector<geax::frontend::SetStatement *> &on_match_items,
               const std::vector<geax::frontend::SetStatement *> &on_create_items,
               const geax::frontend::PathPattern *path_pattern, PatternGraph *pattern_graph)
        : OpBase(OpType::GQL_MERGE, "Gql Merge"),
          on_match_items_(on_match_items),
          on_create_items_(on_create_items),
          path_pattern_(path_pattern),
          sym_tab_(pattern_graph->symbol_table),
          pattern_graph_(pattern_graph) {
        for (auto &node : pattern_graph_->GetNodes()) {
            if (node.derivation_ == Node::MERGED) modifies.emplace_back(node.Alias());
            for (auto rr : node.RhsRelps()) {
                auto &r = pattern_graph_->GetRelationship(rr);
                if (r.derivation_ == Relationship::MERGED) modifies.emplace_back(r.Alias());
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
        record = children.empty()
                     ? std::make_shared<Record>(sym_tab_.symbols.size(), &sym_tab_)
                     : children[0]->record;
        return OP_OK;
    }

    void MergeVE(RTContext *ctx);
    void MergeChains(RTContext *ctx, const geax::frontend::Node *node_patt,
                 const geax::frontend::Node *rls_node_pattern,
                 const geax::frontend::Edge *rls_patt);
    void MergeNode(RTContext *ctx, const geax::frontend::Node *node_pattern);
    void ExtractProperties(RTContext *ctx, const std::string &var, std::unordered_map<std::string, Value>& props,
                       const std::vector<geax::frontend::SetStatement *>& set_items);
    void ExtractProperties(RTContext *ctx, std::unordered_map<std::string, Value>& ext_props,
                       const geax::frontend::ElementFiller *filler);
    OpResult RealConsume(RTContext *ctx) override {
        if (state == StreamDepleted) return OP_DEPLETED;
        if (standalone_) {
            MergeVE(ctx);
            ResultSummary(ctx);
            state = StreamDepleted;
            return OP_OK;
        } else {
            if (children.size() > 1) CYPHER_TODO();
            auto child = children[0];
            if (summary_) {
                while (child->Consume(ctx) == OP_OK) MergeVE(ctx);
                ResultSummary(ctx);
                state = StreamDepleted;
                return OP_OK;
            } else {
                if (child->Consume(ctx) == OP_OK) {
                    MergeVE(ctx);
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
