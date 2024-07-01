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

#ifndef FRONTEND_ISOGQL_GQLASTVISITOR_H_
#define FRONTEND_ISOGQL_GQLASTVISITOR_H_

#include <type_traits>

#include "antlr4-runtime.h"
#include "geax-front-end/GEAXErrorCode.h"
#include "geax-front-end/ast/Ast.h"
#include "geax-front-end/isogql/GQLResolveCtx.h"
#include "geax-front-end/isogql/parser/GqlParserBaseVisitor.h"
#include "parser/GqlParser.h"

namespace geax {
namespace frontend {

/**
 * A GQLAstVisitor visits the parse tree of a ISO/GQL query, to generate a responding IR.
 */
class GQLAstVisitor : public parser::GqlParserBaseVisitor {
public:
    explicit GQLAstVisitor(GQLResolveCtx &ctx)
        : ctx_(ctx),
          childRes_(nullptr),
          childRet_(GEAXErrorCode::GEAX_SUCCEED),
          faRes_(nullptr),
          token_(nullptr) {}

    AstNode *result() const { return childRes_; }
    GEAXErrorCode error() const { return childRet_; }

private:
    using parser::GqlParserBaseVisitor::visitFullEdgePattern;
    using parser::GqlParserBaseVisitor::visitInsertEdgePattern;
    using parser::GqlParserBaseVisitor::visitPathMode;

    // statement
    std::any visitGqlRequest(parser::GqlParser::GqlRequestContext *ctx) override;
    std::any visitSessionActivity(parser::GqlParser::SessionActivityContext *ctx) override;
    std::any visitExplainActivity(parser::GqlParser::ExplainActivityContext *ctx) override;
    std::any visitGqlFullTransaction(parser::GqlParser::GqlFullTransactionContext *ctx) override;
    std::any visitGqlNormalTransaction(
        parser::GqlParser::GqlNormalTransactionContext *ctx) override;
    std::any visitGqlEndTransaction(parser::GqlParser::GqlEndTransactionContext *ctx) override;
    std::any visitStartTransactionCommand(
        parser::GqlParser::StartTransactionCommandContext *ctx) override;
    std::any visitSessionSetCommand(parser::GqlParser::SessionSetCommandContext *ctx) override;
    std::any visitSessionResetCommand(parser::GqlParser::SessionResetCommandContext *ctx) override;
    std::any visitSessionSetValueParameterClause(
        parser::GqlParser::SessionSetValueParameterClauseContext *ctx) override;
    std::any visitSessionSetGraphParameterClause(
        parser::GqlParser::SessionSetGraphParameterClauseContext *ctx) override;
    std::any visitSessionSetBindingTableParameterClause(
        parser::GqlParser::SessionSetBindingTableParameterClauseContext *ctx) override;
    std::any visitProcedureBody(parser::GqlParser::ProcedureBodyContext *ctx) override;
    std::any visitBindingVariableDefinitionBlock(
        parser::GqlParser::BindingVariableDefinitionBlockContext *ctx) override;
    std::any visitGraphVariableDefinition(
        parser::GqlParser::GraphVariableDefinitionContext *ctx) override;
    std::any visitBindingTableVariableDefinition(
        parser::GqlParser::BindingTableVariableDefinitionContext *ctx) override;
    std::any visitValueVariableDefinition(
        parser::GqlParser::ValueVariableDefinitionContext *ctx) override;
    std::any visitNestedQuerySpecification(
        parser::GqlParser::NestedQuerySpecificationContext *ctx) override;
    std::any visitObjectExpressionPrimary(
        parser::GqlParser::ObjectExpressionPrimaryContext *ctx) override;
    std::any visitBindingTableReference(
        parser::GqlParser::BindingTableReferenceContext *ctx) override;
    std::any visitObjectNameOrBindingVariable(
        parser::GqlParser::ObjectNameOrBindingVariableContext *ctx) override;
    std::any visitStatementBlock(parser::GqlParser::StatementBlockContext *ctx) override;
    std::any visitNextStatement(parser::GqlParser::NextStatementContext *ctx) override;
    std::any visitManagerStatement(parser::GqlParser::ManagerStatementContext *ctx) override;
    std::any visitLinearCatalogModifyingStatement(
        parser::GqlParser::LinearCatalogModifyingStatementContext *ctx) override;
    std::any visitQueryStatement(parser::GqlParser::QueryStatementContext *ctx) override;
    std::any visitStandaloneCallStatement(
        parser::GqlParser::StandaloneCallStatementContext *ctx) override;
    std::any visitJoinQueryExpression(parser::GqlParser::JoinQueryExpressionContext *ctx) override;
    std::any visitCompositeQueryExpression(
        parser::GqlParser::CompositeQueryExpressionContext *ctx) override;
    std::any visitFocusedPrimitiveResultStatement(
        parser::GqlParser::FocusedPrimitiveResultStatementContext *ctx) override;
    std::any visitFocusedQueryStatement(
        parser::GqlParser::FocusedQueryStatementContext *ctx) override;
    std::any visitAmbientLinearQueryStatement(
        parser::GqlParser::AmbientLinearQueryStatementContext *ctx) override;
    std::any visitPrimitiveResultStatement(
        parser::GqlParser::PrimitiveResultStatementContext *ctx) override;
    std::any visitAmbientLinearDataModifyingStatementBody(
        parser::GqlParser::AmbientLinearDataModifyingStatementBodyContext *ctx) override;

