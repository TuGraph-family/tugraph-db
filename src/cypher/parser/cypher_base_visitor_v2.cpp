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
#include "cypher/parser/cypher_base_visitor_v2.h"
#include "cypher/utils/geax_util.h"
#include "cypher/parser/data_typedef.h"
#include "cypher/cypher_exception.h"
#include "geax-front-end/ast/AstNodeVisitor.h"
#include "cypher/parser/generated/LcypherParser.h"
#include "lgraph/lgraph_exceptions.h"

namespace parser {

#ifndef ALLOC_GEAOBJECT
#define ALLOC_GEAOBJECT(obj) objAlloc_.allocate<obj>()
#endif  // !ALLOC_GEAOBJECT

#ifndef SWITCH_CONTEXT_VISIT
#define SWITCH_CONTEXT_VISIT(ctx, current)     \
    ({                                         \
        geax::frontend::AstNode *prev = node_; \
        node_ = current;                       \
        ctx->accept(this);                     \
        node_ = prev;                          \
    })
#endif  // !SWITCH_CONTEXT_VISIT

#ifndef SWITCH_CONTEXT_VISIT_CHILDREN
#define SWITCH_CONTEXT_VISIT_CHILDREN(ctx, current) \
    ({                                              \
        geax::frontend::AstNode *prev = node_;      \
        node_ = current;                            \
        visitChildren(ctx);                         \
        node_ = prev;                               \
    })
#endif  // !SWITCH_CONTEXT_VISIT_CHILDREN

template <typename Base, typename Drive>
void checkedCast(Base *b, Drive *&d) {
    static_assert(std::is_base_of<Base, Drive>::value,
                  "type `Base` must be the base of type `Drive`");
    d = dynamic_cast<Drive *>(b);
    assert(d);
}

template <typename TargetType>
void checkedAnyCast(const std::any &s, TargetType *&d) {
    try {
        d = std::any_cast<TargetType *>(s);
    } catch (...) {
        // TODO(lingsu): remove in future
        assert(false);
    }
}
template <typename TargetType>
void checkedAnyCast(const std::any &s, TargetType &d) {
    try {
        d = std::any_cast<TargetType>(s);
    } catch (...) {
        // TODO(lingsu): remove in future
        assert(false);
    }
}

const std::unordered_map<std::string, geax::frontend::GeneralSetFunction>
    CypherBaseVisitorV2::S_AGG_LIST = {
        {"avg", geax::frontend::GeneralSetFunction::kAvg},
        {"count", geax::frontend::GeneralSetFunction::kCount},
        {"max", geax::frontend::GeneralSetFunction::kMax},
        {"min", geax::frontend::GeneralSetFunction::kMin},
        {"sum", geax::frontend::GeneralSetFunction::kSum},
        {"collect", geax::frontend::GeneralSetFunction::kCollect},
        {"stdDevSamp", geax::frontend::GeneralSetFunction::kStdDevSamp},
        {"stdDevPop", geax::frontend::GeneralSetFunction::kStdDevPop},
        {"groutConcat", geax::frontend::GeneralSetFunction::kGroupConcat},
        {"stDev", geax::frontend::GeneralSetFunction::kStDev},
        {"stDevP", geax::frontend::GeneralSetFunction::kStDevP},
        {"variance", geax::frontend::GeneralSetFunction::kVariance},
        {"varianceP", geax::frontend::GeneralSetFunction::kVarianceP}};

const std::unordered_map<std::string, geax::frontend::BinarySetFunction>
    CypherBaseVisitorV2::S_BAGG_LIST = {
        {"percentileCont", geax::frontend::BinarySetFunction::kPercentileCont},
        {"percentileDisc", geax::frontend::BinarySetFunction::kPercentileDisc},
};

std::string CypherBaseVisitorV2::GetFullText(antlr4::ParserRuleContext *ruleCtx) const {
    if (ruleCtx->children.size() == 0) {
        return "";
    }
    auto *startToken = ruleCtx->getStart();
    auto *stopToken = ruleCtx->getStop();
    antlr4::misc::Interval interval(startToken->getStartIndex(), stopToken->getStopIndex());
    return ruleCtx->getStart()->getInputStream()->getText(interval);
}

CypherBaseVisitorV2::CypherBaseVisitorV2(geax::common::ObjectArenaAllocator &objAlloc,
                                         antlr4::tree::ParseTree *tree)
    : objAlloc_(objAlloc),
      node_(ALLOC_GEAOBJECT(geax::frontend::NormalTransaction)),
      anonymous_idx_(0),
      visit_types_(),
      path_chain_(nullptr),
      filter_in_with_clause_(nullptr) {
    tree->accept(this);
}

std::any CypherBaseVisitorV2::visitOC_Cypher(LcypherParser::OC_CypherContext *ctx) {
    return visitChildren(ctx);
}

std::string CypherBaseVisitorV2::GenAnonymousAlias(bool is_node) {
    std::string alias(ANONYMOUS);
    if (is_node) {
        alias.append("N").append(std::to_string(anonymous_idx_));
    } else {
        alias.append("R").append(std::to_string(anonymous_idx_));
    }
    anonymous_idx_++;
    return alias;
}

// TODO(lingsu): support EXPLAIN
std::any CypherBaseVisitorV2::visitOC_Statement(LcypherParser::OC_StatementContext *ctx) {
    geax::frontend::NormalTransaction *node = nullptr;
    checkedCast(node_, node);
    cmd_type_ = ctx->EXPLAIN() ? parser::CmdType::EXPLAIN
                : ctx->PROFILE() ? parser::CmdType::PROFILE
                : parser::CmdType::QUERY;
    auto body = ALLOC_GEAOBJECT(geax::frontend::ProcedureBody);
    node->setProcedureBody(body);
    SWITCH_CONTEXT_VISIT_CHILDREN(ctx, body);
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Query(LcypherParser::OC_QueryContext *ctx) {
    return visitChildren(ctx);
    geax::frontend::ProcedureBody *node = nullptr;
    checkedCast(node_, node);
    auto headStmt = ALLOC_GEAOBJECT(geax::frontend::StatementWithYield);
    node->appendStatement(headStmt);
    SWITCH_CONTEXT_VISIT_CHILDREN(ctx, headStmt);
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_RegularQuery(LcypherParser::OC_RegularQueryContext *ctx) {
    geax::frontend::ProcedureBody *body = nullptr;
    checkedCast(node_, body);
    SWITCH_CONTEXT_VISIT(ctx->oC_SingleQuery(), body);
    if (!ctx->oC_Union().empty()) {
        VisitGuard guard(VisitType::kUnionClause, visit_types_);
        for (auto token : ctx->oC_Union()) {
            SWITCH_CONTEXT_VISIT(token, body);
        }
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Union(LcypherParser::OC_UnionContext *ctx) {
    return visitChildren(ctx);
}

std::any CypherBaseVisitorV2::visitOC_SingleQuery(LcypherParser::OC_SingleQueryContext *ctx) {
    if (ctx->oC_MultiPartQuery()) {
        return visit(ctx->oC_MultiPartQuery());
    } else {
        return visit(ctx->oC_SinglePartQuery());
    }
}

std::any CypherBaseVisitorV2::visitOC_SinglePartQuery(
    LcypherParser::OC_SinglePartQueryContext *ctx) {
    if (ctx->oC_ReadingClause().size() > 2) NOT_SUPPORT_AND_THROW();
    geax::frontend::ProcedureBody *body = nullptr;
    checkedCast(node_, body);
    geax::frontend::StatementWithYield* node;
    if (ctx->oC_UpdatingClause().empty()) {
        VisitGuard guard(VisitType::kReadingClause, visit_types_);
        auto l = ALLOC_GEAOBJECT(geax::frontend::AmbientLinearQueryStatement);
        if (!VisitGuard::InClause(VisitType::kUnionClause, visit_types_)) {
            node = ALLOC_GEAOBJECT(geax::frontend::StatementWithYield);
            body->appendStatement(node);
            auto stmt = ALLOC_GEAOBJECT(geax::frontend::QueryStatement);
            node->setStatement(stmt);
            auto join = ALLOC_GEAOBJECT(geax::frontend::JoinQueryExpression);
            stmt->setJoinQuery(join);
            auto co = ALLOC_GEAOBJECT(geax::frontend::CompositeQueryStatement);
            join->setHead(co);
            co->setHead(l);
        } else {
            node = body->statements().back();
            auto stmt = (geax::frontend::QueryStatement*)node->statement();
            stmt->joinQuery()->head()->appendBody(ALLOC_GEAOBJECT(geax::frontend::Union), l);
        }
        SWITCH_CONTEXT_VISIT_CHILDREN(ctx, l);
        if (VisitGuard::InClause(VisitType::kSinglePartQuery, visit_types_) &&
            filter_in_with_clause_) {
            l->appendQueryStatement(filter_in_with_clause_);
        }
    } else {
        VisitGuard guard(VisitType::kUpdatingClause, visit_types_);
        node = ALLOC_GEAOBJECT(geax::frontend::StatementWithYield);
        body->appendStatement(node);
        auto stmt = ALLOC_GEAOBJECT(geax::frontend::LinearDataModifyingStatement);
        node->setStatement(stmt);
        SWITCH_CONTEXT_VISIT_CHILDREN(ctx, stmt);
        if (VisitGuard::InClause(VisitType::kSinglePartQuery, visit_types_) &&
            filter_in_with_clause_) {
            stmt->appendQueryStatement(filter_in_with_clause_);
        }
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_MultiPartQuery(LcypherParser::OC_MultiPartQueryContext *ctx) {
    geax::frontend::ProcedureBody *body = nullptr;
    checkedCast(node_, body);
    std::vector<std::vector<antlr4::tree::ParseTree *>> clause;
    std::vector<antlr4::tree::ParseTree *> temp;
    for (auto child : ctx->children) {
        if (typeid(*child) == typeid(parser::LcypherParser::OC_ReadingClauseContext)) {
            temp.push_back(child);
        } else if (typeid(*child) == typeid(parser::LcypherParser::OC_UpdatingClauseContext)) {
            temp.push_back(child);
        } else if (typeid(*child) == typeid(parser::LcypherParser::OC_WithContext)) {
            temp.push_back(child);
            clause.emplace_back(temp);
            temp.clear();
        }
    }
    if (ctx->oC_UpdatingClause().empty()) {
        VisitGuard guard(VisitType::kReadingClause, visit_types_);
        for (auto &level_clause : clause) {
            auto node = ALLOC_GEAOBJECT(geax::frontend::StatementWithYield);
            body->appendStatement(node);
            auto stmt = ALLOC_GEAOBJECT(geax::frontend::QueryStatement);
            node->setStatement(stmt);
            auto join = ALLOC_GEAOBJECT(geax::frontend::JoinQueryExpression);
            stmt->setJoinQuery(join);
            auto co = ALLOC_GEAOBJECT(geax::frontend::CompositeQueryStatement);
            join->setHead(co);
            auto l = ALLOC_GEAOBJECT(geax::frontend::AmbientLinearQueryStatement);
            co->setHead(l);
            for (auto c : level_clause) {
                SWITCH_CONTEXT_VISIT(c, l);
            }
        }
    } else {
        VisitGuard guard(VisitType::kUpdatingClause, visit_types_);
        for (auto &level_clause : clause) {
            auto node = ALLOC_GEAOBJECT(geax::frontend::StatementWithYield);
            body->appendStatement(node);
            auto stmt = ALLOC_GEAOBJECT(geax::frontend::LinearDataModifyingStatement);
            node->setStatement(stmt);
            for (auto c : level_clause) {
                SWITCH_CONTEXT_VISIT(c, stmt);
            }
        }
    }
    VisitGuard guard(VisitType::kSinglePartQuery, visit_types_);
    SWITCH_CONTEXT_VISIT(ctx->oC_SinglePartQuery(), body);
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_UpdatingClause(LcypherParser::OC_UpdatingClauseContext *ctx) {
    return visitChildren(ctx);
}

std::any CypherBaseVisitorV2::visitOC_ReadingClause(LcypherParser::OC_ReadingClauseContext *ctx) {
    return visitChildren(ctx);
}

std::any CypherBaseVisitorV2::visitOC_Match(LcypherParser::OC_MatchContext *ctx) {
    if (VisitGuard::InClause(VisitType::kReadingClause, visit_types_)) {
        geax::frontend::AmbientLinearQueryStatement *node = nullptr;
        checkedCast(node_, node);
        auto match = ALLOC_GEAOBJECT(geax::frontend::MatchStatement);
        node->appendQueryStatement(match);
        if (ctx->OPTIONAL_()) match->setStatementMode(geax::frontend::StatementMode::kOptional);
        auto graph_pattern = ALLOC_GEAOBJECT(geax::frontend::GraphPattern);
        match->setGraphPattern(graph_pattern);
        VisitGuard guard(VisitType::kReadingPattern, visit_types_);
        SWITCH_CONTEXT_VISIT(ctx->oC_Pattern(), graph_pattern);
        if (ctx->oC_Where()) {
            SWITCH_CONTEXT_VISIT(ctx->oC_Where(), graph_pattern);
        }

    } else if (VisitGuard::InClause(VisitType::kUpdatingClause, visit_types_)) {
        geax::frontend::LinearDataModifyingStatement *node = nullptr;
        checkedCast(node_, node);
        VisitGuard guard(VisitType::kMatchPattern, visit_types_);
        auto match = ALLOC_GEAOBJECT(geax::frontend::MatchStatement);
        node->appendQueryStatement(match);
        auto graph_pattern = ALLOC_GEAOBJECT(geax::frontend::GraphPattern);
        match->setGraphPattern(graph_pattern);
        SWITCH_CONTEXT_VISIT(ctx->oC_Pattern(), graph_pattern);
        if (ctx->oC_Where()) {
            SWITCH_CONTEXT_VISIT(ctx->oC_Where(), graph_pattern);
        }
    }

    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Unwind(LcypherParser::OC_UnwindContext *ctx) {
    geax::frontend::UnwindStatement *unwind = nullptr;
    if (VisitGuard::InClause(VisitType::kReadingClause, visit_types_)) {
        geax::frontend::AmbientLinearQueryStatement *node = nullptr;
        checkedCast(node_, node);
        unwind = ALLOC_GEAOBJECT(geax::frontend::UnwindStatement);
        node->appendQueryStatement(unwind);

    } else if (VisitGuard::InClause(VisitType::kUpdatingClause, visit_types_)) {
        geax::frontend::LinearDataModifyingStatement *node = nullptr;
        checkedCast(node_, node);
        unwind = ALLOC_GEAOBJECT(geax::frontend::UnwindStatement);
        node->appendQueryStatement(unwind);
    }
    geax::frontend::Expr *expr = nullptr;
    checkedAnyCast(visit(ctx->oC_Expression()), expr);
    unwind->setList(expr);
    geax::frontend::Expr *name_expr = nullptr;
    checkedAnyCast(visit(ctx->oC_Variable()), name_expr);
    geax::frontend::VString *vstr = nullptr;
    checkedCast(name_expr, vstr);
    std::string variable = vstr->val();
    unwind->setVariable(std::move(variable));
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Merge(LcypherParser::OC_MergeContext *ctx) {
    geax::frontend::LinearDataModifyingStatement *node = nullptr;
    checkedCast(node_, node);
    auto merge = ALLOC_GEAOBJECT(geax::frontend::MergeStatement);
    node->appendModifyStatement(merge);
    auto path_pattern = ALLOC_GEAOBJECT(geax::frontend::PathPattern);
    merge->setPathPattern(path_pattern);
    VisitGuard guard(VisitType::kMergeClause, visit_types_);
    SWITCH_CONTEXT_VISIT(ctx->oC_PatternPart(), path_pattern);
    for (auto action : ctx->oC_MergeAction()) {
        SWITCH_CONTEXT_VISIT(action, merge);
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_MergeAction(LcypherParser::OC_MergeActionContext *ctx) {
    if (ctx->MATCH()) {
        VisitGuard guard(VisitType::kMergeOnMatch, visit_types_);
        return visit(ctx->oC_Set());
    } else if (ctx->CREATE()) {
        VisitGuard guard(VisitType::kMergeOnCreate, visit_types_);
        return visit(ctx->oC_Set());
    }
    NOT_SUPPORT_AND_THROW();
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Create(LcypherParser::OC_CreateContext *ctx) {
    if (VisitGuard::InClause(VisitType::kReadingClause, visit_types_)) {
        NOT_SUPPORT_AND_THROW();
    } else if (VisitGuard::InClause(VisitType::kUpdatingClause, visit_types_)) {
        geax::frontend::LinearDataModifyingStatement *node = nullptr;
        checkedCast(node_, node);
        auto insert = ALLOC_GEAOBJECT(geax::frontend::InsertStatement);
        node->appendModifyStatement(insert);
        VisitGuard guard(VisitType::kUpdatingPattern, visit_types_);
        SWITCH_CONTEXT_VISIT(ctx->oC_Pattern(), insert);
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Set(LcypherParser::OC_SetContext *ctx) {
    if (VisitGuard::InClause(VisitType::kMergeClause, visit_types_)) {
        geax::frontend::MergeStatement *node = nullptr;
        checkedCast(node_, node);
        if (VisitGuard::InClause(VisitType::kMergeOnMatch, visit_types_)) {
            for (auto &item : ctx->oC_SetItem()) {
                auto stmt = ALLOC_GEAOBJECT(geax::frontend::SetStatement);
                node->appendOnMatch(stmt);
                SWITCH_CONTEXT_VISIT(item, stmt);
            }
        } else if (VisitGuard::InClause(VisitType::kMergeOnCreate, visit_types_)) {
            for (auto &item : ctx->oC_SetItem()) {
                auto stmt = ALLOC_GEAOBJECT(geax::frontend::SetStatement);
                node->appendOnCreate(stmt);
                SWITCH_CONTEXT_VISIT(item, stmt);
            }
        }
    } else if (VisitGuard::InClause(VisitType::kUpdatingClause, visit_types_)) {
        geax::frontend::LinearDataModifyingStatement *node = nullptr;
        checkedCast(node_, node);
        for (auto &item : ctx->oC_SetItem()) {
            auto stmt = ALLOC_GEAOBJECT(geax::frontend::SetStatement);
            node->appendModifyStatement(stmt);
            SWITCH_CONTEXT_VISIT(item, stmt);
        }
    } else {
        NOT_SUPPORT_AND_THROW();
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_SetItem(LcypherParser::OC_SetItemContext *ctx) {
    geax::frontend::SetStatement *node = nullptr;
    checkedCast(node_, node);
    if (ctx->oC_PropertyExpression()) {
        auto update = ALLOC_GEAOBJECT(geax::frontend::UpdateProperties);
        node->appendItem(update);
        VisitGuard guard(VisitType::kSetVariable, visit_types_);
        SWITCH_CONTEXT_VISIT(ctx->oC_PropertyExpression(), update);
        geax::frontend::Expr *expr = nullptr;
        VisitGuard guardNull(VisitType::kSetNull, visit_types_);
        checkedAnyCast(visit(ctx->oC_Expression()), expr);
        auto &prop = update->structs()->properties();
        // so ugly！！！
        geax::frontend::Expr **src = const_cast<geax::frontend::Expr **>(&(std::get<1>(prop[0])));
        *src = expr;
    } else {
        geax::frontend::Expr *name_expr = nullptr;
        checkedAnyCast(visit(ctx->oC_Variable()), name_expr);
        geax::frontend::VString *vstr = nullptr;
        checkedCast(name_expr, vstr);
        std::string variable = vstr->val();
        auto update = ALLOC_GEAOBJECT(geax::frontend::UpdateProperties);
        node->appendItem(update);
        update->setV(std::move(variable));
        if (ctx->oC_Expression()) {
            VisitGuard guard(VisitType::kSetVariable, visit_types_);
            VisitGuard guardLabel(VisitType::kSetLabel, visit_types_);
            SWITCH_CONTEXT_VISIT(ctx->oC_Expression(), update);
        } else {
            THROW_CODE(InputError, FMA_FMT("Variable `{}` not defined", vstr->val()));
        }
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Delete(LcypherParser::OC_DeleteContext *ctx) {
    if (VisitGuard::InClause(VisitType::kReadingClause, visit_types_)) {
        NOT_SUPPORT_AND_THROW();
    } else if (VisitGuard::InClause(VisitType::kUpdatingClause, visit_types_)) {
        geax::frontend::LinearDataModifyingStatement *node = nullptr;
        checkedCast(node_, node);
        auto del = ALLOC_GEAOBJECT(geax::frontend::DeleteStatement);
        node->appendModifyStatement(del);
        for (auto e : ctx->oC_Expression()) {
            VisitGuard guard(VisitType::kDeleteVariable, visit_types_);
            geax::frontend::Expr *expr = nullptr;
            checkedAnyCast(visit(e), expr);
            if (expr->type() != geax::frontend::AstNodeType::kRef) {
                THROW_CODE(InputError, "Type mismatch: expected Node, Path or Relationship");
            }
            std::string field = ((geax::frontend::Ref *)expr)->name();
            del->appendItem(std::move(field));
        }
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Remove(LcypherParser::OC_RemoveContext *ctx) {
    VisitGuard::InClause(VisitType::kUpdatingClause, visit_types_);
    geax::frontend::LinearDataModifyingStatement *node = nullptr;
    checkedCast(node_, node);
    for (auto &item : ctx->oC_RemoveItem()) {
        auto stmt = ALLOC_GEAOBJECT(geax::frontend::RemoveStatement);
        node->appendModifyStatement(stmt);
        SWITCH_CONTEXT_VISIT(item, stmt);
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_RemoveItem(LcypherParser::OC_RemoveItemContext *ctx) {
    geax::frontend::RemoveStatement *node = nullptr;
    checkedCast(node_, node);
    if (ctx->oC_PropertyExpression()) {
        geax::frontend::Expr *name_expr = nullptr, *property_expr = nullptr;
        auto pe_ctx = ctx->oC_PropertyExpression();
        checkedAnyCast(visit(ctx->oC_PropertyExpression()->oC_Atom()), name_expr);
        if (pe_ctx->oC_PropertyLookup().empty())
            CYPHER_TODO();
        checkedAnyCast(visit(pe_ctx->oC_PropertyLookup(0)), property_expr);
        geax::frontend::Ref *vstr = nullptr;
        geax::frontend::VString *pstr = nullptr;
        checkedCast(name_expr, vstr);
        checkedCast(property_expr, pstr);
        std::string variable = vstr->name(), property = pstr->val();
        auto remove = ALLOC_GEAOBJECT(geax::frontend::RemoveSingleProperty);
        node->appendItem(remove);
        remove->setV(std::move(variable));
        remove->setProperty(std::move(property));
    } else {
        CYPHER_TODO();
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_InQueryCall(LcypherParser::OC_InQueryCallContext *ctx) {
    geax::frontend::AmbientLinearQueryStatement *node = nullptr;
    checkedCast(node_, node);
    auto query_call = ALLOC_GEAOBJECT(geax::frontend::CallQueryStatement);
    node->appendQueryStatement(query_call);
    auto procedure = ALLOC_GEAOBJECT(geax::frontend::CallProcedureStatement);
    query_call->setProcedureStatement(procedure);
    auto inquery_query = ALLOC_GEAOBJECT(geax::frontend::InQueryProcedureCall);
    procedure->setProcedureCall(inquery_query);
    VisitGuard guard(VisitType::kInQueryCall, visit_types_);
    SWITCH_CONTEXT_VISIT(ctx->oC_ExplicitProcedureInvocation(), inquery_query);
    if (ctx->oC_YieldItems()) {
        auto yield_fields = ALLOC_GEAOBJECT(geax::frontend::YieldField);
        inquery_query->setYield(yield_fields);
        SWITCH_CONTEXT_VISIT(ctx->oC_YieldItems(), yield_fields);
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_StandaloneCall(LcypherParser::OC_StandaloneCallContext *ctx) {
    geax::frontend::ProcedureBody *body = nullptr;
    checkedCast(node_, body);
    auto node = ALLOC_GEAOBJECT(geax::frontend::StatementWithYield);
    body->appendStatement(node);
    auto stand = ALLOC_GEAOBJECT(geax::frontend::StandaloneCallStatement);
    node->setStatement(stand);
    auto procedure = ALLOC_GEAOBJECT(geax::frontend::CallProcedureStatement);
    stand->setProcedureStatement(procedure);
    auto named_procedure = ALLOC_GEAOBJECT(geax::frontend::NamedProcedureCall);
    procedure->setProcedureCall(named_procedure);
    VisitGuard guard(VisitType::kStandaloneCall, visit_types_);
    if (ctx->oC_ExplicitProcedureInvocation()) {
        SWITCH_CONTEXT_VISIT(ctx->oC_ExplicitProcedureInvocation(), named_procedure);
    } else if (ctx->oC_ImplicitProcedureInvocation()) {
        SWITCH_CONTEXT_VISIT(ctx->oC_ImplicitProcedureInvocation(), named_procedure);
    }
    if (ctx->oC_YieldItems()) {
        auto yield_fields = ALLOC_GEAOBJECT(geax::frontend::YieldField);
        named_procedure->setYield(yield_fields);
        SWITCH_CONTEXT_VISIT(ctx->oC_YieldItems(), yield_fields);
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_YieldItems(LcypherParser::OC_YieldItemsContext *ctx) {
    if (ctx->oC_Where()) {
        visit(ctx->oC_Where());
    }
    for (auto &item : ctx->oC_YieldItem()) {
        visit(item);
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_YieldItem(LcypherParser::OC_YieldItemContext *ctx) {
    if (ctx->AS()) NOT_SUPPORT_AND_THROW();
    geax::frontend::YieldField *yield_fields = nullptr;
    checkedCast(node_, yield_fields);
    yield_fields->appendItem(ctx->oC_Variable()->getText(), ctx->oC_Variable()->getText());
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_With(LcypherParser::OC_WithContext *ctx) {
    if (VisitGuard::InClause(VisitType::kReadingClause, visit_types_)) {
        geax::frontend::AmbientLinearQueryStatement *node = nullptr;
        checkedCast(node_, node);
        auto result = ALLOC_GEAOBJECT(geax::frontend::PrimitiveResultStatement);
        result->setDistinct(ctx->DISTINCT() != nullptr);
        node->setResultStatement(result);
        VisitGuard guard(VisitType::kWithClause, visit_types_);
        SWITCH_CONTEXT_VISIT(ctx->oC_ReturnBody(), result);
        if (ctx->oC_Where()) {
            SWITCH_CONTEXT_VISIT(ctx->oC_Where(), node);
        }
    } else if (VisitGuard::InClause(VisitType::kUpdatingClause, visit_types_)) {
        geax::frontend::LinearDataModifyingStatement *node = nullptr;
        checkedCast(node_, node);
        auto result = ALLOC_GEAOBJECT(geax::frontend::PrimitiveResultStatement);
        result->setDistinct(ctx->DISTINCT() != nullptr);
        node->setResultStatement(result);
        VisitGuard guard(VisitType::kWithClause, visit_types_);
        SWITCH_CONTEXT_VISIT(ctx->oC_ReturnBody(), result);
        if (ctx->oC_Where()) {
            SWITCH_CONTEXT_VISIT(ctx->oC_Where(), node);
        }
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Return(LcypherParser::OC_ReturnContext *ctx) {
    if (VisitGuard::InClause(VisitType::kReadingClause, visit_types_)) {
        geax::frontend::AmbientLinearQueryStatement *node = nullptr;
        checkedCast(node_, node);
        auto result = ALLOC_GEAOBJECT(geax::frontend::PrimitiveResultStatement);
        result->setDistinct(ctx->DISTINCT() != nullptr);
        node->setResultStatement(result);
        SWITCH_CONTEXT_VISIT(ctx->oC_ReturnBody(), result);
    } else if (VisitGuard::InClause(VisitType::kUpdatingClause, visit_types_)) {
        geax::frontend::LinearDataModifyingStatement *node = nullptr;
        checkedCast(node_, node);
        auto result = ALLOC_GEAOBJECT(geax::frontend::PrimitiveResultStatement);
        result->setDistinct(ctx->DISTINCT() != nullptr);
        node->setResultStatement(result);
        SWITCH_CONTEXT_VISIT(ctx->oC_ReturnBody(), result);
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_ReturnBody(LcypherParser::OC_ReturnBodyContext *ctx) {
    visit(ctx->oC_ReturnItems());
    if (ctx->oC_Order()) {
        visit(ctx->oC_Order());
    }
    if (ctx->oC_Limit()) {
        visit(ctx->oC_Limit());
    }
    if (ctx->oC_Skip()) {
        visit(ctx->oC_Skip());
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_ReturnItems(LcypherParser::OC_ReturnItemsContext *ctx) {
    if (VisitGuard::InClause(VisitType::kWithClause, visit_types_)) {
        if (ctx->children[0]->getText() == "*")
            THROW_CODE(CypherException, "WITH * syntax is not implemented now");
    }
    for (auto &item : ctx->oC_ReturnItem()) {
        visit(item);
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_ReturnItem(LcypherParser::OC_ReturnItemContext *ctx) {
    geax::frontend::PrimitiveResultStatement *node = nullptr;
    checkedCast(node_, node);
    geax::frontend::Expr *expr = nullptr;
    checkedAnyCast(visit(ctx->oC_Expression()), expr);
    std::string variable;
    if (ctx->oC_Variable()) {
        variable = ctx->oC_Variable()->getText();
    } else {
        if (VisitGuard::InClause(VisitType::kWithClause, visit_types_)) {
            geax::frontend::Expr *with_expr = nullptr;
            checkedAnyCast(visit(ctx->oC_Expression()), with_expr);
            if (with_expr->type() != geax::frontend::AstNodeType::kRef) {
                THROW_CODE(ParserException, "Non-variable expression in WITH must be aliased");
            }
        }
        variable = GetFullText(ctx->oC_Expression());
    }
    node->appendItem(std::move(variable), expr);
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Order(LcypherParser::OC_OrderContext *ctx) {
    for (size_t idx = 0; idx < ctx->oC_SortItem().size(); ++idx) {
        visit(ctx->oC_SortItem(idx));
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Skip(LcypherParser::OC_SkipContext *ctx) {
    geax::frontend::PrimitiveResultStatement *node = nullptr;
    checkedCast(node_, node);
    geax::frontend::Expr *expr = nullptr;
    checkedAnyCast(visit(ctx->oC_Expression()), expr);
    geax::frontend::VInt *integer = nullptr;
    checkedCast(expr, integer);
    node->setOffset(integer->val());
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Limit(LcypherParser::OC_LimitContext *ctx) {
    geax::frontend::PrimitiveResultStatement *node = nullptr;
    checkedCast(node_, node);
    geax::frontend::Expr *expr = nullptr;
    checkedAnyCast(visit(ctx->oC_Expression()), expr);
    geax::frontend::VInt *integer = nullptr;
    checkedCast(expr, integer);
    node->setLimit(integer->val());
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_SortItem(LcypherParser::OC_SortItemContext *ctx) {
    geax::frontend::PrimitiveResultStatement *node = nullptr;
    checkedCast(node_, node);
    auto order = ALLOC_GEAOBJECT(geax::frontend::OrderByField);
    node->appendOrderBy(order);
    geax::frontend::Expr *expr = nullptr;
    checkedAnyCast(visit(ctx->oC_Expression()), expr);
    bool ascending = ctx->ASCENDING() != nullptr || ctx->ASC() != nullptr ||
                     (ctx->DESCENDING() == nullptr && ctx->DESC() == nullptr);
    order->setField(expr);
    order->setOrder(!ascending);
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Hint(LcypherParser::OC_HintContext *ctx) {
    NOT_SUPPORT_AND_THROW();
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Where(LcypherParser::OC_WhereContext *ctx) {
    if (VisitGuard::InClause(VisitType::kWithClause, visit_types_)) {
        if (VisitGuard::InClause(VisitType::kReadingClause, visit_types_) ||
            VisitGuard::InClause(VisitType::kUpdatingClause, visit_types_)) {
            filter_in_with_clause_ = ALLOC_GEAOBJECT(geax::frontend::FilterStatement);
            geax::frontend::Expr *where = nullptr;
            checkedAnyCast(visit(ctx->oC_Expression()), where);
            filter_in_with_clause_->setPredicate(where);
        } else {
            NOT_SUPPORT_AND_THROW();
        }
    } else if (VisitGuard::InClause(VisitType::kInQueryCall, visit_types_)) {
        geax::frontend::YieldField *yield_fields = nullptr;
        checkedCast(node_, yield_fields);
        geax::frontend::Expr *expr = nullptr;
        checkedAnyCast(visit(ctx->oC_Expression()), expr);
        yield_fields->setPredicate(expr);
    } else {
        geax::frontend::GraphPattern *node = nullptr;
        checkedCast(node_, node);
        geax::frontend::Expr *where = nullptr;
        checkedAnyCast(visit(ctx->oC_Expression()), where);
        node->setWhere(where);
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Pattern(LcypherParser::OC_PatternContext *ctx) {
    if (VisitGuard::InClause(VisitType::kReadingPattern, visit_types_)) {
        geax::frontend::GraphPattern *node = nullptr;
        checkedCast(node_, node);
        for (auto &ctx_part : ctx->oC_PatternPart()) {
            auto pp = ALLOC_GEAOBJECT(geax::frontend::PathPattern);
            node->appendPathPattern(pp);
            SWITCH_CONTEXT_VISIT(ctx_part, pp);
        }
    } else if (VisitGuard::InClause(VisitType::kUpdatingPattern, visit_types_)) {
        for (auto &ctx_part : ctx->oC_PatternPart()) {
            visit(ctx_part);
        }
    } else if (VisitGuard::InClause(VisitType::kMatchPattern, visit_types_)) {
        geax::frontend::GraphPattern *node = nullptr;
        checkedCast(node_, node);
        for (auto &ctx_part : ctx->oC_PatternPart()) {
            auto pp = ALLOC_GEAOBJECT(geax::frontend::PathPattern);
            node->appendPathPattern(pp);
            SWITCH_CONTEXT_VISIT(ctx_part, pp);
        }
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_PatternPart(LcypherParser::OC_PatternPartContext *ctx) {
    if (VisitGuard::InClause(VisitType::kReadingPattern, visit_types_) ||
        VisitGuard::InClause(VisitType::kMergeClause, visit_types_)) {
        geax::frontend::PathPattern *node = nullptr;
        checkedCast(node_, node);
        if (ctx->oC_Variable() != nullptr) {
            // named path
            geax::frontend::Expr *name_expr = nullptr;
            checkedAnyCast(visit(ctx->oC_Variable()), name_expr);
            geax::frontend::VString *vstr = nullptr;
            checkedCast(name_expr, vstr);
            std::string variable = vstr->val();
            node->setAlias(std::move(variable));
        }
        auto pc = ALLOC_GEAOBJECT(geax::frontend::PathChain);
        node->appendChain(pc);
        SWITCH_CONTEXT_VISIT(ctx->oC_AnonymousPatternPart(), pc);
    } else if (VisitGuard::InClause(VisitType::kUpdatingPattern, visit_types_)) {
        geax::frontend::InsertStatement *node = nullptr;
        checkedCast(node_, node);
        auto pc = ALLOC_GEAOBJECT(geax::frontend::PathChain);
        node->appendPath(pc);
        SWITCH_CONTEXT_VISIT(ctx->oC_AnonymousPatternPart(), pc);
    } else if (VisitGuard::InClause(VisitType::kMatchPattern, visit_types_)) {
        geax::frontend::PathPattern *node = nullptr;
        checkedCast(node_, node);
        if (ctx->oC_Variable() != nullptr) {
            // named path
            geax::frontend::Expr *name_expr = nullptr;
            checkedAnyCast(visit(ctx->oC_Variable()), name_expr);
            geax::frontend::VString *vstr = nullptr;
            checkedCast(name_expr, vstr);
            std::string variable = vstr->val();
            node->setAlias(std::move(variable));
        }
        auto pc = ALLOC_GEAOBJECT(geax::frontend::PathChain);
        node->appendChain(pc);
        SWITCH_CONTEXT_VISIT(ctx->oC_AnonymousPatternPart(), pc);
    } else {
        NOT_SUPPORT_AND_THROW();
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_AnonymousPatternPart(
    LcypherParser::OC_AnonymousPatternPartContext *ctx) {
    return visit(ctx->oC_PatternElement());
}

std::any CypherBaseVisitorV2::visitOC_PatternElement(LcypherParser::OC_PatternElementContext *ctx) {
    if (ctx->oC_NodePattern() == nullptr) NOT_SUPPORT_AND_THROW();
    geax::frontend::PathChain *pathChain = nullptr;
    checkedCast(node_, pathChain);
    auto node = ALLOC_GEAOBJECT(geax::frontend::Node);
    pathChain->setHead(node);
    SWITCH_CONTEXT_VISIT(ctx->oC_NodePattern(), node);
    for (auto &chain : ctx->oC_PatternElementChain()) {
        SWITCH_CONTEXT_VISIT(chain, pathChain);
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_NodePattern(LcypherParser::OC_NodePatternContext *ctx) {
    geax::frontend::Node *node = nullptr;
    checkedCast(node_, node);
    auto filler = ALLOC_GEAOBJECT(geax::frontend::ElementFiller);
    node->setFiller(filler);
    if (ctx->oC_Variable() != nullptr) {
        geax::frontend::Expr *name_expr = nullptr;
        checkedAnyCast(visit(ctx->oC_Variable()), name_expr);
        geax::frontend::VString *vstr = nullptr;
        checkedCast(name_expr, vstr);
        std::string alias = vstr->val();
        filler->setV(std::move(alias));
    } else {
        // if alias is absent, generate an alias for the node
        filler->setV(GenAnonymousAlias(true));
    }

    if (ctx->oC_NodeLabels() != nullptr) {
        std::vector<std::string> labels;
        checkedAnyCast(visit(ctx->oC_NodeLabels()), labels);
        if (labels.size() == 1) {
            auto l = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
            l->setLabel(std::move(labels[0]));
            filler->setLabel(l);
        } else if (labels.size() == 2) {
            auto root = ALLOC_GEAOBJECT(geax::frontend::LabelOr);
            auto l = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
            auto r = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
            l->setLabel(std::move(labels[0]));
            r->setLabel(std::move(labels[1]));
            root->setLeft(l);
            root->setRight(r);
            filler->setLabel(root);
        } else {
            auto root = ALLOC_GEAOBJECT(geax::frontend::LabelOr);
            auto label = root;
            for (size_t idx = 0; idx < labels.size() - 2; ++idx) {
                auto o = ALLOC_GEAOBJECT(geax::frontend::LabelOr);
                auto l = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
                l->setLabel(std::move(labels[idx]));
                label->setLeft(l);
                label->setRight(o);
                label = o;
            }
            auto l = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
            auto r = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
            l->setLabel(std::move(labels[labels.size() - 2]));
            r->setLabel(std::move(labels[labels.size() - 1]));
            label->setLeft(l);
            label->setRight(r);
            filler->setLabel(root);
        }
    }

    if (ctx->oC_Properties() != nullptr) {
        SWITCH_CONTEXT_VISIT(ctx->oC_Properties(), filler);
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_PatternElementChain(
    LcypherParser::OC_PatternElementChainContext *ctx) {
    geax::frontend::PathChain *pathChain = nullptr;
    checkedCast(node_, pathChain);
    if (ctx->oC_RelationshipPattern() && ctx->oC_NodePattern()) {
        auto edge = ALLOC_GEAOBJECT(geax::frontend::Edge);
        SWITCH_CONTEXT_VISIT(ctx->oC_RelationshipPattern(), edge);
        auto node = ALLOC_GEAOBJECT(geax::frontend::Node);
        SWITCH_CONTEXT_VISIT(ctx->oC_NodePattern(), node);
        pathChain->appendTail(edge, node);
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_RelationshipPattern(
    LcypherParser::OC_RelationshipPatternContext *ctx) {
    geax::frontend::Edge *edge = nullptr;
    checkedCast(node_, edge);
    geax::frontend::EdgeDirection direction;
    if (ctx->oC_LeftArrowHead() != nullptr && ctx->oC_RightArrowHead() == nullptr) {
        direction = geax::frontend::EdgeDirection::kPointLeft;
    } else if (ctx->oC_RightArrowHead() != nullptr && ctx->oC_LeftArrowHead() == nullptr) {
        direction = geax::frontend::EdgeDirection::kPointRight;
    } else {
        direction = geax::frontend::EdgeDirection::kAnyDirected;
    }
    edge->setDirection(direction);
    if (ctx->oC_RelationshipDetail()) {
        geax::frontend::AstNode *prev = node_;
        visit(ctx->oC_RelationshipDetail());
        node_ = prev;
    } else {
        auto filler = ALLOC_GEAOBJECT(geax::frontend::ElementFiller);
        edge->setFiller(filler);
        std::string variable = GenAnonymousAlias(false);
        filler->setV(std::move(variable));
        edge->setHopRange(-1, -1);
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_RelationshipDetail(
    LcypherParser::OC_RelationshipDetailContext *ctx) {
    geax::frontend::Edge *edge = nullptr;
    checkedCast(node_, edge);
    auto filler = ALLOC_GEAOBJECT(geax::frontend::ElementFiller);
    edge->setFiller(filler);

    std::string variable;
    if (ctx->oC_Variable() != nullptr) {
        geax::frontend::Expr *name_expr = nullptr;
        checkedAnyCast(visit(ctx->oC_Variable()), name_expr);
        geax::frontend::VString *vstr = nullptr;
        checkedCast(name_expr, vstr);
        variable = vstr->val();
    } else {
        // if alias is absent, generate an alias for the relationship
        variable = GenAnonymousAlias(false);
    }
    filler->setV(std::move(variable));

    if (ctx->oC_RelationshipTypes() != nullptr) {
        SWITCH_CONTEXT_VISIT(ctx->oC_RelationshipTypes(), filler);
    }

    if (ctx->oC_Properties() != nullptr) {
        SWITCH_CONTEXT_VISIT(ctx->oC_Properties(), filler);
    }

    if (ctx->oC_RangeLiteral() != nullptr) {
        SWITCH_CONTEXT_VISIT(ctx->oC_RangeLiteral(), edge);
    }

    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Properties(LcypherParser::OC_PropertiesContext *ctx) {
    if (ctx->oC_MapLiteral() == nullptr) {
        NOT_SUPPORT_AND_THROW();
    }
    if (ctx->oC_MapLiteral()) {
        return visit(ctx->oC_MapLiteral());
    }
    if (ctx->oC_Parameter()) {
        NOT_SUPPORT_AND_THROW();
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_RelationshipTypes(
    LcypherParser::OC_RelationshipTypesContext *ctx) {
    geax::frontend::ElementFiller *node = nullptr;
    checkedCast(node_, node);
    std::vector<std::string> label_strs;
    for (auto &ctx_label : ctx->oC_RelTypeName()) {
        geax::frontend::Expr *name_expr = nullptr;
        checkedAnyCast(visit(ctx_label), name_expr);
        geax::frontend::VString *vstr = nullptr;
        checkedCast(name_expr, vstr);
        std::string name = vstr->val();
        label_strs.emplace_back(name);
    }
    if (label_strs.size() == 1) {
        auto l = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
        l->setLabel(std::move(label_strs[0]));
        node->setLabel(l);
        return 0;
    }
    if (label_strs.size() == 2) {
        auto root = ALLOC_GEAOBJECT(geax::frontend::LabelOr);
        auto l = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
        l->setLabel(std::move(label_strs[0]));
        auto r = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
        r->setLabel(std::move(label_strs[1]));
        root->setLeft(l);
        root->setRight(r);
        node->setLabel(root);
        return 0;
    }
    auto root = ALLOC_GEAOBJECT(geax::frontend::LabelOr);
    auto label = root;
    for (size_t idx = 0; idx < label_strs.size() - 2; ++idx) {
        auto o = ALLOC_GEAOBJECT(geax::frontend::LabelOr);
        auto l = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
        l->setLabel(std::move(label_strs[idx]));
        label->setLeft(l);
        label->setRight(o);
        label = o;
    }
    auto l = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
    l->setLabel(std::move(label_strs[label_strs.size() - 2]));
    auto r = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
    r->setLabel(std::move(label_strs[label_strs.size() - 1]));
    label->setLeft(l);
    label->setRight(r);
    node->setLabel(root);
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_NodeLabels(LcypherParser::OC_NodeLabelsContext *ctx) {
    std::vector<std::string> label_strs;
    for (auto &ctx_label : ctx->oC_NodeLabel()) {
        geax::frontend::Expr *name_expr = nullptr;
        checkedAnyCast(visit(ctx_label), name_expr);
        geax::frontend::VString *vstr = nullptr;
        checkedCast(name_expr, vstr);
        std::string name = vstr->val();
        label_strs.emplace_back(name);
    }
    return label_strs;
}

std::any CypherBaseVisitorV2::visitOC_NodeLabel(LcypherParser::OC_NodeLabelContext *ctx) {
    return visit(ctx->oC_LabelName());
}

std::any CypherBaseVisitorV2::visitOC_RangeLiteral(LcypherParser::OC_RangeLiteralContext *ctx) {
    geax::frontend::Edge *edge = nullptr;
    checkedCast(node_, edge);
    int low = 0, hight = 0;
    std::vector<antlr4::tree::ParseTree *> valid_children;
    for (const auto &it : ctx->children) {
        const auto &text = it->getText();
        if (!std::all_of(text.cbegin(), text.cend(), ::isspace)) {
            valid_children.emplace_back(it);
        }
    }
    switch (valid_children.size()) {
    case 2:
        {
            if (!ctx->oC_IntegerLiteral().empty()) {
                low = std::stoi(ctx->oC_IntegerLiteral(0)->getText());
                hight = low;
            } else {
                CYPHER_THROW_ASSERT(valid_children.at(1)->getText() == "..");
                low = 1;
                hight = VAR_LEN_EXPAND_MAX_HOP;
            }
            break;
        }
    case 3:
        {
            if (valid_children.at(2)->getText() == "..") {
                low = std::stoi(ctx->oC_IntegerLiteral(0)->getText());
                hight = VAR_LEN_EXPAND_MAX_HOP;
            } else {
                CYPHER_THROW_ASSERT(valid_children.at(1)->getText() == "..");
                low = 1;
                hight = std::stoi(ctx->oC_IntegerLiteral(0)->getText());
            }
            break;
        }
    case 4:
        {
            CYPHER_THROW_ASSERT(ctx->oC_IntegerLiteral().size() == 2);
            low = std::stoi(ctx->oC_IntegerLiteral(0)->getText());
            hight = std::stoi(ctx->oC_IntegerLiteral(1)->getText());
            break;
        }
    default:
        CYPHER_TODO();
    }
    edge->setHopRange(low, hight);
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_LabelName(LcypherParser::OC_LabelNameContext *ctx) {
    return visit(ctx->oC_SchemaName());
}

std::any CypherBaseVisitorV2::visitOC_RelTypeName(LcypherParser::OC_RelTypeNameContext *ctx) {
    return visit(ctx->oC_SchemaName());
}

std::any CypherBaseVisitorV2::visitOC_Expression(LcypherParser::OC_ExpressionContext *ctx) {
    return ctx->oC_OrExpression()->accept(this);
}

std::any CypherBaseVisitorV2::visitOC_OrExpression(LcypherParser::OC_OrExpressionContext *ctx) {
    geax::frontend::Expr *left = nullptr, *right = nullptr;
    for (size_t idx = 0; idx < ctx->oC_XorExpression().size(); ++idx) {
        left = right;
        checkedAnyCast(visit(ctx->oC_XorExpression(idx)), right);
        if (idx > 0) {
            auto op = ALLOC_GEAOBJECT(geax::frontend::BOr);
            op->setLeft(left);
            op->setRight(right);
            right = op;
        }
    }
    return right;
}

std::any CypherBaseVisitorV2::visitOC_XorExpression(LcypherParser::OC_XorExpressionContext *ctx) {
    geax::frontend::Expr *left = nullptr, *right = nullptr;
    for (size_t idx = 0; idx < ctx->oC_AndExpression().size(); ++idx) {
        left = right;
        checkedAnyCast(visit(ctx->oC_AndExpression(idx)), right);
        if (idx > 0) {
            auto op = ALLOC_GEAOBJECT(geax::frontend::BXor);
            op->setLeft(left);
            op->setRight(right);
            right = op;
        }
    }
    return right;
}

std::any CypherBaseVisitorV2::visitOC_AndExpression(LcypherParser::OC_AndExpressionContext *ctx) {
    geax::frontend::Expr *left = nullptr, *right = nullptr;
    for (size_t idx = 0; idx < ctx->oC_NotExpression().size(); ++idx) {
        left = right;
        checkedAnyCast(visit(ctx->oC_NotExpression(idx)), right);
        if (idx > 0) {
            auto op = ALLOC_GEAOBJECT(geax::frontend::BAnd);
            op->setLeft(left);
            op->setRight(right);
            right = op;
        }
    }
    return right;
}

std::any CypherBaseVisitorV2::visitOC_NotExpression(LcypherParser::OC_NotExpressionContext *ctx) {
    if (ctx->NOT().size() % 2) {
        auto op = ALLOC_GEAOBJECT(geax::frontend::Not);
        geax::frontend::Expr *expr = nullptr;
        checkedAnyCast(visit(ctx->oC_ComparisonExpression()), expr);
        op->setExpr(expr);
        return (geax::frontend::Expr *)op;
    } else {
        return visit(ctx->oC_ComparisonExpression());
    }
}

std::any CypherBaseVisitorV2::visitOC_ComparisonExpression(
    LcypherParser::OC_ComparisonExpressionContext *ctx) {
    geax::frontend::Expr *expr = nullptr;
    checkedAnyCast(visit(ctx->oC_AddOrSubtractExpression()), expr);
    if (ctx->oC_PartialComparisonExpression().empty()) return expr;
    if (ctx->oC_PartialComparisonExpression().size() > 1) NOT_SUPPORT_AND_THROW();
    geax::frontend::BinaryOp *bi = nullptr;
    checkedAnyCast(visit(ctx->oC_PartialComparisonExpression(0)), bi);
    bi->setLeft(expr);
    return (geax::frontend::Expr *)bi;
}

std::any CypherBaseVisitorV2::visitOC_AddOrSubtractExpression(
    LcypherParser::OC_AddOrSubtractExpressionContext *ctx) {
    std::vector<std::string> ops;
    ops.reserve(ctx->children.size());
    for (auto c : ctx->children) {
        if (c->getText() == "+" || c->getText() == "-") {
            ops.emplace_back(c->getText());
        }
    }
    if (ops.size() != ctx->oC_MultiplyDivideModuloExpression().size() - 1) {
        NOT_SUPPORT_AND_THROW();
    }
    geax::frontend::Expr *left = nullptr, *right = nullptr;
    for (size_t idx = 0; idx < ctx->oC_MultiplyDivideModuloExpression().size(); ++idx) {
        left = right;
        checkedAnyCast(visit(ctx->oC_MultiplyDivideModuloExpression(idx)), right);
        if (idx > 0) {
            geax::frontend::BinaryOp *op = nullptr;
            if (ops[idx - 1] == "+") {
                op = ALLOC_GEAOBJECT(geax::frontend::BAdd);
            } else if (ops[idx - 1] == "-") {
                op = ALLOC_GEAOBJECT(geax::frontend::BSub);
            }
            op->setLeft(left);
            op->setRight(right);
            right = op;
        }
    }
    return right;
}

std::any CypherBaseVisitorV2::visitOC_MultiplyDivideModuloExpression(
    LcypherParser::OC_MultiplyDivideModuloExpressionContext *ctx) {
    std::vector<std::string> ops;
    ops.reserve(ctx->children.size());
    for (auto c : ctx->children) {
        if (c->getText() == "*" || c->getText() == "/" || c->getText() == "%") {
            ops.emplace_back(c->getText());
        }
    }
    if (ops.size() != ctx->oC_PowerOfExpression().size() - 1) {
        NOT_SUPPORT_AND_THROW();
    }
    geax::frontend::Expr *left = nullptr, *right = nullptr;
    for (size_t idx = 0; idx < ctx->oC_PowerOfExpression().size(); ++idx) {
        left = right;
        checkedAnyCast(visit(ctx->oC_PowerOfExpression(idx)), right);
        if (idx > 0) {
            geax::frontend::BinaryOp *op = nullptr;
            if (ops[idx - 1] == "*") {
                op = ALLOC_GEAOBJECT(geax::frontend::BMul);
            } else if (ops[idx - 1] == "/") {
                op = ALLOC_GEAOBJECT(geax::frontend::BDiv);
            } else if (ops[idx - 1] == "%") {
                op = ALLOC_GEAOBJECT(geax::frontend::BMod);
            }
            op->setLeft(left);
            op->setRight(right);
            right = op;
        }
    }
    return right;
}

std::any CypherBaseVisitorV2::visitOC_PowerOfExpression(
    LcypherParser::OC_PowerOfExpressionContext *ctx) {
    std::vector<std::string> ops;
    ops.reserve(ctx->children.size());
    for (auto c : ctx->children) {
        if (c->getText() == "^") {
            ops.emplace_back(c->getText());
        }
    }
    if (ops.size() != ctx->oC_UnaryAddOrSubtractExpression().size() - 1) {
        NOT_SUPPORT_AND_THROW();
    }
    geax::frontend::Expr *left = nullptr, *right = nullptr;
    for (size_t idx = 0; idx < ctx->oC_UnaryAddOrSubtractExpression().size(); ++idx) {
        left = right;
        checkedAnyCast(visit(ctx->oC_UnaryAddOrSubtractExpression(idx)), right);
        if (idx > 0) {
            geax::frontend::BinaryOp *op = nullptr;
            if (ops[idx - 1] == "^") {
                op = ALLOC_GEAOBJECT(geax::frontend::BSquare);
            }
            op->setLeft(left);
            op->setRight(right);
            right = op;
        }
    }
    return right;
}

std::any CypherBaseVisitorV2::visitOC_UnaryAddOrSubtractExpression(
    LcypherParser::OC_UnaryAddOrSubtractExpressionContext *ctx) {
    int sign = 0;
    if (ctx->children.size() > 1) {
        for (auto &child : ctx->children) {
            // TODO(anyone) not elegant
            if (child->getText() == "-") sign ^= 0x1;
        }
    }
    geax::frontend::Expr *expr = nullptr;
    checkedAnyCast(visit(ctx->oC_StringListNullOperatorExpression()), expr);
    if (sign) {
        auto neg = ALLOC_GEAOBJECT(geax::frontend::Neg);
        neg->setExpr(expr);
        expr = neg;
    }
    return expr;
}

std::any CypherBaseVisitorV2::visitOC_StringListNullOperatorExpression(
    LcypherParser::OC_StringListNullOperatorExpressionContext *ctx) {
    geax::frontend::Expr *expr = nullptr;
    checkedAnyCast(visit(ctx->oC_PropertyOrLabelsExpression()), expr);
    if (!ctx->oC_NullOperatorExpression().empty()) {
        if (ctx->oC_NullOperatorExpression().size() > 1) NOT_SUPPORT_AND_THROW();
        geax::frontend::Expr *is_null_expr = nullptr;
        checkedAnyCast(visit(ctx->oC_NullOperatorExpression()[0]), is_null_expr);
        if (typeid(*is_null_expr) == typeid(geax::frontend::IsNull)) {
            geax::frontend::IsNull *is_null = nullptr;
            checkedCast(is_null_expr, is_null);
            is_null->setExpr(expr);
        } else if (typeid(*is_null_expr) == typeid(geax::frontend::Not)) {
            geax::frontend::Not *not_expr = nullptr;
            checkedCast(is_null_expr, not_expr);
            geax::frontend::Expr *inner = not_expr->expr();
            geax::frontend::IsNull *is_null = nullptr;
            checkedCast(inner, is_null);
            is_null->setExpr(expr);
        } else {
            NOT_SUPPORT_AND_THROW();
        }
        return is_null_expr;
    } else if (!ctx->oC_StringOperatorExpression().empty()) {
        if (ctx->oC_StringOperatorExpression().size() > 1) NOT_SUPPORT_AND_THROW();
        for (auto e : ctx->oC_StringOperatorExpression()) {
            geax::frontend::Expr *func_expr = nullptr;
            checkedAnyCast(visit(e), func_expr);
            geax::frontend::Function *func = nullptr;
            checkedCast(func_expr, func);
            func->appendArg(expr);
            return (geax::frontend::Expr *)func;
        }
    } else if (!ctx->oC_ListOperatorExpression().empty()) {
        if (ctx->oC_ListOperatorExpression().size() > 1) NOT_SUPPORT_AND_THROW();
        auto list_op_expr = ctx->oC_ListOperatorExpression(0);
        if (list_op_expr->IN()) {
            CYPHER_THROW_ASSERT(list_op_expr->oC_PropertyOrLabelsExpression());
            geax::frontend::Expr *right = nullptr;
            checkedAnyCast(visit(list_op_expr->oC_PropertyOrLabelsExpression()), right);
            auto bin = ALLOC_GEAOBJECT(geax::frontend::BIn);
            bin->setLeft(expr);
            bin->setRight(right);
            return (geax::frontend::Expr *)bin;
        } else {
            CYPHER_THROW_ASSERT(!list_op_expr->oC_Expression().empty());
            auto func = ALLOC_GEAOBJECT(geax::frontend::Function);
            func->setName("subscript");
            func->appendArg(expr);
            for (size_t idx = 0; idx < list_op_expr->oC_Expression().size(); ++idx) {
                geax::frontend::Expr *expr = nullptr;
                checkedAnyCast(visit(list_op_expr->oC_Expression(idx)), expr);
                func->appendArg(expr);
            }
            return (geax::frontend::Expr *)func;
        }
    }
    return expr;
}

std::any CypherBaseVisitorV2::visitOC_ListOperatorExpression(
    LcypherParser::OC_ListOperatorExpressionContext *ctx) {
    return visitChildren(ctx);
}

std::any CypherBaseVisitorV2::visitOC_StringOperatorExpression(
    LcypherParser::OC_StringOperatorExpressionContext *ctx) {
    std::string name = ctx->STARTS()     ? "StartsWith"
                       : ctx->ENDS()     ? "EndsWith"
                       : ctx->CONTAINS() ? "Contains"
                                         : "Regexp";
    auto func = ALLOC_GEAOBJECT(geax::frontend::Function);
    func->setName(std::move(name));
    geax::frontend::Expr *expr = nullptr;
    checkedAnyCast(visit(ctx->oC_PropertyOrLabelsExpression()), expr);
    func->appendArg(expr);
    return (geax::frontend::Expr *)func;
}

std::any CypherBaseVisitorV2::visitOC_NullOperatorExpression(
    LcypherParser::OC_NullOperatorExpressionContext *ctx) {
    if (ctx->NOT()) {
        auto nl = ALLOC_GEAOBJECT(geax::frontend::IsNull);
        auto nt = ALLOC_GEAOBJECT(geax::frontend::Not);
        nt->setExpr(nl);
        return (geax::frontend::Expr *)nt;
    } else {
        auto nul = ALLOC_GEAOBJECT(geax::frontend::IsNull);
        return (geax::frontend::Expr *)nul;
    }
    NOT_SUPPORT_AND_THROW();
}

std::any CypherBaseVisitorV2::visitOC_PropertyOrLabelsExpression(
    LcypherParser::OC_PropertyOrLabelsExpressionContext *ctx) {
    if (ctx->children.size() == 1) return visit(ctx->oC_Atom());

    if (!ctx->oC_PropertyLookup().empty()) {
        if (ctx->oC_PropertyLookup().size() > 1) NOT_SUPPORT_AND_THROW();
        auto field = ALLOC_GEAOBJECT(geax::frontend::GetField);
        geax::frontend::Expr *name_expr = nullptr;
        checkedAnyCast(visit(ctx->oC_PropertyLookup(0)), name_expr);
        geax::frontend::VString *vstr = nullptr;
        checkedCast(name_expr, vstr);
        std::string field_name = vstr->val();
        field->setFieldName(std::move(field_name));
        geax::frontend::Expr *expr = nullptr;
        checkedAnyCast(visit(ctx->oC_Atom()), expr);
        field->setExpr(expr);
        return (geax::frontend::Expr *)field;
    }
    if (ctx->oC_NodeLabels()) {
        std::vector<std::string> labels;
        checkedAnyCast(visit(ctx->oC_NodeLabels()), labels);
        auto is_labeled = ALLOC_GEAOBJECT(geax::frontend::IsLabeled);
        geax::frontend::Expr *expr = nullptr;
        checkedAnyCast(visit(ctx->oC_Atom()), expr);
        is_labeled->setExpr(expr);
        if (labels.size() == 1) {
            auto l = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
            l->setLabel(std::move(labels[0]));
            is_labeled->setLabelTree(l);
        } else if (labels.size() == 2) {
            auto root = ALLOC_GEAOBJECT(geax::frontend::LabelOr);
            auto l = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
            auto r = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
            l->setLabel(std::move(labels[0]));
            r->setLabel(std::move(labels[1]));
            root->setLeft(l);
            root->setRight(r);
            is_labeled->setLabelTree(root);
        } else {
            auto root = ALLOC_GEAOBJECT(geax::frontend::LabelOr);
            auto label = root;
            for (size_t idx = 0; idx < labels.size() - 2; ++idx) {
                auto o = ALLOC_GEAOBJECT(geax::frontend::LabelOr);
                auto l = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
                l->setLabel(std::move(labels[idx]));
                label->setLeft(l);
                label->setRight(o);
                label = o;
            }
            auto l = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
            auto r = ALLOC_GEAOBJECT(geax::frontend::SingleLabel);
            l->setLabel(std::move(labels[labels.size() - 2]));
            r->setLabel(std::move(labels[labels.size() - 1]));
            label->setLeft(l);
            label->setRight(r);
            is_labeled->setLabelTree(root);
        }
        return (geax::frontend::Expr *)is_labeled;
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Atom(LcypherParser::OC_AtomContext *ctx) {
    if (ctx->oC_Variable()) {
        if (VisitGuard::InClause(VisitType::kSetLabel, visit_types_)) {
            // TODO(lpp) support update vertex or edge
            THROW_CODE(CypherException, "Not support to update the entire vertex or "
                       "edge in set clause.");
        }
        geax::frontend::Expr *name_expr = nullptr;
        checkedAnyCast(visit(ctx->oC_Variable()), name_expr);
        geax::frontend::VString *vstr = nullptr;
        checkedCast(name_expr, vstr);
        std::string name = vstr->val();
        auto expr = ALLOC_GEAOBJECT(geax::frontend::Ref);
        if (list_comprehension_depth == 0 || list_comprehension_anonymous_symbols_.find(name) ==
                                                 list_comprehension_anonymous_symbols_.end()) {
            expr->setName(std::move(name));
        } else {
            auto list_comprehension_name = list_comprehension_anonymous_symbols_[name].top();
            expr->setName(std::move(list_comprehension_name));
        }
        return (geax::frontend::Expr *)expr;
    } else if (ctx->oC_Literal()) {
        return visit(ctx->oC_Literal());
    } else if (ctx->oC_FunctionInvocation()) {
        return visit(ctx->oC_FunctionInvocation());
    } else if (ctx->oC_ParenthesizedExpression()) {
        return visit(ctx->oC_ParenthesizedExpression());
    } else if (ctx->oC_RelationshipsPattern()) {
        return visit(ctx->oC_RelationshipsPattern());
    } else if (ctx->oC_CaseExpression()) {
        return visit(ctx->oC_CaseExpression());
    } else if (ctx->oC_FilterExpression()) {
        return visit(ctx->oC_FilterExpression());
    } else if (ctx->oC_ListComprehension()) {
        return visit(ctx->oC_ListComprehension());
    } else if (ctx->oC_PatternComprehension()) {
        return visit(ctx->oC_PatternComprehension());
    } else if (ctx->oC_Parameter()) {
        return visit(ctx->oC_Parameter());
    } else if (ctx->COUNT()) {
        auto func = ALLOC_GEAOBJECT(geax::frontend::AggFunc);
        func->setFuncName(geax::frontend::GeneralSetFunction::kCount);
        func->setDistinct(false);
        auto param = ALLOC_GEAOBJECT(geax::frontend::VString);
        param->setVal(std::string("*"));
        func->setExpr(param);
        return (geax::frontend::Expr *)func;
    }
    NOT_SUPPORT_AND_THROW();
}

std::any CypherBaseVisitorV2::visitOC_Literal(LcypherParser::OC_LiteralContext *ctx) {
    if (ctx->StringLiteral()) {
        std::string str = ctx->StringLiteral()->getText();
        CYPHER_THROW_ASSERT(!str.empty() && (str[0] == '\'' || str[0] == '\"') &&
                            (str[str.size() - 1] == '\'' || str[str.size() - 1] == '\"'));
        str = str.substr(1, str.size() - 2);
        auto expr = ALLOC_GEAOBJECT(geax::frontend::VString);
        expr->setVal(std::move(str));
        return (geax::frontend::Expr *)expr;
    } else if (ctx->oC_NumberLiteral()) {
        return visit(ctx->oC_NumberLiteral());
    } else if (ctx->oC_BooleanLiteral()) {
        return visit(ctx->oC_BooleanLiteral());
    } else if (ctx->oC_MapLiteral()) {
        return visit(ctx->oC_MapLiteral());
    } else if (ctx->NULL_()) {
        if (VisitGuard::InClause(VisitType::kSetNull, visit_types_)) {
            auto n = ALLOC_GEAOBJECT(geax::frontend::VNull);
            return (geax::frontend::Expr *)n;
        } else if (VisitGuard::InClause(VisitType::kSetVariable, visit_types_)) {
            geax::frontend::UpdateProperties *up = nullptr;
            checkedCast(node_, up);
            auto prop = ALLOC_GEAOBJECT(geax::frontend::PropStruct);
            up->setStructs(prop);
            auto n = ALLOC_GEAOBJECT(geax::frontend::VNull);
            prop->appendProperty("", n);
            return (geax::frontend::Expr *)n;
        } else {
            auto n = ALLOC_GEAOBJECT(geax::frontend::VNull);
            return (geax::frontend::Expr *)n;
        }
    } else if (ctx->oC_ListLiteral()) {
        return visit(ctx->oC_ListLiteral());
    }
    NOT_SUPPORT_AND_THROW();
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_BooleanLiteral(LcypherParser::OC_BooleanLiteralContext *ctx) {
    auto d = ALLOC_GEAOBJECT(geax::frontend::VBool);
    if (ctx->TRUE_()) {
        d->setVal(true);
    } else {
        d->setVal(false);
    }
    return (geax::frontend::Expr *)d;
}

std::any CypherBaseVisitorV2::visitOC_ListLiteral(LcypherParser::OC_ListLiteralContext *ctx) {
    auto list = ALLOC_GEAOBJECT(geax::frontend::MkList);
    for (auto &ctx_expr : ctx->oC_Expression()) {
        geax::frontend::Expr *expr = nullptr;
        checkedAnyCast(visit(ctx_expr), expr);
        list->appendElem(expr);
    }
    return (geax::frontend::Expr *)list;
}

std::any CypherBaseVisitorV2::visitOC_PartialComparisonExpression(
    LcypherParser::OC_PartialComparisonExpressionContext *ctx) {
    geax::frontend::Expr *expr = nullptr;
    checkedAnyCast(visit(ctx->oC_AddOrSubtractExpression()), expr);
    geax::frontend::BinaryOp *bi = nullptr;
    auto predicate = ctx->getStart()->getText();
    if (predicate == "=") {
        bi = ALLOC_GEAOBJECT(geax::frontend::BEqual);
    } else if (predicate == "<>") {
        bi = ALLOC_GEAOBJECT(geax::frontend::BNotEqual);
    } else if (predicate == "<") {
        bi = ALLOC_GEAOBJECT(geax::frontend::BSmallerThan);
    } else if (predicate == ">") {
        bi = ALLOC_GEAOBJECT(geax::frontend::BGreaterThan);
    } else if (predicate == "<=") {
        bi = ALLOC_GEAOBJECT(geax::frontend::BNotGreaterThan);
    } else if (predicate == ">=") {
        bi = ALLOC_GEAOBJECT(geax::frontend::BNotSmallerThan);
    }
    bi->setRight(expr);
    return bi;
}

std::any CypherBaseVisitorV2::visitOC_ParenthesizedExpression(
    LcypherParser::OC_ParenthesizedExpressionContext *ctx) {
    return visit(ctx->oC_Expression());
}

std::any CypherBaseVisitorV2::visitOC_RelationshipsPattern(
    LcypherParser::OC_RelationshipsPatternContext *ctx) {
    path_chain_ = ALLOC_GEAOBJECT(geax::frontend::PathChain);
    auto node = ALLOC_GEAOBJECT(geax::frontend::Node);
    path_chain_->setHead(node);
    SWITCH_CONTEXT_VISIT(ctx->oC_NodePattern(), node);
    for (auto &chain : ctx->oC_PatternElementChain()) {
        SWITCH_CONTEXT_VISIT(chain, path_chain_);
    }
    return (geax::frontend::Expr *)node;
}

std::any CypherBaseVisitorV2::visitOC_FilterExpression(
    LcypherParser::OC_FilterExpressionContext *ctx) {
    return visitChildren(ctx);
}

std::any CypherBaseVisitorV2::visitOC_IdInColl(LcypherParser::OC_IdInCollContext *ctx) {
    geax::frontend::ListComprehension *listComprehension = nullptr;
    checkedCast(node_, listComprehension);
    auto var = ctx->oC_Variable()->getText();
    if (list_comprehension_anonymous_symbols_.find(var) ==
        list_comprehension_anonymous_symbols_.end()) {
        list_comprehension_anonymous_symbols_[var] = std::stack<std::string>();
    }
    auto anonymous_var = GenerateListComprehension(var);
    list_comprehension_anonymous_symbols_[var].push(anonymous_var);
    var = anonymous_var;
    auto variable_ref = ALLOC_GEAOBJECT(geax::frontend::Ref);
    variable_ref->setName(std::move(var));
    listComprehension->setVariable(variable_ref);
    geax::frontend::Expr *in_expr;
    checkedAnyCast(visit(ctx->oC_Expression()), in_expr);
    listComprehension->setInExpression(in_expr);
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_FunctionInvocation(
    LcypherParser::OC_FunctionInvocationContext *ctx) {
    std::string name;
    checkedAnyCast(visit(ctx->oC_FunctionName()), name);
    geax::frontend::Expr *res = nullptr;
    auto it = S_AGG_LIST.find(name);
    auto bit = S_BAGG_LIST.find(name);
    if (name == "EXISTS") {
        if (ctx->oC_Expression().size() > 1) NOT_SUPPORT_AND_THROW();
        geax::frontend::Expr *expr = nullptr;
        checkedAnyCast(visit(ctx->oC_Expression(0)), expr);
        if (typeid(*expr) == typeid(geax::frontend::GetField)) {
            auto func = ALLOC_GEAOBJECT(geax::frontend::Function);
            func->setName(std::move(name));
            func->appendArg(expr);
            res = func;
        } else if (typeid(*expr) == typeid(geax::frontend::Node)) {
            auto exists = ALLOC_GEAOBJECT(geax::frontend::Exists);
            exists->appendPathChain(path_chain_);
            res = exists;
        } else {
            NOT_SUPPORT_AND_THROW();
        }
    } else if (it != S_AGG_LIST.end()) {
        CYPHER_THROW_ASSERT(ctx->oC_Expression().size() == 1);
        auto func = ALLOC_GEAOBJECT(geax::frontend::AggFunc);
        func->setFuncName(it->second);
        if (ctx->DISTINCT()) {
            func->setDistinct(true);
        } else {
            func->setDistinct(false);
        }
        geax::frontend::Expr *expr = nullptr;
        checkedAnyCast(visit(ctx->oC_Expression(0)), expr);
        func->setExpr(expr);
        res = func;
    } else if (bit != S_BAGG_LIST.end()) {
        CYPHER_THROW_ASSERT(ctx->oC_Expression().size() == 2);
        auto func = ALLOC_GEAOBJECT(geax::frontend::BAggFunc);
        func->setFuncName(bit->second);
        geax::frontend::Expr *left = nullptr, *right = nullptr;
        checkedAnyCast(visit(ctx->oC_Expression(0)), left);
        checkedAnyCast(visit(ctx->oC_Expression(1)), right);
        bool distinct = ctx->DISTINCT() ? true : false;
        func->setLExpr(distinct, left);
        func->setRExpr(right);
        res = func;
    } else {
        auto func = ALLOC_GEAOBJECT(geax::frontend::Function);
        func->setName(std::move(name));
        for (size_t idx = 0; idx < ctx->oC_Expression().size(); ++idx) {
            geax::frontend::Expr *expr = nullptr;
            checkedAnyCast(visit(ctx->oC_Expression(idx)), expr);
            func->appendArg(expr);
        }
        res = func;
    }
    return res;
}

std::any CypherBaseVisitorV2::visitOC_FunctionName(LcypherParser::OC_FunctionNameContext *ctx) {
    if (ctx->EXISTS()) {
        return std::string("EXISTS");
    }
    std::string name;
    if (ctx->oC_Namespace()) {
        checkedAnyCast(visit(ctx->oC_Namespace()), name);
    }
    name.append(ctx->oC_SymbolicName()->getText());
    return name;
}

std::any CypherBaseVisitorV2::visitOC_ExplicitProcedureInvocation(
    LcypherParser::OC_ExplicitProcedureInvocationContext *ctx) {
    if (VisitGuard::InClause(VisitType::kStandaloneCall, visit_types_)) {
        geax::frontend::NamedProcedureCall *node = nullptr;
        checkedCast(node_, node);
        std::string fun_name;
        checkedAnyCast(visit(ctx->oC_ProcedureName()), fun_name);
        geax::frontend::StringParam name = fun_name;
        node->setName(std::move(name));
        for (auto oc : ctx->oC_Expression()) {
            geax::frontend::Expr *expr = nullptr;
            checkedAnyCast(visit(oc), expr);
            node->appendArg(expr);
        }
    } else if (VisitGuard::InClause(VisitType::kInQueryCall, visit_types_)) {
        geax::frontend::InQueryProcedureCall *node = nullptr;
        checkedCast(node_, node);
        std::string fun_name;
        checkedAnyCast(visit(ctx->oC_ProcedureName()), fun_name);
        geax::frontend::StringParam name = fun_name;
        node->setName(std::move(name));
        for (auto oc : ctx->oC_Expression()) {
            geax::frontend::Expr *expr = nullptr;
            checkedAnyCast(visit(oc), expr);
            node->appendArg(expr);
        }
    } else {
        NOT_SUPPORT_AND_THROW();
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_ImplicitProcedureInvocation(
    LcypherParser::OC_ImplicitProcedureInvocationContext *ctx) {
    geax::frontend::NamedProcedureCall *node = nullptr;
    checkedCast(node_, node);
    std::string fun_name;
    checkedAnyCast(visit(ctx->oC_ProcedureName()), fun_name);
    geax::frontend::StringParam name = fun_name;
    node->setName(std::move(name));
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_ProcedureResultField(
    LcypherParser::OC_ProcedureResultFieldContext *ctx) {
    NOT_SUPPORT_AND_THROW();
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_ProcedureName(LcypherParser::OC_ProcedureNameContext *ctx) {
    std::string name;
    if (ctx->oC_Namespace()) {
        checkedAnyCast(visit(ctx->oC_Namespace()), name);
    }
    name.append(ctx->oC_SymbolicName()->getText());
    return name;
}

std::any CypherBaseVisitorV2::visitOC_Namespace(LcypherParser::OC_NamespaceContext *ctx) {
    std::string name_space;
    for (auto &s : ctx->oC_SymbolicName()) name_space.append(s->getText()).append(".");
    return name_space;
}

std::any CypherBaseVisitorV2::visitOC_ListComprehension(
    LcypherParser::OC_ListComprehensionContext *ctx) {
    list_comprehension_depth++;
    auto listComprehension = ALLOC_GEAOBJECT(geax::frontend::ListComprehension);
    SWITCH_CONTEXT_VISIT(ctx->oC_FilterExpression(), listComprehension);
    geax::frontend::Expr *op_expr;
    if (ctx->oC_Expression()) {
        checkedAnyCast(visit(ctx->oC_Expression()), op_expr);
    } else {
        CYPHER_TODO();
    }
    listComprehension->setOpExpression(op_expr);
    list_comprehension_depth--;
    list_comprehension_anonymous_symbols_[ctx->oC_FilterExpression()->
                                          oC_IdInColl()->oC_Variable()->getText()].pop();
    return (geax::frontend::Expr*)listComprehension;
}

std::any CypherBaseVisitorV2::visitOC_PatternComprehension(
    LcypherParser::OC_PatternComprehensionContext *ctx) {
    NOT_SUPPORT_AND_THROW();
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_PropertyLookup(LcypherParser::OC_PropertyLookupContext *ctx) {
    return visit(ctx->oC_PropertyKeyName());
}

std::any CypherBaseVisitorV2::visitOC_CaseExpression(LcypherParser::OC_CaseExpressionContext *ctx) {
    auto case_clause = ALLOC_GEAOBJECT(geax::frontend::Case);
    if (ctx->oC_Expression().empty()) {
        for (auto caseAlternatives : ctx->oC_CaseAlternatives()) {
            SWITCH_CONTEXT_VISIT(caseAlternatives, case_clause);
        }
    } else if (ctx->oC_Expression().size() == 2) {
        for (auto caseAlternatives : ctx->oC_CaseAlternatives()) {
            SWITCH_CONTEXT_VISIT(caseAlternatives, case_clause);
        }
        geax::frontend::Expr *head = nullptr;
        checkedAnyCast(visit(ctx->oC_Expression()[0]), head);
        case_clause->setInput(head);
        geax::frontend::Expr *tail = nullptr;
        checkedAnyCast(visit(ctx->oC_Expression()[1]), tail);
        case_clause->setElseBody(tail);
    } else if (ctx->ELSE() != nullptr) {
        if (ctx->oC_Expression().size() != 1) CYPHER_TODO();
        for (auto caseAlternatives : ctx->oC_CaseAlternatives()) {
            SWITCH_CONTEXT_VISIT(caseAlternatives, case_clause);
        }
        geax::frontend::Expr *expr = nullptr;
        checkedAnyCast(visit(ctx->oC_Expression()[0]), expr);
        case_clause->setElseBody(expr);
    } else {
        for (auto caseAlternatives : ctx->oC_CaseAlternatives()) {
            SWITCH_CONTEXT_VISIT(caseAlternatives, case_clause);
        }
        geax::frontend::Expr *head = nullptr;
        checkedAnyCast(visit(ctx->oC_Expression()[0]), head);
        case_clause->setInput(head);
    }
    return (geax::frontend::Expr *)case_clause;
}

std::any CypherBaseVisitorV2::visitOC_CaseAlternatives(
    LcypherParser::OC_CaseAlternativesContext *ctx) {
    geax::frontend::Case *node = nullptr;
    checkedCast(node_, node);
    geax::frontend::Expr *left = nullptr;
    if (ctx->oC_Expression().size() != 2) CYPHER_TODO();
    checkedAnyCast(visit(ctx->oC_Expression()[0]), left);
    geax::frontend::Expr *right = nullptr;
    checkedAnyCast(visit(ctx->oC_Expression()[1]), right);
    node->appendCaseBody(left, right);
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Variable(LcypherParser::OC_VariableContext *ctx) {
    return visit(ctx->oC_SymbolicName());
}

std::any CypherBaseVisitorV2::visitOC_NumberLiteral(LcypherParser::OC_NumberLiteralContext *ctx) {
    if (ctx->oC_DoubleLiteral()) {
        return visit(ctx->oC_DoubleLiteral());
    } else if (ctx->oC_IntegerLiteral()) {
        return visit(ctx->oC_IntegerLiteral());
    }
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_MapLiteral(LcypherParser::OC_MapLiteralContext *ctx) {
    geax::frontend::Expr *res = nullptr;
    // this must be the fist condition， don't move!!!
    if (VisitGuard::InClause(VisitType::kSetVariable, visit_types_)) {
        geax::frontend::UpdateProperties *node = nullptr;
        checkedCast(node_, node);
        if (ctx->oC_Expression().size() != ctx->oC_PropertyKeyName().size())
            NOT_SUPPORT_AND_THROW();
        auto ps = ALLOC_GEAOBJECT(geax::frontend::PropStruct);
        node->setStructs(ps);
        for (size_t idx = 0; idx < ctx->oC_PropertyKeyName().size(); ++idx) {
            geax::frontend::Expr *expr = nullptr;
            checkedAnyCast(visit(ctx->oC_Expression(idx)), expr);
            geax::frontend::Expr *name_expr = nullptr;
            checkedAnyCast(visit(ctx->oC_PropertyKeyName(idx)), name_expr);
            geax::frontend::VString *vstr = nullptr;
            checkedCast(name_expr, vstr);
            std::string name = vstr->val();
            ps->appendProperty(std::move(name), expr);
        }
    } else if (VisitGuard::InClause(VisitType::kReadingPattern, visit_types_) ||
               VisitGuard::InClause(VisitType::kUpdatingClause, visit_types_)) {
        geax::frontend::ElementFiller *filler = nullptr;
        checkedCast(node_, filler);
        if (ctx->oC_Expression().size() != ctx->oC_PropertyKeyName().size())
            NOT_SUPPORT_AND_THROW();
        auto ps = ALLOC_GEAOBJECT(geax::frontend::PropStruct);
        filler->appendPredicate(ps);
        for (size_t idx = 0; idx < ctx->oC_PropertyKeyName().size(); ++idx) {
            geax::frontend::Expr *expr = nullptr;
            checkedAnyCast(visit(ctx->oC_Expression(idx)), expr);
            geax::frontend::Expr *name_expr = nullptr;
            checkedAnyCast(visit(ctx->oC_PropertyKeyName(idx)), name_expr);
            geax::frontend::VString *vstr = nullptr;
            checkedCast(name_expr, vstr);
            std::string name = vstr->val();
            ps->appendProperty(std::move(name), expr);
        }
    } else if (VisitGuard::InClause(VisitType::kStandaloneCall, visit_types_) ||
               VisitGuard::InClause(VisitType::kInQueryCall, visit_types_) ||
               VisitGuard::InClause(VisitType::kReadingClause, visit_types_)) {
        auto map = ALLOC_GEAOBJECT(geax::frontend::MkMap);
        if (ctx->oC_Expression().size() != ctx->oC_PropertyKeyName().size())
            NOT_SUPPORT_AND_THROW();
        for (size_t idx = 0; idx < ctx->oC_PropertyKeyName().size(); ++idx) {
            geax::frontend::Expr *expr = nullptr;
            checkedAnyCast(visit(ctx->oC_Expression(idx)), expr);
            geax::frontend::Expr *name_expr = nullptr;
            checkedAnyCast(visit(ctx->oC_PropertyKeyName(idx)), name_expr);
            map->appendElem(name_expr, expr);
        }
        res = map;
    } else {
        NOT_SUPPORT_AND_THROW();
    }
    return res;
}

std::any CypherBaseVisitorV2::visitOC_Parameter(LcypherParser::OC_ParameterContext *ctx) {
    auto param = ALLOC_GEAOBJECT(geax::frontend::Param);
    param->setName(ctx->getText());
    return (geax::frontend::Expr *)param;
}

std::any CypherBaseVisitorV2::visitOC_PropertyExpression(
    LcypherParser::OC_PropertyExpressionContext *ctx) {
    geax::frontend::Expr *ret = nullptr;
    if (VisitGuard::InClause(VisitType::kSetVariable, visit_types_)) {
        if (ctx->oC_PropertyLookup().size() > 1) NOT_SUPPORT_AND_THROW();
        geax::frontend::UpdateProperties *node = nullptr;
        checkedCast(node_, node);
        geax::frontend::Expr *name_expr = nullptr;
        checkedAnyCast(visit(ctx->oC_Atom()), name_expr);
        geax::frontend::Ref *ref = nullptr;
        checkedCast(name_expr, ref);
        std::string var = ref->name();
        node->setV(std::move(var));

        auto ps = ALLOC_GEAOBJECT(geax::frontend::PropStruct);
        node->setStructs(ps);
        geax::frontend::Expr *prop = nullptr;
        checkedAnyCast(visit(ctx->oC_PropertyLookup(0)), prop);
        geax::frontend::VString *prop_str = nullptr;
        checkedCast(prop, prop_str);
        std::string property = prop_str->val();
        ps->appendProperty(std::move(property), nullptr);
    }
    return ret;
}

std::any CypherBaseVisitorV2::visitOC_PropertyKeyName(
    LcypherParser::OC_PropertyKeyNameContext *ctx) {
    return visit(ctx->oC_SchemaName());
}

std::any CypherBaseVisitorV2::visitOC_IntegerLiteral(LcypherParser::OC_IntegerLiteralContext *ctx) {
    auto integer = ALLOC_GEAOBJECT(geax::frontend::VInt);
    integer->setVal(std::stol(ctx->getText()));
    return (geax::frontend::Expr *)integer;
}

std::any CypherBaseVisitorV2::visitOC_DoubleLiteral(LcypherParser::OC_DoubleLiteralContext *ctx) {
    auto d = ALLOC_GEAOBJECT(geax::frontend::VDouble);
    d->setVal(std::stod(ctx->getText()));
    return (geax::frontend::Expr *)d;
}

std::any CypherBaseVisitorV2::visitOC_SchemaName(LcypherParser::OC_SchemaNameContext *ctx) {
    if (ctx->oC_ReservedWord()) {
        auto s = ALLOC_GEAOBJECT(geax::frontend::VString);
        s->setVal(ctx->getText());
        return (geax::frontend::Expr *)s;
    } else {
        return visitOC_SymbolicName(ctx->oC_SymbolicName());
    }
    return visitChildren(ctx);
}

std::any CypherBaseVisitorV2::visitOC_ReservedWord(LcypherParser::OC_ReservedWordContext *ctx) {
    NOT_SUPPORT_AND_THROW();
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_SymbolicName(LcypherParser::OC_SymbolicNameContext *ctx) {
    auto s = ALLOC_GEAOBJECT(geax::frontend::VString);
    s->setVal(ctx->getText());
    return (geax::frontend::Expr *)s;
}

std::any CypherBaseVisitorV2::visitOC_LeftArrowHead(LcypherParser::OC_LeftArrowHeadContext *ctx) {
    NOT_SUPPORT_AND_THROW();
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_RightArrowHead(LcypherParser::OC_RightArrowHeadContext *ctx) {
    NOT_SUPPORT_AND_THROW();
    return 0;
}

std::any CypherBaseVisitorV2::visitOC_Dash(LcypherParser::OC_DashContext *ctx) {
    NOT_SUPPORT_AND_THROW();
    return 0;
}

}  //  end of namespace parser
