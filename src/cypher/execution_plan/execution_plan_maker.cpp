/**
 * Copyright 2023 AntGroup CO., Ltd.
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

#include "cypher/execution_plan/execution_plan_maker.h"
#include "cypher/arithmetic/arithmetic_expression.h"
#include "cypher/execution_plan/clause_guard.h"
#include "cypher/execution_plan/ops/op_cartesian_product.h"
#include "cypher/execution_plan/ops/op_expand_all.h"
#include "cypher/execution_plan/ops/op_create.h"
#include "cypher/execution_plan/ops/op_delete.h"
#include "cypher/execution_plan/ops/op_merge.h"
#include "cypher/execution_plan/ops/op_remove.h"
#include "cypher/execution_plan/ops/op_set.h"
#include "cypher/execution_plan/ops/op_filter.h"
#include "cypher/execution_plan/ops/op_node_scan.h"
#include "cypher/execution_plan/ops/op_produce_results.h"
#include "cypher/execution_plan/ops/op_project.h"
#include "cypher/execution_plan/ops/op_inquery_call.h"
#include "cypher/execution_plan/ops/op_standalone_call.h"
#include "cypher/execution_plan/ops/op_argument.h"
#include "cypher/execution_plan/ops/op_apply.h"
#include "cypher/execution_plan/ops/op_limit.h"
#include "cypher/execution_plan/ops/op_skip.h"
#include "cypher/execution_plan/ops/op_aggregate.h"
#include "cypher/execution_plan/ops/op_optional.h"
#include "cypher/execution_plan/ops/op_unwind.h"
#include "cypher/execution_plan/ops/op_var_len_expand.h"
#include "cypher/execution_plan/ops/op_sort.h"
#include "cypher/execution_plan/ops/op_distinct.h"
#include "cypher/execution_plan/ops/op_union.h"
#include "cypher/filter/filter.h"
#include "cypher/utils/geax_util.h"
#include "geax-front-end/ast/Ast.h"

namespace cypher {

// Locates all "taps" (entry points) of root.
static void _StreamTaps(OpBase* root, std::vector<OpBase*>& taps) {
    if (!root->children.empty()) {
        for (auto child : root->children) _StreamTaps(child, taps);
    } else {
        taps.emplace_back(root);
    }
}

static inline void _StreamArgs(OpBase* root, std::vector<OpBase*>& args) {
    if (root->type == OpType::ARGUMENT) args.emplace_back(root);
    for (auto child : root->children) _StreamArgs(child, args);
}

static OpBase* _Connect(OpBase* lhs, OpBase* rhs, PatternGraph* pattern_graph) {
    std::vector<OpBase*> taps;
    _StreamTaps(rhs, taps);
    //
    // see issue #357: https://code.alipay.com/fma/tugraph-db/issues/357
    // see issue #188: https://code.alipay.com/fma/tugraph-db/issues/188
    //
    // # Example A
    // WITH 'a' as a UNWIND ['a', 'b'] as k RETURN a, k
    //
    // Execution Plan:
    // Produce Results
    //     Project [a,k]
    //         Cartesian Product
    //             Unwind [[a,b],k]
    //             Project [a]
    //
    // The execution plan before issue#357&#188 fix is as follows:
    //
    // Plan parts:
    // Project [a]
    // Produce Results
    //     Project [a,k]
    //         Unwind [[a,b],k]
    //
    // -----
    // # Example B
    // WITH [1, 3, 5, 7] as lst UNWIND [9]+lst AS x RETURN x, lst
    //
    // Execution Plan:
    // Produce Results
    //     Project [x,lst]
    //         Apply
    //             Unwind [([9],lst,+),x]
    //                 Argument [lst]
    //             Project [lst]
    //
    // The execution plan before issue#357&#188 fix is as follows:
    //
    // Execution Plan:
    // Produce Results
    //     Project [x,lst]
    //         Unwind [([9],lst,+),x]
    //             Project [lst]
    //
    if (taps.size() == 1 && !taps[0]->IsScan() && taps[0]->type != OpType::UNWIND) {
        /* Single tap, entry point isn't a SCAN operation, e.g.
         * MATCH (b) WITH b.v AS V RETURN V
         * MATCH (b) WITH b.v+1 AS V CREATE (n {v:V})
         * MATCH (b) WITH b RETURN b */
        if (taps[0]->type == OpType::PROJECT && lhs->type == OpType::PROJECT) {
            auto parent = taps[0]->parent;
            parent->RemoveChild(taps[0]);
            parent->AddChild(lhs);
            delete taps[0];
        } else {
            taps[0]->AddChild(lhs);
        }
    } else {
        /* Multiple taps or a single SCAN tap, e.g.
         * MATCH (b) WITH b.v AS V MATCH (c) return V,c
         * MATCH (b) WITH b.v AS V MATCH (c),(d) return c, V, d */
        OpBase* connection = nullptr;
        std::vector<OpBase*> args;  // argument operations
        _StreamArgs(rhs, args);
        if (!args.empty()) {
            if (args.size() > 1) CYPHER_TODO();
            connection = new Apply(dynamic_cast<Argument*>(args[0]), pattern_graph);
        } else {
            CYPHER_TODO();
            //connection = new CartesianProduct();
        }
        OpBase* stream_root = rhs;
        // find the *root* project/aggregate op
        while (!stream_root->IsStreamRoot()) {
            if (stream_root->children.size() != 1) CYPHER_TODO();
            stream_root = stream_root->children[0];
        }
        CYPHER_THROW_ASSERT(stream_root->IsStreamRoot());
        stream_root->AddChild(lhs);
        stream_root->PushInBetween(connection);
    }
    return rhs;
}

