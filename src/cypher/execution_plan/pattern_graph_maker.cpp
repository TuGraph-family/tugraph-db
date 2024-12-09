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

#include "cypher/execution_plan/pattern_graph_maker.h"
#include "common/logger.h"
#include <map>
#include <unordered_set>
#include "common/exceptions.h"
#include "cypher/execution_plan/clause_guard.h"
#include "cypher/procedure/procedure.h"
#include "cypher/utils/geax_util.h"
#include "geax-front-end/ast/Ast.h"

namespace cypher {

geax::frontend::GEAXErrorCode PatternGraphMaker::Build(geax::frontend::AstNode* astNode,
                                                       RTContext* ctx) {
    ctx_ = ctx;
    cur_pattern_graph_ = -1;
    cur_types_.clear();
    return std::any_cast<geax::frontend::GEAXErrorCode>(astNode->accept(*this));
}

std::any PatternGraphMaker::visit(geax::frontend::GraphPattern* node) {
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
    }
    if (node->yield().has_value()) {
        NOT_SUPPORT();
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::PathPattern* node) {
    if (node->prefix().has_value()) {
        NOT_SUPPORT();
    }
    auto& chains = node->chains();
    for (auto chain : chains) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(chain);
    }
    if (node->alias().has_value()) {
        auto& pattern_graph = pattern_graphs_[cur_pattern_graph_];
        pattern_graph.symbol_table.symbols.emplace(
            node->alias().value(),
            SymbolNode(symbols_idx_[cur_pattern_graph_]++, cypher::SymbolNode::NAMED_PATH,
                       SymbolNode::Scope::LOCAL));

        auto& path_elements = pattern_graph.symbol_table.anot_collection.path_elements;
        std::vector<Node>& nodes = pattern_graph.GetNodes();
        std::vector<Relationship>& relationships = pattern_graph.GetRelationships();
        std::vector<std::shared_ptr<geax::frontend::Ref>> paths;
        paths.reserve(nodes.size() + relationships.size());
        size_t idx;
        std::string alias;
        for (idx = 0; idx < relationships.size(); ++idx) {
            std::shared_ptr<geax::frontend::Ref> node_ref = std::make_shared<geax::frontend::Ref>();
            alias = nodes[idx].Alias();
            node_ref->setName(std::move(alias));
            paths.push_back(node_ref);
            std::shared_ptr<geax::frontend::Ref> relp_ref = std::make_shared<geax::frontend::Ref>();
            alias = relationships[idx].Alias();
            relp_ref->setName(std::move(alias));
            paths.push_back(relp_ref);
        }
        std::shared_ptr<geax::frontend::Ref> node_ref = std::make_shared<geax::frontend::Ref>();
        alias = nodes[idx].Alias();
        node_ref->setName(std::move(alias));
        paths.push_back(node_ref);
        path_elements.emplace(node->alias().value(), paths);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::PathChain* node) {
    auto head = node->head();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(head);
    // todo: ...
    ClauseGuard cg(node->type(), cur_types_);
    auto& tails = node->tails();
    if (tails.size() == 0) {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }
    for (auto [edge, end_node] : tails) {
        start_t_ = node_t_;
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(end_node);
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(edge);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::Node* node) {
    ClauseGuard cg(node->type(), cur_types_);
    node_t_ = std::make_shared<Node>();
    node_t_->ast_node_ = node;
    auto filler = node->filler();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(filler);
    AddNode(node_t_.get());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::Edge* node) {
    ClauseGuard cg(node->type(), cur_types_);
    relp_t_ = std::make_shared<Relationship>();
    auto& pattern_graph = pattern_graphs_[cur_pattern_graph_];
    relp_t_->SetLhs(pattern_graph.GetNode(start_t_->Alias()).ID());
    relp_t_->SetRhs(pattern_graph.GetNode(node_t_->Alias()).ID());
    auto filler = node->filler();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(filler);
    relp_t_->ast_node_ = node;
    switch (node->direction()) {
    case geax::frontend::EdgeDirection::kPointRight:
        relp_t_->direction_ = parser::LinkDirection::LEFT_TO_RIGHT;
        break;
    case geax::frontend::EdgeDirection::kPointLeft:
        relp_t_->direction_ = parser::LinkDirection::RIGHT_TO_LEFT;
        break;
    case geax::frontend::EdgeDirection::kAnyDirected:
        relp_t_->direction_ = parser::LinkDirection::DIR_NOT_SPECIFIED;
        break;
    default:
        NOT_SUPPORT();
    }
    if (node->hopRange().has_value()) {
        auto min_hop = std::get_if<int64_t>(&std::get<0>(node->hopRange().value()));
        if (min_hop) {
            relp_t_->min_hop_ = *min_hop;
        } else {
            NOT_SUPPORT();
        }
        auto optional_max_hop = std::get<1>(node->hopRange().value());
        if (optional_max_hop.has_value()) {
            auto max_hop = std::get_if<int64_t>(&optional_max_hop.value());
            if (max_hop) {
                relp_t_->max_hop_ = *max_hop > parser::VAR_LEN_EXPAND_MAX_HOP
                                        ? parser::VAR_LEN_EXPAND_MAX_HOP
                                        : *max_hop;
            } else {
                NOT_SUPPORT();
            }
        } else {
            relp_t_->max_hop_ = parser::VAR_LEN_EXPAND_MAX_HOP;
        }
    }
    LOG_DEBUG("{} {}", relp_t_->MinHop(), relp_t_->MaxHop());
    AddRelationship(relp_t_.get());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::ElementFiller* node) {
    // variable
    auto& variable = node->v();
    if (!variable.has_value()) {
        NOT_SUPPORT();
    }
    SymbolNode::Type symbol_node_type;
    auto& symbols = pattern_graphs_[cur_pattern_graph_].symbol_table.symbols;
    auto& pattern_graph = pattern_graphs_[cur_pattern_graph_];
    if (ClauseGuard::InClause(geax::frontend::AstNodeType::kNode, cur_types_)) {
        node_t_->SetAlias(variable.value());
        symbol_node_type = SymbolNode::Type::NODE;
        auto it = symbols.find(variable.value());
        if (it == symbols.end()) {
            if (pattern_graph.GetNode(variable.value()).Empty()) {
                if (ClauseGuard::InClause(geax::frontend::AstNodeType::kMatchStatement,
                                          cur_types_)) {
                    node_t_->derivation_ = Node::Derivation::MATCHED;
                } else if (ClauseGuard::InClause(geax::frontend::AstNodeType::kInsertStatement,
                                                 cur_types_)) {
                    node_t_->derivation_ = Node::Derivation::CREATED;
                } else if (ClauseGuard::InClause(geax::frontend::AstNodeType::kMergeStatement,
                                                 cur_types_)) {
                    node_t_->derivation_ = Node::Derivation::MERGED;
                } else if (ClauseGuard::InClause(geax::frontend::AstNodeType::kExists,
                                                 cur_types_)) {
                    node_t_->derivation_ = Node::Derivation::MATCHED;
                } else {
                    NOT_SUPPORT();
                }
            }
        } else {
            if (it->second.scope == cypher::SymbolNode::ARGUMENT) {
                node_t_->derivation_ = Node::Derivation::ARGUMENT;
            }
            if (ClauseGuard::InClause(geax::frontend::AstNodeType::kMergeStatement, cur_types_)) {
                if (it->second.scope == cypher::SymbolNode::LOCAL) {
                    node_t_->Visited() = true;
                }
            }
        }
    } else if (ClauseGuard::InClause(geax::frontend::AstNodeType::kEdge, cur_types_)) {
        relp_t_->SetAlias(variable.value());
        symbol_node_type = SymbolNode::Type::RELATIONSHIP;
        auto it = symbols.find(variable.value());
        if (it == symbols.end()) {
            if (pattern_graph.GetRelationship(variable.value()).Empty()) {
                if (ClauseGuard::InClause(geax::frontend::AstNodeType::kMatchStatement,
                                          cur_types_)) {
                    relp_t_->derivation_ = Relationship::Derivation::MATCHED;
                } else if (ClauseGuard::InClause(geax::frontend::AstNodeType::kInsertStatement,
                                                 cur_types_)) {
                    relp_t_->derivation_ = Relationship::Derivation::CREATED;
                } else if (ClauseGuard::InClause(geax::frontend::AstNodeType::kMergeStatement,
                                                 cur_types_)) {
                    relp_t_->derivation_ = Relationship::Derivation::MERGED;
                } else if (ClauseGuard::InClause(geax::frontend::AstNodeType::kExists,
                                                 cur_types_)) {
                    relp_t_->derivation_ = Relationship::Derivation::MATCHED;
                } else {
                    NOT_SUPPORT();
                }
            }
        } else {
            if (it->second.scope == cypher::SymbolNode::ARGUMENT) {
                relp_t_->derivation_ = Relationship::Derivation::ARGUMENT;
            }
        }
    } else {
        NOT_SUPPORT();
    }
    AddSymbol(variable.value(), symbol_node_type, cypher::SymbolNode::LOCAL);
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

std::any PatternGraphMaker::visit(geax::frontend::WhereClause* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->predicate());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::OrderByField* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::PathModePrefix* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::PathSearchPrefix* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::SingleLabel* node) {
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

std::any PatternGraphMaker::visit(geax::frontend::LabelOr* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::LabelAnd* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::LabelNot* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::PropStruct* node) {
    if (ClauseGuard::InClause(geax::frontend::AstNodeType::kMatchStatement, cur_types_) ||
        ClauseGuard::InClause(geax::frontend::AstNodeType::kMergeStatement, cur_types_)) {
        if (ClauseGuard::InClause(geax::frontend::AstNodeType::kNode, cur_types_) ||
            ClauseGuard::InClause(geax::frontend::AstNodeType::kEdge, cur_types_)) {
            return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
        }
        NOT_SUPPORT();
    } else if (ClauseGuard::InClause(geax::frontend::AstNodeType::kInsertStatement, cur_types_)) {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    } else {
        NOT_SUPPORT();
    }
}

std::any PatternGraphMaker::visit(geax::frontend::YieldField* node) {
    auto p = global_ptable.GetProcedure(curr_procedure_name_);
    if (!p) {
        THROW_CODE(CypherException, "No such procedure: {}", curr_procedure_name_);
    }
    for (auto& pair : node->items()) {
        const std::string& name = std::get<0>(pair);
        ProcedureResultType type;
        bool found = false;
        for (auto& r : p->signature.result_list) {
            if (r.name == name) {
                type = r.type;
                found = true;
            }
        }
        if (!found) {
            THROW_CODE(CypherException, "No such yield field: {} in {}", name, p->proc_name);
        }
        switch (type) {
            case ProcedureResultType::Value: {
                AddSymbol(name, cypher::SymbolNode::CONSTANT, cypher::SymbolNode::LOCAL);
                break;
            }
            case ProcedureResultType::Node: {
                Node n;
                n.SetAlias(name);
                n.derivation_ = Node::Derivation::YIELD;
                AddNode(&n);
                AddSymbol(name, cypher::SymbolNode::NODE, cypher::SymbolNode::LOCAL);
                break;
            }
            case ProcedureResultType::Relationship: {
                CYPHER_TODO();
                AddSymbol(name, cypher::SymbolNode::RELATIONSHIP, cypher::SymbolNode::LOCAL);
                break;
            }
            default: THROW_CODE(CypherException, "Unkown yield field type: {}", (int)type);
        }
    }
    if (node->predicate()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->predicate());
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::TableFunctionClause* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::ReadConsistency* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::AllowAnonymousTable* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::OpConcurrent* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::EdgeOnJoin* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::SetAllProperties* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::UpdateProperties* node) {
    auto& alias = node->v();
    auto& symbols = pattern_graphs_[cur_pattern_graph_].symbol_table.symbols;
    if (symbols.find(alias) == symbols.end()) {
        THROW_CODE(InputError, "Variable `{}` not defined", alias);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::SetLabel* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::RemoveLabel* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::SetSingleProperty* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::SetSchemaClause* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::SetGraphClause* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::SetTimeZoneClause* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::SetParamClause* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::ResetAll* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::ResetSchema* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::ResetTimeZone* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::ResetGraph* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::ResetParam* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::GetField* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::TupleGet* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::Not* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::Neg* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::Tilde* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::VSome* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BEqual* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BNotEqual* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BGreaterThan* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BNotSmallerThan* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BSmallerThan* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BNotGreaterThan* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BSafeEqual* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BAdd* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BSub* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BDiv* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BMul* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BMod* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BSquare* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BAnd* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BOr* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BXor* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BBitAnd* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BBitOr* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BBitXor* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BBitLeftShift* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BBitRightShift* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BConcat* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BIndex* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BLike* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BIn* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::If* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->condition());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->trueBody());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->falseBody());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::Function* node) {
    for (auto& arg : node->args()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(arg);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::Case* node) {
    if (node->input().has_value()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->input().value());
    }
    for (auto& case_body : node->caseBodies()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(std::get<0>(case_body));
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(std::get<1>(case_body));
    }
    if (node->elseBody().has_value()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->elseBody().value());
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::Cast* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::MatchCase* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->input());
    for (auto& case_body : node->cases()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(std::get<0>(case_body));
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(std::get<1>(case_body));
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::AggFunc* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
    for (auto& expr : node->distinctBy()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(expr);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::BAggFunc* node) {
    auto& left = node->lExpr();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(std::get<1>(left));
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->rExpr());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::MultiCount* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::Windowing* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::MkList* node) {
    for (auto elem : node->elems()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(elem);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::MkMap* node) {
    for (auto [elem1, elem2] : node->elems()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(elem1);
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(elem2);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::MkRecord* node) {
    for (auto [elem1, elem2] : node->elems()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(elem2);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::MkSet* node) {
    for (auto elem : node->elems()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(elem);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::MkTuple* node) {
    for (auto elem : node->elems()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(elem);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::VBool* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::VInt* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::VDouble* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::VString* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::VDate* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::VDatetime* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::VDuration* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::VTime* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::VNull* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::VNone* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::Ref* node) {
    auto& symbols = pattern_graphs_[cur_pattern_graph_].symbol_table.symbols;
    if (symbols.find(node->name()) == symbols.end()) {
        THROW_CODE(CypherException, "Unknown variable: " + node->name());
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::Param* node) {
    AddSymbol(node->name(), SymbolNode::Type::PARAMETER, cypher::SymbolNode::LOCAL);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::IsNull* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::IsDirected* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::IsNormalized* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::IsSourceOf* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::IsDestinationOf* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::IsLabeled* node) {
    ClauseGuard cg(node->type(), cur_types_);
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->labelTree());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::Same* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::AllDifferent* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::Exists* node) {
    pattern_graphs_.resize(pattern_graphs_.size() + 1);
    symbols_idx_.resize(symbols_idx_.size() + 1);
    ++cur_pattern_graph_;
    ClauseGuard cg(node->type(), cur_types_);
    for (auto& path_chain : node->pathChains()) {
        auto head = path_chain->head();
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(head);
        ClauseGuard cg(node->type(), cur_types_);
        auto& tails = path_chain->tails();
        if (tails.size() == 0) {
            return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
        }
        for (auto [edge, end_node] : tails) {
            start_t_ = node_t_;
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(end_node);
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(edge);
        }
    }
    auto& symbols_prev = pattern_graphs_[cur_pattern_graph_ - 1].symbol_table.symbols;
    auto& symbols_cur = pattern_graphs_[cur_pattern_graph_].symbol_table.symbols;
    std::unordered_map<std::string, SymbolNode> temp_symbols;
    std::map<size_t, std::pair<std::string, SymbolNode>> argument_symbols;
    size_t temp_symbols_idx = 0;
    for (auto& [alias, symbol] : symbols_prev) {
        auto it = symbols_cur.find(alias);
        if (it != symbols_cur.end()) {
            argument_symbols.emplace(
                symbol.id,
                std::make_pair(alias, SymbolNode(symbol.id, symbol.type, SymbolNode::ARGUMENT)));
        }
    }

    for (auto& [id, pair] : argument_symbols) {
        temp_symbols.emplace(pair.first,
                             SymbolNode(temp_symbols_idx++, pair.second.type, pair.second.scope));
    }

    for (auto& [alias, symbol] : symbols_cur) {
        auto it = symbols_prev.find(alias);
        if (it == symbols_prev.end()) {
            temp_symbols.emplace(alias, SymbolNode(temp_symbols_idx++, symbol.type, symbol.scope));
        }
    }
    std::swap(symbols_cur, temp_symbols);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::ExplainActivity* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::SessionActivity* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::TransactionActivity* node) {
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->transaction());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::FullTransaction* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::NormalTransaction* node) {
    if (node->endTransaction().has_value()) {
        NOT_SUPPORT();
    }
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->query());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::StartTransaction* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::CommitTransaction* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::RollBackTransaction* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::SessionSet* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::SessionReset* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::ProcedureBody* node) {
    size_t pattern_graphs_size = 0;
    for (auto i : node->statements()) {
        auto statement = i->statement();
        pattern_graphs_size += 1;
        if (statement->type() == geax::frontend::AstNodeType::kQueryStatement) {
            geax::frontend::QueryStatement* queryStatement =
                (geax::frontend::QueryStatement*)statement;
            // not support join query, but support union query
            // in CompositeQueryStatement
            int union_size = queryStatement->joinQuery()->head()->body().size();
            pattern_graphs_size += union_size;
            if (union_size) {
                for (int j = 0; j <= union_size; ++j) {
                    pattern_graph_in_union_.push_back(true);
                }
            } else {
                pattern_graph_in_union_.push_back(false);
            }
        } else {
            pattern_graph_in_union_.push_back(false);
        }
    }
    pattern_graphs_.resize(pattern_graphs_size);
    symbols_idx_.resize(pattern_graphs_size, 0);
    for (auto stmt : node->statements()) {
        cur_pattern_graph_ += 1;
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(stmt);
    }
    /* Introduce argument nodes & relationships */
    // todo: revisit when arguments also in MATCH
    for (auto& graph : pattern_graphs_) {
        int invisible_node_idx = 0;
        for (auto& a : graph.symbol_table.symbols) {
            if (a.second.scope == SymbolNode::ARGUMENT) {
                if (a.second.type == SymbolNode::NODE && graph.GetNode(a.first).Empty()) {
                    graph.AddNode("", a.first, Node::ARGUMENT);
                } else if (a.second.type == SymbolNode::RELATIONSHIP &&
                           graph.GetRelationship(a.first).Empty()) {
                    auto src_alias = std::string(parser::INVISIBLE)
                                         .append("NODE_")
                                         .append(std::to_string(invisible_node_idx++));
                    auto dst_alias = std::string(parser::INVISIBLE)
                                         .append("NODE_")
                                         .append(std::to_string(invisible_node_idx++));
                    auto src_nid = graph.AddNode("", src_alias, Node::ARGUMENT);
                    auto dst_nid = graph.AddNode("", dst_alias, Node::ARGUMENT);
                    graph.AddRelationship(std::set<std::string>{}, src_nid, dst_nid,
                                          parser::LinkDirection::UNKNOWN, a.first,
                                          Relationship::ARGUMENT);
                    auto& src_node = graph.GetNode(src_nid);
                    auto& dst_node = graph.GetNode(dst_nid);
                    src_node.Visited() = true;
                    dst_node.Visited() = true;
                }
            }
        }
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::SchemaFromPath* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::BindingValue* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::BindingGraph* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::BindingTable* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::BindingTableInnerQuery* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::BindingTableInnerExpr* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::StatementWithYield* node) {
    auto stmt = node->statement();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(stmt);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::QueryStatement* node) {
    auto join_query = node->joinQuery();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(join_query);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::StandaloneCallStatement* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::JoinQueryExpression* node) {
    auto head = node->head();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(head);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::JoinRightPart* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::CompositeQueryStatement* node) {
    auto head = node->head();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(head);
    for (auto statement : node->body()) {
        cur_pattern_graph_ += 1;
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(std::get<1>(statement));
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::AmbientLinearQueryStatement* node) {
    auto& query_stmts = node->queryStatements();
    int match_count = 0;
    for (auto &stat : query_stmts) {
        if (stat->type() == geax::frontend::AstNodeType::kMatchStatement) {
            match_count++;
        }
    }
    if (match_count > 1) {
        THROW_CODE(CypherException, "Not support more than one (optional) match clause.");
    }
    for (auto query_stmt : query_stmts) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(query_stmt);
    }
    auto projection = node->resultStatement();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(projection);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::SelectStatement* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::FocusedQueryStatement* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::FocusedResultStatement* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::MatchStatement* node) {
    ClauseGuard cg(node->type(), cur_types_);
    auto graph = node->graphPattern();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(graph);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::FilterStatement* node) {
    ClauseGuard cg(node->type(), cur_types_);
    auto predicate = node->predicate();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(predicate);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::CallQueryStatement* node) {
    auto procedureStatement = node->procedureStatement();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(procedureStatement);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::CallProcedureStatement* node) {
    auto procedure = node->procedureCall();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(procedure);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::InlineProcedureCall* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::NamedProcedureCall* node) {
    curr_procedure_name_ = std::get<std::string>(node->name());
    if (node->yield().has_value()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->yield().value());
    }
    for (auto& expr : node->args()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(expr);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::ForStatement* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::PrimitiveResultStatement* node) {
    auto& items = node->items();
    auto& pattern_graph = pattern_graphs_[cur_pattern_graph_];
    std::unordered_set<std::string> filter;
    for (auto item : items) {
        auto alias = std::get<0>(item);
        if (!filter.count(alias)) {
            filter.insert(alias);
        } else {
            THROW_CODE(CypherException, "Duplicate alias: {}", alias);
        }
        auto expr = std::get<1>(item);
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(expr);
        SymbolNode::Type symbol_type = SymbolNode::Type::CONSTANT;
        if (auto ref = dynamic_cast<geax::frontend::Ref*>(expr)) {
            auto ref_symbol = pattern_graph.symbol_table.symbols.find(ref->name());
            if (ref_symbol != pattern_graph.symbol_table.symbols.end()) {
                symbol_type = ref_symbol->second.type;
            }
        }
        if (cur_pattern_graph_ < pattern_graphs_.size() - 1 &&
            pattern_graph.symbol_table.symbols.find(alias) ==
                pattern_graph.symbol_table.symbols.end()) {
            pattern_graph.symbol_table.symbols.emplace(
                alias,
                SymbolNode(symbols_idx_[cur_pattern_graph_]++, symbol_type, SymbolNode::LOCAL));
        }
        if (!pattern_graph_in_union_[cur_pattern_graph_]) {
            if (cur_pattern_graph_ < pattern_graphs_.size() - 1 &&
                pattern_graphs_[cur_pattern_graph_ + 1].symbol_table.symbols.find(alias) ==
                    pattern_graphs_[cur_pattern_graph_ + 1].symbol_table.symbols.end()) {
                pattern_graphs_[cur_pattern_graph_ + 1].symbol_table.symbols.emplace(
                    alias, SymbolNode(symbols_idx_[cur_pattern_graph_ + 1]++, symbol_type,
                                      SymbolNode::ARGUMENT));
            }
        }
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::CatalogModifyStatement* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::LinearDataModifyingStatement* node) {
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
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::InsertStatement* node) {
    ClauseGuard cg(node->type(), cur_types_);
    auto& paths = node->paths();
    for (auto path : paths) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(path);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::ReplaceStatement* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::SetStatement* node) {
    auto& set_items = node->items();
    for (auto item : set_items) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(item);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::DeleteStatement* node) {
    auto& items = node->items();
    auto& symbols = pattern_graphs_[cur_pattern_graph_].symbol_table.symbols;
    for (auto& item : items) {
        if (symbols.find(item) == symbols.end()) {
            THROW_CODE(InputError, "Variable `{}` not defined", item);
        }
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::RemoveStatement* ) {
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::MergeStatement* node) {
    ClauseGuard cg(node->type(), cur_types_);
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->pathPattern());
    for (auto& match : node->onMatch()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(match);
    }
    for (auto& create : node->onCreate()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(create);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::OtherWise* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::Union* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::Except* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::Intersect* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::ShowProcessListStatement* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::KillStatement* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::ManagerStatement* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::visit(geax::frontend::UnwindStatement* node) {
    const std::string& variable = node->variable();
    auto list = node->list();
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(list);
    auto var_scope = cypher::SymbolNode::Scope::LOCAL;
    if (typeid(*list) == typeid(geax::frontend::Ref)) {
        geax::frontend::Ref* ref = (geax::frontend::Ref*)list;
        if (pattern_graphs_[cur_pattern_graph_].symbol_table.symbols.find(ref->name()) !=
            pattern_graphs_[cur_pattern_graph_].symbol_table.symbols.end()) {
            var_scope = cypher::SymbolNode::Scope::DERIVED_ARGUMENT;
        }
    }
    AddSymbol(variable, SymbolNode::Type::CONSTANT, var_scope);
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::InQueryProcedureCall* node) {
    curr_procedure_name_ = std::get<std::string>(node->name());
    if (node->yield().has_value()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->yield().value());
    }
    for (auto& expr : node->args()) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(expr);
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::DummyNode* ) { NOT_SUPPORT(); }

std::any PatternGraphMaker::reportError() { return error_msg_; }

void PatternGraphMaker::AddSymbol(const std::string& symbol_alias, cypher::SymbolNode::Type type,
                                  cypher::SymbolNode::Scope scope) {
    auto& table = pattern_graphs_[cur_pattern_graph_].symbol_table.symbols;
    if (table.find(symbol_alias) != table.end()) return;
    table.emplace(symbol_alias, SymbolNode(symbols_idx_[cur_pattern_graph_]++, type, scope));
}

void PatternGraphMaker::AddNode(Node* node) {
    auto& pattern_graph = pattern_graphs_[cur_pattern_graph_];
    if (!pattern_graph.GetNode(node->Alias()).Empty()) {
        if (node->Visited()) pattern_graph.GetNode(node->Alias()).Visited() = true;
        return;
    }
    pattern_graph.AddNode(node);
}

void PatternGraphMaker::AddRelationship(Relationship* rel) {
    auto& pattern_graph = pattern_graphs_[cur_pattern_graph_];
    if (!pattern_graph.GetRelationship(rel->Alias()).Empty()) {
        return;
    }
    pattern_graph.AddRelationship(rel);
}

std::any PatternGraphMaker::visit(geax::frontend::RemoveSingleProperty* ) { NOT_SUPPORT(); }
std::any PatternGraphMaker::visit(geax::frontend::ListComprehension* node) {
    geax::frontend::Ref* ref = nullptr;
    checkedCast(node->getVariable(), ref);
    AddSymbol(ref->name(), cypher::SymbolNode::CONSTANT, cypher::SymbolNode::LOCAL);
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->getVariable());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->getInExpression());
    if (node->getWhereExpression() != nullptr) {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->getWhereExpression());
    }
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->getOpExpression());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any PatternGraphMaker::visit(geax::frontend::PredicateFunction* node) {
    geax::frontend::Ref* ref = nullptr;
    checkedCast(node->getVariable(), ref);
    AddSymbol(ref->name(), cypher::SymbolNode::CONSTANT, cypher::SymbolNode::LOCAL);
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->getVariable());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->getInExpression());
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->getWhereExpression());
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

}  // namespace cypher
