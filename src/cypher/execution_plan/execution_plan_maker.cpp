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

#include "cypher/utils/geax_util.h"
#include "cypher/execution_plan/clause_guard.h"
#include "cypher/execution_plan/ops/ops.h"
#include "cypher/execution_plan/execution_plan_maker.h"

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
            connection = new CartesianProduct();
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
                                                        OpBase*& root) {
    cur_types_.clear();
    cur_pattern_graph_ = -1;
    auto ret = std::any_cast<geax::frontend::GEAXErrorCode>(astNode->accept(*this));
    if (ret != geax::frontend::GEAXErrorCode::GEAX_SUCCEED) {
        return ret;
    }
    _DumpPlanBeforeConnect(0, false);
    LOG_DEBUG() << "Dump plan finished!";
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

std::string ExecutionPlanMaker::_DumpPlanBeforeConnect(int indent, bool statistics) const {
    std::string s = "Execution Plan Before Connect: \n";
    for (size_t i = 0; i < pattern_graph_root_.size(); i++) {
        if (should_connect_[i]) {
            OpBase::DumpStream(pattern_graph_root_[i], 0, false, s);
        }
    }
    LOG_DEBUG() << s;
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
        throw lgraph::CypherException("Unknown variable: " + node->Alias() + " at " +
                                      std::string(__FILE__) + ":" + std::to_string(__LINE__));
    }
    auto& pf = node->Prop();
    OpBase* scan_op = nullptr;
    bool has_arg = false;
    for (auto& a : sym_tab->symbols) {
        if (a.second.scope == SymbolNode::ARGUMENT) {
            has_arg = true;
            break;
        }
    }
    if (!has_arg) {
        // no argument type in symbol table
        if (pf.type == Property::VALUE || pf.type == Property::PARAMETER) {
            /* use index when possible. weak index lookup if label absent */
            scan_op = new NodeIndexSeek(node, sym_tab);
        } else if (pf.type == Property::VARIABLE) {
            /* UNWIND [1,2] AS x MATCH (n {id:x}) RETURN n */
            scan_op = new NodeIndexSeekDynamic(node, sym_tab);
            if (sym_tab->symbols.find(pf.value_alias) == sym_tab->symbols.end()) {
                throw lgraph::CypherException("Unknown variable: " + pf.value_alias + " at " +
                                              std::string(__FILE__) + ":" +
                                              std::to_string(__LINE__));
            }
            // if (!part.unwind_clause) CYPHER_TODO();
        }
        if (!scan_op) {
            if (!node->Label().empty()) {
                /* labeled */
                scan_op = new NodeByLabelScan(node, sym_tab);
            } else {
                /* Node not labeled, no other option but a full scan. */
                scan_op = new AllNodeScan(node, sym_tab);
            }
        }
        ops.emplace_back(scan_op);
    } else {
        // has argument type in symbol table
        if (it->second.scope == SymbolNode::ARGUMENT) {
            if (skip_arg_op) return;
        } else if (pf.type == Property::VALUE || pf.type == Property::PARAMETER) {
            /* use index when possible. weak index lookup if label absent */
            scan_op = new NodeIndexSeekDynamic(node, sym_tab);
        } else if (pf.type == Property::VARIABLE) {
            scan_op = new NodeIndexSeekDynamic(node, sym_tab);
            /* WITH 'sth' AS x MATCH (n {name:x}) RETURN n  */
            auto i = sym_tab->symbols.find(pf.value_alias);
            if (i == sym_tab->symbols.end()) {
                throw lgraph::CypherException("Unknown variable: " + pf.value_alias + " at " +
                                              std::string(__FILE__) + ":" +
                                              std::to_string(__LINE__));
            }
        }
        if (!scan_op && it->second.scope != SymbolNode::ARGUMENT) {
            if (!node->Label().empty()) {
                scan_op = new NodeByLabelScanDynamic(node, sym_tab);
            } else {
                /* Node not labeled, no other option but a full scan. */
                scan_op = new AllNodeScanDynamic(node, sym_tab);
            }
        }
        if (scan_op) {
            ops.emplace_back(scan_op);
        }
    }
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
        is_end_path_ = true;
        equal_filter_.push_back(nullptr);
        has_filter_per_level_.push_back(false);
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(end_node);
        is_end_path_ = false;
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(edge);
        auto& start = pattern_graph.GetNode(start_t_->Alias());
        auto& relp = pattern_graph.GetRelationship(relp_t_->Alias());
        auto& end = pattern_graph.GetNode(node_t_->Alias());
        OpBase* expand_op;
        LOG_DEBUG() << relp.MinHop() << " " << relp.MaxHop();
        if (relp.VarLen()) {
            expand_op = new VarLenExpand(&pattern_graph, &start, &end, &relp);
        } else {
            expand_op = new ExpandAll(&pattern_graph, &start, &end, &relp);
        }
        expand_ops.emplace_back(expand_op);
        if (has_filter_per_level_[filter_level_]) {
            OpFilter* filter = new OpFilter(std::make_shared<lgraph::GeaxExprFilter>(
                equal_filter_[filter_level_], pattern_graphs_[cur_pattern_graph_].symbol_table));
            expand_ops.push_back(filter);
        }
        if (op_filter_ != nullptr) {
            expand_ops.push_back(op_filter_);
            op_filter_ = nullptr;
        }
        ++filter_level_;
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
        auto& sym_tab = pattern_graphs_[cur_pattern_graph_].symbol_table;
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
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }
    // The handling here is for the Match Clause, other situations need to be considered.
    // The current design of the Visitor results in different logic for handling PathChain in
    // different Clauses (Match, Create, and even Exists), which makes the code overly complex and
    // needs refactoring.
    if (!ClauseGuard::InClause(geax::frontend::AstNodeType::kMatchStatement, cur_types_)) {
        return geax::frontend::GEAXErrorCode::GEAX_ERROR;
    }

    if (auto op = _SingleBranchConnect(expand_ops)) {
        // Handle the case where multiple PathChains are connected, with special handling
        // for cases where multiple PathChains are mutually independent.
        // Considering that _UpdateStreamRoot connections are more complex and AllNodeScan
        // cannot be connected directly, this determination requires additional processing.
        bool is_pathchain_independent = true;
        if (is_pathchain_independent) {
            if (pattern_graph_root_[cur_pattern_graph_] &&
                (pattern_graph_root_[cur_pattern_graph_]->type != OpType::ARGUMENT &&
                 pattern_graph_root_[cur_pattern_graph_]->type != OpType::UNWIND)) {
                auto connection = new CartesianProduct();
                connection->AddChild(pattern_graph_root_[cur_pattern_graph_]);
                connection->AddChild(op);
                pattern_graph_root_[cur_pattern_graph_] = connection;
            } else if (pattern_graph_root_[cur_pattern_graph_] &&
                       pattern_graph_root_[cur_pattern_graph_]->type == OpType::UNWIND &&
                       (*expand_ops.rbegin())->IsScan()) {
                auto op = *expand_ops.rbegin();
                auto connection = new CartesianProduct();
                connection->AddChild(pattern_graph_root_[cur_pattern_graph_]);
                connection->AddChild(op);
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
        }
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
            expand_ops.push_back(op_filter_);
            op_filter_ = nullptr;
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
    auto& label = node->label();
    if (label.has_value()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(label.value());
    }
    // predicates
    auto& predicates = node->predicates();
    if (predicates.size() > 1) {
        NOT_SUPPORT();
    }
    for (auto predicate : predicates) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(predicate);
    }

    if (is_end_path_ && has_filter_per_level_[filter_level_]) {
        auto expr = equal_filter_[filter_level_];
        auto field = (geax::frontend::GetField*)expr->left();
        auto ref = objAlloc_.allocate<geax::frontend::Ref>();
        auto name = variable.value();
        ref->setName(std::move(name));
        field->setExpr(ref);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::WhereClause* node) {
    if (op_filter_ != nullptr) {
        NOT_SUPPORT();
    }
    op_filter_ = new OpFilter(std::make_shared<lgraph::GeaxExprFilter>(
        node->predicate(), pattern_graphs_[cur_pattern_graph_].symbol_table));
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::OrderByField* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::PathModePrefix* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::PathSearchPrefix* node) { NOT_SUPPORT(); }

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

std::any ExecutionPlanMaker::visit(geax::frontend::LabelOr* node) {
    if (node->left()) ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    if (node->right()) ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::LabelAnd* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::LabelNot* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::PropStruct* node) {
    auto& properties = node->properties();
    if (properties.size() != 1) {
        NOT_SUPPORT();
    }
    auto property = node->properties()[0];
    Property p;
    auto [key, value] = property;
    p.field = key;

    geax::frontend::BEqual* expr = nullptr;
    geax::frontend::GetField* field = nullptr;
    if (is_end_path_) {
        expr = objAlloc_.allocate<geax::frontend::BEqual>();
        field = objAlloc_.allocate<geax::frontend::GetField>();
        std::string fieldName = key;
        field->setFieldName(std::move(fieldName));
        expr->setLeft(field);
        has_filter_per_level_[filter_level_] = true;
        equal_filter_[filter_level_] = expr;
    }
    if (value->type() == geax::frontend::AstNodeType::kVString) {
        p.type = Property::VALUE;
        p.value = lgraph::FieldData(((geax::frontend::VString*)value)->val());
        if (is_end_path_) {
            auto right = objAlloc_.allocate<geax::frontend::VString>();
            std::string str = ((geax::frontend::VString*)value)->val();
            right->setVal(std::move(str));
            expr->setRight(right);
        }
    } else if (value->type() == geax::frontend::AstNodeType::kVInt) {
        p.type = Property::VALUE;
        p.value = lgraph::FieldData(((geax::frontend::VInt*)value)->val());
        if (is_end_path_) {
            auto right = objAlloc_.allocate<geax::frontend::VInt>();
            right->setVal(((geax::frontend::VInt*)value)->val());
            expr->setRight(right);
        }
    } else if (value->type() == geax::frontend::AstNodeType::kVBool) {
        p.type = Property::VALUE;
        p.value = lgraph::FieldData(((geax::frontend::VBool*)value)->val());
        if (is_end_path_) {
            auto right = objAlloc_.allocate<geax::frontend::VBool>();
            right->setVal(((geax::frontend::VBool*)value)->val());
            expr->setRight(right);
        }
    } else if (value->type() == geax::frontend::AstNodeType::kVDouble) {
        p.type = Property::VALUE;
        p.value = lgraph::FieldData(((geax::frontend::VDouble*)value)->val());
        if (is_end_path_) {
            auto right = objAlloc_.allocate<geax::frontend::VDouble>();
            right->setVal(((geax::frontend::VDouble*)value)->val());
            expr->setRight(right);
        }
    } else if (value->type() == geax::frontend::AstNodeType::kRef) {
        p.type = Property::VARIABLE;
        p.value = lgraph::FieldData(((geax::frontend::Ref*)value)->name());
        if (is_end_path_) {
            auto right = objAlloc_.allocate<geax::frontend::Ref>();
            auto val = ((geax::frontend::Ref*)value)->name();
            right->setName(std::move(val));
            expr->setRight(right);
        }
    } else if (value->type() == geax::frontend::AstNodeType::kParam) {
        p.type = Property::PARAMETER;
        p.value = lgraph::FieldData(((geax::frontend::Param*)value)->name());
        if (is_end_path_) {
            auto right = objAlloc_.allocate<geax::frontend::Ref>();
            auto val = ((geax::frontend::Param*)value)->name();
            right->setName(std::move(val));
            expr->setRight(right);
        }
    } else if (value->type() == geax::frontend::AstNodeType::kGetField) {
        p.type = Property::VARIABLE;
        p.value_alias = ((geax::frontend::Ref*)((
                         (geax::frontend::GetField*)value)->expr()))->name();
        p.value = lgraph::FieldData(p.value_alias);
        p.map_field_name = ((geax::frontend::GetField*)value)->fieldName();
        if (is_end_path_) {
            auto right = objAlloc_.allocate<geax::frontend::GetField>();
            auto field_name = ((geax::frontend::GetField*)value)->fieldName();
            right->setFieldName(std::move(field_name));
            right->setExpr(((geax::frontend::GetField*)value)->expr());
            expr->setRight(right);
        }
    } else {
        NOT_SUPPORT();
    }
    if (ClauseGuard::InClause(geax::frontend::AstNodeType::kNode, cur_types_)) {
        node_t_->SetProperty(p);
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    } else if (ClauseGuard::InClause(geax::frontend::AstNodeType::kEdge, cur_types_)) {
        NOT_SUPPORT();
    }
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

std::any ExecutionPlanMaker::visit(geax::frontend::TableFunctionClause* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ReadConsistency* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::AllowAnonymousTable* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::OpConcurrent* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::EdgeOnJoin* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetAllProperties* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::UpdateProperties* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetLabel* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetSingleProperty* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetSchemaClause* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetGraphClause* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetTimeZoneClause* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetParamClause* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ResetAll* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ResetSchema* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ResetTimeZone* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ResetGraph* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ResetParam* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::GetField* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::TupleGet* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Not* node) {
    if (node->expr()) ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::Neg* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::Tilde* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VSome* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BEqual* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BNotEqual* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BGreaterThan* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BNotSmallerThan* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BSmallerThan* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BNotGreaterThan* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BSafeEqual* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BAdd* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BSub* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BDiv* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BMul* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BMod* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BSquare* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BAnd* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BOr* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BXor* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BBitAnd* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BBitOr* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BBitXor* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BBitLeftShift* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BBitRightShift* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BConcat* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BIndex* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BLike* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BIn* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::If* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::Function* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::Case* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::Cast* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::MatchCase* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::AggFunc* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::BAggFunc* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::MultiCount* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Windowing* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::MkList* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::MkMap* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::MkRecord* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::MkSet* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::MkTuple* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VBool* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::VInt* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::VDouble* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::VString* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::VDate* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::VDatetime* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::VDuration* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VTime* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VNull* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::VNone* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Ref* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::IsNull* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::IsDirected* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::IsNormalized* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::IsSourceOf* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::IsDestinationOf* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::IsLabeled* node) {
    ClauseGuard cg(node->type(), cur_types_);
    if (node->expr()) ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
    if (node->labelTree()) ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->labelTree());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::Same* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::AllDifferent* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Exists* node) {
    ++cur_pattern_graph_;
    ClauseGuard cg(node->type(), cur_types_);
    for (auto& path_chain : node->pathChains()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(path_chain);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::ExplainActivity* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SessionActivity* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::TransactionActivity* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->transaction());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::FullTransaction* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::NormalTransaction* node) {
    if (node->endTransaction().has_value()) {
        NOT_SUPPORT();
    }
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->query());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::StartTransaction* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::CommitTransaction* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::RollBackTransaction* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SessionSet* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SessionReset* node) { NOT_SUPPORT(); }

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

