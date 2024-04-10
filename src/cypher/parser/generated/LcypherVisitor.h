
// Generated from src/cypher/grammar/Lcypher.g4 by ANTLR 4.13.0

#pragma once


#include "antlr4-runtime.h"
#include "LcypherParser.h"


namespace parser {

/**
 * This class defines an abstract visitor for a parse tree
 * produced by LcypherParser.
 */
class  LcypherVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by LcypherParser.
   */
    virtual std::any visitOC_Cypher(LcypherParser::OC_CypherContext *context) = 0;

    virtual std::any visitOC_Statement(LcypherParser::OC_StatementContext *context) = 0;

    virtual std::any visitOC_Query(LcypherParser::OC_QueryContext *context) = 0;

    virtual std::any visitOC_RegularQuery(LcypherParser::OC_RegularQueryContext *context) = 0;

    virtual std::any visitOC_Union(LcypherParser::OC_UnionContext *context) = 0;

    virtual std::any visitOC_SingleQuery(LcypherParser::OC_SingleQueryContext *context) = 0;

    virtual std::any visitOC_SinglePartQuery(LcypherParser::OC_SinglePartQueryContext *context) = 0;

    virtual std::any visitOC_MultiPartQuery(LcypherParser::OC_MultiPartQueryContext *context) = 0;

    virtual std::any visitOC_UpdatingClause(LcypherParser::OC_UpdatingClauseContext *context) = 0;

    virtual std::any visitOC_ReadingClause(LcypherParser::OC_ReadingClauseContext *context) = 0;

    virtual std::any visitOC_Match(LcypherParser::OC_MatchContext *context) = 0;

    virtual std::any visitOC_Unwind(LcypherParser::OC_UnwindContext *context) = 0;

    virtual std::any visitOC_Merge(LcypherParser::OC_MergeContext *context) = 0;

    virtual std::any visitOC_MergeAction(LcypherParser::OC_MergeActionContext *context) = 0;

    virtual std::any visitOC_Create(LcypherParser::OC_CreateContext *context) = 0;

    virtual std::any visitOC_Set(LcypherParser::OC_SetContext *context) = 0;

    virtual std::any visitOC_SetItem(LcypherParser::OC_SetItemContext *context) = 0;

    virtual std::any visitOC_Delete(LcypherParser::OC_DeleteContext *context) = 0;

    virtual std::any visitOC_Remove(LcypherParser::OC_RemoveContext *context) = 0;

    virtual std::any visitOC_RemoveItem(LcypherParser::OC_RemoveItemContext *context) = 0;

    virtual std::any visitOC_InQueryCall(LcypherParser::OC_InQueryCallContext *context) = 0;

    virtual std::any visitOC_StandaloneCall(LcypherParser::OC_StandaloneCallContext *context) = 0;

    virtual std::any visitOC_YieldItems(LcypherParser::OC_YieldItemsContext *context) = 0;

    virtual std::any visitOC_YieldItem(LcypherParser::OC_YieldItemContext *context) = 0;

    virtual std::any visitOC_With(LcypherParser::OC_WithContext *context) = 0;

    virtual std::any visitOC_Return(LcypherParser::OC_ReturnContext *context) = 0;

    virtual std::any visitOC_ReturnBody(LcypherParser::OC_ReturnBodyContext *context) = 0;

    virtual std::any visitOC_ReturnItems(LcypherParser::OC_ReturnItemsContext *context) = 0;

    virtual std::any visitOC_ReturnItem(LcypherParser::OC_ReturnItemContext *context) = 0;

    virtual std::any visitOC_Order(LcypherParser::OC_OrderContext *context) = 0;

    virtual std::any visitOC_Skip(LcypherParser::OC_SkipContext *context) = 0;

    virtual std::any visitOC_Limit(LcypherParser::OC_LimitContext *context) = 0;

    virtual std::any visitOC_SortItem(LcypherParser::OC_SortItemContext *context) = 0;

    virtual std::any visitOC_Hint(LcypherParser::OC_HintContext *context) = 0;