static bool CheckReturnElements(const std::vector<std::string>& last_ret,
                                const std::vector<std::string>& now_ret) {
    // some certain single query (e.g. MATCH(n) RETURN *) without no return items
    // cannot be unioned.
    if (last_ret.empty() || now_ret.empty()) {
        return false;
    }
    // if two queries return different size of items, cannot be unioned
    if (last_ret.size() != now_ret.size()) {
        return false;
    }
    for (int j = 0; j < (int)last_ret.size(); j++) {
        if (last_ret[j] != now_ret[j]) return false;
    }
    return true;
}

geax::frontend::GEAXErrorCode ExecutionPlanMaker::Build(geax::frontend::AstNode* astNode,
                                                        OpBase*& root, RTContext* ctx) {
    ctx_ = ctx;
    cur_types_.clear();
    cur_pattern_graph_ = -1;
    auto ret = std::any_cast<geax::frontend::GEAXErrorCode>(astNode->accept(*this));
    if (ret != geax::frontend::GEAXErrorCode::GEAX_SUCCEED) {
        return ret;
    }
    //LOG_DEBUG(_DumpPlanBeforeConnect(0, false));
    //LOG_DEBUG("Dump plan finished!");
    root = pattern_graph_root_[0];
    for (size_t i = 1; i < pattern_graph_root_.size(); i++) {
        if (should_connect_[i]) {
            root = _Connect(root, pattern_graph_root_[i], &(pattern_graphs_[i]));
        }
    }
    if (op_filter_ != nullptr) {
        NOT_SUPPORT();
    }
    return ret;
}

std::string ExecutionPlanMaker::_DumpPlanBeforeConnect(int , bool ) const {
    std::string s = "Execution Plan Before Connect: \n";
    for (size_t i = 0; i < pattern_graph_root_.size(); i++) {
        OpBase::DumpStream(pattern_graph_root_[i], 0, false, s);
    }
    return s;
}

void ExecutionPlanMaker::_UpdateStreamRoot(OpBase* new_root, OpBase*& root) {
    if (root) {
        /* The new root should have no parent, but may have children if we've constructed
         * a chain of traversals/scans. */
        CYPHER_THROW_ASSERT(!root->parent && !new_root->parent);
        /* Find the deepest child of the new root operation.
         * Currently, we can only follow the first child.
         * todo: This may be inadequate later. */
        OpBase* tail = new_root;
        while (!tail->children.empty()) {
            if (tail->children.size() > 1 || tail->type == OpType::CARTESIAN_PRODUCT ||
                tail->type == OpType::APPLY) {
                CYPHER_TODO();
            }
            tail = tail->children[0];
        }
        // Append the old root to the tail of the new root's chain.
        tail->AddChild(root);
    }
    root = new_root;
}

OpBase* ExecutionPlanMaker::_SingleBranchConnect(const std::vector<OpBase*>& ops) {
    if (ops.empty()) return nullptr;
    OpBase *child, *parent = ops[0];
    for (int i = 1; i < (int)ops.size(); i++) {
        child = ops[i];
        parent->AddChild(child);
        parent = child;
    }
    return ops[0];
}

void ExecutionPlanMaker::_AddScanOp(const SymbolTable* sym_tab, Node* node,
                                    std::vector<OpBase*>& ops, bool skip_arg_op) {
    auto it = sym_tab->symbols.find(node->Alias());
    if (it == sym_tab->symbols.end()) {
        THROW_CODE(CypherException, "Unknown variable: " + node->Alias());
    }
    if (it->second.scope == SymbolNode::ARGUMENT) {
        return;
    }
    ops.emplace_back(new NodeScan(ctx_->txn_, node, sym_tab));
    /*bool has_arg = false;
    for (auto& a : sym_tab->symbols) {
        if (a.second.scope == SymbolNode::ARGUMENT) {
            has_arg = true;
            break;
        }
    }
    bool has_variable = false;
    for (auto& prop : node->Props()) {
        if (prop.type == Property::VARIABLE) {
            has_variable = true;
            break;
        }
    }
    OpBase* scan_op = nullptr;
    if (has_arg || has_variable) {
        scan_op = new NodeScanDynamic(node, sym_tab);
    } else {
        scan_op = new NodeScan(node, sym_tab);
    }
    ops.emplace_back(scan_op);*/
}

