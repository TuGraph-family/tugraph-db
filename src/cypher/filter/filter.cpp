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
// Created by wt on 6/29/18.
//
#include <regex>

#include "cypher/filter/filter.h"
#include "execution_plan/ops/op_expand_all.h"
#include "execution_plan/ops/op_var_len_expand.h"
#include "execution_plan/ops/op_immediate_argument.h"

namespace cypher {

// TODO(anyone) also defined in execution_plan.cpp
// Connect ops into a single branch stream.
static OpBase *_SingleBranchConnect(const std::vector<OpBase *> &ops) {
    if (ops.empty()) return nullptr;
    OpBase *child, *parent = ops[0];
    for (int i = 1; i < (int)ops.size(); i++) {
        child = ops[i];
        parent->AddChild(child);
        parent = child;
    }
    return ops[0];
}

}  // namespace cypher

namespace lgraph {
std::map<lgraph::CompareOp, std::string> RangeFilter::_compare_name = {
    {lgraph::LBR_EQ, "="}, {lgraph::LBR_NEQ, "!="}, {lgraph::LBR_LT, "<"},
    {lgraph::LBR_GT, ">"}, {lgraph::LBR_LE, "<="},  {lgraph::LBR_GE, ">="},
};

static void BuildNestedSymbolTable(cypher::PatternGraph &pattern) {
    int symbol_idx = 0;
    auto &nested_sym_tab = pattern.symbol_table.symbols;
    for (auto &node : pattern.GetNodes()) {
        if (nested_sym_tab.find(node.Alias()) != nested_sym_tab.end()) continue;
        nested_sym_tab.emplace(
            node.Alias(),
            cypher::SymbolNode(symbol_idx++, cypher::SymbolNode::NODE, cypher::SymbolNode::LOCAL));
    }
    for (auto &relp : pattern.GetRelationships()) {
        if (nested_sym_tab.find(relp.Alias()) != nested_sym_tab.end()) continue;
        nested_sym_tab.emplace(relp.Alias(),
                               cypher::SymbolNode(symbol_idx++, cypher::SymbolNode::RELATIONSHIP,
                                                  cypher::SymbolNode::LOCAL));
    }
}

static void SetScopeNestedSymbolTable(const cypher::SymbolTable &sym_tab,
                                      cypher::PatternGraph &pattern) {
    auto &nested_sym_tab = pattern.symbol_table.symbols;
    for (auto &s : nested_sym_tab) {
        auto i = sym_tab.symbols.find(s.first);
        s.second.scope =
            i != sym_tab.symbols.end() ? cypher::SymbolNode::ARGUMENT : cypher::SymbolNode::LOCAL;
    }
}

void TestExists::BuildNestedExecutionPlan() {
    std::vector<cypher::NodeID> start_nodes;
    /* The argument nodes are specific, we add them into start nodes first.  */
    for (auto &node : nested_pattern_->GetNodes()) {
        node.Visited() = false;
        if (node.derivation_ == cypher::Node::ARGUMENT) start_nodes.emplace_back(node.ID());
    }
    if (start_nodes.empty()) CYPHER_TODO();
    for (auto &node : nested_pattern_->GetNodes()) {
        if (node.derivation_ != cypher::Node::CREATED) start_nodes.emplace_back(node.ID());
    }
    auto expand_streams = nested_pattern_->CollectExpandStreams(start_nodes, false);
    if (expand_streams.size() > 1) CYPHER_TODO();
    CYPHER_THROW_ASSERT(!expand_streams.empty() && !expand_streams[0].empty());
    std::vector<cypher::OpBase *> expand_ops;
    auto *ia = new cypher::ImmediateArgument(nested_pattern_.get());
    expand_ops.emplace_back(ia);
    for (auto &step : expand_streams[0]) {
        auto &start = nested_pattern_->GetNode(std::get<0>(step));
        auto &relp = nested_pattern_->GetRelationship(std::get<1>(step));
        auto &neighbor = nested_pattern_->GetNode(std::get<2>(step));
        if (relp.Empty() && neighbor.Empty()) {
            CYPHER_TODO();
        } else if (relp.VarLen()) {
            auto expand_op =
                new cypher::VarLenExpand(nested_pattern_.get(), &start, &neighbor, &relp);
            expand_ops.emplace_back(expand_op);
        } else {
            auto expand_op = new cypher::ExpandAll(nested_pattern_.get(), &start, &neighbor, &relp);
            expand_ops.emplace_back(expand_op);
        }
        // add property filter op
        if (neighbor.Prop().type != cypher::Property::NUL) {
            CYPHER_TODO();
        }
    }
    std::reverse(expand_ops.begin(), expand_ops.end());
    nested_plan_ = cypher::_SingleBranchConnect(expand_ops);
}

TestExists::TestExists(const cypher::SymbolTable &sym_tab,
                       const std::shared_ptr<cypher::PatternGraph> &pattern)
    : nested_pattern_(pattern) {
    _type = TEST_EXISTS_FILTER;
    /* build nested execution plan */
    BuildNestedSymbolTable(*nested_pattern_);
    /* set scope flag for nested symbol table */
    SetScopeNestedSymbolTable(sym_tab, *nested_pattern_);
    BuildNestedExecutionPlan();
#ifndef NDEBUG
    LOG_DEBUG() << "Nested execution plan:";
    std::string s;
    cypher::OpBase::DumpStream(nested_plan_, 4, false, s);
    LOG_DEBUG() << s;
#endif
}

TestExists::~TestExists() {
    // free ops
    cypher::OpBase::FreeStream(nested_plan_);
}

bool TestExists::DoFilter(cypher::RTContext *ctx, const cypher::Record &record) {
    if (!nested_plan_) {
        auto value = property_.Evaluate(ctx, record);
        return !value.IsNull();
    } else {
        /* e.g.
         * MATCH (a)->(b)->(c) RETURN exists((c)->()->(a))  */
        nested_plan_->Initialize(ctx);
        // immediate argument receive
        cypher::OpBase *op = nested_plan_;
        while (!op->children.empty()) op = op->children[0];
        CYPHER_THROW_ASSERT(op->type == cypher::OpType::ARGUMENT);
        auto ia = dynamic_cast<cypher::ImmediateArgument *>(op);
        ia->Receive(&record);
        bool passed = nested_plan_->Consume(ctx) == cypher::OpBase::OP_OK;
        return passed;
    }
}

bool StringFilter::DoFilter(cypher::RTContext *ctx, const cypher::Record &record) {
    auto left = lhs.Evaluate(ctx, record);
    auto right = rhs.Evaluate(ctx, record);
    if (!left.constant.IsString() || !right.constant.IsString()) {
        return false;
    }
    auto &lstr = left.constant.scalar.string();
    auto &rstr = right.constant.scalar.string();
    switch (compare_op) {
    case STARTS_WITH:
        return lstr.compare(0, rstr.size(), rstr) == 0;
    case ENDS_WITH:
        return lstr.size() >= rstr.size() &&
               lstr.compare(lstr.size() - rstr.size(), rstr.size(), rstr) == 0;
    case CONTAINS:
        return lstr.find(rstr) != std::string::npos;
    case REGEXP:
        return std::regex_match(lstr, std::regex(rstr));
    default:
        return false;
    }
}
}  // namespace lgraph
