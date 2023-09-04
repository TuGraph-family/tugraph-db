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
 *
 *  Author:
 *         Yaochi <boyao.zby@alibaba-inc.com>
 */

#include "geax-front-end/isogql/GQLAstVisitor.h"

#include <unordered_map>

#include "geax-front-end/utils/Conv.h"
#include "geax-front-end/utils/Copilot.h"
#include "geax-front-end/utils/Logging.h"
#include "geax-front-end/utils/TinyScopeGuard.h"

#ifndef VISIT_RULE
#define VISIT_RULE(RuleCtx, NodeType, Node) VISIT_RULE_I(RuleCtx, NodeType, Node)
#define VISIT_RULE_I(RuleCtx, NodeType, Node)                                          \
    ({                                                                                 \
        RuleCtx->accept(this);                                                         \
        if (GEAX_RET_FAIL(checkChildResult())) {                                       \
            LOG(WARNING) << "Failed to check child result: " << K(ret);                \
        } else if (GEAX_UNLIKELY(childRes_->type() != NodeType)) {                     \
            ret = GEAXErrorCode::GEAX_COMMON_PARSE_ERROR;                              \
            LOG(WARNING) << "Result should be " << KV("type", NodeType) << ", actual " \
                         << KV("type", childRes_->type()) << DK(ret);                  \
        } else if (GEAX_UNLIKELY(!uncheckedCast(childRes_, Node))) {                   \
            ret = GEAXErrorCode::GEAX_COMMON_PARSE_ERROR;                              \
            LOG(WARNING) << "Failed to cast to node " << #NodeType << ": " << K(ret);  \
        }                                                                              \
        ret;                                                                           \
    })
#endif  // !VISIT_RULE

#ifndef ACCEPT_RULE
#define ACCEPT_RULE(RuleCtx) ACCEPT_RULE_I(RuleCtx)
#define ACCEPT_RULE_I(RuleCtx)                                          \
    ({                                                                  \
        RuleCtx->accept(this);                                          \
        if (GEAX_RET_FAIL(checkChildResult())) {                        \
            LOG(WARNING) << "Failed to check child result: " << K(ret); \
        }                                                               \
        ret;                                                            \
    })
#endif  // !ACCEPT_RULE

#ifndef VISIT_RULE_WITH_FA
#define VISIT_RULE_WITH_FA(RuleCtx, FaNode) VISIT_RULE_WITH_FA_I(RuleCtx, FaNode)
#define VISIT_RULE_WITH_FA_I(RuleCtx, FaNode)                           \
    ({                                                                  \
        AstNode* faRes = faRes_;                                        \
        faRes_ = FaNode;                                                \
        RuleCtx->accept(this);                                          \
        if (GEAX_RET_FAIL(checkChildRet())) {                           \
            LOG(WARNING) << "Failed to check child result: " << K(ret); \
        } else {                                                        \
            /* Recover fa back to its original value */                 \
            faRes_ = faRes;                                             \
        }                                                               \
        ret;                                                            \
    })
#endif  // !VISIT_RULE_WITH_FA

#ifndef VISIT_EXPR
#define VISIT_EXPR(RuleCtx, ExprNode) VISIT_EXPR_I(RuleCtx, ExprNode)
#define VISIT_EXPR_I(RuleCtx, ExprNode)                                 \
    ({                                                                  \
        RuleCtx->accept(this);                                          \
        if (GEAX_RET_FAIL(checkChildResult())) {                        \
            LOG(WARNING) << "Failed to check child result: " << K(ret); \
        } else if (GEAX_UNLIKELY(!checkedCast(childRes_, ExprNode))) {  \
            ret = GEAXErrorCode::GEAX_COMMON_PARSE_ERROR;               \
            LOG(WARNING) << "Failed to cast to ExprNode: " << K(ret);   \
        }                                                               \
        ret;                                                            \
    })
#endif  // !VISIT_EXPR

#ifndef VISIT_TOKEN
#define VISIT_TOKEN(RuleCtx, token) VISIT_TOKEN_I(RuleCtx, token)
#define VISIT_TOKEN_I(RuleCtx, token)                                   \
    ({                                                                  \
        RuleCtx->accept(this);                                          \
        if (GEAX_RET_FAIL(checkTokenResult())) {                        \
            LOG(WARNING) << "Failed to check token result: " << K(ret); \
        } else {                                                        \
            token = token_->getType();                                  \
        }                                                               \
        ret;                                                            \
    })
#endif  // !VISIT_TOKEN

#ifndef ALLOC_GEAOBJECT
#define ALLOC_GEAOBJECT(obj) ctx_.objAlloc().allocate<obj>()
#endif  // !ALLOC_GEAOBJECT

#ifndef DEFER
#define DEFER(res, ret)                 \
    auto defer = TinyScopeGuard([&]() { \
        childRes_ = res;                \
        childRet_ = ret;                \
    })
#endif  // !DEFER

#ifndef DEFER_RET
#define DEFER_RET(ret) auto defer = TinyScopeGuard([&]() { childRet_ = ret; })
#endif  // !DEFER_RET

using namespace parser;