    std::any visitInsertStatement(parser::GqlParser::InsertStatementContext *ctx) override;
    std::any visitSetStatement(parser::GqlParser::SetStatementContext *ctx) override;
    std::any visitSetPropertyItem(parser::GqlParser::SetPropertyItemContext *ctx) override;
    std::any visitSetAllPropertiesItem(
        parser::GqlParser::SetAllPropertiesItemContext *ctx) override;
    std::any visitUpdatePropertiesItem(
        parser::GqlParser::UpdatePropertiesItemContext *ctx) override;
    std::any visitSetLabelItem(parser::GqlParser::SetLabelItemContext *ctx) override;
    std::any visitDeleteStatement(parser::GqlParser::DeleteStatementContext *ctx) override;
    std::any visitReplaceStatement(parser::GqlParser::ReplaceStatementContext *ctx) override;
    std::any visitMergeStatement(parser::GqlParser::MergeStatementContext *ctx) override;
    std::any visitGqlMergeOnMatch(parser::GqlParser::GqlMergeOnMatchContext *ctx) override;
    std::any visitGqlMergeOnCreate(parser::GqlParser::GqlMergeOnCreateContext *ctx) override;

    std::any visitFilterStatement(parser::GqlParser::FilterStatementContext *ctx) override;
    std::any visitSimpleMatchStatement(
        parser::GqlParser::SimpleMatchStatementContext *ctx) override;
    std::any visitOptionalMatchStatement(
        parser::GqlParser::OptionalMatchStatementContext *ctx) override;
    std::any visitMatchStatementBlock(parser::GqlParser::MatchStatementBlockContext *ctx) override;
    std::any visitForStatement(parser::GqlParser::ForStatementContext *ctx) override;

    std::any visitReturnStatementBody(parser::GqlParser::ReturnStatementBodyContext *ctx) override;
    std::any visitReturnItemList(parser::GqlParser::ReturnItemListContext *ctx) override;
    std::any visitReturnItem(parser::GqlParser::ReturnItemContext *ctx) override;
    std::any visitBindingVariable(parser::GqlParser::BindingVariableContext *ctx) override;
    std::any visitParameter(parser::GqlParser::ParameterContext *ctx) override;
    std::any visitHintItemlist(parser::GqlParser::HintItemlistContext *ctx) override;
    std::any visitGqlReadConsistency(parser::GqlParser::GqlReadConsistencyContext *ctx) override;
    std::any visitGqlAllowAnonymousTable(
        parser::GqlParser::GqlAllowAnonymousTableContext *ctx) override;
    std::any visitGqlEdgeOnJoin(parser::GqlParser::GqlEdgeOnJoinContext *ctx) override;
    std::any visitSelectStatement(parser::GqlParser::SelectStatementContext *ctx) override;
    std::any visitSelectItemList(parser::GqlParser::SelectItemListContext *ctx) override;
    std::any visitSelectItem(parser::GqlParser::SelectItemContext *ctx) override;
    std::any visitSelectStatementBody(parser::GqlParser::SelectStatementBodyContext *ctx) override;

