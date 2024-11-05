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
#include "cypher/graph/graph.h"
#include "geax-front-end/ast/clause/SetItem.h"

// TODO(anyone) get resources then set all
namespace cypher {

class OpGqlSet : public OpBase {
    const std::vector<geax::frontend::SetItem*>& set_items_;
    bool summary_ = false;

    size_t GetRecordIdx(const std::string &var) {
        auto it = record->symbol_table->symbols.find(var);
        CYPHER_THROW_ASSERT(it != record->symbol_table->symbols.end());
        return it->second.id;
    }

    void SetVertex(RTContext *ctx, const std::string& variable,
                   geax::frontend::UpdateProperties* update);
    void SetEdge(RTContext *ctx, const std::string& variable,
                 geax::frontend::UpdateProperties* update);
    void SetVE(RTContext *ctx);
    void ResultSummary(RTContext *ctx);

 public:
    OpGqlSet(const std::vector<geax::frontend::SetItem*>& set_items,
            PatternGraph *pattern_graph)
        : OpBase(OpType::GQL_UPDATE, "GqlSet")
        , set_items_(set_items) {
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
        if (state == StreamDepleted) {
            return OP_DEPLETED;
        }
        if (children.size() > 1) CYPHER_TODO();
        auto child = children[0];
        if (summary_) {
            while (child->Consume(ctx) == OP_OK) SetVE(ctx);
            ResultSummary(ctx);
            state = StreamDepleted;
            return OP_OK;
        } else {
            if (child->Consume(ctx) == OP_OK) {
                SetVE(ctx);
                return OP_OK;
            } else {
                return OP_DEPLETED;
            }
        }
        return OP_OK;
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
