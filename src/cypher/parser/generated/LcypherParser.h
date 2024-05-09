
// Generated from src/cypher/grammar/Lcypher.g4 by ANTLR 4.13.0

#pragma once


#include "antlr4-runtime.h"


namespace parser {


class  LcypherParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, T__15 = 16, T__16 = 17, T__17 = 18, T__18 = 19, T__19 = 20, 
    T__20 = 21, T__21 = 22, T__22 = 23, T__23 = 24, T__24 = 25, T__25 = 26, 
    T__26 = 27, T__27 = 28, T__28 = 29, T__29 = 30, T__30 = 31, T__31 = 32, 
    T__32 = 33, T__33 = 34, T__34 = 35, T__35 = 36, T__36 = 37, T__37 = 38, 
    T__38 = 39, T__39 = 40, T__40 = 41, T__41 = 42, T__42 = 43, T__43 = 44, 
    T__44 = 45, EXPLAIN = 46, PROFILE = 47, UNION = 48, ALL = 49, OPTIONAL_ = 50, 
    MATCH = 51, UNWIND = 52, AS = 53, MERGE = 54, ON = 55, CREATE = 56, 
    SET = 57, DETACH = 58, DELETE_ = 59, REMOVE = 60, CALL = 61, YIELD = 62, 
    WITH = 63, DISTINCT = 64, RETURN = 65, ORDER = 66, BY = 67, L_SKIP = 68, 
    LIMIT = 69, ASCENDING = 70, ASC = 71, DESCENDING = 72, DESC = 73, USING = 74, 
    JOIN = 75, START = 76, WHERE = 77, OR = 78, XOR = 79, AND = 80, NOT = 81, 
    IN = 82, STARTS = 83, ENDS = 84, CONTAINS = 85, REGEXP = 86, IS = 87, 
    NULL_ = 88, COUNT = 89, ANY = 90, NONE = 91, SINGLE = 92, TRUE_ = 93, 
    FALSE_ = 94, EXISTS = 95, CASE = 96, ELSE = 97, END = 98, WHEN = 99, 
    THEN = 100, StringLiteral = 101, EscapedChar = 102, HexInteger = 103, 
    DecimalInteger = 104, OctalInteger = 105, HexLetter = 106, HexDigit = 107, 
    Digit = 108, NonZeroDigit = 109, NonZeroOctDigit = 110, OctDigit = 111, 
    ZeroDigit = 112, ExponentDecimalReal = 113, RegularDecimalReal = 114, 
    FILTER = 115, EXTRACT = 116, UnescapedSymbolicName = 117, CONSTRAINT = 118, 
    DO = 119, FOR = 120, REQUIRE = 121, UNIQUE = 122, MANDATORY = 123, SCALAR = 124, 
    OF = 125, ADD = 126, DROP = 127, IdentifierStart = 128, IdentifierPart = 129, 
    EscapedSymbolicName = 130, SP = 131, WHITESPACE = 132, Comment = 133
  };

  enum {
    RuleOC_Cypher = 0, RuleOC_Statement = 1, RuleOC_Query = 2, RuleOC_RegularQuery = 3, 
    RuleOC_Union = 4, RuleOC_SingleQuery = 5, RuleOC_SinglePartQuery = 6, 
    RuleOC_MultiPartQuery = 7, RuleOC_UpdatingClause = 8, RuleOC_ReadingClause = 9, 
    RuleOC_Match = 10, RuleOC_Unwind = 11, RuleOC_Merge = 12, RuleOC_MergeAction = 13, 
    RuleOC_Create = 14, RuleOC_Set = 15, RuleOC_SetItem = 16, RuleOC_Delete = 17, 
    RuleOC_Remove = 18, RuleOC_RemoveItem = 19, RuleOC_InQueryCall = 20, 
    RuleOC_StandaloneCall = 21, RuleOC_YieldItems = 22, RuleOC_YieldItem = 23, 
    RuleOC_With = 24, RuleOC_Return = 25, RuleOC_ReturnBody = 26, RuleOC_ReturnItems = 27, 
    RuleOC_ReturnItem = 28, RuleOC_Order = 29, RuleOC_Skip = 30, RuleOC_Limit = 31, 
    RuleOC_SortItem = 32, RuleOC_Hint = 33, RuleOC_Where = 34, RuleOC_Pattern = 35, 
    RuleOC_PatternPart = 36, RuleOC_AnonymousPatternPart = 37, RuleOC_PatternElement = 38, 
    RuleOC_NodePattern = 39, RuleOC_PatternElementChain = 40, RuleOC_RelationshipPattern = 41, 
    RuleOC_RelationshipDetail = 42, RuleOC_Properties = 43, RuleOC_RelationshipTypes = 44, 
    RuleOC_NodeLabels = 45, RuleOC_NodeLabel = 46, RuleOC_RangeLiteral = 47, 
    RuleOC_LabelName = 48, RuleOC_RelTypeName = 49, RuleOC_Expression = 50, 
    RuleOC_OrExpression = 51, RuleOC_XorExpression = 52, RuleOC_AndExpression = 53, 
    RuleOC_NotExpression = 54, RuleOC_ComparisonExpression = 55, RuleOC_AddOrSubtractExpression = 56, 
    RuleOC_MultiplyDivideModuloExpression = 57, RuleOC_PowerOfExpression = 58, 
    RuleOC_UnaryAddOrSubtractExpression = 59, RuleOC_StringListNullOperatorExpression = 60, 
    RuleOC_ListOperatorExpression = 61, RuleOC_StringOperatorExpression = 62, 
    RuleOC_NullOperatorExpression = 63, RuleOC_PropertyOrLabelsExpression = 64, 
    RuleOC_Atom = 65, RuleOC_Literal = 66, RuleOC_BooleanLiteral = 67, RuleOC_ListLiteral = 68, 
    RuleOC_PartialComparisonExpression = 69, RuleOC_ParenthesizedExpression = 70, 
    RuleOC_RelationshipsPattern = 71, RuleOC_FilterExpression = 72, RuleOC_IdInColl = 73, 
    RuleOC_FunctionInvocation = 74, RuleOC_FunctionName = 75, RuleOC_ExplicitProcedureInvocation = 76, 
    RuleOC_ImplicitProcedureInvocation = 77, RuleOC_ProcedureResultField = 78, 
    RuleOC_ProcedureName = 79, RuleOC_Namespace = 80, RuleOC_ListComprehension = 81, 
    RuleOC_PatternComprehension = 82, RuleOC_PropertyLookup = 83, RuleOC_CaseExpression = 84, 
    RuleOC_CaseAlternatives = 85, RuleOC_Variable = 86, RuleOC_NumberLiteral = 87, 
    RuleOC_MapLiteral = 88, RuleOC_Parameter = 89, RuleOC_PropertyExpression = 90, 
    RuleOC_PropertyKeyName = 91, RuleOC_IntegerLiteral = 92, RuleOC_DoubleLiteral = 93, 
    RuleOC_SchemaName = 94, RuleOC_SymbolicName = 95, RuleOC_ReservedWord = 96, 
    RuleOC_LeftArrowHead = 97, RuleOC_RightArrowHead = 98, RuleOC_Dash = 99
  };

  explicit LcypherParser(antlr4::TokenStream *input);

  LcypherParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~LcypherParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class OC_CypherContext;
  class OC_StatementContext;
  class OC_QueryContext;
  class OC_RegularQueryContext;
  class OC_UnionContext;
  class OC_SingleQueryContext;
  class OC_SinglePartQueryContext;
  class OC_MultiPartQueryContext;
  class OC_UpdatingClauseContext;
  class OC_ReadingClauseContext;
  class OC_MatchContext;
  class OC_UnwindContext;
  class OC_MergeContext;
  class OC_MergeActionContext;
  class OC_CreateContext;
  class OC_SetContext;
  class OC_SetItemContext;
  class OC_DeleteContext;
  class OC_RemoveContext;
  class OC_RemoveItemContext;
  class OC_InQueryCallContext;
  class OC_StandaloneCallContext;
  class OC_YieldItemsContext;
  class OC_YieldItemContext;
  class OC_WithContext;
  class OC_ReturnContext;
  class OC_ReturnBodyContext;
  class OC_ReturnItemsContext;
  class OC_ReturnItemContext;
  class OC_OrderContext;
  class OC_SkipContext;
  class OC_LimitContext;
  class OC_SortItemContext;
  class OC_HintContext;
  class OC_WhereContext;
  class OC_PatternContext;
  class OC_PatternPartContext;
  class OC_AnonymousPatternPartContext;
  class OC_PatternElementContext;
  class OC_NodePatternContext;
  class OC_PatternElementChainContext;
  class OC_RelationshipPatternContext;
  class OC_RelationshipDetailContext;
  class OC_PropertiesContext;
  class OC_RelationshipTypesContext;
  class OC_NodeLabelsContext;
  class OC_NodeLabelContext;
  class OC_RangeLiteralContext;
  class OC_LabelNameContext;
  class OC_RelTypeNameContext;
  class OC_ExpressionContext;
  class OC_OrExpressionContext;
  class OC_XorExpressionContext;
  class OC_AndExpressionContext;
  class OC_NotExpressionContext;
  class OC_ComparisonExpressionContext;
  class OC_AddOrSubtractExpressionContext;
  class OC_MultiplyDivideModuloExpressionContext;
  class OC_PowerOfExpressionContext;
  class OC_UnaryAddOrSubtractExpressionContext;
  class OC_StringListNullOperatorExpressionContext;
  class OC_ListOperatorExpressionContext;
  class OC_StringOperatorExpressionContext;
  class OC_NullOperatorExpressionContext;
  class OC_PropertyOrLabelsExpressionContext;
  class OC_AtomContext;
  class OC_LiteralContext;
  class OC_BooleanLiteralContext;
  class OC_ListLiteralContext;
  class OC_PartialComparisonExpressionContext;
  class OC_ParenthesizedExpressionContext;
  class OC_RelationshipsPatternContext;
  class OC_FilterExpressionContext;
  class OC_IdInCollContext;
  class OC_FunctionInvocationContext;
  class OC_FunctionNameContext;
  class OC_ExplicitProcedureInvocationContext;
  class OC_ImplicitProcedureInvocationContext;
  class OC_ProcedureResultFieldContext;
  class OC_ProcedureNameContext;
  class OC_NamespaceContext;
  class OC_ListComprehensionContext;
  class OC_PatternComprehensionContext;
  class OC_PropertyLookupContext;
  class OC_CaseExpressionContext;
  class OC_CaseAlternativesContext;
  class OC_VariableContext;
  class OC_NumberLiteralContext;
  class OC_MapLiteralContext;
  class OC_ParameterContext;
  class OC_PropertyExpressionContext;
  class OC_PropertyKeyNameContext;
  class OC_IntegerLiteralContext;
  class OC_DoubleLiteralContext;
  class OC_SchemaNameContext;
  class OC_SymbolicNameContext;
  class OC_ReservedWordContext;
  class OC_LeftArrowHeadContext;
  class OC_RightArrowHeadContext;
  class OC_DashContext; 

  class  OC_CypherContext : public antlr4::ParserRuleContext {
  public:
    OC_CypherContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_StatementContext *oC_Statement();
    antlr4::tree::TerminalNode *EOF();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_CypherContext* oC_Cypher();

  class  OC_StatementContext : public antlr4::ParserRuleContext {
  public:
    OC_StatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_QueryContext *oC_Query();
    antlr4::tree::TerminalNode *EXPLAIN();
    antlr4::tree::TerminalNode *SP();
    antlr4::tree::TerminalNode *PROFILE();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_StatementContext* oC_Statement();

  class  OC_QueryContext : public antlr4::ParserRuleContext {
  public:
    OC_QueryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_RegularQueryContext *oC_RegularQuery();
    OC_StandaloneCallContext *oC_StandaloneCall();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_QueryContext* oC_Query();

  class  OC_RegularQueryContext : public antlr4::ParserRuleContext {
  public:
    OC_RegularQueryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_SingleQueryContext *oC_SingleQuery();
    std::vector<OC_UnionContext *> oC_Union();
    OC_UnionContext* oC_Union(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_RegularQueryContext* oC_RegularQuery();

  class  OC_UnionContext : public antlr4::ParserRuleContext {
  public:
    OC_UnionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *UNION();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    antlr4::tree::TerminalNode *ALL();
    OC_SingleQueryContext *oC_SingleQuery();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_UnionContext* oC_Union();

  class  OC_SingleQueryContext : public antlr4::ParserRuleContext {
  public:
    OC_SingleQueryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_SinglePartQueryContext *oC_SinglePartQuery();
    OC_MultiPartQueryContext *oC_MultiPartQuery();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_SingleQueryContext* oC_SingleQuery();

  class  OC_SinglePartQueryContext : public antlr4::ParserRuleContext {
  public:
    OC_SinglePartQueryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_ReturnContext *oC_Return();
    std::vector<OC_ReadingClauseContext *> oC_ReadingClause();
    OC_ReadingClauseContext* oC_ReadingClause(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    std::vector<OC_UpdatingClauseContext *> oC_UpdatingClause();
    OC_UpdatingClauseContext* oC_UpdatingClause(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_SinglePartQueryContext* oC_SinglePartQuery();

  class  OC_MultiPartQueryContext : public antlr4::ParserRuleContext {
  public:
    OC_MultiPartQueryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_SinglePartQueryContext *oC_SinglePartQuery();
    std::vector<OC_WithContext *> oC_With();
    OC_WithContext* oC_With(size_t i);
    std::vector<OC_ReadingClauseContext *> oC_ReadingClause();
    OC_ReadingClauseContext* oC_ReadingClause(size_t i);
    std::vector<OC_UpdatingClauseContext *> oC_UpdatingClause();
    OC_UpdatingClauseContext* oC_UpdatingClause(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_MultiPartQueryContext* oC_MultiPartQuery();

  class  OC_UpdatingClauseContext : public antlr4::ParserRuleContext {
  public:
    OC_UpdatingClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_CreateContext *oC_Create();
    OC_MergeContext *oC_Merge();
    OC_DeleteContext *oC_Delete();
    OC_SetContext *oC_Set();
    OC_RemoveContext *oC_Remove();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_UpdatingClauseContext* oC_UpdatingClause();

  class  OC_ReadingClauseContext : public antlr4::ParserRuleContext {
  public:
    OC_ReadingClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_MatchContext *oC_Match();
    OC_UnwindContext *oC_Unwind();
    OC_InQueryCallContext *oC_InQueryCall();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_ReadingClauseContext* oC_ReadingClause();

  class  OC_MatchContext : public antlr4::ParserRuleContext {
  public:
    OC_MatchContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *MATCH();
    OC_PatternContext *oC_Pattern();
    antlr4::tree::TerminalNode *OPTIONAL_();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    std::vector<OC_HintContext *> oC_Hint();
    OC_HintContext* oC_Hint(size_t i);
    OC_WhereContext *oC_Where();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_MatchContext* oC_Match();

  class  OC_UnwindContext : public antlr4::ParserRuleContext {
  public:
    OC_UnwindContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *UNWIND();
    OC_ExpressionContext *oC_Expression();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    antlr4::tree::TerminalNode *AS();
    OC_VariableContext *oC_Variable();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_UnwindContext* oC_Unwind();

  class  OC_MergeContext : public antlr4::ParserRuleContext {
  public:
    OC_MergeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *MERGE();
    OC_PatternPartContext *oC_PatternPart();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    std::vector<OC_MergeActionContext *> oC_MergeAction();
    OC_MergeActionContext* oC_MergeAction(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_MergeContext* oC_Merge();

  class  OC_MergeActionContext : public antlr4::ParserRuleContext {
  public:
    OC_MergeActionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ON();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    antlr4::tree::TerminalNode *MATCH();
    OC_SetContext *oC_Set();
    antlr4::tree::TerminalNode *CREATE();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_MergeActionContext* oC_MergeAction();

  class  OC_CreateContext : public antlr4::ParserRuleContext {
  public:
    OC_CreateContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CREATE();
    OC_PatternContext *oC_Pattern();
    antlr4::tree::TerminalNode *SP();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_CreateContext* oC_Create();

  class  OC_SetContext : public antlr4::ParserRuleContext {
  public:
    OC_SetContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *SET();
    std::vector<OC_SetItemContext *> oC_SetItem();
    OC_SetItemContext* oC_SetItem(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_SetContext* oC_Set();

  class  OC_SetItemContext : public antlr4::ParserRuleContext {
  public:
    OC_SetItemContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_PropertyExpressionContext *oC_PropertyExpression();
    OC_ExpressionContext *oC_Expression();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    OC_VariableContext *oC_Variable();
    OC_NodeLabelsContext *oC_NodeLabels();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_SetItemContext* oC_SetItem();

  class  OC_DeleteContext : public antlr4::ParserRuleContext {
  public:
    OC_DeleteContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DELETE_();
    std::vector<OC_ExpressionContext *> oC_Expression();
    OC_ExpressionContext* oC_Expression(size_t i);
    antlr4::tree::TerminalNode *DETACH();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_DeleteContext* oC_Delete();

  class  OC_RemoveContext : public antlr4::ParserRuleContext {
  public:
    OC_RemoveContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *REMOVE();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    std::vector<OC_RemoveItemContext *> oC_RemoveItem();
    OC_RemoveItemContext* oC_RemoveItem(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_RemoveContext* oC_Remove();

  class  OC_RemoveItemContext : public antlr4::ParserRuleContext {
  public:
    OC_RemoveItemContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_VariableContext *oC_Variable();
    OC_NodeLabelsContext *oC_NodeLabels();
    OC_PropertyExpressionContext *oC_PropertyExpression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_RemoveItemContext* oC_RemoveItem();

  class  OC_InQueryCallContext : public antlr4::ParserRuleContext {
  public:
    OC_InQueryCallContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CALL();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    OC_ExplicitProcedureInvocationContext *oC_ExplicitProcedureInvocation();
    antlr4::tree::TerminalNode *YIELD();
    OC_YieldItemsContext *oC_YieldItems();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_InQueryCallContext* oC_InQueryCall();

  class  OC_StandaloneCallContext : public antlr4::ParserRuleContext {
  public:
    OC_StandaloneCallContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CALL();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    OC_ExplicitProcedureInvocationContext *oC_ExplicitProcedureInvocation();
    OC_ImplicitProcedureInvocationContext *oC_ImplicitProcedureInvocation();
    antlr4::tree::TerminalNode *YIELD();
    OC_YieldItemsContext *oC_YieldItems();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_StandaloneCallContext* oC_StandaloneCall();

  class  OC_YieldItemsContext : public antlr4::ParserRuleContext {
  public:
    OC_YieldItemsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_WhereContext *oC_Where();
    std::vector<OC_YieldItemContext *> oC_YieldItem();
    OC_YieldItemContext* oC_YieldItem(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_YieldItemsContext* oC_YieldItems();

  class  OC_YieldItemContext : public antlr4::ParserRuleContext {
  public:
    OC_YieldItemContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_VariableContext *oC_Variable();
    OC_ProcedureResultFieldContext *oC_ProcedureResultField();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    antlr4::tree::TerminalNode *AS();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_YieldItemContext* oC_YieldItem();

  class  OC_WithContext : public antlr4::ParserRuleContext {
  public:
    OC_WithContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *WITH();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    OC_ReturnBodyContext *oC_ReturnBody();
    antlr4::tree::TerminalNode *DISTINCT();
    OC_WhereContext *oC_Where();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_WithContext* oC_With();

  class  OC_ReturnContext : public antlr4::ParserRuleContext {
  public:
    OC_ReturnContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *RETURN();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    OC_ReturnBodyContext *oC_ReturnBody();
    antlr4::tree::TerminalNode *DISTINCT();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_ReturnContext* oC_Return();

  class  OC_ReturnBodyContext : public antlr4::ParserRuleContext {
  public:
    OC_ReturnBodyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_ReturnItemsContext *oC_ReturnItems();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    OC_OrderContext *oC_Order();
    OC_SkipContext *oC_Skip();
    OC_LimitContext *oC_Limit();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_ReturnBodyContext* oC_ReturnBody();

  class  OC_ReturnItemsContext : public antlr4::ParserRuleContext {
  public:
    OC_ReturnItemsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<OC_ReturnItemContext *> oC_ReturnItem();
    OC_ReturnItemContext* oC_ReturnItem(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_ReturnItemsContext* oC_ReturnItems();

  class  OC_ReturnItemContext : public antlr4::ParserRuleContext {
  public:
    OC_ReturnItemContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_ExpressionContext *oC_Expression();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    antlr4::tree::TerminalNode *AS();
    OC_VariableContext *oC_Variable();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_ReturnItemContext* oC_ReturnItem();

  class  OC_OrderContext : public antlr4::ParserRuleContext {
  public:
    OC_OrderContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ORDER();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    antlr4::tree::TerminalNode *BY();
    std::vector<OC_SortItemContext *> oC_SortItem();
    OC_SortItemContext* oC_SortItem(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_OrderContext* oC_Order();

  class  OC_SkipContext : public antlr4::ParserRuleContext {
  public:
    OC_SkipContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *L_SKIP();
    antlr4::tree::TerminalNode *SP();
    OC_ExpressionContext *oC_Expression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_SkipContext* oC_Skip();

  class  OC_LimitContext : public antlr4::ParserRuleContext {
  public:
    OC_LimitContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LIMIT();
    antlr4::tree::TerminalNode *SP();
    OC_ExpressionContext *oC_Expression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_LimitContext* oC_Limit();

  class  OC_SortItemContext : public antlr4::ParserRuleContext {
  public:
    OC_SortItemContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_ExpressionContext *oC_Expression();
    antlr4::tree::TerminalNode *ASCENDING();
    antlr4::tree::TerminalNode *ASC();
    antlr4::tree::TerminalNode *DESCENDING();
    antlr4::tree::TerminalNode *DESC();
    antlr4::tree::TerminalNode *SP();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_SortItemContext* oC_SortItem();

  class  OC_HintContext : public antlr4::ParserRuleContext {
  public:
    OC_HintContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *USING();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    antlr4::tree::TerminalNode *JOIN();
    antlr4::tree::TerminalNode *ON();
    OC_VariableContext *oC_Variable();
    antlr4::tree::TerminalNode *START();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_HintContext* oC_Hint();

  class  OC_WhereContext : public antlr4::ParserRuleContext {
  public:
    OC_WhereContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *WHERE();
    antlr4::tree::TerminalNode *SP();
    OC_ExpressionContext *oC_Expression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_WhereContext* oC_Where();

  class  OC_PatternContext : public antlr4::ParserRuleContext {
  public:
    OC_PatternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<OC_PatternPartContext *> oC_PatternPart();
    OC_PatternPartContext* oC_PatternPart(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_PatternContext* oC_Pattern();

  class  OC_PatternPartContext : public antlr4::ParserRuleContext {
  public:
    OC_PatternPartContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_VariableContext *oC_Variable();
    OC_AnonymousPatternPartContext *oC_AnonymousPatternPart();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_PatternPartContext* oC_PatternPart();

  class  OC_AnonymousPatternPartContext : public antlr4::ParserRuleContext {
  public:
    OC_AnonymousPatternPartContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_PatternElementContext *oC_PatternElement();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_AnonymousPatternPartContext* oC_AnonymousPatternPart();

  class  OC_PatternElementContext : public antlr4::ParserRuleContext {
  public:
    OC_PatternElementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_NodePatternContext *oC_NodePattern();
    std::vector<OC_PatternElementChainContext *> oC_PatternElementChain();
    OC_PatternElementChainContext* oC_PatternElementChain(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    OC_PatternElementContext *oC_PatternElement();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_PatternElementContext* oC_PatternElement();

  class  OC_NodePatternContext : public antlr4::ParserRuleContext {
  public:
    OC_NodePatternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    OC_VariableContext *oC_Variable();
    OC_NodeLabelsContext *oC_NodeLabels();
    OC_PropertiesContext *oC_Properties();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_NodePatternContext* oC_NodePattern();

  class  OC_PatternElementChainContext : public antlr4::ParserRuleContext {
  public:
    OC_PatternElementChainContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_RelationshipPatternContext *oC_RelationshipPattern();
    OC_NodePatternContext *oC_NodePattern();
    antlr4::tree::TerminalNode *SP();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_PatternElementChainContext* oC_PatternElementChain();

  class  OC_RelationshipPatternContext : public antlr4::ParserRuleContext {
  public:
    OC_RelationshipPatternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_LeftArrowHeadContext *oC_LeftArrowHead();
    std::vector<OC_DashContext *> oC_Dash();
    OC_DashContext* oC_Dash(size_t i);
    OC_RightArrowHeadContext *oC_RightArrowHead();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    OC_RelationshipDetailContext *oC_RelationshipDetail();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_RelationshipPatternContext* oC_RelationshipPattern();

  class  OC_RelationshipDetailContext : public antlr4::ParserRuleContext {
  public:
    OC_RelationshipDetailContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    OC_VariableContext *oC_Variable();
    OC_RelationshipTypesContext *oC_RelationshipTypes();
    OC_RangeLiteralContext *oC_RangeLiteral();
    OC_PropertiesContext *oC_Properties();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_RelationshipDetailContext* oC_RelationshipDetail();

  class  OC_PropertiesContext : public antlr4::ParserRuleContext {
  public:
    OC_PropertiesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_MapLiteralContext *oC_MapLiteral();
    OC_ParameterContext *oC_Parameter();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_PropertiesContext* oC_Properties();

  class  OC_RelationshipTypesContext : public antlr4::ParserRuleContext {
  public:
    OC_RelationshipTypesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<OC_RelTypeNameContext *> oC_RelTypeName();
    OC_RelTypeNameContext* oC_RelTypeName(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_RelationshipTypesContext* oC_RelationshipTypes();

  class  OC_NodeLabelsContext : public antlr4::ParserRuleContext {
  public:
    OC_NodeLabelsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<OC_NodeLabelContext *> oC_NodeLabel();
    OC_NodeLabelContext* oC_NodeLabel(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_NodeLabelsContext* oC_NodeLabels();

  class  OC_NodeLabelContext : public antlr4::ParserRuleContext {
  public:
    OC_NodeLabelContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_LabelNameContext *oC_LabelName();
    antlr4::tree::TerminalNode *SP();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_NodeLabelContext* oC_NodeLabel();

  class  OC_RangeLiteralContext : public antlr4::ParserRuleContext {
  public:
    OC_RangeLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    std::vector<OC_IntegerLiteralContext *> oC_IntegerLiteral();
    OC_IntegerLiteralContext* oC_IntegerLiteral(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_RangeLiteralContext* oC_RangeLiteral();

  class  OC_LabelNameContext : public antlr4::ParserRuleContext {
  public:
    OC_LabelNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_SchemaNameContext *oC_SchemaName();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_LabelNameContext* oC_LabelName();

  class  OC_RelTypeNameContext : public antlr4::ParserRuleContext {
  public:
    OC_RelTypeNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_SchemaNameContext *oC_SchemaName();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_RelTypeNameContext* oC_RelTypeName();

  class  OC_ExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_ExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_OrExpressionContext *oC_OrExpression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_ExpressionContext* oC_Expression();

  class  OC_OrExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_OrExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<OC_XorExpressionContext *> oC_XorExpression();
    OC_XorExpressionContext* oC_XorExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    std::vector<antlr4::tree::TerminalNode *> OR();
    antlr4::tree::TerminalNode* OR(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_OrExpressionContext* oC_OrExpression();

  class  OC_XorExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_XorExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<OC_AndExpressionContext *> oC_AndExpression();
    OC_AndExpressionContext* oC_AndExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    std::vector<antlr4::tree::TerminalNode *> XOR();
    antlr4::tree::TerminalNode* XOR(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_XorExpressionContext* oC_XorExpression();

  class  OC_AndExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_AndExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<OC_NotExpressionContext *> oC_NotExpression();
    OC_NotExpressionContext* oC_NotExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    std::vector<antlr4::tree::TerminalNode *> AND();
    antlr4::tree::TerminalNode* AND(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_AndExpressionContext* oC_AndExpression();

  class  OC_NotExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_NotExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_ComparisonExpressionContext *oC_ComparisonExpression();
    std::vector<antlr4::tree::TerminalNode *> NOT();
    antlr4::tree::TerminalNode* NOT(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_NotExpressionContext* oC_NotExpression();

  class  OC_ComparisonExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_ComparisonExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_AddOrSubtractExpressionContext *oC_AddOrSubtractExpression();
    std::vector<OC_PartialComparisonExpressionContext *> oC_PartialComparisonExpression();
    OC_PartialComparisonExpressionContext* oC_PartialComparisonExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_ComparisonExpressionContext* oC_ComparisonExpression();

  class  OC_AddOrSubtractExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_AddOrSubtractExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<OC_MultiplyDivideModuloExpressionContext *> oC_MultiplyDivideModuloExpression();
    OC_MultiplyDivideModuloExpressionContext* oC_MultiplyDivideModuloExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_AddOrSubtractExpressionContext* oC_AddOrSubtractExpression();

  class  OC_MultiplyDivideModuloExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_MultiplyDivideModuloExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<OC_PowerOfExpressionContext *> oC_PowerOfExpression();
    OC_PowerOfExpressionContext* oC_PowerOfExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_MultiplyDivideModuloExpressionContext* oC_MultiplyDivideModuloExpression();

  class  OC_PowerOfExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_PowerOfExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<OC_UnaryAddOrSubtractExpressionContext *> oC_UnaryAddOrSubtractExpression();
    OC_UnaryAddOrSubtractExpressionContext* oC_UnaryAddOrSubtractExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_PowerOfExpressionContext* oC_PowerOfExpression();

  class  OC_UnaryAddOrSubtractExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_UnaryAddOrSubtractExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_StringListNullOperatorExpressionContext *oC_StringListNullOperatorExpression();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_UnaryAddOrSubtractExpressionContext* oC_UnaryAddOrSubtractExpression();

  class  OC_StringListNullOperatorExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_StringListNullOperatorExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_PropertyOrLabelsExpressionContext *oC_PropertyOrLabelsExpression();
    std::vector<OC_StringOperatorExpressionContext *> oC_StringOperatorExpression();
    OC_StringOperatorExpressionContext* oC_StringOperatorExpression(size_t i);
    std::vector<OC_ListOperatorExpressionContext *> oC_ListOperatorExpression();
    OC_ListOperatorExpressionContext* oC_ListOperatorExpression(size_t i);
    std::vector<OC_NullOperatorExpressionContext *> oC_NullOperatorExpression();
    OC_NullOperatorExpressionContext* oC_NullOperatorExpression(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_StringListNullOperatorExpressionContext* oC_StringListNullOperatorExpression();

  class  OC_ListOperatorExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_ListOperatorExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    antlr4::tree::TerminalNode *IN();
    OC_PropertyOrLabelsExpressionContext *oC_PropertyOrLabelsExpression();
    std::vector<OC_ExpressionContext *> oC_Expression();
    OC_ExpressionContext* oC_Expression(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_ListOperatorExpressionContext* oC_ListOperatorExpression();

  class  OC_StringOperatorExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_StringOperatorExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_PropertyOrLabelsExpressionContext *oC_PropertyOrLabelsExpression();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    antlr4::tree::TerminalNode *STARTS();
    antlr4::tree::TerminalNode *WITH();
    antlr4::tree::TerminalNode *ENDS();
    antlr4::tree::TerminalNode *CONTAINS();
    antlr4::tree::TerminalNode *REGEXP();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_StringOperatorExpressionContext* oC_StringOperatorExpression();

  class  OC_NullOperatorExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_NullOperatorExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    antlr4::tree::TerminalNode *IS();
    antlr4::tree::TerminalNode *NULL_();
    antlr4::tree::TerminalNode *NOT();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_NullOperatorExpressionContext* oC_NullOperatorExpression();

  class  OC_PropertyOrLabelsExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_PropertyOrLabelsExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_AtomContext *oC_Atom();
    std::vector<OC_PropertyLookupContext *> oC_PropertyLookup();
    OC_PropertyLookupContext* oC_PropertyLookup(size_t i);
    OC_NodeLabelsContext *oC_NodeLabels();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_PropertyOrLabelsExpressionContext* oC_PropertyOrLabelsExpression();

  class  OC_AtomContext : public antlr4::ParserRuleContext {
  public:
    OC_AtomContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_LiteralContext *oC_Literal();
    OC_ParameterContext *oC_Parameter();
    OC_CaseExpressionContext *oC_CaseExpression();
    antlr4::tree::TerminalNode *COUNT();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    OC_ListComprehensionContext *oC_ListComprehension();
    OC_PatternComprehensionContext *oC_PatternComprehension();
    antlr4::tree::TerminalNode *ALL();
    OC_FilterExpressionContext *oC_FilterExpression();
    antlr4::tree::TerminalNode *ANY();
    antlr4::tree::TerminalNode *NONE();
    antlr4::tree::TerminalNode *SINGLE();
    OC_RelationshipsPatternContext *oC_RelationshipsPattern();
    OC_ParenthesizedExpressionContext *oC_ParenthesizedExpression();
    OC_FunctionInvocationContext *oC_FunctionInvocation();
    OC_VariableContext *oC_Variable();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_AtomContext* oC_Atom();

  class  OC_LiteralContext : public antlr4::ParserRuleContext {
  public:
    OC_LiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_NumberLiteralContext *oC_NumberLiteral();
    antlr4::tree::TerminalNode *StringLiteral();
    OC_BooleanLiteralContext *oC_BooleanLiteral();
    antlr4::tree::TerminalNode *NULL_();
    OC_MapLiteralContext *oC_MapLiteral();
    OC_ListLiteralContext *oC_ListLiteral();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_LiteralContext* oC_Literal();

  class  OC_BooleanLiteralContext : public antlr4::ParserRuleContext {
  public:
    OC_BooleanLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *TRUE_();
    antlr4::tree::TerminalNode *FALSE_();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_BooleanLiteralContext* oC_BooleanLiteral();

  class  OC_ListLiteralContext : public antlr4::ParserRuleContext {
  public:
    OC_ListLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    std::vector<OC_ExpressionContext *> oC_Expression();
    OC_ExpressionContext* oC_Expression(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_ListLiteralContext* oC_ListLiteral();

  class  OC_PartialComparisonExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_PartialComparisonExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_AddOrSubtractExpressionContext *oC_AddOrSubtractExpression();
    antlr4::tree::TerminalNode *SP();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_PartialComparisonExpressionContext* oC_PartialComparisonExpression();

  class  OC_ParenthesizedExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_ParenthesizedExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_ExpressionContext *oC_Expression();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_ParenthesizedExpressionContext* oC_ParenthesizedExpression();

  class  OC_RelationshipsPatternContext : public antlr4::ParserRuleContext {
  public:
    OC_RelationshipsPatternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_NodePatternContext *oC_NodePattern();
    std::vector<OC_PatternElementChainContext *> oC_PatternElementChain();
    OC_PatternElementChainContext* oC_PatternElementChain(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_RelationshipsPatternContext* oC_RelationshipsPattern();

  class  OC_FilterExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_FilterExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_IdInCollContext *oC_IdInColl();
    OC_WhereContext *oC_Where();
    antlr4::tree::TerminalNode *SP();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_FilterExpressionContext* oC_FilterExpression();

  class  OC_IdInCollContext : public antlr4::ParserRuleContext {
  public:
    OC_IdInCollContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_VariableContext *oC_Variable();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    antlr4::tree::TerminalNode *IN();
    OC_ExpressionContext *oC_Expression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_IdInCollContext* oC_IdInColl();

  class  OC_FunctionInvocationContext : public antlr4::ParserRuleContext {
  public:
    OC_FunctionInvocationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_FunctionNameContext *oC_FunctionName();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    antlr4::tree::TerminalNode *DISTINCT();
    std::vector<OC_ExpressionContext *> oC_Expression();
    OC_ExpressionContext* oC_Expression(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_FunctionInvocationContext* oC_FunctionInvocation();

  class  OC_FunctionNameContext : public antlr4::ParserRuleContext {
  public:
    OC_FunctionNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_NamespaceContext *oC_Namespace();
    OC_SymbolicNameContext *oC_SymbolicName();
    antlr4::tree::TerminalNode *EXISTS();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_FunctionNameContext* oC_FunctionName();

  class  OC_ExplicitProcedureInvocationContext : public antlr4::ParserRuleContext {
  public:
    OC_ExplicitProcedureInvocationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_ProcedureNameContext *oC_ProcedureName();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    std::vector<OC_ExpressionContext *> oC_Expression();
    OC_ExpressionContext* oC_Expression(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_ExplicitProcedureInvocationContext* oC_ExplicitProcedureInvocation();

  class  OC_ImplicitProcedureInvocationContext : public antlr4::ParserRuleContext {
  public:
    OC_ImplicitProcedureInvocationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_ProcedureNameContext *oC_ProcedureName();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_ImplicitProcedureInvocationContext* oC_ImplicitProcedureInvocation();

  class  OC_ProcedureResultFieldContext : public antlr4::ParserRuleContext {
  public:
    OC_ProcedureResultFieldContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_SymbolicNameContext *oC_SymbolicName();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_ProcedureResultFieldContext* oC_ProcedureResultField();

  class  OC_ProcedureNameContext : public antlr4::ParserRuleContext {
  public:
    OC_ProcedureNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_NamespaceContext *oC_Namespace();
    OC_SymbolicNameContext *oC_SymbolicName();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_ProcedureNameContext* oC_ProcedureName();

  class  OC_NamespaceContext : public antlr4::ParserRuleContext {
  public:
    OC_NamespaceContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<OC_SymbolicNameContext *> oC_SymbolicName();
    OC_SymbolicNameContext* oC_SymbolicName(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_NamespaceContext* oC_Namespace();

  class  OC_ListComprehensionContext : public antlr4::ParserRuleContext {
  public:
    OC_ListComprehensionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_FilterExpressionContext *oC_FilterExpression();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    OC_ExpressionContext *oC_Expression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_ListComprehensionContext* oC_ListComprehension();

  class  OC_PatternComprehensionContext : public antlr4::ParserRuleContext {
  public:
    OC_PatternComprehensionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_RelationshipsPatternContext *oC_RelationshipsPattern();
    std::vector<OC_ExpressionContext *> oC_Expression();
    OC_ExpressionContext* oC_Expression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    OC_VariableContext *oC_Variable();
    antlr4::tree::TerminalNode *WHERE();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_PatternComprehensionContext* oC_PatternComprehension();

  class  OC_PropertyLookupContext : public antlr4::ParserRuleContext {
  public:
    OC_PropertyLookupContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_PropertyKeyNameContext *oC_PropertyKeyName();
    antlr4::tree::TerminalNode *SP();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_PropertyLookupContext* oC_PropertyLookup();

  class  OC_CaseExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_CaseExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *END();
    antlr4::tree::TerminalNode *ELSE();
    std::vector<OC_ExpressionContext *> oC_Expression();
    OC_ExpressionContext* oC_Expression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    antlr4::tree::TerminalNode *CASE();
    std::vector<OC_CaseAlternativesContext *> oC_CaseAlternatives();
    OC_CaseAlternativesContext* oC_CaseAlternatives(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_CaseExpressionContext* oC_CaseExpression();

  class  OC_CaseAlternativesContext : public antlr4::ParserRuleContext {
  public:
    OC_CaseAlternativesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *WHEN();
    std::vector<OC_ExpressionContext *> oC_Expression();
    OC_ExpressionContext* oC_Expression(size_t i);
    antlr4::tree::TerminalNode *THEN();
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_CaseAlternativesContext* oC_CaseAlternatives();

  class  OC_VariableContext : public antlr4::ParserRuleContext {
  public:
    OC_VariableContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_SymbolicNameContext *oC_SymbolicName();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_VariableContext* oC_Variable();

  class  OC_NumberLiteralContext : public antlr4::ParserRuleContext {
  public:
    OC_NumberLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_DoubleLiteralContext *oC_DoubleLiteral();
    OC_IntegerLiteralContext *oC_IntegerLiteral();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_NumberLiteralContext* oC_NumberLiteral();

  class  OC_MapLiteralContext : public antlr4::ParserRuleContext {
  public:
    OC_MapLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);
    std::vector<OC_PropertyKeyNameContext *> oC_PropertyKeyName();
    OC_PropertyKeyNameContext* oC_PropertyKeyName(size_t i);
    std::vector<OC_ExpressionContext *> oC_Expression();
    OC_ExpressionContext* oC_Expression(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_MapLiteralContext* oC_MapLiteral();

  class  OC_ParameterContext : public antlr4::ParserRuleContext {
  public:
    OC_ParameterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_SymbolicNameContext *oC_SymbolicName();
    antlr4::tree::TerminalNode *DecimalInteger();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_ParameterContext* oC_Parameter();

  class  OC_PropertyExpressionContext : public antlr4::ParserRuleContext {
  public:
    OC_PropertyExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_AtomContext *oC_Atom();
    std::vector<OC_PropertyLookupContext *> oC_PropertyLookup();
    OC_PropertyLookupContext* oC_PropertyLookup(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SP();
    antlr4::tree::TerminalNode* SP(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_PropertyExpressionContext* oC_PropertyExpression();

  class  OC_PropertyKeyNameContext : public antlr4::ParserRuleContext {
  public:
    OC_PropertyKeyNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_SchemaNameContext *oC_SchemaName();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_PropertyKeyNameContext* oC_PropertyKeyName();

  class  OC_IntegerLiteralContext : public antlr4::ParserRuleContext {
  public:
    OC_IntegerLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *HexInteger();
    antlr4::tree::TerminalNode *OctalInteger();
    antlr4::tree::TerminalNode *DecimalInteger();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_IntegerLiteralContext* oC_IntegerLiteral();

  class  OC_DoubleLiteralContext : public antlr4::ParserRuleContext {
  public:
    OC_DoubleLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ExponentDecimalReal();
    antlr4::tree::TerminalNode *RegularDecimalReal();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_DoubleLiteralContext* oC_DoubleLiteral();

  class  OC_SchemaNameContext : public antlr4::ParserRuleContext {
  public:
    OC_SchemaNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    OC_SymbolicNameContext *oC_SymbolicName();
    OC_ReservedWordContext *oC_ReservedWord();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_SchemaNameContext* oC_SchemaName();

  class  OC_SymbolicNameContext : public antlr4::ParserRuleContext {
  public:
    OC_SymbolicNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *UnescapedSymbolicName();
    antlr4::tree::TerminalNode *EscapedSymbolicName();
    antlr4::tree::TerminalNode *HexLetter();
    antlr4::tree::TerminalNode *COUNT();
    antlr4::tree::TerminalNode *FILTER();
    antlr4::tree::TerminalNode *EXTRACT();
    antlr4::tree::TerminalNode *ANY();
    antlr4::tree::TerminalNode *NONE();
    antlr4::tree::TerminalNode *SINGLE();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_SymbolicNameContext* oC_SymbolicName();

  class  OC_ReservedWordContext : public antlr4::ParserRuleContext {
  public:
    OC_ReservedWordContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ALL();
    antlr4::tree::TerminalNode *ASC();
    antlr4::tree::TerminalNode *ASCENDING();
    antlr4::tree::TerminalNode *BY();
    antlr4::tree::TerminalNode *CREATE();
    antlr4::tree::TerminalNode *DELETE_();
    antlr4::tree::TerminalNode *DESC();
    antlr4::tree::TerminalNode *DESCENDING();
    antlr4::tree::TerminalNode *DETACH();
    antlr4::tree::TerminalNode *EXISTS();
    antlr4::tree::TerminalNode *LIMIT();
    antlr4::tree::TerminalNode *MATCH();
    antlr4::tree::TerminalNode *MERGE();
    antlr4::tree::TerminalNode *ON();
    antlr4::tree::TerminalNode *OPTIONAL_();
    antlr4::tree::TerminalNode *ORDER();
    antlr4::tree::TerminalNode *REMOVE();
    antlr4::tree::TerminalNode *RETURN();
    antlr4::tree::TerminalNode *SET();
    antlr4::tree::TerminalNode *L_SKIP();
    antlr4::tree::TerminalNode *WHERE();
    antlr4::tree::TerminalNode *WITH();
    antlr4::tree::TerminalNode *UNION();
    antlr4::tree::TerminalNode *UNWIND();
    antlr4::tree::TerminalNode *AND();
    antlr4::tree::TerminalNode *AS();
    antlr4::tree::TerminalNode *CONTAINS();
    antlr4::tree::TerminalNode *DISTINCT();
    antlr4::tree::TerminalNode *ENDS();
    antlr4::tree::TerminalNode *IN();
    antlr4::tree::TerminalNode *IS();
    antlr4::tree::TerminalNode *NOT();
    antlr4::tree::TerminalNode *OR();
    antlr4::tree::TerminalNode *STARTS();
    antlr4::tree::TerminalNode *XOR();
    antlr4::tree::TerminalNode *FALSE_();
    antlr4::tree::TerminalNode *TRUE_();
    antlr4::tree::TerminalNode *NULL_();
    antlr4::tree::TerminalNode *CONSTRAINT();
    antlr4::tree::TerminalNode *DO();
    antlr4::tree::TerminalNode *FOR();
    antlr4::tree::TerminalNode *REQUIRE();
    antlr4::tree::TerminalNode *UNIQUE();
    antlr4::tree::TerminalNode *CASE();
    antlr4::tree::TerminalNode *WHEN();
    antlr4::tree::TerminalNode *THEN();
    antlr4::tree::TerminalNode *ELSE();
    antlr4::tree::TerminalNode *END();
    antlr4::tree::TerminalNode *MANDATORY();
    antlr4::tree::TerminalNode *SCALAR();
    antlr4::tree::TerminalNode *OF();
    antlr4::tree::TerminalNode *ADD();
    antlr4::tree::TerminalNode *DROP();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_ReservedWordContext* oC_ReservedWord();

  class  OC_LeftArrowHeadContext : public antlr4::ParserRuleContext {
  public:
    OC_LeftArrowHeadContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_LeftArrowHeadContext* oC_LeftArrowHead();

  class  OC_RightArrowHeadContext : public antlr4::ParserRuleContext {
  public:
    OC_RightArrowHeadContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_RightArrowHeadContext* oC_RightArrowHead();

  class  OC_DashContext : public antlr4::ParserRuleContext {
  public:
    OC_DashContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OC_DashContext* oC_Dash();


  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

}  // namespace parser