    // orderByAndPageStatement
    std::any visitOrderByAndPageStatement(
        parser::GqlParser::OrderByAndPageStatementContext *ctx) override;
    std::any visitSortSpecification(parser::GqlParser::SortSpecificationContext *ctx) override;

    // call procedure
    std::any visitCallQueryStatement(parser::GqlParser::CallQueryStatementContext *ctx) override;
    std::any visitCallProcedureStatement(
        parser::GqlParser::CallProcedureStatementContext *ctx) override;
    std::any visitInlineProcedureCall(parser::GqlParser::InlineProcedureCallContext *ctx) override;
    std::any visitNamedProcedureCall(parser::GqlParser::NamedProcedureCallContext *ctx) override;

    // clause
    std::any visitGraphPattern(parser::GqlParser::GraphPatternContext *ctx) override;
    std::any visitPathPatternList(parser::GqlParser::PathPatternListContext *ctx) override;
    std::any visitPathPattern(parser::GqlParser::PathPatternContext *ctx) override;
    std::any visitPathTerm(parser::GqlParser::PathTermContext *ctx) override;
    std::any visitPathPatternUnion(parser::GqlParser::PathPatternUnionContext *ctx) override;
    std::any visitPathMultisetAlternation(
        parser::GqlParser::PathMultisetAlternationContext *ctx) override;
    std::any visitRepeatableElementsMatchMode(
        parser::GqlParser::RepeatableElementsMatchModeContext *ctx) override;
    std::any visitDifferentEdgesMatchMode(
        parser::GqlParser::DifferentEdgesMatchModeContext *ctx) override;
    std::any visitYieldItemList(parser::GqlParser::YieldItemListContext *ctx) override;
    std::any visitYieldItem(parser::GqlParser::YieldItemContext *ctx) override;
    std::any visitPathModePrefix(parser::GqlParser::PathModePrefixContext *ctx) override;
    std::any visitAllPathSearch(parser::GqlParser::AllPathSearchContext *ctx) override;
    std::any visitAnyPathSearch(parser::GqlParser::AnyPathSearchContext *ctx) override;
    std::any visitAllShortestPathSearch(
        parser::GqlParser::AllShortestPathSearchContext *ctx) override;
    std::any visitAnyShortestPathSearch(
        parser::GqlParser::AnyShortestPathSearchContext *ctx) override;
    std::any visitCountedShortestPathSearch(
        parser::GqlParser::CountedShortestPathSearchContext *ctx) override;
    std::any visitCountedShortestGroupSearch(
        parser::GqlParser::CountedShortestGroupSearchContext *ctx) override;
    GEAXErrorCode visitPathMode(parser::GqlParser::PathModeContext *ctx, ModeType &modeType);
    std::any visitQuantifiedPathPrimary(
        parser::GqlParser::QuantifiedPathPrimaryContext *ctx) override;
    std::any visitQuestionedPathPrimary(
        parser::GqlParser::QuestionedPathPrimaryContext *ctx) override;
    std::any visitGqlGraphPatternAsteriskQuantifier(
        parser::GqlParser::GqlGraphPatternAsteriskQuantifierContext *ctx) override;
    std::any visitGqlGraphPatternPlusSignQuantifier(
        parser::GqlParser::GqlGraphPatternPlusSignQuantifierContext *ctx) override;
    std::any visitFixedQuantifier(parser::GqlParser::FixedQuantifierContext *ctx) override;
    std::any visitGeneralQuantifier(parser::GqlParser::GeneralQuantifierContext *ctx) override;
    GEAXErrorCode setEdgePatternHopRange(AstNode *edgeNode, IntParam &&lower,
                                         std::optional<IntParam> &&upper);