namespace geax {
namespace frontend {

using geax::utils::checkedCast;
using geax::utils::uncheckedCast;

// Some basic rules:
// 1) we do not check rule ctx pointer as parameters, because we checked before passing it
// 2) but MUST check its children
// 3) if you change faRes_, please recover it after visiting a node, which is done by
// VISIT_RULE_WITH_FA

std::any GQLAstVisitor::visitGqlRequest(GqlParser::GqlRequestContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    if (GEAX_RET_FAIL(byPass(ctx))) {
        LOG(WARNING) << "Failed to visit children of GqlRequest: " << K(ret);
    } else if (GEAX_RET_FAIL(checkChildResult())) {
        LOG(WARNING) << "Failed to check child result: " << K(ret);
    } else if (GEAX_UNLIKELY_NE(childRes_->type(), AstNodeType::kProcedureBody)) {
        ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
        LOG(WARNING) << "Statement type is not supported now: " << KV("type", childRes_->type())
                     << DK(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitProcedureBody(GqlParser::ProcedureBodyContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    ProcedureBody* procedure = ALLOC_GEAOBJECT(ProcedureBody);
    DEFER(procedure, ret);

    GqlParser::StatementBlockContext* stmtCtx = nullptr;
    if (ctx->atSchemaClause() != nullptr) {
        ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
        LOG(WARNING) << "AtSchemaClause is not supported now: " << K(ret);
    } else if (ctx->bindingVariableDefinitionBlock() != nullptr) {
        ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
        LOG(WARNING) << "BindingVariable is not supported now: " << K(ret);
    } else if (GEAX_IS_NULL(stmtCtx = ctx->statementBlock())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "StatementBlock is not set: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(stmtCtx, procedure))) {
        LOG(WARNING) << "Failed to visit rule StatementBlock: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitStatementBlock(GqlParser::StatementBlockContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    ProcedureBody* procedure = nullptr;
    GqlParser::StatementContext* stmtCtx = nullptr;
    Statement* stmt = nullptr;
    if (GEAX_IS_NULL(procedure = castAs<ProcedureBody>(faRes_, AstNodeType::kProcedureBody))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kProcedureBody: " << K(ret);
    } else if (GEAX_IS_NULL(stmtCtx = ctx->statement())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Child of rule StatementBlock is not set: " << K(ret);
    } else if (GEAX_RET_FAIL(ACCEPT_RULE(stmtCtx))) {
        LOG(WARNING) << "Failed to visit rule Statement: " << K(ret);
    } else if (GEAX_UNLIKELY(!checkedCast(childRes_, stmt))) {
        ret = GEAXErrorCode::GEAX_OOPS;
        LOG(WARNING) << "Result after visiting rule Statement should be Statement: " << K(ret);
    } else {
        StatementWithYield* headStmt = ALLOC_GEAOBJECT(StatementWithYield);
        headStmt->setStatement(stmt);
        YieldField* yieldNode = ALLOC_GEAOBJECT(YieldField);
        headStmt->setYield(yieldNode);
        procedure->appendStatement(headStmt);
        auto nextStmts = ctx->nextStatement();
        for (auto i = 0u; i < nextStmts.size() && GEAX_OK(ret); ++i) {
            StatementWithYield* nextStmt = nullptr;
            if (GEAX_IS_NULL(nextStmts[i])) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "ThenStatement is not set in rule StatementBlock: " << K(ret);
            } else if (GEAX_RET_FAIL(
                           VISIT_RULE(nextStmts[i], AstNodeType::kStatementWithYield, nextStmt))) {
                LOG(WARNING) << "Failed to visit rule ThenStatement: " << K(ret);
            } else {
                procedure->appendStatement(nextStmt);
            }
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitNextStatement(GqlParser::NextStatementContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    StatementWithYield* nextStmt = ALLOC_GEAOBJECT(StatementWithYield);
    DEFER(nextStmt, ret);

    GqlParser::YieldClauseContext* yieldCtx = nullptr;
    if (nullptr != (yieldCtx = ctx->yieldClause())) {
        YieldField* yieldNode = nullptr;
        if (GEAX_RET_FAIL(VISIT_RULE(yieldCtx, AstNodeType::kYieldField, yieldNode))) {
            LOG(WARNING) << "Failed to visit rule Yield: " << K(ret);
        } else {
            nextStmt->setYield(yieldNode);
        }
    } else {
        YieldField* yieldNode = ALLOC_GEAOBJECT(YieldField);
        nextStmt->setYield(yieldNode);
    }

    GqlParser::StatementContext* stmtCtx = nullptr;
    Statement* stmt = nullptr;
    if (GEAX_FAIL(ret)) {
    } else if (GEAX_IS_NULL(stmtCtx = ctx->statement())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Statement is not set in rule ThenStatement: " << K(ret);
    } else if (GEAX_RET_FAIL(ACCEPT_RULE(stmtCtx))) {
        LOG(WARNING) << "Failed to visit rule Statement: " << K(ret);
    } else if (GEAX_UNLIKELY(!checkedCast(childRes_, stmt))) {
        ret = GEAXErrorCode::GEAX_OOPS;
        LOG(WARNING) << "Result after visiting rule Statement should be Statement: " << K(ret);
    } else {
        nextStmt->setStatement(stmt);
    }

    return std::any();
}

std::any GQLAstVisitor::visitManagerStatement(GqlParser::ManagerStatementContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    if (ctx->SHOW() != nullptr) {
        ShowProcessListStatement* stmt = ALLOC_GEAOBJECT(ShowProcessListStatement);
        DEFER(stmt, ret);
    } else if (ctx->KILL() != nullptr) {
        KillStatement* stmt = ALLOC_GEAOBJECT(KillStatement);
        GqlParser::KillModeContext* kctx = ctx->killMode();
        DEFER(stmt, ret);

        VInt* integerNode = nullptr;
        GqlParser::IntegerLiteralContext* numCtx = nullptr;
        if (GEAX_IS_NULL(numCtx = ctx->integerLiteral())) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "Query number is not set in kill statement: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_EXPR(numCtx, integerNode))) {
            LOG(WARNING) << "Failed to visit IntegerLiteral: " << K(ret);
        } else {
            stmt->setId(integerNode->val());
            if (kctx != nullptr) {
                if (size_t token = 0; GEAX_RET_FAIL(VISIT_TOKEN(kctx, token))) {
                    LOG(WARNING) << "Failed to visit token killmode";
                } else {
                    switch (token) {
                    case GqlParser::CONNECTION:
                        stmt->setQuery(false);
                        break;
                    case GqlParser::QUERY:
                        stmt->setQuery(true);
                        break;
                    default:
                        ret = GEAXErrorCode::GEAX_OOPS;
                        LOG(WARNING) << "Invalid killmode token type"
                                     << KV("token", token_->getText()) << DK(ret);
                    }
                }
            } else {
                stmt->setQuery(false);
            }
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitLinearCatalogModifyingStatement(
    GqlParser::LinearCatalogModifyingStatementContext* ctx) {
    childRet_ = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    return std::any();
}

std::any GQLAstVisitor::visitQueryStatement(GqlParser::QueryStatementContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    QueryStatement* queryStmt = ALLOC_GEAOBJECT(QueryStatement);
    DEFER(queryStmt, ret);

    GqlParser::JoinQueryExpressionContext* joinCtx = nullptr;
    JoinQueryExpression* joinQuery = nullptr;
    if (GEAX_IS_NULL(joinCtx = ctx->joinQueryExpression())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "JoinQueryExpression is not set in rule QueryStatement: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(joinCtx, AstNodeType::kJoinQuery, joinQuery))) {
        LOG(WARNING) << "Failed to visit joinQueryExpression: " << K(ret);
    } else {
        queryStmt->setJoinQuery(joinQuery);
    }

    return std::any();
}

std::any GQLAstVisitor::visitJoinQueryExpression(GqlParser::JoinQueryExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    JoinQueryExpression* joinQuery = ALLOC_GEAOBJECT(JoinQueryExpression);
    DEFER(joinQuery, ret);

    std::vector<GqlParser::JoinRightPartContext*> joinCtxs = ctx->joinRightPart();
    GqlParser::CompositeQueryStatementContext* compositeCtx = nullptr;
    CompositeQueryStatement* compositeStmt = nullptr;
    if (GEAX_UNLIKELY(joinCtxs.size() != 0)) {
        ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
        LOG(WARNING) << "Join is not supported yet: " << K(ret);
    } else if (GEAX_IS_NULL(compositeCtx = ctx->compositeQueryStatement())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "CompositeQueryStatement is not set in rule JoinQueryExpression: "
                     << K(ret);
    } else if (GEAX_RET_FAIL(
                   VISIT_RULE(compositeCtx, AstNodeType::kCompositeStatement, compositeStmt))) {
        LOG(WARNING) << "Failed to visit CompositeQueryStatement: " << K(ret);
    } else {
        joinQuery->setHead(compositeStmt);
    }

    return std::any();
}

std::any GQLAstVisitor::visitCompositeQueryExpression(
    GqlParser::CompositeQueryExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    CompositeQueryStatement* compositeStmt = ALLOC_GEAOBJECT(CompositeQueryStatement);
    DEFER(compositeStmt, ret);

    auto queryCtxs = ctx->linearQueryStatement();
    auto conjunctionCtxs = ctx->queryConjunction();
    LinearQueryStatement* linearStmt = nullptr;
    if (GEAX_UNLIKELY_NE(queryCtxs.size(), conjunctionCtxs.size() + 1)) {
        ret = GEAXErrorCode::GEAX_OOPS;
        LOG(WARNING) << "Num mismatch: " << KV("queryCtx", queryCtxs.size())
                     << DKV("conjunctionCtx", conjunctionCtxs.size()) << DK(ret);
    } else if (GEAX_RET_FAIL(ACCEPT_RULE(queryCtxs[0]))) {
        LOG(WARNING) << "Failed to visit rule LinearQueryStatement: " << K(ret);
    } else if (GEAX_UNLIKELY(!checkedCast(childRes_, linearStmt))) {
        ret = GEAXErrorCode::GEAX_OOPS;
        LOG(WARNING) << "Child result should be a LinearQueryStatement: " << K(ret);
    } else {
        compositeStmt->setHead(linearStmt);
        for (auto i = 0u; i < conjunctionCtxs.size() && GEAX_OK(ret); ++i) {
            LinearQueryStatement* stmt = nullptr;
            QueryConjunctionType* type = nullptr;
            if (GEAX_ANY_NULL(conjunctionCtxs[i], queryCtxs[i + 1])) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "Child of CompositeQueryStatement is not set: " << K(ret);
            } else if (GEAX_RET_FAIL(ACCEPT_RULE(queryCtxs[i + 1]))) {
                LOG(WARNING) << "Failed to visit rule LinearQueryStatement: " << K(ret);
            } else if (GEAX_UNLIKELY(!checkedCast(childRes_, stmt))) {
                ret = GEAXErrorCode::GEAX_OOPS;
                LOG(WARNING) << "Child result should be a LinearQueryStatement: " << K(ret);
            } else if (GEAX_RET_FAIL(ACCEPT_RULE(conjunctionCtxs[i]))) {
                LOG(WARNING) << "Failed to visit set operator " << K(ret);
            } else if (GEAX_UNLIKELY(!checkedCast(childRes_, type))) {
                ret = GEAXErrorCode::GEAX_OOPS;
                LOG(WARNING) << "Child result should be a QueryConjunctionType: " << K(ret);
            } else {
                compositeStmt->appendBody(type, stmt);
            }
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitFocusedPrimitiveResultStatement(
    GqlParser::FocusedPrimitiveResultStatementContext* ctx) {
    childRet_ = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    return std::any();
}

std::any GQLAstVisitor::visitFocusedQueryStatement(GqlParser::FocusedQueryStatementContext* ctx) {
    childRet_ = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    return std::any();
}

std::any GQLAstVisitor::visitAmbientLinearQueryStatement(
    GqlParser::AmbientLinearQueryStatementContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    AmbientLinearQueryStatement* ambientStmt = ALLOC_GEAOBJECT(AmbientLinearQueryStatement);
    DEFER(ambientStmt, ret);

    PrimitiveResultStatement* result = nullptr;
    GqlParser::PrimitiveResultStatementContext* resultCtx = nullptr;
    GqlParser::SimpleLinearQueryStatementContext* linearStmtCtx = nullptr;
    if (GEAX_IS_NULL(resultCtx = ctx->primitiveResultStatement())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "PrimitiveResultStatement is not set in AmbientLinearQueryStatement: "
                     << K(ret);
    } else if (GEAX_RET_FAIL(
                   VISIT_RULE(resultCtx, AstNodeType::kPrimitiveResultStatement, result))) {
        LOG(WARNING) << "Failed to visit PrimitiveResultStatement: " << K(ret);
    } else {
        ambientStmt->setResultStatement(result);
    }

    if (GEAX_FAIL(ret)) {
    } else if (nullptr != (linearStmtCtx = ctx->simpleLinearQueryStatement())) {
        auto queryStmtList = linearStmtCtx->simpleQueryStatement();
        for (auto i = 0u; GEAX_OK(ret) && i < queryStmtList.size(); ++i) {
            if (GEAX_IS_NULL(queryStmtList[i])) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "SimpleQueryStatement is not set in SimpleLinearQueryStatement: "
                             << K(ret);
            } else if (GEAX_RET_FAIL(ACCEPT_RULE(queryStmtList[i]))) {
                LOG(WARNING) << "Failed to visit rule SimpleQueryStatement: " << K(ret);
            } else {
                switch (childRes_->type()) {
                case AstNodeType::kMatchStatement:
                    ambientStmt->appendQueryStatement(
                        castAs<MatchStatement>(childRes_, AstNodeType::kMatchStatement));
                    break;
                case AstNodeType::kFilterStatement:
                    ambientStmt->appendQueryStatement(
                        castAs<FilterStatement>(childRes_, AstNodeType::kFilterStatement));
                    break;
                default:
                    ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
                    LOG(WARNING) << "Child type is not supported. " << KV("type", childRes_->type())
                                 << DK(ret);
                    break;
                }
            }
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitPrimitiveResultStatement(
    GqlParser::PrimitiveResultStatementContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    PrimitiveResultStatement* result = ALLOC_GEAOBJECT(PrimitiveResultStatement);
    DEFER(result, ret);

    GqlParser::ReturnStatementContext* returnCtx = nullptr;
    GqlParser::OrderByAndPageStatementContext* orderByCtx = nullptr;
    if (GEAX_IS_NULL(returnCtx = ctx->returnStatement())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "ReturnStatement is not set in PrimitiveResultStatement: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(returnCtx, result))) {
        LOG(WARNING) << "Failed is to visit ReturnStatement: " << K(ret);
    }

    if (GEAX_FAIL(ret)) {
    } else if (nullptr != (orderByCtx = ctx->orderByAndPageStatement())) {
        if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(orderByCtx, result))) {
            LOG(WARNING) << "Failed is to visit OrderByAndPageStatement: " << K(ret);
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitAmbientLinearDataModifyingStatementBody(
    GqlParser::AmbientLinearDataModifyingStatementBodyContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    LinearDataModifyingStatement* dataModifyStmt = ALLOC_GEAOBJECT(LinearDataModifyingStatement);
    DEFER(dataModifyStmt, ret);

    PrimitiveResultStatement* result = nullptr;
    GqlParser::PrimitiveResultStatementContext* resultCtx = nullptr;
    GqlParser::SimpleLinearQueryStatementContext* linearStmtCtx = nullptr;

    if (nullptr != (resultCtx = ctx->primitiveResultStatement())) {
        if (GEAX_RET_FAIL(VISIT_RULE(resultCtx, AstNodeType::kPrimitiveResultStatement, result))) {
            LOG(WARNING) << "Failed to visit PrimitiveResultStatement: " << K(ret);
        } else {
            dataModifyStmt->setResultStatement(result);
        }
    }

    if (GEAX_FAIL(ret)) {
    } else if (nullptr != (linearStmtCtx = ctx->simpleLinearQueryStatement())) {
        auto queryStmtList = linearStmtCtx->simpleQueryStatement();
        for (auto i = 0u; GEAX_OK(ret) && i < queryStmtList.size(); ++i) {
            if (GEAX_IS_NULL(queryStmtList[i])) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "SimpleQueryStatement is not set in SimpleLinearQueryStatement: "
                             << K(ret);
            } else if (GEAX_RET_FAIL(ACCEPT_RULE(queryStmtList[i]))) {
                LOG(WARNING) << "Failed to visit rule SimpleQueryStatement: " << K(ret);
            } else {
                switch (childRes_->type()) {
                case AstNodeType::kMatchStatement: {
                    MatchStatement* match = nullptr;
                    if (GEAX_IS_NULL(match = castAs<MatchStatement>(
                                         childRes_, AstNodeType::kMatchStatement))) {
                        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                        LOG(WARNING) << "Failed to cast to node kMatchStatement: " << K(ret);
                    } else {
                        // TODO(ljr) : multi pathPatterns convert multiMatch. If multi pathPatterns
                        // are implemented, change them back.
                        auto graphPattern = match->graphPattern();
                        if (GEAX_IS_NULL(graphPattern)) {
                            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                            LOG(WARNING) << "GraphPattern is not set in MatchStatement: " << K(ret);
                        } else if (graphPattern->pathPatterns().size() > 1) {
                            // do not process mode/keep/where/yield clause
                            if (graphPattern->keep().has_value() ||
                                graphPattern->matchMode().has_value() ||
                                graphPattern->where().has_value() ||
                                (graphPattern->yield() != nullptr &&
                                 !graphPattern->yield()->items().empty())) {
                                ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
                                LOG(WARNING) << "mode/keep/where/yield is not supported in "
                                                "converting to multi-match: "
                                             << K(ret);
                            } else {
                                for (auto& pathPattern : graphPattern->pathPatterns()) {
                                    if (pathPattern->prefix().has_value() ||
                                        pathPattern->alias().has_value() ||
                                        pathPattern->opType().has_value() ||
                                        pathPattern->chains().size() != 1 ||
                                        (pathPattern->chains()[0] != nullptr &&
                                         pathPattern->chains()[0]->tails().size() != 0)) {
                                        ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
                                        LOG(WARNING) << "complex pathpattern is not supported in "
                                                        "converting to multi-match: "
                                                     << K(ret);
                                    } else {
                                        GraphPattern* newGraphPattern =
                                            ALLOC_GEAOBJECT(GraphPattern);
                                        newGraphPattern->appendPathPattern(pathPattern);
                                        // but yield is not optional, so we must set a empty yield
                                        YieldField* yieldNode = ALLOC_GEAOBJECT(YieldField);
                                        newGraphPattern->setYield(yieldNode);
                                        MatchStatement* matchTmp = ALLOC_GEAOBJECT(MatchStatement);
                                        matchTmp->setGraphPattern(newGraphPattern);
                                        dataModifyStmt->appendQueryStatement(matchTmp);
                                    }
                                }
                            }
                        } else {
                            dataModifyStmt->appendQueryStatement(match);
                        }
                    }
                    break;
                }
                default:
                    ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
                    LOG(WARNING) << "Child type is not supported. " << KV("type", childRes_->type())
                                 << DK(ret);
                    break;
                }
            }
        }
    }
    auto modifyList = ctx->simpleDataModifyingStatement();
    for (auto i = 0u; GEAX_OK(ret) && i < modifyList.size(); ++i) {
        if (GEAX_IS_NULL(modifyList[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "Child is not set in AmbientLinearDataModifyingStatementBody: "
                         << K(ret);
        } else if (GEAX_RET_FAIL(ACCEPT_RULE(modifyList[i]))) {
            LOG(WARNING) << "Failed to visit SimpleDataModifyingStatement: " << K(ret);
        } else {
            switch (childRes_->type()) {
            case AstNodeType::kInsertStatement: {
                InsertStatement* insert =
                    castAs<InsertStatement>(childRes_, AstNodeType::kInsertStatement);
                dataModifyStmt->appendModifyStatement(insert);
                break;
            }
            case AstNodeType::kDeleteStatement: {
                DeleteStatement* del =
                    castAs<DeleteStatement>(childRes_, AstNodeType::kDeleteStatement);
                dataModifyStmt->appendModifyStatement(del);
                break;
            }
            case AstNodeType::kSetStatement: {
                SetStatement* set = castAs<SetStatement>(childRes_, AstNodeType::kSetStatement);
                dataModifyStmt->appendModifyStatement(set);
                break;
            }
            case AstNodeType::kReplaceStatement: {
                ReplaceStatement* replace =
                    castAs<ReplaceStatement>(childRes_, AstNodeType::kReplaceStatement);
                dataModifyStmt->appendModifyStatement(replace);
                break;
            }
            default:
                ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
                LOG(WARNING) << "Child type is not supported. " << KV("type", childRes_->type())
                             << DK(ret);
                break;
            }
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitInsertStatement(GqlParser::InsertStatementContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    InsertStatement* insert = ALLOC_GEAOBJECT(InsertStatement);
    DEFER(insert, ret);

    GqlParser::InsertGraphPatternContext* insertGraphPatternCtx = nullptr;
    if (GEAX_IS_NULL(insertGraphPatternCtx = ctx->insertGraphPattern())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "InsertGraphPatternContext is not set: " << K(ret);
    } else {
        GqlParser::InsertPathPatternListContext* patternListCtx = nullptr;
        if (GEAX_IS_NULL(patternListCtx = insertGraphPatternCtx->insertPathPatternList())) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "InsertPathPatternListContext is not set: " << K(ret);
        } else {
            auto patterns = patternListCtx->insertPathPattern();
            for (size_t i = 0u; i < patterns.size() && GEAX_OK(ret); ++i) {
                PathChain* pathChain = nullptr;
                if (GEAX_IS_NULL(patterns[i])) {
                    ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                    LOG(WARNING) << "simplePathPattern is not set: " << K(ret);
                } else if (GEAX_RET_FAIL(
                               VISIT_RULE(patterns[i], AstNodeType::kPathChain, pathChain))) {
                    LOG(WARNING) << "Fail to visit SimplePathPattern: " << K(ret);
                } else {
                    insert->appendPath(pathChain);
                }
            }
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitSetStatement(GqlParser::SetStatementContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    SetStatement* setStmt = ALLOC_GEAOBJECT(SetStatement);
    DEFER(setStmt, ret);

    GqlParser::SetItemListContext* setListCtx = nullptr;
    if (GEAX_IS_NULL(setListCtx = ctx->setItemList())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "SetItemList is not set: " << K(ret);
    } else {
        std::vector<GqlParser::SetItemContext*> setItemCtxVec = setListCtx->setItem();
        for (size_t i = 0; GEAX_OK(ret) && i < setItemCtxVec.size(); ++i) {
            if (GEAX_IS_NULL(setItemCtxVec[i])) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "SetItem is not set in SetStatement: " << K(ret);
            } else if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(setItemCtxVec[i], setStmt))) {
                LOG(WARNING) << "Failed to visit setItem: " << K(ret);
            }
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitSetPropertyItem(GqlParser::SetPropertyItemContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    GqlParser::BindingVariableContext* refCtx = nullptr;
    GqlParser::PropertyNameContext* nameCtx = nullptr;
    GqlParser::ExpressionContext* exprCtx = nullptr;
    Expr* expr = nullptr;
    SetStatement* setStmt = nullptr;
    if (GEAX_IS_NULL(setStmt = castAs<SetStatement>(faRes_, AstNodeType::kSetStatement))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kSetStatement: " << K(ret);
    } else if (GEAX_IS_NULL(ctx->bindingVariableReference()) ||
               GEAX_IS_NULL(refCtx = ctx->bindingVariableReference()->bindingVariable())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "BindingVariable is not set: " << K(ret);
    } else if (GEAX_IS_NULL(nameCtx = ctx->propertyName())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "PropertyName is not set: " << K(ret);
    } else if (GEAX_IS_NULL(exprCtx = ctx->expression())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Expression is not set: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, expr))) {
        LOG(WARNING) << "Failed to visit rule Expression: " << K(ret);
    } else {
        SetSingleProperty* singleProperty = ALLOC_GEAOBJECT(SetSingleProperty);
        singleProperty->setV(refCtx->getText());
        singleProperty->setProperty(trimAccentGrave(nameCtx->getText()));
        singleProperty->setValue(expr);
        setStmt->appendItem(singleProperty);
    }

    return std::any();
}

std::any GQLAstVisitor::visitSetAllPropertiesItem(GqlParser::SetAllPropertiesItemContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    GqlParser::PropertyKeyValuePairListContext* propListCtx = nullptr;
    GqlParser::BindingVariableContext* refCtx = nullptr;
    PropStruct* prop = nullptr;
    SetStatement* setStmt = nullptr;
    if (GEAX_IS_NULL(setStmt = castAs<SetStatement>(faRes_, AstNodeType::kSetStatement))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kSetStatement: " << K(ret);
    } else if (GEAX_IS_NULL(ctx->bindingVariableReference()) ||
               GEAX_IS_NULL(refCtx = ctx->bindingVariableReference()->bindingVariable())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "BindingVariable is not set : " << K(ret);
    } else if (GEAX_IS_NULL(propListCtx = ctx->propertyKeyValuePairList())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "PropertyKeyValuePairList is not set: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(propListCtx, AstNodeType::kPropStruct, prop))) {
        LOG(WARNING) << "Failed to visit PropertyKeyValuePairList: " << K(ret);
    } else {
        SetAllProperties* properties = ALLOC_GEAOBJECT(SetAllProperties);
        properties->setV(refCtx->getText());
        properties->setStructs(prop);
        setStmt->appendItem(properties);
    }

    return std::any();
}

std::any GQLAstVisitor::visitUpdatePropertiesItem(GqlParser::UpdatePropertiesItemContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    GqlParser::PropertyKeyValuePairListContext* propListCtx = nullptr;
    GqlParser::BindingVariableContext* refCtx = nullptr;
    PropStruct* prop = nullptr;
    SetStatement* setStmt = nullptr;
    if (GEAX_IS_NULL(setStmt = castAs<SetStatement>(faRes_, AstNodeType::kSetStatement))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kSetStatement: " << K(ret);
    } else if (GEAX_IS_NULL(ctx->bindingVariableReference()) ||
               GEAX_IS_NULL(refCtx = ctx->bindingVariableReference()->bindingVariable())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "BindingVariable is not set : " << K(ret);
    } else if (GEAX_IS_NULL(propListCtx = ctx->propertyKeyValuePairList())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "PropertyKeyValuePairList is not set: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(propListCtx, AstNodeType::kPropStruct, prop))) {
        LOG(WARNING) << "Failed to visit PropertyKeyValuePairList: " << K(ret);
    } else {
        UpdateProperties* properties = ALLOC_GEAOBJECT(UpdateProperties);
        properties->setV(refCtx->getText());
        properties->setStructs(prop);
        setStmt->appendItem(properties);
    }

    return std::any();
}

std::any GQLAstVisitor::visitSetLabelItem(GqlParser::SetLabelItemContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    GqlParser::LabelNameContext* labelCtx = nullptr;
    GqlParser::BindingVariableContext* refCtx = nullptr;
    SingleLabel* label = nullptr;
    SetStatement* setStmt = nullptr;
    if (GEAX_IS_NULL(setStmt = castAs<SetStatement>(faRes_, AstNodeType::kSetStatement))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kSetStatement: " << K(ret);
    } else if (GEAX_IS_NULL(ctx->bindingVariableReference()) ||
               GEAX_IS_NULL(refCtx = ctx->bindingVariableReference()->bindingVariable())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "BindingVariable is not set : " << K(ret);
    } else if (GEAX_IS_NULL(labelCtx = ctx->labelName())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "LabelName is not set: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(labelCtx, AstNodeType::kSingleLabel, label))) {
        LOG(WARNING) << "Failed visit LabelName: " << K(ret);
    } else {
        SetLabel* setLable = ALLOC_GEAOBJECT(SetLabel);
        setLable->setV(refCtx->getText());
        setLable->setLabel(label);
        setStmt->appendItem(setLable);
    }

    return std::any();
}

std::any GQLAstVisitor::visitDeleteStatement(GqlParser::DeleteStatementContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DeleteStatement* del = ALLOC_GEAOBJECT(DeleteStatement);
    DEFER(del, ret);

    GqlParser::DeleteItemListContext* deleteItemListCtx = nullptr;
    if (nullptr != ctx->DETACH() || nullptr != ctx->NODETACH()) {
        ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
        LOG(WARNING) << "DETACH/NODETACH is not supported in DeleteStatement: " << K(ret);
    } else if (GEAX_IS_NULL(deleteItemListCtx = ctx->deleteItemList())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "DeleteItemList is not set in DeleteStatement: " << K(ret);
    } else {
        auto deleteItemCtxVec = deleteItemListCtx->deleteItem();
        for (size_t i = 0; GEAX_OK(ret) && i < deleteItemCtxVec.size(); ++i) {
            GqlParser::ExpressionContext* exprCtx = nullptr;
            Ref* ref = nullptr;
            if (GEAX_IS_NULL(deleteItemCtxVec[i])) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "DeleteItem is not set in DeleteStatement: " << K(ret);
            } else if (GEAX_IS_NULL(exprCtx = deleteItemCtxVec[i]->expression())) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "Failed to expression " << K(ret);
            } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, ref))) {
                LOG(WARNING) << "Failed to visit expr: " << K(ret);
            } else {
                del->appendItem(std::move(ref->name()));
            }
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitReplaceStatement(GqlParser::ReplaceStatementContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    ReplaceStatement* replace = ALLOC_GEAOBJECT(ReplaceStatement);
    DEFER(replace, ret);

    GqlParser::InsertGraphPatternContext* insertGraphPatternCtx = nullptr;
    if (GEAX_IS_NULL(insertGraphPatternCtx = ctx->insertGraphPattern())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "InsertGraphPatternContext is not set: " << K(ret);
    } else {
        GqlParser::InsertPathPatternListContext* patternListCtx = nullptr;
        if (GEAX_IS_NULL(patternListCtx = insertGraphPatternCtx->insertPathPatternList())) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "InsertPathPatternListContext is not set: " << K(ret);
        } else {
            auto patterns = patternListCtx->insertPathPattern();
            for (size_t i = 0u; i < patterns.size() && GEAX_OK(ret); ++i) {
                PathChain* pathChain = nullptr;
                if (GEAX_IS_NULL(patterns[i])) {
                    ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                    LOG(WARNING) << "simplePathPattern is not set: " << K(ret);
                } else if (GEAX_RET_FAIL(
                               VISIT_RULE(patterns[i], AstNodeType::kPathChain, pathChain))) {
                    LOG(WARNING) << "Fail to visit SimplePathPattern: " << K(ret);
                } else {
                    replace->appendPath(pathChain);
                }
            }
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitFilterStatement(GqlParser::FilterStatementContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    FilterStatement* filter = ALLOC_GEAOBJECT(FilterStatement);
    DEFER(filter, ret);

    GqlParser::WhereClauseContext* whereCtx = nullptr;
    GqlParser::ExpressionContext* exprCtx = nullptr;
    Expr* expr = nullptr;
    if ((whereCtx = ctx->whereClause()) != nullptr) {
        exprCtx = whereCtx->expression();
    } else {
        exprCtx = ctx->expression();
    }
    if (GEAX_IS_NULL(exprCtx)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Expression is not set in rule FilterStatementContext: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, expr))) {
        LOG(WARNING) << "Failed to visit rule Expression: " << K(ret);
    } else {
        filter->setPredicate(expr);
    }

    return std::any();
}

std::any GQLAstVisitor::visitSimpleMatchStatement(GqlParser::SimpleMatchStatementContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    MatchStatement* match = ALLOC_GEAOBJECT(MatchStatement);
    DEFER(match, ret);

    GqlParser::GraphPatternBindingTableContext* graphBindingTableCtx = nullptr;
    GqlParser::GraphPatternContext* graphPatternCtx = nullptr;
    GqlParser::GraphPatternYieldClauseContext* yieldCtx = nullptr;
    GraphPattern* graphPattern = nullptr;

    if (GEAX_IS_NULL(graphBindingTableCtx = ctx->graphPatternBindingTable())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "GraphPatternBindingTable is not set in SimpleMatchStatement: " << K(ret);
    } else if (GEAX_IS_NULL(graphPatternCtx = graphBindingTableCtx->graphPattern())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "GraphPattern is not set in GraphPatternBindingTable: " << K(ret);
    } else if (GEAX_RET_FAIL(
                   VISIT_RULE(graphPatternCtx, AstNodeType::kGraphPattern, graphPattern))) {
        LOG(WARNING) << "Failed to visit rule GraphPattern: " << K(ret);
    } else {
        match->setGraphPattern(graphPattern);
    }
    if (GEAX_FAIL(ret)) {
    } else if (nullptr != (yieldCtx = graphBindingTableCtx->graphPatternYieldClause())) {
        YieldField* yieldNode = nullptr;
        if (GEAX_RET_FAIL(VISIT_RULE(yieldCtx, AstNodeType::kYieldField, yieldNode))) {
            LOG(WARNING) << "Failed to visit rule Yield: " << K(ret);
        } else {
            graphPattern->setYield(yieldNode);
        }
    } else {
        YieldField* yieldNode = ALLOC_GEAOBJECT(YieldField);
        graphPattern->setYield(yieldNode);
    }

    return std::any();
}

std::any GQLAstVisitor::visitOptionalMatchStatement(GqlParser::OptionalMatchStatementContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    MatchStatement* match = nullptr;
    DEFER(match, ret);

    GqlParser::OptionalOperandContext* optionCtx = nullptr;
    if (GEAX_IS_NULL(optionCtx = ctx->optionalOperand())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "OptionalOperand is not set in OptionalMatchStatement: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(optionCtx, AstNodeType::kMatchStatement, match))) {
        LOG(WARNING) << "Failed to visit rule GraphPattern: " << K(ret);
    } else {
        match->setStatementMode(StatementMode::kOptional);
    }

    return std::any();
}

std::any GQLAstVisitor::visitMatchStatementBlock(GqlParser::MatchStatementBlockContext* ctx) {
    childRet_ = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    return std::any();
}

std::any GQLAstVisitor::visitGraphPattern(GqlParser::GraphPatternContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    GraphPattern* graphPattern = ALLOC_GEAOBJECT(GraphPattern);
    DEFER(graphPattern, ret);

    GqlParser::MatchModeContext* modeCtx = nullptr;
    GqlParser::PathPatternListContext* pathCtx = nullptr;
    GqlParser::KeepClauseContext* keepCtx = nullptr;
    GqlParser::WhereClauseContext* whereCtx = nullptr;
    for (auto i = 0u; GEAX_OK(ret) && i < ctx->children.size(); ++i) {
        pathCtx = nullptr;
        if (nullptr == modeCtx && checkedCast(ctx->children[i], modeCtx)) {
            if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(modeCtx, graphPattern))) {
                LOG(WARNING) << "Failed to visit rule MatchMode: " << K(ret);
            }
        } else if (checkedCast(ctx->children[i], pathCtx)) {
            if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(pathCtx, graphPattern))) {
                LOG(WARNING) << "Faield to visit rule PathPatternList: " << K(ret);
            }
        } else if (nullptr == keepCtx && checkedCast(ctx->children[i], keepCtx)) {
            PathPrefix* keep = nullptr;
            if (GEAX_RET_FAIL(ACCEPT_RULE(keepCtx))) {
                LOG(WARNING) << "Failed to visit rule KeepClause: " << K(ret);
            } else if (GEAX_UNLIKELY(!checkedCast(childRes_, keep))) {
                ret = GEAXErrorCode::GEAX_OOPS;
                LOG(WARNING) << "Child result should be a PathPrefix: " << K(ret);
            } else {
                graphPattern->setKeep(keep);
            }
        } else if (nullptr == whereCtx && checkedCast(ctx->children[i], whereCtx)) {
            Expr* expr = nullptr;
            GqlParser::ExpressionContext* exprCtx = nullptr;
            if (GEAX_IS_NULL(exprCtx = whereCtx->expression())) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "Child of WhereClause is not set: " << K(ret);
            } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, expr))) {
                LOG(WARNING) << "Failed to visit rule Expression: " << K(ret);
            } else {
                graphPattern->setWhere(expr);
            }
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitPathPatternList(GqlParser::PathPatternListContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    GraphPattern* graphPattern = nullptr;
    if (GEAX_IS_NULL(graphPattern = castAs<GraphPattern>(faRes_, AstNodeType::kGraphPattern))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kGraphPattern: " << K(ret);
    }

    std::vector<GqlParser::PathPatternContext*> pathCtxs = ctx->pathPattern();
    for (auto i = 0u; GEAX_OK(ret) && i < pathCtxs.size(); ++i) {
        PathPattern* pathPattern = nullptr;
        if (GEAX_IS_NULL(pathCtxs[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "PathPattern is not set in rule PathPatternList: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_RULE(pathCtxs[i], AstNodeType::kPathPattern, pathPattern))) {
            LOG(WARNING) << "Failed to visit rule PathPattern: " << K(ret);
        } else {
            graphPattern->appendPathPattern(pathPattern);
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitPathPattern(GqlParser::PathPatternContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    PathPattern* pathPattern = ALLOC_GEAOBJECT(PathPattern);
    DEFER(pathPattern, ret);

    for (auto i = 0u; i < ctx->children.size() && GEAX_OK(ret); ++i) {
        GqlParser::PathVariableDeclarationContext* varCtx = nullptr;
        GqlParser::PathPatternPrefixContext* prefixCtx = nullptr;
        GqlParser::PathPatternExpressionContext* pathExprCtx = nullptr;
        if (checkedCast(ctx->children[i], varCtx)) {
            GqlParser::PathVariableContext* pathVarCtx = nullptr;
            if (GEAX_IS_NULL(pathVarCtx = varCtx->pathVariable())) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "PathVariable is not set in rule PathVariableDeclaration: "
                             << K(ret);
            } else {
                pathPattern->setAlias(pathVarCtx->getText());
            }
        } else if (checkedCast(ctx->children[i], prefixCtx)) {
            PathPrefix* prefix = nullptr;
            if (GEAX_RET_FAIL(ACCEPT_RULE(prefixCtx))) {
                LOG(WARNING) << "Failed to visit rule PathPatternPrefix: " << K(ret);
            } else if (GEAX_UNLIKELY(!checkedCast(childRes_, prefix))) {
                ret = GEAXErrorCode::GEAX_OOPS;
                LOG(WARNING) << "Child result should be a PathPrefix: " << K(ret);
            } else {
                pathPattern->setPrefix(prefix);
            }
        } else if (checkedCast(ctx->children[i], pathExprCtx)) {
            PathChain* pathChain = nullptr;
            if (GEAX_RET_FAIL(VISIT_RULE(pathExprCtx, AstNodeType::kPathChain, pathChain))) {
                LOG(WARNING) << "Failed to visit rule PathPatternExpression: " << K(ret);
            } else {
                pathPattern->appendChain(pathChain);
            }
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitPathTerm(GqlParser::PathTermContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    PathChain* pathChain = ALLOC_GEAOBJECT(PathChain);
    DEFER(pathChain, ret);

    Node* start = nullptr;
    std::vector<GqlParser::PathFactorContext*> pathCtxs = ctx->pathFactor();
    if (GEAX_UNLIKELY(pathCtxs.empty())) {
        ret = GEAXErrorCode::GEAX_OOPS;
        LOG(WARNING) << "Child of rule PathFactor should be 0: " << K(ret);
    } else if (GEAX_IS_NULL(pathCtxs[0])) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Child of rule PathFactor is not set: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(pathCtxs[0], AstNodeType::kNode, start))) {
        LOG(WARNING) << "Failed to visit rule PathFactor: " << K(ret);
    } else {
        pathChain->setHead(start);
    }
    for (auto i = 1u; i + 1 < pathCtxs.size() && GEAX_OK(ret); i += 2) {
        Node* v = nullptr;
        Edge* e = nullptr;
        if (GEAX_ANY_NULL(pathCtxs[i], pathCtxs[i + 1])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "PathFactor is not set in rule PathFactor: "
                         << KMP(pathCtxs[i], pathCtxs[i + 1]) << DKM(i, ret);
        } else if (GEAX_RET_FAIL(VISIT_RULE(pathCtxs[i], AstNodeType::kEdge, e))) {
            LOG(WARNING) << "Failed to visit rule PathFactor: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_RULE(pathCtxs[i + 1], AstNodeType::kNode, v))) {
            LOG(WARNING) << "Failed to visit rule PathFactor: " << K(ret);
        } else {
            pathChain->appendTail(e, v);
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitPathPatternUnion(GqlParser::PathPatternUnionContext* ctx) {
    childRet_ = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    return std::any();
}
std::any GQLAstVisitor::visitPathMultisetAlternation(
    GqlParser::PathMultisetAlternationContext* ctx) {
    childRet_ = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    return std::any();
}

std::any GQLAstVisitor::visitRepeatableElementsMatchMode(
    GqlParser::RepeatableElementsMatchModeContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    GraphPattern* graphPattern = nullptr;
    if (GEAX_IS_NULL(graphPattern = castAs<GraphPattern>(faRes_, AstNodeType::kGraphPattern))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kGraphPattern: " << K(ret);
    } else {
        graphPattern->setMatchMode(MatchMode::kRepeatable);
    }

    return std::any();
}

std::any GQLAstVisitor::visitDifferentEdgesMatchMode(
    GqlParser::DifferentEdgesMatchModeContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    GraphPattern* graphPattern = nullptr;
    if (GEAX_IS_NULL(graphPattern = castAs<GraphPattern>(faRes_, AstNodeType::kGraphPattern))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kGraphPattern: " << K(ret);
    } else {
        graphPattern->setMatchMode(MatchMode::kDifferent);
    }

    return std::any();
}

std::any GQLAstVisitor::visitYieldItemList(GqlParser::YieldItemListContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    YieldField* yield = ALLOC_GEAOBJECT(YieldField);
    DEFER(yield, ret);

    std::vector<GqlParser::YieldItemContext*> yieldItems = ctx->yieldItem();
    for (auto i = 0u; GEAX_OK(ret) && i < yieldItems.size(); ++i) {
        if (GEAX_IS_NULL(yieldItems[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "YieldItem is not set in YieldItemList: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(yieldItems[i], yield))) {
            LOG(WARNING) << "Failed to visit rule YieldItem: " << K(ret);
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitYieldItem(GqlParser::YieldItemContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    YieldField* yield = nullptr;
    GqlParser::YieldItemNameContext* nameCtx = nullptr;
    if (GEAX_IS_NULL(yield = castAs<YieldField>(faRes_, AstNodeType::kYieldField))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kYieldField: " << K(ret);
    } else if (GEAX_IS_NULL(nameCtx = ctx->yieldItemName())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "YieldItemName is not set in YieldItem: " << K(ret);
    } else {
        std::string alias;
        GqlParser::YieldItemAliasContext* aliasCtx = nullptr;
        if ((aliasCtx = ctx->yieldItemAlias()) != nullptr) {
            GqlParser::BindingVariableContext* varCtx = nullptr;
            if (GEAX_IS_NULL(varCtx = aliasCtx->bindingVariable())) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "Identifier is not set in YieldItem: " << K(ret);
            } else {
                alias = varCtx->getText();
            }
        } else {
            alias = nameCtx->getText();
        }
        if (GEAX_OK(ret)) {
            yield->appendItem(nameCtx->getText(), std::move(alias));
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitPathModePrefix(GqlParser::PathModePrefixContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    PathModePrefix* prefix = ALLOC_GEAOBJECT(PathModePrefix);
    DEFER(prefix, ret);

    GqlParser::PathModeContext* pathModeCtx = nullptr;
    ModeType modeType;
    if (GEAX_IS_NULL(pathModeCtx = ctx->pathMode())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Child of rule PathModePrefixContext is not set: " << K(ret);
    } else if (GEAX_RET_FAIL(visitPathMode(pathModeCtx, modeType))) {
        LOG(WARNING) << "Failed to visit PathMode: " << K(ret);
    } else {
        prefix->setPathMode(modeType);
    }

    return std::any();
}

std::any GQLAstVisitor::visitAllPathSearch(GqlParser::AllPathSearchContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    PathSearchPrefix* prefix = ALLOC_GEAOBJECT(PathSearchPrefix);
    DEFER(prefix, ret);

    GqlParser::PathModeContext* pathModeCtx = nullptr;
    if (nullptr != (pathModeCtx = ctx->pathMode())) {
        ModeType modeType;
        if (GEAX_RET_FAIL(visitPathMode(pathModeCtx, modeType))) {
            LOG(WARNING) << "Failed to visit PathMode: " << K(ret);
        } else {
            prefix->setSearchMode(modeType);
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitAnyPathSearch(GqlParser::AnyPathSearchContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    PathSearchPrefix* prefix = ALLOC_GEAOBJECT(PathSearchPrefix);
    DEFER(prefix, ret);

    GqlParser::PathModeContext* pathModeCtx = nullptr;
    GqlParser::NumberOfPathsContext* numCtx = nullptr;
    prefix->setSearchType(SearchType::kAnySearch);
    if (nullptr != (numCtx = ctx->numberOfPaths())) {
        VInt* integerNode = nullptr;
        if (GEAX_RET_FAIL(VISIT_EXPR(numCtx, integerNode))) {
            LOG(WARNING) << "Failed to visit IntegerLiteral: " << K(ret);
        } else {
            prefix->setNum(integerNode->val());
        }
    }
    if (GEAX_FAIL(ret)) {
    } else if (nullptr != (pathModeCtx = ctx->pathMode())) {
        ModeType modeType;
        if (GEAX_RET_FAIL(visitPathMode(pathModeCtx, modeType))) {
            LOG(WARNING) << "Failed to visit PathMode: " << K(ret);
        } else {
            prefix->setSearchMode(modeType);
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitAllShortestPathSearch(GqlParser::AllShortestPathSearchContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    PathSearchPrefix* prefix = ALLOC_GEAOBJECT(PathSearchPrefix);
    DEFER(prefix, ret);
    GqlParser::PathModeContext* pathModeCtx = nullptr;
    if (nullptr != (pathModeCtx = ctx->pathMode())) {
        ModeType modeType;
        if (GEAX_RET_FAIL(visitPathMode(pathModeCtx, modeType))) {
            LOG(WARNING) << "Failed to visit PathMode: " << K(ret);
        } else {
            prefix->setSearchMode(modeType);
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitAnyShortestPathSearch(GqlParser::AnyShortestPathSearchContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    PathSearchPrefix* prefix = ALLOC_GEAOBJECT(PathSearchPrefix);
    DEFER(prefix, ret);

    GqlParser::PathModeContext* pathModeCtx = nullptr;
    prefix->setSearchType(SearchType::kAnyShortest);
    if (nullptr != (pathModeCtx = ctx->pathMode())) {
        ModeType modeType;
        if (GEAX_RET_FAIL(visitPathMode(pathModeCtx, modeType))) {
            LOG(WARNING) << "Failed to visit PathMode: " << K(ret);
        } else {
            prefix->setSearchMode(modeType);
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitCountedShortestPathSearch(
    GqlParser::CountedShortestPathSearchContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    PathSearchPrefix* prefix = ALLOC_GEAOBJECT(PathSearchPrefix);
    DEFER(prefix, ret);

    GqlParser::PathModeContext* pathModeCtx = nullptr;
    GqlParser::NumberOfPathsContext* numCtx = nullptr;
    prefix->setSearchType(SearchType::kCountedShortestPaths);
    VInt* integerNode = nullptr;
    if (GEAX_IS_NULL(numCtx = ctx->numberOfPaths())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "NumberOfPaths is not set in CountedShortestPathSearch: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(numCtx, integerNode))) {
        LOG(WARNING) << "Failed to visit IntegerLiteral: " << K(ret);
    } else {
        prefix->setNum(integerNode->val());
    }
    if (GEAX_FAIL(ret)) {
    } else if (nullptr != (pathModeCtx = ctx->pathMode())) {
        ModeType modeType;
        if (GEAX_RET_FAIL(visitPathMode(pathModeCtx, modeType))) {
            LOG(WARNING) << "Failed to visit PathMode: " << K(ret);
        } else {
            prefix->setSearchMode(modeType);
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitCountedShortestGroupSearch(
    GqlParser::CountedShortestGroupSearchContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    PathSearchPrefix* prefix = ALLOC_GEAOBJECT(PathSearchPrefix);
    DEFER(prefix, ret);

    GqlParser::PathModeContext* pathModeCtx = nullptr;
    GqlParser::NumberOfGroupsContext* numCtx = nullptr;
    prefix->setSearchType(SearchType::kCountedShortestGroups);
    VInt* integerNode = nullptr;
    if (GEAX_IS_NULL(numCtx = ctx->numberOfGroups())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "NumberOfGroups is not set in CountedShortestPathSearch: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(numCtx, integerNode))) {
        LOG(WARNING) << "Failed to visit IntegerLiteral: " << K(ret);
    } else {
        prefix->setNum(integerNode->val());
    }
    if (GEAX_FAIL(ret)) {
    } else if (nullptr != (pathModeCtx = ctx->pathMode())) {
        ModeType modeType;
        if (GEAX_RET_FAIL(visitPathMode(pathModeCtx, modeType))) {
            LOG(WARNING) << "Failed to visit PathMode: " << K(ret);
        } else {
            prefix->setSearchMode(modeType);
        }
    }
    return std::any();
}

GEAXErrorCode GQLAstVisitor::visitPathMode(GqlParser::PathModeContext* ctx, ModeType& modeType) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    if (GEAX_RET_FAIL(byPass(ctx))) {
        LOG(WARNING) << "Failed to visit children of rule PathMode: " << K(ret);
    } else if (GEAX_RET_FAIL(checkTokenResult())) {
        LOG(WARNING) << "Failed to check token result: " << K(ret);
    } else {
        switch (token_->getType()) {
        case GqlParser::WALK:
            modeType = ModeType::kWalk;
            break;
        case GqlParser::TRAIL:
            modeType = ModeType::kTrail;
            break;
        case GqlParser::SIMPLE:
            modeType = ModeType::kSimple;
            break;
        case GqlParser::ACYCLIC:
            modeType = ModeType::kAcyclic;
            break;
        default:
            ret = GEAXErrorCode::GEAX_OOPS;
            LOG(WARNING) << "Invalid token value: " << KV("token", token_->getText()) << DK(ret);
            break;
        }
    }
    return ret;
}

std::any GQLAstVisitor::visitQuantifiedPathPrimary(GqlParser::QuantifiedPathPrimaryContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    AstNode* node = nullptr;
    DEFER(node, ret);

    auto pathPrimaryCtx = ctx->pathPrimary();
    auto patternQuantifier = ctx->graphPatternQuantifier();
    if (GEAX_ANY_NULL(pathPrimaryCtx, patternQuantifier)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "pathPrimary/graphPatternQuantifier is not set: "
                     << KMP(pathPrimaryCtx, patternQuantifier) << DK(ret);
    } else if (GEAX_RET_FAIL(ACCEPT_RULE(pathPrimaryCtx))) {
        LOG(WARNING) << "Failed to visit rule pathPrimary: " << K(ret);
    } else {
        if (childRes_->type() == AstNodeType::kEdge || childRes_->type() == AstNodeType::kNode) {
            node = childRes_;
        } else {
            ret = GEAXErrorCode::GEAX_OOPS;
            LOG(WARNING) << "Invalid AstNode: " << KV("type", childRes_->type()) << DK(ret);
        }
    }
    if (GEAX_OK(ret) && GEAX_RET_FAIL(VISIT_RULE_WITH_FA(patternQuantifier, node))) {
        LOG(WARNING) << "Failed to visit rule graphPatternQuantifier: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitQuestionedPathPrimary(GqlParser::QuestionedPathPrimaryContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    AstNode* node = nullptr;
    DEFER(node, ret);

    auto pathPrimaryCtx = ctx->pathPrimary();
    if (GEAX_IS_NULL(pathPrimaryCtx)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "pathPrimary is not set: " << K(ret);
    } else if (GEAX_RET_FAIL(ACCEPT_RULE(pathPrimaryCtx))) {
        LOG(WARNING) << "Failed to visit rule pathPrimary: " << K(ret);
    } else {
        if (childRes_->type() == AstNodeType::kEdge || childRes_->type() == AstNodeType::kNode) {
            node = childRes_;
        } else {
            ret = GEAXErrorCode::GEAX_OOPS;
            LOG(WARNING) << "Invalid AstNode: " << KV("type", childRes_->type()) << DK(ret);
        }
    }
    std::optional<IntParam> upper{1};
    IntParam lower = 0;
    if (GEAX_OK(ret) &&
        GEAX_RET_FAIL(setEdgePatternHopRange(node, std::move(lower), std::move(upper)))) {
        LOG(WARNING) << "Failed to setEdgePatternHopRange: " << DK(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlGraphPatternAsteriskQuantifier(
    GqlParser::GqlGraphPatternAsteriskQuantifierContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);
    std::optional<IntParam> upper;
    IntParam lower = 0;
    if (GEAX_RET_FAIL(setEdgePatternHopRange(faRes_, std::move(lower), std::move(upper)))) {
        LOG(WARNING) << "Failed to setEdgePatternHopRange: " << DK(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlGraphPatternPlusSignQuantifier(
    GqlParser::GqlGraphPatternPlusSignQuantifierContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);
    std::optional<IntParam> upper;
    IntParam lower = 1;
    if (GEAX_RET_FAIL(setEdgePatternHopRange(faRes_, std::move(lower), std::move(upper)))) {
        LOG(WARNING) << "Failed to setEdgePatternHopRange: " << DK(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitFixedQuantifier(GqlParser::FixedQuantifierContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    auto integerCtx = ctx->unsignedIntegerSpecification();
    std::optional<IntParam> upper;
    IntParam lower;
    if (GEAX_IS_NULL(integerCtx)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "integerLiteral of GqlGraphPatternFixedQuantifier is not set: " << K(ret);
    } else if (GEAX_RET_FAIL(ACCEPT_RULE(integerCtx))) {
        LOG(WARNING) << "Failed to visit IntegerLiteral: " << K(ret);
    } else {
        switch (childRes_->type()) {
        case AstNodeType::kVInt: {
            VInt* integer = castAs<VInt>(childRes_, AstNodeType::kVInt);
            upper = integer->val();
            lower = integer->val();
            break;
        }
        case AstNodeType::kParam: {
            Param* param = castAs<Param>(childRes_, AstNodeType::kParam);
            upper = param;
            lower = param;
            break;
        }
        default: {
            ret = GEAXErrorCode::GEAX_OOPS;
            LOG(WARNING) << "Child result should be VInt or Param: " << K(ret);
            break;
        }
        }
    }
    if (GEAX_OK(ret)) {
        if (GEAX_RET_FAIL(setEdgePatternHopRange(faRes_, std::move(lower), std::move(upper)))) {
            LOG(WARNING) << "Failed to setEdgePatternHopRange: " << DK(ret);
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitGeneralQuantifier(GqlParser::GeneralQuantifierContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    auto lowerCtx = ctx->lowerBound();
    auto upperCtx = ctx->upperBound();
    IntParam lower = 0;
    std::optional<IntParam> upper;
    if (lowerCtx != nullptr) {
        GqlParser::UnsignedIntegerSpecificationContext* ictx = nullptr;
        if (GEAX_IS_NULL(ictx = lowerCtx->unsignedIntegerSpecification())) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "LowerBound is not set unsinged integer: " << K(ret);
        } else if (GEAX_RET_FAIL(ACCEPT_RULE(ictx))) {
            LOG(WARNING) << "Failed to visit IntegerLiteral: " << K(ret);
        } else {
            switch (childRes_->type()) {
            case AstNodeType::kVInt: {
                VInt* integer = castAs<VInt>(childRes_, AstNodeType::kVInt);
                lower = integer->val();
                break;
            }
            case AstNodeType::kParam: {
                Param* param = castAs<Param>(childRes_, AstNodeType::kParam);
                lower = param;
                break;
            }
            default: {
                ret = GEAXErrorCode::GEAX_OOPS;
                LOG(WARNING) << "Child result should be VInt or Param: " << K(ret);
                break;
            }
            }
        }
    }
    if (GEAX_OK(ret) && upperCtx != nullptr) {
        GqlParser::UnsignedIntegerSpecificationContext* ictx = nullptr;
        if (GEAX_IS_NULL(ictx = upperCtx->unsignedIntegerSpecification())) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "UpperBound is not set unsinged integer: " << K(ret);
        } else if (GEAX_RET_FAIL(ACCEPT_RULE(ictx))) {
            LOG(WARNING) << "Failed to visit IntegerLiteral: " << K(ret);
        } else {
            switch (childRes_->type()) {
            case AstNodeType::kVInt: {
                VInt* integer = castAs<VInt>(childRes_, AstNodeType::kVInt);
                upper = integer->val();
                break;
            }
            case AstNodeType::kParam: {
                Param* param = castAs<Param>(childRes_, AstNodeType::kParam);
                upper = param;
                break;
            }
            default: {
                ret = GEAXErrorCode::GEAX_OOPS;
                LOG(WARNING) << "Child result should be VInt or Param: " << K(ret);
                break;
            }
            }
        }
    }
    if (GEAX_OK(ret)) {
        if (GEAX_RET_FAIL(setEdgePatternHopRange(faRes_, std::move(lower), std::move(upper)))) {
            LOG(WARNING) << "Failed to setEdgePatternHopRange: " << DK(ret);
        }
    }

    return std::any();
}

GEAXErrorCode GQLAstVisitor::setEdgePatternHopRange(AstNode* fa, IntParam&& lower,
                                                    std::optional<IntParam>&& upper) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);
    Edge* edgeNode = nullptr;
    if (GEAX_IS_NULL(fa)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Fa is not set: " << K(ret);
    } else if (fa->type() == AstNodeType::kNode) {
        ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
        LOG(WARNING) << "GraphPatternQuantifier is not supported in VertexPattern: " << K(ret);
    } else if (GEAX_UNLIKELY(!checkedCast(fa, edgeNode))) {
        ret = GEAXErrorCode::GEAX_OOPS;
        LOG(ERROR) << "Failed to cast to node kEdge: " << DK(ret);
    } else {
        edgeNode->setHopRange(std::move(lower), std::move(upper));
    }
    return ret;
}

std::any GQLAstVisitor::visitOrderByAndPageStatement(
    GqlParser::OrderByAndPageStatementContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    PrimitiveResultStatement* result = nullptr;
    if (GEAX_IS_NULL(result = castAs<PrimitiveResultStatement>(
                         faRes_, AstNodeType::kPrimitiveResultStatement))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kPrimitiveResultStatement: " << K(ret);
    } else {
        for (auto i = 0u; GEAX_OK(ret) && i < ctx->children.size(); ++i) {
            GqlParser::OffsetClauseContext* offsetCtx = nullptr;
            GqlParser::LimitClauseContext* limitCtx = nullptr;
            GqlParser::OrderByClauseContext* orderbyCtx = nullptr;
            if (checkedCast(ctx->children[i], offsetCtx)) {
                GqlParser::UnsignedIntegerSpecificationContext* ictx = nullptr;
                if (GEAX_IS_NULL(ictx = offsetCtx->unsignedIntegerSpecification())) {
                    ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                    LOG(WARNING) << "offset is not set unsinged integer: " << K(ret);
                } else if (GEAX_RET_FAIL(ACCEPT_RULE(ictx))) {
                    LOG(WARNING) << "Failed to visit expr rule unsignedIntegerSpecification: "
                                 << K(ret);
                } else {
                    switch (childRes_->type()) {
                    case AstNodeType::kVInt: {
                        VInt* integer = castAs<VInt>(childRes_, AstNodeType::kVInt);
                        result->setOffset(integer->val());
                        break;
                    }
                    case AstNodeType::kParam: {
                        Param* param = castAs<Param>(childRes_, AstNodeType::kParam);
                        result->setOffset(param);
                        break;
                    }
                    default: {
                        ret = GEAXErrorCode::GEAX_OOPS;
                        LOG(WARNING) << "Child result should be VInt or Param: " << K(ret);
                        break;
                    }
                    }
                }
            } else if (checkedCast(ctx->children[i], limitCtx)) {
                GqlParser::UnsignedIntegerSpecificationContext* ictx = nullptr;
                if (GEAX_IS_NULL(ictx = limitCtx->unsignedIntegerSpecification())) {
                    ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                    LOG(WARNING) << "limit is not set unsigned integer: " << K(ret);
                } else if (GEAX_RET_FAIL(ACCEPT_RULE(ictx))) {
                    LOG(WARNING) << "Failed to visit expr rule UnsignedIntegerSpecification: "
                                 << K(ret);
                } else {
                    switch (childRes_->type()) {
                    case AstNodeType::kVInt: {
                        VInt* integer = castAs<VInt>(childRes_, AstNodeType::kVInt);
                        result->setLimit(integer->val());
                        break;
                    }
                    case AstNodeType::kParam: {
                        Param* param = castAs<Param>(childRes_, AstNodeType::kParam);
                        result->setLimit(param);
                        break;
                    }
                    default: {
                        ret = GEAXErrorCode::GEAX_OOPS;
                        LOG(WARNING) << "Child result should be VInt or Param: " << K(ret);
                        break;
                    }
                    }
                }
            } else if (checkedCast(ctx->children[i], orderbyCtx)) {
                GqlParser::SortSpecificationListContext* orderListCtx = nullptr;
                if (GEAX_IS_NULL(orderListCtx = orderbyCtx->sortSpecificationList())) {
                    ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                    LOG(WARNING) << "Child of OrderByClause is not set: " << K(ret);
                } else {
                    auto items = orderListCtx->sortSpecification();
                    for (auto j = 0u; GEAX_OK(ret) && j < items.size(); ++j) {
                        OrderByField* field = nullptr;
                        if (GEAX_IS_NULL(items[j])) {
                            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                            LOG(WARNING) << "Child of SortSpecification is not set: " << K(ret);
                        } else if (GEAX_RET_FAIL(
                                       VISIT_RULE(items[j], AstNodeType::kOrderByField, field))) {
                            LOG(WARNING) << "Failed to visit rule sortspecification: " << K(ret);
                        } else {
                            result->appendOrderBy(field);
                        }
                    }
                }
            } else {
                ret = GEAXErrorCode::GEAX_OOPS;
                LOG(WARNING) << "Invalid AstNode: " << KV("type", childRes_->type());
            }
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitSortSpecification(GqlParser::SortSpecificationContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    OrderByField* field = ALLOC_GEAOBJECT(OrderByField);
    DEFER(field, ret);

    bool order = false;
    bool nullOrder = false;
    GqlParser::SortKeyContext* keyCtx = nullptr;
    GqlParser::OrderingSpecificationContext* orderCtx = nullptr;
    GqlParser::NullOrderingContext* nullOrderCtx = nullptr;
    GqlParser::ExpressionContext* exprCtx = nullptr;
    if (GEAX_IS_NULL(keyCtx = ctx->sortKey())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Sortkey is not set in SortSpecification: " << K(ret);
    } else {
        // handle sort expr
        Expr* expr = nullptr;
        if (GEAX_IS_NULL(exprCtx = keyCtx->expression())) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "Expression is not set in SortKey: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, expr))) {
            LOG(WARNING) << "Failed to visit sort key expression " << K(ret);
        } else {
            field->setField(expr);
        }

        if (GEAX_OK(ret)) {
            // order
            if (nullptr != (orderCtx = ctx->orderingSpecification())) {
                if (nullptr != (orderCtx->ASC()) || nullptr != (orderCtx->ASCENDING())) {
                    order = false;
                } else if (nullptr != (orderCtx->DESC()) || nullptr != (orderCtx->DESCENDING())) {
                    order = true;
                }
                field->setOrder(order);
            }
            // null order
            if (nullptr != (nullOrderCtx = ctx->nullOrdering())) {
                if (nullptr != (nullOrderCtx->FIRST())) {
                    nullOrder = false;
                } else if (nullptr != (nullOrderCtx->LAST())) {
                    nullOrder = true;
                }
                field->setNullOrder(nullOrder);
            }
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitInsertPathPattern(GqlParser::InsertPathPatternContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    PathChain* pathChain = ALLOC_GEAOBJECT(PathChain);
    DEFER(pathChain, ret);

    Node* start = nullptr;
    std::vector<GqlParser::InsertNodePatternContext*> nodeList = ctx->insertNodePattern();
    std::vector<GqlParser::InsertEdgePatternContext*> edgeList = ctx->insertEdgePattern();
    if (GEAX_UNLIKELY(nodeList.empty())) {
        ret = GEAXErrorCode::GEAX_OOPS;
        LOG(WARNING) << "Number of InsertNodePattern should not be 0: " << K(ret);
    } else if (GEAX_UNLIKELY_NE(nodeList.size(), edgeList.size() + 1)) {
        ret = GEAXErrorCode::GEAX_OOPS;
        LOG(WARNING) << "Num mismatch: " << KV("nodeList", nodeList.size())
                     << DKV("edgeList", edgeList.size()) << DK(ret);
    } else if (GEAX_IS_NULL(nodeList[0])) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "InsertNodePattern is not set: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(nodeList[0], AstNodeType::kNode, start))) {
        LOG(WARNING) << "Failed to visit InsertNodePattern: " << K(ret);
    } else {
        pathChain->setHead(start);
    }
    for (auto i = 0u; i < edgeList.size() && GEAX_OK(ret); ++i) {
        Node* v = nullptr;
        Edge* e = nullptr;
        if (GEAX_ANY_NULL(edgeList[i], nodeList[i + 1])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "InsertNodePattern/InsertEdgePattern is not set in InsertPathPattern: "
                         << KMP(edgeList[i], nodeList[i + 1]) << DKM(i, ret);
        } else if (GEAX_RET_FAIL(VISIT_RULE(edgeList[i], AstNodeType::kEdge, e))) {
            LOG(WARNING) << "Failed to visit InsertNodePattern: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_RULE(nodeList[i + 1], AstNodeType::kNode, v))) {
            LOG(WARNING) << "Failed to visit InsertEdgePattern: " << K(ret);
        } else {
            pathChain->appendTail(e, v);
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitInsertNodePattern(GqlParser::InsertNodePatternContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Node* v = ALLOC_GEAOBJECT(Node);
    DEFER(v, ret);

    GqlParser::InsertElementPatternFillerContext* fillerCtx = nullptr;
    ElementFiller* filler = nullptr;
    if (nullptr == (fillerCtx = ctx->insertElementPatternFiller())) {
        ElementFiller* emptyFiller = ALLOC_GEAOBJECT(ElementFiller);
        v->setFiller(emptyFiller);
    } else if (GEAX_RET_FAIL(VISIT_RULE(fillerCtx, AstNodeType::kElementFiller, filler))) {
        LOG(WARNING) << "Failed to visit InsertElementPatternFillerContext: " << K(ret);
    } else {
        v->setFiller(filler);
    }

    return std::any();
}

GEAXErrorCode GQLAstVisitor::visitInsertEdgePattern(
    EdgeDirection direction, GqlParser::InsertElementPatternFillerContext* fillerCtx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Edge* e = ALLOC_GEAOBJECT(Edge);
    DEFER(e, ret);

    ElementFiller* filler = nullptr;
    if (nullptr == fillerCtx) {
        ElementFiller* emptyFiller = ALLOC_GEAOBJECT(ElementFiller);
        e->setFiller(emptyFiller);
    } else if (GEAX_RET_FAIL(VISIT_RULE(fillerCtx, AstNodeType::kElementFiller, filler))) {
        LOG(WARNING) << "Failed to visit rule InsertElementPatternFiller: " << K(ret);
    } else {
        e->setDirection(direction);
        e->setFiller(filler);
    }
    return ret;
}

std::any GQLAstVisitor::visitInsertEdgePointingLeft(GqlParser::InsertEdgePointingLeftContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    if (GEAX_RET_FAIL(
            visitInsertEdgePattern(EdgeDirection::kPointLeft, ctx->insertElementPatternFiller()))) {
        LOG(WARNING) << "Failed to visit InsertEdgePointingLeft" << DK(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitInsertEdgePointingRight(
    GqlParser::InsertEdgePointingRightContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    if (GEAX_RET_FAIL(visitInsertEdgePattern(EdgeDirection::kPointRight,
                                             ctx->insertElementPatternFiller()))) {
        LOG(WARNING) << "Failed to visit InsertEdgePointingRight" << DK(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitInsertEdgeUndirected(GqlParser::InsertEdgeUndirectedContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    if (GEAX_RET_FAIL(visitInsertEdgePattern(EdgeDirection::kAnyDirected,
                                             ctx->insertElementPatternFiller()))) {
        LOG(WARNING) << "Failed to visit InsertEdgeUndirected" << DK(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitInsertElementPatternFiller(
    GqlParser::InsertElementPatternFillerContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    ElementFiller* filler = ALLOC_GEAOBJECT(ElementFiller);
    DEFER(filler, ret);

    for (auto i = 0u; i < ctx->children.size() && GEAX_OK(ret); ++i) {
        GqlParser::ElementVariableDeclarationContext* varCtx = nullptr;
        GqlParser::LabelAndPropertySetSpecificationContext* labelPropertyCtx = nullptr;
        if (checkedCast(ctx->children[i], varCtx)) {
            if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(varCtx, filler))) {
                LOG(WARNING) << "Failed to visit ElementVariableDeclaration: " << K(ret);
            }
        } else if (checkedCast(ctx->children[i], labelPropertyCtx)) {
            if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(labelPropertyCtx, filler))) {
                LOG(WARNING) << "Failed to visit LabelAndPropertySetSpecification: " << K(ret);
            }
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitLabelAndPropertySetSpecification(
    GqlParser::LabelAndPropertySetSpecificationContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    ElementFiller* filler = nullptr;
    if (GEAX_IS_NULL(filler = castAs<ElementFiller>(faRes_, AstNodeType::kElementFiller))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kElementFiller: " << K(ret);
    }
    PropStruct* prop = nullptr;
    LabelTree* label = nullptr;
    for (auto i = 0u; i < ctx->children.size() && GEAX_OK(ret); ++i) {
        GqlParser::LabelSetSpecificationContext* labelCtx = nullptr;
        GqlParser::ElementPropertySpecificationContext* propertyCtx = nullptr;
        if (checkedCast(ctx->children[i], labelCtx)) {
            if (GEAX_RET_FAIL(ACCEPT_RULE(labelCtx))) {
                LOG(WARNING) << "Failed visit LabelSetSpecification: " << K(ret);
            } else if (GEAX_UNLIKELY(!checkedCast(childRes_, label))) {
                ret = GEAXErrorCode::GEAX_OOPS;
                LOG(WARNING) << "Child result should be a LabelTree: " << K(ret);
            } else {
                filler->setLabel(label);
            }
        } else if (checkedCast(ctx->children[i], propertyCtx)) {
            if (GEAX_RET_FAIL(VISIT_RULE(propertyCtx, AstNodeType::kPropStruct, prop))) {
                LOG(WARNING) << "Failed to visit ElementPropertySpecification: " << K(ret);
            } else {
                filler->appendPredicate(prop);
            }
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitLabelSetSpecification(GqlParser::LabelSetSpecificationContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    LabelTree* labelTree = nullptr;
    DEFER(labelTree, ret);

    std::vector<GqlParser::LabelNameContext*> labelCtxs = ctx->labelName();
    for (auto i = 0u; i < labelCtxs.size() && GEAX_OK(ret); ++i) {
        LabelTree* label = nullptr;
        if (GEAX_IS_NULL(labelCtxs[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "LabelName is not set: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_EXPR(labelCtxs[i], label))) {
            LOG(WARNING) << "Failed to visit rule LabelName: " << K(ret);
        } else {
            if (labelTree != nullptr) {
                LabelAnd* labelAnd = ALLOC_GEAOBJECT(LabelAnd);
                labelAnd->setLeft(labelTree);
                labelAnd->setRight(label);
                labelTree = labelAnd;
            } else {
                labelTree = label;
            }
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitNodePattern(GqlParser::NodePatternContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Node* v = ALLOC_GEAOBJECT(Node);
    DEFER(v, ret);

    GqlParser::ElementPatternFillerContext* fillerCtx = nullptr;
    ElementFiller* filler = nullptr;
    if (GEAX_IS_NULL(fillerCtx = ctx->elementPatternFiller())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "elementPatternFiller is not set in rule Node: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(fillerCtx, AstNodeType::kElementFiller, filler))) {
        LOG(WARNING) << "Failed to visit rule ElementPatternFillter: " << K(ret);
    } else {
        v->setFiller(filler);
    }

    return std::any();
}

std::any GQLAstVisitor::visitElementPatternFiller(GqlParser::ElementPatternFillerContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    ElementFiller* filler = ALLOC_GEAOBJECT(ElementFiller);
    DEFER(filler, ret);

    for (auto i = 0u; i < ctx->children.size() && GEAX_OK(ret); ++i) {
        GqlParser::ElementVariableDeclarationContext* varCtx = nullptr;
        GqlParser::IsLabelExpressionContext* labelExprCtx = nullptr;
        GqlParser::ElementPatternPredicateContext* predicateCtx = nullptr;
        if (checkedCast(ctx->children[i], varCtx)) {
            if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(varCtx, filler))) {
                LOG(WARNING) << "Failed to visit ElementVariableDeclaration: " << K(ret);
            }
        } else if (checkedCast(ctx->children[i], labelExprCtx)) {
            LabelTree* label = nullptr;
            if (GEAX_RET_FAIL(ACCEPT_RULE(labelExprCtx))) {
                LOG(WARNING) << "Failed visit IsLabelExpression: " << K(ret);
            } else if (GEAX_UNLIKELY(!checkedCast(childRes_, label))) {
                ret = GEAXErrorCode::GEAX_OOPS;
                LOG(WARNING) << "Child result should be a LabelTree: " << K(ret);
            } else {
                filler->setLabel(label);
            }
        } else if (checkedCast(ctx->children[i], predicateCtx)) {
            if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(predicateCtx, filler))) {
                LOG(WARNING) << "Failed to visit ElementPatternPredicate: " << K(ret);
            }
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitElementVariableDeclaration(
    GqlParser::ElementVariableDeclarationContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    ElementFiller* filler = nullptr;
    if (GEAX_IS_NULL(filler = castAs<ElementFiller>(faRes_, AstNodeType::kElementFiller))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kElementFiller: " << K(ret);
    } else {
        GqlParser::ElementVariableContext* varCtx = nullptr;
        if (nullptr != ctx->TEMP()) {
            ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
            LOG(WARNING) << "Temp is not supperted yet: " << K(ret);
        } else if (GEAX_IS_NULL(varCtx = ctx->elementVariable())) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "ElementVariable is not set in ElementVariableDeclaration: " << K(ret);
        } else {
            filler->setV(varCtx->getText());
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitElementPatternPredicate(
    GqlParser::ElementPatternPredicateContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    ElementFiller* filler = nullptr;
    if (GEAX_IS_NULL(filler = castAs<ElementFiller>(faRes_, AstNodeType::kElementFiller))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kElementFiller: " << K(ret);
    } else {
        GqlParser::WhereClauseContext* whereCtx = nullptr;
        GqlParser::ElementPropertySpecificationContext* propertyCtx = nullptr;
        GqlParser::PerNodeLimitClauseContext* perNodeCtx = nullptr;
        GqlParser::PerNodeLimitWherePredicateContext* perNodeWhereCtx = nullptr;
        GqlParser::PerNodeLimitPropertyPredicateContext* perNodePropertyCtx = nullptr;
        GqlParser::PerShardLimitClauseContext* perShardCtx = nullptr;
        GqlParser::PerShardLimitWherePredicateContext* perShardWhereCtx = nullptr;
        GqlParser::PerShardLimitPropertyPredicateContext* perShardPropertyCtx = nullptr;
        if (GEAX_UNLIKELY(ctx->children.size() != 1)) {
            ret = GEAXErrorCode::GEAX_COMMON_SYNTAX_ERROR;
            LOG(WARNING) << "Invalid children num for rule ElementPatternPredicate: "
                         << DKV("size", ctx->children.size()) << DK(ret);
        } else if (checkedCast(ctx->children[0], whereCtx)) {
            WhereClause* where = nullptr;
            if (GEAX_RET_FAIL(VISIT_RULE(whereCtx, AstNodeType::kWhere, where))) {
                LOG(WARNING) << "Failed to visit WhereClause: " << K(ret);
            } else {
                filler->appendPredicate(where);
            }
        } else if (checkedCast(ctx->children[0], propertyCtx)) {
            PropStruct* prop = nullptr;
            if (GEAX_RET_FAIL(VISIT_RULE(propertyCtx, AstNodeType::kPropStruct, prop))) {
                LOG(WARNING) << "Failed to visit ElementPropertySpecification: " << K(ret);
            } else {
                filler->appendPredicate(prop);
            }
        } else if (checkedCast(ctx->children[0], perNodeCtx)) {
            TableFunctionClause* tableFunc = nullptr;
            if (GEAX_RET_FAIL(VISIT_RULE(perNodeCtx, AstNodeType::kTableFunction, tableFunc))) {
                LOG(WARNING) << "Failed to visit PerNodeLimitClause: " << K(ret);
            } else {
                filler->appendPredicate(tableFunc);
            }
        } else if (checkedCast(ctx->children[0], perNodeWhereCtx)) {
            if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(perNodeWhereCtx, filler))) {
                LOG(WARNING) << "Failed to visit PerNodeLimitWherePredicate: " << K(ret);
            }
        } else if (checkedCast(ctx->children[0], perNodePropertyCtx)) {
            if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(perNodePropertyCtx, filler))) {
                LOG(WARNING) << "Failed to visit PerNodeLimitPropertyPredicate: " << K(ret);
            }
        } else if (checkedCast(ctx->children[0], perShardCtx)) {
            TableFunctionClause* tableFunc = nullptr;
            if (GEAX_RET_FAIL(VISIT_RULE(perShardCtx, AstNodeType::kTableFunction, tableFunc))) {
                LOG(WARNING) << "Failed to visit PerShardLimitClause: " << K(ret);
            } else {
                filler->appendPredicate(tableFunc);
            }
        } else if (checkedCast(ctx->children[0], perShardWhereCtx)) {
            if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(perShardWhereCtx, filler))) {
                LOG(WARNING) << "Failed to visit PerShardLimitWherePredicate: " << K(ret);
            }
        } else if (checkedCast(ctx->children[0], perShardPropertyCtx)) {
            if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(perShardPropertyCtx, filler))) {
                LOG(WARNING) << "Failed to visit PerShardLimitPropertyPredicate: " << K(ret);
            }
        } else {
            ret = GEAXErrorCode::GEAX_OOPS;
            LOG(WARNING) << "Invalid AstNode: " << KV("type", childRes_->type());
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitPropertyKeyValuePairList(
    GqlParser::PropertyKeyValuePairListContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    PropStruct* prop = ALLOC_GEAOBJECT(PropStruct);
    DEFER(prop, ret);

    std::vector<GqlParser::PropertyKeyValuePairContext*> kvCtxs = ctx->propertyKeyValuePair();
    for (auto i = 0u; GEAX_OK(ret) && i < kvCtxs.size(); ++i) {
        if (GEAX_IS_NULL(kvCtxs[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "Child of PropertyKeyValuePairList is not set: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(kvCtxs[i], prop))) {
            LOG(WARNING) << "Failed to visit rule PropertyKeyValuePair: " << K(ret);
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitPropertyKeyValuePair(GqlParser::PropertyKeyValuePairContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    PropStruct* prop = nullptr;
    Expr* value = nullptr;
    GqlParser::PropertyNameContext* nameCtx = nullptr;
    GqlParser::ExpressionContext* exprCtx = nullptr;
    if (GEAX_IS_NULL(prop = castAs<PropStruct>(faRes_, AstNodeType::kPropStruct))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kPropStruct: " << K(ret);
    } else if (GEAX_IS_NULL(nameCtx = ctx->propertyName())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Child of rule PropertyKeyValuePair is not set: " << K(ret);
    } else if (GEAX_IS_NULL(exprCtx = ctx->expression())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Child of rule PropertyKeyValuePair is not set: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, value))) {
        LOG(WARNING) << "Failed to visit rule Expression: " << K(ret);
    } else {
        prop->appendProperty(trimAccentGrave(nameCtx->getText()), value);
    }

    return std::any();
}

std::any GQLAstVisitor::visitWhereClause(GqlParser::WhereClauseContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    WhereClause* where = ALLOC_GEAOBJECT(WhereClause);
    DEFER(where, ret);

    Expr* expr = nullptr;
    GqlParser::ExpressionContext* exprCtx = nullptr;
    if (GEAX_IS_NULL(exprCtx = ctx->expression())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Child of WhereClause is not set: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, expr))) {
        LOG(WARNING) << "Failed to visit rule Expression: " << K(ret);
    } else {
        where->setPredicate(expr);
    }

    return std::any();
}

std::any GQLAstVisitor::visitPerNodeLimitClause(GqlParser::PerNodeLimitClauseContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    TableFunctionClause* tableFunc = ALLOC_GEAOBJECT(TableFunctionClause);
    DEFER(tableFunc, ret);

    Function* func = nullptr;
    if (GEAX_RET_FAIL(
            visitPerLimitFunction(ctx->unsignedIntegerSpecification(), ctx->PER_NODE_LIMIT()))) {
        LOG(WARNING) << "Failed to visit PerLimitFunction: " << K(ret);
    } else if (GEAX_UNLIKELY(!checkedCast(childRes_, func))) {
        ret = GEAXErrorCode::GEAX_OOPS;
        LOG(WARNING) << "Child result should be a Function: " << K(ret);
    } else {
        tableFunc->setFunction(func);
    }
    return std::any();
}

std::any GQLAstVisitor::visitPerShardLimitClause(GqlParser::PerShardLimitClauseContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    TableFunctionClause* tableFunc = ALLOC_GEAOBJECT(TableFunctionClause);
    DEFER(tableFunc, ret);

    Function* func = nullptr;
    if (GEAX_RET_FAIL(
            visitPerLimitFunction(ctx->unsignedIntegerSpecification(), ctx->PER_SHARD_LIMIT()))) {
        LOG(WARNING) << "Failed to visit PerLimitFunction: " << K(ret);
    } else if (GEAX_UNLIKELY(!checkedCast(childRes_, func))) {
        ret = GEAXErrorCode::GEAX_OOPS;
        LOG(WARNING) << "Child result should be a Function: " << K(ret);
    } else {
        tableFunc->setFunction(func);
    }
    return std::any();
}

GEAXErrorCode GQLAstVisitor::visitPerLimitFunction(
    GqlParser::UnsignedIntegerSpecificationContext* ictx, antlr4::tree::TerminalNode* perName) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Function* func = ALLOC_GEAOBJECT(Function);
    DEFER(func, ret);

    Expr* expr = nullptr;
    if (GEAX_ANY_NULL(perName, ictx)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "UnsignedIntegerSpecification or perName is not set: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(ictx, expr))) {
        LOG(WARNING) << "Failed to visit unsignedIntegerSpecification: " << K(ret);
    } else {
        func->setName(perName->getText());
        func->appendArg(expr);
    }
    return ret;
}

std::any GQLAstVisitor::visitPerNodeLimitLeftWherePredicate(
    GqlParser::PerNodeLimitLeftWherePredicateContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    ElementFiller* filler = nullptr;
    if (GEAX_IS_NULL(filler = castAs<ElementFiller>(faRes_, AstNodeType::kElementFiller))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kElementFiller: " << K(ret);
    } else if (GEAX_RET_FAIL(
                   visitPerLimitLeftWherePredicate(ctx->lhs, ctx->whereClause(), filler))) {
        LOG(WARNING) << "Failed to visit PerLimitLeftWherePredicate: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitPerShardLimitLeftWherePredicate(
    GqlParser::PerShardLimitLeftWherePredicateContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    ElementFiller* filler = nullptr;
    if (GEAX_IS_NULL(filler = castAs<ElementFiller>(faRes_, AstNodeType::kElementFiller))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kElementFiller: " << K(ret);
    } else if (GEAX_RET_FAIL(
                   visitPerLimitLeftWherePredicate(ctx->lhs, ctx->whereClause(), filler))) {
        LOG(WARNING) << "Failed to visit PerLimitLeftWherePredicate: " << K(ret);
    }
    return std::any();
}

GEAXErrorCode GQLAstVisitor::visitPerLimitLeftWherePredicate(
    antlr4::ParserRuleContext* lhs, GqlParser::WhereClauseContext* whereCtx,
    ElementFiller* filler) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;

    TableFunctionClause* tableFunc = nullptr;
    WhereClause* where = nullptr;
    if (GEAX_ANY_NULL(lhs, whereCtx, filler)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "PerNodeLimitClause/WhereClause/ElementFiller is not set : " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(lhs, AstNodeType::kTableFunction, tableFunc))) {
        LOG(WARNING) << "Failed to visit lhs perNodeLimitClause: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(whereCtx, AstNodeType::kWhere, where))) {
        LOG(WARNING) << "Failed to visit whereClause: " << K(ret);
    } else {
        filler->appendPredicate(tableFunc);
        filler->appendPredicate(where);
    }
    return ret;
}

std::any GQLAstVisitor::visitPerNodeLimitRightWherePredicate(
    GqlParser::PerNodeLimitRightWherePredicateContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    ElementFiller* filler = nullptr;
    if (GEAX_IS_NULL(filler = castAs<ElementFiller>(faRes_, AstNodeType::kElementFiller))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kElementFiller: " << K(ret);
    } else if (GEAX_RET_FAIL(
                   visitPerLimitRightWherePredicate(ctx->rhs, ctx->whereClause(), filler))) {
        LOG(WARNING) << "Failed to visit PerLimitLeftWherePredicate: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitPerShardLimitRightWherePredicate(
    GqlParser::PerShardLimitRightWherePredicateContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    ElementFiller* filler = nullptr;
    if (GEAX_IS_NULL(filler = castAs<ElementFiller>(faRes_, AstNodeType::kElementFiller))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kElementFiller: " << K(ret);
    } else if (GEAX_RET_FAIL(
                   visitPerLimitRightWherePredicate(ctx->rhs, ctx->whereClause(), filler))) {
        LOG(WARNING) << "Failed to visit PerLimitRightWherePredicate: " << K(ret);
    }
    return std::any();
}

GEAXErrorCode GQLAstVisitor::visitPerLimitRightWherePredicate(
    antlr4::ParserRuleContext* rhs, GqlParser::WhereClauseContext* whereCtx,
    ElementFiller* filler) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;

    WhereClause* where = nullptr;
    TableFunctionClause* tableFunc = nullptr;
    if (GEAX_ANY_NULL(rhs, whereCtx, filler)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "PerNodeLimitClause/WhereClause/ElementFiller is not set : " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(rhs, AstNodeType::kTableFunction, tableFunc))) {
        LOG(WARNING) << "Failed to visit rhs perNodeLimitClause: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(whereCtx, AstNodeType::kWhere, where))) {
        LOG(WARNING) << "Failed to visit whereClause: " << K(ret);
    } else {
        filler->appendPredicate(where);
        filler->appendPredicate(tableFunc);
    }
    return ret;
}

std::any GQLAstVisitor::visitPerNodeLimitBothWherePredicate(
    GqlParser::PerNodeLimitBothWherePredicateContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    ElementFiller* filler = nullptr;
    if (GEAX_IS_NULL(filler = castAs<ElementFiller>(faRes_, AstNodeType::kElementFiller))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kElementFiller: " << K(ret);
    } else if (GEAX_RET_FAIL(visitPerLimitBothWherePredicate(ctx->lhs, ctx->rhs, ctx->whereClause(),
                                                             filler))) {
        LOG(WARNING) << "Failed to visit PerLimitBothWherePredicate: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitPerShardLimitBothWherePredicate(
    GqlParser::PerShardLimitBothWherePredicateContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    ElementFiller* filler = nullptr;
    if (GEAX_IS_NULL(filler = castAs<ElementFiller>(faRes_, AstNodeType::kElementFiller))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kElementFiller: " << K(ret);
    } else if (GEAX_RET_FAIL(visitPerLimitBothWherePredicate(ctx->lhs, ctx->rhs, ctx->whereClause(),
                                                             filler))) {
        LOG(WARNING) << "Failed to visit PerLimitBothWherePredicate: " << K(ret);
    }
    return std::any();
}

GEAXErrorCode GQLAstVisitor::visitPerLimitBothWherePredicate(
    antlr4::ParserRuleContext* lhs, antlr4::ParserRuleContext* rhs,
    GqlParser::WhereClauseContext* whereCtx, ElementFiller* filler) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;

    WhereClause* where = nullptr;
    TableFunctionClause* tableFuncBefore = nullptr;
    TableFunctionClause* tableFuncAfter = nullptr;
    if (GEAX_ANY_NULL(lhs, rhs, whereCtx, filler)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "PerNodeLimitClause/WhereClause/ElementFiller is not set : " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(lhs, AstNodeType::kTableFunction, tableFuncBefore))) {
        LOG(WARNING) << "Failed to visit lhs perNodeLimitClause: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(rhs, AstNodeType::kTableFunction, tableFuncAfter))) {
        LOG(WARNING) << "Failed to visit rhs perNodeLimitClause: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(whereCtx, AstNodeType::kWhere, where))) {
        LOG(WARNING) << "Failed to visit whereClause: " << K(ret);
    } else {
        filler->appendPredicate(tableFuncBefore);
        filler->appendPredicate(where);
        filler->appendPredicate(tableFuncAfter);
    }
    return ret;
}

std::any GQLAstVisitor::visitPerNodeLimitLeftPropertyPredicate(
    GqlParser::PerNodeLimitLeftPropertyPredicateContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    ElementFiller* filler = nullptr;
    if (GEAX_IS_NULL(filler = castAs<ElementFiller>(faRes_, AstNodeType::kElementFiller))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kElementFiller: " << K(ret);
    } else if (GEAX_RET_FAIL(visitPerLimitLeftPropertyPredicate(
                   ctx->lhs, ctx->elementPropertySpecification(), filler))) {
        LOG(WARNING) << "Failed to visit PerLimitBothWherePredicate: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitPerShardLimitLeftPropertyPredicate(
    GqlParser::PerShardLimitLeftPropertyPredicateContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    ElementFiller* filler = nullptr;
    if (GEAX_IS_NULL(filler = castAs<ElementFiller>(faRes_, AstNodeType::kElementFiller))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kElementFiller: " << K(ret);
    } else if (GEAX_RET_FAIL(visitPerLimitLeftPropertyPredicate(
                   ctx->lhs, ctx->elementPropertySpecification(), filler))) {
        LOG(WARNING) << "Failed to visit PerLimitBothWherePredicate: " << K(ret);
    }
    return std::any();
}

GEAXErrorCode GQLAstVisitor::visitPerLimitLeftPropertyPredicate(
    antlr4::ParserRuleContext* lhs, GqlParser::ElementPropertySpecificationContext* propertyCtx,
    ElementFiller* filler) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;

    TableFunctionClause* tableFunc = nullptr;
    PropStruct* prop = nullptr;
    if (GEAX_ANY_NULL(lhs, propertyCtx, filler)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "PerNodeLimitClause/PropertySpecification/ElementFiller is not set : "
                     << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(lhs, AstNodeType::kTableFunction, tableFunc))) {
        LOG(WARNING) << "Failed to visit lhs perNodeLimitClause: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(propertyCtx, AstNodeType::kPropStruct, prop))) {
        LOG(WARNING) << "Failed to visit ElementPropertySpecification: " << K(ret);
    } else {
        filler->appendPredicate(tableFunc);
        filler->appendPredicate(prop);
    }
    return ret;
}

std::any GQLAstVisitor::visitPerNodeLimitRightPropertyPredicate(
    GqlParser::PerNodeLimitRightPropertyPredicateContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    ElementFiller* filler = nullptr;
    if (GEAX_IS_NULL(filler = castAs<ElementFiller>(faRes_, AstNodeType::kElementFiller))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kElementFiller: " << K(ret);
    } else if (GEAX_RET_FAIL(visitPerLimitRightPropertyPredicate(
                   ctx->rhs, ctx->elementPropertySpecification(), filler))) {
        LOG(WARNING) << "Failed to visit PerLimitBothWherePredicate: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitPerShardLimitRightPropertyPredicate(
    GqlParser::PerShardLimitRightPropertyPredicateContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    ElementFiller* filler = nullptr;
    if (GEAX_IS_NULL(filler = castAs<ElementFiller>(faRes_, AstNodeType::kElementFiller))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kElementFiller: " << K(ret);
    } else if (GEAX_RET_FAIL(visitPerLimitRightPropertyPredicate(
                   ctx->rhs, ctx->elementPropertySpecification(), filler))) {
        LOG(WARNING) << "Failed to visit PerLimitBothWherePredicate: " << K(ret);
    }
    return std::any();
}

GEAXErrorCode GQLAstVisitor::visitPerLimitRightPropertyPredicate(
    antlr4::ParserRuleContext* rhs, GqlParser::ElementPropertySpecificationContext* propertyCtx,
    ElementFiller* filler) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;

    TableFunctionClause* tableFunc = nullptr;
    PropStruct* prop = nullptr;
    if (GEAX_ANY_NULL(rhs, propertyCtx, filler)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "PerNodeLimitClause/PropertySpecification/ElementFiller is not set : "
                     << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(rhs, AstNodeType::kTableFunction, tableFunc))) {
        LOG(WARNING) << "Failed to visit rhs perNodeLimitClause: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(propertyCtx, AstNodeType::kPropStruct, prop))) {
        LOG(WARNING) << "Failed to visit ElementPropertySpecification: " << K(ret);
    } else {
        filler->appendPredicate(prop);
        filler->appendPredicate(tableFunc);
    }
    return ret;
}

std::any GQLAstVisitor::visitPerNodeLimitBothPropertyPredicate(
    GqlParser::PerNodeLimitBothPropertyPredicateContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    ElementFiller* filler = nullptr;
    if (GEAX_IS_NULL(filler = castAs<ElementFiller>(faRes_, AstNodeType::kElementFiller))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kElementFiller: " << K(ret);
    } else if (GEAX_RET_FAIL(visitPerLimitBothPropertyPredicate(
                   ctx->lhs, ctx->rhs, ctx->elementPropertySpecification(), filler))) {
        LOG(WARNING) << "Failed to visit PerLimitBothWherePredicate: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitPerShardLimitBothPropertyPredicate(
    GqlParser::PerShardLimitBothPropertyPredicateContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    ElementFiller* filler = nullptr;
    if (GEAX_IS_NULL(filler = castAs<ElementFiller>(faRes_, AstNodeType::kElementFiller))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kElementFiller: " << K(ret);
    } else if (GEAX_RET_FAIL(visitPerLimitBothPropertyPredicate(
                   ctx->lhs, ctx->rhs, ctx->elementPropertySpecification(), filler))) {
        LOG(WARNING) << "Failed to visit PerLimitBothWherePredicate: " << K(ret);
    }
    return std::any();
}

GEAXErrorCode GQLAstVisitor::visitPerLimitBothPropertyPredicate(
    antlr4::ParserRuleContext* lhs, antlr4::ParserRuleContext* rhs,
    GqlParser::ElementPropertySpecificationContext* propertyCtx, ElementFiller* filler) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;

    TableFunctionClause* tableFuncBefore = nullptr;
    TableFunctionClause* tableFuncAfter = nullptr;
    PropStruct* prop = nullptr;
    if (GEAX_ANY_NULL(lhs, rhs, propertyCtx, filler)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "PerNodeLimitClause/PropertySpecification/ElementFiller is not set : "
                     << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(lhs, AstNodeType::kTableFunction, tableFuncBefore))) {
        LOG(WARNING) << "Failed to visit lhs perNodeLimitClause: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(rhs, AstNodeType::kTableFunction, tableFuncAfter))) {
        LOG(WARNING) << "Failed to visit rhs perNodeLimitClause: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(propertyCtx, AstNodeType::kPropStruct, prop))) {
        LOG(WARNING) << "Failed to visit ElementPropertySpecification: " << K(ret);
    } else {
        filler->appendPredicate(tableFuncBefore);
        filler->appendPredicate(prop);
        filler->appendPredicate(tableFuncAfter);
    }
    return ret;
}

std::any GQLAstVisitor::visitGroupingElement(GqlParser::GroupingElementContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    PrimitiveResultStatement* result = nullptr;
    Expr* expr = nullptr;
    GqlParser::ExpressionContext* exprCtx = nullptr;
    if (GEAX_IS_NULL(result = castAs<PrimitiveResultStatement>(
                         faRes_, AstNodeType::kPrimitiveResultStatement))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Faild to cast to node kPrimitiveResultStatement: " << K(ret);
    } else if (GEAX_IS_NULL(exprCtx = ctx->expression())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Child of grouping element is not set: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, expr))) {
        LOG(WARNING) << "Failed to visit rule group by expression: " << K(ret);
    } else {
        result->appendGroupKey(expr);
    }
    return std::any();
}

GEAXErrorCode GQLAstVisitor::visitFullEdgePattern(
    EdgeDirection direction, GqlParser::ElementPatternFillerContext* fillerCtx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Edge* e = ALLOC_GEAOBJECT(Edge);
    DEFER(e, ret);

    ElementFiller* filler = nullptr;
    if (GEAX_IS_NULL(fillerCtx)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "ElementPatternFiller is not set in rule FullEdgePattern: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(fillerCtx, AstNodeType::kElementFiller, filler))) {
        LOG(WARNING) << "Failed to visit rule ElementPatternFiller: " << K(ret);
    } else {
        e->setDirection(direction);
        e->setFiller(filler);
    }
    return ret;
}

std::any GQLAstVisitor::visitFullEdgePointingLeft(GqlParser::FullEdgePointingLeftContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    if (GEAX_RET_FAIL(
            visitFullEdgePattern(EdgeDirection::kPointLeft, ctx->elementPatternFiller()))) {
        LOG(WARNING) << "Failed to visit FullEdgePointingLeft" << DK(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitFullEdgeUndirected(GqlParser::FullEdgeUndirectedContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    if (GEAX_RET_FAIL(
            visitFullEdgePattern(EdgeDirection::kUndirected, ctx->elementPatternFiller()))) {
        LOG(WARNING) << "Failed to visit FullEdgeUndirected" << DK(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitFullEdgePointingRight(GqlParser::FullEdgePointingRightContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    if (GEAX_RET_FAIL(
            visitFullEdgePattern(EdgeDirection::kPointRight, ctx->elementPatternFiller()))) {
        LOG(WARNING) << "Failed to visit FullEdgePointingRight" << DK(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitFullEdgeLeftOrUndirected(
    GqlParser::FullEdgeLeftOrUndirectedContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    if (GEAX_RET_FAIL(
            visitFullEdgePattern(EdgeDirection::kLeftOrUndirected, ctx->elementPatternFiller()))) {
        LOG(WARNING) << "Failed to visit FullEdgeLeftOrUndirected" << DK(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitFullEdgeUndirectedOrRight(
    GqlParser::FullEdgeUndirectedOrRightContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    if (GEAX_RET_FAIL(
            visitFullEdgePattern(EdgeDirection::kRightOrUndirected, ctx->elementPatternFiller()))) {
        LOG(WARNING) << "Failed to visit FullEdgeUndirectedOrRight" << DK(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitFullEdgeLeftOrRight(GqlParser::FullEdgeLeftOrRightContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    if (GEAX_RET_FAIL(
            visitFullEdgePattern(EdgeDirection::kLeftOrRight, ctx->elementPatternFiller()))) {
        LOG(WARNING) << "Failed to visit FullEdgeLeftOrRight" << DK(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitFullEdgeAnyDirection(GqlParser::FullEdgeAnyDirectionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    if (GEAX_RET_FAIL(
            visitFullEdgePattern(EdgeDirection::kAnyDirected, ctx->elementPatternFiller()))) {
        LOG(WARNING) << "Failed to visit FullEdgeAnyDirection" << DK(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitAbbreviatedEdgePointingLeft(
    GqlParser::AbbreviatedEdgePointingLeftContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Edge* e = ALLOC_GEAOBJECT(Edge);
    DEFER(e, ret);

    e->setDirection(EdgeDirection::kPointLeft);
    e->setFiller(ALLOC_GEAOBJECT(ElementFiller));
    return std::any();
}

std::any GQLAstVisitor::visitAbbreviatedEdgeUndirected(
    GqlParser::AbbreviatedEdgeUndirectedContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Edge* e = ALLOC_GEAOBJECT(Edge);
    DEFER(e, ret);

    e->setDirection(EdgeDirection::kUndirected);
    e->setFiller(ALLOC_GEAOBJECT(ElementFiller));
    return std::any();
}

std::any GQLAstVisitor::visitAbbreviatedEdgePointingRight(
    GqlParser::AbbreviatedEdgePointingRightContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Edge* e = ALLOC_GEAOBJECT(Edge);
    DEFER(e, ret);

    e->setDirection(EdgeDirection::kPointRight);
    e->setFiller(ALLOC_GEAOBJECT(ElementFiller));
    return std::any();
}

std::any GQLAstVisitor::visitAbbreviatedEdgeLeftOrUndirected(
    GqlParser::AbbreviatedEdgeLeftOrUndirectedContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Edge* e = ALLOC_GEAOBJECT(Edge);
    DEFER(e, ret);

    e->setDirection(EdgeDirection::kLeftOrUndirected);
    e->setFiller(ALLOC_GEAOBJECT(ElementFiller));
    return std::any();
}

std::any GQLAstVisitor::visitAbbreviatedEdgeUndirectedOrRight(
    GqlParser::AbbreviatedEdgeUndirectedOrRightContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Edge* e = ALLOC_GEAOBJECT(Edge);
    DEFER(e, ret);

    e->setDirection(EdgeDirection::kRightOrUndirected);
    e->setFiller(ALLOC_GEAOBJECT(ElementFiller));
    return std::any();
}

std::any GQLAstVisitor::visitAbbreviatedEdgeLeftOrRight(
    GqlParser::AbbreviatedEdgeLeftOrRightContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Edge* e = ALLOC_GEAOBJECT(Edge);
    DEFER(e, ret);

    e->setDirection(EdgeDirection::kLeftOrRight);
    e->setFiller(ALLOC_GEAOBJECT(ElementFiller));
    return std::any();
}

std::any GQLAstVisitor::visitAbbreviatedEdgeAnyDirection(
    GqlParser::AbbreviatedEdgeAnyDirectionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Edge* e = ALLOC_GEAOBJECT(Edge);
    DEFER(e, ret);

    e->setDirection(EdgeDirection::kAnyDirected);
    e->setFiller(ALLOC_GEAOBJECT(ElementFiller));
    return std::any();
}

std::any GQLAstVisitor::visitReturnStatementBody(GqlParser::ReturnStatementBodyContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    GqlParser::HintItemlistContext* hintItems = nullptr;
    GqlParser::SetQuantifierContext* quantifier = nullptr;
    GqlParser::ReturnItemListContext* retItems = nullptr;
    GqlParser::GroupByClauseContext* groupBy = nullptr;
    antlr4::tree::TerminalNode* all = nullptr;

    PrimitiveResultStatement* result = nullptr;
    if (GEAX_IS_NULL(result = castAs<PrimitiveResultStatement>(
                         faRes_, AstNodeType::kPrimitiveResultStatement))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kPrimitiveResultStatement: " << K(ret);
    }
    for (auto i = 0u; i < ctx->children.size() && GEAX_OK(ret); ++i) {
        // since dynamic_casting a nullptr will procude a nullptr,
        // we do not check the nullness here
        if (hintItems == nullptr && checkedCast(ctx->children[i], hintItems)) {
            if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(hintItems, result))) {
                LOG(WARNING) << "Failed to visit rule HintItemlist: " << K(ret);
            }
        } else if (quantifier == nullptr && checkedCast(ctx->children[i], quantifier)) {
            if (size_t token = 0; GEAX_RET_FAIL(VISIT_TOKEN(quantifier, token))) {
                LOG(WARNING) << "Failed to visit token SetQuantifier: " << K(ret);
            } else {
                switch (token) {
                case GqlParser::DISTINCT:
                    result->setDistinct(true);
                    break;
                case GqlParser::ALL:
                    result->setDistinct(false);
                    break;
                default:
                    ret = GEAXErrorCode::GEAX_OOPS;
                    LOG(WARNING) << "Invalid SetQuantifier token type: "
                                 << KV("token", token_->getText()) << DK(ret);
                    break;
                }
            }
        } else if (retItems == nullptr && checkedCast(ctx->children[i], retItems)) {
            if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(retItems, result))) {
                LOG(WARNING) << "Failed to visit rule ReturnItemList: " << K(ret);
            }
        } else if (groupBy == nullptr && checkedCast(ctx->children[i], groupBy)) {
            GqlParser::GroupingElementListContext* groupbyItemsCtx = nullptr;
            if (GEAX_IS_NULL(groupbyItemsCtx = groupBy->groupingElementList())) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "Child of GroupByClause is not set: " << K(ret);
            } else {
                auto items = groupbyItemsCtx->groupingElement();
                for (auto j = 0u; GEAX_OK(ret) && j < items.size(); ++j) {
                    if (GEAX_IS_NULL(items[j])) {
                        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                        LOG(WARNING) << "GroupingElement is not set: " << K(ret);
                    } else if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(items[j], result))) {
                        LOG(WARNING) << "Failed to visit rule GroupByField: " << K(ret);
                    }
                }
            }
        } else if (all == nullptr && checkedCast(ctx->children[i], all)) {
            // TODO(boyao.zby) return * is not implemnent
            ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
            LOG(WARNING) << "RETURN * or NO BINDINGS is not supported yet: " << K(ret);
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitHintItemlist(GqlParser::HintItemlistContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    PrimitiveResultStatement* result = nullptr;
    if (GEAX_IS_NULL(result = castAs<PrimitiveResultStatement>(
                         faRes_, AstNodeType::kPrimitiveResultStatement))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kPrimitiveResultStatement: " << K(ret);
    }
    std::vector<GqlParser::HintItemContext*> hintListCtx = ctx->hintItem();
    for (auto i = 0u; GEAX_OK(ret) && i < hintListCtx.size(); ++i) {
        Hint* hint = nullptr;
        if (GEAX_IS_NULL(hintListCtx[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "HintItem is not set in HintItemlist: " << K(ret);
        } else if (GEAX_RET_FAIL(ACCEPT_RULE(hintListCtx[i]))) {
            LOG(WARNING) << "Failed to visit rule HintItem: " << K(ret);
        } else if (GEAX_UNLIKELY(!checkedCast(childRes_, hint))) {
            ret = GEAXErrorCode::GEAX_OOPS;
            LOG(WARNING) << "Child result should be a Hint: " << K(ret);
        } else {
            result->appendHint(hint);
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlReadConsistency(parser::GqlParser::GqlReadConsistencyContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    ReadConsistency* readConsistency = ALLOC_GEAOBJECT(ReadConsistency);
    DEFER(readConsistency, ret);

    GqlParser::IdentifierContext* identifierCtx = nullptr;
    if (GEAX_IS_NULL(identifierCtx = ctx->identifier())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Identifier is not set in rule GqlReadConsistency: " << K(ret);
    } else {
        readConsistency->setVal(identifierCtx->getText());
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlAllowAnonymousTable(
    parser::GqlParser::GqlAllowAnonymousTableContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    AllowAnonymousTable* allowAnonymousTable = ALLOC_GEAOBJECT(AllowAnonymousTable);
    DEFER(allowAnonymousTable, ret);

    GqlParser::UnsignedBooleanSpecificationContext* boolCtx = nullptr;
    if (GEAX_IS_NULL(boolCtx = ctx->unsignedBooleanSpecification())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "UnsignedBooleanSpecification is not set in rule GqlAllowAnonymousTable: "
                     << K(ret);
    } else if (GEAX_RET_FAIL(ACCEPT_RULE(boolCtx))) {
        LOG(WARNING) << "Failed to visit expr rule unsignedBooleanSpecification: " << K(ret);
    } else {
        switch (childRes_->type()) {
        case AstNodeType::kVBool: {
            VBool* vBool = castAs<VBool>(childRes_, AstNodeType::kVBool);
            allowAnonymousTable->setVal(vBool->val());
            break;
        }
        case AstNodeType::kParam: {
            Param* param = castAs<Param>(childRes_, AstNodeType::kParam);
            allowAnonymousTable->setVal(param);
            break;
        }
        default: {
            ret = GEAXErrorCode::GEAX_OOPS;
            LOG(WARNING) << "Child result should be VBool or Param: " << K(ret);
            break;
        }
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitReturnItemList(GqlParser::ReturnItemListContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    PrimitiveResultStatement* result = nullptr;
    if (GEAX_IS_NULL(result = castAs<PrimitiveResultStatement>(
                         faRes_, AstNodeType::kPrimitiveResultStatement))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kPrimitiveResultStatement: " << K(ret);
    }
    std::vector<GqlParser::ReturnItemContext*> returnListCtx = ctx->returnItem();
    for (auto i = 0u; GEAX_OK(ret) && i < returnListCtx.size(); ++i) {
        if (GEAX_IS_NULL(returnListCtx[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "ReturnItem is not set in ReturnItemlist: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(returnListCtx[i], result))) {
            LOG(WARNING) << "Failed to visit rule ReturnItem: " << K(ret);
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitReturnItem(GqlParser::ReturnItemContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    GqlParser::ExpressionContext* exprCtx = nullptr;
    Expr* expr = nullptr;
    PrimitiveResultStatement* result = nullptr;
    if (GEAX_IS_NULL(result = castAs<PrimitiveResultStatement>(
                         faRes_, AstNodeType::kPrimitiveResultStatement))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kPrimitiveResultStatement: " << K(ret);
    } else if (GEAX_IS_NULL(exprCtx = ctx->expression())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Expression is not set in rule ReturnItem: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, expr))) {
        LOG(WARNING) << "Failed to visit expr rule: " << K(ret);
    }

    GqlParser::IdentifierContext* idCtx = nullptr;
    std::string name;
    if (GEAX_FAIL(ret)) {
    } else if ((idCtx = ctx->identifier()) != nullptr) {
        name = idCtx->getText();
    } else {
        name = getFullText(exprCtx);
    }

    if (GEAX_OK(ret)) {
        result->appendItem(std::move(name), expr);
    }

    return std::any();
}

std::any GQLAstVisitor::visitSelectStatement(GqlParser::SelectStatementContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    SelectStatement* selectStmt = ALLOC_GEAOBJECT(SelectStatement);
    DEFER(selectStmt, ret);

    GqlParser::HintItemlistContext* hintItems = nullptr;
    GqlParser::SetQuantifierContext* quantifier = nullptr;
    GqlParser::SelectItemListContext* selectItems = nullptr;
    GqlParser::SelectStatementBodyContext* selectBody = nullptr;
    GqlParser::GroupByClauseContext* groupBy = nullptr;
    GqlParser::OffsetClauseContext* offset = nullptr;
    GqlParser::LimitClauseContext* limit = nullptr;
    GqlParser::OrderByClauseContext* orderby = nullptr;
    GqlParser::HavingClauseContext* having = nullptr;
    GqlParser::WhereClauseContext* where = nullptr;
    antlr4::tree::TerminalNode* all = nullptr;

    PrimitiveResultStatement* result = ALLOC_GEAOBJECT(PrimitiveResultStatement);
    faRes_ = result;
    // skip the 'SELECT'
    for (auto i = 1u; i < ctx->children.size() && GEAX_OK(ret); ++i) {
        // since dynamic_casting a nullptr will procude a nullptr,
        // we do not check the nullness here
        if (hintItems == nullptr && checkedCast(ctx->children[i], hintItems)) {
            if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(hintItems, result))) {
                LOG(WARNING) << "Failed to visit rule HintItemlist: " << K(ret);
            }
        } else if (quantifier == nullptr && checkedCast(ctx->children[i], quantifier)) {
            if (size_t token = 0; GEAX_RET_FAIL(VISIT_TOKEN(quantifier, token))) {
                LOG(WARNING) << "Failed to visit token SetQuantifier: " << K(ret);
            } else {
                switch (token) {
                case GqlParser::DISTINCT:
                    result->setDistinct(true);
                    break;
                case GqlParser::ALL:
                    result->setDistinct(false);
                    break;
                default:
                    ret = GEAXErrorCode::GEAX_OOPS;
                    LOG(WARNING) << "Invalid SetQuantifier token type: "
                                 << KV("token", token_->getText()) << DK(ret);
                    break;
                }
            }
        } else if (selectItems == nullptr && checkedCast(ctx->children[i], selectItems)) {
            if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(selectItems, result))) {
                LOG(WARNING) << "Failed to visit rule ReturnItemList: " << K(ret);
            }
        } else if (selectBody == nullptr && checkedCast(ctx->children[i], selectBody)) {
            if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(selectBody, selectStmt))) {
                LOG(WARNING) << "Failed to visit rule SelectStatementBody: " << K(ret);
            }
        } else if (groupBy == nullptr && checkedCast(ctx->children[i], groupBy)) {
            GqlParser::GroupingElementListContext* groupbyItemsCtx = nullptr;
            if (GEAX_IS_NULL(groupbyItemsCtx = groupBy->groupingElementList())) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "Child of GroupByClause is not set: " << K(ret);
            } else {
                auto items = groupbyItemsCtx->groupingElement();
                for (auto j = 0u; GEAX_OK(ret) && j < items.size(); ++j) {
                    if (GEAX_IS_NULL(items[j])) {
                        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                        LOG(WARNING) << "GroupingElement is not set: " << K(ret);
                    } else if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(items[j], result))) {
                        LOG(WARNING) << "Failed to visit rule GroupByField: " << K(ret);
                    }
                }
            }
        } else if (having == nullptr && checkedCast(ctx->children[i], having)) {
            Expr* expr = nullptr;
            GqlParser::ExpressionContext* exprCtx = nullptr;
            if (GEAX_IS_NULL(exprCtx = having->expression())) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "Child of HavingClause is not set: " << K(ret);
            } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, expr))) {
                LOG(WARNING) << "Failed to visit rule Expression: " << K(ret);
            } else {
                selectStmt->setHaving(expr);
            }
        } else if (where == nullptr && checkedCast(ctx->children[i], where)) {
            Expr* expr = nullptr;
            GqlParser::ExpressionContext* exprCtx = nullptr;
            if (GEAX_IS_NULL(exprCtx = where->expression())) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "Child of WhereClause is not set: " << K(ret);
            } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, expr))) {
                LOG(WARNING) << "Failed to visit rule Expression: " << K(ret);
            } else {
                selectStmt->setWhere(expr);
            }
        } else if (offset == nullptr && checkedCast(ctx->children[i], offset)) {
            GqlParser::UnsignedIntegerSpecificationContext* ictx = nullptr;
            if (GEAX_IS_NULL(ictx = offset->unsignedIntegerSpecification())) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "offset is not set unsinged integer: " << K(ret);
            } else if (GEAX_RET_FAIL(ACCEPT_RULE(ictx))) {
                LOG(WARNING) << "Failed to visit expr rule unsignedIntegerSpecification: "
                             << K(ret);
            } else {
                switch (childRes_->type()) {
                case AstNodeType::kVInt: {
                    VInt* integer = castAs<VInt>(childRes_, AstNodeType::kVInt);
                    result->setOffset(integer->val());
                    break;
                }
                case AstNodeType::kParam: {
                    Param* param = castAs<Param>(childRes_, AstNodeType::kParam);
                    result->setOffset(param);
                    break;
                }
                default: {
                    ret = GEAXErrorCode::GEAX_OOPS;
                    LOG(WARNING) << "Child result should be VInt or Param: " << K(ret);
                    break;
                }
                }
            }
        } else if (limit == nullptr && checkedCast(ctx->children[i], limit)) {
            GqlParser::UnsignedIntegerSpecificationContext* ictx = nullptr;
            if (GEAX_IS_NULL(ictx = limit->unsignedIntegerSpecification())) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "limit is not set unsigned integer: " << K(ret);
            } else if (GEAX_RET_FAIL(ACCEPT_RULE(ictx))) {
                LOG(WARNING) << "Failed to visit expr rule UnsignedIntegerSpecification: "
                             << K(ret);
            } else {
                switch (childRes_->type()) {
                case AstNodeType::kVInt: {
                    VInt* integer = castAs<VInt>(childRes_, AstNodeType::kVInt);
                    result->setLimit(integer->val());
                    break;
                }
                case AstNodeType::kParam: {
                    Param* param = castAs<Param>(childRes_, AstNodeType::kParam);
                    result->setLimit(param);
                    break;
                }
                default: {
                    ret = GEAXErrorCode::GEAX_OOPS;
                    LOG(WARNING) << "Child result should be VInt or Param: " << K(ret);
                    break;
                }
                }
            }
        } else if (orderby == nullptr && checkedCast(ctx->children[i], orderby)) {
            GqlParser::SortSpecificationListContext* orderListCtx = nullptr;
            if (GEAX_IS_NULL(orderListCtx = orderby->sortSpecificationList())) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "Child of OrderByClause is not set: " << K(ret);
            } else {
                auto items = orderListCtx->sortSpecification();
                for (auto j = 0u; GEAX_OK(ret) && j < items.size(); ++j) {
                    OrderByField* field = nullptr;
                    if (GEAX_IS_NULL(items[j])) {
                        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                        LOG(WARNING) << "Child of SortSpecification is not set: " << K(ret);
                    } else if (GEAX_RET_FAIL(
                                   VISIT_RULE(items[j], AstNodeType::kOrderByField, field))) {
                        LOG(WARNING) << "Failed to visit rule sortspecification: " << K(ret);
                    } else {
                        result->appendOrderBy(field);
                    }
                }
            }
        } else if (all == nullptr && checkedCast(ctx->children[i], all)) {
            // TODO(boyao.zby) select * is not implemnent
            ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
            LOG(WARNING) << "SELECT * is not supported yet: " << K(ret);
        }
    }
    if (GEAX_OK(ret)) {
        selectStmt->setResultStatement(result);
    }
    return std::any();
}

std::any GQLAstVisitor::visitSelectItemList(GqlParser::SelectItemListContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    PrimitiveResultStatement* result = nullptr;
    if (GEAX_IS_NULL(result = castAs<PrimitiveResultStatement>(
                         faRes_, AstNodeType::kPrimitiveResultStatement))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kPrimitiveResultStatement: " << K(ret);
    }
    std::vector<GqlParser::SelectItemContext*> selectListCtx = ctx->selectItem();
    for (auto i = 0u; GEAX_OK(ret) && i < selectListCtx.size(); ++i) {
        if (GEAX_IS_NULL(selectListCtx[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "SelectItem is not set in SelectItemlist: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(selectListCtx[i], result))) {
            LOG(WARNING) << "Failed to visit rule SelectItem: " << K(ret);
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitSelectItem(GqlParser::SelectItemContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    GqlParser::ExpressionContext* exprCtx = nullptr;
    Expr* expr = nullptr;
    PrimitiveResultStatement* result = nullptr;
    if (GEAX_IS_NULL(result = castAs<PrimitiveResultStatement>(
                         faRes_, AstNodeType::kPrimitiveResultStatement))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kPrimitiveResultStatement: " << K(ret);
    } else if (GEAX_IS_NULL(exprCtx = ctx->expression())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Expression is not set in rule ReturnItem: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, expr))) {
        LOG(WARNING) << "Failed to visit expr rule: " << K(ret);
    }

    GqlParser::IdentifierContext* idCtx = nullptr;
    std::string name;
    if (GEAX_FAIL(ret)) {
    } else if ((idCtx = ctx->identifier()) != nullptr) {
        name = idCtx->getText();
    } else {
        name = getFullText(exprCtx);
    }

    if (GEAX_OK(ret)) {
        result->appendItem(std::move(name), expr);
    }

    return std::any();
}

std::any GQLAstVisitor::visitSelectStatementBody(GqlParser::SelectStatementBodyContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    SelectStatement* selectStmt = nullptr;
    if (GEAX_IS_NULL(selectStmt = castAs<SelectStatement>(faRes_, AstNodeType::kSelectStatement))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node kSelectStatement: " << K(ret);
    }
    auto matchList = ctx->matchStatement();
    if (matchList.size() == 0) {
        ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
        LOG(WARNING) << "FROM is not supported in SelectStatement yet:" << K(ret);
    }
    for (auto i = 0u; GEAX_OK(ret) && i < matchList.size(); ++i) {
        MatchStatement* match = nullptr;
        if (GEAX_IS_NULL(matchList[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "Child of rule matchStatement is not set: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_RULE(matchList[i], AstNodeType::kMatchStatement, match))) {
            LOG(WARNING) << "Failed to visit rule matchStatement: " << K(ret);
        } else {
            selectStmt->appendFromClause("", match);
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitBindingVariable(GqlParser::BindingVariableContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Ref* ref = ALLOC_GEAOBJECT(Ref);
    DEFER(ref, ret);

    ref->setName(ctx->getText());

    return std::any();
}

std::any GQLAstVisitor::visitParameter(GqlParser::ParameterContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Param* param = ALLOC_GEAOBJECT(Param);
    DEFER(param, ret);

    param->setName(ctx->getText());

    return std::any();
}

std::any GQLAstVisitor::visitGqlNotExpression(GqlParser::GqlNotExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Not* notExpr = ALLOC_GEAOBJECT(Not);
    DEFER(notExpr, ret);

    Expr* expr = nullptr;
    GqlParser::ExpressionContext* exprCtx = nullptr;
    if (GEAX_IS_NULL(exprCtx = ctx->expression())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Expression is not set in rule NotExpresson: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, expr))) {
        LOG(WARNING) << "Failed to visit rule expression: " << K(ret);
    } else {
        notExpr->setExpr(expr);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlLogicalAndExpression(
    GqlParser::GqlLogicalAndExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    BAnd* exprNode = ALLOC_GEAOBJECT(BAnd);
    DEFER(exprNode, ret);

    if (GEAX_RET_FAIL(visitBinaryExpr(ctx->lhs, ctx->rhs, exprNode))) {
        LOG(WARNING) << "Failed to visit LogicalAndExpression: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlLogicalXorExpression(
    GqlParser::GqlLogicalXorExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    BXor* exprNode = ALLOC_GEAOBJECT(BXor);
    DEFER(exprNode, ret);

    if (GEAX_RET_FAIL(visitBinaryExpr(ctx->lhs, ctx->rhs, exprNode))) {
        LOG(WARNING) << "Failed to visit LogicalXorExpression: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlLogicalOrExpression(GqlParser::GqlLogicalOrExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    BOr* exprNode = ALLOC_GEAOBJECT(BOr);
    DEFER(exprNode, ret);

    if (GEAX_RET_FAIL(visitBinaryExpr(ctx->lhs, ctx->rhs, exprNode))) {
        LOG(WARNING) << "Failed to visit LogicalOrExpression: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlBooleanTestExpression(
    GqlParser::GqlBooleanTestExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Expr* expr = nullptr;
    DEFER(expr, ret);

    bool isEqual = true;
    bool truthValue = false;
    for (auto i = 0u; GEAX_OK(ret) && i < ctx->children.size(); ++i) {
        if (GEAX_IS_NULL(ctx->children[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(ERROR) << "Child is not set for rule GqlBooleanTestExpression: " << K(ret);
        } else if (ctx->children[i]->getTreeType() == antlr4::tree::ParseTreeType::RULE) {
            antlr4::ParserRuleContext* rule = nullptr;
            if (GEAX_UNLIKELY(!uncheckedCast(ctx->children[i], rule))) {
                ret = GEAXErrorCode::GEAX_OOPS;
                LOG(WARNING) << "Failed to cast to rule base: " << K(ret);
            } else if (rule->getRuleIndex() == GqlParser::RuleExpressionPredicate) {
                if (GEAX_RET_FAIL(VISIT_EXPR(rule, expr))) {
                    LOG(WARNING) << "Failed to visit rule ExpressionPredicate: " << K(ret);
                }
            } else if (GEAX_RET_FAIL(byPass(rule))) {  // truthValue
                LOG(WARNING) << "Failed to visit children of rule truthValue: " << K(ret);
            } else if (token_->getType() == GqlParser::TRUE) {
                truthValue = true;
            } else if (token_->getType() == GqlParser::FALSE) {
                truthValue = false;
            } else {
                ret = GEAXErrorCode::GEAX_OOPS;
                LOG(WARNING) << "Invalid token value: " << KV("token", token_->getType())
                             << DK(ret);
            }
        } else if (ctx->children[i]->getTreeType() == antlr4::tree::ParseTreeType::TERMINAL) {
            if (auto token{0}; GEAX_RET_FAIL(VISIT_TOKEN(ctx->children[i], token))) {
                LOG(WARNING) << "Failed to check token result: " << K(ret);
            } else {
                switch (token) {
                case GqlParser::IS:
                    [[fallthrough]];
                case GqlParser::EQUALS_OPERATOR:
                    isEqual = true;
                    break;
                case GqlParser::NOT:
                    [[fallthrough]];
                case GqlParser::NOT_EQUALS_OPERATOR:
                    isEqual = false;
                    break;
                default:
                    ret = GEAXErrorCode::GEAX_OOPS;
                    LOG(WARNING) << "Invalid token value: " << KV("token", token_->getText())
                                 << DK(ret);
                    break;
                }
            }
        }
    }
    // is true
    // is false
    // not true
    // not false
    if (GEAX_FAIL(ret)) {
    } else if (isEqual ^ truthValue) {
        Not* notExpr = ALLOC_GEAOBJECT(Not);
        notExpr->setExpr(expr);
        expr = notExpr;
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlBitXorExpression(GqlParser::GqlBitXorExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    BBitXor* exprNode = ALLOC_GEAOBJECT(BBitXor);
    DEFER(exprNode, ret);

    if (GEAX_RET_FAIL(visitBinaryExpr(ctx->lhs, ctx->rhs, exprNode))) {
        LOG(WARNING) << "Failed to visit BitXorExpression: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlBitShiftExpression(GqlParser::GqlBitShiftExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    BinaryOp* exprNode = nullptr;
    DEFER(exprNode, ret);

    if (ctx->LEFT_SHIFT() != nullptr) {
        exprNode = ALLOC_GEAOBJECT(BBitLeftShift);
    } else {
        exprNode = ALLOC_GEAOBJECT(BBitRightShift);
    }
    if (GEAX_RET_FAIL(visitBinaryExpr(ctx->lhs, ctx->rhs, exprNode))) {
        LOG(WARNING) << "Failed to visit BitXorExpression: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlBitAndExpression(GqlParser::GqlBitAndExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    BBitAnd* exprNode = ALLOC_GEAOBJECT(BBitAnd);
    DEFER(exprNode, ret);

    if (GEAX_RET_FAIL(visitBinaryExpr(ctx->lhs, ctx->rhs, exprNode))) {
        LOG(WARNING) << "Failed to visit BitXorExpression: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlBitOrExpression(GqlParser::GqlBitOrExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    BBitOr* exprNode = ALLOC_GEAOBJECT(BBitOr);
    DEFER(exprNode, ret);

    if (GEAX_RET_FAIL(visitBinaryExpr(ctx->lhs, ctx->rhs, exprNode))) {
        LOG(WARNING) << "Failed to visit BitXorExpression: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlHighArithmeticExpression(
    GqlParser::GqlHighArithmeticExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    BinaryOp* exprNode = nullptr;
    DEFER(exprNode, ret);

    if (GEAX_IS_NULL(ctx->op)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "HighArithmeticOperator is not set: " << K(ret);
    } else {
        switch (ctx->op->getType()) {
        case GqlParser::ASTERISK:
            exprNode = ALLOC_GEAOBJECT(BMul);
            break;
        case GqlParser::SOLIDUS:
            exprNode = ALLOC_GEAOBJECT(BDiv);
            break;
        case GqlParser::PERCENT:
        case GqlParser::MOD:
            exprNode = ALLOC_GEAOBJECT(BMod);
            break;
        default:
            ret = GEAXErrorCode::GEAX_OOPS;
            LOG(WARNING) << "Invalid token value: " << KV("token", ctx->op->getType()) << DK(ret);
            break;
        }

        if (GEAX_OK(ret) && GEAX_RET_FAIL(visitBinaryExpr(ctx->lhs, ctx->rhs, exprNode))) {
            LOG(WARNING) << "Failed to visit HighArithmeticExpression: " << K(ret);
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlLowArithmeticExpression(
    GqlParser::GqlLowArithmeticExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    BinaryOp* exprNode = nullptr;
    DEFER(exprNode, ret);

    if (ctx->PLUS_SIGN() != nullptr) {
        exprNode = ALLOC_GEAOBJECT(BAdd);
    } else {
        exprNode = ALLOC_GEAOBJECT(BSub);
    }
    if (GEAX_RET_FAIL(visitBinaryExpr(ctx->lhs, ctx->rhs, exprNode))) {
        LOG(WARNING) << "Failed to visit LowArithmeticExpression: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlComparisonExpression(
    GqlParser::GqlComparisonExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    BinaryOp* expr = nullptr;
    DEFER(expr, ret);

    GqlParser::CompOpContext* compCtx = ctx->compOp();
    if (GEAX_ANY_NULL(ctx->lhs, ctx->rhs, compCtx)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Child of GqlComparisonExpression is not set: "
                     << KMP(ctx->lhs, ctx->rhs, compCtx) << DK(ret);
    } else if (GEAX_RET_FAIL(byPass(compCtx))) {
        LOG(WARNING) << "Failed to visit children of rule CompOp: " << K(ret);
    } else if (GEAX_RET_FAIL(checkTokenResult())) {
        LOG(WARNING) << "Failed to check token result: " << K(ret);
    } else {
        switch (token_->getType()) {
        case GqlParser::EQUALS_OPERATOR:
            expr = ALLOC_GEAOBJECT(BEqual);
            break;
        case GqlParser::NOT_EQUALS_OPERATOR:
            expr = ALLOC_GEAOBJECT(BNotEqual);
            break;
        case GqlParser::LEFT_ANGLE_BRACKET:
            expr = ALLOC_GEAOBJECT(BSmallerThan);
            break;
        case GqlParser::RIGHT_ANGLE_BRACKET:
            expr = ALLOC_GEAOBJECT(BGreaterThan);
            break;
        case GqlParser::LESS_THAN_OR_EQUALS_OPERATOR:
            expr = ALLOC_GEAOBJECT(BNotGreaterThan);
            break;
        case GqlParser::GREATER_THAN_OR_EQUALS_OPERATOR:
            expr = ALLOC_GEAOBJECT(BNotSmallerThan);
            break;
        case GqlParser::SAFE_EXQUAL_OPERATOR:
            expr = ALLOC_GEAOBJECT(BSafeEqual);
            break;
        default:
            ret = GEAXErrorCode::GEAX_OOPS;
            LOG(ERROR) << "Invalid token type: " << KV("token", token_->getText()) << DK(ret);
            break;
        }
    }

    Expr* lhs = nullptr;
    Expr* rhs = nullptr;
    if (GEAX_FAIL(ret)) {
    } else if (GEAX_RET_FAIL(VISIT_EXPR(ctx->lhs, lhs))) {
        LOG(WARNING) << "Failed to visit lhs expr: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(ctx->rhs, rhs))) {
        LOG(WARNING) << "Failed to visit rhs expr: " << K(ret);
    } else {
        expr->setLeft(lhs);
        expr->setRight(rhs);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlInExpression(GqlParser::GqlInExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    GqlParser::ExpressionPredicateContext* exprCtx = ctx->expressionPredicate();
    GqlParser::ListValueContext* listCtx = ctx->listValue();
    Expr* lhs = nullptr;
    Expr* rhs = nullptr;
    if (GEAX_ANY_NULL(exprCtx, listCtx)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Child of GqlInExpression is not set " << KMP(exprCtx, listCtx) << DK(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, lhs))) {
        LOG(WARNING) << "Failed to visit lhs expr: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(listCtx, rhs))) {
        LOG(WARNING) << "Failed to visit rhs expr: " << K(ret);
    } else {
        BIn* in = ALLOC_GEAOBJECT(BIn);
        in->setLeft(lhs);
        in->setRight(rhs);
        if (nullptr != ctx->NOT()) {
            Not* notOp = ALLOC_GEAOBJECT(Not);
            notOp->setExpr(in);
            childRes_ = notOp;
        } else {
            childRes_ = in;
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitGqlLikeExpression(GqlParser::GqlLikeExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    Expr* selfExprNode{nullptr};
    Expr* regexExprNode{nullptr};
    if (GEAX_ANY_NULL(ctx->self, ctx->regex)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Child of GqlLikeExpression is not set " << KMP(ctx->self, ctx->regex)
                     << DK(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(ctx->self, selfExprNode))) {
        LOG(WARNING) << "Fail to visit self children of rule GqlLikeExpression" << DK(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(ctx->regex, regexExprNode))) {
        LOG(WARNING) << "Fail to visit regex children of rule GqlLikeExpression" << DK(ret);
    } else {
        BLike* like = ALLOC_GEAOBJECT(BLike);
        like->setLeft(selfExprNode);
        like->setRight(regexExprNode);
        if (nullptr != ctx->NOT()) {
            Not* notOp = ALLOC_GEAOBJECT(Not);
            notOp->setExpr(like);
            childRes_ = notOp;
        } else {
            childRes_ = like;
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitGqlBetweenExpression(GqlParser::GqlBetweenExpressionContext* ctx) {
    // childRet_ = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    return std::any();
}

std::any GQLAstVisitor::visitGqlExistsExpression(GqlParser::GqlExistsExpressionContext* ctx) {
    // childRet_ = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    return std::any();
}

std::any GQLAstVisitor::visitGqlNullExpression(GqlParser::GqlNullExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    auto expr = ctx->expressionPredicate();
    auto nullCond = ctx->nullPredicateCond();
    Expr* exprNode = nullptr;
    if (GEAX_ANY_NULL(expr, nullCond)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Child of GqlNullExpression is not set " << KMP(expr, nullCond) << DK(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(expr, exprNode))) {
        LOG(WARNING) << "Fail to visit expressionPredicate children of rule GqlNullExpression"
                     << DK(ret);
    } else {
        IsNull* isNull = ALLOC_GEAOBJECT(IsNull);
        isNull->setExpr(exprNode);
        if (nullptr != nullCond->NOT()) {
            Not* notOp = ALLOC_GEAOBJECT(Not);
            notOp->setExpr(isNull);
            childRes_ = notOp;
        } else {
            childRes_ = isNull;
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitGqlNormalizedExpression(
    GqlParser::GqlNormalizedExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    auto expr = ctx->expressionPredicate();
    auto normalizedCond = ctx->normalizedPredicateCond();
    Expr* exprNode = nullptr;
    if (GEAX_ANY_NULL(expr, normalizedCond)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Child of GqlNormalizedExpression is not set " << KMP(expr, normalizedCond)
                     << DK(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(expr, exprNode))) {
        LOG(WARNING) << "Fail to visit expressionAtom children of rule GqlNormalizedExpression"
                     << DK(ret);
    } else {
        IsNormalized* isNormalized = ALLOC_GEAOBJECT(IsNormalized);
        isNormalized->setExpr(exprNode);
        if (nullptr != normalizedCond->NOT()) {
            Not* notOp = ALLOC_GEAOBJECT(Not);
            notOp->setExpr(isNormalized);
            childRes_ = notOp;
        } else {
            childRes_ = isNormalized;
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitGqlDirectedExpression(GqlParser::GqlDirectedExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    auto eleRef = ctx->elementVariableReference();
    auto directedCond = ctx->directedPredicateCond();
    Ref* ref = nullptr;
    if (GEAX_ANY_NULL(eleRef, directedCond)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Child of GqlDirectedExpression is not set " << KMP(eleRef, directedCond)
                     << DK(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(eleRef, ref))) {
        LOG(WARNING) << "Fail to visit elementReference children of rule GqlDirectedExpression"
                     << DK(ret);
    } else {
        IsDirected* isDirected = ALLOC_GEAOBJECT(IsDirected);
        isDirected->setExpr(ref);
        if (nullptr != directedCond->NOT()) {
            Not* notOp = ALLOC_GEAOBJECT(Not);
            notOp->setExpr(isDirected);
            childRes_ = notOp;
        } else {
            childRes_ = isDirected;
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitGqlLabeledExpression(GqlParser::GqlLabeledExpressionContext* ctx) {
    // childRet_ = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    return std::any();
}

std::any GQLAstVisitor::visitGqlSourceDestinationExpression(
    GqlParser::GqlSourceDestinationExpressionContext* ctx) {
    // childRet_ = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    return std::any();
}

std::any GQLAstVisitor::visitGqlAllDifferentExpression(
    GqlParser::GqlAllDifferentExpressionContext* ctx) {
    // childRet_ = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    return std::any();
}

std::any GQLAstVisitor::visitGqlSameExpression(GqlParser::GqlSameExpressionContext* ctx) {
    // childRet_ = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    return std::any();
}

std::any GQLAstVisitor::visitGqlUnaryExpression(GqlParser::GqlUnaryExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Expr* unaryOp = nullptr;
    DEFER(unaryOp, ret);

    Expr* exprNode{nullptr};
    auto atomCtx = ctx->expressionAtom();
    auto unaryOpCtx = ctx->unaryOperator();
    if (GEAX_ANY_NULL(atomCtx, unaryOpCtx)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Child of GqlUnaryExpression is not set " << KMP(atomCtx, unaryOpCtx)
                     << DK(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(atomCtx, exprNode))) {
        LOG(WARNING) << "Fail to visit children of rule GqlUnaryExpression" << DK(ret);
    } else if (auto token{0}; GEAX_RET_FAIL(VISIT_TOKEN(unaryOpCtx, token))) {
        LOG(WARNING) << "Fail to visit children of rule GqlUnaryExpression" << DK(ret);
    } else {
        switch (token) {
        case GqlParser::EXCLAMATION_MARK: {
            Not* notOp = ALLOC_GEAOBJECT(Not);
            notOp->setExpr(exprNode);
            unaryOp = notOp;
            break;
        }
        case GqlParser::TILDE: {
            Tilde* tildeOp = ALLOC_GEAOBJECT(Tilde);
            tildeOp->setExpr(exprNode);
            unaryOp = tildeOp;
            break;
        }
        case GqlParser::MINUS_SIGN: {
            Neg* negOp = ALLOC_GEAOBJECT(Neg);
            negOp->setExpr(exprNode);
            unaryOp = negOp;
            break;
        }
        case GqlParser::PLUS_SIGN: {
            unaryOp = exprNode;
            break;
        }
        default: {
            ret = GEAXErrorCode::GEAX_OOPS;
            LOG(WARNING) << "Invalid unaryOperator token type: " << KV("token", token) << K(ret);
            break;
        }
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitGqlValueQueryExpression(
    GqlParser::GqlValueQueryExpressionContext* ctx) {
    // childRet_ = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    return std::any();
}

// TODO(boyao.zby) only support A|B|C now
std::any GQLAstVisitor::visitLabelExpression(GqlParser::LabelExpressionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    LabelTree* labelTree = nullptr;
    DEFER(labelTree, ret);

    std::vector<GqlParser::LabelTermContext*> labelCtxs = ctx->labelTerm();
    for (auto i = 0u; GEAX_OK(ret) && i < labelCtxs.size(); ++i) {
        LabelTree* label = nullptr;
        if (GEAX_IS_NULL(labelCtxs[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "Child of rule LabelExpression is not set: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_EXPR(labelCtxs[i], label))) {
            LOG(WARNING) << "Failed to visit rule LabelTerm: " << K(ret);
        } else {
            if (labelTree != nullptr) {
                LabelOr* labelOr = ALLOC_GEAOBJECT(LabelOr);
                labelOr->setLeft(labelTree);
                labelOr->setRight(label);
                labelTree = labelOr;
            } else {
                labelTree = label;
            }
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitLabelTerm(GqlParser::LabelTermContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    LabelTree* labelTree = nullptr;
    DEFER(labelTree, ret);

    std::vector<GqlParser::LabelFactorContext*> labelCtxs = ctx->labelFactor();
    for (auto i = 0u; i < labelCtxs.size() && GEAX_OK(ret); ++i) {
        LabelTree* label = nullptr;
        if (GEAX_IS_NULL(labelCtxs[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "Child of rule LabelTerm is not set: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_EXPR(labelCtxs[i], label))) {
            LOG(WARNING) << "Failed to visit rule LabelFactor: " << K(ret);
        } else {
            if (labelTree != nullptr) {
                LabelAnd* labelAnd = ALLOC_GEAOBJECT(LabelAnd);
                labelAnd->setLeft(labelTree);
                labelAnd->setRight(label);
                labelTree = labelAnd;
            } else {
                labelTree = label;
            }
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitLabelFactor(GqlParser::LabelFactorContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    LabelTree* labelTree = nullptr;
    DEFER(labelTree, ret);

    GqlParser::LabelPrimaryContext* labelCtx = ctx->labelPrimary();
    SingleLabel* label = nullptr;
    if (GEAX_IS_NULL(labelCtx)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "LabelPrimary is not set in LabelFactor: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE(labelCtx, AstNodeType::kSingleLabel, label))) {
        LOG(WARNING) << "Failed to visit rule LabelPrimary: " << K(ret);
    } else {
        if (nullptr != ctx->EXCLAMATION_MARK()) {
            LabelNot* labelNot = ALLOC_GEAOBJECT(LabelNot);
            labelNot->setExpr(label);
            labelTree = labelNot;
        } else {
            labelTree = label;
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitLabelName(GqlParser::LabelNameContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    SingleLabel* label = ALLOC_GEAOBJECT(SingleLabel);
    DEFER(label, ret);

    std::string name = ctx->getText();
    if (name.compare("``") == 0) {
        ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
        LOG(WARNING) << "Empty label name is not supported: " << K(ret);
    } else {
        label->setLabel(trimAccentGrave(name));
    }
    return std::any();
}

std::any GQLAstVisitor::visitFloatLiteral(GqlParser::FloatLiteralContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    VDouble* d = ALLOC_GEAOBJECT(VDouble);
    DEFER(d, ret);

    d->setVal(atof(ctx->getText().c_str()));

    return std::any();
}

std::any GQLAstVisitor::visitIntegerLiteral(GqlParser::IntegerLiteralContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    VInt* i = ALLOC_GEAOBJECT(VInt);
    DEFER(i, ret);
    if (ctx->UNSIGNED_OCTAL_INTEGER() != nullptr) {
        i->setVal(std::strtoll(ctx->getText().replace(0, 2, "0").c_str(), nullptr, 8));
    } else if (ctx->UNSIGNED_HEXADECIMAL_INTEGER() != nullptr) {
        i->setVal(std::strtoll(ctx->getText().c_str(), nullptr, 16));
    } else if (ctx->UNSIGNED_BINARY_INTEGER() != nullptr) {
        i->setVal(std::strtoll(ctx->getText().substr(2).c_str(), nullptr, 2));
    } else {
        i->setVal(std::atoll(ctx->getText().c_str()));
    }
    return std::any();
}

std::any GQLAstVisitor::visitBooleanLiteral(GqlParser::BooleanLiteralContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    VBool* b = ALLOC_GEAOBJECT(VBool);
    DEFER(b, ret);

    // TODO(boyao.zby) NOT consider UNKNOWN
    b->setVal(ctx->TRUE() != nullptr);

    return std::any();
}

std::any GQLAstVisitor::visitCharacterStringLiteral(GqlParser::CharacterStringLiteralContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    VString* s = ALLOC_GEAOBJECT(VString);
    DEFER(s, ret);

    // token_->getText() need to remove the quotes
    std::string str = ctx->getText();
    s->setVal(escapeText(str.substr(1, str.size() - 2)));

    return std::any();
}

std::any GQLAstVisitor::visitByteStringLiteral(GqlParser::ByteStringLiteralContext* ctx) {
    childRet_ = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    return std::any();
}

std::any GQLAstVisitor::visitNullLiteral(GqlParser::NullLiteralContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    VNull* n = ALLOC_GEAOBJECT(VNull);
    DEFER(n, ret);
    return std::any();
}

std::any GQLAstVisitor::visitDateLiteral(GqlParser::DateLiteralContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    VDate* d = ALLOC_GEAOBJECT(VDate);
    DEFER(d, ret);

    auto strCtx = ctx->characterStringLiteral();
    if (GEAX_IS_NULL(strCtx)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Date string is not set for rule DateLiteral: " << K(ret);
    } else {
        std::string str = strCtx->getText();
        // strCtx->getText() need to remove the quotes
        d->setVal(str.substr(1, str.size() - 2));
    }
    return std::any();
}

std::any GQLAstVisitor::visitDatetimeLiteral(GqlParser::DatetimeLiteralContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    VDatetime* d = ALLOC_GEAOBJECT(VDatetime);
    DEFER(d, ret);

    // TODO(boyao.zby) timestamp is not considered
    auto strCtx = ctx->characterStringLiteral();
    if (GEAX_IS_NULL(strCtx)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Datetime string is not set for rule DateTimeLiteral: " << K(ret);
    } else {
        std::string str = strCtx->getText();
        // strCtx->getText() need to remove the quotes
        d->setVal(str.substr(1, str.size() - 2));
    }
    return std::any();
}

std::any GQLAstVisitor::visitTimeLiteral(GqlParser::TimeLiteralContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    VTime* t = ALLOC_GEAOBJECT(VTime);
    DEFER(t, ret);

    auto strCtx = ctx->characterStringLiteral();
    if (GEAX_IS_NULL(strCtx)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Datetime string is not set for rule TimeLiteral: " << K(ret);
    } else {
        std::string str = strCtx->getText();
        // strCtx->getText() need to remove the quotes
        t->setVal(str.substr(1, str.size() - 2));
    }
    return std::any();
}

std::any GQLAstVisitor::visitListLiteral(GqlParser::ListLiteralContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    MkList* l = ALLOC_GEAOBJECT(MkList);
    DEFER(l, ret);
    std::vector<GqlParser::GeneralLiteralContext*> literalCtxs = ctx->generalLiteral();
    if (GEAX_RET_FAIL(visitListLiteralItem(literalCtxs, l))) {
        LOG(WARNING) << "Failed to visit list: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitMapLiteral(GqlParser::MapLiteralContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    MkMap* map = ALLOC_GEAOBJECT(MkMap);
    DEFER(map, ret);

    std::vector<GqlParser::MapElementLiteralContext*> eleCtxs = ctx->mapElementLiteral();
    for (auto i = 0u; GEAX_OK(ret) && i < eleCtxs.size(); ++i) {
        Expr* k = nullptr;
        Expr* v = nullptr;
        if (GEAX_IS_NULL(eleCtxs[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "MapElementLiteral is not set for rule MapLiteral: " << K(ret);
        } else if (GEAX_ANY_NULL(eleCtxs[i]->key, eleCtxs[i]->value)) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "k/v is not set for rule MapElementLiteral: "
                         << KMP(eleCtxs[i]->key, eleCtxs[i]->value) << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_EXPR(eleCtxs[i]->key, k))) {
            LOG(WARNING) << "Failed to visit key expr in MapElementLiteral: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_EXPR(eleCtxs[i]->value, v))) {
            LOG(WARNING) << "Failed to visit value expr in MapElementLiteral: " << K(ret);
        } else {
            map->appendElem(k, v);
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitRecordLiteral(GqlParser::RecordLiteralContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    MkRecord* record = ALLOC_GEAOBJECT(MkRecord);
    DEFER(record, ret);

    std::vector<GqlParser::RecordFieldLiteralContext*> fieldCtxs = ctx->recordFieldLiteral();
    for (auto i = 0u; GEAX_OK(ret) && i < fieldCtxs.size(); ++i) {
        Expr* v = nullptr;
        if (GEAX_IS_NULL(fieldCtxs[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "Field is not set for rule RecordLiteral: " << K(ret);
        } else if (GEAX_ANY_NULL(fieldCtxs[i]->key, fieldCtxs[i]->value)) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "k/v is not set for rule RecordLiteral: "
                         << KMP(fieldCtxs[i]->key, fieldCtxs[i]->value) << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_EXPR(fieldCtxs[i]->value, v))) {
            LOG(WARNING) << "Failed to visit value expr in RecordFieldLiteral: " << K(ret);
        } else {
            record->appendElem(fieldCtxs[i]->key->getText(), v);
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitDurationLiteral(GqlParser::DurationLiteralContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    VDuration* d = ALLOC_GEAOBJECT(VDuration);
    DEFER(d, ret);

    auto strCtx = ctx->characterStringLiteral();
    if (GEAX_IS_NULL(strCtx)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Duration string is not set for rule DurationLiteral: " << K(ret);
    } else {
        std::string str = strCtx->getText();
        // strCtx->getText() need to remove the quotes
        d->setVal(escapeText(str.substr(1, str.size() - 2)));
    }
    return std::any();
}

std::any GQLAstVisitor::visitListValueConstructor(GqlParser::ListValueConstructorContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    MkList* l = ALLOC_GEAOBJECT(MkList);
    DEFER(l, ret);

    std::vector<GqlParser::ExpressionContext*> exprCtxs = ctx->expression();
    if (GEAX_RET_FAIL(visitListExpr(exprCtxs, l))) {
        LOG(WARNING) << "Failed to visit list: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitMapValueConstructor(GqlParser::MapValueConstructorContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    MkMap* map = ALLOC_GEAOBJECT(MkMap);
    DEFER(map, ret);

    std::vector<GqlParser::MapElementContext*> eleCtxs = ctx->mapElement();
    for (auto i = 0u; GEAX_OK(ret) && i < eleCtxs.size(); ++i) {
        Expr* k = nullptr;
        Expr* v = nullptr;
        if (GEAX_IS_NULL(eleCtxs[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "MapElement is not set for rule MapConstructor: " << K(ret);
        } else if (GEAX_ANY_NULL(eleCtxs[i]->key, eleCtxs[i]->value)) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "k/v is not set for rule MapElement: "
                         << KMP(eleCtxs[i]->key, eleCtxs[i]->value) << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_EXPR(eleCtxs[i]->key, k))) {
            LOG(WARNING) << "Failed to visit key expr in MapElement: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_EXPR(eleCtxs[i]->value, v))) {
            LOG(WARNING) << "Failed to visit value expr in MapElement: " << K(ret);
        } else {
            map->appendElem(k, v);
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitRecordValueConstructor(GqlParser::RecordValueConstructorContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    MkRecord* record = ALLOC_GEAOBJECT(MkRecord);
    DEFER(record, ret);

    std::vector<GqlParser::FieldContext*> fieldCtxs = ctx->field();
    for (auto i = 0u; GEAX_OK(ret) && i < fieldCtxs.size(); ++i) {
        Expr* v = nullptr;
        if (GEAX_IS_NULL(fieldCtxs[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "Field is not set for rule RecordConstructor: " << K(ret);
        } else if (GEAX_ANY_NULL(fieldCtxs[i]->key, fieldCtxs[i]->value)) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "k/v is not set for rule RecordConstructor: "
                         << KMP(fieldCtxs[i]->key, fieldCtxs[i]->value) << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_EXPR(fieldCtxs[i]->value, v))) {
            LOG(WARNING) << "Failed to visit value expr in Field: " << K(ret);
        } else {
            record->appendElem(fieldCtxs[i]->key->getText(), v);
        }
    }

    return std::any();
}

std::any GQLAstVisitor::visitListValue(GqlParser::ListValueContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    MkList* l = ALLOC_GEAOBJECT(MkList);
    DEFER(l, ret);

    std::vector<GqlParser::ExpressionContext*> exprCtxs = ctx->expression();
    if (GEAX_RET_FAIL(visitListExpr(exprCtxs, l))) {
        LOG(WARNING) << "Failed to visit list: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlCountAllFunction(GqlParser::GqlCountAllFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    AggFunc* agg = ALLOC_GEAOBJECT(AggFunc);
    DEFER(agg, ret);

    // count(*) -> count(all 1)
    agg->setFuncName(GeneralSetFunction::kCount);
    agg->setDistinct(false);
    VInt* i = ALLOC_GEAOBJECT(VInt);
    i->setVal(1);
    agg->setExpr(i);

    return std::any();
}

std::any GQLAstVisitor::visitGqlCountDistinctFunction(
    GqlParser::GqlCountDistinctFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    MultiCount* count = ALLOC_GEAOBJECT(MultiCount);
    DEFER(count, ret);

    auto exprCtxs = ctx->expression();
    for (auto i = 0u; GEAX_OK(ret) && i < exprCtxs.size(); ++i) {
        Expr* expr = nullptr;
        if (GEAX_IS_NULL(exprCtxs[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "Child of GqlCountDistinctFunction is not set: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtxs[i], expr))) {
            LOG(WARNING) << "Failed to visit expr rule: " << K(ret);
        } else {
            count->appendArg(expr);
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlGeneralSetFunction(GqlParser::GqlGeneralSetFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    AggFunc* agg = ALLOC_GEAOBJECT(AggFunc);
    DEFER(agg, ret);

    GqlParser::GeneralSetFunctionTypeContext* fnCtx = ctx->generalSetFunctionType();
    GqlParser::ExpressionContext* exprCtx = ctx->expression();
    GqlParser::SetQuantifierContext* quantifierCtx = ctx->setQuantifier();
    if (GEAX_RET_FAIL(visitAggExpr(fnCtx, quantifierCtx, exprCtx, agg))) {
        LOG(WARNING) << "Failed to visit AggExpr: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlBinarySetFunction(GqlParser::GqlBinarySetFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    BAggFunc* agg = ALLOC_GEAOBJECT(BAggFunc);
    DEFER(agg, ret);

    GqlParser::BinarySetFunctionTypeContext* typeCtx = nullptr;
    if (GEAX_IS_NULL(typeCtx = ctx->binarySetFunctionType())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "binarySetFunctionType is not set in rule GqlBinarySetFunction: " << K(ret);
    } else if (size_t token = 0; GEAX_RET_FAIL(VISIT_TOKEN(typeCtx, token))) {
        LOG(WARNING) << "Failed to visit token rule binarySetFunctionType: " << K(ret);
    } else {
        switch (token) {
        case GqlParser::PERCENTILE_CONT:
            agg->setFuncName(BinarySetFunction::kPercentileCont);
            break;
        case GqlParser::PERCENTILE_DISC:
            agg->setFuncName(BinarySetFunction::kPercentileDisc);
            break;
        default:
            ret = GEAXErrorCode::GEAX_OOPS;
            LOG(WARNING) << "Invalid token type: " << KV("token", token_->getType()) << K(ret);
            break;
        }
    }

    GqlParser::SetQuantifierContext* quantifierCtx = nullptr;
    bool isDistinct = false;
    if (GEAX_FAIL(ret)) {
    } else if ((quantifierCtx = ctx->setQuantifier()) == nullptr) {
    } else if (auto token = 0; GEAX_RET_FAIL(VISIT_TOKEN(quantifierCtx, token))) {
        LOG(WARNING) << "Failed to visit token SetQuantifier: " << K(ret);
    } else {
        switch (token) {
        case GqlParser::DISTINCT:
            isDistinct = true;
            break;
        case GqlParser::ALL:
            isDistinct = false;
            break;
        default:
            ret = GEAXErrorCode::GEAX_OOPS;
            LOG(WARNING) << "Invalid SetQuantifier token type: " << KV("token", token_->getText())
                         << DK(ret);
        }
    }

    Expr* lhs = nullptr;
    Expr* rhs = nullptr;
    if (GEAX_FAIL(ret)) {
    } else if (GEAX_ANY_NULL(ctx->lhs, ctx->rhs)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "lhs/rhs of rule GqlBinarySetFunction is not set: "
                     << KMP(ctx->lhs, ctx->rhs) << DK(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(ctx->lhs, lhs))) {
        LOG(WARNING) << "Failed to visit left rule expr: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(ctx->rhs, rhs))) {
        LOG(WARNING) << "Failed to visit rhs rule expr: " << K(ret);
    } else {
        agg->setLExpr(isDistinct, lhs);
        agg->setRExpr(rhs);
    }

    return std::any();
}

std::any GQLAstVisitor::visitGqlWindowFunction(GqlParser::GqlWindowFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Windowing* window = ALLOC_GEAOBJECT(Windowing);
    DEFER(window, ret);

    GqlParser::GeneralSetFunctionTypeContext* generalCtx = nullptr;
    GqlParser::WindowFunctionTypeContext* windowCtx = nullptr;
    if (nullptr != (generalCtx = ctx->generalSetFunctionType())) {
        AggFunc* agg = ALLOC_GEAOBJECT(AggFunc);
        if (GEAX_RET_FAIL(visitAggExpr(generalCtx, ctx->setQuantifier(), ctx->expression(), agg))) {
            LOG(WARNING) << "Failed to visit AggExpr: " << K(ret);
        } else {
            window->setExpr(agg);
        }
    } else if (nullptr != (windowCtx = ctx->windowFunctionType())) {
        Function* func = nullptr;
        if (nullptr != ctx->expression() || nullptr != ctx->setQuantifier()) {
            ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
            LOG(WARNING) << "SetQuantifier/Expression is not supported in WindowFunctionType: "
                         << K(ret);
        } else if (GEAX_RET_FAIL(visitFunction(windowCtx, {}))) {
            LOG(WARNING) << "Failed to visit WindowFunction: " << DK(ret);
        } else if (GEAX_UNLIKELY(!checkedCast(childRes_, func))) {
            ret = GEAXErrorCode::GEAX_OOPS;
            LOG(WARNING) << "Child result should be a Function: " << K(ret);
        } else {
            window->setExpr(func);
        }
    } else {
        ret = GEAXErrorCode::GEAX_OOPS;
        LOG(WARNING) << "Invalid windowing type: " << K(ret);
    }
    GqlParser::WindowClauseContext* windowClauseCtx = nullptr;
    if (GEAX_FAIL(ret)) {
    } else if (GEAX_IS_NULL(windowClauseCtx = ctx->windowClause())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "WindowClause is not set in GqlWindowFunction: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_RULE_WITH_FA(windowClauseCtx, window))) {
        LOG(WARNING) << "Failed to visit rule WindowClause: " << K(ret);
    }

    return std::any();
}

std::any GQLAstVisitor::visitWindowClause(GqlParser::WindowClauseContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Windowing* window = nullptr;

    if (GEAX_IS_NULL(window = castAs<Windowing>(faRes_, AstNodeType::kWindowing))) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Failed to cast to node Windowing: " << K(ret);
    } else {
        std::vector<GqlParser::ExpressionAtomContext*> exprCtxs = ctx->expressionAtom();
        for (auto i = 0u; GEAX_OK(ret) && i < exprCtxs.size(); ++i) {
            Expr* expr = nullptr;
            if (GEAX_IS_NULL(exprCtxs[i])) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "Expression is not set in WindowClause: " << K(ret);
            } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtxs[i], expr))) {
                LOG(WARNING) << "Failed to visit expr: " << K(ret);
            } else {
                window->appendPartitionBy(expr);
            }
        }
    }

    GqlParser::OrderByClauseContext* orderByCtx = nullptr;
    GqlParser::SortSpecificationListContext* sortListCtx = nullptr;
    if (GEAX_FAIL(ret)) {
    } else if (nullptr == (orderByCtx = ctx->orderByClause())) {
        // do nothing
    } else if (GEAX_IS_NULL(sortListCtx = orderByCtx->sortSpecificationList())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "SortSpecificationList is not set: " << K(ret);
    } else {
        std::vector<GqlParser::SortSpecificationContext*> sortCtxs =
            sortListCtx->sortSpecification();
        for (auto i = 0u; GEAX_OK(ret) && i < sortCtxs.size(); ++i) {
            GqlParser::SortKeyContext* keyCtx = nullptr;
            GqlParser::OrderingSpecificationContext* orderCtx = nullptr;
            GqlParser::ExpressionContext* exprCtx = nullptr;
            Expr* expr = nullptr;
            bool order = false;
            if (GEAX_IS_NULL(sortCtxs[i])) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "SortSpecification is not set: " << K(ret);
            } else if (GEAX_NOT_NULL(sortCtxs[i]->nullOrdering())) {
                ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
                LOG(WARNING) << "NullOrdering is not supported in WindowClause: " << K(ret);
            } else if (GEAX_IS_NULL(keyCtx = sortCtxs[i]->sortKey())) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "Sortkey is not set in SortSpecification: " << K(ret);
            } else if (GEAX_IS_NULL(exprCtx = keyCtx->expression())) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "Expression is not set in SortKey: " << K(ret);
            } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, expr))) {
                LOG(WARNING) << "Failed to visit sort key expression " << K(ret);
            } else if (nullptr != (orderCtx = sortCtxs[i]->orderingSpecification())) {
                if (nullptr != (orderCtx->ASC()) || nullptr != (orderCtx->ASCENDING())) {
                    order = false;
                } else if (nullptr != (orderCtx->DESC()) || nullptr != (orderCtx->DESCENDING())) {
                    order = true;
                }
            }
            if (GEAX_OK(ret)) {
                window->appendOrderByClause(expr, order);
            }
        }
    }

    return std::any();
}

GEAXErrorCode GQLAstVisitor::visitAggExpr(GqlParser::GeneralSetFunctionTypeContext* typeCtx,
                                          GqlParser::SetQuantifierContext* quantifierCtx,
                                          GqlParser::ExpressionContext* exprCtx, AggFunc* agg) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    if (GEAX_ANY_NULL(typeCtx, exprCtx)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "SetFunctionType/Expression of rule GqlGeneralSetFunction is not set: "
                     << KMP(typeCtx, exprCtx) << DK(ret);
    } else if (size_t token = 0; GEAX_RET_FAIL(VISIT_TOKEN(typeCtx, token))) {
        LOG(WARNING) << "Failed to visit rule roken: " << K(ret);
    } else {
        switch (token) {
        case GqlParser::AVG:
            agg->setFuncName(GeneralSetFunction::kAvg);
            break;
        case GqlParser::COUNT:
            agg->setFuncName(GeneralSetFunction::kCount);
            break;
        case GqlParser::MAX_:
            agg->setFuncName(GeneralSetFunction::kMax);
            break;
        case GqlParser::MIN_:
            agg->setFuncName(GeneralSetFunction::kMin);
            break;
        case GqlParser::SUM:
            agg->setFuncName(GeneralSetFunction::kSum);
            break;
        case GqlParser::COLLECT:
            agg->setFuncName(GeneralSetFunction::kCollect);
            break;
        case GqlParser::STDDEV_SAMP:
            agg->setFuncName(GeneralSetFunction::kStdDevSamp);
            break;
        case GqlParser::STDDEV_POP:
            agg->setFuncName(GeneralSetFunction::kStdDevPop);
            break;
        case GqlParser::GROUP_CONCAT:
            agg->setFuncName(GeneralSetFunction::kGroupConcat);
            break;
        default:
            ret = GEAXErrorCode::GEAX_OOPS;
            LOG(WARNING) << "Invalid token type: " << KV("token", token_->getType()) << K(ret);
            break;
        }
    }
    if (GEAX_FAIL(ret)) {
    } else if (nullptr == quantifierCtx) {
    } else if (size_t token = 0; GEAX_RET_FAIL(VISIT_TOKEN(quantifierCtx, token))) {
        LOG(WARNING) << "Failed to visit token SetQuantifier: " << K(ret);
    } else {
        switch (token) {
        case GqlParser::DISTINCT:
            agg->setDistinct(true);
            break;
        case GqlParser::ALL:
            agg->setDistinct(false);
            break;
        default:
            ret = GEAXErrorCode::GEAX_OOPS;
            LOG(WARNING) << "Invalid SetQuantifier token type: " << KV("token", token_->getText())
                         << DK(ret);
        }
    }

    Expr* expr = nullptr;
    if (GEAX_FAIL(ret)) {
    } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, expr))) {
        LOG(WARNING) << "Failed to visit rule Expression: " << K(ret);
    } else {
        agg->setExpr(expr);
    }

    return ret;
}

std::any GQLAstVisitor::visitGqlDistinctInGeneralFunction(
    GqlParser::GqlDistinctInGeneralFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    AggFunc* agg = ALLOC_GEAOBJECT(AggFunc);
    DEFER(agg, ret);

    std::vector<GqlParser::ExpressionContext*> exprCtxs = ctx->expression();
    if (GEAX_UNLIKELY(exprCtxs.size() < 2)) {
        ret = GEAXErrorCode::GEAX_OOPS;
        LOG(WARNING) << "Num mismatch, expected greater than 2 parameters but actually "
                     << KV("size", exprCtxs.size()) << DK(ret);
    } else if (GEAX_IS_NULL(exprCtxs[0])) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Expression of rule GqlDistinctInGeneralFunction is not set: " << K(ret);
    } else {
        GqlParser::GeneralSetFunctionTypeContext* fnCtx = ctx->generalSetFunctionType();
        GqlParser::ExpressionContext* exprCtx = exprCtxs[0];
        if (GEAX_RET_FAIL(visitAggExpr(fnCtx, nullptr, exprCtx, agg))) {
            LOG(WARNING) << "Failed to visit AggExpr: " << K(ret);
        } else {
            for (auto i = 1u; GEAX_OK(ret) && i < exprCtxs.size(); ++i) {
                Expr* distinctBy = nullptr;
                if (GEAX_IS_NULL(exprCtxs[i])) {
                    ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                    LOG(WARNING) << "Expression of rule GqlDistinctInGeneralFunction is not set: "
                                 << K(ret);
                } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtxs[i], distinctBy))) {
                    LOG(WARNING) << "Failed to visit rule Expression: " << K(ret);
                } else {
                    agg->appendDistinctBy(distinctBy);
                }
            }
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlOneArgScalarFunction(
    GqlParser::GqlOneArgScalarFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);
    if (GEAX_RET_FAIL(
            visitFunction(ctx->oneArgNumericFunctionName(), {ctx->functionParameter()}))) {
        LOG(WARNING) << "Failed to visit rule GqlOneArgScalarFunction: " << DK(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlTwoArgScalarFunction(
    GqlParser::GqlTwoArgScalarFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);
    if (GEAX_RET_FAIL(visitFunction(ctx->twoArgNumericFunctionName(), ctx->functionParameter()))) {
        LOG(WARNING) << "Failed to visit rule GqlTwoArgScalarFunction: " << DK(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlIfFunction(GqlParser::GqlIfFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    If* ifExpr = ALLOC_GEAOBJECT(If);
    DEFER(ifExpr, ret);

    auto params = ctx->functionParameter();
    Expr* cond = nullptr;
    Expr* trueBody = nullptr;
    Expr* falseBody = nullptr;
    if (GEAX_UNLIKELY(params.size() != 3)) {
        ret = GEAXErrorCode::GEAX_OOPS;
        LOG(WARNING) << "Num mismatch, expected three parameters but actually "
                     << KV("size", params.size()) << DK(ret);
    } else if (GEAX_ANY_NULL(params[0], params[1], params[2])) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "rule FunctionParameter is not set: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(params[0], cond))) {
        LOG(WARNING) << "Failed to visit rule condition: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(params[1], trueBody))) {
        LOG(WARNING) << "Failed to visit rule trueBody: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(params[2], falseBody))) {
        LOG(WARNING) << "Failed to visit rule falseBody: " << K(ret);
    } else {
        ifExpr->setCondition(cond);
        ifExpr->setTrueBody(trueBody);
        ifExpr->setFalseBody(falseBody);
    }

    return std::any();
}

std::any GQLAstVisitor::visitGeneralFunction(GqlParser::GeneralFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);
    if (GEAX_RET_FAIL(visitFunction(ctx->functionName(), ctx->functionParameter()))) {
        LOG(WARNING) << "Failed to visit rule GeneralFunction: " << K(ret);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlListTrimFunction(GqlParser::GqlListTrimFunctionContext* ctx) {
    childRet_ = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    return std::any();
}
std::any GQLAstVisitor::visitGqlElementsOfPathFunction(
    GqlParser::GqlElementsOfPathFunctionContext* ctx) {
    childRet_ = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    return std::any();
}

GEAXErrorCode GQLAstVisitor::visitFunction(
    antlr4::ParserRuleContext* funcName,
    const std::vector<GqlParser::FunctionParameterContext*>& params) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Function* func = ALLOC_GEAOBJECT(Function);
    DEFER(func, ret);
    if (GEAX_IS_NULL(funcName)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "funcName is not set: " << K(ret);
    } else {
        // TODO(ljr) : g4 consider function:stringLiteral L_PAREN parameters R_PAREN;
        func->setName(funcName->getText());
        for (auto i = 0u; GEAX_OK(ret) && i < params.size(); ++i) {
            Expr* expr = nullptr;
            if (GEAX_IS_NULL(params[i])) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "FunctionParameter is not set: " << K(ret);
            } else if (GEAX_RET_FAIL(VISIT_EXPR(params[i], expr))) {
                LOG(WARNING) << "Failed to visit rule FunctionParameter: " << K(ret);
            } else {
                func->appendArg(expr);
            }
        }
    }
    return ret;
}

std::any GQLAstVisitor::visitGqlSubstringFunction(GqlParser::GqlSubstringFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Function* func = ALLOC_GEAOBJECT(Function);
    DEFER(func, ret);

    Expr* strExpr = nullptr;
    Expr* startExpr = nullptr;
    if (GEAX_ANY_NULL(ctx->str, ctx->startPos)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "str/startPos is not set in rule GqlSubstringFunction: "
                     << KMP(ctx->str, ctx->startPos) << DK(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(ctx->str, strExpr))) {
        LOG(WARNING) << "Failed to visit str expr: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(ctx->startPos, startExpr))) {
        LOG(WARNING) << "Failed to visit startPos expr: " << K(ret);
    } else {
        func->setName(ctx->SUBSTR()->getText());
        func->appendArg(strExpr);
        func->appendArg(startExpr);
        if (ctx->len != nullptr) {
            Expr* lengthExpr = nullptr;
            if (GEAX_RET_FAIL(VISIT_EXPR(ctx->len, lengthExpr))) {
                LOG(WARNING) << "Failed to visit length expr: " << K(ret);
            } else {
                func->appendArg(lengthExpr);
            }
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlFoldStringFunction(GqlParser::GqlFoldStringFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Function* func = ALLOC_GEAOBJECT(Function);
    DEFER(func, ret);

    Expr* strExpr = nullptr;
    GqlParser::ExpressionAtomContext* exprCtx = nullptr;
    if (GEAX_IS_NULL(exprCtx = ctx->expressionAtom())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "ExpressionAtom is not set in rule GqlFoldStringFunction: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, strExpr))) {
        LOG(WARNING) << "Failed to visit ExpressionAtom: " << K(ret);
    } else if (GEAX_IS_NULL(ctx->dir)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Dir is not set in rule GqlFoldStringFunction: " << K(ret);
    } else {
        func->setName(ctx->dir->getText());
        func->appendArg(strExpr);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlSingleTrimStringFunction(
    GqlParser::GqlSingleTrimStringFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Function* func = ALLOC_GEAOBJECT(Function);
    DEFER(func, ret);

    Expr* trimExpr = nullptr;
    std::vector<GqlParser::ExpressionAtomContext*> exprCtxs = ctx->expressionAtom();
    if (nullptr != ctx->FROM() || nullptr != ctx->trimSpecification() || exprCtxs.size() != 1) {
        ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
        LOG(WARNING) << "TrimSpecification is not supported yet: " << K(ret);
    } else if (GEAX_IS_NULL(ctx->trimSrc)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "trimSrc is not set in rule GqlTrimStringFunction: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(ctx->trimSrc, trimExpr))) {
        LOG(WARNING) << "Failed to visit trimSrc: " << K(ret);
    } else {
        func->setName(ctx->TRIM()->getText());
        func->appendArg(trimExpr);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlMultiTrimStringFunction(
    GqlParser::GqlMultiTrimStringFunctionContext* ctx) {
    childRet_ = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    return std::any();
}

// TODO(boyao.zby) we only support n.a currently, more complicated cases should be supported.
std::any GQLAstVisitor::visitPropertyReference(GqlParser::PropertyReferenceContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    GetField* getField = ALLOC_GEAOBJECT(GetField);
    DEFER(getField, ret);

    Ref* ref = nullptr;
    auto varCtx = ctx->variable();
    auto propCtx = ctx->propertyName();
    if (GEAX_ANY_NULL(varCtx, propCtx)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Child of rule PropertyReference is not set: " << KMP(varCtx, propCtx)
                     << DK(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(varCtx, ref))) {
        LOG(WARNING) << "Failed to visit expr rule expressionAtom: " << K(ret);
    } else {
        getField->setFieldName(trimAccentGrave(propCtx->getText()));
        getField->setExpr(ref);
    }

    return std::any();
}

std::any GQLAstVisitor::visitGqlNullIfCaseFunction(GqlParser::GqlNullIfCaseFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Function* func = ALLOC_GEAOBJECT(Function);
    DEFER(func, ret);

    Expr* lhs = nullptr;
    Expr* rhs = nullptr;
    if (GEAX_ANY_NULL(ctx->lhs, ctx->rhs)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Children of GqlNullIfExpression is not set: " << KMP(ctx->lhs, ctx->rhs)
                     << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(ctx->lhs, lhs))) {
        LOG(WARNING) << "Failed to visit left child expression: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(ctx->rhs, rhs))) {
        LOG(WARNING) << "Failed to visit right child expression: " << K(ret);
    } else {
        func->setName(ctx->NULLIF()->getText());
        func->appendArg(lhs);
        func->appendArg(rhs);
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlCoalesceCaseFunction(
    GqlParser::GqlCoalesceCaseFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Function* func = ALLOC_GEAOBJECT(Function);
    DEFER(func, ret);

    func->setName(ctx->COALESCE()->getText());
    std::vector<GqlParser::ExpressionContext*> exprCtxs = ctx->expression();
    for (auto i = 0u; i < exprCtxs.size() && GEAX_OK(ret); ++i) {
        Expr* expr = nullptr;
        if (GEAX_IS_NULL(exprCtxs[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "Expression is not set in rule GqlCoalesceExpression: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtxs[i], expr))) {
            LOG(WARNING) << "Failed to visit expr: " << K(ret);
        } else {
            func->appendArg(expr);
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlSimpleCaseFunction(GqlParser::GqlSimpleCaseFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Case* caseNode = ALLOC_GEAOBJECT(Case);
    DEFER(caseNode, ret);

    Expr* condNode = nullptr;
    GqlParser::ExpressionAtomContext* exprCtx = nullptr;
    GqlParser::ElseClauseContext* elseCtx = nullptr;
    if (GEAX_IS_NULL(exprCtx = ctx->expressionAtom())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Expression of GqlSimpleCaseFunction is not set: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, condNode))) {
        LOG(WARNING) << "Failed to visit expr of rule GqlSimpleCaseFunction: " << K(ret);
    } else {
        caseNode->setInput(condNode);
    }
    if (GEAX_OK(ret) && nullptr != (elseCtx = ctx->elseClause())) {
        Expr* elseNode = nullptr;
        if (GEAX_RET_FAIL(VISIT_EXPR(elseCtx, elseNode))) {
            LOG(WARNING) << "Failed to visit else expr of rule GqlSimpleCaseFunction: " << K(ret);
        } else {
            caseNode->setElseBody(elseNode);
        }
    }
    if (GEAX_OK(ret)) {
        auto whenClauses = ctx->simpleWhenClause();
        for (auto i = 0u; i < whenClauses.size() && GEAX_OK(ret); ++i) {
            if (GEAX_IS_NULL(whenClauses[i])) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "SimpleWhenClause is not set: " << K(ret);
                break;
            }
            std::vector<GqlParser::WhenOperandContext*> whenOps = whenClauses[i]->whenOperand();
            GqlParser::ExpressionContext* thenExpr = nullptr;
            Expr* thenNode = nullptr;
            // Treat <WHEN whenOperand (Comma whenOperand)* THEN expression>
            // as multiple pairs of IFCase.
            if (GEAX_IS_NULL(thenExpr = whenClauses[i]->expression())) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "ThenExpr of SimpleWhenClause is not set: " << K(ret);
            } else if (GEAX_RET_FAIL(VISIT_EXPR(thenExpr, thenNode))) {
                LOG(WARNING) << "Failed to visit then expr of rule SimpleWhenClause: " << K(ret);
            }
            for (auto j = 0u; j < whenOps.size() && GEAX_OK(ret); ++j) {
                GqlParser::WhenOperandContext* whenOp = whenOps[j];
                GqlParser::ExpressionAtomContext* whenExpr = nullptr;
                Expr* whenNode = nullptr;
                if (GEAX_IS_NULL(whenOp)) {
                    ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                    LOG(WARNING) << "WhenOperand is not set: " << K(ret);
                } else if (GEAX_UNLIKELY_NE(whenOp->children.size(), 1)) {
                    ret = GEAXErrorCode::GEAX_OOPS;
                    LOG(WARNING) << "Num mismatch, expected 1 parameters but actually "
                                 << KV("size", whenOp->children.size()) << DK(ret);
                } else if (!checkedCast(whenOp->children[0], whenExpr)) {
                    // Other: Comparision/null/directed/labeled/source/destination predicate
                    // The old gql case expr does not support these types, so here is unsupported.
                    ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
                    LOG(WARNING) << "Unsupported WhenOperand: "
                                 << KV("content", whenOp->children[0]->getText()) << DK(ret);
                } else if (GEAX_IS_NULL(whenExpr)) {
                    ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                    LOG(WARNING) << "WhenExpr of SimpleWhenClause is not set: " << K(ret);
                } else if (GEAX_RET_FAIL(VISIT_EXPR(whenExpr, whenNode))) {
                    LOG(WARNING) << "Failed to visit when expr of rule SimpleWhenClause: "
                                 << K(ret);
                } else {
                    caseNode->appendCaseBody(whenNode, thenNode);
                }
            }
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitGqlSearchedCaseFunction(
    GqlParser::GqlSearchedCaseFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Case* caseNode = ALLOC_GEAOBJECT(Case);
    DEFER(caseNode, ret);

    GqlParser::ElseClauseContext* elseCtx = nullptr;
    if (nullptr != (elseCtx = ctx->elseClause())) {
        Expr* elseNode{nullptr};
        if (GEAX_RET_FAIL(VISIT_EXPR(elseCtx, elseNode))) {
            LOG(WARNING) << "Failed to visit else expr of rule GqlSearchedCaseFunction: " << K(ret);
        } else {
            caseNode->setElseBody(elseNode);
        }
    }
    if (GEAX_OK(ret)) {
        GqlParser::ExpressionContext* whenExpr{nullptr};
        GqlParser::ExpressionContext* thenExpr{nullptr};
        Expr* whenNode{nullptr};
        Expr* thenNode{nullptr};
        std::vector<GqlParser::SearchedWhenClauseContext*> whenExprs = ctx->searchedWhenClause();
        for (auto i = 0u; i < whenExprs.size() && GEAX_OK(ret); ++i) {
            if (GEAX_IS_NULL(whenExprs[i])) {
                ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                LOG(WARNING) << "SearchedWhenClause is not set: " << K(ret);
                break;
            } else if (GEAX_UNLIKELY(whenExprs[i]->expression().size() != 2)) {
                ret = GEAXErrorCode::GEAX_OOPS;
                LOG(WARNING) << "Expr of rule SearchedWhenClause should be 2: " << K(ret);
            } else {
                whenExpr = whenExprs[i]->expression(0);
                thenExpr = whenExprs[i]->expression(1);
                if (GEAX_ANY_NULL(whenExpr, thenExpr)) {
                    ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
                    LOG(WARNING) << "Children of SearchedWhenClause is not set: "
                                 << KMP(whenExpr, thenExpr) << DK(ret);
                } else if (GEAX_RET_FAIL(VISIT_EXPR(whenExpr, whenNode))) {
                    LOG(WARNING) << "Failed to visit when expr of rule SearchedWhenClause: "
                                 << K(ret);
                } else if (GEAX_RET_FAIL(VISIT_EXPR(thenExpr, thenNode))) {
                    LOG(WARNING) << "Failed to visit then expr of rule SearchedWhenClause: "
                                 << K(ret);
                } else {
                    caseNode->appendCaseBody(whenNode, thenNode);
                }
            }
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitCastFunction(GqlParser::CastFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Cast* castNode = ALLOC_GEAOBJECT(Cast);
    DEFER(castNode, ret);

    Expr* expr = nullptr;
    auto exprCtx = ctx->expression();
    auto typeCtx = ctx->valueType();
    if (GEAX_ANY_NULL(exprCtx, typeCtx)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "expr/type is not set in rule castFunction: " << KMP(exprCtx, typeCtx)
                     << DK(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtx, expr))) {
        LOG(WARNING) << "Failed to visit expr: " << K(ret);
    } else if (GEAX_RET_FAIL(byPass(typeCtx))) {
        LOG(WARNING) << "Failed to visit children of rule PredefinedType: " << K(ret);
    } else if (GEAX_RET_FAIL(checkTokenResult())) {
        LOG(WARNING) << "Failed to check token result: " << K(ret);
    } else {
        castNode->setCastType(token_->getText());
        castNode->setExpr(expr);
    }
    return std::any();
}

std::any GQLAstVisitor::visitElementFunction(GqlParser::ElementFunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    Ref* var = nullptr;
    auto funcName = ctx->elementFunctionName();
    auto varCtx = ctx->variable();
    if (GEAX_ANY_NULL(funcName, varCtx)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "funcName/variable is not set in rule ElementFunction: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(varCtx, var))) {
        LOG(WARNING) << "Failed to visit expr Variable: " << K(ret);
    } else if (size_t token = 0; GEAX_RET_FAIL(VISIT_TOKEN(funcName, token))) {
        LOG(WARNING) << "Failed to visit token: " << K(ret);
    } else {
        switch (token) {
        case GqlParser::ID:
        case GqlParser::TIMESTAMP: {
            // TODO(jingru.ljr): consider id(some functions that return a vertex)
            GetField* getField = ALLOC_GEAOBJECT(GetField);
            DEFER(getField, ret);
            getField->setExpr(var);
            getField->setFieldName("@" + token_->getText());
            break;
        }
        case GqlParser::ELEMENT_ID:
            ret = GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
            LOG(WARNING) << "Unsupported function: " << KV("token", token_->getText()) << DK(ret);
            break;
        default:
            ret = GEAXErrorCode::GEAX_OOPS;
            LOG(WARNING) << "Invalid token value: " << KV("token", token_->getText()) << DK(ret);
            break;
        }
    }
    return std::any();
}

std::any GQLAstVisitor::visitQueryConjunction(GqlParser::QueryConjunctionContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    QueryConjunctionType* type = nullptr;
    DEFER(type, ret);

    if (nullptr != ctx->OTHERWISE()) {
        type = ALLOC_GEAOBJECT(OtherWise);
    } else if (GEAX_IS_NULL(ctx->setOperator())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "SetOperator is not set in QueryConjunction: " << K(ret);
    } else if (GEAX_RET_FAIL(ACCEPT_RULE(ctx->setOperator()))) {
        LOG(WARNING) << "Failed to visit set operator " << K(ret);
    } else if (GEAX_UNLIKELY(!checkedCast(childRes_, type))) {
        ret = GEAXErrorCode::GEAX_OOPS;
        LOG(WARNING) << "Child result should be a QueryConjunctionType: " << K(ret);
    }

    return std::any();
}

std::any GQLAstVisitor::visitSetOperator(GqlParser::SetOperatorContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    DEFER_RET(ret);

    GqlParser::SetQuantifierContext* setQuantifierCtx = ctx->setQuantifier();
    if (ctx->UNION() != nullptr) {
        Union* type = ALLOC_GEAOBJECT(Union);
        if (setQuantifierCtx != nullptr) {
            if (setQuantifierCtx->DISTINCT()) {
                type->setDistinct(true);
            } else if (setQuantifierCtx->ALL()) {
                type->setDistinct(false);
            }
        }
        childRes_ = type;
    } else if (ctx->EXCEPT() != nullptr) {
        Except* type = ALLOC_GEAOBJECT(Except);
        if (setQuantifierCtx != nullptr) {
            if (setQuantifierCtx->DISTINCT()) {
                type->setDistinct(true);
            } else if (setQuantifierCtx->ALL()) {
                type->setDistinct(false);
            }
        }
        childRes_ = type;
    } else if (ctx->INTERSECT() != nullptr) {
        Intersect* type = ALLOC_GEAOBJECT(Intersect);
        if (setQuantifierCtx != nullptr) {
            if (setQuantifierCtx->DISTINCT()) {
                type->setDistinct(true);
            } else if (setQuantifierCtx->ALL()) {
                type->setDistinct(false);
            }
        }
        childRes_ = type;
    }

    return std::any();
}

std::any GQLAstVisitor::visitGqlPropertyReference(GqlParser::GqlPropertyReferenceContext* ctx) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    GetField* getField = ALLOC_GEAOBJECT(GetField);
    DEFER(getField, ret);

    // n.name
    // (n).name
    Expr* ref = nullptr;
    GqlParser::ExpressionAtomContext* atomCtx = nullptr;
    GqlParser::PropertyNameContext* propCtx = nullptr;
    if (GEAX_IS_NULL(atomCtx = ctx->expressionAtom())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "ExpressionAtom is not set in GqlPropertyReference: " << K(ret);
    } else if (GEAX_IS_NULL(propCtx = ctx->propertyName())) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "PropertyName is not set in GqlPropertyReference: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(atomCtx, ref))) {
        LOG(WARNING) << "Failed to visit expr rule ExpressionAtom: " << K(ret);
    } else {
        getField->setFieldName(trimAccentGrave(propCtx->getText()));
        getField->setExpr(ref);
    }

    return std::any();
}

GEAXErrorCode GQLAstVisitor::visitBinaryExpr(antlr4::ParserRuleContext* lhs,
                                             antlr4::ParserRuleContext* rhs, BinaryOp* fa) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    Expr* lexpr = nullptr;
    Expr* rexpr = nullptr;
    if (GEAX_ANY_NULL(lhs, rhs, fa)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Children is not set for binary expression: " << KMP(lhs, rhs, fa)
                     << DK(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(lhs, lexpr))) {
        LOG(WARNING) << "Failed to visit left expression: " << K(ret);
    } else if (GEAX_RET_FAIL(VISIT_EXPR(rhs, rexpr))) {
        LOG(WARNING) << "Failed to visit right expression: " << K(ret);
    } else {
        fa->setLeft(lexpr);
        fa->setRight(rexpr);
    }
    return ret;
}

GEAXErrorCode GQLAstVisitor::visitListExpr(std::vector<GqlParser::ExpressionContext*>& exprCtxs,
                                           MkList* list) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    for (auto i = 0u; GEAX_OK(ret) && i < exprCtxs.size(); ++i) {
        Expr* expr = nullptr;
        if (GEAX_IS_NULL(exprCtxs[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "Expression is not set for rule ListConstructor/ListValue: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_EXPR(exprCtxs[i], expr))) {
            LOG(WARNING) << "Failed to visit expr rule: " << K(ret);
        } else {
            list->appendElem(expr);
        }
    }
    return ret;
}

GEAXErrorCode GQLAstVisitor::visitListLiteralItem(
    std::vector<GqlParser::GeneralLiteralContext*>& literalCtxs, MkList* list) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    for (auto i = 0u; GEAX_OK(ret) && i < literalCtxs.size(); ++i) {
        Expr* literal = nullptr;
        if (GEAX_IS_NULL(literalCtxs[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "Literal is not set for rule ListLiteral: " << K(ret);
        } else if (GEAX_RET_FAIL(VISIT_EXPR(literalCtxs[i], literal))) {
            LOG(WARNING) << "Failed to visit literal rule: " << K(ret);
        } else {
            list->appendElem(literal);
        }
    }
    return ret;
}

GEAXErrorCode GQLAstVisitor::byPass(antlr4::tree::ParseTree* node) {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    for (auto i = 0u; i < node->children.size() && GEAX_OK(ret); ++i) {
        if (GEAX_IS_NULL(node->children[i])) {
            ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
            LOG(WARNING) << "Child of rule is nullptr: " << K(ret);
        } else {
            node->children[i]->accept(this);
            if (GEAX_RET_FAIL(childRet_)) {
                LOG(WARNING) << "Failed to visit rule: " << K(ret);
            }
        }
    }
    return ret;
}

std::any GQLAstVisitor::visitChildren(antlr4::tree::ParseTree* node) {
    childRet_ = byPass(node);
    return std::any();
}

GEAXErrorCode GQLAstVisitor::checkChildResult() const {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    if (GEAX_RET_FAIL(childRet_)) {
        LOG(WARNING) << "Failed to visit child: " << K(ret);
    } else if (GEAX_IS_NULL(childRes_)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Child result is nullptr: " << K(ret);
    }
    return ret;
}

GEAXErrorCode GQLAstVisitor::checkChildRet() const {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    if (GEAX_RET_FAIL(childRet_)) {
        LOG(WARNING) << "Failed to visit child: " << K(ret);
    }
    return ret;
}

GEAXErrorCode GQLAstVisitor::checkTokenResult() const {
    auto ret = GEAXErrorCode::GEAX_SUCCEED;
    if (GEAX_RET_FAIL(childRet_)) {
        LOG(WARNING) << "Failed to visit token: " << K(ret);
    } else if (GEAX_IS_NULL(token_)) {
        ret = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Token result is nullptr: " << K(ret);
    }
    return ret;
}

std::any GQLAstVisitor::visitTerminal(antlr4::tree::TerminalNode* node) {
    if (GEAX_IS_NULL(token_ = node->getSymbol())) {
        childRet_ = GEAXErrorCode::GEAX_COMMON_NULLPTR;
        LOG(WARNING) << "Token is not set: " << K(childRet_);
    }
    return std::any();
}

std::string GQLAstVisitor::getFullText(antlr4::ParserRuleContext* ruleCtx) const {
    if (ruleCtx->children.size() == 0) {
        return "";
    }
    auto* startToken = ruleCtx->getStart();
    auto* stopToken = ruleCtx->getStop();
    antlr4::misc::Interval interval(startToken->getStartIndex(), stopToken->getStopIndex());
    return ruleCtx->getStart()->getInputStream()->getText(interval);
}

std::string GQLAstVisitor::escapeText(const std::string& text) const {
    static const std::unordered_map<char, char> kSpChars = {
        {'\'', '\''}, {'"', '"'},  {'\\', '\\'}, {'t', '\t'}, {'b', '\b'}, {'n', '\n'},
        {'r', '\r'},  {'f', '\f'}, {'`', '`'},   {'0', '\0'}
        // TODO(suosi): \Unnnn, \Unnnnnn
    };
    static const char kEscapeChar = '\\';
    std::string buff;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == kEscapeChar) {
            if (++i < text.size()) {
                if (auto it = kSpChars.find(text[i]); it != kSpChars.end()) {
                    buff.push_back(it->second);
                } else {
                    buff.push_back(text[i]);
                }
            }
        } else {
            buff.push_back(text[i]);
        }
    }
    return buff;
}

template <typename T>
T* GQLAstVisitor::castAs(AstNode* node, AstNodeType type) {
    static_assert(std::is_base_of_v<AstNode, T>,
                  "Cast type `T` must be a derived class of AstNode");

    T* ret = nullptr;
    if (GEAX_NOT_NULL(node) && GEAX_LIKELY(node->type() == type)) {
        ret = static_cast<T*>(node);
    }
    return ret;
}
}  // namespace frontend
}  // namespace geax