std::any ExecutionPlanMaker::visit(geax::frontend::SchemaFromPath* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BindingValue* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BindingGraph* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BindingTable* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BindingTableInnerQuery* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BindingTableInnerExpr* node) { NOT_SUPPORT(); }

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

std::any ExecutionPlanMaker::visit(geax::frontend::JoinRightPart* node) { NOT_SUPPORT(); }

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

std::any ExecutionPlanMaker::visit(geax::frontend::SelectStatement* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::FocusedQueryStatement* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::FocusedResultStatement* node) { NOT_SUPPORT(); }

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

std::any ExecutionPlanMaker::visit(geax::frontend::InlineProcedureCall* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::NamedProcedureCall* node) {
    std::string name = std::get<std::string>(node->name());
    std::vector<OpBase*> expand_ops;
    auto op = new GqlStandaloneCall(name, node->args(), node->yield(),
                                    pattern_graphs_[cur_pattern_graph_].symbol_table);
    expand_ops.emplace_back(op);
    auto produce = new ProduceResults();
    expand_ops.emplace_back(produce);
    std::reverse(expand_ops.begin(), expand_ops.end());
    if (auto op = _SingleBranchConnect(expand_ops)) {
        _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
    }

    auto p = global_ptable.GetProcedure(name);
    if (p == nullptr) {
        result_info_.header.colums.emplace_back(name);
    } else {
        auto& yield = node->yield();
        auto& result = p->signature.result_list;
        if (!yield.has_value()) {
            for (auto& r : result) {
                result_info_.header.colums.emplace_back(r.name, r.name, false, r.type);
            }
        } else {
            auto& items = yield.value()->items();
            for (auto& item : items) {
                for (auto& r : result) {
                    if (std::get<0>(item) == r.name) {
                        result_info_.header.colums.emplace_back(r.name, r.name, false, r.type);
                        break;
                    }
                }
            }
        }
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::ForStatement* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::PrimitiveResultStatement* node) {
    auto& items = node->items();
    std::vector<OpBase*> ops;
    std::vector<std::tuple<ArithExprNode, std::string>> arith_items;
    if (!should_connect_[cur_pattern_graph_]) {
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
            throw lgraph::CypherException(
                "All sub queries in an UNION must have the same column names.");
        }
    }
    result_info_.header.colums.clear();
    ClauseGuard cg(node->type(), cur_types_);
    for (auto& item : items) {
        auto expr = std::get<1>(item);
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(expr);
        ArithExprNode ae(expr, pattern_graphs_[cur_pattern_graph_].symbol_table);
        auto alias = std::get<0>(item);
        arith_items.push_back(std::make_tuple(ae, alias));
        AstAggExprDetector agg_expr_detector(expr);
        bool has_aggregation = false;
        if (agg_expr_detector.Validate()) {
            has_aggregation = agg_expr_detector.HasValidAggFunc();
        } else {
            error_msg_ = std::any_cast<std::string>(agg_expr_detector.reportError());
            return geax::frontend::GEAXErrorCode::GEAX_ERROR;
        }
        // build header
        if (expr->type() == geax::frontend::AstNodeType::kRef) {
            std::string& name = ((geax::frontend::Ref*)expr)->name();
            auto it = pattern_graphs_[cur_pattern_graph_].symbol_table.symbols.find(name);
            if (it == pattern_graphs_[cur_pattern_graph_].symbol_table.symbols.end()) {
                error_msg_ = "Unknown variable: " + name;
                return geax::frontend::GEAXErrorCode::GEAX_ERROR;
            }
            lgraph_api::LGraphType column_type;
            switch (it->second.type) {
            case SymbolNode::NODE:
                column_type = lgraph_api::LGraphType::NODE;
                break;
            case SymbolNode::RELATIONSHIP:
                column_type = lgraph_api::LGraphType::RELATIONSHIP;
                break;
            case SymbolNode::NAMED_PATH:
                column_type = lgraph_api::LGraphType::PATH;
                break;
            case SymbolNode::CONSTANT:
            case SymbolNode::PARAMETER:
                column_type = lgraph_api::LGraphType::ANY;
                break;
            default:
                NOT_SUPPORT();
            }
            result_info_.header.colums.emplace_back(name, alias, has_aggregation, column_type);
        } else {
            result_info_.header.colums.emplace_back(alias, alias, has_aggregation,
                                                    lgraph_api::LGraphType::ANY);
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
        THROW_CODE(InputError, FMA_FMT("Unknown order by field"));
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

std::any ExecutionPlanMaker::visit(geax::frontend::CatalogModifyStatement* node) { NOT_SUPPORT(); }

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
    auto op = new OpGqlCreate(node->paths(), &pattern_graph);
    _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::ReplaceStatement* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetStatement* node) {
    auto& pattern_graph = pattern_graphs_[cur_pattern_graph_];
    auto op = new OpGqlSet(node->items(), &pattern_graph);
    _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::DeleteStatement* node) {
    auto op = new OpGqlDelete(node->items());
    _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::RemoveStatement* node) {
    auto& pattern_graph = pattern_graphs_[cur_pattern_graph_];
    for (auto& item : node->items()) {
        geax::frontend::RemoveSingleProperty* remove;
        checkedCast(item, remove);
        if (pattern_graphs_[cur_pattern_graph_].symbol_table.symbols.find(remove->v()) ==
            pattern_graphs_[cur_pattern_graph_].symbol_table.symbols.end()) {
            THROW_CODE(InputError, "Variable `{}` not defined", remove->v());
        }
    }
    auto op = new OpGqlRemove(node->items(), &pattern_graph);
    _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::MergeStatement* node) {
    auto& pattern_graph = pattern_graphs_[cur_pattern_graph_];
    auto op =
        new OpGqlMerge(node->onMatch(), node->onCreate(), node->pathPattern(), &pattern_graph);
    _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::OtherWise* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Union* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Except* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Intersect* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Param* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ShowProcessListStatement* node) {
    NOT_SUPPORT();
}

std::any ExecutionPlanMaker::visit(geax::frontend::KillStatement* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ManagerStatement* node) { NOT_SUPPORT(); }

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

std::any ExecutionPlanMaker::visit(geax::frontend::DummyNode* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::reportError() { return error_msg_; }

std::any ExecutionPlanMaker::visit(geax::frontend::RemoveSingleProperty* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}
std::any ExecutionPlanMaker::visit(geax::frontend::ListComprehension* node) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

}  // namespace cypher