    std::any visitInsertPathPattern(parser::GqlParser::InsertPathPatternContext *ctx) override;
    std::any visitInsertNodePattern(parser::GqlParser::InsertNodePatternContext *ctx) override;
    GEAXErrorCode visitInsertEdgePattern(
        EdgeDirection direction, parser::GqlParser::InsertElementPatternFillerContext *fillerCtx);
    std::any visitInsertEdgePointingLeft(
        parser::GqlParser::InsertEdgePointingLeftContext *ctx) override;
    std::any visitInsertEdgePointingRight(
        parser::GqlParser::InsertEdgePointingRightContext *ctx) override;
    std::any visitInsertEdgeUndirected(
        parser::GqlParser::InsertEdgeUndirectedContext *ctx) override;
    std::any visitInsertElementPatternFiller(
        parser::GqlParser::InsertElementPatternFillerContext *ctx) override;
    std::any visitLabelAndPropertySetSpecification(
        parser::GqlParser::LabelAndPropertySetSpecificationContext *ctx) override;
    std::any visitLabelSetSpecification(
        parser::GqlParser::LabelSetSpecificationContext *ctx) override;
    std::any visitNodePattern(parser::GqlParser::NodePatternContext *ctx) override;
    std::any visitElementPatternFiller(
        parser::GqlParser::ElementPatternFillerContext *ctx) override;
    std::any visitElementVariableDeclaration(
        parser::GqlParser::ElementVariableDeclarationContext *ctx) override;
    GEAXErrorCode visitElementPatternPredicates(
        std::vector<parser::GqlParser::ElementPatternPredicateContext *> &predicateCtxs,
        ElementFiller *filler);

    GEAXErrorCode visitFullEdgePattern(EdgeDirection direction,
                                       parser::GqlParser::ElementPatternFillerContext *fillerCtx);
    std::any visitFullEdgePointingLeft(
        parser::GqlParser::FullEdgePointingLeftContext *ctx) override;
    std::any visitFullEdgeUndirected(parser::GqlParser::FullEdgeUndirectedContext *ctx) override;
    std::any visitFullEdgePointingRight(
        parser::GqlParser::FullEdgePointingRightContext *ctx) override;
    std::any visitFullEdgeLeftOrUndirected(
        parser::GqlParser::FullEdgeLeftOrUndirectedContext *ctx) override;
    std::any visitFullEdgeUndirectedOrRight(
        parser::GqlParser::FullEdgeUndirectedOrRightContext *ctx) override;
    std::any visitFullEdgeLeftOrRight(parser::GqlParser::FullEdgeLeftOrRightContext *ctx) override;
    std::any visitFullEdgeAnyDirection(
        parser::GqlParser::FullEdgeAnyDirectionContext *ctx) override;

    std::any visitAbbreviatedEdgePointingLeft(
        parser::GqlParser::AbbreviatedEdgePointingLeftContext *ctx) override;
    std::any visitAbbreviatedEdgeUndirected(
        parser::GqlParser::AbbreviatedEdgeUndirectedContext *ctx) override;
    std::any visitAbbreviatedEdgePointingRight(
        parser::GqlParser::AbbreviatedEdgePointingRightContext *ctx) override;
    std::any visitAbbreviatedEdgeLeftOrUndirected(
        parser::GqlParser::AbbreviatedEdgeLeftOrUndirectedContext *ctx) override;
    std::any visitAbbreviatedEdgeUndirectedOrRight(
        parser::GqlParser::AbbreviatedEdgeUndirectedOrRightContext *ctx) override;
    std::any visitAbbreviatedEdgeLeftOrRight(
        parser::GqlParser::AbbreviatedEdgeLeftOrRightContext *ctx) override;
    std::any visitAbbreviatedEdgeAnyDirection(
        parser::GqlParser::AbbreviatedEdgeAnyDirectionContext *ctx) override;

