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

// Generated from Lcypher.g4 by ANTLR 4.9.2

#pragma once

#include "antlr4-runtime/antlr4-runtime.h"
#include "parser/generated/LcypherVisitor.h"
#include "cypher/cypher_exception.h"
#include "geax-front-end/common/ObjectAllocator.h"
#include "geax-front-end/ast/Ast.h"
#include "cypher/parser/data_typedef.h"

#if __APPLE__
#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif
#endif  // #if __APPLE__

namespace parser {

enum class VisitType {
    kReadingClause,
    kUpdatingClause,
    kMergeClause,
    kMergeOnMatch,
    kMergeOnCreate,
    kReadingPattern,
    kUpdatingPattern,
    kMatchPattern,
    kSetVariable,
    kSetLabel,
    kSetNull,
    kDeleteVariable,
    kWithClause,
    kStandaloneCall,
    kInQueryCall,
    kSinglePartQuery,
    kUnionClause
};

class VisitGuard {
    VisitType type_;
    std::unordered_set<VisitType>& cur_types_;
 public:
    VisitGuard(VisitType type, std::unordered_set<VisitType>& cur_types)
        : type_(type), cur_types_(cur_types) {
        cur_types.emplace(type_);
    }

    ~VisitGuard() { cur_types_.erase(type_); }

