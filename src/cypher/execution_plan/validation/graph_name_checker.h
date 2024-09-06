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

#include "cypher/execution_plan/ops/ops.h"
#include "cypher/execution_plan/validation/validation_pass.h"
#include "cypher/execution_plan/runtime_context.h"
#include "cypher/execution_plan/visitor/visitor.h"

namespace cypher {
class GraphNameCheckerVisitor : public Visitor {
 public:
    explicit GraphNameCheckerVisitor(cypher::RTContext *ctx) : ctx_(ctx) {}

    void Visit(const OpBase &op) override {
        std::vector<const OpBase *> ops = {&op};
        for (size_t i = 0; i < ops.size(); i++) {
            auto op_ = ops[i];
            ops.insert(ops.end(), op_->children.begin(), op_->children.end());
            op_->Accept(this);
        }
        if (!has_standalone_call && ctx_->graph_.empty()) {
            throw lgraph::CypherException("Graph name cannot be empty.");
        }
    }

    void Visit(const Aggregate &op) override{};
    void Visit(const AllNodeScan &op) override{};
    void Visit(const AllNodeScanDynamic &op) override{};
    void Visit(const Apply &op) override{};
    void Visit(const Argument &op) override{};
    void Visit(const CartesianProduct &op) override{};
    void Visit(const OpCreate &op) override{};
    void Visit(const OpDelete &op) override{};
    void Visit(const Distinct &op) override{};
    void Visit(const ExpandAll &op) override{};
    void Visit(const OpFilter &op) override{};
    void Visit(const InQueryCall &op) override{};
    void Visit(const Limit &op) override{};
    void Visit(const OpMerge &op) override{};
    void Visit(const NodeByIdSeek &op) override{};
    void Visit(const NodeByLabelScan &op) override{};
    void Visit(const NodeByLabelScanDynamic &op) override{};
    void Visit(const NodeIndexSeek &op) override{};
    void Visit(const NodeIndexSeekDynamic &op) override{};
    void Visit(const Optional &op) override{};
    void Visit(const ProduceResults &op) override{};
    void Visit(const Project &op) override{};
    void Visit(const RelationshipCount &op) override{};
    void Visit(const OpRemove &op) override{};
    void Visit(const OpSet &op) override{};
    void Visit(const Skip &op) override{};
    void Visit(const Sort &op) override{};
    void Visit(const StandaloneCall &op) override { has_standalone_call = true; };
    void Visit(const OpGqlStandaloneCall &op) override { has_standalone_call = true; };
    void Visit(const TopN &op) override{};
    void Visit(const Union &op) override{};
    void Visit(const Unwind &op) override{};
    void Visit(const VarLenExpand &op) override{};
    void Visit(const VarLenExpandInto &op) override{};
    void Visit(const ImmediateArgument &op) override{};
    void Visit(const Traversal &op) override{};
    void Visit(const OpGqlCreate &op) override {}
    void Visit(const OpGqlDelete &op) override {}
    void Visit(const GqlInQueryCall &op) override {}
    void Visit(const OpGqlMerge &op) override {}
    void Visit(const OpGqlRemove &op) override {}
    void Visit(const OpGqlSet &op) override {}
    void Visit(const OpGqlTraversal &op) override {}


 private:
    cypher::RTContext *ctx_ = nullptr;
    bool has_standalone_call = false;
};

class GraphNameChecker : public ValidationPass {
 public:
    GraphNameChecker(OpBase *root, cypher::RTContext *ctx)
        : ValidationPass(typeid(GraphNameChecker).name()), root_(root), ctx_(ctx) {}

    bool Gate() override { return true; }

    int Execute() override {
        GraphNameCheckerVisitor visitor(ctx_);
        visitor.Visit(*root_);
        return 0;
    }

 private:
    OpBase *root_;
    cypher::RTContext *ctx_;
};

}  // namespace cypher