    std::any visitWhereClause(parser::GqlParser::WhereClauseContext *ctx) override;
    std::any visitGroupingElement(parser::GqlParser::GroupingElementContext *ctx) override;

    // non ISOGQL standard
    std::any visitElementTableFunction(
        parser::GqlParser::ElementTableFunctionContext *ctx) override;

    // expression
    std::any visitGqlNotExpression(parser::GqlParser::GqlNotExpressionContext *ctx) override;
    std::any visitGqlLogicalAndExpression(
        parser::GqlParser::GqlLogicalAndExpressionContext *ctx) override;
    std::any visitGqlLogicalXorExpression(
        parser::GqlParser::GqlLogicalXorExpressionContext *ctx) override;
    std::any visitGqlLogicalOrExpression(
        parser::GqlParser::GqlLogicalOrExpressionContext *ctx) override;
    std::any visitGqlBooleanTestExpression(
        parser::GqlParser::GqlBooleanTestExpressionContext *ctx) override;

    // predicates
    std::any visitGqlInExpression(parser::GqlParser::GqlInExpressionContext *ctx) override;
    std::any visitGqlLikeExpression(parser::GqlParser::GqlLikeExpressionContext *ctx) override;
    std::any visitGqlBetweenExpression(
        parser::GqlParser::GqlBetweenExpressionContext *ctx) override;
    std::any visitGqlComparisonExpression(
        parser::GqlParser::GqlComparisonExpressionContext *ctx) override;
    std::any visitGqlExistsExpression(parser::GqlParser::GqlExistsExpressionContext *ctx) override;
    std::any visitGqlNullExpression(parser::GqlParser::GqlNullExpressionContext *ctx) override;
    std::any visitGqlNormalizedExpression(
        parser::GqlParser::GqlNormalizedExpressionContext *ctx) override;
    std::any visitGqlDirectedExpression(
        parser::GqlParser::GqlDirectedExpressionContext *ctx) override;
    std::any visitGqlLabeledExpression(
        parser::GqlParser::GqlLabeledExpressionContext *ctx) override;
    std::any visitGqlSourceDestinationExpression(
        parser::GqlParser::GqlSourceDestinationExpressionContext *ctx) override;
    std::any visitGqlAllDifferentExpression(
        parser::GqlParser::GqlAllDifferentExpressionContext *ctx) override;
    std::any visitGqlSameExpression(parser::GqlParser::GqlSameExpressionContext *ctx) override;

    // atom
    std::any visitGqlPropertyReference(
        parser::GqlParser::GqlPropertyReferenceContext *ctx) override;
    std::any visitGqlUnaryExpression(parser::GqlParser::GqlUnaryExpressionContext *ctx) override;
    std::any visitGqlValueQueryExpression(
        parser::GqlParser::GqlValueQueryExpressionContext *ctx) override;
    std::any visitGqlBitXorExpression(parser::GqlParser::GqlBitXorExpressionContext *ctx) override;
    std::any visitGqlBitShiftExpression(
        parser::GqlParser::GqlBitShiftExpressionContext *ctx) override;
    std::any visitGqlBitAndExpression(parser::GqlParser::GqlBitAndExpressionContext *ctx) override;
    std::any visitGqlBitOrExpression(parser::GqlParser::GqlBitOrExpressionContext *ctx) override;
    std::any visitGqlHighArithmeticExpression(
        parser::GqlParser::GqlHighArithmeticExpressionContext *ctx) override;
    std::any visitGqlLowArithmeticExpression(
        parser::GqlParser::GqlLowArithmeticExpressionContext *ctx) override;

    std::any visitLabelExpression(parser::GqlParser::LabelExpressionContext *ctx) override;
    std::any visitLabelTerm(parser::GqlParser::LabelTermContext *ctx) override;
    std::any visitLabelFactor(parser::GqlParser::LabelFactorContext *ctx) override;