    virtual std::any visitOC_Where(LcypherParser::OC_WhereContext *context) = 0;

    virtual std::any visitOC_Pattern(LcypherParser::OC_PatternContext *context) = 0;

    virtual std::any visitOC_PatternPart(LcypherParser::OC_PatternPartContext *context) = 0;

    virtual std::any visitOC_AnonymousPatternPart(LcypherParser::OC_AnonymousPatternPartContext *context) = 0;

    virtual std::any visitOC_PatternElement(LcypherParser::OC_PatternElementContext *context) = 0;

    virtual std::any visitOC_NodePattern(LcypherParser::OC_NodePatternContext *context) = 0;

    virtual std::any visitOC_PatternElementChain(LcypherParser::OC_PatternElementChainContext *context) = 0;

    virtual std::any visitOC_RelationshipPattern(LcypherParser::OC_RelationshipPatternContext *context) = 0;

    virtual std::any visitOC_RelationshipDetail(LcypherParser::OC_RelationshipDetailContext *context) = 0;

    virtual std::any visitOC_Properties(LcypherParser::OC_PropertiesContext *context) = 0;

    virtual std::any visitOC_RelationshipTypes(LcypherParser::OC_RelationshipTypesContext *context) = 0;

    virtual std::any visitOC_NodeLabels(LcypherParser::OC_NodeLabelsContext *context) = 0;

    virtual std::any visitOC_NodeLabel(LcypherParser::OC_NodeLabelContext *context) = 0;

    virtual std::any visitOC_RangeLiteral(LcypherParser::OC_RangeLiteralContext *context) = 0;

    virtual std::any visitOC_LabelName(LcypherParser::OC_LabelNameContext *context) = 0;

    virtual std::any visitOC_RelTypeName(LcypherParser::OC_RelTypeNameContext *context) = 0;

    virtual std::any visitOC_Expression(LcypherParser::OC_ExpressionContext *context) = 0;

    virtual std::any visitOC_OrExpression(LcypherParser::OC_OrExpressionContext *context) = 0;

    virtual std::any visitOC_XorExpression(LcypherParser::OC_XorExpressionContext *context) = 0;

    virtual std::any visitOC_AndExpression(LcypherParser::OC_AndExpressionContext *context) = 0;

    virtual std::any visitOC_NotExpression(LcypherParser::OC_NotExpressionContext *context) = 0;

    virtual std::any visitOC_ComparisonExpression(LcypherParser::OC_ComparisonExpressionContext *context) = 0;

    virtual std::any visitOC_AddOrSubtractExpression(LcypherParser::OC_AddOrSubtractExpressionContext *context) = 0;

    virtual std::any visitOC_MultiplyDivideModuloExpression(LcypherParser::OC_MultiplyDivideModuloExpressionContext *context) = 0;

    virtual std::any visitOC_PowerOfExpression(LcypherParser::OC_PowerOfExpressionContext *context) = 0;

    virtual std::any visitOC_UnaryAddOrSubtractExpression(LcypherParser::OC_UnaryAddOrSubtractExpressionContext *context) = 0;

    virtual std::any visitOC_StringListNullOperatorExpression(LcypherParser::OC_StringListNullOperatorExpressionContext *context) = 0;

    virtual std::any visitOC_ListOperatorExpression(LcypherParser::OC_ListOperatorExpressionContext *context) = 0;

    virtual std::any visitOC_StringOperatorExpression(LcypherParser::OC_StringOperatorExpressionContext *context) = 0;

    virtual std::any visitOC_NullOperatorExpression(LcypherParser::OC_NullOperatorExpressionContext *context) = 0;

    virtual std::any visitOC_PropertyOrLabelsExpression(LcypherParser::OC_PropertyOrLabelsExpressionContext *context) = 0;

    virtual std::any visitOC_Atom(LcypherParser::OC_AtomContext *context) = 0;

    virtual std::any visitOC_Literal(LcypherParser::OC_LiteralContext *context) = 0;

