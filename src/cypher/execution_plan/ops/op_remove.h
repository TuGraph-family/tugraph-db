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
#include "geax-front-end/ast/clause/RemoveItem.h"
#include "cypher/graph/graph.h"

namespace cypher {

class OpGqlRemove : public OpBase {
    const std::vector<geax::frontend::RemoveItem*>& remove_items_;
    bool summary_ = false;

 public:
    OpGqlRemove(const std::vector<geax::frontend::RemoveItem*>& remove_items,
                PatternGraph *pattern_graph)
        : OpBase(OpType::GQL_REMOVE, "GqlRemove")
        , remove_items_(remove_items) {
        state = StreamUnInitialized;
    }

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(parent && children.size() == 1);
        summary_ = !parent->parent;
        auto res = children[0]->Initialize(ctx);
        record = children[0]->record;
        state = StreamUnInitialized;
        return res;
    }

    void ResultSummary(RTContext *ctx);
    void Remove(RTContext *ctx);
    OpResult RealConsume(RTContext *ctx) override;
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