    // literal
    std::any visitFloatLiteral(parser::GqlParser::FloatLiteralContext *ctx) override;
    std::any visitIntegerLiteral(parser::GqlParser::IntegerLiteralContext *ctx) override;
    std::any visitBooleanLiteral(parser::GqlParser::BooleanLiteralContext *ctx) override;
    std::any visitCharacterStringLiteral(
        parser::GqlParser::CharacterStringLiteralContext *ctx) override;
    std::any visitByteStringLiteral(parser::GqlParser::ByteStringLiteralContext *ctx) override;
    std::any visitNullLiteral(parser::GqlParser::NullLiteralContext *ctx) override;
    std::any visitDateLiteral(parser::GqlParser::DateLiteralContext *ctx) override;
    std::any visitDatetimeLiteral(parser::GqlParser::DatetimeLiteralContext *ctx) override;
    std::any visitTimeLiteral(parser::GqlParser::TimeLiteralContext *ctx) override;
    std::any visitListLiteral(parser::GqlParser::ListLiteralContext *ctx) override;
    std::any visitMapLiteral(parser::GqlParser::MapLiteralContext *ctx) override;
    std::any visitRecordLiteral(parser::GqlParser::RecordLiteralContext *ctx) override;
    std::any visitDurationLiteral(parser::GqlParser::DurationLiteralContext *ctx) override;
    std::any visitPropertyKeyValuePairList(
        parser::GqlParser::PropertyKeyValuePairListContext *ctx) override;
    std::any visitPropertyKeyValuePair(
        parser::GqlParser::PropertyKeyValuePairContext *ctx) override;

    // functions
    std::any visitGqlOneArgScalarFunction(
        parser::GqlParser::GqlOneArgScalarFunctionContext *ctx) override;
    std::any visitGqlTwoArgScalarFunction(
        parser::GqlParser::GqlTwoArgScalarFunctionContext *ctx) override;
    std::any visitGqlIfFunction(parser::GqlParser::GqlIfFunctionContext *ctx) override;
    std::any visitGeneralFunction(parser::GqlParser::GeneralFunctionContext *ctx) override;
    std::any visitGqlSubstringFunction(
        parser::GqlParser::GqlSubstringFunctionContext *ctx) override;
    std::any visitGqlFoldStringFunction(
        parser::GqlParser::GqlFoldStringFunctionContext *ctx) override;
    std::any visitGqlSingleTrimStringFunction(
        parser::GqlParser::GqlSingleTrimStringFunctionContext *ctx) override;
    std::any visitGqlMultiTrimStringFunction(
        parser::GqlParser::GqlMultiTrimStringFunctionContext *ctx) override;
    std::any visitElementFunction(parser::GqlParser::ElementFunctionContext *ctx) override;
    std::any visitGqlListTrimFunction(parser::GqlParser::GqlListTrimFunctionContext *ctx) override;
    std::any visitGqlElementsOfPathFunction(
        parser::GqlParser::GqlElementsOfPathFunctionContext *ctx) override;

    // aggregate functions
    std::any visitGqlCountAllFunction(parser::GqlParser::GqlCountAllFunctionContext *ctx) override;
    std::any visitGqlCountDistinctFunction(
        parser::GqlParser::GqlCountDistinctFunctionContext *ctx) override;
    std::any visitGqlGeneralSetFunction(
        parser::GqlParser::GqlGeneralSetFunctionContext *ctx) override;
    std::any visitGqlBinarySetFunction(
        parser::GqlParser::GqlBinarySetFunctionContext *ctx) override;
    std::any visitGqlWindowFunction(parser::GqlParser::GqlWindowFunctionContext *ctx) override;
    std::any visitWindowClause(parser::GqlParser::WindowClauseContext *ctx) override;
    GEAXErrorCode visitAggExpr(parser::GqlParser::GeneralSetFunctionTypeContext *typeCtx,
                               parser::GqlParser::SetQuantifierContext *quantifierCtx,
                               parser::GqlParser::ExpressionContext *exprCtx, AggFunc *agg);
    std::any visitGqlDistinctInGeneralFunction(
        parser::GqlParser::GqlDistinctInGeneralFunctionContext *ctx) override;