std::any ExecutionPlanMaker::visit(geax::frontend::GraphPattern* node) {
    auto& path_patterns = node->pathPatterns();
    for (auto path_pattern : path_patterns) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(path_pattern);
    }
    if (node->matchMode().has_value()) {
        NOT_SUPPORT();
    }
    if (node->keep().has_value()) {
        NOT_SUPPORT();
    }
    if (node->where().has_value()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->where().value());
        auto expr_filter = std::make_shared<lgraph::GeaxExprFilter>(
            node->where().value(), pattern_graphs_[cur_pattern_graph_].symbol_table);
        auto op_filter = new OpFilter(expr_filter);
        if (pattern_graph_root_[cur_pattern_graph_]) {
            op_filter->AddChild(pattern_graph_root_[cur_pattern_graph_]);
        }
        pattern_graph_root_[cur_pattern_graph_] = op_filter;
    }
    if (node->yield().has_value()) {
        NOT_SUPPORT();
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::PathPattern* node) {
    auto& chains = node->chains();
    for (auto chain : chains) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(chain);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::PathChain* node) {
    auto& pattern_graph = pattern_graphs_[cur_pattern_graph_];
    auto head = node->head();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(head);
    std::vector<OpBase*> expand_ops;
    if (last_op_) {
        expand_ops.emplace_back(last_op_);
        last_op_ = nullptr;
    }
    // todo: ...
    ClauseGuard cg(node->type(), cur_types_);
    auto& tails = node->tails();
    // TODO(lingsu): generate the pattern graph
    // and select the starting Node and end Node according to the pattern graph
    for (auto [edge, end_node] : tails) {
        start_t_ = node_t_;
        //is_end_path_ = true;
        //equal_filter_.push_back(nullptr);
        //has_filter_per_level_.push_back(false);
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(end_node);
        //is_end_path_ = false;
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(edge);
        auto& start = pattern_graph.GetNode(start_t_->Alias());
        auto& relp = pattern_graph.GetRelationship(relp_t_->Alias());
        auto& end = pattern_graph.GetNode(node_t_->Alias());
        OpBase* expand_op;
        if (relp.VarLen()) {
            //NOT_SUPPORT();
            expand_op = new VarLenExpand(&pattern_graph, &start, &end, &relp);
        } else {
            expand_op = new ExpandAll(&pattern_graph, &start, &end, &relp);
        }
        expand_ops.emplace_back(expand_op);
        //if (has_filter_per_level_[filter_level_]) {
        //    NOT_SUPPORT();
            /*OpFilter* filter = new OpFilter(std::make_shared<lgraph::GeaxExprFilter>(
                    equal_filter_[filter_level_], pattern_graphs_[cur_pattern_graph_].symbol_table));
            expand_ops.push_back(filter);*/
        //}
        if (op_filter_ != nullptr) {
            NOT_SUPPORT();
            /*expand_ops.push_back(op_filter_);
            op_filter_ = nullptr;*/
        }
        //++filter_level_;
    }
    std::reverse(expand_ops.begin(), expand_ops.end());
    // For the exists pattern query, SemiAllpy is simulated by combining Limit Optional and
    // ExpandAll operators. e.g MATCH (n {name:'Rachel Kempson'}),(m:Person) RETURN
    // exists((n)-[:MARRIED]->(m)) The resulting execution plan is as follows Produce Results
    //    Project [exists((n)-[:MARRIED]->(m))]
    //      Apply
    //          Limit [1]
    //              Optional
    //                  Expand(Into) [n --> m ]
    //                      Argument [n,m]
    //          Cartesian Product
    //              Node Index Seek [n]  IN []
    //              Node By Label Scan [m:Person]
    if (ClauseGuard::InClause(geax::frontend::AstNodeType::kExists, cur_types_)) {
        NOT_SUPPORT();
        /*auto& sym_tab = pattern_graphs_[cur_pattern_graph_].symbol_table;
        std::vector<OpBase*> rewriter;
        if (ClauseGuard::InClause(geax::frontend::AstNodeType::kPrimitiveResultStatement,
                                  cur_types_)) {
            auto limit = new Limit(1);
            rewriter.push_back(limit);
            auto optional = new Optional();
            rewriter.push_back(optional);
        }
        rewriter.insert(rewriter.end(), expand_ops.begin(), expand_ops.end());
        auto argument = new Argument(&sym_tab);
        rewriter.push_back(argument);
        auto op = _SingleBranchConnect(rewriter);
        pattern_graph_root_[cur_pattern_graph_] = op;
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;*/
    }
    // The handling here is for the Match Clause, other situations need to be considered.
    // The current design of the Visitor results in different logic for handling PathChain in
    // different Clauses (Match, Create, and even Exists), which makes the code overly complex and
    // needs refactoring.
    if (!ClauseGuard::InClause(geax::frontend::AstNodeType::kMatchStatement, cur_types_)) {
        return geax::frontend::GEAXErrorCode::GEAX_ERROR;
    }

    if (auto op = _SingleBranchConnect(expand_ops)) {
        if (pattern_graph_root_[cur_pattern_graph_]) {
            _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
        } else {
            pattern_graph_root_[cur_pattern_graph_] = op;
        }

        // Handle the case where multiple PathChains are connected, with special handling
        // for cases where multiple PathChains are mutually independent.
        // Considering that _UpdateStreamRoot connections are more complex and NodeScan
        // cannot be connected directly, this determination requires additional processing.
        /*bool is_pathchain_independent = true;
        if (is_pathchain_independent) {
            if (pattern_graph_root_[cur_pattern_graph_] &&
                (pattern_graph_root_[cur_pattern_graph_]->type != OpType::ARGUMENT &&
                 pattern_graph_root_[cur_pattern_graph_]->type != OpType::UNWIND)) {
                //NOT_SUPPORT();
                auto connection = new CartesianProduct();
                connection->AddChild(pattern_graph_root_[cur_pattern_graph_]);
                connection->AddChild(op);
                pattern_graph_root_[cur_pattern_graph_] = connection;
            } else if (pattern_graph_root_[cur_pattern_graph_] &&
                       pattern_graph_root_[cur_pattern_graph_]->type == OpType::UNWIND &&
                       (*expand_ops.rbegin())->IsScan()) {
                auto rbegin = *expand_ops.rbegin();
                auto connection = new CartesianProduct();
                connection->AddChild(pattern_graph_root_[cur_pattern_graph_]);
                connection->AddChild(rbegin);
                pattern_graph_root_[cur_pattern_graph_] = connection;
            } else if (pattern_graph_root_[cur_pattern_graph_]) {
                _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
            } else {
                pattern_graph_root_[cur_pattern_graph_] = op;
            }
        } else {
            // For the associated case, currently handled through ast node rewriter, but this has
            // performance issues. In the future, MultiMatch will be rewritten.
            CYPHER_TODO();
        }*/
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::Node* node) {
    ClauseGuard cg(node->type(), cur_types_);
    node_t_ = std::make_shared<Node>();
    if (ClauseGuard::InClause(geax::frontend::AstNodeType::kMatchStatement, cur_types_)) {
        node_t_->derivation_ = Node::Derivation::MATCHED;
    }
    auto filler = node->filler();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(filler);
    auto& pattern_graph = pattern_graphs_[cur_pattern_graph_];
    if (!ClauseGuard::InClause(geax::frontend::AstNodeType::kPathChain, cur_types_)) {
        std::vector<OpBase*> expand_ops;
        auto& start = pattern_graph.GetNode(node_t_->Alias());
        _AddScanOp(&pattern_graph.symbol_table, &start, expand_ops, false);
        if (op_filter_ != nullptr) {
            NOT_SUPPORT();
            //expand_ops.push_back(op_filter_);
            //op_filter_ = nullptr;
        }
        std::reverse(expand_ops.begin(), expand_ops.end());
        if (auto op = _SingleBranchConnect(expand_ops)) {
            // Do not connect op to pattern_graph_root_[cur_pattern_graph_] for now,
            // use last_op_ to store it.
            last_op_ = op;
        }
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::Edge* node) {
    ClauseGuard cg(node->type(), cur_types_);
    relp_t_ = std::make_shared<Relationship>();
    auto filler = node->filler();
    if (filler) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(filler);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::ElementFiller* node) {
    // variable
    auto& variable = node->v();
    if (!variable.has_value()) {
        NOT_SUPPORT();
    }
    if (ClauseGuard::InClause(geax::frontend::AstNodeType::kNode, cur_types_)) {
        node_t_->SetAlias(variable.value());
    } else if (ClauseGuard::InClause(geax::frontend::AstNodeType::kEdge, cur_types_)) {
        relp_t_->SetAlias(variable.value());
    }
    // label
    /*auto& label = node->label();
    if (label.has_value()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(label.value());
    }*/
    // predicates
    /*auto& predicates = node->predicates();
    if (predicates.size() > 1) {
        NOT_SUPPORT();
    }
    for (auto predicate : predicates) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(predicate);
    }*/

    /*if (is_end_path_ && has_filter_per_level_[filter_level_]) {
        auto expr = equal_filter_[filter_level_];
        auto field = (geax::frontend::GetField*)expr->left();
        auto ref = objAlloc_.allocate<geax::frontend::Ref>();
        auto name = variable.value();
        ref->setName(std::move(name));
        field->setExpr(ref);
    }*/
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::WhereClause* ) {
    NOT_SUPPORT();
}

std::any ExecutionPlanMaker::visit(geax::frontend::OrderByField* ) {
    NOT_SUPPORT();
}

std::any ExecutionPlanMaker::visit(geax::frontend::PathModePrefix* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::PathSearchPrefix* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SingleLabel* node) {
    if (ClauseGuard::InClause(geax::frontend::AstNodeType::kNode, cur_types_)) {
        node_t_->SetLabel(node->label());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    } else if ((ClauseGuard::InClause(geax::frontend::AstNodeType::kEdge, cur_types_))) {
        relp_t_->AddType(node->label());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    } else if ((ClauseGuard::InClause(geax::frontend::AstNodeType::kIsLabeled, cur_types_))) {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }
    NOT_SUPPORT();
}

std::any ExecutionPlanMaker::visit(geax::frontend::LabelOr* ) {
    NOT_SUPPORT();
}

std::any ExecutionPlanMaker::visit(geax::frontend::LabelAnd* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::LabelNot* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::PropStruct* node) {
    NOT_SUPPORT();
}

std::any ExecutionPlanMaker::visit(geax::frontend::YieldField* node) {
    if (node->predicate()) {
        auto expr_filter = std::make_shared<lgraph::GeaxExprFilter>(
            node->predicate(), pattern_graphs_[cur_pattern_graph_].symbol_table);
        auto op_filter = new OpFilter(expr_filter);
        op_filter->AddChild(pattern_graph_root_[cur_pattern_graph_]);
        pattern_graph_root_[cur_pattern_graph_] = op_filter;
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::TableFunctionClause* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ReadConsistency* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::AllowAnonymousTable* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::OpConcurrent* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::EdgeOnJoin* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetAllProperties* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::UpdateProperties* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetLabel* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetSingleProperty* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetSchemaClause* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetGraphClause* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetTimeZoneClause* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetParamClause* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ResetAll* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ResetSchema* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ResetTimeZone* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ResetGraph* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ResetParam* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::GetField* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::TupleGet* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Not* node) {
    if (node->expr()) ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::Neg* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::Tilde* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VSome* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BEqual* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BNotEqual* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BGreaterThan* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BNotSmallerThan* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BSmallerThan* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BNotGreaterThan* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BSafeEqual* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BAdd* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BSub* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BDiv* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BMul* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BMod* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BSquare* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BAnd* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BOr* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BXor* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BBitAnd* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BBitOr* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BBitXor* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BBitLeftShift* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BBitRightShift* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BConcat* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BIndex* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BLike* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BIn* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::If* ) {
    NOT_SUPPORT();
}

std::any ExecutionPlanMaker::visit(geax::frontend::Function* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::Case* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::Cast* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::MatchCase* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::AggFunc* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BAggFunc* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::MultiCount* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Windowing* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::MkList* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::MkMap* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::MkRecord* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::MkSet* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::MkTuple* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VBool* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::VInt* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::VDouble* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::VString* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::VDate* ) {
    NOT_SUPPORT();
}

std::any ExecutionPlanMaker::visit(geax::frontend::VDatetime* ) {
    NOT_SUPPORT();
}

std::any ExecutionPlanMaker::visit(geax::frontend::VDuration* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VTime* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VNull* ) {
    NOT_SUPPORT();
}

std::any ExecutionPlanMaker::visit(geax::frontend::VNone* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Ref* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::IsNull* ) {
    NOT_SUPPORT();
}

std::any ExecutionPlanMaker::visit(geax::frontend::IsDirected* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::IsNormalized* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::IsSourceOf* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::IsDestinationOf* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::IsLabeled* node) {
    ClauseGuard cg(node->type(), cur_types_);
    if (node->expr()) ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
    if (node->labelTree()) ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->labelTree());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::Same* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::AllDifferent* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Exists* ) {
    NOT_SUPPORT();
}

std::any ExecutionPlanMaker::visit(geax::frontend::ExplainActivity* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SessionActivity* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::TransactionActivity* ) {
    NOT_SUPPORT();
}

std::any ExecutionPlanMaker::visit(geax::frontend::FullTransaction* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::NormalTransaction* node) {
    if (node->endTransaction().has_value()) {
        NOT_SUPPORT();
    }
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->query());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::StartTransaction* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::CommitTransaction* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::RollBackTransaction* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SessionSet* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SessionReset* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ProcedureBody* node) {
    pattern_graph_size_ = pattern_graphs_.size();
    pattern_graph_root_.resize(pattern_graph_size_, nullptr);
    should_connect_.resize(pattern_graph_size_, true);
    cur_pattern_graph_ = -1;
    for (size_t i = 0; i < node->statements().size(); i++) {
        cur_pattern_graph_ += 1;
        // Build Argument
        auto& sym_tab = pattern_graphs_[cur_pattern_graph_].symbol_table;
        for (auto& symbol : sym_tab.symbols) {
            if (symbol.second.scope == SymbolNode::ARGUMENT) {
                auto argument = new Argument(&sym_tab);
                _UpdateStreamRoot(argument, pattern_graph_root_[i]);
                break;
            }
        }
        // Build Statement
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->statements()[i]);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::SchemaFromPath* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BindingValue* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BindingGraph* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BindingTable* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BindingTableInnerQuery* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BindingTableInnerExpr* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::StatementWithYield* node) {
    auto stmt = node->statement();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(stmt);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::QueryStatement* node) {
    auto join_query = node->joinQuery();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(join_query);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::StandaloneCallStatement* node) {
    auto stmt = node->procedureStatement();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(stmt);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::JoinQueryExpression* node) {
    auto head = node->head();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(head);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::JoinRightPart* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::CompositeQueryStatement* node) {
    auto head = node->head();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(head);
    if (!node->body().empty()) {
        auto op_union = new Union();
        op_union->AddChild(pattern_graph_root_[cur_pattern_graph_]);
        auto op_produce = new ProduceResults();
        op_produce->AddChild(op_union);
        pattern_graph_root_[cur_pattern_graph_] = op_produce;
        for (auto statement : node->body()) {
            cur_pattern_graph_ += 1;
            should_connect_[cur_pattern_graph_] = false;
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(std::get<1>(statement));
            op_union->AddChild(pattern_graph_root_[cur_pattern_graph_]->children[0]);
        }
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::AmbientLinearQueryStatement* node) {
    auto& query_stmts = node->queryStatements();
    for (auto query_stmt : query_stmts) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(query_stmt);
    }
    auto projection = node->resultStatement();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(projection);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::SelectStatement* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::FocusedQueryStatement* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::FocusedResultStatement* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::MatchStatement* node) {
    ClauseGuard cg(node->type(), cur_types_);
    auto graph = node->graphPattern();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(graph);
    if (node->statementMode().has_value() &&
        node->statementMode().value() == geax::frontend::StatementMode::kOptional) {
        _UpdateStreamRoot(new Optional(), pattern_graph_root_[cur_pattern_graph_]);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::FilterStatement* node) {
    auto expr_filter = std::make_shared<lgraph::GeaxExprFilter>(
        node->predicate(), pattern_graphs_[cur_pattern_graph_].symbol_table);
    auto op_filter = new OpFilter(expr_filter);
    if (pattern_graph_root_[cur_pattern_graph_]) {
        op_filter->AddChild(pattern_graph_root_[cur_pattern_graph_]);
    }
    pattern_graph_root_[cur_pattern_graph_] = op_filter;
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::CallQueryStatement* node) {
    auto procedureStatement = node->procedureStatement();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(procedureStatement);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::CallProcedureStatement* node) {
    auto procedure = node->procedureCall();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(procedure);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::InlineProcedureCall* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::NamedProcedureCall* node) {
    std::string name = std::get<std::string>(node->name());
    std::vector<OpBase*> expand_ops;
    auto standalone_call_op = new GqlStandaloneCall(name, node->args(), node->yield(),
                                      pattern_graphs_[cur_pattern_graph_].symbol_table);
    expand_ops.emplace_back(standalone_call_op);
    auto produce = new ProduceResults();
    expand_ops.emplace_back(produce);
    std::reverse(expand_ops.begin(), expand_ops.end());
    if (auto op = _SingleBranchConnect(expand_ops)) {
        _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
    }

    result_info_.header.colums.clear();
    auto p = global_ptable.GetProcedure(name);
    assert(p);
    auto& yield = node->yield();
    assert(yield);
    auto& result = p->signature.result_list;
    auto& items = yield.value()->items();
    for (auto& item : items) {
        for (auto& r : result) {
            if (std::get<0>(item) == r.name) {
                result_info_.header.colums.emplace_back(r.name, r.name, false);
                break;
            }
        }
    }

    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::ForStatement* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::PrimitiveResultStatement* node) {
    auto& items = node->items();
    std::vector<OpBase*> ops;
    std::vector<std::tuple<ArithExprNode, std::string>> arith_items;
    if (!should_connect_[cur_pattern_graph_]) {
        // NOT_SUPPORT();
        std::vector<std::string> last_ret, now_ret;
        for (auto& col : result_info_.header.colums) {
            last_ret.push_back(col.alias);
        }
        for (auto& item : items) {
            now_ret.push_back(std::get<0>(item));
        }
        std::sort(last_ret.begin(), last_ret.end());
        std::sort(now_ret.begin(), now_ret.end());
        if (!CheckReturnElements(last_ret, now_ret)) {
            THROW_CODE(CypherException,
                    "All sub queries in an UNION must have the same column names.");
        }
    }
    //result_info_.header.clear();
    result_info_.header.colums.clear();
    ClauseGuard cg(node->type(), cur_types_);
    for (auto& item : items) {
        auto expr = std::get<1>(item);
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(expr);
        ArithExprNode ae(expr, pattern_graphs_[cur_pattern_graph_].symbol_table);
        auto alias = std::get<0>(item);
        arith_items.emplace_back(ae, alias);
        AstAggExprDetector agg_expr_detector(expr);
        bool has_aggregation = false;
        if (agg_expr_detector.Validate()) {
            has_aggregation = agg_expr_detector.HasValidAggFunc();
        } else {
            error_msg_ = std::any_cast<std::string>(agg_expr_detector.reportError());
            return geax::frontend::GEAXErrorCode::GEAX_ERROR;
        }
        if (std::get<2>(item)) {
            continue;
        }
        // build header
        if (expr->type() == geax::frontend::AstNodeType::kRef) {
            std::string& name = ((geax::frontend::Ref*)expr)->name();
            auto it = pattern_graphs_[cur_pattern_graph_].symbol_table.symbols.find(name);
            if (it == pattern_graphs_[cur_pattern_graph_].symbol_table.symbols.end()) {
                error_msg_ = "Unknown variable: " + name;
                return geax::frontend::GEAXErrorCode::GEAX_ERROR;
            }
            result_info_.header.colums.emplace_back(name, alias, has_aggregation);
        } else {
            result_info_.header.colums.emplace_back(alias, alias, has_aggregation);
        }
    }
    if (cur_pattern_graph_ == pattern_graph_size_ - 1) {
        auto result = new ProduceResults();
        ops.push_back(result);
    }
    if (node->limit().has_value()) {
        ops.emplace_back(new Limit(std::get<0>(node->limit().value())));
    }
    if (node->offset().has_value()) {
        ops.emplace_back(new Skip(std::get<0>(node->offset().value())));
    }
    std::vector<std::pair<int, bool>> order_by_items;
    // TODO(lingsu): check whether the orderby key appears in the symbol table or return items,
    // and GetField type should complement the return items
    for (auto order_by_field : node->orderBys()) {
        for (size_t i = 0; i < items.size(); ++i) {
            if (auto order_by_ref = dynamic_cast<geax::frontend::Ref*>(order_by_field->field())) {
                auto field_name = std::get<0>(items[i]);
                if (order_by_ref->name() == field_name) {
                    order_by_items.emplace_back(std::make_pair(i, !order_by_field->order()));
                    break;
                }
            } else if (auto field =
                    dynamic_cast<geax::frontend::GetField*>(order_by_field->field())) {
                if (auto order_by_ref = dynamic_cast<geax::frontend::Ref*>(field->expr())) {
                    auto field_name = std::get<0>(items[i]);
                    std::string field_name_str = order_by_ref->name();
                    field_name_str.append(".");
                    field_name_str.append(field->fieldName());
                    if (field_name_str == field_name) {
                        order_by_items.emplace_back(std::make_pair(i, !order_by_field->order()));
                        break;
                    }
                } else {
                    NOT_SUPPORT();
                }
            } else {
                NOT_SUPPORT();
            }
        }
    }
    if (order_by_items.size() != node->orderBys().size()) {
        THROW_CODE(InputError, "Unknown order by field");
    }
    if (!order_by_items.empty()) {
        ops.emplace_back(new Sort(
                order_by_items, node->offset().has_value() ? std::get<0>(node->offset().value()) : -1,
                node->limit().has_value() ? std::get<0>(node->limit().value()) : -1));
    }
    // AGGREGATE
    bool has_aggregation = false;
    {
        std::vector<ArithExprNode> aggregated_expressions;
        std::vector<std::string> aggr_item_names;
        std::vector<ArithExprNode> noneaggregated_expressions;
        std::vector<std::string> noneaggr_item_names;
        std::vector<std::string> item_names;
        std::vector<ArithExprNode> arith_items;
        for (auto& item : items) {
            ArithExprNode ae(std::get<1>(item), pattern_graphs_[cur_pattern_graph_].symbol_table);
            auto alias = std::get<0>(item);
            if (ae.ContainsAggregation()) {
                aggregated_expressions.push_back(ae);
                aggr_item_names.push_back(alias);
            } else {
                noneaggregated_expressions.push_back(ae);
                noneaggr_item_names.push_back(alias);
            }
            item_names.push_back(alias);
        }
        has_aggregation = !aggregated_expressions.empty();
        if (has_aggregation) {
            ops.emplace_back(new Aggregate(
                    aggregated_expressions, aggr_item_names, noneaggregated_expressions,
                    noneaggr_item_names, item_names, &pattern_graphs_[cur_pattern_graph_].symbol_table,
                    result_info_.header));
        }
    }
    if (!has_aggregation) {
        if (node->distinct()) {
            ops.emplace_back(new Distinct());
        }
        ops.emplace_back(
                new Project(arith_items, &pattern_graphs_[cur_pattern_graph_].symbol_table));
    }
    if (auto op = _SingleBranchConnect(ops)) {
        _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::CatalogModifyStatement* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::LinearDataModifyingStatement* node) {
    auto& queryStatements = node->queryStatements();
    for (auto queryStatement : queryStatements) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(queryStatement);
    }
    auto& modifyStatements_ = node->modifyStatements();
    for (auto modifyStatement : modifyStatements_) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(modifyStatement);
    }
    if (node->resultStatement().has_value()) {
        auto resultStatement = node->resultStatement().value();
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(resultStatement);
    } else {
        auto result = new ProduceResults();
        _UpdateStreamRoot(result, pattern_graph_root_[cur_pattern_graph_]);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::InsertStatement* node) {
    auto& pattern_graph = pattern_graphs_[cur_pattern_graph_];
    auto op = new cypher::OpGqlCreate(node->paths(), &pattern_graph);
    _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
    result_info_.header.colums.clear();
    result_info_.header.colums.emplace_back(
        "<SUMMARY>", "", false);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::ReplaceStatement* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetStatement* node) {
    auto& pattern_graph = pattern_graphs_[cur_pattern_graph_];
    auto op = new OpGqlSet(node->items(), &pattern_graph);
    _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
    result_info_.header.colums.clear();
    result_info_.header.colums.emplace_back(
        "<SUMMARY>", "", false);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::DeleteStatement* node) {
    auto op = new OpGqlDelete(node->items());
    _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
    result_info_.header.colums.clear();
    result_info_.header.colums.emplace_back(
        "<SUMMARY>", "", false);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::RemoveStatement* node) {
    auto& pattern_graph = pattern_graphs_[cur_pattern_graph_];
    for (auto item : node->items()) {
        if (typeid(*item) == typeid(geax::frontend::RemoveSingleProperty)) {
            geax::frontend::RemoveSingleProperty* remove;
            checkedCast(item, remove);
            if (pattern_graphs_[cur_pattern_graph_].symbol_table.symbols.find(
                    remove->v()) == pattern_graphs_[cur_pattern_graph_]
                                        .symbol_table.symbols.end()) {
                THROW_CODE(InputError, "Variable `{}` not defined",
                           remove->v());
            }
        } else if (typeid(*item) == typeid(geax::frontend::RemoveLabel)) {
            geax::frontend::RemoveLabel* remove;
            checkedCast(item, remove);
            if (pattern_graphs_[cur_pattern_graph_].symbol_table.symbols.find(
                    remove->v()) == pattern_graphs_[cur_pattern_graph_]
                                        .symbol_table.symbols.end()) {
                THROW_CODE(InputError, "Variable `{}` not defined",
                           remove->v());
            }
        }
    }
    auto op = new OpGqlRemove(node->items(), &pattern_graph);
    _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
    result_info_.header.colums.clear();
    result_info_.header.colums.emplace_back(
        "<SUMMARY>", "", false);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::MergeStatement* node) {
    auto& pattern_graph = pattern_graphs_[cur_pattern_graph_];
    auto op =
        new OpGqlMerge(node->onMatch(), node->onCreate(), node->pathPattern(), &pattern_graph);
    _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
    result_info_.header.colums.clear();
    result_info_.header.colums.emplace_back(
        "<SUMMARY>", "", false);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::OtherWise* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Union* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Except* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Intersect* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Param* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ShowProcessListStatement* ) {
    NOT_SUPPORT();
}

std::any ExecutionPlanMaker::visit(geax::frontend::KillStatement* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ManagerStatement* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::UnwindStatement* node) {
    ArithExprNode exp(node->list(), pattern_graphs_[cur_pattern_graph_].symbol_table);
    auto unwind =
        new Unwind(exp, node->variable(), &pattern_graphs_[cur_pattern_graph_].symbol_table);
    _UpdateStreamRoot(unwind, pattern_graph_root_[cur_pattern_graph_]);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::InQueryProcedureCall* node) {
    std::string name = std::get<std::string>(node->name());
    std::vector<OpBase*> expand_ops;
    auto op =
        new GqlInQueryCall(name, node->args(), node->yield(), &pattern_graphs_[cur_pattern_graph_]);
    expand_ops.emplace_back(op);
    if (auto op = _SingleBranchConnect(expand_ops)) {
        _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
    }
    if (node->yield().has_value()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->yield().value());
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::DummyNode* ) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::reportError() { return error_msg_; }

std::any ExecutionPlanMaker::visit(geax::frontend::RemoveSingleProperty* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::RemoveLabel* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::ListComprehension* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

}  // namespace cypher
