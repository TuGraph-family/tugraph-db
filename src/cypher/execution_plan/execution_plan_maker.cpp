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

geax::frontend::GEAXErrorCode ExecutionPlanMaker::Build(geax::frontend::AstNode* astNode,
                                                        OpBase*& root) {
    cur_types_.clear();
    auto ret = std::any_cast<geax::frontend::GEAXErrorCode>(astNode->accept(*this));
    if (ret != geax::frontend::GEAXErrorCode::GEAX_SUCCEED) {
        return ret;
    }
    _DumpPlanBeforeConnect(0, false);
    root = pattern_graph_root_[0];
    for (size_t i = 1; i < pattern_graph_root_.size(); i++) {
        root = _Connect(root, pattern_graph_root_[i], &(pattern_graphs_[i]));
    }
    if (op_filter_ != nullptr) {
        NOT_SUPPORT();
    }
    return ret;
}

std::string ExecutionPlanMaker::_DumpPlanBeforeConnect(int indent, bool statistics) const {
    std::string s = "Execution Plan Before Connect: \n";
    for (auto seg : pattern_graph_root_) OpBase::DumpStream(seg, 0, false, s);
    FMA_DBG_STREAM(PlanLogger()) << s;
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
        // 符号表中没有type为argument的
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
        // 符号表有type为argument的
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
    if (path_patterns.size() > 1) {
        NOT_SUPPORT();
    }
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
        auto expr_filter = std::make_shared<lgraph::GeaxExprFilter>(
            node->where().value(), pattern_graphs_[cur_pattern_graph_].symbol_table);
        auto op_filter = new OpFilter(expr_filter);
        op_filter->AddChild(pattern_graph_root_[cur_pattern_graph_]);
        pattern_graph_root_[cur_pattern_graph_] = op_filter;
    }
    if (!node->yield()->items().empty()) {
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
    // todo: ...
    ClauseGuard cg(node->type(), cur_types_);
    auto& tails = node->tails();
    std::vector<OpBase*> expand_ops;
    if (tails.size() == 0) {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }
    for (auto [edge, end_node] : tails) {
        start_t_ = node_t_;
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(end_node);
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(edge);
        auto& start = pattern_graph.GetNode(start_t_->Alias());
        auto& relp = pattern_graph.GetRelationship(relp_t_->Alias());
        auto& end = pattern_graph.GetNode(node_t_->Alias());
        OpBase* expand_op;
        FMA_DBG() << relp.MinHop() << " " << relp.MaxHop();
        if (relp.VarLen()) {
            expand_op = new VarLenExpand(&pattern_graph, &start, &end, &relp);
        } else {
            expand_op = new ExpandAll(&pattern_graph, &start, &end, &relp);
        }
        expand_ops.emplace_back(expand_op);
        if (op_filter_ != nullptr) {
            expand_ops.push_back(op_filter_);
            op_filter_ = nullptr;
        }
    }
    std::reverse(expand_ops.begin(), expand_ops.end());
    if (auto op = _SingleBranchConnect(expand_ops)) {
        _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
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
            _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
        }
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::Edge* node) {
    ClauseGuard cg(node->type(), cur_types_);
    relp_t_ = std::make_shared<Relationship>();
    auto filler = node->filler();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(filler);
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

std::any ExecutionPlanMaker::visit(geax::frontend::OrderByField* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::PathModePrefix* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::PathSearchPrefix* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SingleLabel* node) {
    if (ClauseGuard::InClause(geax::frontend::AstNodeType::kNode, cur_types_)) {
        node_t_->SetLabel(node->label());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    } else if ((ClauseGuard::InClause(geax::frontend::AstNodeType::kEdge, cur_types_))) {
        relp_t_->AddType(node->label());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }
    NOT_SUPPORT();
}

std::any ExecutionPlanMaker::visit(geax::frontend::LabelOr* node) { NOT_SUPPORT(); }

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
    if (value->type() == geax::frontend::AstNodeType::kVString) {
        p.type = Property::VALUE;
        p.value = lgraph::FieldData(((geax::frontend::VString*)value)->val());
    } else if (value->type() == geax::frontend::AstNodeType::kVInt) {
        p.type = Property::VALUE;
        p.value = lgraph::FieldData(((geax::frontend::VInt*)value)->val());
    } else if (value->type() == geax::frontend::AstNodeType::kRef) {
        p.type = Property::VARIABLE;
        p.value = lgraph::FieldData(((geax::frontend::Ref*)value)->name());
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

std::any ExecutionPlanMaker::visit(geax::frontend::YieldField* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::TableFunctionClause* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ReadConsistency* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::AllowAnonymousTable* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::OpConcurrent* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetAllProperties* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetLabel* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::GetField* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::TupleGet* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Not* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Neg* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Tilde* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VSome* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BEqual* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BNotEqual* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BGreaterThan* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BNotSmallerThan* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BSmallerThan* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BNotGreaterThan* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BSafeEqual* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BAdd* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BSub* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BDiv* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BMul* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BMod* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BAnd* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BOr* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BXor* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BBitAnd* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BBitOr* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BBitXor* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BBitLeftShift* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BBitRightShift* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BConcat* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BIndex* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BLike* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BIn* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::If* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Function* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Case* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Cast* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::MatchCase* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::AggFunc* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::BAggFunc* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::MultiCount* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Windowing* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::MkList* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::MkMap* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::MkRecord* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::MkSet* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::MkTuple* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VBool* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VInt* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VDouble* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VString* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VDate* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VDatetime* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VDuration* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VTime* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VNull* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::VNone* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Ref* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::IsNull* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::IsDirected* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::IsNormalized* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::IsSourceOf* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::IsDestinationOf* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::IsLabeled* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Same* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::AllDifferent* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Exists* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ProcedureBody* node) {
    pattern_graph_size_ = node->statements().size();
    pattern_graph_root_.resize(pattern_graph_size_, nullptr);
    for (size_t i = 0; i < node->statements().size(); i++) {
        cur_pattern_graph_ = i;
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

std::any ExecutionPlanMaker::visit(geax::frontend::JoinQueryExpression* node) {
    auto head = node->head();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(head);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::JoinRightPart* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::CompositeQueryStatement* node) {
    auto head = node->head();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(head);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::AmbientLinearQueryStatement* node) {
    auto& query_stmts = node->queryStatements();
    if (query_stmts.size() > 1) {
        NOT_SUPPORT();
    }
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

std::any ExecutionPlanMaker::visit(geax::frontend::FilterStatement* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::PrimitiveResultStatement* node) {
    auto& items = node->items();
    std::vector<OpBase*> ops;
    if (cur_pattern_graph_ == pattern_graph_size_ - 1) {
        auto result = new ProduceResults();
        ops.push_back(result);
    }
    std::vector<std::tuple<ArithExprNode, std::string>> arith_items;
    auto& pattern_graph = pattern_graphs_[cur_pattern_graph_];
    result_info_.header.colums.clear();
    for (auto& item : items) {
        ArithExprNode ae(std::get<1>(item), pattern_graph.symbol_table);
        auto alias = std::get<0>(item);
        arith_items.push_back(std::make_tuple(ae, alias));
        AstAggExprDetector agg_expr_detector(std::get<1>(item));
        bool has_aggregation = false;
        if (agg_expr_detector.Validate()) {
            has_aggregation = agg_expr_detector.HasValidAggFunc();
        } else {
            error_msg_ = std::any_cast<std::string>(agg_expr_detector.reportError());
            return geax::frontend::GEAXErrorCode::GEAX_ERROR;
        }
        // build header
        if (std::get<1>(item)->type() == geax::frontend::AstNodeType::kRef) {
            std::string& name = ((geax::frontend::Ref*)std::get<1>(item))->name();
            auto it = pattern_graph.symbol_table.symbols.find(name);
            if (it == pattern_graph.symbol_table.symbols.end()) {
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
    if (node->limit().has_value()) {
        ops.emplace_back(new Limit(std::get<0>(node->limit().value())));
    }
    if (node->offset().has_value()) {
        ops.emplace_back(new Skip(std::get<0>(node->offset().value())));
    }
    std::vector<std::pair<int, bool>> order_by_items;
    for (auto order_by_field : node->orderBys()) {
        for (size_t i = 0; i < items.size(); ++i) {
            if (auto order_by_ref = dynamic_cast<geax::frontend::Ref*>(order_by_field->field())) {
                auto field_name = std::get<0>(items[i]);
                if (order_by_ref->name() == field_name) {
                    order_by_items.emplace_back(std::make_pair(i, !order_by_field->order()));
                    break;
                }
            } else {
                NOT_SUPPORT();
            }
        }
    }
    if (order_by_items.size() != node->orderBys().size()) {
        NOT_SUPPORT_WITH_MSG("Unknown order by field");
    }
    if (!order_by_items.empty()) {
        ops.emplace_back(new Sort(
            order_by_items, node->limit().has_value() ? std::get<0>(node->limit().value()) : -1,
            node->offset().has_value() ? std::get<0>(node->offset().value()) : -1));
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
            ArithExprNode ae(std::get<1>(item), pattern_graph.symbol_table);
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
                noneaggr_item_names, item_names, &pattern_graph.symbol_table, result_info_.header));
        }
    }
    if (!has_aggregation) {
        if (node->distinct()) {
            ops.emplace_back(new Distinct());
        }
        ops.emplace_back(new Project(arith_items, &pattern_graph.symbol_table));
    }
    if (auto op = _SingleBranchConnect(ops)) {
        _UpdateStreamRoot(op, pattern_graph_root_[cur_pattern_graph_]);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any ExecutionPlanMaker::visit(geax::frontend::CatalogModifyStatement* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::LinearDataModifyingStatement* node) {
    NOT_SUPPORT();
}

std::any ExecutionPlanMaker::visit(geax::frontend::InsertStatement* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ReplaceStatement* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetStatement* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::DeleteStatement* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::RemoveStatement* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::OtherWise* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Union* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Except* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Intersect* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::UpdateProperties* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::SetSingleProperty* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::Param* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ShowProcessListStatement* node) {
    NOT_SUPPORT();
}

std::any ExecutionPlanMaker::visit(geax::frontend::KillStatement* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::ManagerStatement* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::visit(geax::frontend::DummyNode* node) { NOT_SUPPORT(); }

std::any ExecutionPlanMaker::reportError() { return error_msg_; }

}  // namespace cypher