    // case functions
    std::any visitGqlNullIfCaseFunction(
        parser::GqlParser::GqlNullIfCaseFunctionContext *ctx) override;
    std::any visitGqlCoalesceCaseFunction(
        parser::GqlParser::GqlCoalesceCaseFunctionContext *ctx) override;
    std::any visitGqlSimpleCaseFunction(
        parser::GqlParser::GqlSimpleCaseFunctionContext *ctx) override;
    std::any visitGqlSearchedCaseFunction(
        parser::GqlParser::GqlSearchedCaseFunctionContext *ctx) override;

    GEAXErrorCode visitFunction(
        antlr4::ParserRuleContext *funcName,
        const std::vector<parser::GqlParser::FunctionParameterContext *> &funcParams);
    GEAXErrorCode visitTableFunction(
        parser::GqlParser::TableFunctionNameContext *funcName,
        const std::vector<parser::GqlParser::EleTableFuncParameterContext *> &funcParams);

    // cast functions
    std::any visitCastFunction(parser::GqlParser::CastFunctionContext *ctx) override;

    std::any visitQueryConjunction(parser::GqlParser::QueryConjunctionContext *ctx) override;
    std::any visitSetOperator(parser::GqlParser::SetOperatorContext *ctx) override;
    std::any visitPropertyReference(parser::GqlParser::PropertyReferenceContext *ctx) override;

    // collections
    std::any visitListValueConstructor(
        parser::GqlParser::ListValueConstructorContext *ctx) override;
    std::any visitMapValueConstructor(parser::GqlParser::MapValueConstructorContext *ctx) override;
    std::any visitRecordValueConstructor(
        parser::GqlParser::RecordValueConstructorContext *ctx) override;
    std::any visitListValue(parser::GqlParser::ListValueContext *ctx) override;

    // names
    std::any visitLabelName(parser::GqlParser::LabelNameContext *ctx) override;

    std::any visitCatalogProcedureParentAndName(
        parser::GqlParser::CatalogProcedureParentAndNameContext *ctx) override;

    std::any visitTerminal(antlr4::tree::TerminalNode *node) override;
    std::any visitChildren(antlr4::tree::ParseTree *node) override;

    // remove function
    std::any visitRemoveStatement(parser::GqlParser::RemoveStatementContext *ctx) override;
    std::any visitRemovePropertyItem(parser::GqlParser::RemovePropertyItemContext *ctx) override;

    // helper functions
    GEAXErrorCode visitBinaryExpr(antlr4::ParserRuleContext *lhs, antlr4::ParserRuleContext *rhs,
                                  BinaryOp *expr);
    GEAXErrorCode visitListExpr(std::vector<parser::GqlParser::ExpressionContext *> &epxrs,
                                MkList *list);
    GEAXErrorCode visitListLiteralItem(
        std::vector<parser::GqlParser::GeneralLiteralContext *> &literalCtxs, MkList *list);

    std::string trimAccentGrave(std::string str) {
        return str.size() >= 2 && str.front() == '`' && str.back() == '`'
                   ? str.substr(1, str.size() - 2)
                   : str;
    }

    template <typename T>
    T *castAs(AstNode *node, AstNodeType type);

    GEAXErrorCode byPass(antlr4::tree::ParseTree *node);
    GEAXErrorCode checkChildResult() const;
    GEAXErrorCode checkChildRet() const;
    GEAXErrorCode checkTokenResult() const;

    std::string getFullText(antlr4::ParserRuleContext *tree) const;
    std::string escapeText(const std::string &text) const;

private:
    GQLResolveCtx &ctx_;
    AstNode *childRes_;
    GEAXErrorCode childRet_;
    AstNode *faRes_;
    antlr4::Token *token_;
};  // class GQLAstVisitor

}  // namespace frontend
}  // namespace geax

#endif  // FRONTEND_ISOGQL_GQLASTVISITOR_H_