    virtual std::any visitOC_BooleanLiteral(LcypherParser::OC_BooleanLiteralContext *context) = 0;

    virtual std::any visitOC_ListLiteral(LcypherParser::OC_ListLiteralContext *context) = 0;

    virtual std::any visitOC_PartialComparisonExpression(LcypherParser::OC_PartialComparisonExpressionContext *context) = 0;

    virtual std::any visitOC_ParenthesizedExpression(LcypherParser::OC_ParenthesizedExpressionContext *context) = 0;

    virtual std::any visitOC_RelationshipsPattern(LcypherParser::OC_RelationshipsPatternContext *context) = 0;

    virtual std::any visitOC_FilterExpression(LcypherParser::OC_FilterExpressionContext *context) = 0;

    virtual std::any visitOC_IdInColl(LcypherParser::OC_IdInCollContext *context) = 0;

    virtual std::any visitOC_FunctionInvocation(LcypherParser::OC_FunctionInvocationContext *context) = 0;

    virtual std::any visitOC_FunctionName(LcypherParser::OC_FunctionNameContext *context) = 0;

    virtual std::any visitOC_ExplicitProcedureInvocation(LcypherParser::OC_ExplicitProcedureInvocationContext *context) = 0;

    virtual std::any visitOC_ImplicitProcedureInvocation(LcypherParser::OC_ImplicitProcedureInvocationContext *context) = 0;

    virtual std::any visitOC_ProcedureResultField(LcypherParser::OC_ProcedureResultFieldContext *context) = 0;

    virtual std::any visitOC_ProcedureName(LcypherParser::OC_ProcedureNameContext *context) = 0;

    virtual std::any visitOC_Namespace(LcypherParser::OC_NamespaceContext *context) = 0;

    virtual std::any visitOC_ListComprehension(LcypherParser::OC_ListComprehensionContext *context) = 0;

    virtual std::any visitOC_PatternComprehension(LcypherParser::OC_PatternComprehensionContext *context) = 0;

    virtual std::any visitOC_PropertyLookup(LcypherParser::OC_PropertyLookupContext *context) = 0;

    virtual std::any visitOC_CaseExpression(LcypherParser::OC_CaseExpressionContext *context) = 0;

    virtual std::any visitOC_CaseAlternatives(LcypherParser::OC_CaseAlternativesContext *context) = 0;

    virtual std::any visitOC_Variable(LcypherParser::OC_VariableContext *context) = 0;

    virtual std::any visitOC_NumberLiteral(LcypherParser::OC_NumberLiteralContext *context) = 0;

    virtual std::any visitOC_MapLiteral(LcypherParser::OC_MapLiteralContext *context) = 0;

    virtual std::any visitOC_Parameter(LcypherParser::OC_ParameterContext *context) = 0;

    virtual std::any visitOC_PropertyExpression(LcypherParser::OC_PropertyExpressionContext *context) = 0;

    virtual std::any visitOC_PropertyKeyName(LcypherParser::OC_PropertyKeyNameContext *context) = 0;

    virtual std::any visitOC_IntegerLiteral(LcypherParser::OC_IntegerLiteralContext *context) = 0;

    virtual std::any visitOC_DoubleLiteral(LcypherParser::OC_DoubleLiteralContext *context) = 0;

    virtual std::any visitOC_SchemaName(LcypherParser::OC_SchemaNameContext *context) = 0;

    virtual std::any visitOC_SymbolicName(LcypherParser::OC_SymbolicNameContext *context) = 0;

    virtual std::any visitOC_ReservedWord(LcypherParser::OC_ReservedWordContext *context) = 0;

    virtual std::any visitOC_LeftArrowHead(LcypherParser::OC_LeftArrowHeadContext *context) = 0;

    virtual std::any visitOC_RightArrowHead(LcypherParser::OC_RightArrowHeadContext *context) = 0;

    virtual std::any visitOC_Dash(LcypherParser::OC_DashContext *context) = 0;


};

}  // namespace parser