    static bool InClause(VisitType type, const std::unordered_set<VisitType>& cur_types) {
        return cur_types.find(type) != cur_types.end();
    }
};

/**
 * This class provides an empty implementation of LcypherVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class CypherBaseVisitorV2 : public LcypherVisitor {
    std::string error_msg_;
    geax::common::ObjectArenaAllocator& objAlloc_;
    geax::frontend::AstNode * node_;
    size_t anonymous_idx_;
    std::unordered_set<VisitType> visit_types_;
    static const std::unordered_map<std::string, geax::frontend::GeneralSetFunction> S_AGG_LIST;
    static const std::unordered_map<std::string, geax::frontend::BinarySetFunction> S_BAGG_LIST;
    geax::frontend::PathChain* path_chain_;
    geax::frontend::FilterStatement* filter_in_with_clause_;
    parser::CmdType cmd_type_;
    std::unordered_map<std::string, std::stack<std::string>> list_comprehension_anonymous_symbols_;
    std::unordered_map<std::string, int32_t> list_comprehension_anonymous_idx_;
    int32_t list_comprehension_depth {0};

    std::string GenerateListComprehension(const std::string &x) {
        return std::string("LIST_COMPREHENSION@") + x + "@" +
               std::to_string(list_comprehension_anonymous_idx_[x]++);
    }

 public:
    CypherBaseVisitorV2() = delete;

    parser::CmdType CommandType() const { return cmd_type_; }

    CypherBaseVisitorV2(geax::common::ObjectArenaAllocator& objAlloc,
            antlr4::tree::ParseTree *tree);

    std::string GenAnonymousAlias(bool is_node);

    std::string GetFullText(antlr4::ParserRuleContext* ruleCtx) const;

    geax::frontend::AstNode *result() const { return node_; }

    std::any visitOC_Cypher(LcypherParser::OC_CypherContext *ctx) override;

    std::any visitOC_Statement(LcypherParser::OC_StatementContext *ctx) override;

    std::any visitOC_Query(LcypherParser::OC_QueryContext *ctx) override;

    std::any visitOC_RegularQuery(LcypherParser::OC_RegularQueryContext *ctx) override;

    std::any visitOC_Union(LcypherParser::OC_UnionContext *ctx) override;

    std::any visitOC_SingleQuery(LcypherParser::OC_SingleQueryContext *ctx) override;

    std::any visitOC_SinglePartQuery(
        LcypherParser::OC_SinglePartQueryContext *ctx) override;

    std::any visitOC_MultiPartQuery(LcypherParser::OC_MultiPartQueryContext *ctx) override;

    std::any visitOC_UpdatingClause(LcypherParser::OC_UpdatingClauseContext *ctx) override;

    std::any visitOC_ReadingClause(LcypherParser::OC_ReadingClauseContext *ctx) override;

    std::any visitOC_Match(LcypherParser::OC_MatchContext *ctx) override;

    std::any visitOC_Unwind(LcypherParser::OC_UnwindContext *ctx) override;

    std::any visitOC_Merge(LcypherParser::OC_MergeContext *ctx) override;

    std::any visitOC_MergeAction(LcypherParser::OC_MergeActionContext *ctx) override;

    std::any visitOC_Create(LcypherParser::OC_CreateContext *ctx) override;

    std::any visitOC_Set(LcypherParser::OC_SetContext *ctx) override;

    std::any visitOC_SetItem(LcypherParser::OC_SetItemContext *ctx) override;

    std::any visitOC_Delete(LcypherParser::OC_DeleteContext *ctx) override;

    std::any visitOC_Remove(LcypherParser::OC_RemoveContext *ctx) override;

    std::any visitOC_RemoveItem(LcypherParser::OC_RemoveItemContext *ctx) override;

    std::any visitOC_InQueryCall(LcypherParser::OC_InQueryCallContext *ctx) override;

    std::any visitOC_StandaloneCall(LcypherParser::OC_StandaloneCallContext *ctx) override;

    std::any visitOC_YieldItems(LcypherParser::OC_YieldItemsContext *ctx) override;

    std::any visitOC_YieldItem(LcypherParser::OC_YieldItemContext *ctx) override;

    std::any visitOC_With(LcypherParser::OC_WithContext *ctx) override;

    std::any visitOC_Return(LcypherParser::OC_ReturnContext *ctx) override;

    std::any visitOC_ReturnBody(LcypherParser::OC_ReturnBodyContext *ctx) override;

    std::any visitOC_ReturnItems(LcypherParser::OC_ReturnItemsContext *ctx) override;

    std::any visitOC_ReturnItem(LcypherParser::OC_ReturnItemContext *ctx) override;

    std::any visitOC_Order(LcypherParser::OC_OrderContext *ctx) override;

    std::any visitOC_Skip(LcypherParser::OC_SkipContext *ctx) override;

    std::any visitOC_Limit(LcypherParser::OC_LimitContext *ctx) override;

    std::any visitOC_SortItem(LcypherParser::OC_SortItemContext *ctx) override;

    std::any visitOC_Hint(LcypherParser::OC_HintContext *ctx) override;

    std::any visitOC_Where(LcypherParser::OC_WhereContext *ctx) override;

    std::any visitOC_Pattern(LcypherParser::OC_PatternContext *ctx) override;

    std::any visitOC_PatternPart(LcypherParser::OC_PatternPartContext *ctx) override;

    std::any visitOC_AnonymousPatternPart(
        LcypherParser::OC_AnonymousPatternPartContext *ctx) override;

    std::any visitOC_PatternElement(LcypherParser::OC_PatternElementContext *ctx) override;

    std::any visitOC_NodePattern(LcypherParser::OC_NodePatternContext *ctx) override;

    std::any visitOC_PatternElementChain(
        LcypherParser::OC_PatternElementChainContext *ctx) override;

    std::any visitOC_RelationshipPattern(
        LcypherParser::OC_RelationshipPatternContext *ctx) override;

    std::any visitOC_RelationshipDetail(
        LcypherParser::OC_RelationshipDetailContext *ctx) override;

    std::any visitOC_Properties(LcypherParser::OC_PropertiesContext *ctx) override;

    std::any visitOC_RelationshipTypes(
        LcypherParser::OC_RelationshipTypesContext *ctx) override;

    std::any visitOC_NodeLabels(LcypherParser::OC_NodeLabelsContext *ctx) override;

    std::any visitOC_NodeLabel(LcypherParser::OC_NodeLabelContext *ctx) override;

    std::any visitOC_RangeLiteral(LcypherParser::OC_RangeLiteralContext *ctx) override;

    std::any visitOC_LabelName(LcypherParser::OC_LabelNameContext *ctx) override;

    std::any visitOC_RelTypeName(LcypherParser::OC_RelTypeNameContext *ctx) override;

    std::any visitOC_Expression(LcypherParser::OC_ExpressionContext *ctx) override;

    std::any visitOC_OrExpression(LcypherParser::OC_OrExpressionContext *ctx) override;

    std::any visitOC_XorExpression(LcypherParser::OC_XorExpressionContext *ctx) override;

    std::any visitOC_AndExpression(LcypherParser::OC_AndExpressionContext *ctx) override;

    std::any visitOC_NotExpression(LcypherParser::OC_NotExpressionContext *ctx) override;

    std::any visitOC_ComparisonExpression(
        LcypherParser::OC_ComparisonExpressionContext *ctx) override;

    std::any visitOC_AddOrSubtractExpression(
        LcypherParser::OC_AddOrSubtractExpressionContext *ctx) override;

    std::any visitOC_MultiplyDivideModuloExpression(
        LcypherParser::OC_MultiplyDivideModuloExpressionContext *ctx) override;

    std::any visitOC_PowerOfExpression(
        LcypherParser::OC_PowerOfExpressionContext *ctx) override;

    std::any visitOC_UnaryAddOrSubtractExpression(
        LcypherParser::OC_UnaryAddOrSubtractExpressionContext *ctx) override;

    std::any visitOC_StringListNullOperatorExpression(
        LcypherParser::OC_StringListNullOperatorExpressionContext *ctx) override;

    std::any visitOC_ListOperatorExpression(
        LcypherParser::OC_ListOperatorExpressionContext *ctx) override;

    std::any visitOC_StringOperatorExpression(
        LcypherParser::OC_StringOperatorExpressionContext *ctx) override;

    std::any visitOC_NullOperatorExpression(
        LcypherParser::OC_NullOperatorExpressionContext *ctx) override;

    std::any visitOC_PropertyOrLabelsExpression(
        LcypherParser::OC_PropertyOrLabelsExpressionContext *ctx) override;

    std::any visitOC_Atom(LcypherParser::OC_AtomContext *ctx) override;

    std::any visitOC_Literal(LcypherParser::OC_LiteralContext *ctx) override;

    std::any visitOC_BooleanLiteral(LcypherParser::OC_BooleanLiteralContext *ctx) override;

    std::any visitOC_ListLiteral(LcypherParser::OC_ListLiteralContext *ctx) override;

    std::any visitOC_PartialComparisonExpression(
        LcypherParser::OC_PartialComparisonExpressionContext *ctx) override;

    std::any visitOC_ParenthesizedExpression(
        LcypherParser::OC_ParenthesizedExpressionContext *ctx) override;

    std::any visitOC_RelationshipsPattern(
        LcypherParser::OC_RelationshipsPatternContext *ctx) override;

    std::any visitOC_FilterExpression(
        LcypherParser::OC_FilterExpressionContext *ctx) override;

    std::any visitOC_IdInColl(LcypherParser::OC_IdInCollContext *ctx) override;

    std::any visitOC_FunctionInvocation(
        LcypherParser::OC_FunctionInvocationContext *ctx) override;

    std::any visitOC_FunctionName(LcypherParser::OC_FunctionNameContext *ctx) override;

    std::any visitOC_ExplicitProcedureInvocation(
        LcypherParser::OC_ExplicitProcedureInvocationContext *ctx) override;

    std::any visitOC_ImplicitProcedureInvocation(
        LcypherParser::OC_ImplicitProcedureInvocationContext *ctx) override;

    std::any visitOC_ProcedureResultField(
        LcypherParser::OC_ProcedureResultFieldContext *ctx) override;

    std::any visitOC_ProcedureName(LcypherParser::OC_ProcedureNameContext *ctx) override;

    std::any visitOC_Namespace(LcypherParser::OC_NamespaceContext *ctx) override;

    std::any visitOC_ListComprehension(
        LcypherParser::OC_ListComprehensionContext *ctx) override;

    std::any visitOC_PatternComprehension(
        LcypherParser::OC_PatternComprehensionContext *ctx) override;

    std::any visitOC_PropertyLookup(LcypherParser::OC_PropertyLookupContext *ctx) override;

    std::any visitOC_CaseExpression(LcypherParser::OC_CaseExpressionContext *ctx) override;

    std::any visitOC_CaseAlternatives(
        LcypherParser::OC_CaseAlternativesContext *ctx) override;

    std::any visitOC_Variable(LcypherParser::OC_VariableContext *ctx) override;

    std::any visitOC_NumberLiteral(LcypherParser::OC_NumberLiteralContext *ctx) override;

    std::any visitOC_MapLiteral(LcypherParser::OC_MapLiteralContext *ctx) override;

    std::any visitOC_Parameter(LcypherParser::OC_ParameterContext *ctx) override;

    std::any visitOC_PropertyExpression(
        LcypherParser::OC_PropertyExpressionContext *ctx) override;

    std::any visitOC_PropertyKeyName(
        LcypherParser::OC_PropertyKeyNameContext *ctx) override;

    std::any visitOC_IntegerLiteral(LcypherParser::OC_IntegerLiteralContext *ctx) override;

    std::any visitOC_DoubleLiteral(LcypherParser::OC_DoubleLiteralContext *ctx) override;

    std::any visitOC_SchemaName(LcypherParser::OC_SchemaNameContext *ctx) override;

    std::any visitOC_ReservedWord(LcypherParser::OC_ReservedWordContext *ctx) override;

    std::any visitOC_SymbolicName(LcypherParser::OC_SymbolicNameContext *ctx) override;

    std::any visitOC_LeftArrowHead(LcypherParser::OC_LeftArrowHeadContext *ctx) override;

    std::any visitOC_RightArrowHead(LcypherParser::OC_RightArrowHeadContext *ctx) override;

    std::any visitOC_Dash(LcypherParser::OC_DashContext *ctx) override;
};

}  // namespace parser
