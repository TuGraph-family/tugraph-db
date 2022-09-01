
// Generated from Lcypher.g4 by ANTLR 4.9.2


#include "LcypherVisitor.h"

#include "LcypherParser.h"


using namespace antlrcpp;
using namespace parser;
using namespace antlr4;

LcypherParser::LcypherParser(TokenStream *input) : Parser(input) {
  _interpreter = new atn::ParserATNSimulator(this, _atn, _decisionToDFA, _sharedContextCache);
}

LcypherParser::~LcypherParser() {
  delete _interpreter;
}

std::string LcypherParser::getGrammarFileName() const {
  return "Lcypher.g4";
}

const std::vector<std::string>& LcypherParser::getRuleNames() const {
  return _ruleNames;
}

dfa::Vocabulary& LcypherParser::getVocabulary() const {
  return _vocabulary;
}


//----------------- OC_CypherContext ------------------------------------------------------------------

LcypherParser::OC_CypherContext::OC_CypherContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_StatementContext* LcypherParser::OC_CypherContext::oC_Statement() {
  return getRuleContext<LcypherParser::OC_StatementContext>(0);
}

tree::TerminalNode* LcypherParser::OC_CypherContext::EOF() {
  return getToken(LcypherParser::EOF, 0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_CypherContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_CypherContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_CypherContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Cypher;
}


antlrcpp::Any LcypherParser::OC_CypherContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Cypher(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_CypherContext* LcypherParser::oC_Cypher() {
  OC_CypherContext *_localctx = _tracker.createInstance<OC_CypherContext>(_ctx, getState());
  enterRule(_localctx, 0, LcypherParser::RuleOC_Cypher);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(201);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(200);
      match(LcypherParser::SP);
    }
    setState(203);
    oC_Statement();
    setState(208);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
    case 1: {
      setState(205);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(204);
        match(LcypherParser::SP);
      }
      setState(207);
      match(LcypherParser::T__0);
      break;
    }

    default:
      break;
    }
    setState(211);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(210);
      match(LcypherParser::SP);
    }
    setState(213);
    match(LcypherParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_StatementContext ------------------------------------------------------------------

LcypherParser::OC_StatementContext::OC_StatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_QueryContext* LcypherParser::OC_StatementContext::oC_Query() {
  return getRuleContext<LcypherParser::OC_QueryContext>(0);
}

tree::TerminalNode* LcypherParser::OC_StatementContext::EXPLAIN() {
  return getToken(LcypherParser::EXPLAIN, 0);
}

tree::TerminalNode* LcypherParser::OC_StatementContext::SP() {
  return getToken(LcypherParser::SP, 0);
}

tree::TerminalNode* LcypherParser::OC_StatementContext::PROFILE() {
  return getToken(LcypherParser::PROFILE, 0);
}


size_t LcypherParser::OC_StatementContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Statement;
}


antlrcpp::Any LcypherParser::OC_StatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Statement(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_StatementContext* LcypherParser::oC_Statement() {
  OC_StatementContext *_localctx = _tracker.createInstance<OC_StatementContext>(_ctx, getState());
  enterRule(_localctx, 2, LcypherParser::RuleOC_Statement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(226);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case LcypherParser::OPTIONAL_:
      case LcypherParser::MATCH:
      case LcypherParser::UNWIND:
      case LcypherParser::MERGE:
      case LcypherParser::CREATE:
      case LcypherParser::SET:
      case LcypherParser::DETACH:
      case LcypherParser::DELETE_:
      case LcypherParser::REMOVE:
      case LcypherParser::CALL:
      case LcypherParser::WITH:
      case LcypherParser::RETURN: {
        enterOuterAlt(_localctx, 1);
        setState(215);
        oC_Query();
        break;
      }

      case LcypherParser::EXPLAIN: {
        enterOuterAlt(_localctx, 2);
        setState(216);
        match(LcypherParser::EXPLAIN);
        setState(218);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(217);
          match(LcypherParser::SP);
        }
        setState(220);
        oC_Query();
        break;
      }

      case LcypherParser::PROFILE: {
        enterOuterAlt(_localctx, 3);
        setState(221);
        match(LcypherParser::PROFILE);
        setState(223);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(222);
          match(LcypherParser::SP);
        }
        setState(225);
        oC_Query();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_QueryContext ------------------------------------------------------------------

LcypherParser::OC_QueryContext::OC_QueryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_RegularQueryContext* LcypherParser::OC_QueryContext::oC_RegularQuery() {
  return getRuleContext<LcypherParser::OC_RegularQueryContext>(0);
}

LcypherParser::OC_StandaloneCallContext* LcypherParser::OC_QueryContext::oC_StandaloneCall() {
  return getRuleContext<LcypherParser::OC_StandaloneCallContext>(0);
}


size_t LcypherParser::OC_QueryContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Query;
}


antlrcpp::Any LcypherParser::OC_QueryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Query(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_QueryContext* LcypherParser::oC_Query() {
  OC_QueryContext *_localctx = _tracker.createInstance<OC_QueryContext>(_ctx, getState());
  enterRule(_localctx, 4, LcypherParser::RuleOC_Query);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(230);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 7, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(228);
      oC_RegularQuery();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(229);
      oC_StandaloneCall();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_RegularQueryContext ------------------------------------------------------------------

LcypherParser::OC_RegularQueryContext::OC_RegularQueryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_SingleQueryContext* LcypherParser::OC_RegularQueryContext::oC_SingleQuery() {
  return getRuleContext<LcypherParser::OC_SingleQueryContext>(0);
}

std::vector<LcypherParser::OC_UnionContext *> LcypherParser::OC_RegularQueryContext::oC_Union() {
  return getRuleContexts<LcypherParser::OC_UnionContext>();
}

LcypherParser::OC_UnionContext* LcypherParser::OC_RegularQueryContext::oC_Union(size_t i) {
  return getRuleContext<LcypherParser::OC_UnionContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_RegularQueryContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_RegularQueryContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_RegularQueryContext::getRuleIndex() const {
  return LcypherParser::RuleOC_RegularQuery;
}


antlrcpp::Any LcypherParser::OC_RegularQueryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_RegularQuery(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_RegularQueryContext* LcypherParser::oC_RegularQuery() {
  OC_RegularQueryContext *_localctx = _tracker.createInstance<OC_RegularQueryContext>(_ctx, getState());
  enterRule(_localctx, 6, LcypherParser::RuleOC_RegularQuery);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(232);
    oC_SingleQuery();
    setState(239);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(234);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(233);
          match(LcypherParser::SP);
        }
        setState(236);
        oC_Union(); 
      }
      setState(241);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_UnionContext ------------------------------------------------------------------

LcypherParser::OC_UnionContext::OC_UnionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_UnionContext::UNION() {
  return getToken(LcypherParser::UNION, 0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_UnionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_UnionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

tree::TerminalNode* LcypherParser::OC_UnionContext::ALL() {
  return getToken(LcypherParser::ALL, 0);
}

LcypherParser::OC_SingleQueryContext* LcypherParser::OC_UnionContext::oC_SingleQuery() {
  return getRuleContext<LcypherParser::OC_SingleQueryContext>(0);
}


size_t LcypherParser::OC_UnionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Union;
}


antlrcpp::Any LcypherParser::OC_UnionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Union(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_UnionContext* LcypherParser::oC_Union() {
  OC_UnionContext *_localctx = _tracker.createInstance<OC_UnionContext>(_ctx, getState());
  enterRule(_localctx, 8, LcypherParser::RuleOC_Union);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(254);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(242);
      match(LcypherParser::UNION);
      setState(243);
      match(LcypherParser::SP);
      setState(244);
      match(LcypherParser::ALL);
      setState(246);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(245);
        match(LcypherParser::SP);
      }
      setState(248);
      oC_SingleQuery();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(249);
      match(LcypherParser::UNION);
      setState(251);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(250);
        match(LcypherParser::SP);
      }
      setState(253);
      oC_SingleQuery();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_SingleQueryContext ------------------------------------------------------------------

LcypherParser::OC_SingleQueryContext::OC_SingleQueryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_SinglePartQueryContext* LcypherParser::OC_SingleQueryContext::oC_SinglePartQuery() {
  return getRuleContext<LcypherParser::OC_SinglePartQueryContext>(0);
}

LcypherParser::OC_MultiPartQueryContext* LcypherParser::OC_SingleQueryContext::oC_MultiPartQuery() {
  return getRuleContext<LcypherParser::OC_MultiPartQueryContext>(0);
}


size_t LcypherParser::OC_SingleQueryContext::getRuleIndex() const {
  return LcypherParser::RuleOC_SingleQuery;
}


antlrcpp::Any LcypherParser::OC_SingleQueryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_SingleQuery(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_SingleQueryContext* LcypherParser::oC_SingleQuery() {
  OC_SingleQueryContext *_localctx = _tracker.createInstance<OC_SingleQueryContext>(_ctx, getState());
  enterRule(_localctx, 10, LcypherParser::RuleOC_SingleQuery);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(258);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(256);
      oC_SinglePartQuery();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(257);
      oC_MultiPartQuery();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_SinglePartQueryContext ------------------------------------------------------------------

LcypherParser::OC_SinglePartQueryContext::OC_SinglePartQueryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_ReturnContext* LcypherParser::OC_SinglePartQueryContext::oC_Return() {
  return getRuleContext<LcypherParser::OC_ReturnContext>(0);
}

std::vector<LcypherParser::OC_ReadingClauseContext *> LcypherParser::OC_SinglePartQueryContext::oC_ReadingClause() {
  return getRuleContexts<LcypherParser::OC_ReadingClauseContext>();
}

LcypherParser::OC_ReadingClauseContext* LcypherParser::OC_SinglePartQueryContext::oC_ReadingClause(size_t i) {
  return getRuleContext<LcypherParser::OC_ReadingClauseContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_SinglePartQueryContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_SinglePartQueryContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

std::vector<LcypherParser::OC_UpdatingClauseContext *> LcypherParser::OC_SinglePartQueryContext::oC_UpdatingClause() {
  return getRuleContexts<LcypherParser::OC_UpdatingClauseContext>();
}

LcypherParser::OC_UpdatingClauseContext* LcypherParser::OC_SinglePartQueryContext::oC_UpdatingClause(size_t i) {
  return getRuleContext<LcypherParser::OC_UpdatingClauseContext>(i);
}


size_t LcypherParser::OC_SinglePartQueryContext::getRuleIndex() const {
  return LcypherParser::RuleOC_SinglePartQuery;
}


antlrcpp::Any LcypherParser::OC_SinglePartQueryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_SinglePartQuery(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_SinglePartQueryContext* LcypherParser::oC_SinglePartQuery() {
  OC_SinglePartQueryContext *_localctx = _tracker.createInstance<OC_SinglePartQueryContext>(_ctx, getState());
  enterRule(_localctx, 12, LcypherParser::RuleOC_SinglePartQuery);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    setState(295);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 22, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(266);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << LcypherParser::OPTIONAL_)
        | (1ULL << LcypherParser::MATCH)
        | (1ULL << LcypherParser::UNWIND)
        | (1ULL << LcypherParser::CALL))) != 0)) {
        setState(260);
        oC_ReadingClause();
        setState(262);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(261);
          match(LcypherParser::SP);
        }
        setState(268);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(269);
      oC_Return();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(276);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << LcypherParser::OPTIONAL_)
        | (1ULL << LcypherParser::MATCH)
        | (1ULL << LcypherParser::UNWIND)
        | (1ULL << LcypherParser::CALL))) != 0)) {
        setState(270);
        oC_ReadingClause();
        setState(272);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(271);
          match(LcypherParser::SP);
        }
        setState(278);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(279);
      oC_UpdatingClause();
      setState(286);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 19, _ctx);
      while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
        if (alt == 1) {
          setState(281);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == LcypherParser::SP) {
            setState(280);
            match(LcypherParser::SP);
          }
          setState(283);
          oC_UpdatingClause(); 
        }
        setState(288);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 19, _ctx);
      }
      setState(293);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx)) {
      case 1: {
        setState(290);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(289);
          match(LcypherParser::SP);
        }
        setState(292);
        oC_Return();
        break;
      }

      default:
        break;
      }
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_MultiPartQueryContext ------------------------------------------------------------------

LcypherParser::OC_MultiPartQueryContext::OC_MultiPartQueryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_SinglePartQueryContext* LcypherParser::OC_MultiPartQueryContext::oC_SinglePartQuery() {
  return getRuleContext<LcypherParser::OC_SinglePartQueryContext>(0);
}

std::vector<LcypherParser::OC_WithContext *> LcypherParser::OC_MultiPartQueryContext::oC_With() {
  return getRuleContexts<LcypherParser::OC_WithContext>();
}

LcypherParser::OC_WithContext* LcypherParser::OC_MultiPartQueryContext::oC_With(size_t i) {
  return getRuleContext<LcypherParser::OC_WithContext>(i);
}

std::vector<LcypherParser::OC_ReadingClauseContext *> LcypherParser::OC_MultiPartQueryContext::oC_ReadingClause() {
  return getRuleContexts<LcypherParser::OC_ReadingClauseContext>();
}

LcypherParser::OC_ReadingClauseContext* LcypherParser::OC_MultiPartQueryContext::oC_ReadingClause(size_t i) {
  return getRuleContext<LcypherParser::OC_ReadingClauseContext>(i);
}

std::vector<LcypherParser::OC_UpdatingClauseContext *> LcypherParser::OC_MultiPartQueryContext::oC_UpdatingClause() {
  return getRuleContexts<LcypherParser::OC_UpdatingClauseContext>();
}

LcypherParser::OC_UpdatingClauseContext* LcypherParser::OC_MultiPartQueryContext::oC_UpdatingClause(size_t i) {
  return getRuleContext<LcypherParser::OC_UpdatingClauseContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_MultiPartQueryContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_MultiPartQueryContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_MultiPartQueryContext::getRuleIndex() const {
  return LcypherParser::RuleOC_MultiPartQuery;
}


antlrcpp::Any LcypherParser::OC_MultiPartQueryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_MultiPartQuery(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_MultiPartQueryContext* LcypherParser::oC_MultiPartQuery() {
  OC_MultiPartQueryContext *_localctx = _tracker.createInstance<OC_MultiPartQueryContext>(_ctx, getState());
  enterRule(_localctx, 14, LcypherParser::RuleOC_MultiPartQuery);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(319); 
    _errHandler->sync(this);
    alt = 1;
    do {
      switch (alt) {
        case 1: {
              setState(303);
              _errHandler->sync(this);
              _la = _input->LA(1);
              while ((((_la & ~ 0x3fULL) == 0) &&
                ((1ULL << _la) & ((1ULL << LcypherParser::OPTIONAL_)
                | (1ULL << LcypherParser::MATCH)
                | (1ULL << LcypherParser::UNWIND)
                | (1ULL << LcypherParser::CALL))) != 0)) {
                setState(297);
                oC_ReadingClause();
                setState(299);
                _errHandler->sync(this);

                _la = _input->LA(1);
                if (_la == LcypherParser::SP) {
                  setState(298);
                  match(LcypherParser::SP);
                }
                setState(305);
                _errHandler->sync(this);
                _la = _input->LA(1);
              }
              setState(312);
              _errHandler->sync(this);
              _la = _input->LA(1);
              while ((((_la & ~ 0x3fULL) == 0) &&
                ((1ULL << _la) & ((1ULL << LcypherParser::MERGE)
                | (1ULL << LcypherParser::CREATE)
                | (1ULL << LcypherParser::SET)
                | (1ULL << LcypherParser::DETACH)
                | (1ULL << LcypherParser::DELETE_)
                | (1ULL << LcypherParser::REMOVE))) != 0)) {
                setState(306);
                oC_UpdatingClause();
                setState(308);
                _errHandler->sync(this);

                _la = _input->LA(1);
                if (_la == LcypherParser::SP) {
                  setState(307);
                  match(LcypherParser::SP);
                }
                setState(314);
                _errHandler->sync(this);
                _la = _input->LA(1);
              }
              setState(315);
              oC_With();
              setState(317);
              _errHandler->sync(this);

              _la = _input->LA(1);
              if (_la == LcypherParser::SP) {
                setState(316);
                match(LcypherParser::SP);
              }
              break;
            }

      default:
        throw NoViableAltException(this);
      }
      setState(321); 
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 28, _ctx);
    } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
    setState(323);
    oC_SinglePartQuery();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_UpdatingClauseContext ------------------------------------------------------------------

LcypherParser::OC_UpdatingClauseContext::OC_UpdatingClauseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_CreateContext* LcypherParser::OC_UpdatingClauseContext::oC_Create() {
  return getRuleContext<LcypherParser::OC_CreateContext>(0);
}

LcypherParser::OC_MergeContext* LcypherParser::OC_UpdatingClauseContext::oC_Merge() {
  return getRuleContext<LcypherParser::OC_MergeContext>(0);
}

LcypherParser::OC_DeleteContext* LcypherParser::OC_UpdatingClauseContext::oC_Delete() {
  return getRuleContext<LcypherParser::OC_DeleteContext>(0);
}

LcypherParser::OC_SetContext* LcypherParser::OC_UpdatingClauseContext::oC_Set() {
  return getRuleContext<LcypherParser::OC_SetContext>(0);
}

LcypherParser::OC_RemoveContext* LcypherParser::OC_UpdatingClauseContext::oC_Remove() {
  return getRuleContext<LcypherParser::OC_RemoveContext>(0);
}


size_t LcypherParser::OC_UpdatingClauseContext::getRuleIndex() const {
  return LcypherParser::RuleOC_UpdatingClause;
}


antlrcpp::Any LcypherParser::OC_UpdatingClauseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_UpdatingClause(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_UpdatingClauseContext* LcypherParser::oC_UpdatingClause() {
  OC_UpdatingClauseContext *_localctx = _tracker.createInstance<OC_UpdatingClauseContext>(_ctx, getState());
  enterRule(_localctx, 16, LcypherParser::RuleOC_UpdatingClause);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(330);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case LcypherParser::CREATE: {
        enterOuterAlt(_localctx, 1);
        setState(325);
        oC_Create();
        break;
      }

      case LcypherParser::MERGE: {
        enterOuterAlt(_localctx, 2);
        setState(326);
        oC_Merge();
        break;
      }

      case LcypherParser::DETACH:
      case LcypherParser::DELETE_: {
        enterOuterAlt(_localctx, 3);
        setState(327);
        oC_Delete();
        break;
      }

      case LcypherParser::SET: {
        enterOuterAlt(_localctx, 4);
        setState(328);
        oC_Set();
        break;
      }

      case LcypherParser::REMOVE: {
        enterOuterAlt(_localctx, 5);
        setState(329);
        oC_Remove();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_ReadingClauseContext ------------------------------------------------------------------

LcypherParser::OC_ReadingClauseContext::OC_ReadingClauseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_MatchContext* LcypherParser::OC_ReadingClauseContext::oC_Match() {
  return getRuleContext<LcypherParser::OC_MatchContext>(0);
}

LcypherParser::OC_UnwindContext* LcypherParser::OC_ReadingClauseContext::oC_Unwind() {
  return getRuleContext<LcypherParser::OC_UnwindContext>(0);
}

LcypherParser::OC_InQueryCallContext* LcypherParser::OC_ReadingClauseContext::oC_InQueryCall() {
  return getRuleContext<LcypherParser::OC_InQueryCallContext>(0);
}


size_t LcypherParser::OC_ReadingClauseContext::getRuleIndex() const {
  return LcypherParser::RuleOC_ReadingClause;
}


antlrcpp::Any LcypherParser::OC_ReadingClauseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_ReadingClause(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_ReadingClauseContext* LcypherParser::oC_ReadingClause() {
  OC_ReadingClauseContext *_localctx = _tracker.createInstance<OC_ReadingClauseContext>(_ctx, getState());
  enterRule(_localctx, 18, LcypherParser::RuleOC_ReadingClause);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(335);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case LcypherParser::OPTIONAL_:
      case LcypherParser::MATCH: {
        enterOuterAlt(_localctx, 1);
        setState(332);
        oC_Match();
        break;
      }

      case LcypherParser::UNWIND: {
        enterOuterAlt(_localctx, 2);
        setState(333);
        oC_Unwind();
        break;
      }

      case LcypherParser::CALL: {
        enterOuterAlt(_localctx, 3);
        setState(334);
        oC_InQueryCall();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_MatchContext ------------------------------------------------------------------

LcypherParser::OC_MatchContext::OC_MatchContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_MatchContext::MATCH() {
  return getToken(LcypherParser::MATCH, 0);
}

LcypherParser::OC_PatternContext* LcypherParser::OC_MatchContext::oC_Pattern() {
  return getRuleContext<LcypherParser::OC_PatternContext>(0);
}

tree::TerminalNode* LcypherParser::OC_MatchContext::OPTIONAL_() {
  return getToken(LcypherParser::OPTIONAL_, 0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_MatchContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_MatchContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

std::vector<LcypherParser::OC_HintContext *> LcypherParser::OC_MatchContext::oC_Hint() {
  return getRuleContexts<LcypherParser::OC_HintContext>();
}

LcypherParser::OC_HintContext* LcypherParser::OC_MatchContext::oC_Hint(size_t i) {
  return getRuleContext<LcypherParser::OC_HintContext>(i);
}

LcypherParser::OC_WhereContext* LcypherParser::OC_MatchContext::oC_Where() {
  return getRuleContext<LcypherParser::OC_WhereContext>(0);
}


size_t LcypherParser::OC_MatchContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Match;
}


antlrcpp::Any LcypherParser::OC_MatchContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Match(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_MatchContext* LcypherParser::oC_Match() {
  OC_MatchContext *_localctx = _tracker.createInstance<OC_MatchContext>(_ctx, getState());
  enterRule(_localctx, 20, LcypherParser::RuleOC_Match);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(339);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::OPTIONAL_) {
      setState(337);
      match(LcypherParser::OPTIONAL_);
      setState(338);
      match(LcypherParser::SP);
    }
    setState(341);
    match(LcypherParser::MATCH);
    setState(343);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(342);
      match(LcypherParser::SP);
    }
    setState(345);
    oC_Pattern();
    setState(352);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(347);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(346);
          match(LcypherParser::SP);
        }
        setState(349);
        oC_Hint(); 
      }
      setState(354);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx);
    }
    setState(359);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 36, _ctx)) {
    case 1: {
      setState(356);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(355);
        match(LcypherParser::SP);
      }
      setState(358);
      oC_Where();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_UnwindContext ------------------------------------------------------------------

LcypherParser::OC_UnwindContext::OC_UnwindContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_UnwindContext::UNWIND() {
  return getToken(LcypherParser::UNWIND, 0);
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_UnwindContext::oC_Expression() {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_UnwindContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_UnwindContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

tree::TerminalNode* LcypherParser::OC_UnwindContext::AS() {
  return getToken(LcypherParser::AS, 0);
}

LcypherParser::OC_VariableContext* LcypherParser::OC_UnwindContext::oC_Variable() {
  return getRuleContext<LcypherParser::OC_VariableContext>(0);
}


size_t LcypherParser::OC_UnwindContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Unwind;
}


antlrcpp::Any LcypherParser::OC_UnwindContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Unwind(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_UnwindContext* LcypherParser::oC_Unwind() {
  OC_UnwindContext *_localctx = _tracker.createInstance<OC_UnwindContext>(_ctx, getState());
  enterRule(_localctx, 22, LcypherParser::RuleOC_Unwind);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(361);
    match(LcypherParser::UNWIND);
    setState(363);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(362);
      match(LcypherParser::SP);
    }
    setState(365);
    oC_Expression();
    setState(366);
    match(LcypherParser::SP);
    setState(367);
    match(LcypherParser::AS);
    setState(368);
    match(LcypherParser::SP);
    setState(369);
    oC_Variable();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_MergeContext ------------------------------------------------------------------

LcypherParser::OC_MergeContext::OC_MergeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_MergeContext::MERGE() {
  return getToken(LcypherParser::MERGE, 0);
}

LcypherParser::OC_PatternPartContext* LcypherParser::OC_MergeContext::oC_PatternPart() {
  return getRuleContext<LcypherParser::OC_PatternPartContext>(0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_MergeContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_MergeContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

std::vector<LcypherParser::OC_MergeActionContext *> LcypherParser::OC_MergeContext::oC_MergeAction() {
  return getRuleContexts<LcypherParser::OC_MergeActionContext>();
}

LcypherParser::OC_MergeActionContext* LcypherParser::OC_MergeContext::oC_MergeAction(size_t i) {
  return getRuleContext<LcypherParser::OC_MergeActionContext>(i);
}


size_t LcypherParser::OC_MergeContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Merge;
}


antlrcpp::Any LcypherParser::OC_MergeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Merge(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_MergeContext* LcypherParser::oC_Merge() {
  OC_MergeContext *_localctx = _tracker.createInstance<OC_MergeContext>(_ctx, getState());
  enterRule(_localctx, 24, LcypherParser::RuleOC_Merge);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(371);
    match(LcypherParser::MERGE);
    setState(373);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(372);
      match(LcypherParser::SP);
    }
    setState(375);
    oC_PatternPart();
    setState(380);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(376);
        match(LcypherParser::SP);
        setState(377);
        oC_MergeAction(); 
      }
      setState(382);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_MergeActionContext ------------------------------------------------------------------

LcypherParser::OC_MergeActionContext::OC_MergeActionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_MergeActionContext::ON() {
  return getToken(LcypherParser::ON, 0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_MergeActionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_MergeActionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

tree::TerminalNode* LcypherParser::OC_MergeActionContext::MATCH() {
  return getToken(LcypherParser::MATCH, 0);
}

LcypherParser::OC_SetContext* LcypherParser::OC_MergeActionContext::oC_Set() {
  return getRuleContext<LcypherParser::OC_SetContext>(0);
}

tree::TerminalNode* LcypherParser::OC_MergeActionContext::CREATE() {
  return getToken(LcypherParser::CREATE, 0);
}


size_t LcypherParser::OC_MergeActionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_MergeAction;
}


antlrcpp::Any LcypherParser::OC_MergeActionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_MergeAction(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_MergeActionContext* LcypherParser::oC_MergeAction() {
  OC_MergeActionContext *_localctx = _tracker.createInstance<OC_MergeActionContext>(_ctx, getState());
  enterRule(_localctx, 26, LcypherParser::RuleOC_MergeAction);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(393);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 40, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(383);
      match(LcypherParser::ON);
      setState(384);
      match(LcypherParser::SP);
      setState(385);
      match(LcypherParser::MATCH);
      setState(386);
      match(LcypherParser::SP);
      setState(387);
      oC_Set();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(388);
      match(LcypherParser::ON);
      setState(389);
      match(LcypherParser::SP);
      setState(390);
      match(LcypherParser::CREATE);
      setState(391);
      match(LcypherParser::SP);
      setState(392);
      oC_Set();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_CreateContext ------------------------------------------------------------------

LcypherParser::OC_CreateContext::OC_CreateContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_CreateContext::CREATE() {
  return getToken(LcypherParser::CREATE, 0);
}

LcypherParser::OC_PatternContext* LcypherParser::OC_CreateContext::oC_Pattern() {
  return getRuleContext<LcypherParser::OC_PatternContext>(0);
}

tree::TerminalNode* LcypherParser::OC_CreateContext::SP() {
  return getToken(LcypherParser::SP, 0);
}


size_t LcypherParser::OC_CreateContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Create;
}


antlrcpp::Any LcypherParser::OC_CreateContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Create(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_CreateContext* LcypherParser::oC_Create() {
  OC_CreateContext *_localctx = _tracker.createInstance<OC_CreateContext>(_ctx, getState());
  enterRule(_localctx, 28, LcypherParser::RuleOC_Create);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(395);
    match(LcypherParser::CREATE);
    setState(397);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(396);
      match(LcypherParser::SP);
    }
    setState(399);
    oC_Pattern();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_SetContext ------------------------------------------------------------------

LcypherParser::OC_SetContext::OC_SetContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_SetContext::SET() {
  return getToken(LcypherParser::SET, 0);
}

std::vector<LcypherParser::OC_SetItemContext *> LcypherParser::OC_SetContext::oC_SetItem() {
  return getRuleContexts<LcypherParser::OC_SetItemContext>();
}

LcypherParser::OC_SetItemContext* LcypherParser::OC_SetContext::oC_SetItem(size_t i) {
  return getRuleContext<LcypherParser::OC_SetItemContext>(i);
}

tree::TerminalNode* LcypherParser::OC_SetContext::SP() {
  return getToken(LcypherParser::SP, 0);
}


size_t LcypherParser::OC_SetContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Set;
}


antlrcpp::Any LcypherParser::OC_SetContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Set(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_SetContext* LcypherParser::oC_Set() {
  OC_SetContext *_localctx = _tracker.createInstance<OC_SetContext>(_ctx, getState());
  enterRule(_localctx, 30, LcypherParser::RuleOC_Set);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(401);
    match(LcypherParser::SET);
    setState(403);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(402);
      match(LcypherParser::SP);
    }
    setState(405);
    oC_SetItem();
    setState(410);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == LcypherParser::T__1) {
      setState(406);
      match(LcypherParser::T__1);
      setState(407);
      oC_SetItem();
      setState(412);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_SetItemContext ------------------------------------------------------------------

LcypherParser::OC_SetItemContext::OC_SetItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_PropertyExpressionContext* LcypherParser::OC_SetItemContext::oC_PropertyExpression() {
  return getRuleContext<LcypherParser::OC_PropertyExpressionContext>(0);
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_SetItemContext::oC_Expression() {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_SetItemContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_SetItemContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

LcypherParser::OC_VariableContext* LcypherParser::OC_SetItemContext::oC_Variable() {
  return getRuleContext<LcypherParser::OC_VariableContext>(0);
}

LcypherParser::OC_NodeLabelsContext* LcypherParser::OC_SetItemContext::oC_NodeLabels() {
  return getRuleContext<LcypherParser::OC_NodeLabelsContext>(0);
}


size_t LcypherParser::OC_SetItemContext::getRuleIndex() const {
  return LcypherParser::RuleOC_SetItem;
}


antlrcpp::Any LcypherParser::OC_SetItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_SetItem(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_SetItemContext* LcypherParser::oC_SetItem() {
  OC_SetItemContext *_localctx = _tracker.createInstance<OC_SetItemContext>(_ctx, getState());
  enterRule(_localctx, 32, LcypherParser::RuleOC_SetItem);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(449);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 51, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(413);
      oC_PropertyExpression();
      setState(415);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(414);
        match(LcypherParser::SP);
      }
      setState(417);
      match(LcypherParser::T__2);
      setState(419);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(418);
        match(LcypherParser::SP);
      }
      setState(421);
      oC_Expression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(423);
      oC_Variable();
      setState(425);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(424);
        match(LcypherParser::SP);
      }
      setState(427);
      match(LcypherParser::T__2);
      setState(429);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(428);
        match(LcypherParser::SP);
      }
      setState(431);
      oC_Expression();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(433);
      oC_Variable();
      setState(435);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(434);
        match(LcypherParser::SP);
      }
      setState(437);
      match(LcypherParser::T__3);
      setState(439);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(438);
        match(LcypherParser::SP);
      }
      setState(441);
      oC_Expression();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(443);
      oC_Variable();
      setState(445);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(444);
        match(LcypherParser::SP);
      }
      setState(447);
      oC_NodeLabels();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_DeleteContext ------------------------------------------------------------------

LcypherParser::OC_DeleteContext::OC_DeleteContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_DeleteContext::DELETE_() {
  return getToken(LcypherParser::DELETE_, 0);
}

std::vector<LcypherParser::OC_ExpressionContext *> LcypherParser::OC_DeleteContext::oC_Expression() {
  return getRuleContexts<LcypherParser::OC_ExpressionContext>();
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_DeleteContext::oC_Expression(size_t i) {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(i);
}

tree::TerminalNode* LcypherParser::OC_DeleteContext::DETACH() {
  return getToken(LcypherParser::DETACH, 0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_DeleteContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_DeleteContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_DeleteContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Delete;
}


antlrcpp::Any LcypherParser::OC_DeleteContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Delete(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_DeleteContext* LcypherParser::oC_Delete() {
  OC_DeleteContext *_localctx = _tracker.createInstance<OC_DeleteContext>(_ctx, getState());
  enterRule(_localctx, 34, LcypherParser::RuleOC_Delete);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(453);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::DETACH) {
      setState(451);
      match(LcypherParser::DETACH);
      setState(452);
      match(LcypherParser::SP);
    }
    setState(455);
    match(LcypherParser::DELETE_);
    setState(457);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(456);
      match(LcypherParser::SP);
    }
    setState(459);
    oC_Expression();
    setState(470);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 56, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(461);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(460);
          match(LcypherParser::SP);
        }
        setState(463);
        match(LcypherParser::T__1);
        setState(465);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(464);
          match(LcypherParser::SP);
        }
        setState(467);
        oC_Expression(); 
      }
      setState(472);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 56, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_RemoveContext ------------------------------------------------------------------

LcypherParser::OC_RemoveContext::OC_RemoveContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_RemoveContext::REMOVE() {
  return getToken(LcypherParser::REMOVE, 0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_RemoveContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_RemoveContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

std::vector<LcypherParser::OC_RemoveItemContext *> LcypherParser::OC_RemoveContext::oC_RemoveItem() {
  return getRuleContexts<LcypherParser::OC_RemoveItemContext>();
}

LcypherParser::OC_RemoveItemContext* LcypherParser::OC_RemoveContext::oC_RemoveItem(size_t i) {
  return getRuleContext<LcypherParser::OC_RemoveItemContext>(i);
}


size_t LcypherParser::OC_RemoveContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Remove;
}


antlrcpp::Any LcypherParser::OC_RemoveContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Remove(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_RemoveContext* LcypherParser::oC_Remove() {
  OC_RemoveContext *_localctx = _tracker.createInstance<OC_RemoveContext>(_ctx, getState());
  enterRule(_localctx, 36, LcypherParser::RuleOC_Remove);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(473);
    match(LcypherParser::REMOVE);
    setState(474);
    match(LcypherParser::SP);
    setState(475);
    oC_RemoveItem();
    setState(486);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 59, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(477);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(476);
          match(LcypherParser::SP);
        }
        setState(479);
        match(LcypherParser::T__1);
        setState(481);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(480);
          match(LcypherParser::SP);
        }
        setState(483);
        oC_RemoveItem(); 
      }
      setState(488);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 59, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_RemoveItemContext ------------------------------------------------------------------

LcypherParser::OC_RemoveItemContext::OC_RemoveItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_VariableContext* LcypherParser::OC_RemoveItemContext::oC_Variable() {
  return getRuleContext<LcypherParser::OC_VariableContext>(0);
}

LcypherParser::OC_NodeLabelsContext* LcypherParser::OC_RemoveItemContext::oC_NodeLabels() {
  return getRuleContext<LcypherParser::OC_NodeLabelsContext>(0);
}

LcypherParser::OC_PropertyExpressionContext* LcypherParser::OC_RemoveItemContext::oC_PropertyExpression() {
  return getRuleContext<LcypherParser::OC_PropertyExpressionContext>(0);
}


size_t LcypherParser::OC_RemoveItemContext::getRuleIndex() const {
  return LcypherParser::RuleOC_RemoveItem;
}


antlrcpp::Any LcypherParser::OC_RemoveItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_RemoveItem(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_RemoveItemContext* LcypherParser::oC_RemoveItem() {
  OC_RemoveItemContext *_localctx = _tracker.createInstance<OC_RemoveItemContext>(_ctx, getState());
  enterRule(_localctx, 38, LcypherParser::RuleOC_RemoveItem);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(493);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 60, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(489);
      oC_Variable();
      setState(490);
      oC_NodeLabels();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(492);
      oC_PropertyExpression();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_InQueryCallContext ------------------------------------------------------------------

LcypherParser::OC_InQueryCallContext::OC_InQueryCallContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_InQueryCallContext::CALL() {
  return getToken(LcypherParser::CALL, 0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_InQueryCallContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_InQueryCallContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

LcypherParser::OC_ExplicitProcedureInvocationContext* LcypherParser::OC_InQueryCallContext::oC_ExplicitProcedureInvocation() {
  return getRuleContext<LcypherParser::OC_ExplicitProcedureInvocationContext>(0);
}

tree::TerminalNode* LcypherParser::OC_InQueryCallContext::YIELD() {
  return getToken(LcypherParser::YIELD, 0);
}

LcypherParser::OC_YieldItemsContext* LcypherParser::OC_InQueryCallContext::oC_YieldItems() {
  return getRuleContext<LcypherParser::OC_YieldItemsContext>(0);
}


size_t LcypherParser::OC_InQueryCallContext::getRuleIndex() const {
  return LcypherParser::RuleOC_InQueryCall;
}


antlrcpp::Any LcypherParser::OC_InQueryCallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_InQueryCall(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_InQueryCallContext* LcypherParser::oC_InQueryCall() {
  OC_InQueryCallContext *_localctx = _tracker.createInstance<OC_InQueryCallContext>(_ctx, getState());
  enterRule(_localctx, 40, LcypherParser::RuleOC_InQueryCall);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(495);
    match(LcypherParser::CALL);
    setState(496);
    match(LcypherParser::SP);
    setState(497);
    oC_ExplicitProcedureInvocation();
    setState(504);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 62, _ctx)) {
    case 1: {
      setState(499);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(498);
        match(LcypherParser::SP);
      }
      setState(501);
      match(LcypherParser::YIELD);
      setState(502);
      match(LcypherParser::SP);
      setState(503);
      oC_YieldItems();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_StandaloneCallContext ------------------------------------------------------------------

LcypherParser::OC_StandaloneCallContext::OC_StandaloneCallContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_StandaloneCallContext::CALL() {
  return getToken(LcypherParser::CALL, 0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_StandaloneCallContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_StandaloneCallContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

LcypherParser::OC_ExplicitProcedureInvocationContext* LcypherParser::OC_StandaloneCallContext::oC_ExplicitProcedureInvocation() {
  return getRuleContext<LcypherParser::OC_ExplicitProcedureInvocationContext>(0);
}

LcypherParser::OC_ImplicitProcedureInvocationContext* LcypherParser::OC_StandaloneCallContext::oC_ImplicitProcedureInvocation() {
  return getRuleContext<LcypherParser::OC_ImplicitProcedureInvocationContext>(0);
}

tree::TerminalNode* LcypherParser::OC_StandaloneCallContext::YIELD() {
  return getToken(LcypherParser::YIELD, 0);
}

LcypherParser::OC_YieldItemsContext* LcypherParser::OC_StandaloneCallContext::oC_YieldItems() {
  return getRuleContext<LcypherParser::OC_YieldItemsContext>(0);
}


size_t LcypherParser::OC_StandaloneCallContext::getRuleIndex() const {
  return LcypherParser::RuleOC_StandaloneCall;
}


antlrcpp::Any LcypherParser::OC_StandaloneCallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_StandaloneCall(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_StandaloneCallContext* LcypherParser::oC_StandaloneCall() {
  OC_StandaloneCallContext *_localctx = _tracker.createInstance<OC_StandaloneCallContext>(_ctx, getState());
  enterRule(_localctx, 42, LcypherParser::RuleOC_StandaloneCall);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(506);
    match(LcypherParser::CALL);
    setState(507);
    match(LcypherParser::SP);
    setState(510);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 63, _ctx)) {
    case 1: {
      setState(508);
      oC_ExplicitProcedureInvocation();
      break;
    }

    case 2: {
      setState(509);
      oC_ImplicitProcedureInvocation();
      break;
    }

    default:
      break;
    }
    setState(516);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 64, _ctx)) {
    case 1: {
      setState(512);
      match(LcypherParser::SP);
      setState(513);
      match(LcypherParser::YIELD);
      setState(514);
      match(LcypherParser::SP);
      setState(515);
      oC_YieldItems();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_YieldItemsContext ------------------------------------------------------------------

LcypherParser::OC_YieldItemsContext::OC_YieldItemsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_WhereContext* LcypherParser::OC_YieldItemsContext::oC_Where() {
  return getRuleContext<LcypherParser::OC_WhereContext>(0);
}

std::vector<LcypherParser::OC_YieldItemContext *> LcypherParser::OC_YieldItemsContext::oC_YieldItem() {
  return getRuleContexts<LcypherParser::OC_YieldItemContext>();
}

LcypherParser::OC_YieldItemContext* LcypherParser::OC_YieldItemsContext::oC_YieldItem(size_t i) {
  return getRuleContext<LcypherParser::OC_YieldItemContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_YieldItemsContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_YieldItemsContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_YieldItemsContext::getRuleIndex() const {
  return LcypherParser::RuleOC_YieldItems;
}


antlrcpp::Any LcypherParser::OC_YieldItemsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_YieldItems(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_YieldItemsContext* LcypherParser::oC_YieldItems() {
  OC_YieldItemsContext *_localctx = _tracker.createInstance<OC_YieldItemsContext>(_ctx, getState());
  enterRule(_localctx, 44, LcypherParser::RuleOC_YieldItems);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(533);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case LcypherParser::T__4: {
        setState(518);
        match(LcypherParser::T__4);
        break;
      }

      case LcypherParser::COUNT:
      case LcypherParser::ANY:
      case LcypherParser::NONE:
      case LcypherParser::SINGLE:
      case LcypherParser::HexLetter:
      case LcypherParser::FILTER:
      case LcypherParser::EXTRACT:
      case LcypherParser::UnescapedSymbolicName:
      case LcypherParser::EscapedSymbolicName: {
        setState(519);
        oC_YieldItem();
        setState(530);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 67, _ctx);
        while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
          if (alt == 1) {
            setState(521);
            _errHandler->sync(this);

            _la = _input->LA(1);
            if (_la == LcypherParser::SP) {
              setState(520);
              match(LcypherParser::SP);
            }
            setState(523);
            match(LcypherParser::T__1);
            setState(525);
            _errHandler->sync(this);

            _la = _input->LA(1);
            if (_la == LcypherParser::SP) {
              setState(524);
              match(LcypherParser::SP);
            }
            setState(527);
            oC_YieldItem(); 
          }
          setState(532);
          _errHandler->sync(this);
          alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 67, _ctx);
        }
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(539);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 70, _ctx)) {
    case 1: {
      setState(536);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(535);
        match(LcypherParser::SP);
      }
      setState(538);
      oC_Where();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_YieldItemContext ------------------------------------------------------------------

LcypherParser::OC_YieldItemContext::OC_YieldItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_VariableContext* LcypherParser::OC_YieldItemContext::oC_Variable() {
  return getRuleContext<LcypherParser::OC_VariableContext>(0);
}

LcypherParser::OC_ProcedureResultFieldContext* LcypherParser::OC_YieldItemContext::oC_ProcedureResultField() {
  return getRuleContext<LcypherParser::OC_ProcedureResultFieldContext>(0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_YieldItemContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_YieldItemContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

tree::TerminalNode* LcypherParser::OC_YieldItemContext::AS() {
  return getToken(LcypherParser::AS, 0);
}


size_t LcypherParser::OC_YieldItemContext::getRuleIndex() const {
  return LcypherParser::RuleOC_YieldItem;
}


antlrcpp::Any LcypherParser::OC_YieldItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_YieldItem(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_YieldItemContext* LcypherParser::oC_YieldItem() {
  OC_YieldItemContext *_localctx = _tracker.createInstance<OC_YieldItemContext>(_ctx, getState());
  enterRule(_localctx, 46, LcypherParser::RuleOC_YieldItem);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(546);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 71, _ctx)) {
    case 1: {
      setState(541);
      oC_ProcedureResultField();
      setState(542);
      match(LcypherParser::SP);
      setState(543);
      match(LcypherParser::AS);
      setState(544);
      match(LcypherParser::SP);
      break;
    }

    default:
      break;
    }
    setState(548);
    oC_Variable();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_WithContext ------------------------------------------------------------------

LcypherParser::OC_WithContext::OC_WithContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_WithContext::WITH() {
  return getToken(LcypherParser::WITH, 0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_WithContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_WithContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

LcypherParser::OC_ReturnBodyContext* LcypherParser::OC_WithContext::oC_ReturnBody() {
  return getRuleContext<LcypherParser::OC_ReturnBodyContext>(0);
}

tree::TerminalNode* LcypherParser::OC_WithContext::DISTINCT() {
  return getToken(LcypherParser::DISTINCT, 0);
}

LcypherParser::OC_WhereContext* LcypherParser::OC_WithContext::oC_Where() {
  return getRuleContext<LcypherParser::OC_WhereContext>(0);
}


size_t LcypherParser::OC_WithContext::getRuleIndex() const {
  return LcypherParser::RuleOC_With;
}


antlrcpp::Any LcypherParser::OC_WithContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_With(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_WithContext* LcypherParser::oC_With() {
  OC_WithContext *_localctx = _tracker.createInstance<OC_WithContext>(_ctx, getState());
  enterRule(_localctx, 48, LcypherParser::RuleOC_With);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(550);
    match(LcypherParser::WITH);
    setState(555);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 73, _ctx)) {
    case 1: {
      setState(552);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(551);
        match(LcypherParser::SP);
      }
      setState(554);
      match(LcypherParser::DISTINCT);
      break;
    }

    default:
      break;
    }
    setState(557);
    match(LcypherParser::SP);
    setState(558);
    oC_ReturnBody();
    setState(563);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 75, _ctx)) {
    case 1: {
      setState(560);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(559);
        match(LcypherParser::SP);
      }
      setState(562);
      oC_Where();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_ReturnContext ------------------------------------------------------------------

LcypherParser::OC_ReturnContext::OC_ReturnContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_ReturnContext::RETURN() {
  return getToken(LcypherParser::RETURN, 0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_ReturnContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_ReturnContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

LcypherParser::OC_ReturnBodyContext* LcypherParser::OC_ReturnContext::oC_ReturnBody() {
  return getRuleContext<LcypherParser::OC_ReturnBodyContext>(0);
}

tree::TerminalNode* LcypherParser::OC_ReturnContext::DISTINCT() {
  return getToken(LcypherParser::DISTINCT, 0);
}


size_t LcypherParser::OC_ReturnContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Return;
}


antlrcpp::Any LcypherParser::OC_ReturnContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Return(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_ReturnContext* LcypherParser::oC_Return() {
  OC_ReturnContext *_localctx = _tracker.createInstance<OC_ReturnContext>(_ctx, getState());
  enterRule(_localctx, 50, LcypherParser::RuleOC_Return);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(565);
    match(LcypherParser::RETURN);
    setState(570);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 77, _ctx)) {
    case 1: {
      setState(567);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(566);
        match(LcypherParser::SP);
      }
      setState(569);
      match(LcypherParser::DISTINCT);
      break;
    }

    default:
      break;
    }
    setState(572);
    match(LcypherParser::SP);
    setState(573);
    oC_ReturnBody();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_ReturnBodyContext ------------------------------------------------------------------

LcypherParser::OC_ReturnBodyContext::OC_ReturnBodyContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_ReturnItemsContext* LcypherParser::OC_ReturnBodyContext::oC_ReturnItems() {
  return getRuleContext<LcypherParser::OC_ReturnItemsContext>(0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_ReturnBodyContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_ReturnBodyContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

LcypherParser::OC_OrderContext* LcypherParser::OC_ReturnBodyContext::oC_Order() {
  return getRuleContext<LcypherParser::OC_OrderContext>(0);
}

LcypherParser::OC_SkipContext* LcypherParser::OC_ReturnBodyContext::oC_Skip() {
  return getRuleContext<LcypherParser::OC_SkipContext>(0);
}

LcypherParser::OC_LimitContext* LcypherParser::OC_ReturnBodyContext::oC_Limit() {
  return getRuleContext<LcypherParser::OC_LimitContext>(0);
}


size_t LcypherParser::OC_ReturnBodyContext::getRuleIndex() const {
  return LcypherParser::RuleOC_ReturnBody;
}


antlrcpp::Any LcypherParser::OC_ReturnBodyContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_ReturnBody(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_ReturnBodyContext* LcypherParser::oC_ReturnBody() {
  OC_ReturnBodyContext *_localctx = _tracker.createInstance<OC_ReturnBodyContext>(_ctx, getState());
  enterRule(_localctx, 52, LcypherParser::RuleOC_ReturnBody);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(575);
    oC_ReturnItems();
    setState(578);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 78, _ctx)) {
    case 1: {
      setState(576);
      match(LcypherParser::SP);
      setState(577);
      oC_Order();
      break;
    }

    default:
      break;
    }
    setState(582);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 79, _ctx)) {
    case 1: {
      setState(580);
      match(LcypherParser::SP);
      setState(581);
      oC_Skip();
      break;
    }

    default:
      break;
    }
    setState(586);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 80, _ctx)) {
    case 1: {
      setState(584);
      match(LcypherParser::SP);
      setState(585);
      oC_Limit();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_ReturnItemsContext ------------------------------------------------------------------

LcypherParser::OC_ReturnItemsContext::OC_ReturnItemsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<LcypherParser::OC_ReturnItemContext *> LcypherParser::OC_ReturnItemsContext::oC_ReturnItem() {
  return getRuleContexts<LcypherParser::OC_ReturnItemContext>();
}

LcypherParser::OC_ReturnItemContext* LcypherParser::OC_ReturnItemsContext::oC_ReturnItem(size_t i) {
  return getRuleContext<LcypherParser::OC_ReturnItemContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_ReturnItemsContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_ReturnItemsContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_ReturnItemsContext::getRuleIndex() const {
  return LcypherParser::RuleOC_ReturnItems;
}


antlrcpp::Any LcypherParser::OC_ReturnItemsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_ReturnItems(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_ReturnItemsContext* LcypherParser::oC_ReturnItems() {
  OC_ReturnItemsContext *_localctx = _tracker.createInstance<OC_ReturnItemsContext>(_ctx, getState());
  enterRule(_localctx, 54, LcypherParser::RuleOC_ReturnItems);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    setState(616);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case LcypherParser::T__4: {
        enterOuterAlt(_localctx, 1);
        setState(588);
        match(LcypherParser::T__4);
        setState(599);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 83, _ctx);
        while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
          if (alt == 1) {
            setState(590);
            _errHandler->sync(this);

            _la = _input->LA(1);
            if (_la == LcypherParser::SP) {
              setState(589);
              match(LcypherParser::SP);
            }
            setState(592);
            match(LcypherParser::T__1);
            setState(594);
            _errHandler->sync(this);

            _la = _input->LA(1);
            if (_la == LcypherParser::SP) {
              setState(593);
              match(LcypherParser::SP);
            }
            setState(596);
            oC_ReturnItem(); 
          }
          setState(601);
          _errHandler->sync(this);
          alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 83, _ctx);
        }
        break;
      }

      case LcypherParser::T__5:
      case LcypherParser::T__7:
      case LcypherParser::T__12:
      case LcypherParser::T__13:
      case LcypherParser::T__23:
      case LcypherParser::T__25:
      case LcypherParser::ALL:
      case LcypherParser::NOT:
      case LcypherParser::NULL_:
      case LcypherParser::COUNT:
      case LcypherParser::ANY:
      case LcypherParser::NONE:
      case LcypherParser::SINGLE:
      case LcypherParser::TRUE_:
      case LcypherParser::FALSE_:
      case LcypherParser::EXISTS:
      case LcypherParser::CASE:
      case LcypherParser::StringLiteral:
      case LcypherParser::HexInteger:
      case LcypherParser::DecimalInteger:
      case LcypherParser::OctalInteger:
      case LcypherParser::HexLetter:
      case LcypherParser::ExponentDecimalReal:
      case LcypherParser::RegularDecimalReal:
      case LcypherParser::FILTER:
      case LcypherParser::EXTRACT:
      case LcypherParser::UnescapedSymbolicName:
      case LcypherParser::EscapedSymbolicName: {
        enterOuterAlt(_localctx, 2);
        setState(602);
        oC_ReturnItem();
        setState(613);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 86, _ctx);
        while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
          if (alt == 1) {
            setState(604);
            _errHandler->sync(this);

            _la = _input->LA(1);
            if (_la == LcypherParser::SP) {
              setState(603);
              match(LcypherParser::SP);
            }
            setState(606);
            match(LcypherParser::T__1);
            setState(608);
            _errHandler->sync(this);

            _la = _input->LA(1);
            if (_la == LcypherParser::SP) {
              setState(607);
              match(LcypherParser::SP);
            }
            setState(610);
            oC_ReturnItem(); 
          }
          setState(615);
          _errHandler->sync(this);
          alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 86, _ctx);
        }
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_ReturnItemContext ------------------------------------------------------------------

LcypherParser::OC_ReturnItemContext::OC_ReturnItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_ReturnItemContext::oC_Expression() {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_ReturnItemContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_ReturnItemContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

tree::TerminalNode* LcypherParser::OC_ReturnItemContext::AS() {
  return getToken(LcypherParser::AS, 0);
}

LcypherParser::OC_VariableContext* LcypherParser::OC_ReturnItemContext::oC_Variable() {
  return getRuleContext<LcypherParser::OC_VariableContext>(0);
}


size_t LcypherParser::OC_ReturnItemContext::getRuleIndex() const {
  return LcypherParser::RuleOC_ReturnItem;
}


antlrcpp::Any LcypherParser::OC_ReturnItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_ReturnItem(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_ReturnItemContext* LcypherParser::oC_ReturnItem() {
  OC_ReturnItemContext *_localctx = _tracker.createInstance<OC_ReturnItemContext>(_ctx, getState());
  enterRule(_localctx, 56, LcypherParser::RuleOC_ReturnItem);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(625);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 88, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(618);
      oC_Expression();
      setState(619);
      match(LcypherParser::SP);
      setState(620);
      match(LcypherParser::AS);
      setState(621);
      match(LcypherParser::SP);
      setState(622);
      oC_Variable();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(624);
      oC_Expression();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_OrderContext ------------------------------------------------------------------

LcypherParser::OC_OrderContext::OC_OrderContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_OrderContext::ORDER() {
  return getToken(LcypherParser::ORDER, 0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_OrderContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_OrderContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

tree::TerminalNode* LcypherParser::OC_OrderContext::BY() {
  return getToken(LcypherParser::BY, 0);
}

std::vector<LcypherParser::OC_SortItemContext *> LcypherParser::OC_OrderContext::oC_SortItem() {
  return getRuleContexts<LcypherParser::OC_SortItemContext>();
}

LcypherParser::OC_SortItemContext* LcypherParser::OC_OrderContext::oC_SortItem(size_t i) {
  return getRuleContext<LcypherParser::OC_SortItemContext>(i);
}


size_t LcypherParser::OC_OrderContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Order;
}


antlrcpp::Any LcypherParser::OC_OrderContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Order(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_OrderContext* LcypherParser::oC_Order() {
  OC_OrderContext *_localctx = _tracker.createInstance<OC_OrderContext>(_ctx, getState());
  enterRule(_localctx, 58, LcypherParser::RuleOC_Order);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(627);
    match(LcypherParser::ORDER);
    setState(628);
    match(LcypherParser::SP);
    setState(629);
    match(LcypherParser::BY);
    setState(630);
    match(LcypherParser::SP);
    setState(631);
    oC_SortItem();
    setState(639);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == LcypherParser::T__1) {
      setState(632);
      match(LcypherParser::T__1);
      setState(634);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(633);
        match(LcypherParser::SP);
      }
      setState(636);
      oC_SortItem();
      setState(641);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_SkipContext ------------------------------------------------------------------

LcypherParser::OC_SkipContext::OC_SkipContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_SkipContext::L_SKIP() {
  return getToken(LcypherParser::L_SKIP, 0);
}

tree::TerminalNode* LcypherParser::OC_SkipContext::SP() {
  return getToken(LcypherParser::SP, 0);
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_SkipContext::oC_Expression() {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(0);
}


size_t LcypherParser::OC_SkipContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Skip;
}


antlrcpp::Any LcypherParser::OC_SkipContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Skip(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_SkipContext* LcypherParser::oC_Skip() {
  OC_SkipContext *_localctx = _tracker.createInstance<OC_SkipContext>(_ctx, getState());
  enterRule(_localctx, 60, LcypherParser::RuleOC_Skip);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(642);
    match(LcypherParser::L_SKIP);
    setState(643);
    match(LcypherParser::SP);
    setState(644);
    oC_Expression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_LimitContext ------------------------------------------------------------------

LcypherParser::OC_LimitContext::OC_LimitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_LimitContext::LIMIT() {
  return getToken(LcypherParser::LIMIT, 0);
}

tree::TerminalNode* LcypherParser::OC_LimitContext::SP() {
  return getToken(LcypherParser::SP, 0);
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_LimitContext::oC_Expression() {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(0);
}


size_t LcypherParser::OC_LimitContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Limit;
}


antlrcpp::Any LcypherParser::OC_LimitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Limit(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_LimitContext* LcypherParser::oC_Limit() {
  OC_LimitContext *_localctx = _tracker.createInstance<OC_LimitContext>(_ctx, getState());
  enterRule(_localctx, 62, LcypherParser::RuleOC_Limit);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(646);
    match(LcypherParser::LIMIT);
    setState(647);
    match(LcypherParser::SP);
    setState(648);
    oC_Expression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_SortItemContext ------------------------------------------------------------------

LcypherParser::OC_SortItemContext::OC_SortItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_SortItemContext::oC_Expression() {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(0);
}

tree::TerminalNode* LcypherParser::OC_SortItemContext::ASCENDING() {
  return getToken(LcypherParser::ASCENDING, 0);
}

tree::TerminalNode* LcypherParser::OC_SortItemContext::ASC() {
  return getToken(LcypherParser::ASC, 0);
}

tree::TerminalNode* LcypherParser::OC_SortItemContext::DESCENDING() {
  return getToken(LcypherParser::DESCENDING, 0);
}

tree::TerminalNode* LcypherParser::OC_SortItemContext::DESC() {
  return getToken(LcypherParser::DESC, 0);
}

tree::TerminalNode* LcypherParser::OC_SortItemContext::SP() {
  return getToken(LcypherParser::SP, 0);
}


size_t LcypherParser::OC_SortItemContext::getRuleIndex() const {
  return LcypherParser::RuleOC_SortItem;
}


antlrcpp::Any LcypherParser::OC_SortItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_SortItem(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_SortItemContext* LcypherParser::oC_SortItem() {
  OC_SortItemContext *_localctx = _tracker.createInstance<OC_SortItemContext>(_ctx, getState());
  enterRule(_localctx, 64, LcypherParser::RuleOC_SortItem);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(650);
    oC_Expression();
    setState(655);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 92, _ctx)) {
    case 1: {
      setState(652);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(651);
        match(LcypherParser::SP);
      }
      setState(654);
      _la = _input->LA(1);
      if (!(((((_la - 70) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 70)) & ((1ULL << (LcypherParser::ASCENDING - 70))
        | (1ULL << (LcypherParser::ASC - 70))
        | (1ULL << (LcypherParser::DESCENDING - 70))
        | (1ULL << (LcypherParser::DESC - 70)))) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_HintContext ------------------------------------------------------------------

LcypherParser::OC_HintContext::OC_HintContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_HintContext::USING() {
  return getToken(LcypherParser::USING, 0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_HintContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_HintContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

tree::TerminalNode* LcypherParser::OC_HintContext::JOIN() {
  return getToken(LcypherParser::JOIN, 0);
}

tree::TerminalNode* LcypherParser::OC_HintContext::ON() {
  return getToken(LcypherParser::ON, 0);
}

LcypherParser::OC_VariableContext* LcypherParser::OC_HintContext::oC_Variable() {
  return getRuleContext<LcypherParser::OC_VariableContext>(0);
}

tree::TerminalNode* LcypherParser::OC_HintContext::START() {
  return getToken(LcypherParser::START, 0);
}


size_t LcypherParser::OC_HintContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Hint;
}


antlrcpp::Any LcypherParser::OC_HintContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Hint(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_HintContext* LcypherParser::oC_Hint() {
  OC_HintContext *_localctx = _tracker.createInstance<OC_HintContext>(_ctx, getState());
  enterRule(_localctx, 66, LcypherParser::RuleOC_Hint);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(671);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 93, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(657);
      match(LcypherParser::USING);
      setState(658);
      match(LcypherParser::SP);
      setState(659);
      match(LcypherParser::JOIN);
      setState(660);
      match(LcypherParser::SP);
      setState(661);
      match(LcypherParser::ON);
      setState(662);
      match(LcypherParser::SP);
      setState(663);
      oC_Variable();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(664);
      match(LcypherParser::USING);
      setState(665);
      match(LcypherParser::SP);
      setState(666);
      match(LcypherParser::START);
      setState(667);
      match(LcypherParser::SP);
      setState(668);
      match(LcypherParser::ON);
      setState(669);
      match(LcypherParser::SP);
      setState(670);
      oC_Variable();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_WhereContext ------------------------------------------------------------------

LcypherParser::OC_WhereContext::OC_WhereContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_WhereContext::WHERE() {
  return getToken(LcypherParser::WHERE, 0);
}

tree::TerminalNode* LcypherParser::OC_WhereContext::SP() {
  return getToken(LcypherParser::SP, 0);
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_WhereContext::oC_Expression() {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(0);
}


size_t LcypherParser::OC_WhereContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Where;
}


antlrcpp::Any LcypherParser::OC_WhereContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Where(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_WhereContext* LcypherParser::oC_Where() {
  OC_WhereContext *_localctx = _tracker.createInstance<OC_WhereContext>(_ctx, getState());
  enterRule(_localctx, 68, LcypherParser::RuleOC_Where);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(673);
    match(LcypherParser::WHERE);
    setState(674);
    match(LcypherParser::SP);
    setState(675);
    oC_Expression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_PatternContext ------------------------------------------------------------------

LcypherParser::OC_PatternContext::OC_PatternContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<LcypherParser::OC_PatternPartContext *> LcypherParser::OC_PatternContext::oC_PatternPart() {
  return getRuleContexts<LcypherParser::OC_PatternPartContext>();
}

LcypherParser::OC_PatternPartContext* LcypherParser::OC_PatternContext::oC_PatternPart(size_t i) {
  return getRuleContext<LcypherParser::OC_PatternPartContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_PatternContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_PatternContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_PatternContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Pattern;
}


antlrcpp::Any LcypherParser::OC_PatternContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Pattern(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_PatternContext* LcypherParser::oC_Pattern() {
  OC_PatternContext *_localctx = _tracker.createInstance<OC_PatternContext>(_ctx, getState());
  enterRule(_localctx, 70, LcypherParser::RuleOC_Pattern);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(677);
    oC_PatternPart();
    setState(688);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 96, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(679);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(678);
          match(LcypherParser::SP);
        }
        setState(681);
        match(LcypherParser::T__1);
        setState(683);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(682);
          match(LcypherParser::SP);
        }
        setState(685);
        oC_PatternPart(); 
      }
      setState(690);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 96, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_PatternPartContext ------------------------------------------------------------------

LcypherParser::OC_PatternPartContext::OC_PatternPartContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_VariableContext* LcypherParser::OC_PatternPartContext::oC_Variable() {
  return getRuleContext<LcypherParser::OC_VariableContext>(0);
}

LcypherParser::OC_AnonymousPatternPartContext* LcypherParser::OC_PatternPartContext::oC_AnonymousPatternPart() {
  return getRuleContext<LcypherParser::OC_AnonymousPatternPartContext>(0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_PatternPartContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_PatternPartContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_PatternPartContext::getRuleIndex() const {
  return LcypherParser::RuleOC_PatternPart;
}


antlrcpp::Any LcypherParser::OC_PatternPartContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_PatternPart(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_PatternPartContext* LcypherParser::oC_PatternPart() {
  OC_PatternPartContext *_localctx = _tracker.createInstance<OC_PatternPartContext>(_ctx, getState());
  enterRule(_localctx, 72, LcypherParser::RuleOC_PatternPart);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(702);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case LcypherParser::COUNT:
      case LcypherParser::ANY:
      case LcypherParser::NONE:
      case LcypherParser::SINGLE:
      case LcypherParser::HexLetter:
      case LcypherParser::FILTER:
      case LcypherParser::EXTRACT:
      case LcypherParser::UnescapedSymbolicName:
      case LcypherParser::EscapedSymbolicName: {
        enterOuterAlt(_localctx, 1);
        setState(691);
        oC_Variable();
        setState(693);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(692);
          match(LcypherParser::SP);
        }
        setState(695);
        match(LcypherParser::T__2);
        setState(697);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(696);
          match(LcypherParser::SP);
        }
        setState(699);
        oC_AnonymousPatternPart();
        break;
      }

      case LcypherParser::T__5: {
        enterOuterAlt(_localctx, 2);
        setState(701);
        oC_AnonymousPatternPart();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_AnonymousPatternPartContext ------------------------------------------------------------------

LcypherParser::OC_AnonymousPatternPartContext::OC_AnonymousPatternPartContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_PatternElementContext* LcypherParser::OC_AnonymousPatternPartContext::oC_PatternElement() {
  return getRuleContext<LcypherParser::OC_PatternElementContext>(0);
}


size_t LcypherParser::OC_AnonymousPatternPartContext::getRuleIndex() const {
  return LcypherParser::RuleOC_AnonymousPatternPart;
}


antlrcpp::Any LcypherParser::OC_AnonymousPatternPartContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_AnonymousPatternPart(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_AnonymousPatternPartContext* LcypherParser::oC_AnonymousPatternPart() {
  OC_AnonymousPatternPartContext *_localctx = _tracker.createInstance<OC_AnonymousPatternPartContext>(_ctx, getState());
  enterRule(_localctx, 74, LcypherParser::RuleOC_AnonymousPatternPart);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(704);
    oC_PatternElement();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_PatternElementContext ------------------------------------------------------------------

LcypherParser::OC_PatternElementContext::OC_PatternElementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_NodePatternContext* LcypherParser::OC_PatternElementContext::oC_NodePattern() {
  return getRuleContext<LcypherParser::OC_NodePatternContext>(0);
}

std::vector<LcypherParser::OC_PatternElementChainContext *> LcypherParser::OC_PatternElementContext::oC_PatternElementChain() {
  return getRuleContexts<LcypherParser::OC_PatternElementChainContext>();
}

LcypherParser::OC_PatternElementChainContext* LcypherParser::OC_PatternElementContext::oC_PatternElementChain(size_t i) {
  return getRuleContext<LcypherParser::OC_PatternElementChainContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_PatternElementContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_PatternElementContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

LcypherParser::OC_PatternElementContext* LcypherParser::OC_PatternElementContext::oC_PatternElement() {
  return getRuleContext<LcypherParser::OC_PatternElementContext>(0);
}


size_t LcypherParser::OC_PatternElementContext::getRuleIndex() const {
  return LcypherParser::RuleOC_PatternElement;
}


antlrcpp::Any LcypherParser::OC_PatternElementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_PatternElement(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_PatternElementContext* LcypherParser::oC_PatternElement() {
  OC_PatternElementContext *_localctx = _tracker.createInstance<OC_PatternElementContext>(_ctx, getState());
  enterRule(_localctx, 76, LcypherParser::RuleOC_PatternElement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    setState(720);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 102, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(706);
      oC_NodePattern();
      setState(713);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 101, _ctx);
      while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
        if (alt == 1) {
          setState(708);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == LcypherParser::SP) {
            setState(707);
            match(LcypherParser::SP);
          }
          setState(710);
          oC_PatternElementChain(); 
        }
        setState(715);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 101, _ctx);
      }
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(716);
      match(LcypherParser::T__5);
      setState(717);
      oC_PatternElement();
      setState(718);
      match(LcypherParser::T__6);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_NodePatternContext ------------------------------------------------------------------

LcypherParser::OC_NodePatternContext::OC_NodePatternContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> LcypherParser::OC_NodePatternContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_NodePatternContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

LcypherParser::OC_VariableContext* LcypherParser::OC_NodePatternContext::oC_Variable() {
  return getRuleContext<LcypherParser::OC_VariableContext>(0);
}

LcypherParser::OC_NodeLabelsContext* LcypherParser::OC_NodePatternContext::oC_NodeLabels() {
  return getRuleContext<LcypherParser::OC_NodeLabelsContext>(0);
}

LcypherParser::OC_PropertiesContext* LcypherParser::OC_NodePatternContext::oC_Properties() {
  return getRuleContext<LcypherParser::OC_PropertiesContext>(0);
}


size_t LcypherParser::OC_NodePatternContext::getRuleIndex() const {
  return LcypherParser::RuleOC_NodePattern;
}


antlrcpp::Any LcypherParser::OC_NodePatternContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_NodePattern(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_NodePatternContext* LcypherParser::oC_NodePattern() {
  OC_NodePatternContext *_localctx = _tracker.createInstance<OC_NodePatternContext>(_ctx, getState());
  enterRule(_localctx, 78, LcypherParser::RuleOC_NodePattern);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(722);
    match(LcypherParser::T__5);
    setState(724);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(723);
      match(LcypherParser::SP);
    }
    setState(730);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 89) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 89)) & ((1ULL << (LcypherParser::COUNT - 89))
      | (1ULL << (LcypherParser::ANY - 89))
      | (1ULL << (LcypherParser::NONE - 89))
      | (1ULL << (LcypherParser::SINGLE - 89))
      | (1ULL << (LcypherParser::HexLetter - 89))
      | (1ULL << (LcypherParser::FILTER - 89))
      | (1ULL << (LcypherParser::EXTRACT - 89))
      | (1ULL << (LcypherParser::UnescapedSymbolicName - 89))
      | (1ULL << (LcypherParser::EscapedSymbolicName - 89)))) != 0)) {
      setState(726);
      oC_Variable();
      setState(728);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(727);
        match(LcypherParser::SP);
      }
    }
    setState(736);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::T__9) {
      setState(732);
      oC_NodeLabels();
      setState(734);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(733);
        match(LcypherParser::SP);
      }
    }
    setState(742);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::T__23

    || _la == LcypherParser::T__25) {
      setState(738);
      oC_Properties();
      setState(740);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(739);
        match(LcypherParser::SP);
      }
    }
    setState(744);
    match(LcypherParser::T__6);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_PatternElementChainContext ------------------------------------------------------------------

LcypherParser::OC_PatternElementChainContext::OC_PatternElementChainContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_RelationshipPatternContext* LcypherParser::OC_PatternElementChainContext::oC_RelationshipPattern() {
  return getRuleContext<LcypherParser::OC_RelationshipPatternContext>(0);
}

LcypherParser::OC_NodePatternContext* LcypherParser::OC_PatternElementChainContext::oC_NodePattern() {
  return getRuleContext<LcypherParser::OC_NodePatternContext>(0);
}

tree::TerminalNode* LcypherParser::OC_PatternElementChainContext::SP() {
  return getToken(LcypherParser::SP, 0);
}


size_t LcypherParser::OC_PatternElementChainContext::getRuleIndex() const {
  return LcypherParser::RuleOC_PatternElementChain;
}


antlrcpp::Any LcypherParser::OC_PatternElementChainContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_PatternElementChain(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_PatternElementChainContext* LcypherParser::oC_PatternElementChain() {
  OC_PatternElementChainContext *_localctx = _tracker.createInstance<OC_PatternElementChainContext>(_ctx, getState());
  enterRule(_localctx, 80, LcypherParser::RuleOC_PatternElementChain);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(746);
    oC_RelationshipPattern();
    setState(748);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(747);
      match(LcypherParser::SP);
    }
    setState(750);
    oC_NodePattern();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_RelationshipPatternContext ------------------------------------------------------------------

LcypherParser::OC_RelationshipPatternContext::OC_RelationshipPatternContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_LeftArrowHeadContext* LcypherParser::OC_RelationshipPatternContext::oC_LeftArrowHead() {
  return getRuleContext<LcypherParser::OC_LeftArrowHeadContext>(0);
}

std::vector<LcypherParser::OC_DashContext *> LcypherParser::OC_RelationshipPatternContext::oC_Dash() {
  return getRuleContexts<LcypherParser::OC_DashContext>();
}

LcypherParser::OC_DashContext* LcypherParser::OC_RelationshipPatternContext::oC_Dash(size_t i) {
  return getRuleContext<LcypherParser::OC_DashContext>(i);
}

LcypherParser::OC_RightArrowHeadContext* LcypherParser::OC_RelationshipPatternContext::oC_RightArrowHead() {
  return getRuleContext<LcypherParser::OC_RightArrowHeadContext>(0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_RelationshipPatternContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_RelationshipPatternContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

LcypherParser::OC_RelationshipDetailContext* LcypherParser::OC_RelationshipPatternContext::oC_RelationshipDetail() {
  return getRuleContext<LcypherParser::OC_RelationshipDetailContext>(0);
}


size_t LcypherParser::OC_RelationshipPatternContext::getRuleIndex() const {
  return LcypherParser::RuleOC_RelationshipPattern;
}


antlrcpp::Any LcypherParser::OC_RelationshipPatternContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_RelationshipPattern(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_RelationshipPatternContext* LcypherParser::oC_RelationshipPattern() {
  OC_RelationshipPatternContext *_localctx = _tracker.createInstance<OC_RelationshipPatternContext>(_ctx, getState());
  enterRule(_localctx, 82, LcypherParser::RuleOC_RelationshipPattern);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(816);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 127, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(752);
      oC_LeftArrowHead();
      setState(754);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(753);
        match(LcypherParser::SP);
      }
      setState(756);
      oC_Dash();
      setState(758);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 112, _ctx)) {
      case 1: {
        setState(757);
        match(LcypherParser::SP);
        break;
      }

      default:
        break;
      }
      setState(761);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::T__7) {
        setState(760);
        oC_RelationshipDetail();
      }
      setState(764);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(763);
        match(LcypherParser::SP);
      }
      setState(766);
      oC_Dash();
      setState(768);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(767);
        match(LcypherParser::SP);
      }
      setState(770);
      oC_RightArrowHead();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(772);
      oC_LeftArrowHead();
      setState(774);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(773);
        match(LcypherParser::SP);
      }
      setState(776);
      oC_Dash();
      setState(778);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 117, _ctx)) {
      case 1: {
        setState(777);
        match(LcypherParser::SP);
        break;
      }

      default:
        break;
      }
      setState(781);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::T__7) {
        setState(780);
        oC_RelationshipDetail();
      }
      setState(784);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(783);
        match(LcypherParser::SP);
      }
      setState(786);
      oC_Dash();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(788);
      oC_Dash();
      setState(790);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 120, _ctx)) {
      case 1: {
        setState(789);
        match(LcypherParser::SP);
        break;
      }

      default:
        break;
      }
      setState(793);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::T__7) {
        setState(792);
        oC_RelationshipDetail();
      }
      setState(796);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(795);
        match(LcypherParser::SP);
      }
      setState(798);
      oC_Dash();
      setState(800);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(799);
        match(LcypherParser::SP);
      }
      setState(802);
      oC_RightArrowHead();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(804);
      oC_Dash();
      setState(806);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 124, _ctx)) {
      case 1: {
        setState(805);
        match(LcypherParser::SP);
        break;
      }

      default:
        break;
      }
      setState(809);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::T__7) {
        setState(808);
        oC_RelationshipDetail();
      }
      setState(812);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(811);
        match(LcypherParser::SP);
      }
      setState(814);
      oC_Dash();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_RelationshipDetailContext ------------------------------------------------------------------

LcypherParser::OC_RelationshipDetailContext::OC_RelationshipDetailContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> LcypherParser::OC_RelationshipDetailContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_RelationshipDetailContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

LcypherParser::OC_VariableContext* LcypherParser::OC_RelationshipDetailContext::oC_Variable() {
  return getRuleContext<LcypherParser::OC_VariableContext>(0);
}

LcypherParser::OC_RelationshipTypesContext* LcypherParser::OC_RelationshipDetailContext::oC_RelationshipTypes() {
  return getRuleContext<LcypherParser::OC_RelationshipTypesContext>(0);
}

LcypherParser::OC_RangeLiteralContext* LcypherParser::OC_RelationshipDetailContext::oC_RangeLiteral() {
  return getRuleContext<LcypherParser::OC_RangeLiteralContext>(0);
}

LcypherParser::OC_PropertiesContext* LcypherParser::OC_RelationshipDetailContext::oC_Properties() {
  return getRuleContext<LcypherParser::OC_PropertiesContext>(0);
}


size_t LcypherParser::OC_RelationshipDetailContext::getRuleIndex() const {
  return LcypherParser::RuleOC_RelationshipDetail;
}


antlrcpp::Any LcypherParser::OC_RelationshipDetailContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_RelationshipDetail(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_RelationshipDetailContext* LcypherParser::oC_RelationshipDetail() {
  OC_RelationshipDetailContext *_localctx = _tracker.createInstance<OC_RelationshipDetailContext>(_ctx, getState());
  enterRule(_localctx, 84, LcypherParser::RuleOC_RelationshipDetail);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(818);
    match(LcypherParser::T__7);
    setState(820);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(819);
      match(LcypherParser::SP);
    }
    setState(826);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 89) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 89)) & ((1ULL << (LcypherParser::COUNT - 89))
      | (1ULL << (LcypherParser::ANY - 89))
      | (1ULL << (LcypherParser::NONE - 89))
      | (1ULL << (LcypherParser::SINGLE - 89))
      | (1ULL << (LcypherParser::HexLetter - 89))
      | (1ULL << (LcypherParser::FILTER - 89))
      | (1ULL << (LcypherParser::EXTRACT - 89))
      | (1ULL << (LcypherParser::UnescapedSymbolicName - 89))
      | (1ULL << (LcypherParser::EscapedSymbolicName - 89)))) != 0)) {
      setState(822);
      oC_Variable();
      setState(824);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(823);
        match(LcypherParser::SP);
      }
    }
    setState(832);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::T__9) {
      setState(828);
      oC_RelationshipTypes();
      setState(830);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(829);
        match(LcypherParser::SP);
      }
    }
    setState(835);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::T__4) {
      setState(834);
      oC_RangeLiteral();
    }
    setState(841);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::T__23

    || _la == LcypherParser::T__25) {
      setState(837);
      oC_Properties();
      setState(839);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(838);
        match(LcypherParser::SP);
      }
    }
    setState(843);
    match(LcypherParser::T__8);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_PropertiesContext ------------------------------------------------------------------

LcypherParser::OC_PropertiesContext::OC_PropertiesContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_MapLiteralContext* LcypherParser::OC_PropertiesContext::oC_MapLiteral() {
  return getRuleContext<LcypherParser::OC_MapLiteralContext>(0);
}

LcypherParser::OC_ParameterContext* LcypherParser::OC_PropertiesContext::oC_Parameter() {
  return getRuleContext<LcypherParser::OC_ParameterContext>(0);
}


size_t LcypherParser::OC_PropertiesContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Properties;
}


antlrcpp::Any LcypherParser::OC_PropertiesContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Properties(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_PropertiesContext* LcypherParser::oC_Properties() {
  OC_PropertiesContext *_localctx = _tracker.createInstance<OC_PropertiesContext>(_ctx, getState());
  enterRule(_localctx, 86, LcypherParser::RuleOC_Properties);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(847);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case LcypherParser::T__23: {
        enterOuterAlt(_localctx, 1);
        setState(845);
        oC_MapLiteral();
        break;
      }

      case LcypherParser::T__25: {
        enterOuterAlt(_localctx, 2);
        setState(846);
        oC_Parameter();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_RelationshipTypesContext ------------------------------------------------------------------

LcypherParser::OC_RelationshipTypesContext::OC_RelationshipTypesContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<LcypherParser::OC_RelTypeNameContext *> LcypherParser::OC_RelationshipTypesContext::oC_RelTypeName() {
  return getRuleContexts<LcypherParser::OC_RelTypeNameContext>();
}

LcypherParser::OC_RelTypeNameContext* LcypherParser::OC_RelationshipTypesContext::oC_RelTypeName(size_t i) {
  return getRuleContext<LcypherParser::OC_RelTypeNameContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_RelationshipTypesContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_RelationshipTypesContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_RelationshipTypesContext::getRuleIndex() const {
  return LcypherParser::RuleOC_RelationshipTypes;
}


antlrcpp::Any LcypherParser::OC_RelationshipTypesContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_RelationshipTypes(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_RelationshipTypesContext* LcypherParser::oC_RelationshipTypes() {
  OC_RelationshipTypesContext *_localctx = _tracker.createInstance<OC_RelationshipTypesContext>(_ctx, getState());
  enterRule(_localctx, 88, LcypherParser::RuleOC_RelationshipTypes);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(849);
    match(LcypherParser::T__9);
    setState(851);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(850);
      match(LcypherParser::SP);
    }
    setState(853);
    oC_RelTypeName();
    setState(867);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 141, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(855);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(854);
          match(LcypherParser::SP);
        }
        setState(857);
        match(LcypherParser::T__10);
        setState(859);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::T__9) {
          setState(858);
          match(LcypherParser::T__9);
        }
        setState(862);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(861);
          match(LcypherParser::SP);
        }
        setState(864);
        oC_RelTypeName(); 
      }
      setState(869);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 141, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_NodeLabelsContext ------------------------------------------------------------------

LcypherParser::OC_NodeLabelsContext::OC_NodeLabelsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<LcypherParser::OC_NodeLabelContext *> LcypherParser::OC_NodeLabelsContext::oC_NodeLabel() {
  return getRuleContexts<LcypherParser::OC_NodeLabelContext>();
}

LcypherParser::OC_NodeLabelContext* LcypherParser::OC_NodeLabelsContext::oC_NodeLabel(size_t i) {
  return getRuleContext<LcypherParser::OC_NodeLabelContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_NodeLabelsContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_NodeLabelsContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_NodeLabelsContext::getRuleIndex() const {
  return LcypherParser::RuleOC_NodeLabels;
}


antlrcpp::Any LcypherParser::OC_NodeLabelsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_NodeLabels(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_NodeLabelsContext* LcypherParser::oC_NodeLabels() {
  OC_NodeLabelsContext *_localctx = _tracker.createInstance<OC_NodeLabelsContext>(_ctx, getState());
  enterRule(_localctx, 90, LcypherParser::RuleOC_NodeLabels);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(870);
    oC_NodeLabel();
    setState(877);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 143, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(872);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(871);
          match(LcypherParser::SP);
        }
        setState(874);
        oC_NodeLabel(); 
      }
      setState(879);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 143, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_NodeLabelContext ------------------------------------------------------------------

LcypherParser::OC_NodeLabelContext::OC_NodeLabelContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_LabelNameContext* LcypherParser::OC_NodeLabelContext::oC_LabelName() {
  return getRuleContext<LcypherParser::OC_LabelNameContext>(0);
}

tree::TerminalNode* LcypherParser::OC_NodeLabelContext::SP() {
  return getToken(LcypherParser::SP, 0);
}


size_t LcypherParser::OC_NodeLabelContext::getRuleIndex() const {
  return LcypherParser::RuleOC_NodeLabel;
}


antlrcpp::Any LcypherParser::OC_NodeLabelContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_NodeLabel(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_NodeLabelContext* LcypherParser::oC_NodeLabel() {
  OC_NodeLabelContext *_localctx = _tracker.createInstance<OC_NodeLabelContext>(_ctx, getState());
  enterRule(_localctx, 92, LcypherParser::RuleOC_NodeLabel);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(880);
    match(LcypherParser::T__9);
    setState(882);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(881);
      match(LcypherParser::SP);
    }
    setState(884);
    oC_LabelName();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_RangeLiteralContext ------------------------------------------------------------------

LcypherParser::OC_RangeLiteralContext::OC_RangeLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> LcypherParser::OC_RangeLiteralContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_RangeLiteralContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

std::vector<LcypherParser::OC_IntegerLiteralContext *> LcypherParser::OC_RangeLiteralContext::oC_IntegerLiteral() {
  return getRuleContexts<LcypherParser::OC_IntegerLiteralContext>();
}

LcypherParser::OC_IntegerLiteralContext* LcypherParser::OC_RangeLiteralContext::oC_IntegerLiteral(size_t i) {
  return getRuleContext<LcypherParser::OC_IntegerLiteralContext>(i);
}


size_t LcypherParser::OC_RangeLiteralContext::getRuleIndex() const {
  return LcypherParser::RuleOC_RangeLiteral;
}


antlrcpp::Any LcypherParser::OC_RangeLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_RangeLiteral(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_RangeLiteralContext* LcypherParser::oC_RangeLiteral() {
  OC_RangeLiteralContext *_localctx = _tracker.createInstance<OC_RangeLiteralContext>(_ctx, getState());
  enterRule(_localctx, 94, LcypherParser::RuleOC_RangeLiteral);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(886);
    match(LcypherParser::T__4);
    setState(888);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(887);
      match(LcypherParser::SP);
    }
    setState(894);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 103) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 103)) & ((1ULL << (LcypherParser::HexInteger - 103))
      | (1ULL << (LcypherParser::DecimalInteger - 103))
      | (1ULL << (LcypherParser::OctalInteger - 103)))) != 0)) {
      setState(890);
      oC_IntegerLiteral();
      setState(892);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(891);
        match(LcypherParser::SP);
      }
    }
    setState(906);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::T__11) {
      setState(896);
      match(LcypherParser::T__11);
      setState(898);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(897);
        match(LcypherParser::SP);
      }
      setState(904);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (((((_la - 103) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 103)) & ((1ULL << (LcypherParser::HexInteger - 103))
        | (1ULL << (LcypherParser::DecimalInteger - 103))
        | (1ULL << (LcypherParser::OctalInteger - 103)))) != 0)) {
        setState(900);
        oC_IntegerLiteral();
        setState(902);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(901);
          match(LcypherParser::SP);
        }
      }
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_LabelNameContext ------------------------------------------------------------------

LcypherParser::OC_LabelNameContext::OC_LabelNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_SchemaNameContext* LcypherParser::OC_LabelNameContext::oC_SchemaName() {
  return getRuleContext<LcypherParser::OC_SchemaNameContext>(0);
}


size_t LcypherParser::OC_LabelNameContext::getRuleIndex() const {
  return LcypherParser::RuleOC_LabelName;
}


antlrcpp::Any LcypherParser::OC_LabelNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_LabelName(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_LabelNameContext* LcypherParser::oC_LabelName() {
  OC_LabelNameContext *_localctx = _tracker.createInstance<OC_LabelNameContext>(_ctx, getState());
  enterRule(_localctx, 96, LcypherParser::RuleOC_LabelName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(908);
    oC_SchemaName();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_RelTypeNameContext ------------------------------------------------------------------

LcypherParser::OC_RelTypeNameContext::OC_RelTypeNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_SchemaNameContext* LcypherParser::OC_RelTypeNameContext::oC_SchemaName() {
  return getRuleContext<LcypherParser::OC_SchemaNameContext>(0);
}


size_t LcypherParser::OC_RelTypeNameContext::getRuleIndex() const {
  return LcypherParser::RuleOC_RelTypeName;
}


antlrcpp::Any LcypherParser::OC_RelTypeNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_RelTypeName(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_RelTypeNameContext* LcypherParser::oC_RelTypeName() {
  OC_RelTypeNameContext *_localctx = _tracker.createInstance<OC_RelTypeNameContext>(_ctx, getState());
  enterRule(_localctx, 98, LcypherParser::RuleOC_RelTypeName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(910);
    oC_SchemaName();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_ExpressionContext ------------------------------------------------------------------

LcypherParser::OC_ExpressionContext::OC_ExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_OrExpressionContext* LcypherParser::OC_ExpressionContext::oC_OrExpression() {
  return getRuleContext<LcypherParser::OC_OrExpressionContext>(0);
}


size_t LcypherParser::OC_ExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Expression;
}


antlrcpp::Any LcypherParser::OC_ExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Expression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_ExpressionContext* LcypherParser::oC_Expression() {
  OC_ExpressionContext *_localctx = _tracker.createInstance<OC_ExpressionContext>(_ctx, getState());
  enterRule(_localctx, 100, LcypherParser::RuleOC_Expression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(912);
    oC_OrExpression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_OrExpressionContext ------------------------------------------------------------------

LcypherParser::OC_OrExpressionContext::OC_OrExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<LcypherParser::OC_XorExpressionContext *> LcypherParser::OC_OrExpressionContext::oC_XorExpression() {
  return getRuleContexts<LcypherParser::OC_XorExpressionContext>();
}

LcypherParser::OC_XorExpressionContext* LcypherParser::OC_OrExpressionContext::oC_XorExpression(size_t i) {
  return getRuleContext<LcypherParser::OC_XorExpressionContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_OrExpressionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_OrExpressionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_OrExpressionContext::OR() {
  return getTokens(LcypherParser::OR);
}

tree::TerminalNode* LcypherParser::OC_OrExpressionContext::OR(size_t i) {
  return getToken(LcypherParser::OR, i);
}


size_t LcypherParser::OC_OrExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_OrExpression;
}


antlrcpp::Any LcypherParser::OC_OrExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_OrExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_OrExpressionContext* LcypherParser::oC_OrExpression() {
  OC_OrExpressionContext *_localctx = _tracker.createInstance<OC_OrExpressionContext>(_ctx, getState());
  enterRule(_localctx, 102, LcypherParser::RuleOC_OrExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(914);
    oC_XorExpression();
    setState(921);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 152, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(915);
        match(LcypherParser::SP);
        setState(916);
        match(LcypherParser::OR);
        setState(917);
        match(LcypherParser::SP);
        setState(918);
        oC_XorExpression(); 
      }
      setState(923);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 152, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_XorExpressionContext ------------------------------------------------------------------

LcypherParser::OC_XorExpressionContext::OC_XorExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<LcypherParser::OC_AndExpressionContext *> LcypherParser::OC_XorExpressionContext::oC_AndExpression() {
  return getRuleContexts<LcypherParser::OC_AndExpressionContext>();
}

LcypherParser::OC_AndExpressionContext* LcypherParser::OC_XorExpressionContext::oC_AndExpression(size_t i) {
  return getRuleContext<LcypherParser::OC_AndExpressionContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_XorExpressionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_XorExpressionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_XorExpressionContext::XOR() {
  return getTokens(LcypherParser::XOR);
}

tree::TerminalNode* LcypherParser::OC_XorExpressionContext::XOR(size_t i) {
  return getToken(LcypherParser::XOR, i);
}


size_t LcypherParser::OC_XorExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_XorExpression;
}


antlrcpp::Any LcypherParser::OC_XorExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_XorExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_XorExpressionContext* LcypherParser::oC_XorExpression() {
  OC_XorExpressionContext *_localctx = _tracker.createInstance<OC_XorExpressionContext>(_ctx, getState());
  enterRule(_localctx, 104, LcypherParser::RuleOC_XorExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(924);
    oC_AndExpression();
    setState(931);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 153, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(925);
        match(LcypherParser::SP);
        setState(926);
        match(LcypherParser::XOR);
        setState(927);
        match(LcypherParser::SP);
        setState(928);
        oC_AndExpression(); 
      }
      setState(933);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 153, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_AndExpressionContext ------------------------------------------------------------------

LcypherParser::OC_AndExpressionContext::OC_AndExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<LcypherParser::OC_NotExpressionContext *> LcypherParser::OC_AndExpressionContext::oC_NotExpression() {
  return getRuleContexts<LcypherParser::OC_NotExpressionContext>();
}

LcypherParser::OC_NotExpressionContext* LcypherParser::OC_AndExpressionContext::oC_NotExpression(size_t i) {
  return getRuleContext<LcypherParser::OC_NotExpressionContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_AndExpressionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_AndExpressionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_AndExpressionContext::AND() {
  return getTokens(LcypherParser::AND);
}

tree::TerminalNode* LcypherParser::OC_AndExpressionContext::AND(size_t i) {
  return getToken(LcypherParser::AND, i);
}


size_t LcypherParser::OC_AndExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_AndExpression;
}


antlrcpp::Any LcypherParser::OC_AndExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_AndExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_AndExpressionContext* LcypherParser::oC_AndExpression() {
  OC_AndExpressionContext *_localctx = _tracker.createInstance<OC_AndExpressionContext>(_ctx, getState());
  enterRule(_localctx, 106, LcypherParser::RuleOC_AndExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(934);
    oC_NotExpression();
    setState(941);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 154, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(935);
        match(LcypherParser::SP);
        setState(936);
        match(LcypherParser::AND);
        setState(937);
        match(LcypherParser::SP);
        setState(938);
        oC_NotExpression(); 
      }
      setState(943);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 154, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_NotExpressionContext ------------------------------------------------------------------

LcypherParser::OC_NotExpressionContext::OC_NotExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_ComparisonExpressionContext* LcypherParser::OC_NotExpressionContext::oC_ComparisonExpression() {
  return getRuleContext<LcypherParser::OC_ComparisonExpressionContext>(0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_NotExpressionContext::NOT() {
  return getTokens(LcypherParser::NOT);
}

tree::TerminalNode* LcypherParser::OC_NotExpressionContext::NOT(size_t i) {
  return getToken(LcypherParser::NOT, i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_NotExpressionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_NotExpressionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_NotExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_NotExpression;
}


antlrcpp::Any LcypherParser::OC_NotExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_NotExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_NotExpressionContext* LcypherParser::oC_NotExpression() {
  OC_NotExpressionContext *_localctx = _tracker.createInstance<OC_NotExpressionContext>(_ctx, getState());
  enterRule(_localctx, 108, LcypherParser::RuleOC_NotExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(950);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == LcypherParser::NOT) {
      setState(944);
      match(LcypherParser::NOT);
      setState(946);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(945);
        match(LcypherParser::SP);
      }
      setState(952);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(953);
    oC_ComparisonExpression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_ComparisonExpressionContext ------------------------------------------------------------------

LcypherParser::OC_ComparisonExpressionContext::OC_ComparisonExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_AddOrSubtractExpressionContext* LcypherParser::OC_ComparisonExpressionContext::oC_AddOrSubtractExpression() {
  return getRuleContext<LcypherParser::OC_AddOrSubtractExpressionContext>(0);
}

std::vector<LcypherParser::OC_PartialComparisonExpressionContext *> LcypherParser::OC_ComparisonExpressionContext::oC_PartialComparisonExpression() {
  return getRuleContexts<LcypherParser::OC_PartialComparisonExpressionContext>();
}

LcypherParser::OC_PartialComparisonExpressionContext* LcypherParser::OC_ComparisonExpressionContext::oC_PartialComparisonExpression(size_t i) {
  return getRuleContext<LcypherParser::OC_PartialComparisonExpressionContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_ComparisonExpressionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_ComparisonExpressionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_ComparisonExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_ComparisonExpression;
}


antlrcpp::Any LcypherParser::OC_ComparisonExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_ComparisonExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_ComparisonExpressionContext* LcypherParser::oC_ComparisonExpression() {
  OC_ComparisonExpressionContext *_localctx = _tracker.createInstance<OC_ComparisonExpressionContext>(_ctx, getState());
  enterRule(_localctx, 110, LcypherParser::RuleOC_ComparisonExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(955);
    oC_AddOrSubtractExpression();
    setState(962);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 158, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(957);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(956);
          match(LcypherParser::SP);
        }
        setState(959);
        oC_PartialComparisonExpression(); 
      }
      setState(964);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 158, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_AddOrSubtractExpressionContext ------------------------------------------------------------------

LcypherParser::OC_AddOrSubtractExpressionContext::OC_AddOrSubtractExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<LcypherParser::OC_MultiplyDivideModuloExpressionContext *> LcypherParser::OC_AddOrSubtractExpressionContext::oC_MultiplyDivideModuloExpression() {
  return getRuleContexts<LcypherParser::OC_MultiplyDivideModuloExpressionContext>();
}

LcypherParser::OC_MultiplyDivideModuloExpressionContext* LcypherParser::OC_AddOrSubtractExpressionContext::oC_MultiplyDivideModuloExpression(size_t i) {
  return getRuleContext<LcypherParser::OC_MultiplyDivideModuloExpressionContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_AddOrSubtractExpressionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_AddOrSubtractExpressionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_AddOrSubtractExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_AddOrSubtractExpression;
}


antlrcpp::Any LcypherParser::OC_AddOrSubtractExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_AddOrSubtractExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_AddOrSubtractExpressionContext* LcypherParser::oC_AddOrSubtractExpression() {
  OC_AddOrSubtractExpressionContext *_localctx = _tracker.createInstance<OC_AddOrSubtractExpressionContext>(_ctx, getState());
  enterRule(_localctx, 112, LcypherParser::RuleOC_AddOrSubtractExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(965);
    oC_MultiplyDivideModuloExpression();
    setState(984);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 164, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(982);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 163, _ctx)) {
        case 1: {
          setState(967);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == LcypherParser::SP) {
            setState(966);
            match(LcypherParser::SP);
          }
          setState(969);
          match(LcypherParser::T__12);
          setState(971);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == LcypherParser::SP) {
            setState(970);
            match(LcypherParser::SP);
          }
          setState(973);
          oC_MultiplyDivideModuloExpression();
          break;
        }

        case 2: {
          setState(975);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == LcypherParser::SP) {
            setState(974);
            match(LcypherParser::SP);
          }
          setState(977);
          match(LcypherParser::T__13);
          setState(979);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == LcypherParser::SP) {
            setState(978);
            match(LcypherParser::SP);
          }
          setState(981);
          oC_MultiplyDivideModuloExpression();
          break;
        }

        default:
          break;
        } 
      }
      setState(986);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 164, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_MultiplyDivideModuloExpressionContext ------------------------------------------------------------------

LcypherParser::OC_MultiplyDivideModuloExpressionContext::OC_MultiplyDivideModuloExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<LcypherParser::OC_PowerOfExpressionContext *> LcypherParser::OC_MultiplyDivideModuloExpressionContext::oC_PowerOfExpression() {
  return getRuleContexts<LcypherParser::OC_PowerOfExpressionContext>();
}

LcypherParser::OC_PowerOfExpressionContext* LcypherParser::OC_MultiplyDivideModuloExpressionContext::oC_PowerOfExpression(size_t i) {
  return getRuleContext<LcypherParser::OC_PowerOfExpressionContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_MultiplyDivideModuloExpressionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_MultiplyDivideModuloExpressionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_MultiplyDivideModuloExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_MultiplyDivideModuloExpression;
}


antlrcpp::Any LcypherParser::OC_MultiplyDivideModuloExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_MultiplyDivideModuloExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_MultiplyDivideModuloExpressionContext* LcypherParser::oC_MultiplyDivideModuloExpression() {
  OC_MultiplyDivideModuloExpressionContext *_localctx = _tracker.createInstance<OC_MultiplyDivideModuloExpressionContext>(_ctx, getState());
  enterRule(_localctx, 114, LcypherParser::RuleOC_MultiplyDivideModuloExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(987);
    oC_PowerOfExpression();
    setState(1014);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 172, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(1012);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 171, _ctx)) {
        case 1: {
          setState(989);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == LcypherParser::SP) {
            setState(988);
            match(LcypherParser::SP);
          }
          setState(991);
          match(LcypherParser::T__4);
          setState(993);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == LcypherParser::SP) {
            setState(992);
            match(LcypherParser::SP);
          }
          setState(995);
          oC_PowerOfExpression();
          break;
        }

        case 2: {
          setState(997);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == LcypherParser::SP) {
            setState(996);
            match(LcypherParser::SP);
          }
          setState(999);
          match(LcypherParser::T__14);
          setState(1001);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == LcypherParser::SP) {
            setState(1000);
            match(LcypherParser::SP);
          }
          setState(1003);
          oC_PowerOfExpression();
          break;
        }

        case 3: {
          setState(1005);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == LcypherParser::SP) {
            setState(1004);
            match(LcypherParser::SP);
          }
          setState(1007);
          match(LcypherParser::T__15);
          setState(1009);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == LcypherParser::SP) {
            setState(1008);
            match(LcypherParser::SP);
          }
          setState(1011);
          oC_PowerOfExpression();
          break;
        }

        default:
          break;
        } 
      }
      setState(1016);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 172, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_PowerOfExpressionContext ------------------------------------------------------------------

LcypherParser::OC_PowerOfExpressionContext::OC_PowerOfExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<LcypherParser::OC_UnaryAddOrSubtractExpressionContext *> LcypherParser::OC_PowerOfExpressionContext::oC_UnaryAddOrSubtractExpression() {
  return getRuleContexts<LcypherParser::OC_UnaryAddOrSubtractExpressionContext>();
}

LcypherParser::OC_UnaryAddOrSubtractExpressionContext* LcypherParser::OC_PowerOfExpressionContext::oC_UnaryAddOrSubtractExpression(size_t i) {
  return getRuleContext<LcypherParser::OC_UnaryAddOrSubtractExpressionContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_PowerOfExpressionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_PowerOfExpressionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_PowerOfExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_PowerOfExpression;
}


antlrcpp::Any LcypherParser::OC_PowerOfExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_PowerOfExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_PowerOfExpressionContext* LcypherParser::oC_PowerOfExpression() {
  OC_PowerOfExpressionContext *_localctx = _tracker.createInstance<OC_PowerOfExpressionContext>(_ctx, getState());
  enterRule(_localctx, 116, LcypherParser::RuleOC_PowerOfExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(1017);
    oC_UnaryAddOrSubtractExpression();
    setState(1028);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 175, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(1019);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1018);
          match(LcypherParser::SP);
        }
        setState(1021);
        match(LcypherParser::T__16);
        setState(1023);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1022);
          match(LcypherParser::SP);
        }
        setState(1025);
        oC_UnaryAddOrSubtractExpression(); 
      }
      setState(1030);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 175, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_UnaryAddOrSubtractExpressionContext ------------------------------------------------------------------

LcypherParser::OC_UnaryAddOrSubtractExpressionContext::OC_UnaryAddOrSubtractExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_StringListNullOperatorExpressionContext* LcypherParser::OC_UnaryAddOrSubtractExpressionContext::oC_StringListNullOperatorExpression() {
  return getRuleContext<LcypherParser::OC_StringListNullOperatorExpressionContext>(0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_UnaryAddOrSubtractExpressionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_UnaryAddOrSubtractExpressionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_UnaryAddOrSubtractExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_UnaryAddOrSubtractExpression;
}


antlrcpp::Any LcypherParser::OC_UnaryAddOrSubtractExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_UnaryAddOrSubtractExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_UnaryAddOrSubtractExpressionContext* LcypherParser::oC_UnaryAddOrSubtractExpression() {
  OC_UnaryAddOrSubtractExpressionContext *_localctx = _tracker.createInstance<OC_UnaryAddOrSubtractExpressionContext>(_ctx, getState());
  enterRule(_localctx, 118, LcypherParser::RuleOC_UnaryAddOrSubtractExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1037);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == LcypherParser::T__12

    || _la == LcypherParser::T__13) {
      setState(1031);
      _la = _input->LA(1);
      if (!(_la == LcypherParser::T__12

      || _la == LcypherParser::T__13)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(1033);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1032);
        match(LcypherParser::SP);
      }
      setState(1039);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(1040);
    oC_StringListNullOperatorExpression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_StringListNullOperatorExpressionContext ------------------------------------------------------------------

LcypherParser::OC_StringListNullOperatorExpressionContext::OC_StringListNullOperatorExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_PropertyOrLabelsExpressionContext* LcypherParser::OC_StringListNullOperatorExpressionContext::oC_PropertyOrLabelsExpression() {
  return getRuleContext<LcypherParser::OC_PropertyOrLabelsExpressionContext>(0);
}

std::vector<LcypherParser::OC_StringOperatorExpressionContext *> LcypherParser::OC_StringListNullOperatorExpressionContext::oC_StringOperatorExpression() {
  return getRuleContexts<LcypherParser::OC_StringOperatorExpressionContext>();
}

LcypherParser::OC_StringOperatorExpressionContext* LcypherParser::OC_StringListNullOperatorExpressionContext::oC_StringOperatorExpression(size_t i) {
  return getRuleContext<LcypherParser::OC_StringOperatorExpressionContext>(i);
}

std::vector<LcypherParser::OC_ListOperatorExpressionContext *> LcypherParser::OC_StringListNullOperatorExpressionContext::oC_ListOperatorExpression() {
  return getRuleContexts<LcypherParser::OC_ListOperatorExpressionContext>();
}

LcypherParser::OC_ListOperatorExpressionContext* LcypherParser::OC_StringListNullOperatorExpressionContext::oC_ListOperatorExpression(size_t i) {
  return getRuleContext<LcypherParser::OC_ListOperatorExpressionContext>(i);
}

std::vector<LcypherParser::OC_NullOperatorExpressionContext *> LcypherParser::OC_StringListNullOperatorExpressionContext::oC_NullOperatorExpression() {
  return getRuleContexts<LcypherParser::OC_NullOperatorExpressionContext>();
}

LcypherParser::OC_NullOperatorExpressionContext* LcypherParser::OC_StringListNullOperatorExpressionContext::oC_NullOperatorExpression(size_t i) {
  return getRuleContext<LcypherParser::OC_NullOperatorExpressionContext>(i);
}


size_t LcypherParser::OC_StringListNullOperatorExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_StringListNullOperatorExpression;
}


antlrcpp::Any LcypherParser::OC_StringListNullOperatorExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_StringListNullOperatorExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_StringListNullOperatorExpressionContext* LcypherParser::oC_StringListNullOperatorExpression() {
  OC_StringListNullOperatorExpressionContext *_localctx = _tracker.createInstance<OC_StringListNullOperatorExpressionContext>(_ctx, getState());
  enterRule(_localctx, 120, LcypherParser::RuleOC_StringListNullOperatorExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(1042);
    oC_PropertyOrLabelsExpression();
    setState(1048);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 179, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(1046);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 178, _ctx)) {
        case 1: {
          setState(1043);
          oC_StringOperatorExpression();
          break;
        }

        case 2: {
          setState(1044);
          oC_ListOperatorExpression();
          break;
        }

        case 3: {
          setState(1045);
          oC_NullOperatorExpression();
          break;
        }

        default:
          break;
        } 
      }
      setState(1050);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 179, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_ListOperatorExpressionContext ------------------------------------------------------------------

LcypherParser::OC_ListOperatorExpressionContext::OC_ListOperatorExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> LcypherParser::OC_ListOperatorExpressionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_ListOperatorExpressionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

tree::TerminalNode* LcypherParser::OC_ListOperatorExpressionContext::IN() {
  return getToken(LcypherParser::IN, 0);
}

LcypherParser::OC_PropertyOrLabelsExpressionContext* LcypherParser::OC_ListOperatorExpressionContext::oC_PropertyOrLabelsExpression() {
  return getRuleContext<LcypherParser::OC_PropertyOrLabelsExpressionContext>(0);
}

std::vector<LcypherParser::OC_ExpressionContext *> LcypherParser::OC_ListOperatorExpressionContext::oC_Expression() {
  return getRuleContexts<LcypherParser::OC_ExpressionContext>();
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_ListOperatorExpressionContext::oC_Expression(size_t i) {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(i);
}


size_t LcypherParser::OC_ListOperatorExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_ListOperatorExpression;
}


antlrcpp::Any LcypherParser::OC_ListOperatorExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_ListOperatorExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_ListOperatorExpressionContext* LcypherParser::oC_ListOperatorExpression() {
  OC_ListOperatorExpressionContext *_localctx = _tracker.createInstance<OC_ListOperatorExpressionContext>(_ctx, getState());
  enterRule(_localctx, 122, LcypherParser::RuleOC_ListOperatorExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(1076);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 185, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(1051);
      match(LcypherParser::SP);
      setState(1052);
      match(LcypherParser::IN);
      setState(1054);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1053);
        match(LcypherParser::SP);
      }
      setState(1056);
      oC_PropertyOrLabelsExpression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(1058);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1057);
        match(LcypherParser::SP);
      }
      setState(1060);
      match(LcypherParser::T__7);
      setState(1061);
      oC_Expression();
      setState(1062);
      match(LcypherParser::T__8);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(1065);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1064);
        match(LcypherParser::SP);
      }
      setState(1067);
      match(LcypherParser::T__7);
      setState(1069);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << LcypherParser::T__5)
        | (1ULL << LcypherParser::T__7)
        | (1ULL << LcypherParser::T__12)
        | (1ULL << LcypherParser::T__13)
        | (1ULL << LcypherParser::T__23)
        | (1ULL << LcypherParser::T__25)
        | (1ULL << LcypherParser::ALL))) != 0) || ((((_la - 81) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 81)) & ((1ULL << (LcypherParser::NOT - 81))
        | (1ULL << (LcypherParser::NULL_ - 81))
        | (1ULL << (LcypherParser::COUNT - 81))
        | (1ULL << (LcypherParser::ANY - 81))
        | (1ULL << (LcypherParser::NONE - 81))
        | (1ULL << (LcypherParser::SINGLE - 81))
        | (1ULL << (LcypherParser::TRUE_ - 81))
        | (1ULL << (LcypherParser::FALSE_ - 81))
        | (1ULL << (LcypherParser::EXISTS - 81))
        | (1ULL << (LcypherParser::CASE - 81))
        | (1ULL << (LcypherParser::StringLiteral - 81))
        | (1ULL << (LcypherParser::HexInteger - 81))
        | (1ULL << (LcypherParser::DecimalInteger - 81))
        | (1ULL << (LcypherParser::OctalInteger - 81))
        | (1ULL << (LcypherParser::HexLetter - 81))
        | (1ULL << (LcypherParser::ExponentDecimalReal - 81))
        | (1ULL << (LcypherParser::RegularDecimalReal - 81))
        | (1ULL << (LcypherParser::FILTER - 81))
        | (1ULL << (LcypherParser::EXTRACT - 81))
        | (1ULL << (LcypherParser::UnescapedSymbolicName - 81))
        | (1ULL << (LcypherParser::EscapedSymbolicName - 81)))) != 0)) {
        setState(1068);
        oC_Expression();
      }
      setState(1071);
      match(LcypherParser::T__11);
      setState(1073);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << LcypherParser::T__5)
        | (1ULL << LcypherParser::T__7)
        | (1ULL << LcypherParser::T__12)
        | (1ULL << LcypherParser::T__13)
        | (1ULL << LcypherParser::T__23)
        | (1ULL << LcypherParser::T__25)
        | (1ULL << LcypherParser::ALL))) != 0) || ((((_la - 81) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 81)) & ((1ULL << (LcypherParser::NOT - 81))
        | (1ULL << (LcypherParser::NULL_ - 81))
        | (1ULL << (LcypherParser::COUNT - 81))
        | (1ULL << (LcypherParser::ANY - 81))
        | (1ULL << (LcypherParser::NONE - 81))
        | (1ULL << (LcypherParser::SINGLE - 81))
        | (1ULL << (LcypherParser::TRUE_ - 81))
        | (1ULL << (LcypherParser::FALSE_ - 81))
        | (1ULL << (LcypherParser::EXISTS - 81))
        | (1ULL << (LcypherParser::CASE - 81))
        | (1ULL << (LcypherParser::StringLiteral - 81))
        | (1ULL << (LcypherParser::HexInteger - 81))
        | (1ULL << (LcypherParser::DecimalInteger - 81))
        | (1ULL << (LcypherParser::OctalInteger - 81))
        | (1ULL << (LcypherParser::HexLetter - 81))
        | (1ULL << (LcypherParser::ExponentDecimalReal - 81))
        | (1ULL << (LcypherParser::RegularDecimalReal - 81))
        | (1ULL << (LcypherParser::FILTER - 81))
        | (1ULL << (LcypherParser::EXTRACT - 81))
        | (1ULL << (LcypherParser::UnescapedSymbolicName - 81))
        | (1ULL << (LcypherParser::EscapedSymbolicName - 81)))) != 0)) {
        setState(1072);
        oC_Expression();
      }
      setState(1075);
      match(LcypherParser::T__8);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_StringOperatorExpressionContext ------------------------------------------------------------------

LcypherParser::OC_StringOperatorExpressionContext::OC_StringOperatorExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_PropertyOrLabelsExpressionContext* LcypherParser::OC_StringOperatorExpressionContext::oC_PropertyOrLabelsExpression() {
  return getRuleContext<LcypherParser::OC_PropertyOrLabelsExpressionContext>(0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_StringOperatorExpressionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_StringOperatorExpressionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

tree::TerminalNode* LcypherParser::OC_StringOperatorExpressionContext::STARTS() {
  return getToken(LcypherParser::STARTS, 0);
}

tree::TerminalNode* LcypherParser::OC_StringOperatorExpressionContext::WITH() {
  return getToken(LcypherParser::WITH, 0);
}

tree::TerminalNode* LcypherParser::OC_StringOperatorExpressionContext::ENDS() {
  return getToken(LcypherParser::ENDS, 0);
}

tree::TerminalNode* LcypherParser::OC_StringOperatorExpressionContext::CONTAINS() {
  return getToken(LcypherParser::CONTAINS, 0);
}

tree::TerminalNode* LcypherParser::OC_StringOperatorExpressionContext::REGEXP() {
  return getToken(LcypherParser::REGEXP, 0);
}


size_t LcypherParser::OC_StringOperatorExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_StringOperatorExpression;
}


antlrcpp::Any LcypherParser::OC_StringOperatorExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_StringOperatorExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_StringOperatorExpressionContext* LcypherParser::oC_StringOperatorExpression() {
  OC_StringOperatorExpressionContext *_localctx = _tracker.createInstance<OC_StringOperatorExpressionContext>(_ctx, getState());
  enterRule(_localctx, 124, LcypherParser::RuleOC_StringOperatorExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1090);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 186, _ctx)) {
    case 1: {
      setState(1078);
      match(LcypherParser::SP);
      setState(1079);
      match(LcypherParser::STARTS);
      setState(1080);
      match(LcypherParser::SP);
      setState(1081);
      match(LcypherParser::WITH);
      break;
    }

    case 2: {
      setState(1082);
      match(LcypherParser::SP);
      setState(1083);
      match(LcypherParser::ENDS);
      setState(1084);
      match(LcypherParser::SP);
      setState(1085);
      match(LcypherParser::WITH);
      break;
    }

    case 3: {
      setState(1086);
      match(LcypherParser::SP);
      setState(1087);
      match(LcypherParser::CONTAINS);
      break;
    }

    case 4: {
      setState(1088);
      match(LcypherParser::SP);
      setState(1089);
      match(LcypherParser::REGEXP);
      break;
    }

    default:
      break;
    }
    setState(1093);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1092);
      match(LcypherParser::SP);
    }
    setState(1095);
    oC_PropertyOrLabelsExpression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_NullOperatorExpressionContext ------------------------------------------------------------------

LcypherParser::OC_NullOperatorExpressionContext::OC_NullOperatorExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> LcypherParser::OC_NullOperatorExpressionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_NullOperatorExpressionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

tree::TerminalNode* LcypherParser::OC_NullOperatorExpressionContext::IS() {
  return getToken(LcypherParser::IS, 0);
}

tree::TerminalNode* LcypherParser::OC_NullOperatorExpressionContext::NULL_() {
  return getToken(LcypherParser::NULL_, 0);
}

tree::TerminalNode* LcypherParser::OC_NullOperatorExpressionContext::NOT() {
  return getToken(LcypherParser::NOT, 0);
}


size_t LcypherParser::OC_NullOperatorExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_NullOperatorExpression;
}


antlrcpp::Any LcypherParser::OC_NullOperatorExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_NullOperatorExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_NullOperatorExpressionContext* LcypherParser::oC_NullOperatorExpression() {
  OC_NullOperatorExpressionContext *_localctx = _tracker.createInstance<OC_NullOperatorExpressionContext>(_ctx, getState());
  enterRule(_localctx, 126, LcypherParser::RuleOC_NullOperatorExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(1107);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 188, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(1097);
      match(LcypherParser::SP);
      setState(1098);
      match(LcypherParser::IS);
      setState(1099);
      match(LcypherParser::SP);
      setState(1100);
      match(LcypherParser::NULL_);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(1101);
      match(LcypherParser::SP);
      setState(1102);
      match(LcypherParser::IS);
      setState(1103);
      match(LcypherParser::SP);
      setState(1104);
      match(LcypherParser::NOT);
      setState(1105);
      match(LcypherParser::SP);
      setState(1106);
      match(LcypherParser::NULL_);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_PropertyOrLabelsExpressionContext ------------------------------------------------------------------

LcypherParser::OC_PropertyOrLabelsExpressionContext::OC_PropertyOrLabelsExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_AtomContext* LcypherParser::OC_PropertyOrLabelsExpressionContext::oC_Atom() {
  return getRuleContext<LcypherParser::OC_AtomContext>(0);
}

std::vector<LcypherParser::OC_PropertyLookupContext *> LcypherParser::OC_PropertyOrLabelsExpressionContext::oC_PropertyLookup() {
  return getRuleContexts<LcypherParser::OC_PropertyLookupContext>();
}

LcypherParser::OC_PropertyLookupContext* LcypherParser::OC_PropertyOrLabelsExpressionContext::oC_PropertyLookup(size_t i) {
  return getRuleContext<LcypherParser::OC_PropertyLookupContext>(i);
}

LcypherParser::OC_NodeLabelsContext* LcypherParser::OC_PropertyOrLabelsExpressionContext::oC_NodeLabels() {
  return getRuleContext<LcypherParser::OC_NodeLabelsContext>(0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_PropertyOrLabelsExpressionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_PropertyOrLabelsExpressionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_PropertyOrLabelsExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_PropertyOrLabelsExpression;
}


antlrcpp::Any LcypherParser::OC_PropertyOrLabelsExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_PropertyOrLabelsExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_PropertyOrLabelsExpressionContext* LcypherParser::oC_PropertyOrLabelsExpression() {
  OC_PropertyOrLabelsExpressionContext *_localctx = _tracker.createInstance<OC_PropertyOrLabelsExpressionContext>(_ctx, getState());
  enterRule(_localctx, 128, LcypherParser::RuleOC_PropertyOrLabelsExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(1109);
    oC_Atom();
    setState(1116);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 190, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(1111);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1110);
          match(LcypherParser::SP);
        }
        setState(1113);
        oC_PropertyLookup(); 
      }
      setState(1118);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 190, _ctx);
    }
    setState(1123);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 192, _ctx)) {
    case 1: {
      setState(1120);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1119);
        match(LcypherParser::SP);
      }
      setState(1122);
      oC_NodeLabels();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_AtomContext ------------------------------------------------------------------

LcypherParser::OC_AtomContext::OC_AtomContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_LiteralContext* LcypherParser::OC_AtomContext::oC_Literal() {
  return getRuleContext<LcypherParser::OC_LiteralContext>(0);
}

LcypherParser::OC_ParameterContext* LcypherParser::OC_AtomContext::oC_Parameter() {
  return getRuleContext<LcypherParser::OC_ParameterContext>(0);
}

LcypherParser::OC_CaseExpressionContext* LcypherParser::OC_AtomContext::oC_CaseExpression() {
  return getRuleContext<LcypherParser::OC_CaseExpressionContext>(0);
}

tree::TerminalNode* LcypherParser::OC_AtomContext::COUNT() {
  return getToken(LcypherParser::COUNT, 0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_AtomContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_AtomContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

LcypherParser::OC_ListComprehensionContext* LcypherParser::OC_AtomContext::oC_ListComprehension() {
  return getRuleContext<LcypherParser::OC_ListComprehensionContext>(0);
}

LcypherParser::OC_PatternComprehensionContext* LcypherParser::OC_AtomContext::oC_PatternComprehension() {
  return getRuleContext<LcypherParser::OC_PatternComprehensionContext>(0);
}

tree::TerminalNode* LcypherParser::OC_AtomContext::ALL() {
  return getToken(LcypherParser::ALL, 0);
}

LcypherParser::OC_FilterExpressionContext* LcypherParser::OC_AtomContext::oC_FilterExpression() {
  return getRuleContext<LcypherParser::OC_FilterExpressionContext>(0);
}

tree::TerminalNode* LcypherParser::OC_AtomContext::ANY() {
  return getToken(LcypherParser::ANY, 0);
}

tree::TerminalNode* LcypherParser::OC_AtomContext::NONE() {
  return getToken(LcypherParser::NONE, 0);
}

tree::TerminalNode* LcypherParser::OC_AtomContext::SINGLE() {
  return getToken(LcypherParser::SINGLE, 0);
}

LcypherParser::OC_RelationshipsPatternContext* LcypherParser::OC_AtomContext::oC_RelationshipsPattern() {
  return getRuleContext<LcypherParser::OC_RelationshipsPatternContext>(0);
}

LcypherParser::OC_ParenthesizedExpressionContext* LcypherParser::OC_AtomContext::oC_ParenthesizedExpression() {
  return getRuleContext<LcypherParser::OC_ParenthesizedExpressionContext>(0);
}

LcypherParser::OC_FunctionInvocationContext* LcypherParser::OC_AtomContext::oC_FunctionInvocation() {
  return getRuleContext<LcypherParser::OC_FunctionInvocationContext>(0);
}

LcypherParser::OC_VariableContext* LcypherParser::OC_AtomContext::oC_Variable() {
  return getRuleContext<LcypherParser::OC_VariableContext>(0);
}


size_t LcypherParser::OC_AtomContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Atom;
}


antlrcpp::Any LcypherParser::OC_AtomContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Atom(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_AtomContext* LcypherParser::oC_Atom() {
  OC_AtomContext *_localctx = _tracker.createInstance<OC_AtomContext>(_ctx, getState());
  enterRule(_localctx, 130, LcypherParser::RuleOC_Atom);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(1203);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 208, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(1125);
      oC_Literal();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(1126);
      oC_Parameter();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(1127);
      oC_CaseExpression();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(1128);
      match(LcypherParser::COUNT);
      setState(1130);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1129);
        match(LcypherParser::SP);
      }
      setState(1132);
      match(LcypherParser::T__5);
      setState(1134);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1133);
        match(LcypherParser::SP);
      }
      setState(1136);
      match(LcypherParser::T__4);
      setState(1138);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1137);
        match(LcypherParser::SP);
      }
      setState(1140);
      match(LcypherParser::T__6);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(1141);
      oC_ListComprehension();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(1142);
      oC_PatternComprehension();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(1143);
      match(LcypherParser::ALL);
      setState(1145);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1144);
        match(LcypherParser::SP);
      }
      setState(1147);
      match(LcypherParser::T__5);
      setState(1149);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1148);
        match(LcypherParser::SP);
      }
      setState(1151);
      oC_FilterExpression();
      setState(1153);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1152);
        match(LcypherParser::SP);
      }
      setState(1155);
      match(LcypherParser::T__6);
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(1157);
      match(LcypherParser::ANY);
      setState(1159);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1158);
        match(LcypherParser::SP);
      }
      setState(1161);
      match(LcypherParser::T__5);
      setState(1163);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1162);
        match(LcypherParser::SP);
      }
      setState(1165);
      oC_FilterExpression();
      setState(1167);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1166);
        match(LcypherParser::SP);
      }
      setState(1169);
      match(LcypherParser::T__6);
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(1171);
      match(LcypherParser::NONE);
      setState(1173);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1172);
        match(LcypherParser::SP);
      }
      setState(1175);
      match(LcypherParser::T__5);
      setState(1177);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1176);
        match(LcypherParser::SP);
      }
      setState(1179);
      oC_FilterExpression();
      setState(1181);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1180);
        match(LcypherParser::SP);
      }
      setState(1183);
      match(LcypherParser::T__6);
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(1185);
      match(LcypherParser::SINGLE);
      setState(1187);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1186);
        match(LcypherParser::SP);
      }
      setState(1189);
      match(LcypherParser::T__5);
      setState(1191);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1190);
        match(LcypherParser::SP);
      }
      setState(1193);
      oC_FilterExpression();
      setState(1195);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1194);
        match(LcypherParser::SP);
      }
      setState(1197);
      match(LcypherParser::T__6);
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(1199);
      oC_RelationshipsPattern();
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(1200);
      oC_ParenthesizedExpression();
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(1201);
      oC_FunctionInvocation();
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(1202);
      oC_Variable();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_LiteralContext ------------------------------------------------------------------

LcypherParser::OC_LiteralContext::OC_LiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_NumberLiteralContext* LcypherParser::OC_LiteralContext::oC_NumberLiteral() {
  return getRuleContext<LcypherParser::OC_NumberLiteralContext>(0);
}

tree::TerminalNode* LcypherParser::OC_LiteralContext::StringLiteral() {
  return getToken(LcypherParser::StringLiteral, 0);
}

LcypherParser::OC_BooleanLiteralContext* LcypherParser::OC_LiteralContext::oC_BooleanLiteral() {
  return getRuleContext<LcypherParser::OC_BooleanLiteralContext>(0);
}

tree::TerminalNode* LcypherParser::OC_LiteralContext::NULL_() {
  return getToken(LcypherParser::NULL_, 0);
}

LcypherParser::OC_MapLiteralContext* LcypherParser::OC_LiteralContext::oC_MapLiteral() {
  return getRuleContext<LcypherParser::OC_MapLiteralContext>(0);
}

LcypherParser::OC_ListLiteralContext* LcypherParser::OC_LiteralContext::oC_ListLiteral() {
  return getRuleContext<LcypherParser::OC_ListLiteralContext>(0);
}


size_t LcypherParser::OC_LiteralContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Literal;
}


antlrcpp::Any LcypherParser::OC_LiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Literal(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_LiteralContext* LcypherParser::oC_Literal() {
  OC_LiteralContext *_localctx = _tracker.createInstance<OC_LiteralContext>(_ctx, getState());
  enterRule(_localctx, 132, LcypherParser::RuleOC_Literal);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(1211);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case LcypherParser::HexInteger:
      case LcypherParser::DecimalInteger:
      case LcypherParser::OctalInteger:
      case LcypherParser::ExponentDecimalReal:
      case LcypherParser::RegularDecimalReal: {
        enterOuterAlt(_localctx, 1);
        setState(1205);
        oC_NumberLiteral();
        break;
      }

      case LcypherParser::StringLiteral: {
        enterOuterAlt(_localctx, 2);
        setState(1206);
        match(LcypherParser::StringLiteral);
        break;
      }

      case LcypherParser::TRUE_:
      case LcypherParser::FALSE_: {
        enterOuterAlt(_localctx, 3);
        setState(1207);
        oC_BooleanLiteral();
        break;
      }

      case LcypherParser::NULL_: {
        enterOuterAlt(_localctx, 4);
        setState(1208);
        match(LcypherParser::NULL_);
        break;
      }

      case LcypherParser::T__23: {
        enterOuterAlt(_localctx, 5);
        setState(1209);
        oC_MapLiteral();
        break;
      }

      case LcypherParser::T__7: {
        enterOuterAlt(_localctx, 6);
        setState(1210);
        oC_ListLiteral();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_BooleanLiteralContext ------------------------------------------------------------------

LcypherParser::OC_BooleanLiteralContext::OC_BooleanLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_BooleanLiteralContext::TRUE_() {
  return getToken(LcypherParser::TRUE_, 0);
}

tree::TerminalNode* LcypherParser::OC_BooleanLiteralContext::FALSE_() {
  return getToken(LcypherParser::FALSE_, 0);
}


size_t LcypherParser::OC_BooleanLiteralContext::getRuleIndex() const {
  return LcypherParser::RuleOC_BooleanLiteral;
}


antlrcpp::Any LcypherParser::OC_BooleanLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_BooleanLiteral(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_BooleanLiteralContext* LcypherParser::oC_BooleanLiteral() {
  OC_BooleanLiteralContext *_localctx = _tracker.createInstance<OC_BooleanLiteralContext>(_ctx, getState());
  enterRule(_localctx, 134, LcypherParser::RuleOC_BooleanLiteral);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1213);
    _la = _input->LA(1);
    if (!(_la == LcypherParser::TRUE_

    || _la == LcypherParser::FALSE_)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_ListLiteralContext ------------------------------------------------------------------

LcypherParser::OC_ListLiteralContext::OC_ListLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> LcypherParser::OC_ListLiteralContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_ListLiteralContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

std::vector<LcypherParser::OC_ExpressionContext *> LcypherParser::OC_ListLiteralContext::oC_Expression() {
  return getRuleContexts<LcypherParser::OC_ExpressionContext>();
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_ListLiteralContext::oC_Expression(size_t i) {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(i);
}


size_t LcypherParser::OC_ListLiteralContext::getRuleIndex() const {
  return LcypherParser::RuleOC_ListLiteral;
}


antlrcpp::Any LcypherParser::OC_ListLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_ListLiteral(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_ListLiteralContext* LcypherParser::oC_ListLiteral() {
  OC_ListLiteralContext *_localctx = _tracker.createInstance<OC_ListLiteralContext>(_ctx, getState());
  enterRule(_localctx, 136, LcypherParser::RuleOC_ListLiteral);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1215);
    match(LcypherParser::T__7);
    setState(1217);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1216);
      match(LcypherParser::SP);
    }
    setState(1236);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << LcypherParser::T__5)
      | (1ULL << LcypherParser::T__7)
      | (1ULL << LcypherParser::T__12)
      | (1ULL << LcypherParser::T__13)
      | (1ULL << LcypherParser::T__23)
      | (1ULL << LcypherParser::T__25)
      | (1ULL << LcypherParser::ALL))) != 0) || ((((_la - 81) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 81)) & ((1ULL << (LcypherParser::NOT - 81))
      | (1ULL << (LcypherParser::NULL_ - 81))
      | (1ULL << (LcypherParser::COUNT - 81))
      | (1ULL << (LcypherParser::ANY - 81))
      | (1ULL << (LcypherParser::NONE - 81))
      | (1ULL << (LcypherParser::SINGLE - 81))
      | (1ULL << (LcypherParser::TRUE_ - 81))
      | (1ULL << (LcypherParser::FALSE_ - 81))
      | (1ULL << (LcypherParser::EXISTS - 81))
      | (1ULL << (LcypherParser::CASE - 81))
      | (1ULL << (LcypherParser::StringLiteral - 81))
      | (1ULL << (LcypherParser::HexInteger - 81))
      | (1ULL << (LcypherParser::DecimalInteger - 81))
      | (1ULL << (LcypherParser::OctalInteger - 81))
      | (1ULL << (LcypherParser::HexLetter - 81))
      | (1ULL << (LcypherParser::ExponentDecimalReal - 81))
      | (1ULL << (LcypherParser::RegularDecimalReal - 81))
      | (1ULL << (LcypherParser::FILTER - 81))
      | (1ULL << (LcypherParser::EXTRACT - 81))
      | (1ULL << (LcypherParser::UnescapedSymbolicName - 81))
      | (1ULL << (LcypherParser::EscapedSymbolicName - 81)))) != 0)) {
      setState(1219);
      oC_Expression();
      setState(1221);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1220);
        match(LcypherParser::SP);
      }
      setState(1233);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == LcypherParser::T__1) {
        setState(1223);
        match(LcypherParser::T__1);
        setState(1225);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1224);
          match(LcypherParser::SP);
        }
        setState(1227);
        oC_Expression();
        setState(1229);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1228);
          match(LcypherParser::SP);
        }
        setState(1235);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(1238);
    match(LcypherParser::T__8);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_PartialComparisonExpressionContext ------------------------------------------------------------------

LcypherParser::OC_PartialComparisonExpressionContext::OC_PartialComparisonExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_AddOrSubtractExpressionContext* LcypherParser::OC_PartialComparisonExpressionContext::oC_AddOrSubtractExpression() {
  return getRuleContext<LcypherParser::OC_AddOrSubtractExpressionContext>(0);
}

tree::TerminalNode* LcypherParser::OC_PartialComparisonExpressionContext::SP() {
  return getToken(LcypherParser::SP, 0);
}


size_t LcypherParser::OC_PartialComparisonExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_PartialComparisonExpression;
}


antlrcpp::Any LcypherParser::OC_PartialComparisonExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_PartialComparisonExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_PartialComparisonExpressionContext* LcypherParser::oC_PartialComparisonExpression() {
  OC_PartialComparisonExpressionContext *_localctx = _tracker.createInstance<OC_PartialComparisonExpressionContext>(_ctx, getState());
  enterRule(_localctx, 138, LcypherParser::RuleOC_PartialComparisonExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(1270);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case LcypherParser::T__2: {
        enterOuterAlt(_localctx, 1);
        setState(1240);
        match(LcypherParser::T__2);
        setState(1242);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1241);
          match(LcypherParser::SP);
        }
        setState(1244);
        oC_AddOrSubtractExpression();
        break;
      }

      case LcypherParser::T__17: {
        enterOuterAlt(_localctx, 2);
        setState(1245);
        match(LcypherParser::T__17);
        setState(1247);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1246);
          match(LcypherParser::SP);
        }
        setState(1249);
        oC_AddOrSubtractExpression();
        break;
      }

      case LcypherParser::T__18: {
        enterOuterAlt(_localctx, 3);
        setState(1250);
        match(LcypherParser::T__18);
        setState(1252);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1251);
          match(LcypherParser::SP);
        }
        setState(1254);
        oC_AddOrSubtractExpression();
        break;
      }

      case LcypherParser::T__19: {
        enterOuterAlt(_localctx, 4);
        setState(1255);
        match(LcypherParser::T__19);
        setState(1257);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1256);
          match(LcypherParser::SP);
        }
        setState(1259);
        oC_AddOrSubtractExpression();
        break;
      }

      case LcypherParser::T__20: {
        enterOuterAlt(_localctx, 5);
        setState(1260);
        match(LcypherParser::T__20);
        setState(1262);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1261);
          match(LcypherParser::SP);
        }
        setState(1264);
        oC_AddOrSubtractExpression();
        break;
      }

      case LcypherParser::T__21: {
        enterOuterAlt(_localctx, 6);
        setState(1265);
        match(LcypherParser::T__21);
        setState(1267);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1266);
          match(LcypherParser::SP);
        }
        setState(1269);
        oC_AddOrSubtractExpression();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_ParenthesizedExpressionContext ------------------------------------------------------------------

LcypherParser::OC_ParenthesizedExpressionContext::OC_ParenthesizedExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_ParenthesizedExpressionContext::oC_Expression() {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_ParenthesizedExpressionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_ParenthesizedExpressionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_ParenthesizedExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_ParenthesizedExpression;
}


antlrcpp::Any LcypherParser::OC_ParenthesizedExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_ParenthesizedExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_ParenthesizedExpressionContext* LcypherParser::oC_ParenthesizedExpression() {
  OC_ParenthesizedExpressionContext *_localctx = _tracker.createInstance<OC_ParenthesizedExpressionContext>(_ctx, getState());
  enterRule(_localctx, 140, LcypherParser::RuleOC_ParenthesizedExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1272);
    match(LcypherParser::T__5);
    setState(1274);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1273);
      match(LcypherParser::SP);
    }
    setState(1276);
    oC_Expression();
    setState(1278);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1277);
      match(LcypherParser::SP);
    }
    setState(1280);
    match(LcypherParser::T__6);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_RelationshipsPatternContext ------------------------------------------------------------------

LcypherParser::OC_RelationshipsPatternContext::OC_RelationshipsPatternContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_NodePatternContext* LcypherParser::OC_RelationshipsPatternContext::oC_NodePattern() {
  return getRuleContext<LcypherParser::OC_NodePatternContext>(0);
}

std::vector<LcypherParser::OC_PatternElementChainContext *> LcypherParser::OC_RelationshipsPatternContext::oC_PatternElementChain() {
  return getRuleContexts<LcypherParser::OC_PatternElementChainContext>();
}

LcypherParser::OC_PatternElementChainContext* LcypherParser::OC_RelationshipsPatternContext::oC_PatternElementChain(size_t i) {
  return getRuleContext<LcypherParser::OC_PatternElementChainContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_RelationshipsPatternContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_RelationshipsPatternContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_RelationshipsPatternContext::getRuleIndex() const {
  return LcypherParser::RuleOC_RelationshipsPattern;
}


antlrcpp::Any LcypherParser::OC_RelationshipsPatternContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_RelationshipsPattern(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_RelationshipsPatternContext* LcypherParser::oC_RelationshipsPattern() {
  OC_RelationshipsPatternContext *_localctx = _tracker.createInstance<OC_RelationshipsPatternContext>(_ctx, getState());
  enterRule(_localctx, 142, LcypherParser::RuleOC_RelationshipsPattern);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(1282);
    oC_NodePattern();
    setState(1287); 
    _errHandler->sync(this);
    alt = 1;
    do {
      switch (alt) {
        case 1: {
              setState(1284);
              _errHandler->sync(this);

              _la = _input->LA(1);
              if (_la == LcypherParser::SP) {
                setState(1283);
                match(LcypherParser::SP);
              }
              setState(1286);
              oC_PatternElementChain();
              break;
            }

      default:
        throw NoViableAltException(this);
      }
      setState(1289); 
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 226, _ctx);
    } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_FilterExpressionContext ------------------------------------------------------------------

LcypherParser::OC_FilterExpressionContext::OC_FilterExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_IdInCollContext* LcypherParser::OC_FilterExpressionContext::oC_IdInColl() {
  return getRuleContext<LcypherParser::OC_IdInCollContext>(0);
}

LcypherParser::OC_WhereContext* LcypherParser::OC_FilterExpressionContext::oC_Where() {
  return getRuleContext<LcypherParser::OC_WhereContext>(0);
}

tree::TerminalNode* LcypherParser::OC_FilterExpressionContext::SP() {
  return getToken(LcypherParser::SP, 0);
}


size_t LcypherParser::OC_FilterExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_FilterExpression;
}


antlrcpp::Any LcypherParser::OC_FilterExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_FilterExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_FilterExpressionContext* LcypherParser::oC_FilterExpression() {
  OC_FilterExpressionContext *_localctx = _tracker.createInstance<OC_FilterExpressionContext>(_ctx, getState());
  enterRule(_localctx, 144, LcypherParser::RuleOC_FilterExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1291);
    oC_IdInColl();
    setState(1296);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 228, _ctx)) {
    case 1: {
      setState(1293);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1292);
        match(LcypherParser::SP);
      }
      setState(1295);
      oC_Where();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_IdInCollContext ------------------------------------------------------------------

LcypherParser::OC_IdInCollContext::OC_IdInCollContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_VariableContext* LcypherParser::OC_IdInCollContext::oC_Variable() {
  return getRuleContext<LcypherParser::OC_VariableContext>(0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_IdInCollContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_IdInCollContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

tree::TerminalNode* LcypherParser::OC_IdInCollContext::IN() {
  return getToken(LcypherParser::IN, 0);
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_IdInCollContext::oC_Expression() {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(0);
}


size_t LcypherParser::OC_IdInCollContext::getRuleIndex() const {
  return LcypherParser::RuleOC_IdInColl;
}


antlrcpp::Any LcypherParser::OC_IdInCollContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_IdInColl(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_IdInCollContext* LcypherParser::oC_IdInColl() {
  OC_IdInCollContext *_localctx = _tracker.createInstance<OC_IdInCollContext>(_ctx, getState());
  enterRule(_localctx, 146, LcypherParser::RuleOC_IdInColl);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1298);
    oC_Variable();
    setState(1299);
    match(LcypherParser::SP);
    setState(1300);
    match(LcypherParser::IN);
    setState(1301);
    match(LcypherParser::SP);
    setState(1302);
    oC_Expression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_FunctionInvocationContext ------------------------------------------------------------------

LcypherParser::OC_FunctionInvocationContext::OC_FunctionInvocationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_FunctionNameContext* LcypherParser::OC_FunctionInvocationContext::oC_FunctionName() {
  return getRuleContext<LcypherParser::OC_FunctionNameContext>(0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_FunctionInvocationContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_FunctionInvocationContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

tree::TerminalNode* LcypherParser::OC_FunctionInvocationContext::DISTINCT() {
  return getToken(LcypherParser::DISTINCT, 0);
}

std::vector<LcypherParser::OC_ExpressionContext *> LcypherParser::OC_FunctionInvocationContext::oC_Expression() {
  return getRuleContexts<LcypherParser::OC_ExpressionContext>();
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_FunctionInvocationContext::oC_Expression(size_t i) {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(i);
}


size_t LcypherParser::OC_FunctionInvocationContext::getRuleIndex() const {
  return LcypherParser::RuleOC_FunctionInvocation;
}


antlrcpp::Any LcypherParser::OC_FunctionInvocationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_FunctionInvocation(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_FunctionInvocationContext* LcypherParser::oC_FunctionInvocation() {
  OC_FunctionInvocationContext *_localctx = _tracker.createInstance<OC_FunctionInvocationContext>(_ctx, getState());
  enterRule(_localctx, 148, LcypherParser::RuleOC_FunctionInvocation);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1304);
    oC_FunctionName();
    setState(1306);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1305);
      match(LcypherParser::SP);
    }
    setState(1308);
    match(LcypherParser::T__5);
    setState(1310);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1309);
      match(LcypherParser::SP);
    }
    setState(1316);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::DISTINCT) {
      setState(1312);
      match(LcypherParser::DISTINCT);
      setState(1314);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1313);
        match(LcypherParser::SP);
      }
    }
    setState(1335);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << LcypherParser::T__5)
      | (1ULL << LcypherParser::T__7)
      | (1ULL << LcypherParser::T__12)
      | (1ULL << LcypherParser::T__13)
      | (1ULL << LcypherParser::T__23)
      | (1ULL << LcypherParser::T__25)
      | (1ULL << LcypherParser::ALL))) != 0) || ((((_la - 81) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 81)) & ((1ULL << (LcypherParser::NOT - 81))
      | (1ULL << (LcypherParser::NULL_ - 81))
      | (1ULL << (LcypherParser::COUNT - 81))
      | (1ULL << (LcypherParser::ANY - 81))
      | (1ULL << (LcypherParser::NONE - 81))
      | (1ULL << (LcypherParser::SINGLE - 81))
      | (1ULL << (LcypherParser::TRUE_ - 81))
      | (1ULL << (LcypherParser::FALSE_ - 81))
      | (1ULL << (LcypherParser::EXISTS - 81))
      | (1ULL << (LcypherParser::CASE - 81))
      | (1ULL << (LcypherParser::StringLiteral - 81))
      | (1ULL << (LcypherParser::HexInteger - 81))
      | (1ULL << (LcypherParser::DecimalInteger - 81))
      | (1ULL << (LcypherParser::OctalInteger - 81))
      | (1ULL << (LcypherParser::HexLetter - 81))
      | (1ULL << (LcypherParser::ExponentDecimalReal - 81))
      | (1ULL << (LcypherParser::RegularDecimalReal - 81))
      | (1ULL << (LcypherParser::FILTER - 81))
      | (1ULL << (LcypherParser::EXTRACT - 81))
      | (1ULL << (LcypherParser::UnescapedSymbolicName - 81))
      | (1ULL << (LcypherParser::EscapedSymbolicName - 81)))) != 0)) {
      setState(1318);
      oC_Expression();
      setState(1320);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1319);
        match(LcypherParser::SP);
      }
      setState(1332);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == LcypherParser::T__1) {
        setState(1322);
        match(LcypherParser::T__1);
        setState(1324);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1323);
          match(LcypherParser::SP);
        }
        setState(1326);
        oC_Expression();
        setState(1328);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1327);
          match(LcypherParser::SP);
        }
        setState(1334);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(1337);
    match(LcypherParser::T__6);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_FunctionNameContext ------------------------------------------------------------------

LcypherParser::OC_FunctionNameContext::OC_FunctionNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_NamespaceContext* LcypherParser::OC_FunctionNameContext::oC_Namespace() {
  return getRuleContext<LcypherParser::OC_NamespaceContext>(0);
}

LcypherParser::OC_SymbolicNameContext* LcypherParser::OC_FunctionNameContext::oC_SymbolicName() {
  return getRuleContext<LcypherParser::OC_SymbolicNameContext>(0);
}

tree::TerminalNode* LcypherParser::OC_FunctionNameContext::EXISTS() {
  return getToken(LcypherParser::EXISTS, 0);
}


size_t LcypherParser::OC_FunctionNameContext::getRuleIndex() const {
  return LcypherParser::RuleOC_FunctionName;
}


antlrcpp::Any LcypherParser::OC_FunctionNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_FunctionName(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_FunctionNameContext* LcypherParser::oC_FunctionName() {
  OC_FunctionNameContext *_localctx = _tracker.createInstance<OC_FunctionNameContext>(_ctx, getState());
  enterRule(_localctx, 150, LcypherParser::RuleOC_FunctionName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(1343);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case LcypherParser::COUNT:
      case LcypherParser::ANY:
      case LcypherParser::NONE:
      case LcypherParser::SINGLE:
      case LcypherParser::HexLetter:
      case LcypherParser::FILTER:
      case LcypherParser::EXTRACT:
      case LcypherParser::UnescapedSymbolicName:
      case LcypherParser::EscapedSymbolicName: {
        enterOuterAlt(_localctx, 1);
        setState(1339);
        oC_Namespace();
        setState(1340);
        oC_SymbolicName();
        break;
      }

      case LcypherParser::EXISTS: {
        enterOuterAlt(_localctx, 2);
        setState(1342);
        match(LcypherParser::EXISTS);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_ExplicitProcedureInvocationContext ------------------------------------------------------------------

LcypherParser::OC_ExplicitProcedureInvocationContext::OC_ExplicitProcedureInvocationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_ProcedureNameContext* LcypherParser::OC_ExplicitProcedureInvocationContext::oC_ProcedureName() {
  return getRuleContext<LcypherParser::OC_ProcedureNameContext>(0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_ExplicitProcedureInvocationContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_ExplicitProcedureInvocationContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

std::vector<LcypherParser::OC_ExpressionContext *> LcypherParser::OC_ExplicitProcedureInvocationContext::oC_Expression() {
  return getRuleContexts<LcypherParser::OC_ExpressionContext>();
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_ExplicitProcedureInvocationContext::oC_Expression(size_t i) {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(i);
}


size_t LcypherParser::OC_ExplicitProcedureInvocationContext::getRuleIndex() const {
  return LcypherParser::RuleOC_ExplicitProcedureInvocation;
}


antlrcpp::Any LcypherParser::OC_ExplicitProcedureInvocationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_ExplicitProcedureInvocation(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_ExplicitProcedureInvocationContext* LcypherParser::oC_ExplicitProcedureInvocation() {
  OC_ExplicitProcedureInvocationContext *_localctx = _tracker.createInstance<OC_ExplicitProcedureInvocationContext>(_ctx, getState());
  enterRule(_localctx, 152, LcypherParser::RuleOC_ExplicitProcedureInvocation);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1345);
    oC_ProcedureName();
    setState(1347);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1346);
      match(LcypherParser::SP);
    }
    setState(1349);
    match(LcypherParser::T__5);
    setState(1351);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1350);
      match(LcypherParser::SP);
    }
    setState(1370);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << LcypherParser::T__5)
      | (1ULL << LcypherParser::T__7)
      | (1ULL << LcypherParser::T__12)
      | (1ULL << LcypherParser::T__13)
      | (1ULL << LcypherParser::T__23)
      | (1ULL << LcypherParser::T__25)
      | (1ULL << LcypherParser::ALL))) != 0) || ((((_la - 81) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 81)) & ((1ULL << (LcypherParser::NOT - 81))
      | (1ULL << (LcypherParser::NULL_ - 81))
      | (1ULL << (LcypherParser::COUNT - 81))
      | (1ULL << (LcypherParser::ANY - 81))
      | (1ULL << (LcypherParser::NONE - 81))
      | (1ULL << (LcypherParser::SINGLE - 81))
      | (1ULL << (LcypherParser::TRUE_ - 81))
      | (1ULL << (LcypherParser::FALSE_ - 81))
      | (1ULL << (LcypherParser::EXISTS - 81))
      | (1ULL << (LcypherParser::CASE - 81))
      | (1ULL << (LcypherParser::StringLiteral - 81))
      | (1ULL << (LcypherParser::HexInteger - 81))
      | (1ULL << (LcypherParser::DecimalInteger - 81))
      | (1ULL << (LcypherParser::OctalInteger - 81))
      | (1ULL << (LcypherParser::HexLetter - 81))
      | (1ULL << (LcypherParser::ExponentDecimalReal - 81))
      | (1ULL << (LcypherParser::RegularDecimalReal - 81))
      | (1ULL << (LcypherParser::FILTER - 81))
      | (1ULL << (LcypherParser::EXTRACT - 81))
      | (1ULL << (LcypherParser::UnescapedSymbolicName - 81))
      | (1ULL << (LcypherParser::EscapedSymbolicName - 81)))) != 0)) {
      setState(1353);
      oC_Expression();
      setState(1355);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1354);
        match(LcypherParser::SP);
      }
      setState(1367);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == LcypherParser::T__1) {
        setState(1357);
        match(LcypherParser::T__1);
        setState(1359);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1358);
          match(LcypherParser::SP);
        }
        setState(1361);
        oC_Expression();
        setState(1363);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1362);
          match(LcypherParser::SP);
        }
        setState(1369);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(1372);
    match(LcypherParser::T__6);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_ImplicitProcedureInvocationContext ------------------------------------------------------------------

LcypherParser::OC_ImplicitProcedureInvocationContext::OC_ImplicitProcedureInvocationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_ProcedureNameContext* LcypherParser::OC_ImplicitProcedureInvocationContext::oC_ProcedureName() {
  return getRuleContext<LcypherParser::OC_ProcedureNameContext>(0);
}


size_t LcypherParser::OC_ImplicitProcedureInvocationContext::getRuleIndex() const {
  return LcypherParser::RuleOC_ImplicitProcedureInvocation;
}


antlrcpp::Any LcypherParser::OC_ImplicitProcedureInvocationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_ImplicitProcedureInvocation(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_ImplicitProcedureInvocationContext* LcypherParser::oC_ImplicitProcedureInvocation() {
  OC_ImplicitProcedureInvocationContext *_localctx = _tracker.createInstance<OC_ImplicitProcedureInvocationContext>(_ctx, getState());
  enterRule(_localctx, 154, LcypherParser::RuleOC_ImplicitProcedureInvocation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1374);
    oC_ProcedureName();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_ProcedureResultFieldContext ------------------------------------------------------------------

LcypherParser::OC_ProcedureResultFieldContext::OC_ProcedureResultFieldContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_SymbolicNameContext* LcypherParser::OC_ProcedureResultFieldContext::oC_SymbolicName() {
  return getRuleContext<LcypherParser::OC_SymbolicNameContext>(0);
}


size_t LcypherParser::OC_ProcedureResultFieldContext::getRuleIndex() const {
  return LcypherParser::RuleOC_ProcedureResultField;
}


antlrcpp::Any LcypherParser::OC_ProcedureResultFieldContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_ProcedureResultField(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_ProcedureResultFieldContext* LcypherParser::oC_ProcedureResultField() {
  OC_ProcedureResultFieldContext *_localctx = _tracker.createInstance<OC_ProcedureResultFieldContext>(_ctx, getState());
  enterRule(_localctx, 156, LcypherParser::RuleOC_ProcedureResultField);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1376);
    oC_SymbolicName();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_ProcedureNameContext ------------------------------------------------------------------

LcypherParser::OC_ProcedureNameContext::OC_ProcedureNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_NamespaceContext* LcypherParser::OC_ProcedureNameContext::oC_Namespace() {
  return getRuleContext<LcypherParser::OC_NamespaceContext>(0);
}

LcypherParser::OC_SymbolicNameContext* LcypherParser::OC_ProcedureNameContext::oC_SymbolicName() {
  return getRuleContext<LcypherParser::OC_SymbolicNameContext>(0);
}


size_t LcypherParser::OC_ProcedureNameContext::getRuleIndex() const {
  return LcypherParser::RuleOC_ProcedureName;
}


antlrcpp::Any LcypherParser::OC_ProcedureNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_ProcedureName(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_ProcedureNameContext* LcypherParser::oC_ProcedureName() {
  OC_ProcedureNameContext *_localctx = _tracker.createInstance<OC_ProcedureNameContext>(_ctx, getState());
  enterRule(_localctx, 158, LcypherParser::RuleOC_ProcedureName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1378);
    oC_Namespace();
    setState(1379);
    oC_SymbolicName();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_NamespaceContext ------------------------------------------------------------------

LcypherParser::OC_NamespaceContext::OC_NamespaceContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<LcypherParser::OC_SymbolicNameContext *> LcypherParser::OC_NamespaceContext::oC_SymbolicName() {
  return getRuleContexts<LcypherParser::OC_SymbolicNameContext>();
}

LcypherParser::OC_SymbolicNameContext* LcypherParser::OC_NamespaceContext::oC_SymbolicName(size_t i) {
  return getRuleContext<LcypherParser::OC_SymbolicNameContext>(i);
}


size_t LcypherParser::OC_NamespaceContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Namespace;
}


antlrcpp::Any LcypherParser::OC_NamespaceContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Namespace(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_NamespaceContext* LcypherParser::oC_Namespace() {
  OC_NamespaceContext *_localctx = _tracker.createInstance<OC_NamespaceContext>(_ctx, getState());
  enterRule(_localctx, 160, LcypherParser::RuleOC_Namespace);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(1386);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 246, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(1381);
        oC_SymbolicName();
        setState(1382);
        match(LcypherParser::T__22); 
      }
      setState(1388);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 246, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_ListComprehensionContext ------------------------------------------------------------------

LcypherParser::OC_ListComprehensionContext::OC_ListComprehensionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_FilterExpressionContext* LcypherParser::OC_ListComprehensionContext::oC_FilterExpression() {
  return getRuleContext<LcypherParser::OC_FilterExpressionContext>(0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_ListComprehensionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_ListComprehensionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_ListComprehensionContext::oC_Expression() {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(0);
}


size_t LcypherParser::OC_ListComprehensionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_ListComprehension;
}


antlrcpp::Any LcypherParser::OC_ListComprehensionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_ListComprehension(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_ListComprehensionContext* LcypherParser::oC_ListComprehension() {
  OC_ListComprehensionContext *_localctx = _tracker.createInstance<OC_ListComprehensionContext>(_ctx, getState());
  enterRule(_localctx, 162, LcypherParser::RuleOC_ListComprehension);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1389);
    match(LcypherParser::T__7);
    setState(1391);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1390);
      match(LcypherParser::SP);
    }
    setState(1393);
    oC_FilterExpression();
    setState(1402);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 250, _ctx)) {
    case 1: {
      setState(1395);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1394);
        match(LcypherParser::SP);
      }
      setState(1397);
      match(LcypherParser::T__10);
      setState(1399);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1398);
        match(LcypherParser::SP);
      }
      setState(1401);
      oC_Expression();
      break;
    }

    default:
      break;
    }
    setState(1405);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1404);
      match(LcypherParser::SP);
    }
    setState(1407);
    match(LcypherParser::T__8);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_PatternComprehensionContext ------------------------------------------------------------------

LcypherParser::OC_PatternComprehensionContext::OC_PatternComprehensionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_RelationshipsPatternContext* LcypherParser::OC_PatternComprehensionContext::oC_RelationshipsPattern() {
  return getRuleContext<LcypherParser::OC_RelationshipsPatternContext>(0);
}

std::vector<LcypherParser::OC_ExpressionContext *> LcypherParser::OC_PatternComprehensionContext::oC_Expression() {
  return getRuleContexts<LcypherParser::OC_ExpressionContext>();
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_PatternComprehensionContext::oC_Expression(size_t i) {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_PatternComprehensionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_PatternComprehensionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

LcypherParser::OC_VariableContext* LcypherParser::OC_PatternComprehensionContext::oC_Variable() {
  return getRuleContext<LcypherParser::OC_VariableContext>(0);
}

tree::TerminalNode* LcypherParser::OC_PatternComprehensionContext::WHERE() {
  return getToken(LcypherParser::WHERE, 0);
}


size_t LcypherParser::OC_PatternComprehensionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_PatternComprehension;
}


antlrcpp::Any LcypherParser::OC_PatternComprehensionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_PatternComprehension(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_PatternComprehensionContext* LcypherParser::oC_PatternComprehension() {
  OC_PatternComprehensionContext *_localctx = _tracker.createInstance<OC_PatternComprehensionContext>(_ctx, getState());
  enterRule(_localctx, 164, LcypherParser::RuleOC_PatternComprehension);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1409);
    match(LcypherParser::T__7);
    setState(1411);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1410);
      match(LcypherParser::SP);
    }
    setState(1421);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 89) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 89)) & ((1ULL << (LcypherParser::COUNT - 89))
      | (1ULL << (LcypherParser::ANY - 89))
      | (1ULL << (LcypherParser::NONE - 89))
      | (1ULL << (LcypherParser::SINGLE - 89))
      | (1ULL << (LcypherParser::HexLetter - 89))
      | (1ULL << (LcypherParser::FILTER - 89))
      | (1ULL << (LcypherParser::EXTRACT - 89))
      | (1ULL << (LcypherParser::UnescapedSymbolicName - 89))
      | (1ULL << (LcypherParser::EscapedSymbolicName - 89)))) != 0)) {
      setState(1413);
      oC_Variable();
      setState(1415);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1414);
        match(LcypherParser::SP);
      }
      setState(1417);
      match(LcypherParser::T__2);
      setState(1419);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1418);
        match(LcypherParser::SP);
      }
    }
    setState(1423);
    oC_RelationshipsPattern();
    setState(1425);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1424);
      match(LcypherParser::SP);
    }
    setState(1435);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::WHERE) {
      setState(1427);
      match(LcypherParser::WHERE);
      setState(1429);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1428);
        match(LcypherParser::SP);
      }
      setState(1431);
      oC_Expression();
      setState(1433);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1432);
        match(LcypherParser::SP);
      }
    }
    setState(1437);
    match(LcypherParser::T__10);
    setState(1439);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1438);
      match(LcypherParser::SP);
    }
    setState(1441);
    oC_Expression();
    setState(1443);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1442);
      match(LcypherParser::SP);
    }
    setState(1445);
    match(LcypherParser::T__8);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_PropertyLookupContext ------------------------------------------------------------------

LcypherParser::OC_PropertyLookupContext::OC_PropertyLookupContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_PropertyKeyNameContext* LcypherParser::OC_PropertyLookupContext::oC_PropertyKeyName() {
  return getRuleContext<LcypherParser::OC_PropertyKeyNameContext>(0);
}

tree::TerminalNode* LcypherParser::OC_PropertyLookupContext::SP() {
  return getToken(LcypherParser::SP, 0);
}


size_t LcypherParser::OC_PropertyLookupContext::getRuleIndex() const {
  return LcypherParser::RuleOC_PropertyLookup;
}


antlrcpp::Any LcypherParser::OC_PropertyLookupContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_PropertyLookup(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_PropertyLookupContext* LcypherParser::oC_PropertyLookup() {
  OC_PropertyLookupContext *_localctx = _tracker.createInstance<OC_PropertyLookupContext>(_ctx, getState());
  enterRule(_localctx, 166, LcypherParser::RuleOC_PropertyLookup);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1447);
    match(LcypherParser::T__22);
    setState(1449);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1448);
      match(LcypherParser::SP);
    }

    setState(1451);
    oC_PropertyKeyName();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_CaseExpressionContext ------------------------------------------------------------------

LcypherParser::OC_CaseExpressionContext::OC_CaseExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_CaseExpressionContext::END() {
  return getToken(LcypherParser::END, 0);
}

tree::TerminalNode* LcypherParser::OC_CaseExpressionContext::ELSE() {
  return getToken(LcypherParser::ELSE, 0);
}

std::vector<LcypherParser::OC_ExpressionContext *> LcypherParser::OC_CaseExpressionContext::oC_Expression() {
  return getRuleContexts<LcypherParser::OC_ExpressionContext>();
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_CaseExpressionContext::oC_Expression(size_t i) {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_CaseExpressionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_CaseExpressionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

tree::TerminalNode* LcypherParser::OC_CaseExpressionContext::CASE() {
  return getToken(LcypherParser::CASE, 0);
}

std::vector<LcypherParser::OC_CaseAlternativesContext *> LcypherParser::OC_CaseExpressionContext::oC_CaseAlternatives() {
  return getRuleContexts<LcypherParser::OC_CaseAlternativesContext>();
}

LcypherParser::OC_CaseAlternativesContext* LcypherParser::OC_CaseExpressionContext::oC_CaseAlternatives(size_t i) {
  return getRuleContext<LcypherParser::OC_CaseAlternativesContext>(i);
}


size_t LcypherParser::OC_CaseExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_CaseExpression;
}


antlrcpp::Any LcypherParser::OC_CaseExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_CaseExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_CaseExpressionContext* LcypherParser::oC_CaseExpression() {
  OC_CaseExpressionContext *_localctx = _tracker.createInstance<OC_CaseExpressionContext>(_ctx, getState());
  enterRule(_localctx, 168, LcypherParser::RuleOC_CaseExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(1475);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 268, _ctx)) {
    case 1: {
      setState(1453);
      match(LcypherParser::CASE);
      setState(1458); 
      _errHandler->sync(this);
      alt = 1;
      do {
        switch (alt) {
          case 1: {
                setState(1455);
                _errHandler->sync(this);

                _la = _input->LA(1);
                if (_la == LcypherParser::SP) {
                  setState(1454);
                  match(LcypherParser::SP);
                }
                setState(1457);
                oC_CaseAlternatives();
                break;
              }

        default:
          throw NoViableAltException(this);
        }
        setState(1460); 
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 264, _ctx);
      } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
      break;
    }

    case 2: {
      setState(1462);
      match(LcypherParser::CASE);
      setState(1464);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1463);
        match(LcypherParser::SP);
      }
      setState(1466);
      oC_Expression();
      setState(1471); 
      _errHandler->sync(this);
      alt = 1;
      do {
        switch (alt) {
          case 1: {
                setState(1468);
                _errHandler->sync(this);

                _la = _input->LA(1);
                if (_la == LcypherParser::SP) {
                  setState(1467);
                  match(LcypherParser::SP);
                }
                setState(1470);
                oC_CaseAlternatives();
                break;
              }

        default:
          throw NoViableAltException(this);
        }
        setState(1473); 
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 267, _ctx);
      } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
      break;
    }

    default:
      break;
    }
    setState(1485);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 271, _ctx)) {
    case 1: {
      setState(1478);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1477);
        match(LcypherParser::SP);
      }
      setState(1480);
      match(LcypherParser::ELSE);
      setState(1482);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1481);
        match(LcypherParser::SP);
      }
      setState(1484);
      oC_Expression();
      break;
    }

    default:
      break;
    }
    setState(1488);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1487);
      match(LcypherParser::SP);
    }
    setState(1490);
    match(LcypherParser::END);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_CaseAlternativesContext ------------------------------------------------------------------

LcypherParser::OC_CaseAlternativesContext::OC_CaseAlternativesContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_CaseAlternativesContext::WHEN() {
  return getToken(LcypherParser::WHEN, 0);
}

std::vector<LcypherParser::OC_ExpressionContext *> LcypherParser::OC_CaseAlternativesContext::oC_Expression() {
  return getRuleContexts<LcypherParser::OC_ExpressionContext>();
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_CaseAlternativesContext::oC_Expression(size_t i) {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(i);
}

tree::TerminalNode* LcypherParser::OC_CaseAlternativesContext::THEN() {
  return getToken(LcypherParser::THEN, 0);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_CaseAlternativesContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_CaseAlternativesContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_CaseAlternativesContext::getRuleIndex() const {
  return LcypherParser::RuleOC_CaseAlternatives;
}


antlrcpp::Any LcypherParser::OC_CaseAlternativesContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_CaseAlternatives(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_CaseAlternativesContext* LcypherParser::oC_CaseAlternatives() {
  OC_CaseAlternativesContext *_localctx = _tracker.createInstance<OC_CaseAlternativesContext>(_ctx, getState());
  enterRule(_localctx, 170, LcypherParser::RuleOC_CaseAlternatives);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1492);
    match(LcypherParser::WHEN);
    setState(1494);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1493);
      match(LcypherParser::SP);
    }
    setState(1496);
    oC_Expression();
    setState(1498);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1497);
      match(LcypherParser::SP);
    }
    setState(1500);
    match(LcypherParser::THEN);
    setState(1502);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1501);
      match(LcypherParser::SP);
    }
    setState(1504);
    oC_Expression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_VariableContext ------------------------------------------------------------------

LcypherParser::OC_VariableContext::OC_VariableContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_SymbolicNameContext* LcypherParser::OC_VariableContext::oC_SymbolicName() {
  return getRuleContext<LcypherParser::OC_SymbolicNameContext>(0);
}


size_t LcypherParser::OC_VariableContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Variable;
}


antlrcpp::Any LcypherParser::OC_VariableContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Variable(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_VariableContext* LcypherParser::oC_Variable() {
  OC_VariableContext *_localctx = _tracker.createInstance<OC_VariableContext>(_ctx, getState());
  enterRule(_localctx, 172, LcypherParser::RuleOC_Variable);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1506);
    oC_SymbolicName();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_NumberLiteralContext ------------------------------------------------------------------

LcypherParser::OC_NumberLiteralContext::OC_NumberLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_DoubleLiteralContext* LcypherParser::OC_NumberLiteralContext::oC_DoubleLiteral() {
  return getRuleContext<LcypherParser::OC_DoubleLiteralContext>(0);
}

LcypherParser::OC_IntegerLiteralContext* LcypherParser::OC_NumberLiteralContext::oC_IntegerLiteral() {
  return getRuleContext<LcypherParser::OC_IntegerLiteralContext>(0);
}


size_t LcypherParser::OC_NumberLiteralContext::getRuleIndex() const {
  return LcypherParser::RuleOC_NumberLiteral;
}


antlrcpp::Any LcypherParser::OC_NumberLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_NumberLiteral(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_NumberLiteralContext* LcypherParser::oC_NumberLiteral() {
  OC_NumberLiteralContext *_localctx = _tracker.createInstance<OC_NumberLiteralContext>(_ctx, getState());
  enterRule(_localctx, 174, LcypherParser::RuleOC_NumberLiteral);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(1510);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case LcypherParser::ExponentDecimalReal:
      case LcypherParser::RegularDecimalReal: {
        enterOuterAlt(_localctx, 1);
        setState(1508);
        oC_DoubleLiteral();
        break;
      }

      case LcypherParser::HexInteger:
      case LcypherParser::DecimalInteger:
      case LcypherParser::OctalInteger: {
        enterOuterAlt(_localctx, 2);
        setState(1509);
        oC_IntegerLiteral();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_MapLiteralContext ------------------------------------------------------------------

LcypherParser::OC_MapLiteralContext::OC_MapLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> LcypherParser::OC_MapLiteralContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_MapLiteralContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}

std::vector<LcypherParser::OC_PropertyKeyNameContext *> LcypherParser::OC_MapLiteralContext::oC_PropertyKeyName() {
  return getRuleContexts<LcypherParser::OC_PropertyKeyNameContext>();
}

LcypherParser::OC_PropertyKeyNameContext* LcypherParser::OC_MapLiteralContext::oC_PropertyKeyName(size_t i) {
  return getRuleContext<LcypherParser::OC_PropertyKeyNameContext>(i);
}

std::vector<LcypherParser::OC_ExpressionContext *> LcypherParser::OC_MapLiteralContext::oC_Expression() {
  return getRuleContexts<LcypherParser::OC_ExpressionContext>();
}

LcypherParser::OC_ExpressionContext* LcypherParser::OC_MapLiteralContext::oC_Expression(size_t i) {
  return getRuleContext<LcypherParser::OC_ExpressionContext>(i);
}


size_t LcypherParser::OC_MapLiteralContext::getRuleIndex() const {
  return LcypherParser::RuleOC_MapLiteral;
}


antlrcpp::Any LcypherParser::OC_MapLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_MapLiteral(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_MapLiteralContext* LcypherParser::oC_MapLiteral() {
  OC_MapLiteralContext *_localctx = _tracker.createInstance<OC_MapLiteralContext>(_ctx, getState());
  enterRule(_localctx, 176, LcypherParser::RuleOC_MapLiteral);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1512);
    match(LcypherParser::T__23);
    setState(1514);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == LcypherParser::SP) {
      setState(1513);
      match(LcypherParser::SP);
    }
    setState(1549);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 48) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 48)) & ((1ULL << (LcypherParser::UNION - 48))
      | (1ULL << (LcypherParser::ALL - 48))
      | (1ULL << (LcypherParser::OPTIONAL_ - 48))
      | (1ULL << (LcypherParser::MATCH - 48))
      | (1ULL << (LcypherParser::UNWIND - 48))
      | (1ULL << (LcypherParser::AS - 48))
      | (1ULL << (LcypherParser::MERGE - 48))
      | (1ULL << (LcypherParser::ON - 48))
      | (1ULL << (LcypherParser::CREATE - 48))
      | (1ULL << (LcypherParser::SET - 48))
      | (1ULL << (LcypherParser::DETACH - 48))
      | (1ULL << (LcypherParser::DELETE_ - 48))
      | (1ULL << (LcypherParser::REMOVE - 48))
      | (1ULL << (LcypherParser::WITH - 48))
      | (1ULL << (LcypherParser::DISTINCT - 48))
      | (1ULL << (LcypherParser::RETURN - 48))
      | (1ULL << (LcypherParser::ORDER - 48))
      | (1ULL << (LcypherParser::BY - 48))
      | (1ULL << (LcypherParser::L_SKIP - 48))
      | (1ULL << (LcypherParser::LIMIT - 48))
      | (1ULL << (LcypherParser::ASCENDING - 48))
      | (1ULL << (LcypherParser::ASC - 48))
      | (1ULL << (LcypherParser::DESCENDING - 48))
      | (1ULL << (LcypherParser::DESC - 48))
      | (1ULL << (LcypherParser::WHERE - 48))
      | (1ULL << (LcypherParser::OR - 48))
      | (1ULL << (LcypherParser::XOR - 48))
      | (1ULL << (LcypherParser::AND - 48))
      | (1ULL << (LcypherParser::NOT - 48))
      | (1ULL << (LcypherParser::IN - 48))
      | (1ULL << (LcypherParser::STARTS - 48))
      | (1ULL << (LcypherParser::ENDS - 48))
      | (1ULL << (LcypherParser::CONTAINS - 48))
      | (1ULL << (LcypherParser::IS - 48))
      | (1ULL << (LcypherParser::NULL_ - 48))
      | (1ULL << (LcypherParser::COUNT - 48))
      | (1ULL << (LcypherParser::ANY - 48))
      | (1ULL << (LcypherParser::NONE - 48))
      | (1ULL << (LcypherParser::SINGLE - 48))
      | (1ULL << (LcypherParser::TRUE_ - 48))
      | (1ULL << (LcypherParser::FALSE_ - 48))
      | (1ULL << (LcypherParser::EXISTS - 48))
      | (1ULL << (LcypherParser::CASE - 48))
      | (1ULL << (LcypherParser::ELSE - 48))
      | (1ULL << (LcypherParser::END - 48))
      | (1ULL << (LcypherParser::WHEN - 48))
      | (1ULL << (LcypherParser::THEN - 48))
      | (1ULL << (LcypherParser::HexLetter - 48)))) != 0) || ((((_la - 115) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 115)) & ((1ULL << (LcypherParser::FILTER - 115))
      | (1ULL << (LcypherParser::EXTRACT - 115))
      | (1ULL << (LcypherParser::UnescapedSymbolicName - 115))
      | (1ULL << (LcypherParser::CONSTRAINT - 115))
      | (1ULL << (LcypherParser::DO - 115))
      | (1ULL << (LcypherParser::FOR - 115))
      | (1ULL << (LcypherParser::REQUIRE - 115))
      | (1ULL << (LcypherParser::UNIQUE - 115))
      | (1ULL << (LcypherParser::MANDATORY - 115))
      | (1ULL << (LcypherParser::SCALAR - 115))
      | (1ULL << (LcypherParser::OF - 115))
      | (1ULL << (LcypherParser::ADD - 115))
      | (1ULL << (LcypherParser::DROP - 115))
      | (1ULL << (LcypherParser::EscapedSymbolicName - 115)))) != 0)) {
      setState(1516);
      oC_PropertyKeyName();
      setState(1518);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1517);
        match(LcypherParser::SP);
      }
      setState(1520);
      match(LcypherParser::T__9);
      setState(1522);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1521);
        match(LcypherParser::SP);
      }
      setState(1524);
      oC_Expression();
      setState(1526);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == LcypherParser::SP) {
        setState(1525);
        match(LcypherParser::SP);
      }
      setState(1546);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == LcypherParser::T__1) {
        setState(1528);
        match(LcypherParser::T__1);
        setState(1530);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1529);
          match(LcypherParser::SP);
        }
        setState(1532);
        oC_PropertyKeyName();
        setState(1534);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1533);
          match(LcypherParser::SP);
        }
        setState(1536);
        match(LcypherParser::T__9);
        setState(1538);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1537);
          match(LcypherParser::SP);
        }
        setState(1540);
        oC_Expression();
        setState(1542);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == LcypherParser::SP) {
          setState(1541);
          match(LcypherParser::SP);
        }
        setState(1548);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(1551);
    match(LcypherParser::T__24);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_ParameterContext ------------------------------------------------------------------

LcypherParser::OC_ParameterContext::OC_ParameterContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_SymbolicNameContext* LcypherParser::OC_ParameterContext::oC_SymbolicName() {
  return getRuleContext<LcypherParser::OC_SymbolicNameContext>(0);
}

tree::TerminalNode* LcypherParser::OC_ParameterContext::DecimalInteger() {
  return getToken(LcypherParser::DecimalInteger, 0);
}


size_t LcypherParser::OC_ParameterContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Parameter;
}


antlrcpp::Any LcypherParser::OC_ParameterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Parameter(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_ParameterContext* LcypherParser::oC_Parameter() {
  OC_ParameterContext *_localctx = _tracker.createInstance<OC_ParameterContext>(_ctx, getState());
  enterRule(_localctx, 178, LcypherParser::RuleOC_Parameter);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1553);
    match(LcypherParser::T__25);
    setState(1556);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case LcypherParser::COUNT:
      case LcypherParser::ANY:
      case LcypherParser::NONE:
      case LcypherParser::SINGLE:
      case LcypherParser::HexLetter:
      case LcypherParser::FILTER:
      case LcypherParser::EXTRACT:
      case LcypherParser::UnescapedSymbolicName:
      case LcypherParser::EscapedSymbolicName: {
        setState(1554);
        oC_SymbolicName();
        break;
      }

      case LcypherParser::DecimalInteger: {
        setState(1555);
        match(LcypherParser::DecimalInteger);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_PropertyExpressionContext ------------------------------------------------------------------

LcypherParser::OC_PropertyExpressionContext::OC_PropertyExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_AtomContext* LcypherParser::OC_PropertyExpressionContext::oC_Atom() {
  return getRuleContext<LcypherParser::OC_AtomContext>(0);
}

std::vector<LcypherParser::OC_PropertyLookupContext *> LcypherParser::OC_PropertyExpressionContext::oC_PropertyLookup() {
  return getRuleContexts<LcypherParser::OC_PropertyLookupContext>();
}

LcypherParser::OC_PropertyLookupContext* LcypherParser::OC_PropertyExpressionContext::oC_PropertyLookup(size_t i) {
  return getRuleContext<LcypherParser::OC_PropertyLookupContext>(i);
}

std::vector<tree::TerminalNode *> LcypherParser::OC_PropertyExpressionContext::SP() {
  return getTokens(LcypherParser::SP);
}

tree::TerminalNode* LcypherParser::OC_PropertyExpressionContext::SP(size_t i) {
  return getToken(LcypherParser::SP, i);
}


size_t LcypherParser::OC_PropertyExpressionContext::getRuleIndex() const {
  return LcypherParser::RuleOC_PropertyExpression;
}


antlrcpp::Any LcypherParser::OC_PropertyExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_PropertyExpression(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_PropertyExpressionContext* LcypherParser::oC_PropertyExpression() {
  OC_PropertyExpressionContext *_localctx = _tracker.createInstance<OC_PropertyExpressionContext>(_ctx, getState());
  enterRule(_localctx, 180, LcypherParser::RuleOC_PropertyExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(1558);
    oC_Atom();
    setState(1563); 
    _errHandler->sync(this);
    alt = 1;
    do {
      switch (alt) {
        case 1: {
              setState(1560);
              _errHandler->sync(this);

              _la = _input->LA(1);
              if (_la == LcypherParser::SP) {
                setState(1559);
                match(LcypherParser::SP);
              }
              setState(1562);
              oC_PropertyLookup();
              break;
            }

      default:
        throw NoViableAltException(this);
      }
      setState(1565); 
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 289, _ctx);
    } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_PropertyKeyNameContext ------------------------------------------------------------------

LcypherParser::OC_PropertyKeyNameContext::OC_PropertyKeyNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_SchemaNameContext* LcypherParser::OC_PropertyKeyNameContext::oC_SchemaName() {
  return getRuleContext<LcypherParser::OC_SchemaNameContext>(0);
}


size_t LcypherParser::OC_PropertyKeyNameContext::getRuleIndex() const {
  return LcypherParser::RuleOC_PropertyKeyName;
}


antlrcpp::Any LcypherParser::OC_PropertyKeyNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_PropertyKeyName(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_PropertyKeyNameContext* LcypherParser::oC_PropertyKeyName() {
  OC_PropertyKeyNameContext *_localctx = _tracker.createInstance<OC_PropertyKeyNameContext>(_ctx, getState());
  enterRule(_localctx, 182, LcypherParser::RuleOC_PropertyKeyName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1567);
    oC_SchemaName();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_IntegerLiteralContext ------------------------------------------------------------------

LcypherParser::OC_IntegerLiteralContext::OC_IntegerLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_IntegerLiteralContext::HexInteger() {
  return getToken(LcypherParser::HexInteger, 0);
}

tree::TerminalNode* LcypherParser::OC_IntegerLiteralContext::OctalInteger() {
  return getToken(LcypherParser::OctalInteger, 0);
}

tree::TerminalNode* LcypherParser::OC_IntegerLiteralContext::DecimalInteger() {
  return getToken(LcypherParser::DecimalInteger, 0);
}


size_t LcypherParser::OC_IntegerLiteralContext::getRuleIndex() const {
  return LcypherParser::RuleOC_IntegerLiteral;
}


antlrcpp::Any LcypherParser::OC_IntegerLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_IntegerLiteral(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_IntegerLiteralContext* LcypherParser::oC_IntegerLiteral() {
  OC_IntegerLiteralContext *_localctx = _tracker.createInstance<OC_IntegerLiteralContext>(_ctx, getState());
  enterRule(_localctx, 184, LcypherParser::RuleOC_IntegerLiteral);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1569);
    _la = _input->LA(1);
    if (!(((((_la - 103) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 103)) & ((1ULL << (LcypherParser::HexInteger - 103))
      | (1ULL << (LcypherParser::DecimalInteger - 103))
      | (1ULL << (LcypherParser::OctalInteger - 103)))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_DoubleLiteralContext ------------------------------------------------------------------

LcypherParser::OC_DoubleLiteralContext::OC_DoubleLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_DoubleLiteralContext::ExponentDecimalReal() {
  return getToken(LcypherParser::ExponentDecimalReal, 0);
}

tree::TerminalNode* LcypherParser::OC_DoubleLiteralContext::RegularDecimalReal() {
  return getToken(LcypherParser::RegularDecimalReal, 0);
}


size_t LcypherParser::OC_DoubleLiteralContext::getRuleIndex() const {
  return LcypherParser::RuleOC_DoubleLiteral;
}


antlrcpp::Any LcypherParser::OC_DoubleLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_DoubleLiteral(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_DoubleLiteralContext* LcypherParser::oC_DoubleLiteral() {
  OC_DoubleLiteralContext *_localctx = _tracker.createInstance<OC_DoubleLiteralContext>(_ctx, getState());
  enterRule(_localctx, 186, LcypherParser::RuleOC_DoubleLiteral);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1571);
    _la = _input->LA(1);
    if (!(_la == LcypherParser::ExponentDecimalReal

    || _la == LcypherParser::RegularDecimalReal)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_SchemaNameContext ------------------------------------------------------------------

LcypherParser::OC_SchemaNameContext::OC_SchemaNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

LcypherParser::OC_SymbolicNameContext* LcypherParser::OC_SchemaNameContext::oC_SymbolicName() {
  return getRuleContext<LcypherParser::OC_SymbolicNameContext>(0);
}

LcypherParser::OC_ReservedWordContext* LcypherParser::OC_SchemaNameContext::oC_ReservedWord() {
  return getRuleContext<LcypherParser::OC_ReservedWordContext>(0);
}


size_t LcypherParser::OC_SchemaNameContext::getRuleIndex() const {
  return LcypherParser::RuleOC_SchemaName;
}


antlrcpp::Any LcypherParser::OC_SchemaNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_SchemaName(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_SchemaNameContext* LcypherParser::oC_SchemaName() {
  OC_SchemaNameContext *_localctx = _tracker.createInstance<OC_SchemaNameContext>(_ctx, getState());
  enterRule(_localctx, 188, LcypherParser::RuleOC_SchemaName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(1575);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case LcypherParser::COUNT:
      case LcypherParser::ANY:
      case LcypherParser::NONE:
      case LcypherParser::SINGLE:
      case LcypherParser::HexLetter:
      case LcypherParser::FILTER:
      case LcypherParser::EXTRACT:
      case LcypherParser::UnescapedSymbolicName:
      case LcypherParser::EscapedSymbolicName: {
        enterOuterAlt(_localctx, 1);
        setState(1573);
        oC_SymbolicName();
        break;
      }

      case LcypherParser::UNION:
      case LcypherParser::ALL:
      case LcypherParser::OPTIONAL_:
      case LcypherParser::MATCH:
      case LcypherParser::UNWIND:
      case LcypherParser::AS:
      case LcypherParser::MERGE:
      case LcypherParser::ON:
      case LcypherParser::CREATE:
      case LcypherParser::SET:
      case LcypherParser::DETACH:
      case LcypherParser::DELETE_:
      case LcypherParser::REMOVE:
      case LcypherParser::WITH:
      case LcypherParser::DISTINCT:
      case LcypherParser::RETURN:
      case LcypherParser::ORDER:
      case LcypherParser::BY:
      case LcypherParser::L_SKIP:
      case LcypherParser::LIMIT:
      case LcypherParser::ASCENDING:
      case LcypherParser::ASC:
      case LcypherParser::DESCENDING:
      case LcypherParser::DESC:
      case LcypherParser::WHERE:
      case LcypherParser::OR:
      case LcypherParser::XOR:
      case LcypherParser::AND:
      case LcypherParser::NOT:
      case LcypherParser::IN:
      case LcypherParser::STARTS:
      case LcypherParser::ENDS:
      case LcypherParser::CONTAINS:
      case LcypherParser::IS:
      case LcypherParser::NULL_:
      case LcypherParser::TRUE_:
      case LcypherParser::FALSE_:
      case LcypherParser::EXISTS:
      case LcypherParser::CASE:
      case LcypherParser::ELSE:
      case LcypherParser::END:
      case LcypherParser::WHEN:
      case LcypherParser::THEN:
      case LcypherParser::CONSTRAINT:
      case LcypherParser::DO:
      case LcypherParser::FOR:
      case LcypherParser::REQUIRE:
      case LcypherParser::UNIQUE:
      case LcypherParser::MANDATORY:
      case LcypherParser::SCALAR:
      case LcypherParser::OF:
      case LcypherParser::ADD:
      case LcypherParser::DROP: {
        enterOuterAlt(_localctx, 2);
        setState(1574);
        oC_ReservedWord();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_SymbolicNameContext ------------------------------------------------------------------

LcypherParser::OC_SymbolicNameContext::OC_SymbolicNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_SymbolicNameContext::UnescapedSymbolicName() {
  return getToken(LcypherParser::UnescapedSymbolicName, 0);
}

tree::TerminalNode* LcypherParser::OC_SymbolicNameContext::EscapedSymbolicName() {
  return getToken(LcypherParser::EscapedSymbolicName, 0);
}

tree::TerminalNode* LcypherParser::OC_SymbolicNameContext::HexLetter() {
  return getToken(LcypherParser::HexLetter, 0);
}

tree::TerminalNode* LcypherParser::OC_SymbolicNameContext::COUNT() {
  return getToken(LcypherParser::COUNT, 0);
}

tree::TerminalNode* LcypherParser::OC_SymbolicNameContext::FILTER() {
  return getToken(LcypherParser::FILTER, 0);
}

tree::TerminalNode* LcypherParser::OC_SymbolicNameContext::EXTRACT() {
  return getToken(LcypherParser::EXTRACT, 0);
}

tree::TerminalNode* LcypherParser::OC_SymbolicNameContext::ANY() {
  return getToken(LcypherParser::ANY, 0);
}

tree::TerminalNode* LcypherParser::OC_SymbolicNameContext::NONE() {
  return getToken(LcypherParser::NONE, 0);
}

tree::TerminalNode* LcypherParser::OC_SymbolicNameContext::SINGLE() {
  return getToken(LcypherParser::SINGLE, 0);
}


size_t LcypherParser::OC_SymbolicNameContext::getRuleIndex() const {
  return LcypherParser::RuleOC_SymbolicName;
}


antlrcpp::Any LcypherParser::OC_SymbolicNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_SymbolicName(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_SymbolicNameContext* LcypherParser::oC_SymbolicName() {
  OC_SymbolicNameContext *_localctx = _tracker.createInstance<OC_SymbolicNameContext>(_ctx, getState());
  enterRule(_localctx, 190, LcypherParser::RuleOC_SymbolicName);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1577);
    _la = _input->LA(1);
    if (!(((((_la - 89) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 89)) & ((1ULL << (LcypherParser::COUNT - 89))
      | (1ULL << (LcypherParser::ANY - 89))
      | (1ULL << (LcypherParser::NONE - 89))
      | (1ULL << (LcypherParser::SINGLE - 89))
      | (1ULL << (LcypherParser::HexLetter - 89))
      | (1ULL << (LcypherParser::FILTER - 89))
      | (1ULL << (LcypherParser::EXTRACT - 89))
      | (1ULL << (LcypherParser::UnescapedSymbolicName - 89))
      | (1ULL << (LcypherParser::EscapedSymbolicName - 89)))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_ReservedWordContext ------------------------------------------------------------------

LcypherParser::OC_ReservedWordContext::OC_ReservedWordContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::ALL() {
  return getToken(LcypherParser::ALL, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::ASC() {
  return getToken(LcypherParser::ASC, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::ASCENDING() {
  return getToken(LcypherParser::ASCENDING, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::BY() {
  return getToken(LcypherParser::BY, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::CREATE() {
  return getToken(LcypherParser::CREATE, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::DELETE_() {
  return getToken(LcypherParser::DELETE_, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::DESC() {
  return getToken(LcypherParser::DESC, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::DESCENDING() {
  return getToken(LcypherParser::DESCENDING, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::DETACH() {
  return getToken(LcypherParser::DETACH, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::EXISTS() {
  return getToken(LcypherParser::EXISTS, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::LIMIT() {
  return getToken(LcypherParser::LIMIT, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::MATCH() {
  return getToken(LcypherParser::MATCH, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::MERGE() {
  return getToken(LcypherParser::MERGE, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::ON() {
  return getToken(LcypherParser::ON, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::OPTIONAL_() {
  return getToken(LcypherParser::OPTIONAL_, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::ORDER() {
  return getToken(LcypherParser::ORDER, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::REMOVE() {
  return getToken(LcypherParser::REMOVE, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::RETURN() {
  return getToken(LcypherParser::RETURN, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::SET() {
  return getToken(LcypherParser::SET, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::L_SKIP() {
  return getToken(LcypherParser::L_SKIP, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::WHERE() {
  return getToken(LcypherParser::WHERE, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::WITH() {
  return getToken(LcypherParser::WITH, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::UNION() {
  return getToken(LcypherParser::UNION, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::UNWIND() {
  return getToken(LcypherParser::UNWIND, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::AND() {
  return getToken(LcypherParser::AND, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::AS() {
  return getToken(LcypherParser::AS, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::CONTAINS() {
  return getToken(LcypherParser::CONTAINS, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::DISTINCT() {
  return getToken(LcypherParser::DISTINCT, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::ENDS() {
  return getToken(LcypherParser::ENDS, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::IN() {
  return getToken(LcypherParser::IN, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::IS() {
  return getToken(LcypherParser::IS, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::NOT() {
  return getToken(LcypherParser::NOT, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::OR() {
  return getToken(LcypherParser::OR, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::STARTS() {
  return getToken(LcypherParser::STARTS, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::XOR() {
  return getToken(LcypherParser::XOR, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::FALSE_() {
  return getToken(LcypherParser::FALSE_, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::TRUE_() {
  return getToken(LcypherParser::TRUE_, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::NULL_() {
  return getToken(LcypherParser::NULL_, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::CONSTRAINT() {
  return getToken(LcypherParser::CONSTRAINT, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::DO() {
  return getToken(LcypherParser::DO, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::FOR() {
  return getToken(LcypherParser::FOR, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::REQUIRE() {
  return getToken(LcypherParser::REQUIRE, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::UNIQUE() {
  return getToken(LcypherParser::UNIQUE, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::CASE() {
  return getToken(LcypherParser::CASE, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::WHEN() {
  return getToken(LcypherParser::WHEN, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::THEN() {
  return getToken(LcypherParser::THEN, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::ELSE() {
  return getToken(LcypherParser::ELSE, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::END() {
  return getToken(LcypherParser::END, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::MANDATORY() {
  return getToken(LcypherParser::MANDATORY, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::SCALAR() {
  return getToken(LcypherParser::SCALAR, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::OF() {
  return getToken(LcypherParser::OF, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::ADD() {
  return getToken(LcypherParser::ADD, 0);
}

tree::TerminalNode* LcypherParser::OC_ReservedWordContext::DROP() {
  return getToken(LcypherParser::DROP, 0);
}


size_t LcypherParser::OC_ReservedWordContext::getRuleIndex() const {
  return LcypherParser::RuleOC_ReservedWord;
}


antlrcpp::Any LcypherParser::OC_ReservedWordContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_ReservedWord(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_ReservedWordContext* LcypherParser::oC_ReservedWord() {
  OC_ReservedWordContext *_localctx = _tracker.createInstance<OC_ReservedWordContext>(_ctx, getState());
  enterRule(_localctx, 192, LcypherParser::RuleOC_ReservedWord);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1579);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << LcypherParser::UNION)
      | (1ULL << LcypherParser::ALL)
      | (1ULL << LcypherParser::OPTIONAL_)
      | (1ULL << LcypherParser::MATCH)
      | (1ULL << LcypherParser::UNWIND)
      | (1ULL << LcypherParser::AS)
      | (1ULL << LcypherParser::MERGE)
      | (1ULL << LcypherParser::ON)
      | (1ULL << LcypherParser::CREATE)
      | (1ULL << LcypherParser::SET)
      | (1ULL << LcypherParser::DETACH)
      | (1ULL << LcypherParser::DELETE_)
      | (1ULL << LcypherParser::REMOVE)
      | (1ULL << LcypherParser::WITH))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & ((1ULL << (LcypherParser::DISTINCT - 64))
      | (1ULL << (LcypherParser::RETURN - 64))
      | (1ULL << (LcypherParser::ORDER - 64))
      | (1ULL << (LcypherParser::BY - 64))
      | (1ULL << (LcypherParser::L_SKIP - 64))
      | (1ULL << (LcypherParser::LIMIT - 64))
      | (1ULL << (LcypherParser::ASCENDING - 64))
      | (1ULL << (LcypherParser::ASC - 64))
      | (1ULL << (LcypherParser::DESCENDING - 64))
      | (1ULL << (LcypherParser::DESC - 64))
      | (1ULL << (LcypherParser::WHERE - 64))
      | (1ULL << (LcypherParser::OR - 64))
      | (1ULL << (LcypherParser::XOR - 64))
      | (1ULL << (LcypherParser::AND - 64))
      | (1ULL << (LcypherParser::NOT - 64))
      | (1ULL << (LcypherParser::IN - 64))
      | (1ULL << (LcypherParser::STARTS - 64))
      | (1ULL << (LcypherParser::ENDS - 64))
      | (1ULL << (LcypherParser::CONTAINS - 64))
      | (1ULL << (LcypherParser::IS - 64))
      | (1ULL << (LcypherParser::NULL_ - 64))
      | (1ULL << (LcypherParser::TRUE_ - 64))
      | (1ULL << (LcypherParser::FALSE_ - 64))
      | (1ULL << (LcypherParser::EXISTS - 64))
      | (1ULL << (LcypherParser::CASE - 64))
      | (1ULL << (LcypherParser::ELSE - 64))
      | (1ULL << (LcypherParser::END - 64))
      | (1ULL << (LcypherParser::WHEN - 64))
      | (1ULL << (LcypherParser::THEN - 64))
      | (1ULL << (LcypherParser::CONSTRAINT - 64))
      | (1ULL << (LcypherParser::DO - 64))
      | (1ULL << (LcypherParser::FOR - 64))
      | (1ULL << (LcypherParser::REQUIRE - 64))
      | (1ULL << (LcypherParser::UNIQUE - 64))
      | (1ULL << (LcypherParser::MANDATORY - 64))
      | (1ULL << (LcypherParser::SCALAR - 64))
      | (1ULL << (LcypherParser::OF - 64))
      | (1ULL << (LcypherParser::ADD - 64))
      | (1ULL << (LcypherParser::DROP - 64)))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_LeftArrowHeadContext ------------------------------------------------------------------

LcypherParser::OC_LeftArrowHeadContext::OC_LeftArrowHeadContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t LcypherParser::OC_LeftArrowHeadContext::getRuleIndex() const {
  return LcypherParser::RuleOC_LeftArrowHead;
}


antlrcpp::Any LcypherParser::OC_LeftArrowHeadContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_LeftArrowHead(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_LeftArrowHeadContext* LcypherParser::oC_LeftArrowHead() {
  OC_LeftArrowHeadContext *_localctx = _tracker.createInstance<OC_LeftArrowHeadContext>(_ctx, getState());
  enterRule(_localctx, 194, LcypherParser::RuleOC_LeftArrowHead);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1581);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << LcypherParser::T__18)
      | (1ULL << LcypherParser::T__26)
      | (1ULL << LcypherParser::T__27)
      | (1ULL << LcypherParser::T__28)
      | (1ULL << LcypherParser::T__29))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_RightArrowHeadContext ------------------------------------------------------------------

LcypherParser::OC_RightArrowHeadContext::OC_RightArrowHeadContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t LcypherParser::OC_RightArrowHeadContext::getRuleIndex() const {
  return LcypherParser::RuleOC_RightArrowHead;
}


antlrcpp::Any LcypherParser::OC_RightArrowHeadContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_RightArrowHead(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_RightArrowHeadContext* LcypherParser::oC_RightArrowHead() {
  OC_RightArrowHeadContext *_localctx = _tracker.createInstance<OC_RightArrowHeadContext>(_ctx, getState());
  enterRule(_localctx, 196, LcypherParser::RuleOC_RightArrowHead);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1583);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << LcypherParser::T__19)
      | (1ULL << LcypherParser::T__30)
      | (1ULL << LcypherParser::T__31)
      | (1ULL << LcypherParser::T__32)
      | (1ULL << LcypherParser::T__33))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OC_DashContext ------------------------------------------------------------------

LcypherParser::OC_DashContext::OC_DashContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t LcypherParser::OC_DashContext::getRuleIndex() const {
  return LcypherParser::RuleOC_Dash;
}


antlrcpp::Any LcypherParser::OC_DashContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<LcypherVisitor*>(visitor))
    return parserVisitor->visitOC_Dash(this);
  else
    return visitor->visitChildren(this);
}

LcypherParser::OC_DashContext* LcypherParser::oC_Dash() {
  OC_DashContext *_localctx = _tracker.createInstance<OC_DashContext>(_ctx, getState());
  enterRule(_localctx, 198, LcypherParser::RuleOC_Dash);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1585);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << LcypherParser::T__13)
      | (1ULL << LcypherParser::T__34)
      | (1ULL << LcypherParser::T__35)
      | (1ULL << LcypherParser::T__36)
      | (1ULL << LcypherParser::T__37)
      | (1ULL << LcypherParser::T__38)
      | (1ULL << LcypherParser::T__39)
      | (1ULL << LcypherParser::T__40)
      | (1ULL << LcypherParser::T__41)
      | (1ULL << LcypherParser::T__42)
      | (1ULL << LcypherParser::T__43)
      | (1ULL << LcypherParser::T__44))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

// Static vars and initialization.
std::vector<dfa::DFA> LcypherParser::_decisionToDFA;
atn::PredictionContextCache LcypherParser::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN LcypherParser::_atn;
std::vector<uint16_t> LcypherParser::_serializedATN;

std::vector<std::string> LcypherParser::_ruleNames = {
  "oC_Cypher", "oC_Statement", "oC_Query", "oC_RegularQuery", "oC_Union", 
  "oC_SingleQuery", "oC_SinglePartQuery", "oC_MultiPartQuery", "oC_UpdatingClause", 
  "oC_ReadingClause", "oC_Match", "oC_Unwind", "oC_Merge", "oC_MergeAction", 
  "oC_Create", "oC_Set", "oC_SetItem", "oC_Delete", "oC_Remove", "oC_RemoveItem", 
  "oC_InQueryCall", "oC_StandaloneCall", "oC_YieldItems", "oC_YieldItem", 
  "oC_With", "oC_Return", "oC_ReturnBody", "oC_ReturnItems", "oC_ReturnItem", 
  "oC_Order", "oC_Skip", "oC_Limit", "oC_SortItem", "oC_Hint", "oC_Where", 
  "oC_Pattern", "oC_PatternPart", "oC_AnonymousPatternPart", "oC_PatternElement", 
  "oC_NodePattern", "oC_PatternElementChain", "oC_RelationshipPattern", 
  "oC_RelationshipDetail", "oC_Properties", "oC_RelationshipTypes", "oC_NodeLabels", 
  "oC_NodeLabel", "oC_RangeLiteral", "oC_LabelName", "oC_RelTypeName", "oC_Expression", 
  "oC_OrExpression", "oC_XorExpression", "oC_AndExpression", "oC_NotExpression", 
  "oC_ComparisonExpression", "oC_AddOrSubtractExpression", "oC_MultiplyDivideModuloExpression", 
  "oC_PowerOfExpression", "oC_UnaryAddOrSubtractExpression", "oC_StringListNullOperatorExpression", 
  "oC_ListOperatorExpression", "oC_StringOperatorExpression", "oC_NullOperatorExpression", 
  "oC_PropertyOrLabelsExpression", "oC_Atom", "oC_Literal", "oC_BooleanLiteral", 
  "oC_ListLiteral", "oC_PartialComparisonExpression", "oC_ParenthesizedExpression", 
  "oC_RelationshipsPattern", "oC_FilterExpression", "oC_IdInColl", "oC_FunctionInvocation", 
  "oC_FunctionName", "oC_ExplicitProcedureInvocation", "oC_ImplicitProcedureInvocation", 
  "oC_ProcedureResultField", "oC_ProcedureName", "oC_Namespace", "oC_ListComprehension", 
  "oC_PatternComprehension", "oC_PropertyLookup", "oC_CaseExpression", "oC_CaseAlternatives", 
  "oC_Variable", "oC_NumberLiteral", "oC_MapLiteral", "oC_Parameter", "oC_PropertyExpression", 
  "oC_PropertyKeyName", "oC_IntegerLiteral", "oC_DoubleLiteral", "oC_SchemaName", 
  "oC_SymbolicName", "oC_ReservedWord", "oC_LeftArrowHead", "oC_RightArrowHead", 
  "oC_Dash"
};

std::vector<std::string> LcypherParser::_literalNames = {
  "", "';'", "','", "'='", "'+='", "'*'", "'('", "')'", "'['", "']'", "':'", 
  "'|'", "'..'", "'+'", "'-'", "'/'", "'%'", "'^'", "'<>'", "'<'", "'>'", 
  "'<='", "'>='", "'.'", "'{'", "'}'", "'$'", "'\u27E8'", "'\u3008'", "'\uFE64'", 
  "'\uFF1C'", "'\u27E9'", "'\u3009'", "'\uFE65'", "'\uFF1E'", "'\u00AD'", 
  "'\u2010'", "'\u2011'", "'\u2012'", "'\u2013'", "'\u2014'", "'\u2015'", 
  "'\u2212'", "'\uFE58'", "'\uFE63'", "'\uFF0D'", "", "", "", "", "", "", 
  "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
  "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
  "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
  "", "", "", "", "", "", "'0'"
};

std::vector<std::string> LcypherParser::_symbolicNames = {
  "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
  "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
  "", "", "", "", "", "", "", "", "", "", "EXPLAIN", "PROFILE", "UNION", 
  "ALL", "OPTIONAL_", "MATCH", "UNWIND", "AS", "MERGE", "ON", "CREATE", 
  "SET", "DETACH", "DELETE_", "REMOVE", "CALL", "YIELD", "WITH", "DISTINCT", 
  "RETURN", "ORDER", "BY", "L_SKIP", "LIMIT", "ASCENDING", "ASC", "DESCENDING", 
  "DESC", "USING", "JOIN", "START", "WHERE", "OR", "XOR", "AND", "NOT", 
  "IN", "STARTS", "ENDS", "CONTAINS", "REGEXP", "IS", "NULL_", "COUNT", 
  "ANY", "NONE", "SINGLE", "TRUE_", "FALSE_", "EXISTS", "CASE", "ELSE", 
  "END", "WHEN", "THEN", "StringLiteral", "EscapedChar", "HexInteger", "DecimalInteger", 
  "OctalInteger", "HexLetter", "HexDigit", "Digit", "NonZeroDigit", "NonZeroOctDigit", 
  "OctDigit", "ZeroDigit", "ExponentDecimalReal", "RegularDecimalReal", 
  "FILTER", "EXTRACT", "UnescapedSymbolicName", "CONSTRAINT", "DO", "FOR", 
  "REQUIRE", "UNIQUE", "MANDATORY", "SCALAR", "OF", "ADD", "DROP", "IdentifierStart", 
  "IdentifierPart", "EscapedSymbolicName", "SP", "WHITESPACE", "Comment"
};

dfa::Vocabulary LcypherParser::_vocabulary(_literalNames, _symbolicNames);

std::vector<std::string> LcypherParser::_tokenNames;

LcypherParser::Initializer::Initializer() {
	for (size_t i = 0; i < _symbolicNames.size(); ++i) {
		std::string name = _vocabulary.getLiteralName(i);
		if (name.empty()) {
			name = _vocabulary.getSymbolicName(i);
		}

		if (name.empty()) {
			_tokenNames.push_back("<INVALID>");
		} else {
      _tokenNames.push_back(name);
    }
	}

  static const uint16_t serializedATNSegment0[] = {
    0x3, 0x608b, 0xa72a, 0x8133, 0xb9ed, 0x417c, 0x3be7, 0x7786, 0x5964, 
       0x3, 0x87, 0x636, 0x4, 0x2, 0x9, 0x2, 0x4, 0x3, 0x9, 0x3, 0x4, 0x4, 
       0x9, 0x4, 0x4, 0x5, 0x9, 0x5, 0x4, 0x6, 0x9, 0x6, 0x4, 0x7, 0x9, 
       0x7, 0x4, 0x8, 0x9, 0x8, 0x4, 0x9, 0x9, 0x9, 0x4, 0xa, 0x9, 0xa, 
       0x4, 0xb, 0x9, 0xb, 0x4, 0xc, 0x9, 0xc, 0x4, 0xd, 0x9, 0xd, 0x4, 
       0xe, 0x9, 0xe, 0x4, 0xf, 0x9, 0xf, 0x4, 0x10, 0x9, 0x10, 0x4, 0x11, 
       0x9, 0x11, 0x4, 0x12, 0x9, 0x12, 0x4, 0x13, 0x9, 0x13, 0x4, 0x14, 
       0x9, 0x14, 0x4, 0x15, 0x9, 0x15, 0x4, 0x16, 0x9, 0x16, 0x4, 0x17, 
       0x9, 0x17, 0x4, 0x18, 0x9, 0x18, 0x4, 0x19, 0x9, 0x19, 0x4, 0x1a, 
       0x9, 0x1a, 0x4, 0x1b, 0x9, 0x1b, 0x4, 0x1c, 0x9, 0x1c, 0x4, 0x1d, 
       0x9, 0x1d, 0x4, 0x1e, 0x9, 0x1e, 0x4, 0x1f, 0x9, 0x1f, 0x4, 0x20, 
       0x9, 0x20, 0x4, 0x21, 0x9, 0x21, 0x4, 0x22, 0x9, 0x22, 0x4, 0x23, 
       0x9, 0x23, 0x4, 0x24, 0x9, 0x24, 0x4, 0x25, 0x9, 0x25, 0x4, 0x26, 
       0x9, 0x26, 0x4, 0x27, 0x9, 0x27, 0x4, 0x28, 0x9, 0x28, 0x4, 0x29, 
       0x9, 0x29, 0x4, 0x2a, 0x9, 0x2a, 0x4, 0x2b, 0x9, 0x2b, 0x4, 0x2c, 
       0x9, 0x2c, 0x4, 0x2d, 0x9, 0x2d, 0x4, 0x2e, 0x9, 0x2e, 0x4, 0x2f, 
       0x9, 0x2f, 0x4, 0x30, 0x9, 0x30, 0x4, 0x31, 0x9, 0x31, 0x4, 0x32, 
       0x9, 0x32, 0x4, 0x33, 0x9, 0x33, 0x4, 0x34, 0x9, 0x34, 0x4, 0x35, 
       0x9, 0x35, 0x4, 0x36, 0x9, 0x36, 0x4, 0x37, 0x9, 0x37, 0x4, 0x38, 
       0x9, 0x38, 0x4, 0x39, 0x9, 0x39, 0x4, 0x3a, 0x9, 0x3a, 0x4, 0x3b, 
       0x9, 0x3b, 0x4, 0x3c, 0x9, 0x3c, 0x4, 0x3d, 0x9, 0x3d, 0x4, 0x3e, 
       0x9, 0x3e, 0x4, 0x3f, 0x9, 0x3f, 0x4, 0x40, 0x9, 0x40, 0x4, 0x41, 
       0x9, 0x41, 0x4, 0x42, 0x9, 0x42, 0x4, 0x43, 0x9, 0x43, 0x4, 0x44, 
       0x9, 0x44, 0x4, 0x45, 0x9, 0x45, 0x4, 0x46, 0x9, 0x46, 0x4, 0x47, 
       0x9, 0x47, 0x4, 0x48, 0x9, 0x48, 0x4, 0x49, 0x9, 0x49, 0x4, 0x4a, 
       0x9, 0x4a, 0x4, 0x4b, 0x9, 0x4b, 0x4, 0x4c, 0x9, 0x4c, 0x4, 0x4d, 
       0x9, 0x4d, 0x4, 0x4e, 0x9, 0x4e, 0x4, 0x4f, 0x9, 0x4f, 0x4, 0x50, 
       0x9, 0x50, 0x4, 0x51, 0x9, 0x51, 0x4, 0x52, 0x9, 0x52, 0x4, 0x53, 
       0x9, 0x53, 0x4, 0x54, 0x9, 0x54, 0x4, 0x55, 0x9, 0x55, 0x4, 0x56, 
       0x9, 0x56, 0x4, 0x57, 0x9, 0x57, 0x4, 0x58, 0x9, 0x58, 0x4, 0x59, 
       0x9, 0x59, 0x4, 0x5a, 0x9, 0x5a, 0x4, 0x5b, 0x9, 0x5b, 0x4, 0x5c, 
       0x9, 0x5c, 0x4, 0x5d, 0x9, 0x5d, 0x4, 0x5e, 0x9, 0x5e, 0x4, 0x5f, 
       0x9, 0x5f, 0x4, 0x60, 0x9, 0x60, 0x4, 0x61, 0x9, 0x61, 0x4, 0x62, 
       0x9, 0x62, 0x4, 0x63, 0x9, 0x63, 0x4, 0x64, 0x9, 0x64, 0x4, 0x65, 
       0x9, 0x65, 0x3, 0x2, 0x5, 0x2, 0xcc, 0xa, 0x2, 0x3, 0x2, 0x3, 0x2, 
       0x5, 0x2, 0xd0, 0xa, 0x2, 0x3, 0x2, 0x5, 0x2, 0xd3, 0xa, 0x2, 0x3, 
       0x2, 0x5, 0x2, 0xd6, 0xa, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x3, 0x3, 
       0x3, 0x3, 0x3, 0x5, 0x3, 0xdd, 0xa, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 
       0x3, 0x5, 0x3, 0xe2, 0xa, 0x3, 0x3, 0x3, 0x5, 0x3, 0xe5, 0xa, 0x3, 
       0x3, 0x4, 0x3, 0x4, 0x5, 0x4, 0xe9, 0xa, 0x4, 0x3, 0x5, 0x3, 0x5, 
       0x5, 0x5, 0xed, 0xa, 0x5, 0x3, 0x5, 0x7, 0x5, 0xf0, 0xa, 0x5, 0xc, 
       0x5, 0xe, 0x5, 0xf3, 0xb, 0x5, 0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 0x3, 
       0x6, 0x5, 0x6, 0xf9, 0xa, 0x6, 0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 0x5, 
       0x6, 0xfe, 0xa, 0x6, 0x3, 0x6, 0x5, 0x6, 0x101, 0xa, 0x6, 0x3, 0x7, 
       0x3, 0x7, 0x5, 0x7, 0x105, 0xa, 0x7, 0x3, 0x8, 0x3, 0x8, 0x5, 0x8, 
       0x109, 0xa, 0x8, 0x7, 0x8, 0x10b, 0xa, 0x8, 0xc, 0x8, 0xe, 0x8, 0x10e, 
       0xb, 0x8, 0x3, 0x8, 0x3, 0x8, 0x3, 0x8, 0x5, 0x8, 0x113, 0xa, 0x8, 
       0x7, 0x8, 0x115, 0xa, 0x8, 0xc, 0x8, 0xe, 0x8, 0x118, 0xb, 0x8, 0x3, 
       0x8, 0x3, 0x8, 0x5, 0x8, 0x11c, 0xa, 0x8, 0x3, 0x8, 0x7, 0x8, 0x11f, 
       0xa, 0x8, 0xc, 0x8, 0xe, 0x8, 0x122, 0xb, 0x8, 0x3, 0x8, 0x5, 0x8, 
       0x125, 0xa, 0x8, 0x3, 0x8, 0x5, 0x8, 0x128, 0xa, 0x8, 0x5, 0x8, 0x12a, 
       0xa, 0x8, 0x3, 0x9, 0x3, 0x9, 0x5, 0x9, 0x12e, 0xa, 0x9, 0x7, 0x9, 
       0x130, 0xa, 0x9, 0xc, 0x9, 0xe, 0x9, 0x133, 0xb, 0x9, 0x3, 0x9, 0x3, 
       0x9, 0x5, 0x9, 0x137, 0xa, 0x9, 0x7, 0x9, 0x139, 0xa, 0x9, 0xc, 0x9, 
       0xe, 0x9, 0x13c, 0xb, 0x9, 0x3, 0x9, 0x3, 0x9, 0x5, 0x9, 0x140, 0xa, 
       0x9, 0x6, 0x9, 0x142, 0xa, 0x9, 0xd, 0x9, 0xe, 0x9, 0x143, 0x3, 0x9, 
       0x3, 0x9, 0x3, 0xa, 0x3, 0xa, 0x3, 0xa, 0x3, 0xa, 0x3, 0xa, 0x5, 
       0xa, 0x14d, 0xa, 0xa, 0x3, 0xb, 0x3, 0xb, 0x3, 0xb, 0x5, 0xb, 0x152, 
       0xa, 0xb, 0x3, 0xc, 0x3, 0xc, 0x5, 0xc, 0x156, 0xa, 0xc, 0x3, 0xc, 
       0x3, 0xc, 0x5, 0xc, 0x15a, 0xa, 0xc, 0x3, 0xc, 0x3, 0xc, 0x5, 0xc, 
       0x15e, 0xa, 0xc, 0x3, 0xc, 0x7, 0xc, 0x161, 0xa, 0xc, 0xc, 0xc, 0xe, 
       0xc, 0x164, 0xb, 0xc, 0x3, 0xc, 0x5, 0xc, 0x167, 0xa, 0xc, 0x3, 0xc, 
       0x5, 0xc, 0x16a, 0xa, 0xc, 0x3, 0xd, 0x3, 0xd, 0x5, 0xd, 0x16e, 0xa, 
       0xd, 0x3, 0xd, 0x3, 0xd, 0x3, 0xd, 0x3, 0xd, 0x3, 0xd, 0x3, 0xd, 
       0x3, 0xe, 0x3, 0xe, 0x5, 0xe, 0x178, 0xa, 0xe, 0x3, 0xe, 0x3, 0xe, 
       0x3, 0xe, 0x7, 0xe, 0x17d, 0xa, 0xe, 0xc, 0xe, 0xe, 0xe, 0x180, 0xb, 
       0xe, 0x3, 0xf, 0x3, 0xf, 0x3, 0xf, 0x3, 0xf, 0x3, 0xf, 0x3, 0xf, 
       0x3, 0xf, 0x3, 0xf, 0x3, 0xf, 0x3, 0xf, 0x5, 0xf, 0x18c, 0xa, 0xf, 
       0x3, 0x10, 0x3, 0x10, 0x5, 0x10, 0x190, 0xa, 0x10, 0x3, 0x10, 0x3, 
       0x10, 0x3, 0x11, 0x3, 0x11, 0x5, 0x11, 0x196, 0xa, 0x11, 0x3, 0x11, 
       0x3, 0x11, 0x3, 0x11, 0x7, 0x11, 0x19b, 0xa, 0x11, 0xc, 0x11, 0xe, 
       0x11, 0x19e, 0xb, 0x11, 0x3, 0x12, 0x3, 0x12, 0x5, 0x12, 0x1a2, 0xa, 
       0x12, 0x3, 0x12, 0x3, 0x12, 0x5, 0x12, 0x1a6, 0xa, 0x12, 0x3, 0x12, 
       0x3, 0x12, 0x3, 0x12, 0x3, 0x12, 0x5, 0x12, 0x1ac, 0xa, 0x12, 0x3, 
       0x12, 0x3, 0x12, 0x5, 0x12, 0x1b0, 0xa, 0x12, 0x3, 0x12, 0x3, 0x12, 
       0x3, 0x12, 0x3, 0x12, 0x5, 0x12, 0x1b6, 0xa, 0x12, 0x3, 0x12, 0x3, 
       0x12, 0x5, 0x12, 0x1ba, 0xa, 0x12, 0x3, 0x12, 0x3, 0x12, 0x3, 0x12, 
       0x3, 0x12, 0x5, 0x12, 0x1c0, 0xa, 0x12, 0x3, 0x12, 0x3, 0x12, 0x5, 
       0x12, 0x1c4, 0xa, 0x12, 0x3, 0x13, 0x3, 0x13, 0x5, 0x13, 0x1c8, 0xa, 
       0x13, 0x3, 0x13, 0x3, 0x13, 0x5, 0x13, 0x1cc, 0xa, 0x13, 0x3, 0x13, 
       0x3, 0x13, 0x5, 0x13, 0x1d0, 0xa, 0x13, 0x3, 0x13, 0x3, 0x13, 0x5, 
       0x13, 0x1d4, 0xa, 0x13, 0x3, 0x13, 0x7, 0x13, 0x1d7, 0xa, 0x13, 0xc, 
       0x13, 0xe, 0x13, 0x1da, 0xb, 0x13, 0x3, 0x14, 0x3, 0x14, 0x3, 0x14, 
       0x3, 0x14, 0x5, 0x14, 0x1e0, 0xa, 0x14, 0x3, 0x14, 0x3, 0x14, 0x5, 
       0x14, 0x1e4, 0xa, 0x14, 0x3, 0x14, 0x7, 0x14, 0x1e7, 0xa, 0x14, 0xc, 
       0x14, 0xe, 0x14, 0x1ea, 0xb, 0x14, 0x3, 0x15, 0x3, 0x15, 0x3, 0x15, 
       0x3, 0x15, 0x5, 0x15, 0x1f0, 0xa, 0x15, 0x3, 0x16, 0x3, 0x16, 0x3, 
       0x16, 0x3, 0x16, 0x5, 0x16, 0x1f6, 0xa, 0x16, 0x3, 0x16, 0x3, 0x16, 
       0x3, 0x16, 0x5, 0x16, 0x1fb, 0xa, 0x16, 0x3, 0x17, 0x3, 0x17, 0x3, 
       0x17, 0x3, 0x17, 0x5, 0x17, 0x201, 0xa, 0x17, 0x3, 0x17, 0x3, 0x17, 
       0x3, 0x17, 0x3, 0x17, 0x5, 0x17, 0x207, 0xa, 0x17, 0x3, 0x18, 0x3, 
       0x18, 0x3, 0x18, 0x5, 0x18, 0x20c, 0xa, 0x18, 0x3, 0x18, 0x3, 0x18, 
       0x5, 0x18, 0x210, 0xa, 0x18, 0x3, 0x18, 0x7, 0x18, 0x213, 0xa, 0x18, 
       0xc, 0x18, 0xe, 0x18, 0x216, 0xb, 0x18, 0x5, 0x18, 0x218, 0xa, 0x18, 
       0x3, 0x18, 0x5, 0x18, 0x21b, 0xa, 0x18, 0x3, 0x18, 0x5, 0x18, 0x21e, 
       0xa, 0x18, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 
       0x5, 0x19, 0x225, 0xa, 0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 0x1a, 0x3, 
       0x1a, 0x5, 0x1a, 0x22b, 0xa, 0x1a, 0x3, 0x1a, 0x5, 0x1a, 0x22e, 0xa, 
       0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x5, 0x1a, 0x233, 0xa, 0x1a, 
       0x3, 0x1a, 0x5, 0x1a, 0x236, 0xa, 0x1a, 0x3, 0x1b, 0x3, 0x1b, 0x5, 
       0x1b, 0x23a, 0xa, 0x1b, 0x3, 0x1b, 0x5, 0x1b, 0x23d, 0xa, 0x1b, 0x3, 
       0x1b, 0x3, 0x1b, 0x3, 0x1b, 0x3, 0x1c, 0x3, 0x1c, 0x3, 0x1c, 0x5, 
       0x1c, 0x245, 0xa, 0x1c, 0x3, 0x1c, 0x3, 0x1c, 0x5, 0x1c, 0x249, 0xa, 
       0x1c, 0x3, 0x1c, 0x3, 0x1c, 0x5, 0x1c, 0x24d, 0xa, 0x1c, 0x3, 0x1d, 
       0x3, 0x1d, 0x5, 0x1d, 0x251, 0xa, 0x1d, 0x3, 0x1d, 0x3, 0x1d, 0x5, 
       0x1d, 0x255, 0xa, 0x1d, 0x3, 0x1d, 0x7, 0x1d, 0x258, 0xa, 0x1d, 0xc, 
       0x1d, 0xe, 0x1d, 0x25b, 0xb, 0x1d, 0x3, 0x1d, 0x3, 0x1d, 0x5, 0x1d, 
       0x25f, 0xa, 0x1d, 0x3, 0x1d, 0x3, 0x1d, 0x5, 0x1d, 0x263, 0xa, 0x1d, 
       0x3, 0x1d, 0x7, 0x1d, 0x266, 0xa, 0x1d, 0xc, 0x1d, 0xe, 0x1d, 0x269, 
       0xb, 0x1d, 0x5, 0x1d, 0x26b, 0xa, 0x1d, 0x3, 0x1e, 0x3, 0x1e, 0x3, 
       0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x5, 0x1e, 0x274, 
       0xa, 0x1e, 0x3, 0x1f, 0x3, 0x1f, 0x3, 0x1f, 0x3, 0x1f, 0x3, 0x1f, 
       0x3, 0x1f, 0x3, 0x1f, 0x5, 0x1f, 0x27d, 0xa, 0x1f, 0x3, 0x1f, 0x7, 
       0x1f, 0x280, 0xa, 0x1f, 0xc, 0x1f, 0xe, 0x1f, 0x283, 0xb, 0x1f, 0x3, 
       0x20, 0x3, 0x20, 0x3, 0x20, 0x3, 0x20, 0x3, 0x21, 0x3, 0x21, 0x3, 
       0x21, 0x3, 0x21, 0x3, 0x22, 0x3, 0x22, 0x5, 0x22, 0x28f, 0xa, 0x22, 
       0x3, 0x22, 0x5, 0x22, 0x292, 0xa, 0x22, 0x3, 0x23, 0x3, 0x23, 0x3, 
       0x23, 0x3, 0x23, 0x3, 0x23, 0x3, 0x23, 0x3, 0x23, 0x3, 0x23, 0x3, 
       0x23, 0x3, 0x23, 0x3, 0x23, 0x3, 0x23, 0x3, 0x23, 0x3, 0x23, 0x5, 
       0x23, 0x2a2, 0xa, 0x23, 0x3, 0x24, 0x3, 0x24, 0x3, 0x24, 0x3, 0x24, 
       0x3, 0x25, 0x3, 0x25, 0x5, 0x25, 0x2aa, 0xa, 0x25, 0x3, 0x25, 0x3, 
       0x25, 0x5, 0x25, 0x2ae, 0xa, 0x25, 0x3, 0x25, 0x7, 0x25, 0x2b1, 0xa, 
       0x25, 0xc, 0x25, 0xe, 0x25, 0x2b4, 0xb, 0x25, 0x3, 0x26, 0x3, 0x26, 
       0x5, 0x26, 0x2b8, 0xa, 0x26, 0x3, 0x26, 0x3, 0x26, 0x5, 0x26, 0x2bc, 
       0xa, 0x26, 0x3, 0x26, 0x3, 0x26, 0x3, 0x26, 0x5, 0x26, 0x2c1, 0xa, 
       0x26, 0x3, 0x27, 0x3, 0x27, 0x3, 0x28, 0x3, 0x28, 0x5, 0x28, 0x2c7, 
       0xa, 0x28, 0x3, 0x28, 0x7, 0x28, 0x2ca, 0xa, 0x28, 0xc, 0x28, 0xe, 
       0x28, 0x2cd, 0xb, 0x28, 0x3, 0x28, 0x3, 0x28, 0x3, 0x28, 0x3, 0x28, 
       0x5, 0x28, 0x2d3, 0xa, 0x28, 0x3, 0x29, 0x3, 0x29, 0x5, 0x29, 0x2d7, 
       0xa, 0x29, 0x3, 0x29, 0x3, 0x29, 0x5, 0x29, 0x2db, 0xa, 0x29, 0x5, 
       0x29, 0x2dd, 0xa, 0x29, 0x3, 0x29, 0x3, 0x29, 0x5, 0x29, 0x2e1, 0xa, 
       0x29, 0x5, 0x29, 0x2e3, 0xa, 0x29, 0x3, 0x29, 0x3, 0x29, 0x5, 0x29, 
       0x2e7, 0xa, 0x29, 0x5, 0x29, 0x2e9, 0xa, 0x29, 0x3, 0x29, 0x3, 0x29, 
       0x3, 0x2a, 0x3, 0x2a, 0x5, 0x2a, 0x2ef, 0xa, 0x2a, 0x3, 0x2a, 0x3, 
       0x2a, 0x3, 0x2b, 0x3, 0x2b, 0x5, 0x2b, 0x2f5, 0xa, 0x2b, 0x3, 0x2b, 
       0x3, 0x2b, 0x5, 0x2b, 0x2f9, 0xa, 0x2b, 0x3, 0x2b, 0x5, 0x2b, 0x2fc, 
       0xa, 0x2b, 0x3, 0x2b, 0x5, 0x2b, 0x2ff, 0xa, 0x2b, 0x3, 0x2b, 0x3, 
       0x2b, 0x5, 0x2b, 0x303, 0xa, 0x2b, 0x3, 0x2b, 0x3, 0x2b, 0x3, 0x2b, 
       0x3, 0x2b, 0x5, 0x2b, 0x309, 0xa, 0x2b, 0x3, 0x2b, 0x3, 0x2b, 0x5, 
       0x2b, 0x30d, 0xa, 0x2b, 0x3, 0x2b, 0x5, 0x2b, 0x310, 0xa, 0x2b, 0x3, 
       0x2b, 0x5, 0x2b, 0x313, 0xa, 0x2b, 0x3, 0x2b, 0x3, 0x2b, 0x3, 0x2b, 
       0x3, 0x2b, 0x5, 0x2b, 0x319, 0xa, 0x2b, 0x3, 0x2b, 0x5, 0x2b, 0x31c, 
       0xa, 0x2b, 0x3, 0x2b, 0x5, 0x2b, 0x31f, 0xa, 0x2b, 0x3, 0x2b, 0x3, 
       0x2b, 0x5, 0x2b, 0x323, 0xa, 0x2b, 0x3, 0x2b, 0x3, 0x2b, 0x3, 0x2b, 
       0x3, 0x2b, 0x5, 0x2b, 0x329, 0xa, 0x2b, 0x3, 0x2b, 0x5, 0x2b, 0x32c, 
       0xa, 0x2b, 0x3, 0x2b, 0x5, 0x2b, 0x32f, 0xa, 0x2b, 0x3, 0x2b, 0x3, 
       0x2b, 0x5, 0x2b, 0x333, 0xa, 0x2b, 0x3, 0x2c, 0x3, 0x2c, 0x5, 0x2c, 
       0x337, 0xa, 0x2c, 0x3, 0x2c, 0x3, 0x2c, 0x5, 0x2c, 0x33b, 0xa, 0x2c, 
       0x5, 0x2c, 0x33d, 0xa, 0x2c, 0x3, 0x2c, 0x3, 0x2c, 0x5, 0x2c, 0x341, 
       0xa, 0x2c, 0x5, 0x2c, 0x343, 0xa, 0x2c, 0x3, 0x2c, 0x5, 0x2c, 0x346, 
       0xa, 0x2c, 0x3, 0x2c, 0x3, 0x2c, 0x5, 0x2c, 0x34a, 0xa, 0x2c, 0x5, 
       0x2c, 0x34c, 0xa, 0x2c, 0x3, 0x2c, 0x3, 0x2c, 0x3, 0x2d, 0x3, 0x2d, 
       0x5, 0x2d, 0x352, 0xa, 0x2d, 0x3, 0x2e, 0x3, 0x2e, 0x5, 0x2e, 0x356, 
       0xa, 0x2e, 0x3, 0x2e, 0x3, 0x2e, 0x5, 0x2e, 0x35a, 0xa, 0x2e, 0x3, 
       0x2e, 0x3, 0x2e, 0x5, 0x2e, 0x35e, 0xa, 0x2e, 0x3, 0x2e, 0x5, 0x2e, 
       0x361, 0xa, 0x2e, 0x3, 0x2e, 0x7, 0x2e, 0x364, 0xa, 0x2e, 0xc, 0x2e, 
       0xe, 0x2e, 0x367, 0xb, 0x2e, 0x3, 0x2f, 0x3, 0x2f, 0x5, 0x2f, 0x36b, 
       0xa, 0x2f, 0x3, 0x2f, 0x7, 0x2f, 0x36e, 0xa, 0x2f, 0xc, 0x2f, 0xe, 
       0x2f, 0x371, 0xb, 0x2f, 0x3, 0x30, 0x3, 0x30, 0x5, 0x30, 0x375, 0xa, 
       0x30, 0x3, 0x30, 0x3, 0x30, 0x3, 0x31, 0x3, 0x31, 0x5, 0x31, 0x37b, 
       0xa, 0x31, 0x3, 0x31, 0x3, 0x31, 0x5, 0x31, 0x37f, 0xa, 0x31, 0x5, 
       0x31, 0x381, 0xa, 0x31, 0x3, 0x31, 0x3, 0x31, 0x5, 0x31, 0x385, 0xa, 
       0x31, 0x3, 0x31, 0x3, 0x31, 0x5, 0x31, 0x389, 0xa, 0x31, 0x5, 0x31, 
       0x38b, 0xa, 0x31, 0x5, 0x31, 0x38d, 0xa, 0x31, 0x3, 0x32, 0x3, 0x32, 
       0x3, 0x33, 0x3, 0x33, 0x3, 0x34, 0x3, 0x34, 0x3, 0x35, 0x3, 0x35, 
       0x3, 0x35, 0x3, 0x35, 0x3, 0x35, 0x7, 0x35, 0x39a, 0xa, 0x35, 0xc, 
       0x35, 0xe, 0x35, 0x39d, 0xb, 0x35, 0x3, 0x36, 0x3, 0x36, 0x3, 0x36, 
       0x3, 0x36, 0x3, 0x36, 0x7, 0x36, 0x3a4, 0xa, 0x36, 0xc, 0x36, 0xe, 
       0x36, 0x3a7, 0xb, 0x36, 0x3, 0x37, 0x3, 0x37, 0x3, 0x37, 0x3, 0x37, 
       0x3, 0x37, 0x7, 0x37, 0x3ae, 0xa, 0x37, 0xc, 0x37, 0xe, 0x37, 0x3b1, 
       0xb, 0x37, 0x3, 0x38, 0x3, 0x38, 0x5, 0x38, 0x3b5, 0xa, 0x38, 0x7, 
       0x38, 0x3b7, 0xa, 0x38, 0xc, 0x38, 0xe, 0x38, 0x3ba, 0xb, 0x38, 0x3, 
       0x38, 0x3, 0x38, 0x3, 0x39, 0x3, 0x39, 0x5, 0x39, 0x3c0, 0xa, 0x39, 
       0x3, 0x39, 0x7, 0x39, 0x3c3, 0xa, 0x39, 0xc, 0x39, 0xe, 0x39, 0x3c6, 
       0xb, 0x39, 0x3, 0x3a, 0x3, 0x3a, 0x5, 0x3a, 0x3ca, 0xa, 0x3a, 0x3, 
       0x3a, 0x3, 0x3a, 0x5, 0x3a, 0x3ce, 0xa, 0x3a, 0x3, 0x3a, 0x3, 0x3a, 
       0x5, 0x3a, 0x3d2, 0xa, 0x3a, 0x3, 0x3a, 0x3, 0x3a, 0x5, 0x3a, 0x3d6, 
       0xa, 0x3a, 0x3, 0x3a, 0x7, 0x3a, 0x3d9, 0xa, 0x3a, 0xc, 0x3a, 0xe, 
       0x3a, 0x3dc, 0xb, 0x3a, 0x3, 0x3b, 0x3, 0x3b, 0x5, 0x3b, 0x3e0, 0xa, 
       0x3b, 0x3, 0x3b, 0x3, 0x3b, 0x5, 0x3b, 0x3e4, 0xa, 0x3b, 0x3, 0x3b, 
       0x3, 0x3b, 0x5, 0x3b, 0x3e8, 0xa, 0x3b, 0x3, 0x3b, 0x3, 0x3b, 0x5, 
       0x3b, 0x3ec, 0xa, 0x3b, 0x3, 0x3b, 0x3, 0x3b, 0x5, 0x3b, 0x3f0, 0xa, 
       0x3b, 0x3, 0x3b, 0x3, 0x3b, 0x5, 0x3b, 0x3f4, 0xa, 0x3b, 0x3, 0x3b, 
       0x7, 0x3b, 0x3f7, 0xa, 0x3b, 0xc, 0x3b, 0xe, 0x3b, 0x3fa, 0xb, 0x3b, 
       0x3, 0x3c, 0x3, 0x3c, 0x5, 0x3c, 0x3fe, 0xa, 0x3c, 0x3, 0x3c, 0x3, 
       0x3c, 0x5, 0x3c, 0x402, 0xa, 0x3c, 0x3, 0x3c, 0x7, 0x3c, 0x405, 0xa, 
       0x3c, 0xc, 0x3c, 0xe, 0x3c, 0x408, 0xb, 0x3c, 0x3, 0x3d, 0x3, 0x3d, 
       0x5, 0x3d, 0x40c, 0xa, 0x3d, 0x7, 0x3d, 0x40e, 0xa, 0x3d, 0xc, 0x3d, 
       0xe, 0x3d, 0x411, 0xb, 0x3d, 0x3, 0x3d, 0x3, 0x3d, 0x3, 0x3e, 0x3, 
       0x3e, 0x3, 0x3e, 0x3, 0x3e, 0x7, 0x3e, 0x419, 0xa, 0x3e, 0xc, 0x3e, 
       0xe, 0x3e, 0x41c, 0xb, 0x3e, 0x3, 0x3f, 0x3, 0x3f, 0x3, 0x3f, 0x5, 
       0x3f, 0x421, 0xa, 0x3f, 0x3, 0x3f, 0x3, 0x3f, 0x5, 0x3f, 0x425, 0xa, 
       0x3f, 0x3, 0x3f, 0x3, 0x3f, 0x3, 0x3f, 0x3, 0x3f, 0x3, 0x3f, 0x5, 
       0x3f, 0x42c, 0xa, 0x3f, 0x3, 0x3f, 0x3, 0x3f, 0x5, 0x3f, 0x430, 0xa, 
       0x3f, 0x3, 0x3f, 0x3, 0x3f, 0x5, 0x3f, 0x434, 0xa, 0x3f, 0x3, 0x3f, 
       0x5, 0x3f, 0x437, 0xa, 0x3f, 0x3, 0x40, 0x3, 0x40, 0x3, 0x40, 0x3, 
       0x40, 0x3, 0x40, 0x3, 0x40, 0x3, 0x40, 0x3, 0x40, 0x3, 0x40, 0x3, 
       0x40, 0x3, 0x40, 0x3, 0x40, 0x5, 0x40, 0x445, 0xa, 0x40, 0x3, 0x40, 
       0x5, 0x40, 0x448, 0xa, 0x40, 0x3, 0x40, 0x3, 0x40, 0x3, 0x41, 0x3, 
       0x41, 0x3, 0x41, 0x3, 0x41, 0x3, 0x41, 0x3, 0x41, 0x3, 0x41, 0x3, 
       0x41, 0x3, 0x41, 0x3, 0x41, 0x5, 0x41, 0x456, 0xa, 0x41, 0x3, 0x42, 
       0x3, 0x42, 0x5, 0x42, 0x45a, 0xa, 0x42, 0x3, 0x42, 0x7, 0x42, 0x45d, 
       0xa, 0x42, 0xc, 0x42, 0xe, 0x42, 0x460, 0xb, 0x42, 0x3, 0x42, 0x5, 
       0x42, 0x463, 0xa, 0x42, 0x3, 0x42, 0x5, 0x42, 0x466, 0xa, 0x42, 0x3, 
       0x43, 0x3, 0x43, 0x3, 0x43, 0x3, 0x43, 0x3, 0x43, 0x5, 0x43, 0x46d, 
       0xa, 0x43, 0x3, 0x43, 0x3, 0x43, 0x5, 0x43, 0x471, 0xa, 0x43, 0x3, 
       0x43, 0x3, 0x43, 0x5, 0x43, 0x475, 0xa, 0x43, 0x3, 0x43, 0x3, 0x43, 
       0x3, 0x43, 0x3, 0x43, 0x3, 0x43, 0x5, 0x43, 0x47c, 0xa, 0x43, 0x3, 
       0x43, 0x3, 0x43, 0x5, 0x43, 0x480, 0xa, 0x43, 0x3, 0x43, 0x3, 0x43, 
       0x5, 0x43, 0x484, 0xa, 0x43, 0x3, 0x43, 0x3, 0x43, 0x3, 0x43, 0x3, 
       0x43, 0x5, 0x43, 0x48a, 0xa, 0x43, 0x3, 0x43, 0x3, 0x43, 0x5, 0x43, 
       0x48e, 0xa, 0x43, 0x3, 0x43, 0x3, 0x43, 0x5, 0x43, 0x492, 0xa, 0x43, 
       0x3, 0x43, 0x3, 0x43, 0x3, 0x43, 0x3, 0x43, 0x5, 0x43, 0x498, 0xa, 
       0x43, 0x3, 0x43, 0x3, 0x43, 0x5, 0x43, 0x49c, 0xa, 0x43, 0x3, 0x43, 
       0x3, 0x43, 0x5, 0x43, 0x4a0, 0xa, 0x43, 0x3, 0x43, 0x3, 0x43, 0x3, 
       0x43, 0x3, 0x43, 0x5, 0x43, 0x4a6, 0xa, 0x43, 0x3, 0x43, 0x3, 0x43, 
       0x5, 0x43, 0x4aa, 0xa, 0x43, 0x3, 0x43, 0x3, 0x43, 0x5, 0x43, 0x4ae, 
       0xa, 0x43, 0x3, 0x43, 0x3, 0x43, 0x3, 0x43, 0x3, 0x43, 0x3, 0x43, 
       0x3, 0x43, 0x5, 0x43, 0x4b6, 0xa, 0x43, 0x3, 0x44, 0x3, 0x44, 0x3, 
       0x44, 0x3, 0x44, 0x3, 0x44, 0x3, 0x44, 0x5, 0x44, 0x4be, 0xa, 0x44, 
       0x3, 0x45, 0x3, 0x45, 0x3, 0x46, 0x3, 0x46, 0x5, 0x46, 0x4c4, 0xa, 
       0x46, 0x3, 0x46, 0x3, 0x46, 0x5, 0x46, 0x4c8, 0xa, 0x46, 0x3, 0x46, 
       0x3, 0x46, 0x5, 0x46, 0x4cc, 0xa, 0x46, 0x3, 0x46, 0x3, 0x46, 0x5, 
       0x46, 0x4d0, 0xa, 0x46, 0x7, 0x46, 0x4d2, 0xa, 0x46, 0xc, 0x46, 0xe, 
       0x46, 0x4d5, 0xb, 0x46, 0x5, 0x46, 0x4d7, 0xa, 0x46, 0x3, 0x46, 0x3, 
       0x46, 0x3, 0x47, 0x3, 0x47, 0x5, 0x47, 0x4dd, 0xa, 0x47, 0x3, 0x47, 
       0x3, 0x47, 0x3, 0x47, 0x5, 0x47, 0x4e2, 0xa, 0x47, 0x3, 0x47, 0x3, 
       0x47, 0x3, 0x47, 0x5, 0x47, 0x4e7, 0xa, 0x47, 0x3, 0x47, 0x3, 0x47, 
       0x3, 0x47, 0x5, 0x47, 0x4ec, 0xa, 0x47, 0x3, 0x47, 0x3, 0x47, 0x3, 
       0x47, 0x5, 0x47, 0x4f1, 0xa, 0x47, 0x3, 0x47, 0x3, 0x47, 0x3, 0x47, 
       0x5, 0x47, 0x4f6, 0xa, 0x47, 0x3, 0x47, 0x5, 0x47, 0x4f9, 0xa, 0x47, 
       0x3, 0x48, 0x3, 0x48, 0x5, 0x48, 0x4fd, 0xa, 0x48, 0x3, 0x48, 0x3, 
       0x48, 0x5, 0x48, 0x501, 0xa, 0x48, 0x3, 0x48, 0x3, 0x48, 0x3, 0x49, 
       0x3, 0x49, 0x5, 0x49, 0x507, 0xa, 0x49, 0x3, 0x49, 0x6, 0x49, 0x50a, 
       0xa, 0x49, 0xd, 0x49, 0xe, 0x49, 0x50b, 0x3, 0x4a, 0x3, 0x4a, 0x5, 
       0x4a, 0x510, 0xa, 0x4a, 0x3, 0x4a, 0x5, 0x4a, 0x513, 0xa, 0x4a, 0x3, 
       0x4b, 0x3, 0x4b, 0x3, 0x4b, 0x3, 0x4b, 0x3, 0x4b, 0x3, 0x4b, 0x3, 
       0x4c, 0x3, 0x4c, 0x5, 0x4c, 0x51d, 0xa, 0x4c, 0x3, 0x4c, 0x3, 0x4c, 
       0x5, 0x4c, 0x521, 0xa, 0x4c, 0x3, 0x4c, 0x3, 0x4c, 0x5, 0x4c, 0x525, 
       0xa, 0x4c, 0x5, 0x4c, 0x527, 0xa, 0x4c, 0x3, 0x4c, 0x3, 0x4c, 0x5, 
       0x4c, 0x52b, 0xa, 0x4c, 0x3, 0x4c, 0x3, 0x4c, 0x5, 0x4c, 0x52f, 0xa, 
       0x4c, 0x3, 0x4c, 0x3, 0x4c, 0x5, 0x4c, 0x533, 0xa, 0x4c, 0x7, 0x4c, 
       0x535, 0xa, 0x4c, 0xc, 0x4c, 0xe, 0x4c, 0x538, 0xb, 0x4c, 0x5, 0x4c, 
       0x53a, 0xa, 0x4c, 0x3, 0x4c, 0x3, 0x4c, 0x3, 0x4d, 0x3, 0x4d, 0x3, 
       0x4d, 0x3, 0x4d, 0x5, 0x4d, 0x542, 0xa, 0x4d, 0x3, 0x4e, 0x3, 0x4e, 
       0x5, 0x4e, 0x546, 0xa, 0x4e, 0x3, 0x4e, 0x3, 0x4e, 0x5, 0x4e, 0x54a, 
       0xa, 0x4e, 0x3, 0x4e, 0x3, 0x4e, 0x5, 0x4e, 0x54e, 0xa, 0x4e, 0x3, 
       0x4e, 0x3, 0x4e, 0x5, 0x4e, 0x552, 0xa, 0x4e, 0x3, 0x4e, 0x3, 0x4e, 
       0x5, 0x4e, 0x556, 0xa, 0x4e, 0x7, 0x4e, 0x558, 0xa, 0x4e, 0xc, 0x4e, 
       0xe, 0x4e, 0x55b, 0xb, 0x4e, 0x5, 0x4e, 0x55d, 0xa, 0x4e, 0x3, 0x4e, 
       0x3, 0x4e, 0x3, 0x4f, 0x3, 0x4f, 0x3, 0x50, 0x3, 0x50, 0x3, 0x51, 
       0x3, 0x51, 0x3, 0x51, 0x3, 0x52, 0x3, 0x52, 0x3, 0x52, 0x7, 0x52, 
       0x56b, 0xa, 0x52, 0xc, 0x52, 0xe, 0x52, 0x56e, 0xb, 0x52, 0x3, 0x53, 
       0x3, 0x53, 0x5, 0x53, 0x572, 0xa, 0x53, 0x3, 0x53, 0x3, 0x53, 0x5, 
       0x53, 0x576, 0xa, 0x53, 0x3, 0x53, 0x3, 0x53, 0x5, 0x53, 0x57a, 0xa, 
       0x53, 0x3, 0x53, 0x5, 0x53, 0x57d, 0xa, 0x53, 0x3, 0x53, 0x5, 0x53, 
       0x580, 0xa, 0x53, 0x3, 0x53, 0x3, 0x53, 0x3, 0x54, 0x3, 0x54, 0x5, 
       0x54, 0x586, 0xa, 0x54, 0x3, 0x54, 0x3, 0x54, 0x5, 0x54, 0x58a, 0xa, 
       0x54, 0x3, 0x54, 0x3, 0x54, 0x5, 0x54, 0x58e, 0xa, 0x54, 0x5, 0x54, 
       0x590, 0xa, 0x54, 0x3, 0x54, 0x3, 0x54, 0x5, 0x54, 0x594, 0xa, 0x54, 
       0x3, 0x54, 0x3, 0x54, 0x5, 0x54, 0x598, 0xa, 0x54, 0x3, 0x54, 0x3, 
       0x54, 0x5, 0x54, 0x59c, 0xa, 0x54, 0x5, 0x54, 0x59e, 0xa, 0x54, 0x3, 
       0x54, 0x3, 0x54, 0x5, 0x54, 0x5a2, 0xa, 0x54, 0x3, 0x54, 0x3, 0x54, 
       0x5, 0x54, 0x5a6, 0xa, 0x54, 0x3, 0x54, 0x3, 0x54, 0x3, 0x55, 0x3, 
       0x55, 0x5, 0x55, 0x5ac, 0xa, 0x55, 0x3, 0x55, 0x3, 0x55, 0x3, 0x56, 
       0x3, 0x56, 0x5, 0x56, 0x5b2, 0xa, 0x56, 0x3, 0x56, 0x6, 0x56, 0x5b5, 
       0xa, 0x56, 0xd, 0x56, 0xe, 0x56, 0x5b6, 0x3, 0x56, 0x3, 0x56, 0x5, 
       0x56, 0x5bb, 0xa, 0x56, 0x3, 0x56, 0x3, 0x56, 0x5, 0x56, 0x5bf, 0xa, 
       0x56, 0x3, 0x56, 0x6, 0x56, 0x5c2, 0xa, 0x56, 0xd, 0x56, 0xe, 0x56, 
       0x5c3, 0x5, 0x56, 0x5c6, 0xa, 0x56, 0x3, 0x56, 0x5, 0x56, 0x5c9, 
       0xa, 0x56, 0x3, 0x56, 0x3, 0x56, 0x5, 0x56, 0x5cd, 0xa, 0x56, 0x3, 
       0x56, 0x5, 0x56, 0x5d0, 0xa, 0x56, 0x3, 0x56, 0x5, 0x56, 0x5d3, 0xa, 
       0x56, 0x3, 0x56, 0x3, 0x56, 0x3, 0x57, 0x3, 0x57, 0x5, 0x57, 0x5d9, 
       0xa, 0x57, 0x3, 0x57, 0x3, 0x57, 0x5, 0x57, 0x5dd, 0xa, 0x57, 0x3, 
       0x57, 0x3, 0x57, 0x5, 0x57, 0x5e1, 0xa, 0x57, 0x3, 0x57, 0x3, 0x57, 
       0x3, 0x58, 0x3, 0x58, 0x3, 0x59, 0x3, 0x59, 0x5, 0x59, 0x5e9, 0xa, 
       0x59, 0x3, 0x5a, 0x3, 0x5a, 0x5, 0x5a, 0x5ed, 0xa, 0x5a, 0x3, 0x5a, 
       0x3, 0x5a, 0x5, 0x5a, 0x5f1, 0xa, 0x5a, 0x3, 0x5a, 0x3, 0x5a, 0x5, 
       0x5a, 0x5f5, 0xa, 0x5a, 0x3, 0x5a, 0x3, 0x5a, 0x5, 0x5a, 0x5f9, 0xa, 
       0x5a, 0x3, 0x5a, 0x3, 0x5a, 0x5, 0x5a, 0x5fd, 0xa, 0x5a, 0x3, 0x5a, 
       0x3, 0x5a, 0x5, 0x5a, 0x601, 0xa, 0x5a, 0x3, 0x5a, 0x3, 0x5a, 0x5, 
       0x5a, 0x605, 0xa, 0x5a, 0x3, 0x5a, 0x3, 0x5a, 0x5, 0x5a, 0x609, 0xa, 
       0x5a, 0x7, 0x5a, 0x60b, 0xa, 0x5a, 0xc, 0x5a, 0xe, 0x5a, 0x60e, 0xb, 
       0x5a, 0x5, 0x5a, 0x610, 0xa, 0x5a, 0x3, 0x5a, 0x3, 0x5a, 0x3, 0x5b, 
       0x3, 0x5b, 0x3, 0x5b, 0x5, 0x5b, 0x617, 0xa, 0x5b, 0x3, 0x5c, 0x3, 
       0x5c, 0x5, 0x5c, 0x61b, 0xa, 0x5c, 0x3, 0x5c, 0x6, 0x5c, 0x61e, 0xa, 
       0x5c, 0xd, 0x5c, 0xe, 0x5c, 0x61f, 0x3, 0x5d, 0x3, 0x5d, 0x3, 0x5e, 
       0x3, 0x5e, 0x3, 0x5f, 0x3, 0x5f, 0x3, 0x60, 0x3, 0x60, 0x5, 0x60, 
       0x62a, 0xa, 0x60, 0x3, 0x61, 0x3, 0x61, 0x3, 0x62, 0x3, 0x62, 0x3, 
       0x63, 0x3, 0x63, 0x3, 0x64, 0x3, 0x64, 0x3, 0x65, 0x3, 0x65, 0x3, 
       0x65, 0x2, 0x2, 0x66, 0x2, 0x4, 0x6, 0x8, 0xa, 0xc, 0xe, 0x10, 0x12, 
       0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e, 0x20, 0x22, 0x24, 0x26, 0x28, 
       0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e, 
       0x40, 0x42, 0x44, 0x46, 0x48, 0x4a, 0x4c, 0x4e, 0x50, 0x52, 0x54, 
       0x56, 0x58, 0x5a, 0x5c, 0x5e, 0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 
       0x6c, 0x6e, 0x70, 0x72, 0x74, 0x76, 0x78, 0x7a, 0x7c, 0x7e, 0x80, 
       0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c, 0x8e, 0x90, 0x92, 0x94, 0x96, 
       0x98, 0x9a, 0x9c, 0x9e, 0xa0, 0xa2, 0xa4, 0xa6, 0xa8, 0xaa, 0xac, 
       0xae, 0xb0, 0xb2, 0xb4, 0xb6, 0xb8, 0xba, 0xbc, 0xbe, 0xc0, 0xc2, 
       0xc4, 0xc6, 0xc8, 0x2, 0xc, 0x3, 0x2, 0x48, 0x4b, 0x3, 0x2, 0xf, 
       0x10, 0x3, 0x2, 0x5f, 0x60, 0x3, 0x2, 0x69, 0x6b, 0x3, 0x2, 0x73, 
       0x74, 0x6, 0x2, 0x5b, 0x5e, 0x6c, 0x6c, 0x75, 0x77, 0x84, 0x84, 0x8, 
       0x2, 0x32, 0x3e, 0x41, 0x4b, 0x4f, 0x57, 0x59, 0x5a, 0x5f, 0x66, 
       0x78, 0x81, 0x4, 0x2, 0x15, 0x15, 0x1d, 0x20, 0x4, 0x2, 0x16, 0x16, 
       0x21, 0x24, 0x4, 0x2, 0x10, 0x10, 0x25, 0x2f, 0x2, 0x716, 0x2, 0xcb, 
       0x3, 0x2, 0x2, 0x2, 0x4, 0xe4, 0x3, 0x2, 0x2, 0x2, 0x6, 0xe8, 0x3, 
       0x2, 0x2, 0x2, 0x8, 0xea, 0x3, 0x2, 0x2, 0x2, 0xa, 0x100, 0x3, 0x2, 
       0x2, 0x2, 0xc, 0x104, 0x3, 0x2, 0x2, 0x2, 0xe, 0x129, 0x3, 0x2, 0x2, 
       0x2, 0x10, 0x141, 0x3, 0x2, 0x2, 0x2, 0x12, 0x14c, 0x3, 0x2, 0x2, 
       0x2, 0x14, 0x151, 0x3, 0x2, 0x2, 0x2, 0x16, 0x155, 0x3, 0x2, 0x2, 
       0x2, 0x18, 0x16b, 0x3, 0x2, 0x2, 0x2, 0x1a, 0x175, 0x3, 0x2, 0x2, 
       0x2, 0x1c, 0x18b, 0x3, 0x2, 0x2, 0x2, 0x1e, 0x18d, 0x3, 0x2, 0x2, 
       0x2, 0x20, 0x193, 0x3, 0x2, 0x2, 0x2, 0x22, 0x1c3, 0x3, 0x2, 0x2, 
       0x2, 0x24, 0x1c7, 0x3, 0x2, 0x2, 0x2, 0x26, 0x1db, 0x3, 0x2, 0x2, 
       0x2, 0x28, 0x1ef, 0x3, 0x2, 0x2, 0x2, 0x2a, 0x1f1, 0x3, 0x2, 0x2, 
       0x2, 0x2c, 0x1fc, 0x3, 0x2, 0x2, 0x2, 0x2e, 0x217, 0x3, 0x2, 0x2, 
       0x2, 0x30, 0x224, 0x3, 0x2, 0x2, 0x2, 0x32, 0x228, 0x3, 0x2, 0x2, 
       0x2, 0x34, 0x237, 0x3, 0x2, 0x2, 0x2, 0x36, 0x241, 0x3, 0x2, 0x2, 
       0x2, 0x38, 0x26a, 0x3, 0x2, 0x2, 0x2, 0x3a, 0x273, 0x3, 0x2, 0x2, 
       0x2, 0x3c, 0x275, 0x3, 0x2, 0x2, 0x2, 0x3e, 0x284, 0x3, 0x2, 0x2, 
       0x2, 0x40, 0x288, 0x3, 0x2, 0x2, 0x2, 0x42, 0x28c, 0x3, 0x2, 0x2, 
       0x2, 0x44, 0x2a1, 0x3, 0x2, 0x2, 0x2, 0x46, 0x2a3, 0x3, 0x2, 0x2, 
       0x2, 0x48, 0x2a7, 0x3, 0x2, 0x2, 0x2, 0x4a, 0x2c0, 0x3, 0x2, 0x2, 
       0x2, 0x4c, 0x2c2, 0x3, 0x2, 0x2, 0x2, 0x4e, 0x2d2, 0x3, 0x2, 0x2, 
       0x2, 0x50, 0x2d4, 0x3, 0x2, 0x2, 0x2, 0x52, 0x2ec, 0x3, 0x2, 0x2, 
       0x2, 0x54, 0x332, 0x3, 0x2, 0x2, 0x2, 0x56, 0x334, 0x3, 0x2, 0x2, 
       0x2, 0x58, 0x351, 0x3, 0x2, 0x2, 0x2, 0x5a, 0x353, 0x3, 0x2, 0x2, 
       0x2, 0x5c, 0x368, 0x3, 0x2, 0x2, 0x2, 0x5e, 0x372, 0x3, 0x2, 0x2, 
       0x2, 0x60, 0x378, 0x3, 0x2, 0x2, 0x2, 0x62, 0x38e, 0x3, 0x2, 0x2, 
       0x2, 0x64, 0x390, 0x3, 0x2, 0x2, 0x2, 0x66, 0x392, 0x3, 0x2, 0x2, 
       0x2, 0x68, 0x394, 0x3, 0x2, 0x2, 0x2, 0x6a, 0x39e, 0x3, 0x2, 0x2, 
       0x2, 0x6c, 0x3a8, 0x3, 0x2, 0x2, 0x2, 0x6e, 0x3b8, 0x3, 0x2, 0x2, 
       0x2, 0x70, 0x3bd, 0x3, 0x2, 0x2, 0x2, 0x72, 0x3c7, 0x3, 0x2, 0x2, 
       0x2, 0x74, 0x3dd, 0x3, 0x2, 0x2, 0x2, 0x76, 0x3fb, 0x3, 0x2, 0x2, 
       0x2, 0x78, 0x40f, 0x3, 0x2, 0x2, 0x2, 0x7a, 0x414, 0x3, 0x2, 0x2, 
       0x2, 0x7c, 0x436, 0x3, 0x2, 0x2, 0x2, 0x7e, 0x444, 0x3, 0x2, 0x2, 
       0x2, 0x80, 0x455, 0x3, 0x2, 0x2, 0x2, 0x82, 0x457, 0x3, 0x2, 0x2, 
       0x2, 0x84, 0x4b5, 0x3, 0x2, 0x2, 0x2, 0x86, 0x4bd, 0x3, 0x2, 0x2, 
       0x2, 0x88, 0x4bf, 0x3, 0x2, 0x2, 0x2, 0x8a, 0x4c1, 0x3, 0x2, 0x2, 
       0x2, 0x8c, 0x4f8, 0x3, 0x2, 0x2, 0x2, 0x8e, 0x4fa, 0x3, 0x2, 0x2, 
       0x2, 0x90, 0x504, 0x3, 0x2, 0x2, 0x2, 0x92, 0x50d, 0x3, 0x2, 0x2, 
       0x2, 0x94, 0x514, 0x3, 0x2, 0x2, 0x2, 0x96, 0x51a, 0x3, 0x2, 0x2, 
       0x2, 0x98, 0x541, 0x3, 0x2, 0x2, 0x2, 0x9a, 0x543, 0x3, 0x2, 0x2, 
       0x2, 0x9c, 0x560, 0x3, 0x2, 0x2, 0x2, 0x9e, 0x562, 0x3, 0x2, 0x2, 
       0x2, 0xa0, 0x564, 0x3, 0x2, 0x2, 0x2, 0xa2, 0x56c, 0x3, 0x2, 0x2, 
       0x2, 0xa4, 0x56f, 0x3, 0x2, 0x2, 0x2, 0xa6, 0x583, 0x3, 0x2, 0x2, 
       0x2, 0xa8, 0x5a9, 0x3, 0x2, 0x2, 0x2, 0xaa, 0x5c5, 0x3, 0x2, 0x2, 
       0x2, 0xac, 0x5d6, 0x3, 0x2, 0x2, 0x2, 0xae, 0x5e4, 0x3, 0x2, 0x2, 
       0x2, 0xb0, 0x5e8, 0x3, 0x2, 0x2, 0x2, 0xb2, 0x5ea, 0x3, 0x2, 0x2, 
       0x2, 0xb4, 0x613, 0x3, 0x2, 0x2, 0x2, 0xb6, 0x618, 0x3, 0x2, 0x2, 
       0x2, 0xb8, 0x621, 0x3, 0x2, 0x2, 0x2, 0xba, 0x623, 0x3, 0x2, 0x2, 
       0x2, 0xbc, 0x625, 0x3, 0x2, 0x2, 0x2, 0xbe, 0x629, 0x3, 0x2, 0x2, 
       0x2, 0xc0, 0x62b, 0x3, 0x2, 0x2, 0x2, 0xc2, 0x62d, 0x3, 0x2, 0x2, 
       0x2, 0xc4, 0x62f, 0x3, 0x2, 0x2, 0x2, 0xc6, 0x631, 0x3, 0x2, 0x2, 
       0x2, 0xc8, 0x633, 0x3, 0x2, 0x2, 0x2, 0xca, 0xcc, 0x7, 0x85, 0x2, 
       0x2, 0xcb, 0xca, 0x3, 0x2, 0x2, 0x2, 0xcb, 0xcc, 0x3, 0x2, 0x2, 0x2, 
       0xcc, 0xcd, 0x3, 0x2, 0x2, 0x2, 0xcd, 0xd2, 0x5, 0x4, 0x3, 0x2, 0xce, 
       0xd0, 0x7, 0x85, 0x2, 0x2, 0xcf, 0xce, 0x3, 0x2, 0x2, 0x2, 0xcf, 
       0xd0, 0x3, 0x2, 0x2, 0x2, 0xd0, 0xd1, 0x3, 0x2, 0x2, 0x2, 0xd1, 0xd3, 
       0x7, 0x3, 0x2, 0x2, 0xd2, 0xcf, 0x3, 0x2, 0x2, 0x2, 0xd2, 0xd3, 0x3, 
       0x2, 0x2, 0x2, 0xd3, 0xd5, 0x3, 0x2, 0x2, 0x2, 0xd4, 0xd6, 0x7, 0x85, 
       0x2, 0x2, 0xd5, 0xd4, 0x3, 0x2, 0x2, 0x2, 0xd5, 0xd6, 0x3, 0x2, 0x2, 
       0x2, 0xd6, 0xd7, 0x3, 0x2, 0x2, 0x2, 0xd7, 0xd8, 0x7, 0x2, 0x2, 0x3, 
       0xd8, 0x3, 0x3, 0x2, 0x2, 0x2, 0xd9, 0xe5, 0x5, 0x6, 0x4, 0x2, 0xda, 
       0xdc, 0x7, 0x30, 0x2, 0x2, 0xdb, 0xdd, 0x7, 0x85, 0x2, 0x2, 0xdc, 
       0xdb, 0x3, 0x2, 0x2, 0x2, 0xdc, 0xdd, 0x3, 0x2, 0x2, 0x2, 0xdd, 0xde, 
       0x3, 0x2, 0x2, 0x2, 0xde, 0xe5, 0x5, 0x6, 0x4, 0x2, 0xdf, 0xe1, 0x7, 
       0x31, 0x2, 0x2, 0xe0, 0xe2, 0x7, 0x85, 0x2, 0x2, 0xe1, 0xe0, 0x3, 
       0x2, 0x2, 0x2, 0xe1, 0xe2, 0x3, 0x2, 0x2, 0x2, 0xe2, 0xe3, 0x3, 0x2, 
       0x2, 0x2, 0xe3, 0xe5, 0x5, 0x6, 0x4, 0x2, 0xe4, 0xd9, 0x3, 0x2, 0x2, 
       0x2, 0xe4, 0xda, 0x3, 0x2, 0x2, 0x2, 0xe4, 0xdf, 0x3, 0x2, 0x2, 0x2, 
       0xe5, 0x5, 0x3, 0x2, 0x2, 0x2, 0xe6, 0xe9, 0x5, 0x8, 0x5, 0x2, 0xe7, 
       0xe9, 0x5, 0x2c, 0x17, 0x2, 0xe8, 0xe6, 0x3, 0x2, 0x2, 0x2, 0xe8, 
       0xe7, 0x3, 0x2, 0x2, 0x2, 0xe9, 0x7, 0x3, 0x2, 0x2, 0x2, 0xea, 0xf1, 
       0x5, 0xc, 0x7, 0x2, 0xeb, 0xed, 0x7, 0x85, 0x2, 0x2, 0xec, 0xeb, 
       0x3, 0x2, 0x2, 0x2, 0xec, 0xed, 0x3, 0x2, 0x2, 0x2, 0xed, 0xee, 0x3, 
       0x2, 0x2, 0x2, 0xee, 0xf0, 0x5, 0xa, 0x6, 0x2, 0xef, 0xec, 0x3, 0x2, 
       0x2, 0x2, 0xf0, 0xf3, 0x3, 0x2, 0x2, 0x2, 0xf1, 0xef, 0x3, 0x2, 0x2, 
       0x2, 0xf1, 0xf2, 0x3, 0x2, 0x2, 0x2, 0xf2, 0x9, 0x3, 0x2, 0x2, 0x2, 
       0xf3, 0xf1, 0x3, 0x2, 0x2, 0x2, 0xf4, 0xf5, 0x7, 0x32, 0x2, 0x2, 
       0xf5, 0xf6, 0x7, 0x85, 0x2, 0x2, 0xf6, 0xf8, 0x7, 0x33, 0x2, 0x2, 
       0xf7, 0xf9, 0x7, 0x85, 0x2, 0x2, 0xf8, 0xf7, 0x3, 0x2, 0x2, 0x2, 
       0xf8, 0xf9, 0x3, 0x2, 0x2, 0x2, 0xf9, 0xfa, 0x3, 0x2, 0x2, 0x2, 0xfa, 
       0x101, 0x5, 0xc, 0x7, 0x2, 0xfb, 0xfd, 0x7, 0x32, 0x2, 0x2, 0xfc, 
       0xfe, 0x7, 0x85, 0x2, 0x2, 0xfd, 0xfc, 0x3, 0x2, 0x2, 0x2, 0xfd, 
       0xfe, 0x3, 0x2, 0x2, 0x2, 0xfe, 0xff, 0x3, 0x2, 0x2, 0x2, 0xff, 0x101, 
       0x5, 0xc, 0x7, 0x2, 0x100, 0xf4, 0x3, 0x2, 0x2, 0x2, 0x100, 0xfb, 
       0x3, 0x2, 0x2, 0x2, 0x101, 0xb, 0x3, 0x2, 0x2, 0x2, 0x102, 0x105, 
       0x5, 0xe, 0x8, 0x2, 0x103, 0x105, 0x5, 0x10, 0x9, 0x2, 0x104, 0x102, 
       0x3, 0x2, 0x2, 0x2, 0x104, 0x103, 0x3, 0x2, 0x2, 0x2, 0x105, 0xd, 
       0x3, 0x2, 0x2, 0x2, 0x106, 0x108, 0x5, 0x14, 0xb, 0x2, 0x107, 0x109, 
       0x7, 0x85, 0x2, 0x2, 0x108, 0x107, 0x3, 0x2, 0x2, 0x2, 0x108, 0x109, 
       0x3, 0x2, 0x2, 0x2, 0x109, 0x10b, 0x3, 0x2, 0x2, 0x2, 0x10a, 0x106, 
       0x3, 0x2, 0x2, 0x2, 0x10b, 0x10e, 0x3, 0x2, 0x2, 0x2, 0x10c, 0x10a, 
       0x3, 0x2, 0x2, 0x2, 0x10c, 0x10d, 0x3, 0x2, 0x2, 0x2, 0x10d, 0x10f, 
       0x3, 0x2, 0x2, 0x2, 0x10e, 0x10c, 0x3, 0x2, 0x2, 0x2, 0x10f, 0x12a, 
       0x5, 0x34, 0x1b, 0x2, 0x110, 0x112, 0x5, 0x14, 0xb, 0x2, 0x111, 0x113, 
       0x7, 0x85, 0x2, 0x2, 0x112, 0x111, 0x3, 0x2, 0x2, 0x2, 0x112, 0x113, 
       0x3, 0x2, 0x2, 0x2, 0x113, 0x115, 0x3, 0x2, 0x2, 0x2, 0x114, 0x110, 
       0x3, 0x2, 0x2, 0x2, 0x115, 0x118, 0x3, 0x2, 0x2, 0x2, 0x116, 0x114, 
       0x3, 0x2, 0x2, 0x2, 0x116, 0x117, 0x3, 0x2, 0x2, 0x2, 0x117, 0x119, 
       0x3, 0x2, 0x2, 0x2, 0x118, 0x116, 0x3, 0x2, 0x2, 0x2, 0x119, 0x120, 
       0x5, 0x12, 0xa, 0x2, 0x11a, 0x11c, 0x7, 0x85, 0x2, 0x2, 0x11b, 0x11a, 
       0x3, 0x2, 0x2, 0x2, 0x11b, 0x11c, 0x3, 0x2, 0x2, 0x2, 0x11c, 0x11d, 
       0x3, 0x2, 0x2, 0x2, 0x11d, 0x11f, 0x5, 0x12, 0xa, 0x2, 0x11e, 0x11b, 
       0x3, 0x2, 0x2, 0x2, 0x11f, 0x122, 0x3, 0x2, 0x2, 0x2, 0x120, 0x11e, 
       0x3, 0x2, 0x2, 0x2, 0x120, 0x121, 0x3, 0x2, 0x2, 0x2, 0x121, 0x127, 
       0x3, 0x2, 0x2, 0x2, 0x122, 0x120, 0x3, 0x2, 0x2, 0x2, 0x123, 0x125, 
       0x7, 0x85, 0x2, 0x2, 0x124, 0x123, 0x3, 0x2, 0x2, 0x2, 0x124, 0x125, 
       0x3, 0x2, 0x2, 0x2, 0x125, 0x126, 0x3, 0x2, 0x2, 0x2, 0x126, 0x128, 
       0x5, 0x34, 0x1b, 0x2, 0x127, 0x124, 0x3, 0x2, 0x2, 0x2, 0x127, 0x128, 
       0x3, 0x2, 0x2, 0x2, 0x128, 0x12a, 0x3, 0x2, 0x2, 0x2, 0x129, 0x10c, 
       0x3, 0x2, 0x2, 0x2, 0x129, 0x116, 0x3, 0x2, 0x2, 0x2, 0x12a, 0xf, 
       0x3, 0x2, 0x2, 0x2, 0x12b, 0x12d, 0x5, 0x14, 0xb, 0x2, 0x12c, 0x12e, 
       0x7, 0x85, 0x2, 0x2, 0x12d, 0x12c, 0x3, 0x2, 0x2, 0x2, 0x12d, 0x12e, 
       0x3, 0x2, 0x2, 0x2, 0x12e, 0x130, 0x3, 0x2, 0x2, 0x2, 0x12f, 0x12b, 
       0x3, 0x2, 0x2, 0x2, 0x130, 0x133, 0x3, 0x2, 0x2, 0x2, 0x131, 0x12f, 
       0x3, 0x2, 0x2, 0x2, 0x131, 0x132, 0x3, 0x2, 0x2, 0x2, 0x132, 0x13a, 
       0x3, 0x2, 0x2, 0x2, 0x133, 0x131, 0x3, 0x2, 0x2, 0x2, 0x134, 0x136, 
       0x5, 0x12, 0xa, 0x2, 0x135, 0x137, 0x7, 0x85, 0x2, 0x2, 0x136, 0x135, 
       0x3, 0x2, 0x2, 0x2, 0x136, 0x137, 0x3, 0x2, 0x2, 0x2, 0x137, 0x139, 
       0x3, 0x2, 0x2, 0x2, 0x138, 0x134, 0x3, 0x2, 0x2, 0x2, 0x139, 0x13c, 
       0x3, 0x2, 0x2, 0x2, 0x13a, 0x138, 0x3, 0x2, 0x2, 0x2, 0x13a, 0x13b, 
       0x3, 0x2, 0x2, 0x2, 0x13b, 0x13d, 0x3, 0x2, 0x2, 0x2, 0x13c, 0x13a, 
       0x3, 0x2, 0x2, 0x2, 0x13d, 0x13f, 0x5, 0x32, 0x1a, 0x2, 0x13e, 0x140, 
       0x7, 0x85, 0x2, 0x2, 0x13f, 0x13e, 0x3, 0x2, 0x2, 0x2, 0x13f, 0x140, 
       0x3, 0x2, 0x2, 0x2, 0x140, 0x142, 0x3, 0x2, 0x2, 0x2, 0x141, 0x131, 
       0x3, 0x2, 0x2, 0x2, 0x142, 0x143, 0x3, 0x2, 0x2, 0x2, 0x143, 0x141, 
       0x3, 0x2, 0x2, 0x2, 0x143, 0x144, 0x3, 0x2, 0x2, 0x2, 0x144, 0x145, 
       0x3, 0x2, 0x2, 0x2, 0x145, 0x146, 0x5, 0xe, 0x8, 0x2, 0x146, 0x11, 
       0x3, 0x2, 0x2, 0x2, 0x147, 0x14d, 0x5, 0x1e, 0x10, 0x2, 0x148, 0x14d, 
       0x5, 0x1a, 0xe, 0x2, 0x149, 0x14d, 0x5, 0x24, 0x13, 0x2, 0x14a, 0x14d, 
       0x5, 0x20, 0x11, 0x2, 0x14b, 0x14d, 0x5, 0x26, 0x14, 0x2, 0x14c, 
       0x147, 0x3, 0x2, 0x2, 0x2, 0x14c, 0x148, 0x3, 0x2, 0x2, 0x2, 0x14c, 
       0x149, 0x3, 0x2, 0x2, 0x2, 0x14c, 0x14a, 0x3, 0x2, 0x2, 0x2, 0x14c, 
       0x14b, 0x3, 0x2, 0x2, 0x2, 0x14d, 0x13, 0x3, 0x2, 0x2, 0x2, 0x14e, 
       0x152, 0x5, 0x16, 0xc, 0x2, 0x14f, 0x152, 0x5, 0x18, 0xd, 0x2, 0x150, 
       0x152, 0x5, 0x2a, 0x16, 0x2, 0x151, 0x14e, 0x3, 0x2, 0x2, 0x2, 0x151, 
       0x14f, 0x3, 0x2, 0x2, 0x2, 0x151, 0x150, 0x3, 0x2, 0x2, 0x2, 0x152, 
       0x15, 0x3, 0x2, 0x2, 0x2, 0x153, 0x154, 0x7, 0x34, 0x2, 0x2, 0x154, 
       0x156, 0x7, 0x85, 0x2, 0x2, 0x155, 0x153, 0x3, 0x2, 0x2, 0x2, 0x155, 
       0x156, 0x3, 0x2, 0x2, 0x2, 0x156, 0x157, 0x3, 0x2, 0x2, 0x2, 0x157, 
       0x159, 0x7, 0x35, 0x2, 0x2, 0x158, 0x15a, 0x7, 0x85, 0x2, 0x2, 0x159, 
       0x158, 0x3, 0x2, 0x2, 0x2, 0x159, 0x15a, 0x3, 0x2, 0x2, 0x2, 0x15a, 
       0x15b, 0x3, 0x2, 0x2, 0x2, 0x15b, 0x162, 0x5, 0x48, 0x25, 0x2, 0x15c, 
       0x15e, 0x7, 0x85, 0x2, 0x2, 0x15d, 0x15c, 0x3, 0x2, 0x2, 0x2, 0x15d, 
       0x15e, 0x3, 0x2, 0x2, 0x2, 0x15e, 0x15f, 0x3, 0x2, 0x2, 0x2, 0x15f, 
       0x161, 0x5, 0x44, 0x23, 0x2, 0x160, 0x15d, 0x3, 0x2, 0x2, 0x2, 0x161, 
       0x164, 0x3, 0x2, 0x2, 0x2, 0x162, 0x160, 0x3, 0x2, 0x2, 0x2, 0x162, 
       0x163, 0x3, 0x2, 0x2, 0x2, 0x163, 0x169, 0x3, 0x2, 0x2, 0x2, 0x164, 
       0x162, 0x3, 0x2, 0x2, 0x2, 0x165, 0x167, 0x7, 0x85, 0x2, 0x2, 0x166, 
       0x165, 0x3, 0x2, 0x2, 0x2, 0x166, 0x167, 0x3, 0x2, 0x2, 0x2, 0x167, 
       0x168, 0x3, 0x2, 0x2, 0x2, 0x168, 0x16a, 0x5, 0x46, 0x24, 0x2, 0x169, 
       0x166, 0x3, 0x2, 0x2, 0x2, 0x169, 0x16a, 0x3, 0x2, 0x2, 0x2, 0x16a, 
       0x17, 0x3, 0x2, 0x2, 0x2, 0x16b, 0x16d, 0x7, 0x36, 0x2, 0x2, 0x16c, 
       0x16e, 0x7, 0x85, 0x2, 0x2, 0x16d, 0x16c, 0x3, 0x2, 0x2, 0x2, 0x16d, 
       0x16e, 0x3, 0x2, 0x2, 0x2, 0x16e, 0x16f, 0x3, 0x2, 0x2, 0x2, 0x16f, 
       0x170, 0x5, 0x66, 0x34, 0x2, 0x170, 0x171, 0x7, 0x85, 0x2, 0x2, 0x171, 
       0x172, 0x7, 0x37, 0x2, 0x2, 0x172, 0x173, 0x7, 0x85, 0x2, 0x2, 0x173, 
       0x174, 0x5, 0xae, 0x58, 0x2, 0x174, 0x19, 0x3, 0x2, 0x2, 0x2, 0x175, 
       0x177, 0x7, 0x38, 0x2, 0x2, 0x176, 0x178, 0x7, 0x85, 0x2, 0x2, 0x177, 
       0x176, 0x3, 0x2, 0x2, 0x2, 0x177, 0x178, 0x3, 0x2, 0x2, 0x2, 0x178, 
       0x179, 0x3, 0x2, 0x2, 0x2, 0x179, 0x17e, 0x5, 0x4a, 0x26, 0x2, 0x17a, 
       0x17b, 0x7, 0x85, 0x2, 0x2, 0x17b, 0x17d, 0x5, 0x1c, 0xf, 0x2, 0x17c, 
       0x17a, 0x3, 0x2, 0x2, 0x2, 0x17d, 0x180, 0x3, 0x2, 0x2, 0x2, 0x17e, 
       0x17c, 0x3, 0x2, 0x2, 0x2, 0x17e, 0x17f, 0x3, 0x2, 0x2, 0x2, 0x17f, 
       0x1b, 0x3, 0x2, 0x2, 0x2, 0x180, 0x17e, 0x3, 0x2, 0x2, 0x2, 0x181, 
       0x182, 0x7, 0x39, 0x2, 0x2, 0x182, 0x183, 0x7, 0x85, 0x2, 0x2, 0x183, 
       0x184, 0x7, 0x35, 0x2, 0x2, 0x184, 0x185, 0x7, 0x85, 0x2, 0x2, 0x185, 
       0x18c, 0x5, 0x20, 0x11, 0x2, 0x186, 0x187, 0x7, 0x39, 0x2, 0x2, 0x187, 
       0x188, 0x7, 0x85, 0x2, 0x2, 0x188, 0x189, 0x7, 0x3a, 0x2, 0x2, 0x189, 
       0x18a, 0x7, 0x85, 0x2, 0x2, 0x18a, 0x18c, 0x5, 0x20, 0x11, 0x2, 0x18b, 
       0x181, 0x3, 0x2, 0x2, 0x2, 0x18b, 0x186, 0x3, 0x2, 0x2, 0x2, 0x18c, 
       0x1d, 0x3, 0x2, 0x2, 0x2, 0x18d, 0x18f, 0x7, 0x3a, 0x2, 0x2, 0x18e, 
       0x190, 0x7, 0x85, 0x2, 0x2, 0x18f, 0x18e, 0x3, 0x2, 0x2, 0x2, 0x18f, 
       0x190, 0x3, 0x2, 0x2, 0x2, 0x190, 0x191, 0x3, 0x2, 0x2, 0x2, 0x191, 
       0x192, 0x5, 0x48, 0x25, 0x2, 0x192, 0x1f, 0x3, 0x2, 0x2, 0x2, 0x193, 
       0x195, 0x7, 0x3b, 0x2, 0x2, 0x194, 0x196, 0x7, 0x85, 0x2, 0x2, 0x195, 
       0x194, 0x3, 0x2, 0x2, 0x2, 0x195, 0x196, 0x3, 0x2, 0x2, 0x2, 0x196, 
       0x197, 0x3, 0x2, 0x2, 0x2, 0x197, 0x19c, 0x5, 0x22, 0x12, 0x2, 0x198, 
       0x199, 0x7, 0x4, 0x2, 0x2, 0x199, 0x19b, 0x5, 0x22, 0x12, 0x2, 0x19a, 
       0x198, 0x3, 0x2, 0x2, 0x2, 0x19b, 0x19e, 0x3, 0x2, 0x2, 0x2, 0x19c, 
       0x19a, 0x3, 0x2, 0x2, 0x2, 0x19c, 0x19d, 0x3, 0x2, 0x2, 0x2, 0x19d, 
       0x21, 0x3, 0x2, 0x2, 0x2, 0x19e, 0x19c, 0x3, 0x2, 0x2, 0x2, 0x19f, 
       0x1a1, 0x5, 0xb6, 0x5c, 0x2, 0x1a0, 0x1a2, 0x7, 0x85, 0x2, 0x2, 0x1a1, 
       0x1a0, 0x3, 0x2, 0x2, 0x2, 0x1a1, 0x1a2, 0x3, 0x2, 0x2, 0x2, 0x1a2, 
       0x1a3, 0x3, 0x2, 0x2, 0x2, 0x1a3, 0x1a5, 0x7, 0x5, 0x2, 0x2, 0x1a4, 
       0x1a6, 0x7, 0x85, 0x2, 0x2, 0x1a5, 0x1a4, 0x3, 0x2, 0x2, 0x2, 0x1a5, 
       0x1a6, 0x3, 0x2, 0x2, 0x2, 0x1a6, 0x1a7, 0x3, 0x2, 0x2, 0x2, 0x1a7, 
       0x1a8, 0x5, 0x66, 0x34, 0x2, 0x1a8, 0x1c4, 0x3, 0x2, 0x2, 0x2, 0x1a9, 
       0x1ab, 0x5, 0xae, 0x58, 0x2, 0x1aa, 0x1ac, 0x7, 0x85, 0x2, 0x2, 0x1ab, 
       0x1aa, 0x3, 0x2, 0x2, 0x2, 0x1ab, 0x1ac, 0x3, 0x2, 0x2, 0x2, 0x1ac, 
       0x1ad, 0x3, 0x2, 0x2, 0x2, 0x1ad, 0x1af, 0x7, 0x5, 0x2, 0x2, 0x1ae, 
       0x1b0, 0x7, 0x85, 0x2, 0x2, 0x1af, 0x1ae, 0x3, 0x2, 0x2, 0x2, 0x1af, 
       0x1b0, 0x3, 0x2, 0x2, 0x2, 0x1b0, 0x1b1, 0x3, 0x2, 0x2, 0x2, 0x1b1, 
       0x1b2, 0x5, 0x66, 0x34, 0x2, 0x1b2, 0x1c4, 0x3, 0x2, 0x2, 0x2, 0x1b3, 
       0x1b5, 0x5, 0xae, 0x58, 0x2, 0x1b4, 0x1b6, 0x7, 0x85, 0x2, 0x2, 0x1b5, 
       0x1b4, 0x3, 0x2, 0x2, 0x2, 0x1b5, 0x1b6, 0x3, 0x2, 0x2, 0x2, 0x1b6, 
       0x1b7, 0x3, 0x2, 0x2, 0x2, 0x1b7, 0x1b9, 0x7, 0x6, 0x2, 0x2, 0x1b8, 
       0x1ba, 0x7, 0x85, 0x2, 0x2, 0x1b9, 0x1b8, 0x3, 0x2, 0x2, 0x2, 0x1b9, 
       0x1ba, 0x3, 0x2, 0x2, 0x2, 0x1ba, 0x1bb, 0x3, 0x2, 0x2, 0x2, 0x1bb, 
       0x1bc, 0x5, 0x66, 0x34, 0x2, 0x1bc, 0x1c4, 0x3, 0x2, 0x2, 0x2, 0x1bd, 
       0x1bf, 0x5, 0xae, 0x58, 0x2, 0x1be, 0x1c0, 0x7, 0x85, 0x2, 0x2, 0x1bf, 
       0x1be, 0x3, 0x2, 0x2, 0x2, 0x1bf, 0x1c0, 0x3, 0x2, 0x2, 0x2, 0x1c0, 
       0x1c1, 0x3, 0x2, 0x2, 0x2, 0x1c1, 0x1c2, 0x5, 0x5c, 0x2f, 0x2, 0x1c2, 
       0x1c4, 0x3, 0x2, 0x2, 0x2, 0x1c3, 0x19f, 0x3, 0x2, 0x2, 0x2, 0x1c3, 
       0x1a9, 0x3, 0x2, 0x2, 0x2, 0x1c3, 0x1b3, 0x3, 0x2, 0x2, 0x2, 0x1c3, 
       0x1bd, 0x3, 0x2, 0x2, 0x2, 0x1c4, 0x23, 0x3, 0x2, 0x2, 0x2, 0x1c5, 
       0x1c6, 0x7, 0x3c, 0x2, 0x2, 0x1c6, 0x1c8, 0x7, 0x85, 0x2, 0x2, 0x1c7, 
       0x1c5, 0x3, 0x2, 0x2, 0x2, 0x1c7, 0x1c8, 0x3, 0x2, 0x2, 0x2, 0x1c8, 
       0x1c9, 0x3, 0x2, 0x2, 0x2, 0x1c9, 0x1cb, 0x7, 0x3d, 0x2, 0x2, 0x1ca, 
       0x1cc, 0x7, 0x85, 0x2, 0x2, 0x1cb, 0x1ca, 0x3, 0x2, 0x2, 0x2, 0x1cb, 
       0x1cc, 0x3, 0x2, 0x2, 0x2, 0x1cc, 0x1cd, 0x3, 0x2, 0x2, 0x2, 0x1cd, 
       0x1d8, 0x5, 0x66, 0x34, 0x2, 0x1ce, 0x1d0, 0x7, 0x85, 0x2, 0x2, 0x1cf, 
       0x1ce, 0x3, 0x2, 0x2, 0x2, 0x1cf, 0x1d0, 0x3, 0x2, 0x2, 0x2, 0x1d0, 
       0x1d1, 0x3, 0x2, 0x2, 0x2, 0x1d1, 0x1d3, 0x7, 0x4, 0x2, 0x2, 0x1d2, 
       0x1d4, 0x7, 0x85, 0x2, 0x2, 0x1d3, 0x1d2, 0x3, 0x2, 0x2, 0x2, 0x1d3, 
       0x1d4, 0x3, 0x2, 0x2, 0x2, 0x1d4, 0x1d5, 0x3, 0x2, 0x2, 0x2, 0x1d5, 
       0x1d7, 0x5, 0x66, 0x34, 0x2, 0x1d6, 0x1cf, 0x3, 0x2, 0x2, 0x2, 0x1d7, 
       0x1da, 0x3, 0x2, 0x2, 0x2, 0x1d8, 0x1d6, 0x3, 0x2, 0x2, 0x2, 0x1d8, 
       0x1d9, 0x3, 0x2, 0x2, 0x2, 0x1d9, 0x25, 0x3, 0x2, 0x2, 0x2, 0x1da, 
       0x1d8, 0x3, 0x2, 0x2, 0x2, 0x1db, 0x1dc, 0x7, 0x3e, 0x2, 0x2, 0x1dc, 
       0x1dd, 0x7, 0x85, 0x2, 0x2, 0x1dd, 0x1e8, 0x5, 0x28, 0x15, 0x2, 0x1de, 
       0x1e0, 0x7, 0x85, 0x2, 0x2, 0x1df, 0x1de, 0x3, 0x2, 0x2, 0x2, 0x1df, 
       0x1e0, 0x3, 0x2, 0x2, 0x2, 0x1e0, 0x1e1, 0x3, 0x2, 0x2, 0x2, 0x1e1, 
       0x1e3, 0x7, 0x4, 0x2, 0x2, 0x1e2, 0x1e4, 0x7, 0x85, 0x2, 0x2, 0x1e3, 
       0x1e2, 0x3, 0x2, 0x2, 0x2, 0x1e3, 0x1e4, 0x3, 0x2, 0x2, 0x2, 0x1e4, 
       0x1e5, 0x3, 0x2, 0x2, 0x2, 0x1e5, 0x1e7, 0x5, 0x28, 0x15, 0x2, 0x1e6, 
       0x1df, 0x3, 0x2, 0x2, 0x2, 0x1e7, 0x1ea, 0x3, 0x2, 0x2, 0x2, 0x1e8, 
       0x1e6, 0x3, 0x2, 0x2, 0x2, 0x1e8, 0x1e9, 0x3, 0x2, 0x2, 0x2, 0x1e9, 
       0x27, 0x3, 0x2, 0x2, 0x2, 0x1ea, 0x1e8, 0x3, 0x2, 0x2, 0x2, 0x1eb, 
       0x1ec, 0x5, 0xae, 0x58, 0x2, 0x1ec, 0x1ed, 0x5, 0x5c, 0x2f, 0x2, 
       0x1ed, 0x1f0, 0x3, 0x2, 0x2, 0x2, 0x1ee, 0x1f0, 0x5, 0xb6, 0x5c, 
       0x2, 0x1ef, 0x1eb, 0x3, 0x2, 0x2, 0x2, 0x1ef, 0x1ee, 0x3, 0x2, 0x2, 
       0x2, 0x1f0, 0x29, 0x3, 0x2, 0x2, 0x2, 0x1f1, 0x1f2, 0x7, 0x3f, 0x2, 
       0x2, 0x1f2, 0x1f3, 0x7, 0x85, 0x2, 0x2, 0x1f3, 0x1fa, 0x5, 0x9a, 
       0x4e, 0x2, 0x1f4, 0x1f6, 0x7, 0x85, 0x2, 0x2, 0x1f5, 0x1f4, 0x3, 
       0x2, 0x2, 0x2, 0x1f5, 0x1f6, 0x3, 0x2, 0x2, 0x2, 0x1f6, 0x1f7, 0x3, 
       0x2, 0x2, 0x2, 0x1f7, 0x1f8, 0x7, 0x40, 0x2, 0x2, 0x1f8, 0x1f9, 0x7, 
       0x85, 0x2, 0x2, 0x1f9, 0x1fb, 0x5, 0x2e, 0x18, 0x2, 0x1fa, 0x1f5, 
       0x3, 0x2, 0x2, 0x2, 0x1fa, 0x1fb, 0x3, 0x2, 0x2, 0x2, 0x1fb, 0x2b, 
       0x3, 0x2, 0x2, 0x2, 0x1fc, 0x1fd, 0x7, 0x3f, 0x2, 0x2, 0x1fd, 0x200, 
       0x7, 0x85, 0x2, 0x2, 0x1fe, 0x201, 0x5, 0x9a, 0x4e, 0x2, 0x1ff, 0x201, 
       0x5, 0x9c, 0x4f, 0x2, 0x200, 0x1fe, 0x3, 0x2, 0x2, 0x2, 0x200, 0x1ff, 
       0x3, 0x2, 0x2, 0x2, 0x201, 0x206, 0x3, 0x2, 0x2, 0x2, 0x202, 0x203, 
       0x7, 0x85, 0x2, 0x2, 0x203, 0x204, 0x7, 0x40, 0x2, 0x2, 0x204, 0x205, 
       0x7, 0x85, 0x2, 0x2, 0x205, 0x207, 0x5, 0x2e, 0x18, 0x2, 0x206, 0x202, 
       0x3, 0x2, 0x2, 0x2, 0x206, 0x207, 0x3, 0x2, 0x2, 0x2, 0x207, 0x2d, 
       0x3, 0x2, 0x2, 0x2, 0x208, 0x218, 0x7, 0x7, 0x2, 0x2, 0x209, 0x214, 
       0x5, 0x30, 0x19, 0x2, 0x20a, 0x20c, 0x7, 0x85, 0x2, 0x2, 0x20b, 0x20a, 
       0x3, 0x2, 0x2, 0x2, 0x20b, 0x20c, 0x3, 0x2, 0x2, 0x2, 0x20c, 0x20d, 
       0x3, 0x2, 0x2, 0x2, 0x20d, 0x20f, 0x7, 0x4, 0x2, 0x2, 0x20e, 0x210, 
       0x7, 0x85, 0x2, 0x2, 0x20f, 0x20e, 0x3, 0x2, 0x2, 0x2, 0x20f, 0x210, 
       0x3, 0x2, 0x2, 0x2, 0x210, 0x211, 0x3, 0x2, 0x2, 0x2, 0x211, 0x213, 
       0x5, 0x30, 0x19, 0x2, 0x212, 0x20b, 0x3, 0x2, 0x2, 0x2, 0x213, 0x216, 
       0x3, 0x2, 0x2, 0x2, 0x214, 0x212, 0x3, 0x2, 0x2, 0x2, 0x214, 0x215, 
       0x3, 0x2, 0x2, 0x2, 0x215, 0x218, 0x3, 0x2, 0x2, 0x2, 0x216, 0x214, 
       0x3, 0x2, 0x2, 0x2, 0x217, 0x208, 0x3, 0x2, 0x2, 0x2, 0x217, 0x209, 
       0x3, 0x2, 0x2, 0x2, 0x218, 0x21d, 0x3, 0x2, 0x2, 0x2, 0x219, 0x21b, 
       0x7, 0x85, 0x2, 0x2, 0x21a, 0x219, 0x3, 0x2, 0x2, 0x2, 0x21a, 0x21b, 
       0x3, 0x2, 0x2, 0x2, 0x21b, 0x21c, 0x3, 0x2, 0x2, 0x2, 0x21c, 0x21e, 
       0x5, 0x46, 0x24, 0x2, 0x21d, 0x21a, 0x3, 0x2, 0x2, 0x2, 0x21d, 0x21e, 
       0x3, 0x2, 0x2, 0x2, 0x21e, 0x2f, 0x3, 0x2, 0x2, 0x2, 0x21f, 0x220, 
       0x5, 0x9e, 0x50, 0x2, 0x220, 0x221, 0x7, 0x85, 0x2, 0x2, 0x221, 0x222, 
       0x7, 0x37, 0x2, 0x2, 0x222, 0x223, 0x7, 0x85, 0x2, 0x2, 0x223, 0x225, 
       0x3, 0x2, 0x2, 0x2, 0x224, 0x21f, 0x3, 0x2, 0x2, 0x2, 0x224, 0x225, 
       0x3, 0x2, 0x2, 0x2, 0x225, 0x226, 0x3, 0x2, 0x2, 0x2, 0x226, 0x227, 
       0x5, 0xae, 0x58, 0x2, 0x227, 0x31, 0x3, 0x2, 0x2, 0x2, 0x228, 0x22d, 
       0x7, 0x41, 0x2, 0x2, 0x229, 0x22b, 0x7, 0x85, 0x2, 0x2, 0x22a, 0x229, 
       0x3, 0x2, 0x2, 0x2, 0x22a, 0x22b, 0x3, 0x2, 0x2, 0x2, 0x22b, 0x22c, 
       0x3, 0x2, 0x2, 0x2, 0x22c, 0x22e, 0x7, 0x42, 0x2, 0x2, 0x22d, 0x22a, 
       0x3, 0x2, 0x2, 0x2, 0x22d, 0x22e, 0x3, 0x2, 0x2, 0x2, 0x22e, 0x22f, 
       0x3, 0x2, 0x2, 0x2, 0x22f, 0x230, 0x7, 0x85, 0x2, 0x2, 0x230, 0x235, 
       0x5, 0x36, 0x1c, 0x2, 0x231, 0x233, 0x7, 0x85, 0x2, 0x2, 0x232, 0x231, 
       0x3, 0x2, 0x2, 0x2, 0x232, 0x233, 0x3, 0x2, 0x2, 0x2, 0x233, 0x234, 
       0x3, 0x2, 0x2, 0x2, 0x234, 0x236, 0x5, 0x46, 0x24, 0x2, 0x235, 0x232, 
       0x3, 0x2, 0x2, 0x2, 0x235, 0x236, 0x3, 0x2, 0x2, 0x2, 0x236, 0x33, 
       0x3, 0x2, 0x2, 0x2, 0x237, 0x23c, 0x7, 0x43, 0x2, 0x2, 0x238, 0x23a, 
       0x7, 0x85, 0x2, 0x2, 0x239, 0x238, 0x3, 0x2, 0x2, 0x2, 0x239, 0x23a, 
       0x3, 0x2, 0x2, 0x2, 0x23a, 0x23b, 0x3, 0x2, 0x2, 0x2, 0x23b, 0x23d, 
       0x7, 0x42, 0x2, 0x2, 0x23c, 0x239, 0x3, 0x2, 0x2, 0x2, 0x23c, 0x23d, 
       0x3, 0x2, 0x2, 0x2, 0x23d, 0x23e, 0x3, 0x2, 0x2, 0x2, 0x23e, 0x23f, 
       0x7, 0x85, 0x2, 0x2, 0x23f, 0x240, 0x5, 0x36, 0x1c, 0x2, 0x240, 0x35, 
       0x3, 0x2, 0x2, 0x2, 0x241, 0x244, 0x5, 0x38, 0x1d, 0x2, 0x242, 0x243, 
       0x7, 0x85, 0x2, 0x2, 0x243, 0x245, 0x5, 0x3c, 0x1f, 0x2, 0x244, 0x242, 
       0x3, 0x2, 0x2, 0x2, 0x244, 0x245, 0x3, 0x2, 0x2, 0x2, 0x245, 0x248, 
       0x3, 0x2, 0x2, 0x2, 0x246, 0x247, 0x7, 0x85, 0x2, 0x2, 0x247, 0x249, 
       0x5, 0x3e, 0x20, 0x2, 0x248, 0x246, 0x3, 0x2, 0x2, 0x2, 0x248, 0x249, 
       0x3, 0x2, 0x2, 0x2, 0x249, 0x24c, 0x3, 0x2, 0x2, 0x2, 0x24a, 0x24b, 
       0x7, 0x85, 0x2, 0x2, 0x24b, 0x24d, 0x5, 0x40, 0x21, 0x2, 0x24c, 0x24a, 
       0x3, 0x2, 0x2, 0x2, 0x24c, 0x24d, 0x3, 0x2, 0x2, 0x2, 0x24d, 0x37, 
       0x3, 0x2, 0x2, 0x2, 0x24e, 0x259, 0x7, 0x7, 0x2, 0x2, 0x24f, 0x251, 
       0x7, 0x85, 0x2, 0x2, 0x250, 0x24f, 0x3, 0x2, 0x2, 0x2, 0x250, 0x251, 
       0x3, 0x2, 0x2, 0x2, 0x251, 0x252, 0x3, 0x2, 0x2, 0x2, 0x252, 0x254, 
       0x7, 0x4, 0x2, 0x2, 0x253, 0x255, 0x7, 0x85, 0x2, 0x2, 0x254, 0x253, 
       0x3, 0x2, 0x2, 0x2, 0x254, 0x255, 0x3, 0x2, 0x2, 0x2, 0x255, 0x256, 
       0x3, 0x2, 0x2, 0x2, 0x256, 0x258, 0x5, 0x3a, 0x1e, 0x2, 0x257, 0x250, 
       0x3, 0x2, 0x2, 0x2, 0x258, 0x25b, 0x3, 0x2, 0x2, 0x2, 0x259, 0x257, 
       0x3, 0x2, 0x2, 0x2, 0x259, 0x25a, 0x3, 0x2, 0x2, 0x2, 0x25a, 0x26b, 
       0x3, 0x2, 0x2, 0x2, 0x25b, 0x259, 0x3, 0x2, 0x2, 0x2, 0x25c, 0x267, 
       0x5, 0x3a, 0x1e, 0x2, 0x25d, 0x25f, 0x7, 0x85, 0x2, 0x2, 0x25e, 0x25d, 
       0x3, 0x2, 0x2, 0x2, 0x25e, 0x25f, 0x3, 0x2, 0x2, 0x2, 0x25f, 0x260, 
       0x3, 0x2, 0x2, 0x2, 0x260, 0x262, 0x7, 0x4, 0x2, 0x2, 0x261, 0x263, 
       0x7, 0x85, 0x2, 0x2, 0x262, 0x261, 0x3, 0x2, 0x2, 0x2, 0x262, 0x263, 
       0x3, 0x2, 0x2, 0x2, 0x263, 0x264, 0x3, 0x2, 0x2, 0x2, 0x264, 0x266, 
       0x5, 0x3a, 0x1e, 0x2, 0x265, 0x25e, 0x3, 0x2, 0x2, 0x2, 0x266, 0x269, 
       0x3, 0x2, 0x2, 0x2, 0x267, 0x265, 0x3, 0x2, 0x2, 0x2, 0x267, 0x268, 
       0x3, 0x2, 0x2, 0x2, 0x268, 0x26b, 0x3, 0x2, 0x2, 0x2, 0x269, 0x267, 
       0x3, 0x2, 0x2, 0x2, 0x26a, 0x24e, 0x3, 0x2, 0x2, 0x2, 0x26a, 0x25c, 
       0x3, 0x2, 0x2, 0x2, 0x26b, 0x39, 0x3, 0x2, 0x2, 0x2, 0x26c, 0x26d, 
       0x5, 0x66, 0x34, 0x2, 0x26d, 0x26e, 0x7, 0x85, 0x2, 0x2, 0x26e, 0x26f, 
       0x7, 0x37, 0x2, 0x2, 0x26f, 0x270, 0x7, 0x85, 0x2, 0x2, 0x270, 0x271, 
       0x5, 0xae, 0x58, 0x2, 0x271, 0x274, 0x3, 0x2, 0x2, 0x2, 0x272, 0x274, 
       0x5, 0x66, 0x34, 0x2, 0x273, 0x26c, 0x3, 0x2, 0x2, 0x2, 0x273, 0x272, 
       0x3, 0x2, 0x2, 0x2, 0x274, 0x3b, 0x3, 0x2, 0x2, 0x2, 0x275, 0x276, 
       0x7, 0x44, 0x2, 0x2, 0x276, 0x277, 0x7, 0x85, 0x2, 0x2, 0x277, 0x278, 
       0x7, 0x45, 0x2, 0x2, 0x278, 0x279, 0x7, 0x85, 0x2, 0x2, 0x279, 0x281, 
       0x5, 0x42, 0x22, 0x2, 0x27a, 0x27c, 0x7, 0x4, 0x2, 0x2, 0x27b, 0x27d, 
       0x7, 0x85, 0x2, 0x2, 0x27c, 0x27b, 0x3, 0x2, 0x2, 0x2, 0x27c, 0x27d, 
       0x3, 0x2, 0x2, 0x2, 0x27d, 0x27e, 0x3, 0x2, 0x2, 0x2, 0x27e, 0x280, 
       0x5, 0x42, 0x22, 0x2, 0x27f, 0x27a, 0x3, 0x2, 0x2, 0x2, 0x280, 0x283, 
       0x3, 0x2, 0x2, 0x2, 0x281, 0x27f, 0x3, 0x2, 0x2, 0x2, 0x281, 0x282, 
       0x3, 0x2, 0x2, 0x2, 0x282, 0x3d, 0x3, 0x2, 0x2, 0x2, 0x283, 0x281, 
       0x3, 0x2, 0x2, 0x2, 0x284, 0x285, 0x7, 0x46, 0x2, 0x2, 0x285, 0x286, 
       0x7, 0x85, 0x2, 0x2, 0x286, 0x287, 0x5, 0x66, 0x34, 0x2, 0x287, 0x3f, 
       0x3, 0x2, 0x2, 0x2, 0x288, 0x289, 0x7, 0x47, 0x2, 0x2, 0x289, 0x28a, 
       0x7, 0x85, 0x2, 0x2, 0x28a, 0x28b, 0x5, 0x66, 0x34, 0x2, 0x28b, 0x41, 
       0x3, 0x2, 0x2, 0x2, 0x28c, 0x291, 0x5, 0x66, 0x34, 0x2, 0x28d, 0x28f, 
       0x7, 0x85, 0x2, 0x2, 0x28e, 0x28d, 0x3, 0x2, 0x2, 0x2, 0x28e, 0x28f, 
       0x3, 0x2, 0x2, 0x2, 0x28f, 0x290, 0x3, 0x2, 0x2, 0x2, 0x290, 0x292, 
       0x9, 0x2, 0x2, 0x2, 0x291, 0x28e, 0x3, 0x2, 0x2, 0x2, 0x291, 0x292, 
       0x3, 0x2, 0x2, 0x2, 0x292, 0x43, 0x3, 0x2, 0x2, 0x2, 0x293, 0x294, 
       0x7, 0x4c, 0x2, 0x2, 0x294, 0x295, 0x7, 0x85, 0x2, 0x2, 0x295, 0x296, 
       0x7, 0x4d, 0x2, 0x2, 0x296, 0x297, 0x7, 0x85, 0x2, 0x2, 0x297, 0x298, 
       0x7, 0x39, 0x2, 0x2, 0x298, 0x299, 0x7, 0x85, 0x2, 0x2, 0x299, 0x2a2, 
       0x5, 0xae, 0x58, 0x2, 0x29a, 0x29b, 0x7, 0x4c, 0x2, 0x2, 0x29b, 0x29c, 
       0x7, 0x85, 0x2, 0x2, 0x29c, 0x29d, 0x7, 0x4e, 0x2, 0x2, 0x29d, 0x29e, 
       0x7, 0x85, 0x2, 0x2, 0x29e, 0x29f, 0x7, 0x39, 0x2, 0x2, 0x29f, 0x2a0, 
       0x7, 0x85, 0x2, 0x2, 0x2a0, 0x2a2, 0x5, 0xae, 0x58, 0x2, 0x2a1, 0x293, 
       0x3, 0x2, 0x2, 0x2, 0x2a1, 0x29a, 0x3, 0x2, 0x2, 0x2, 0x2a2, 0x45, 
       0x3, 0x2, 0x2, 0x2, 0x2a3, 0x2a4, 0x7, 0x4f, 0x2, 0x2, 0x2a4, 0x2a5, 
       0x7, 0x85, 0x2, 0x2, 0x2a5, 0x2a6, 0x5, 0x66, 0x34, 0x2, 0x2a6, 0x47, 
       0x3, 0x2, 0x2, 0x2, 0x2a7, 0x2b2, 0x5, 0x4a, 0x26, 0x2, 0x2a8, 0x2aa, 
       0x7, 0x85, 0x2, 0x2, 0x2a9, 0x2a8, 0x3, 0x2, 0x2, 0x2, 0x2a9, 0x2aa, 
       0x3, 0x2, 0x2, 0x2, 0x2aa, 0x2ab, 0x3, 0x2, 0x2, 0x2, 0x2ab, 0x2ad, 
       0x7, 0x4, 0x2, 0x2, 0x2ac, 0x2ae, 0x7, 0x85, 0x2, 0x2, 0x2ad, 0x2ac, 
       0x3, 0x2, 0x2, 0x2, 0x2ad, 0x2ae, 0x3, 0x2, 0x2, 0x2, 0x2ae, 0x2af, 
       0x3, 0x2, 0x2, 0x2, 0x2af, 0x2b1, 0x5, 0x4a, 0x26, 0x2, 0x2b0, 0x2a9, 
       0x3, 0x2, 0x2, 0x2, 0x2b1, 0x2b4, 0x3, 0x2, 0x2, 0x2, 0x2b2, 0x2b0, 
       0x3, 0x2, 0x2, 0x2, 0x2b2, 0x2b3, 0x3, 0x2, 0x2, 0x2, 0x2b3, 0x49, 
       0x3, 0x2, 0x2, 0x2, 0x2b4, 0x2b2, 0x3, 0x2, 0x2, 0x2, 0x2b5, 0x2b7, 
       0x5, 0xae, 0x58, 0x2, 0x2b6, 0x2b8, 0x7, 0x85, 0x2, 0x2, 0x2b7, 0x2b6, 
       0x3, 0x2, 0x2, 0x2, 0x2b7, 0x2b8, 0x3, 0x2, 0x2, 0x2, 0x2b8, 0x2b9, 
       0x3, 0x2, 0x2, 0x2, 0x2b9, 0x2bb, 0x7, 0x5, 0x2, 0x2, 0x2ba, 0x2bc, 
       0x7, 0x85, 0x2, 0x2, 0x2bb, 0x2ba, 0x3, 0x2, 0x2, 0x2, 0x2bb, 0x2bc, 
       0x3, 0x2, 0x2, 0x2, 0x2bc, 0x2bd, 0x3, 0x2, 0x2, 0x2, 0x2bd, 0x2be, 
       0x5, 0x4c, 0x27, 0x2, 0x2be, 0x2c1, 0x3, 0x2, 0x2, 0x2, 0x2bf, 0x2c1, 
       0x5, 0x4c, 0x27, 0x2, 0x2c0, 0x2b5, 0x3, 0x2, 0x2, 0x2, 0x2c0, 0x2bf, 
       0x3, 0x2, 0x2, 0x2, 0x2c1, 0x4b, 0x3, 0x2, 0x2, 0x2, 0x2c2, 0x2c3, 
       0x5, 0x4e, 0x28, 0x2, 0x2c3, 0x4d, 0x3, 0x2, 0x2, 0x2, 0x2c4, 0x2cb, 
       0x5, 0x50, 0x29, 0x2, 0x2c5, 0x2c7, 0x7, 0x85, 0x2, 0x2, 0x2c6, 0x2c5, 
       0x3, 0x2, 0x2, 0x2, 0x2c6, 0x2c7, 0x3, 0x2, 0x2, 0x2, 0x2c7, 0x2c8, 
       0x3, 0x2, 0x2, 0x2, 0x2c8, 0x2ca, 0x5, 0x52, 0x2a, 0x2, 0x2c9, 0x2c6, 
       0x3, 0x2, 0x2, 0x2, 0x2ca, 0x2cd, 0x3, 0x2, 0x2, 0x2, 0x2cb, 0x2c9, 
       0x3, 0x2, 0x2, 0x2, 0x2cb, 0x2cc, 0x3, 0x2, 0x2, 0x2, 0x2cc, 0x2d3, 
       0x3, 0x2, 0x2, 0x2, 0x2cd, 0x2cb, 0x3, 0x2, 0x2, 0x2, 0x2ce, 0x2cf, 
       0x7, 0x8, 0x2, 0x2, 0x2cf, 0x2d0, 0x5, 0x4e, 0x28, 0x2, 0x2d0, 0x2d1, 
       0x7, 0x9, 0x2, 0x2, 0x2d1, 0x2d3, 0x3, 0x2, 0x2, 0x2, 0x2d2, 0x2c4, 
       0x3, 0x2, 0x2, 0x2, 0x2d2, 0x2ce, 0x3, 0x2, 0x2, 0x2, 0x2d3, 0x4f, 
       0x3, 0x2, 0x2, 0x2, 0x2d4, 0x2d6, 0x7, 0x8, 0x2, 0x2, 0x2d5, 0x2d7, 
       0x7, 0x85, 0x2, 0x2, 0x2d6, 0x2d5, 0x3, 0x2, 0x2, 0x2, 0x2d6, 0x2d7, 
       0x3, 0x2, 0x2, 0x2, 0x2d7, 0x2dc, 0x3, 0x2, 0x2, 0x2, 0x2d8, 0x2da, 
       0x5, 0xae, 0x58, 0x2, 0x2d9, 0x2db, 0x7, 0x85, 0x2, 0x2, 0x2da, 0x2d9, 
       0x3, 0x2, 0x2, 0x2, 0x2da, 0x2db, 0x3, 0x2, 0x2, 0x2, 0x2db, 0x2dd, 
       0x3, 0x2, 0x2, 0x2, 0x2dc, 0x2d8, 0x3, 0x2, 0x2, 0x2, 0x2dc, 0x2dd, 
       0x3, 0x2, 0x2, 0x2, 0x2dd, 0x2e2, 0x3, 0x2, 0x2, 0x2, 0x2de, 0x2e0, 
       0x5, 0x5c, 0x2f, 0x2, 0x2df, 0x2e1, 0x7, 0x85, 0x2, 0x2, 0x2e0, 0x2df, 
       0x3, 0x2, 0x2, 0x2, 0x2e0, 0x2e1, 0x3, 0x2, 0x2, 0x2, 0x2e1, 0x2e3, 
       0x3, 0x2, 0x2, 0x2, 0x2e2, 0x2de, 0x3, 0x2, 0x2, 0x2, 0x2e2, 0x2e3, 
       0x3, 0x2, 0x2, 0x2, 0x2e3, 0x2e8, 0x3, 0x2, 0x2, 0x2, 0x2e4, 0x2e6, 
       0x5, 0x58, 0x2d, 0x2, 0x2e5, 0x2e7, 0x7, 0x85, 0x2, 0x2, 0x2e6, 0x2e5, 
       0x3, 0x2, 0x2, 0x2, 0x2e6, 0x2e7, 0x3, 0x2, 0x2, 0x2, 0x2e7, 0x2e9, 
       0x3, 0x2, 0x2, 0x2, 0x2e8, 0x2e4, 0x3, 0x2, 0x2, 0x2, 0x2e8, 0x2e9, 
       0x3, 0x2, 0x2, 0x2, 0x2e9, 0x2ea, 0x3, 0x2, 0x2, 0x2, 0x2ea, 0x2eb, 
       0x7, 0x9, 0x2, 0x2, 0x2eb, 0x51, 0x3, 0x2, 0x2, 0x2, 0x2ec, 0x2ee, 
       0x5, 0x54, 0x2b, 0x2, 0x2ed, 0x2ef, 0x7, 0x85, 0x2, 0x2, 0x2ee, 0x2ed, 
       0x3, 0x2, 0x2, 0x2, 0x2ee, 0x2ef, 0x3, 0x2, 0x2, 0x2, 0x2ef, 0x2f0, 
       0x3, 0x2, 0x2, 0x2, 0x2f0, 0x2f1, 0x5, 0x50, 0x29, 0x2, 0x2f1, 0x53, 
       0x3, 0x2, 0x2, 0x2, 0x2f2, 0x2f4, 0x5, 0xc4, 0x63, 0x2, 0x2f3, 0x2f5, 
       0x7, 0x85, 0x2, 0x2, 0x2f4, 0x2f3, 0x3, 0x2, 0x2, 0x2, 0x2f4, 0x2f5, 
       0x3, 0x2, 0x2, 0x2, 0x2f5, 0x2f6, 0x3, 0x2, 0x2, 0x2, 0x2f6, 0x2f8, 
       0x5, 0xc8, 0x65, 0x2, 0x2f7, 0x2f9, 0x7, 0x85, 0x2, 0x2, 0x2f8, 0x2f7, 
       0x3, 0x2, 0x2, 0x2, 0x2f8, 0x2f9, 0x3, 0x2, 0x2, 0x2, 0x2f9, 0x2fb, 
       0x3, 0x2, 0x2, 0x2, 0x2fa, 0x2fc, 0x5, 0x56, 0x2c, 0x2, 0x2fb, 0x2fa, 
       0x3, 0x2, 0x2, 0x2, 0x2fb, 0x2fc, 0x3, 0x2, 0x2, 0x2, 0x2fc, 0x2fe, 
       0x3, 0x2, 0x2, 0x2, 0x2fd, 0x2ff, 0x7, 0x85, 0x2, 0x2, 0x2fe, 0x2fd, 
       0x3, 0x2, 0x2, 0x2, 0x2fe, 0x2ff, 0x3, 0x2, 0x2, 0x2, 0x2ff, 0x300, 
       0x3, 0x2, 0x2, 0x2, 0x300, 0x302, 0x5, 0xc8, 0x65, 0x2, 0x301, 0x303, 
       0x7, 0x85, 0x2, 0x2, 0x302, 0x301, 0x3, 0x2, 0x2, 0x2, 0x302, 0x303, 
       0x3, 0x2, 0x2, 0x2, 0x303, 0x304, 0x3, 0x2, 0x2, 0x2, 0x304, 0x305, 
       0x5, 0xc6, 0x64, 0x2, 0x305, 0x333, 0x3, 0x2, 0x2, 0x2, 0x306, 0x308, 
       0x5, 0xc4, 0x63, 0x2, 0x307, 0x309, 0x7, 0x85, 0x2, 0x2, 0x308, 0x307, 
       0x3, 0x2, 0x2, 0x2, 0x308, 0x309, 0x3, 0x2, 0x2, 0x2, 0x309, 0x30a, 
       0x3, 0x2, 0x2, 0x2, 0x30a, 0x30c, 0x5, 0xc8, 0x65, 0x2, 0x30b, 0x30d, 
       0x7, 0x85, 0x2, 0x2, 0x30c, 0x30b, 0x3, 0x2, 0x2, 0x2, 0x30c, 0x30d, 
       0x3, 0x2, 0x2, 0x2, 0x30d, 0x30f, 0x3, 0x2, 0x2, 0x2, 0x30e, 0x310, 
       0x5, 0x56, 0x2c, 0x2, 0x30f, 0x30e, 0x3, 0x2, 0x2, 0x2, 0x30f, 0x310, 
       0x3, 0x2, 0x2, 0x2, 0x310, 0x312, 0x3, 0x2, 0x2, 0x2, 0x311, 0x313, 
       0x7, 0x85, 0x2, 0x2, 0x312, 0x311, 0x3, 0x2, 0x2, 0x2, 0x312, 0x313, 
       0x3, 0x2, 0x2, 0x2, 0x313, 0x314, 0x3, 0x2, 0x2, 0x2, 0x314, 0x315, 
       0x5, 0xc8, 0x65, 0x2, 0x315, 0x333, 0x3, 0x2, 0x2, 0x2, 0x316, 0x318, 
       0x5, 0xc8, 0x65, 0x2, 0x317, 0x319, 0x7, 0x85, 0x2, 0x2, 0x318, 0x317, 
       0x3, 0x2, 0x2, 0x2, 0x318, 0x319, 0x3, 0x2, 0x2, 0x2, 0x319, 0x31b, 
       0x3, 0x2, 0x2, 0x2, 0x31a, 0x31c, 0x5, 0x56, 0x2c, 0x2, 0x31b, 0x31a, 
       0x3, 0x2, 0x2, 0x2, 0x31b, 0x31c, 0x3, 0x2, 0x2, 0x2, 0x31c, 0x31e, 
       0x3, 0x2, 0x2, 0x2, 0x31d, 0x31f, 0x7, 0x85, 0x2, 0x2, 0x31e, 0x31d, 
       0x3, 0x2, 0x2, 0x2, 0x31e, 0x31f, 0x3, 0x2, 0x2, 0x2, 0x31f, 0x320, 
       0x3, 0x2, 0x2, 0x2, 0x320, 0x322, 0x5, 0xc8, 0x65, 0x2, 0x321, 0x323, 
       0x7, 0x85, 0x2, 0x2, 0x322, 0x321, 0x3, 0x2, 0x2, 0x2, 0x322, 0x323, 
       0x3, 0x2, 0x2, 0x2, 0x323, 0x324, 0x3, 0x2, 0x2, 0x2, 0x324, 0x325, 
       0x5, 0xc6, 0x64, 0x2, 0x325, 0x333, 0x3, 0x2, 0x2, 0x2, 0x326, 0x328, 
       0x5, 0xc8, 0x65, 0x2, 0x327, 0x329, 0x7, 0x85, 0x2, 0x2, 0x328, 0x327, 
       0x3, 0x2, 0x2, 0x2, 0x328, 0x329, 0x3, 0x2, 0x2, 0x2, 0x329, 0x32b, 
       0x3, 0x2, 0x2, 0x2, 0x32a, 0x32c, 0x5, 0x56, 0x2c, 0x2, 0x32b, 0x32a, 
       0x3, 0x2, 0x2, 0x2, 0x32b, 0x32c, 0x3, 0x2, 0x2, 0x2, 0x32c, 0x32e, 
       0x3, 0x2, 0x2, 0x2, 0x32d, 0x32f, 0x7, 0x85, 0x2, 0x2, 0x32e, 0x32d, 
       0x3, 0x2, 0x2, 0x2, 0x32e, 0x32f, 0x3, 0x2, 0x2, 0x2, 0x32f, 0x330, 
       0x3, 0x2, 0x2, 0x2, 0x330, 0x331, 0x5, 0xc8, 0x65, 0x2, 0x331, 0x333, 
       0x3, 0x2, 0x2, 0x2, 0x332, 0x2f2, 0x3, 0x2, 0x2, 0x2, 0x332, 0x306, 
       0x3, 0x2, 0x2, 0x2, 0x332, 0x316, 0x3, 0x2, 0x2, 0x2, 0x332, 0x326, 
       0x3, 0x2, 0x2, 0x2, 0x333, 0x55, 0x3, 0x2, 0x2, 0x2, 0x334, 0x336, 
       0x7, 0xa, 0x2, 0x2, 0x335, 0x337, 0x7, 0x85, 0x2, 0x2, 0x336, 0x335, 
       0x3, 0x2, 0x2, 0x2, 0x336, 0x337, 0x3, 0x2, 0x2, 0x2, 0x337, 0x33c, 
       0x3, 0x2, 0x2, 0x2, 0x338, 0x33a, 0x5, 0xae, 0x58, 0x2, 0x339, 0x33b, 
       0x7, 0x85, 0x2, 0x2, 0x33a, 0x339, 0x3, 0x2, 0x2, 0x2, 0x33a, 0x33b, 
       0x3, 0x2, 0x2, 0x2, 0x33b, 0x33d, 0x3, 0x2, 0x2, 0x2, 0x33c, 0x338, 
       0x3, 0x2, 0x2, 0x2, 0x33c, 0x33d, 0x3, 0x2, 0x2, 0x2, 0x33d, 0x342, 
       0x3, 0x2, 0x2, 0x2, 0x33e, 0x340, 0x5, 0x5a, 0x2e, 0x2, 0x33f, 0x341, 
       0x7, 0x85, 0x2, 0x2, 0x340, 0x33f, 0x3, 0x2, 0x2, 0x2, 0x340, 0x341, 
       0x3, 0x2, 0x2, 0x2, 0x341, 0x343, 0x3, 0x2, 0x2, 0x2, 0x342, 0x33e, 
       0x3, 0x2, 0x2, 0x2, 0x342, 0x343, 0x3, 0x2, 0x2, 0x2, 0x343, 0x345, 
       0x3, 0x2, 0x2, 0x2, 0x344, 0x346, 0x5, 0x60, 0x31, 0x2, 0x345, 0x344, 
       0x3, 0x2, 0x2, 0x2, 0x345, 0x346, 0x3, 0x2, 0x2, 0x2, 0x346, 0x34b, 
       0x3, 0x2, 0x2, 0x2, 0x347, 0x349, 0x5, 0x58, 0x2d, 0x2, 0x348, 0x34a, 
       0x7, 0x85, 0x2, 0x2, 0x349, 0x348, 0x3, 0x2, 0x2, 0x2, 0x349, 0x34a, 
       0x3, 0x2, 0x2, 0x2, 0x34a, 0x34c, 0x3, 0x2, 0x2, 0x2, 0x34b, 0x347, 
       0x3, 0x2, 0x2, 0x2, 0x34b, 0x34c, 0x3, 0x2, 0x2, 0x2, 0x34c, 0x34d, 
       0x3, 0x2, 0x2, 0x2, 0x34d, 0x34e, 0x7, 0xb, 0x2, 0x2, 0x34e, 0x57, 
       0x3, 0x2, 0x2, 0x2, 0x34f, 0x352, 0x5, 0xb2, 0x5a, 0x2, 0x350, 0x352, 
       0x5, 0xb4, 0x5b, 0x2, 0x351, 0x34f, 0x3, 0x2, 0x2, 0x2, 0x351, 0x350, 
       0x3, 0x2, 0x2, 0x2, 0x352, 0x59, 0x3, 0x2, 0x2, 0x2, 0x353, 0x355, 
       0x7, 0xc, 0x2, 0x2, 0x354, 0x356, 0x7, 0x85, 0x2, 0x2, 0x355, 0x354, 
       0x3, 0x2, 0x2, 0x2, 0x355, 0x356, 0x3, 0x2, 0x2, 0x2, 0x356, 0x357, 
       0x3, 0x2, 0x2, 0x2, 0x357, 0x365, 0x5, 0x64, 0x33, 0x2, 0x358, 0x35a, 
       0x7, 0x85, 0x2, 0x2, 0x359, 0x358, 0x3, 0x2, 0x2, 0x2, 0x359, 0x35a, 
       0x3, 0x2, 0x2, 0x2, 0x35a, 0x35b, 0x3, 0x2, 0x2, 0x2, 0x35b, 0x35d, 
       0x7, 0xd, 0x2, 0x2, 0x35c, 0x35e, 0x7, 0xc, 0x2, 0x2, 0x35d, 0x35c, 
       0x3, 0x2, 0x2, 0x2, 0x35d, 0x35e, 0x3, 0x2, 0x2, 0x2, 0x35e, 0x360, 
       0x3, 0x2, 0x2, 0x2, 0x35f, 0x361, 0x7, 0x85, 0x2, 0x2, 0x360, 0x35f, 
       0x3, 0x2, 0x2, 0x2, 0x360, 0x361, 0x3, 0x2, 0x2, 0x2, 0x361, 0x362, 
       0x3, 0x2, 0x2, 0x2, 0x362, 0x364, 0x5, 0x64, 0x33, 0x2, 0x363, 0x359, 
       0x3, 0x2, 0x2, 0x2, 0x364, 0x367, 0x3, 0x2, 0x2, 0x2, 0x365, 0x363, 
       0x3, 0x2, 0x2, 0x2, 0x365, 0x366, 0x3, 0x2, 0x2, 0x2, 0x366, 0x5b, 
       0x3, 0x2, 0x2, 0x2, 0x367, 0x365, 0x3, 0x2, 0x2, 0x2, 0x368, 0x36f, 
       0x5, 0x5e, 0x30, 0x2, 0x369, 0x36b, 0x7, 0x85, 0x2, 0x2, 0x36a, 0x369, 
       0x3, 0x2, 0x2, 0x2, 0x36a, 0x36b, 0x3, 0x2, 0x2, 0x2, 0x36b, 0x36c, 
       0x3, 0x2, 0x2, 0x2, 0x36c, 0x36e, 0x5, 0x5e, 0x30, 0x2, 0x36d, 0x36a, 
       0x3, 0x2, 0x2, 0x2, 0x36e, 0x371, 0x3, 0x2, 0x2, 0x2, 0x36f, 0x36d, 
       0x3, 0x2, 0x2, 0x2, 0x36f, 0x370, 0x3, 0x2, 0x2, 0x2, 0x370, 0x5d, 
       0x3, 0x2, 0x2, 0x2, 0x371, 0x36f, 0x3, 0x2, 0x2, 0x2, 0x372, 0x374, 
       0x7, 0xc, 0x2, 0x2, 0x373, 0x375, 0x7, 0x85, 0x2, 0x2, 0x374, 0x373, 
       0x3, 0x2, 0x2, 0x2, 0x374, 0x375, 0x3, 0x2, 0x2, 0x2, 0x375, 0x376, 
       0x3, 0x2, 0x2, 0x2, 0x376, 0x377, 0x5, 0x62, 0x32, 0x2, 0x377, 0x5f, 
       0x3, 0x2, 0x2, 0x2, 0x378, 0x37a, 0x7, 0x7, 0x2, 0x2, 0x379, 0x37b, 
       0x7, 0x85, 0x2, 0x2, 0x37a, 0x379, 0x3, 0x2, 0x2, 0x2, 0x37a, 0x37b, 
       0x3, 0x2, 0x2, 0x2, 0x37b, 0x380, 0x3, 0x2, 0x2, 0x2, 0x37c, 0x37e, 
       0x5, 0xba, 0x5e, 0x2, 0x37d, 0x37f, 0x7, 0x85, 0x2, 0x2, 0x37e, 0x37d, 
       0x3, 0x2, 0x2, 0x2, 0x37e, 0x37f, 0x3, 0x2, 0x2, 0x2, 0x37f, 0x381, 
       0x3, 0x2, 0x2, 0x2, 0x380, 0x37c, 0x3, 0x2, 0x2, 0x2, 0x380, 0x381, 
       0x3, 0x2, 0x2, 0x2, 0x381, 0x38c, 0x3, 0x2, 0x2, 0x2, 0x382, 0x384, 
       0x7, 0xe, 0x2, 0x2, 0x383, 0x385, 0x7, 0x85, 0x2, 0x2, 0x384, 0x383, 
       0x3, 0x2, 0x2, 0x2, 0x384, 0x385, 0x3, 0x2, 0x2, 0x2, 0x385, 0x38a, 
       0x3, 0x2, 0x2, 0x2, 0x386, 0x388, 0x5, 0xba, 0x5e, 0x2, 0x387, 0x389, 
       0x7, 0x85, 0x2, 0x2, 0x388, 0x387, 0x3, 0x2, 0x2, 0x2, 0x388, 0x389, 
       0x3, 0x2, 0x2, 0x2, 0x389, 0x38b, 0x3, 0x2, 0x2, 0x2, 0x38a, 0x386, 
       0x3, 0x2, 0x2, 0x2, 0x38a, 0x38b, 0x3, 0x2, 0x2, 0x2, 0x38b, 0x38d, 
       0x3, 0x2, 0x2, 0x2, 0x38c, 0x382, 0x3, 0x2, 0x2, 0x2, 0x38c, 0x38d, 
       0x3, 0x2, 0x2, 0x2, 0x38d, 0x61, 0x3, 0x2, 0x2, 0x2, 0x38e, 0x38f, 
       0x5, 0xbe, 0x60, 0x2, 0x38f, 0x63, 0x3, 0x2, 0x2, 0x2, 0x390, 0x391, 
       0x5, 0xbe, 0x60, 0x2, 0x391, 0x65, 0x3, 0x2, 0x2, 0x2, 0x392, 0x393, 
       0x5, 0x68, 0x35, 0x2, 0x393, 0x67, 0x3, 0x2, 0x2, 0x2, 0x394, 0x39b, 
       0x5, 0x6a, 0x36, 0x2, 0x395, 0x396, 0x7, 0x85, 0x2, 0x2, 0x396, 0x397, 
       0x7, 0x50, 0x2, 0x2, 0x397, 0x398, 0x7, 0x85, 0x2, 0x2, 0x398, 0x39a, 
       0x5, 0x6a, 0x36, 0x2, 0x399, 0x395, 0x3, 0x2, 0x2, 0x2, 0x39a, 0x39d, 
       0x3, 0x2, 0x2, 0x2, 0x39b, 0x399, 0x3, 0x2, 0x2, 0x2, 0x39b, 0x39c, 
       0x3, 0x2, 0x2, 0x2, 0x39c, 0x69, 0x3, 0x2, 0x2, 0x2, 0x39d, 0x39b, 
       0x3, 0x2, 0x2, 0x2, 0x39e, 0x3a5, 0x5, 0x6c, 0x37, 0x2, 0x39f, 0x3a0, 
       0x7, 0x85, 0x2, 0x2, 0x3a0, 0x3a1, 0x7, 0x51, 0x2, 0x2, 0x3a1, 0x3a2, 
       0x7, 0x85, 0x2, 0x2, 0x3a2, 0x3a4, 0x5, 0x6c, 0x37, 0x2, 0x3a3, 0x39f, 
       0x3, 0x2, 0x2, 0x2, 0x3a4, 0x3a7, 0x3, 0x2, 0x2, 0x2, 0x3a5, 0x3a3, 
       0x3, 0x2, 0x2, 0x2, 0x3a5, 0x3a6, 0x3, 0x2, 0x2, 0x2, 0x3a6, 0x6b, 
       0x3, 0x2, 0x2, 0x2, 0x3a7, 0x3a5, 0x3, 0x2, 0x2, 0x2, 0x3a8, 0x3af, 
       0x5, 0x6e, 0x38, 0x2, 0x3a9, 0x3aa, 0x7, 0x85, 0x2, 0x2, 0x3aa, 0x3ab, 
       0x7, 0x52, 0x2, 0x2, 0x3ab, 0x3ac, 0x7, 0x85, 0x2, 0x2, 0x3ac, 0x3ae, 
       0x5, 0x6e, 0x38, 0x2, 0x3ad, 0x3a9, 0x3, 0x2, 0x2, 0x2, 0x3ae, 0x3b1, 
       0x3, 0x2, 0x2, 0x2, 0x3af, 0x3ad, 0x3, 0x2, 0x2, 0x2, 0x3af, 0x3b0, 
       0x3, 0x2, 0x2, 0x2, 0x3b0, 0x6d, 0x3, 0x2, 0x2, 0x2, 0x3b1, 0x3af, 
       0x3, 0x2, 0x2, 0x2, 0x3b2, 0x3b4, 0x7, 0x53, 0x2, 0x2, 0x3b3, 0x3b5, 
       0x7, 0x85, 0x2, 0x2, 0x3b4, 0x3b3, 0x3, 0x2, 0x2, 0x2, 0x3b4, 0x3b5, 
       0x3, 0x2, 0x2, 0x2, 0x3b5, 0x3b7, 0x3, 0x2, 0x2, 0x2, 0x3b6, 0x3b2, 
       0x3, 0x2, 0x2, 0x2, 0x3b7, 0x3ba, 0x3, 0x2, 0x2, 0x2, 0x3b8, 0x3b6, 
       0x3, 0x2, 0x2, 0x2, 0x3b8, 0x3b9, 0x3, 0x2, 0x2, 0x2, 0x3b9, 0x3bb, 
       0x3, 0x2, 0x2, 0x2, 0x3ba, 0x3b8, 0x3, 0x2, 0x2, 0x2, 0x3bb, 0x3bc, 
       0x5, 0x70, 0x39, 0x2, 0x3bc, 0x6f, 0x3, 0x2, 0x2, 0x2, 0x3bd, 0x3c4, 
       0x5, 0x72, 0x3a, 0x2, 0x3be, 0x3c0, 0x7, 0x85, 0x2, 0x2, 0x3bf, 0x3be, 
       0x3, 0x2, 0x2, 0x2, 0x3bf, 0x3c0, 0x3, 0x2, 0x2, 0x2, 0x3c0, 0x3c1, 
       0x3, 0x2, 0x2, 0x2, 0x3c1, 0x3c3, 0x5, 0x8c, 0x47, 0x2, 0x3c2, 0x3bf, 
       0x3, 0x2, 0x2, 0x2, 0x3c3, 0x3c6, 0x3, 0x2, 0x2, 0x2, 0x3c4, 0x3c2, 
       0x3, 0x2, 0x2, 0x2, 0x3c4, 0x3c5, 0x3, 0x2, 0x2, 0x2, 0x3c5, 0x71, 
       0x3, 0x2, 0x2, 0x2, 0x3c6, 0x3c4, 0x3, 0x2, 0x2, 0x2, 0x3c7, 0x3da, 
       0x5, 0x74, 0x3b, 0x2, 0x3c8, 0x3ca, 0x7, 0x85, 0x2, 0x2, 0x3c9, 0x3c8, 
       0x3, 0x2, 0x2, 0x2, 0x3c9, 0x3ca, 0x3, 0x2, 0x2, 0x2, 0x3ca, 0x3cb, 
       0x3, 0x2, 0x2, 0x2, 0x3cb, 0x3cd, 0x7, 0xf, 0x2, 0x2, 0x3cc, 0x3ce, 
       0x7, 0x85, 0x2, 0x2, 0x3cd, 0x3cc, 0x3, 0x2, 0x2, 0x2, 0x3cd, 0x3ce, 
       0x3, 0x2, 0x2, 0x2, 0x3ce, 0x3cf, 0x3, 0x2, 0x2, 0x2, 0x3cf, 0x3d9, 
       0x5, 0x74, 0x3b, 0x2, 0x3d0, 0x3d2, 0x7, 0x85, 0x2, 0x2, 0x3d1, 0x3d0, 
       0x3, 0x2, 0x2, 0x2, 0x3d1, 0x3d2, 0x3, 0x2, 0x2, 0x2, 0x3d2, 0x3d3, 
       0x3, 0x2, 0x2, 0x2, 0x3d3, 0x3d5, 0x7, 0x10, 0x2, 0x2, 0x3d4, 0x3d6, 
       0x7, 0x85, 0x2, 0x2, 0x3d5, 0x3d4, 0x3, 0x2, 0x2, 0x2, 0x3d5, 0x3d6, 
       0x3, 0x2, 0x2, 0x2, 0x3d6, 0x3d7, 0x3, 0x2, 0x2, 0x2, 0x3d7, 0x3d9, 
       0x5, 0x74, 0x3b, 0x2, 0x3d8, 0x3c9, 0x3, 0x2, 0x2, 0x2, 0x3d8, 0x3d1, 
       0x3, 0x2, 0x2, 0x2, 0x3d9, 0x3dc, 0x3, 0x2, 0x2, 0x2, 0x3da, 0x3d8, 
       0x3, 0x2, 0x2, 0x2, 0x3da, 0x3db, 0x3, 0x2, 0x2, 0x2, 0x3db, 0x73, 
       0x3, 0x2, 0x2, 0x2, 0x3dc, 0x3da, 0x3, 0x2, 0x2, 0x2, 0x3dd, 0x3f8, 
       0x5, 0x76, 0x3c, 0x2, 0x3de, 0x3e0, 0x7, 0x85, 0x2, 0x2, 0x3df, 0x3de, 
       0x3, 0x2, 0x2, 0x2, 0x3df, 0x3e0, 0x3, 0x2, 0x2, 0x2, 0x3e0, 0x3e1, 
       0x3, 0x2, 0x2, 0x2, 0x3e1, 0x3e3, 0x7, 0x7, 0x2, 0x2, 0x3e2, 0x3e4, 
       0x7, 0x85, 0x2, 0x2, 0x3e3, 0x3e2, 0x3, 0x2, 0x2, 0x2, 0x3e3, 0x3e4, 
       0x3, 0x2, 0x2, 0x2, 0x3e4, 0x3e5, 0x3, 0x2, 0x2, 0x2, 0x3e5, 0x3f7, 
       0x5, 0x76, 0x3c, 0x2, 0x3e6, 0x3e8, 0x7, 0x85, 0x2, 0x2, 0x3e7, 0x3e6, 
       0x3, 0x2, 0x2, 0x2, 0x3e7, 0x3e8, 0x3, 0x2, 0x2, 0x2, 0x3e8, 0x3e9, 
       0x3, 0x2, 0x2, 0x2, 0x3e9, 0x3eb, 0x7, 0x11, 0x2, 0x2, 0x3ea, 0x3ec, 
       0x7, 0x85, 0x2, 0x2, 0x3eb, 0x3ea, 0x3, 0x2, 0x2, 0x2, 0x3eb, 0x3ec, 
       0x3, 0x2, 0x2, 0x2, 0x3ec, 0x3ed, 0x3, 0x2, 0x2, 0x2, 0x3ed, 0x3f7, 
       0x5, 0x76, 0x3c, 0x2, 0x3ee, 0x3f0, 0x7, 0x85, 0x2, 0x2, 0x3ef, 0x3ee, 
       0x3, 0x2, 0x2, 0x2, 0x3ef, 0x3f0, 0x3, 0x2, 0x2, 0x2, 0x3f0, 0x3f1, 
       0x3, 0x2, 0x2, 0x2, 0x3f1, 0x3f3, 0x7, 0x12, 0x2, 0x2, 0x3f2, 0x3f4, 
       0x7, 0x85, 0x2, 0x2, 0x3f3, 0x3f2, 0x3, 0x2, 0x2, 0x2, 0x3f3, 0x3f4, 
       0x3, 0x2, 0x2, 0x2, 0x3f4, 0x3f5, 0x3, 0x2, 0x2, 0x2, 0x3f5, 0x3f7, 
       0x5, 0x76, 0x3c, 0x2, 0x3f6, 0x3df, 0x3, 0x2, 0x2, 0x2, 0x3f6, 0x3e7, 
       0x3, 0x2, 0x2, 0x2, 0x3f6, 0x3ef, 0x3, 0x2, 0x2, 0x2, 0x3f7, 0x3fa, 
       0x3, 0x2, 0x2, 0x2, 0x3f8, 0x3f6, 0x3, 0x2, 0x2, 0x2, 0x3f8, 0x3f9, 
       0x3, 0x2, 0x2, 0x2, 0x3f9, 0x75, 0x3, 0x2, 0x2, 0x2, 0x3fa, 0x3f8, 
       0x3, 0x2, 0x2, 0x2, 0x3fb, 0x406, 0x5, 0x78, 0x3d, 0x2, 0x3fc, 0x3fe, 
       0x7, 0x85, 0x2, 0x2, 0x3fd, 0x3fc, 0x3, 0x2, 0x2, 0x2, 0x3fd, 0x3fe, 
       0x3, 0x2, 0x2, 0x2, 0x3fe, 0x3ff, 0x3, 0x2, 0x2, 0x2, 0x3ff, 0x401, 
       0x7, 0x13, 0x2, 0x2, 0x400, 0x402, 0x7, 0x85, 0x2, 0x2, 0x401, 0x400, 
       0x3, 0x2, 0x2, 0x2, 0x401, 0x402, 0x3, 0x2, 0x2, 0x2, 0x402, 0x403, 
       0x3, 0x2, 0x2, 0x2, 0x403, 0x405, 0x5, 0x78, 0x3d, 0x2, 0x404, 0x3fd, 
       0x3, 0x2, 0x2, 0x2, 0x405, 0x408, 0x3, 0x2, 0x2, 0x2, 0x406, 0x404, 
       0x3, 0x2, 0x2, 0x2, 0x406, 0x407, 0x3, 0x2, 0x2, 0x2, 0x407, 0x77, 
       0x3, 0x2, 0x2, 0x2, 0x408, 0x406, 0x3, 0x2, 0x2, 0x2, 0x409, 0x40b, 
       0x9, 0x3, 0x2, 0x2, 0x40a, 0x40c, 0x7, 0x85, 0x2, 0x2, 0x40b, 0x40a, 
       0x3, 0x2, 0x2, 0x2, 0x40b, 0x40c, 0x3, 0x2, 0x2, 0x2, 0x40c, 0x40e, 
       0x3, 0x2, 0x2, 0x2, 0x40d, 0x409, 0x3, 0x2, 0x2, 0x2, 0x40e, 0x411, 
       0x3, 0x2, 0x2, 0x2, 0x40f, 0x40d, 0x3, 0x2, 0x2, 0x2, 0x40f, 0x410, 
       0x3, 0x2, 0x2, 0x2, 0x410, 0x412, 0x3, 0x2, 0x2, 0x2, 0x411, 0x40f, 
       0x3, 0x2, 0x2, 0x2, 0x412, 0x413, 0x5, 0x7a, 0x3e, 0x2, 0x413, 0x79, 
       0x3, 0x2, 0x2, 0x2, 0x414, 0x41a, 0x5, 0x82, 0x42, 0x2, 0x415, 0x419, 
       0x5, 0x7e, 0x40, 0x2, 0x416, 0x419, 0x5, 0x7c, 0x3f, 0x2, 0x417, 
       0x419, 0x5, 0x80, 0x41, 0x2, 0x418, 0x415, 0x3, 0x2, 0x2, 0x2, 0x418, 
       0x416, 0x3, 0x2, 0x2, 0x2, 0x418, 0x417, 0x3, 0x2, 0x2, 0x2, 0x419, 
       0x41c, 0x3, 0x2, 0x2, 0x2, 0x41a, 0x418, 0x3, 0x2, 0x2, 0x2, 0x41a, 
       0x41b, 0x3, 0x2, 0x2, 0x2, 0x41b, 0x7b, 0x3, 0x2, 0x2, 0x2, 0x41c, 
       0x41a, 0x3, 0x2, 0x2, 0x2, 0x41d, 0x41e, 0x7, 0x85, 0x2, 0x2, 0x41e, 
       0x420, 0x7, 0x54, 0x2, 0x2, 0x41f, 0x421, 0x7, 0x85, 0x2, 0x2, 0x420, 
       0x41f, 0x3, 0x2, 0x2, 0x2, 0x420, 0x421, 0x3, 0x2, 0x2, 0x2, 0x421, 
       0x422, 0x3, 0x2, 0x2, 0x2, 0x422, 0x437, 0x5, 0x82, 0x42, 0x2, 0x423, 
       0x425, 0x7, 0x85, 0x2, 0x2, 0x424, 0x423, 0x3, 0x2, 0x2, 0x2, 0x424, 
       0x425, 0x3, 0x2, 0x2, 0x2, 0x425, 0x426, 0x3, 0x2, 0x2, 0x2, 0x426, 
       0x427, 0x7, 0xa, 0x2, 0x2, 0x427, 0x428, 0x5, 0x66, 0x34, 0x2, 0x428, 
       0x429, 0x7, 0xb, 0x2, 0x2, 0x429, 0x437, 0x3, 0x2, 0x2, 0x2, 0x42a, 
       0x42c, 0x7, 0x85, 0x2, 0x2, 0x42b, 0x42a, 0x3, 0x2, 0x2, 0x2, 0x42b, 
       0x42c, 0x3, 0x2, 0x2, 0x2, 0x42c, 0x42d, 0x3, 0x2, 0x2, 0x2, 0x42d, 
       0x42f, 0x7, 0xa, 0x2, 0x2, 0x42e, 0x430, 0x5, 0x66, 0x34, 0x2, 0x42f, 
       0x42e, 0x3, 0x2, 0x2, 0x2, 0x42f, 0x430, 0x3, 0x2, 0x2, 0x2, 0x430, 
       0x431, 0x3, 0x2, 0x2, 0x2, 0x431, 0x433, 0x7, 0xe, 0x2, 0x2, 0x432, 
       0x434, 0x5, 0x66, 0x34, 0x2, 0x433, 0x432, 0x3, 0x2, 0x2, 0x2, 0x433, 
       0x434, 0x3, 0x2, 0x2, 0x2, 0x434, 0x435, 0x3, 0x2, 0x2, 0x2, 0x435, 
       0x437, 0x7, 0xb, 0x2, 0x2, 0x436, 0x41d, 0x3, 0x2, 0x2, 0x2, 0x436, 
       0x424, 0x3, 0x2, 0x2, 0x2, 0x436, 0x42b, 0x3, 0x2, 0x2, 0x2, 0x437, 
       0x7d, 0x3, 0x2, 0x2, 0x2, 0x438, 0x439, 0x7, 0x85, 0x2, 0x2, 0x439, 
       0x43a, 0x7, 0x55, 0x2, 0x2, 0x43a, 0x43b, 0x7, 0x85, 0x2, 0x2, 0x43b, 
       0x445, 0x7, 0x41, 0x2, 0x2, 0x43c, 0x43d, 0x7, 0x85, 0x2, 0x2, 0x43d, 
       0x43e, 0x7, 0x56, 0x2, 0x2, 0x43e, 0x43f, 0x7, 0x85, 0x2, 0x2, 0x43f, 
       0x445, 0x7, 0x41, 0x2, 0x2, 0x440, 0x441, 0x7, 0x85, 0x2, 0x2, 0x441, 
       0x445, 0x7, 0x57, 0x2, 0x2, 0x442, 0x443, 0x7, 0x85, 0x2, 0x2, 0x443, 
       0x445, 0x7, 0x58, 0x2, 0x2, 0x444, 0x438, 0x3, 0x2, 0x2, 0x2, 0x444, 
       0x43c, 0x3, 0x2, 0x2, 0x2, 0x444, 0x440, 0x3, 0x2, 0x2, 0x2, 0x444, 
       0x442, 0x3, 0x2, 0x2, 0x2, 0x445, 0x447, 0x3, 0x2, 0x2, 0x2, 0x446, 
       0x448, 0x7, 0x85, 0x2, 0x2, 0x447, 0x446, 0x3, 0x2, 0x2, 0x2, 0x447, 
       0x448, 0x3, 0x2, 0x2, 0x2, 0x448, 0x449, 0x3, 0x2, 0x2, 0x2, 0x449, 
       0x44a, 0x5, 0x82, 0x42, 0x2, 0x44a, 0x7f, 0x3, 0x2, 0x2, 0x2, 0x44b, 
       0x44c, 0x7, 0x85, 0x2, 0x2, 0x44c, 0x44d, 0x7, 0x59, 0x2, 0x2, 0x44d, 
       0x44e, 0x7, 0x85, 0x2, 0x2, 0x44e, 0x456, 0x7, 0x5a, 0x2, 0x2, 0x44f, 
       0x450, 0x7, 0x85, 0x2, 0x2, 0x450, 0x451, 0x7, 0x59, 0x2, 0x2, 0x451, 
       0x452, 0x7, 0x85, 0x2, 0x2, 0x452, 0x453, 0x7, 0x53, 0x2, 0x2, 0x453, 
       0x454, 0x7, 0x85, 0x2, 0x2, 0x454, 0x456, 0x7, 0x5a, 0x2, 0x2, 0x455, 
       0x44b, 0x3, 0x2, 0x2, 0x2, 0x455, 0x44f, 0x3, 0x2, 0x2, 0x2, 0x456, 
       0x81, 0x3, 0x2, 0x2, 0x2, 0x457, 0x45e, 0x5, 0x84, 0x43, 0x2, 0x458, 
       0x45a, 0x7, 0x85, 0x2, 0x2, 0x459, 0x458, 0x3, 0x2, 0x2, 0x2, 0x459, 
       0x45a, 0x3, 0x2, 0x2, 0x2, 0x45a, 0x45b, 0x3, 0x2, 0x2, 0x2, 0x45b, 
       0x45d, 0x5, 0xa8, 0x55, 0x2, 0x45c, 0x459, 0x3, 0x2, 0x2, 0x2, 0x45d, 
       0x460, 0x3, 0x2, 0x2, 0x2, 0x45e, 0x45c, 0x3, 0x2, 0x2, 0x2, 0x45e, 
       0x45f, 0x3, 0x2, 0x2, 0x2, 0x45f, 0x465, 0x3, 0x2, 0x2, 0x2, 0x460, 
       0x45e, 0x3, 0x2, 0x2, 0x2, 0x461, 0x463, 0x7, 0x85, 0x2, 0x2, 0x462, 
       0x461, 0x3, 0x2, 0x2, 0x2, 0x462, 0x463, 0x3, 0x2, 0x2, 0x2, 0x463, 
       0x464, 0x3, 0x2, 0x2, 0x2, 0x464, 0x466, 0x5, 0x5c, 0x2f, 0x2, 0x465, 
       0x462, 0x3, 0x2, 0x2, 0x2, 0x465, 0x466, 0x3, 0x2, 0x2, 0x2, 0x466, 
       0x83, 0x3, 0x2, 0x2, 0x2, 0x467, 0x4b6, 0x5, 0x86, 0x44, 0x2, 0x468, 
       0x4b6, 0x5, 0xb4, 0x5b, 0x2, 0x469, 0x4b6, 0x5, 0xaa, 0x56, 0x2, 
       0x46a, 0x46c, 0x7, 0x5b, 0x2, 0x2, 0x46b, 0x46d, 0x7, 0x85, 0x2, 
       0x2, 0x46c, 0x46b, 0x3, 0x2, 0x2, 0x2, 0x46c, 0x46d, 0x3, 0x2, 0x2, 
       0x2, 0x46d, 0x46e, 0x3, 0x2, 0x2, 0x2, 0x46e, 0x470, 0x7, 0x8, 0x2, 
       0x2, 0x46f, 0x471, 0x7, 0x85, 0x2, 0x2, 0x470, 0x46f, 0x3, 0x2, 0x2, 
       0x2, 0x470, 0x471, 0x3, 0x2, 0x2, 0x2, 0x471, 0x472, 0x3, 0x2, 0x2, 
       0x2, 0x472, 0x474, 0x7, 0x7, 0x2, 0x2, 0x473, 0x475, 0x7, 0x85, 0x2, 
       0x2, 0x474, 0x473, 0x3, 0x2, 0x2, 0x2, 0x474, 0x475, 0x3, 0x2, 0x2, 
       0x2, 0x475, 0x476, 0x3, 0x2, 0x2, 0x2, 0x476, 0x4b6, 0x7, 0x9, 0x2, 
       0x2, 0x477, 0x4b6, 0x5, 0xa4, 0x53, 0x2, 0x478, 0x4b6, 0x5, 0xa6, 
       0x54, 0x2, 0x479, 0x47b, 0x7, 0x33, 0x2, 0x2, 0x47a, 0x47c, 0x7, 
       0x85, 0x2, 0x2, 0x47b, 0x47a, 0x3, 0x2, 0x2, 0x2, 0x47b, 0x47c, 0x3, 
       0x2, 0x2, 0x2, 0x47c, 0x47d, 0x3, 0x2, 0x2, 0x2, 0x47d, 0x47f, 0x7, 
       0x8, 0x2, 0x2, 0x47e, 0x480, 0x7, 0x85, 0x2, 0x2, 0x47f, 0x47e, 0x3, 
       0x2, 0x2, 0x2, 0x47f, 0x480, 0x3, 0x2, 0x2, 0x2, 0x480, 0x481, 0x3, 
       0x2, 0x2, 0x2, 0x481, 0x483, 0x5, 0x92, 0x4a, 0x2, 0x482, 0x484, 
       0x7, 0x85, 0x2, 0x2, 0x483, 0x482, 0x3, 0x2, 0x2, 0x2, 0x483, 0x484, 
       0x3, 0x2, 0x2, 0x2, 0x484, 0x485, 0x3, 0x2, 0x2, 0x2, 0x485, 0x486, 
       0x7, 0x9, 0x2, 0x2, 0x486, 0x4b6, 0x3, 0x2, 0x2, 0x2, 0x487, 0x489, 
       0x7, 0x5c, 0x2, 0x2, 0x488, 0x48a, 0x7, 0x85, 0x2, 0x2, 0x489, 0x488, 
       0x3, 0x2, 0x2, 0x2, 0x489, 0x48a, 0x3, 0x2, 0x2, 0x2, 0x48a, 0x48b, 
       0x3, 0x2, 0x2, 0x2, 0x48b, 0x48d, 0x7, 0x8, 0x2, 0x2, 0x48c, 0x48e, 
       0x7, 0x85, 0x2, 0x2, 0x48d, 0x48c, 0x3, 0x2, 0x2, 0x2, 0x48d, 0x48e, 
       0x3, 0x2, 0x2, 0x2, 0x48e, 0x48f, 0x3, 0x2, 0x2, 0x2, 0x48f, 0x491, 
       0x5, 0x92, 0x4a, 0x2, 0x490, 0x492, 0x7, 0x85, 0x2, 0x2, 0x491, 0x490, 
       0x3, 0x2, 0x2, 0x2, 0x491, 0x492, 0x3, 0x2, 0x2, 0x2, 0x492, 0x493, 
       0x3, 0x2, 0x2, 0x2, 0x493, 0x494, 0x7, 0x9, 0x2, 0x2, 0x494, 0x4b6, 
       0x3, 0x2, 0x2, 0x2, 0x495, 0x497, 0x7, 0x5d, 0x2, 0x2, 0x496, 0x498, 
       0x7, 0x85, 0x2, 0x2, 0x497, 0x496, 0x3, 0x2, 0x2, 0x2, 0x497, 0x498, 
       0x3, 0x2, 0x2, 0x2, 0x498, 0x499, 0x3, 0x2, 0x2, 0x2, 0x499, 0x49b, 
       0x7, 0x8, 0x2, 0x2, 0x49a, 0x49c, 0x7, 0x85, 0x2, 0x2, 0x49b, 0x49a, 
       0x3, 0x2, 0x2, 0x2, 0x49b, 0x49c, 0x3, 0x2, 0x2, 0x2, 0x49c, 0x49d, 
       0x3, 0x2, 0x2, 0x2, 0x49d, 0x49f, 0x5, 0x92, 0x4a, 0x2, 0x49e, 0x4a0, 
       0x7, 0x85, 0x2, 0x2, 0x49f, 0x49e, 0x3, 0x2, 0x2, 0x2, 0x49f, 0x4a0, 
       0x3, 0x2, 0x2, 0x2, 0x4a0, 0x4a1, 0x3, 0x2, 0x2, 0x2, 0x4a1, 0x4a2, 
       0x7, 0x9, 0x2, 0x2, 0x4a2, 0x4b6, 0x3, 0x2, 0x2, 0x2, 0x4a3, 0x4a5, 
       0x7, 0x5e, 0x2, 0x2, 0x4a4, 0x4a6, 0x7, 0x85, 0x2, 0x2, 0x4a5, 0x4a4, 
       0x3, 0x2, 0x2, 0x2, 0x4a5, 0x4a6, 0x3, 0x2, 0x2, 0x2, 0x4a6, 0x4a7, 
       0x3, 0x2, 0x2, 0x2, 0x4a7, 0x4a9, 0x7, 0x8, 0x2, 0x2, 0x4a8, 0x4aa, 
       0x7, 0x85, 0x2, 0x2, 0x4a9, 0x4a8, 0x3, 0x2, 0x2, 0x2, 0x4a9, 0x4aa, 
       0x3, 0x2, 0x2, 0x2, 0x4aa, 0x4ab, 0x3, 0x2, 0x2, 0x2, 0x4ab, 0x4ad, 
       0x5, 0x92, 0x4a, 0x2, 0x4ac, 0x4ae, 0x7, 0x85, 0x2, 0x2, 0x4ad, 0x4ac, 
       0x3, 0x2, 0x2, 0x2, 0x4ad, 0x4ae, 0x3, 0x2, 0x2, 0x2, 0x4ae, 0x4af, 
       0x3, 0x2, 0x2, 0x2, 0x4af, 0x4b0, 0x7, 0x9, 0x2, 0x2, 0x4b0, 0x4b6, 
       0x3, 0x2, 0x2, 0x2, 0x4b1, 0x4b6, 0x5, 0x90, 0x49, 0x2, 0x4b2, 0x4b6, 
       0x5, 0x8e, 0x48, 0x2, 0x4b3, 0x4b6, 0x5, 0x96, 0x4c, 0x2, 0x4b4, 
       0x4b6, 0x5, 0xae, 0x58, 0x2, 0x4b5, 0x467, 0x3, 0x2, 0x2, 0x2, 0x4b5, 
       0x468, 0x3, 0x2, 0x2, 0x2, 0x4b5, 0x469, 0x3, 0x2, 0x2, 0x2, 0x4b5, 
       0x46a, 0x3, 0x2, 0x2, 0x2, 0x4b5, 0x477, 0x3, 0x2, 0x2, 0x2, 0x4b5, 
       0x478, 0x3, 0x2, 0x2, 0x2, 0x4b5, 0x479, 0x3, 0x2, 0x2, 0x2, 0x4b5, 
       0x487, 0x3, 0x2, 0x2, 0x2, 0x4b5, 0x495, 0x3, 0x2, 0x2, 0x2, 0x4b5, 
       0x4a3, 0x3, 0x2, 0x2, 0x2, 0x4b5, 0x4b1, 0x3, 0x2, 0x2, 0x2, 0x4b5, 
       0x4b2, 0x3, 0x2, 0x2, 0x2, 0x4b5, 0x4b3, 0x3, 0x2, 0x2, 0x2, 0x4b5, 
       0x4b4, 0x3, 0x2, 0x2, 0x2, 0x4b6, 0x85, 0x3, 0x2, 0x2, 0x2, 0x4b7, 
       0x4be, 0x5, 0xb0, 0x59, 0x2, 0x4b8, 0x4be, 0x7, 0x67, 0x2, 0x2, 0x4b9, 
       0x4be, 0x5, 0x88, 0x45, 0x2, 0x4ba, 0x4be, 0x7, 0x5a, 0x2, 0x2, 0x4bb, 
       0x4be, 0x5, 0xb2, 0x5a, 0x2, 0x4bc, 0x4be, 0x5, 0x8a, 0x46, 0x2, 
       0x4bd, 0x4b7, 0x3, 0x2, 0x2, 0x2, 0x4bd, 0x4b8, 0x3, 0x2, 0x2, 0x2, 
       0x4bd, 0x4b9, 0x3, 0x2, 0x2, 0x2, 0x4bd, 0x4ba, 0x3, 0x2, 0x2, 0x2, 
       0x4bd, 0x4bb, 0x3, 0x2, 0x2, 0x2, 0x4bd, 0x4bc, 0x3, 0x2, 0x2, 0x2, 
       0x4be, 0x87, 0x3, 0x2, 0x2, 0x2, 0x4bf, 0x4c0, 0x9, 0x4, 0x2, 0x2, 
       0x4c0, 0x89, 0x3, 0x2, 0x2, 0x2, 0x4c1, 0x4c3, 0x7, 0xa, 0x2, 0x2, 
       0x4c2, 0x4c4, 0x7, 0x85, 0x2, 0x2, 0x4c3, 0x4c2, 0x3, 0x2, 0x2, 0x2, 
       0x4c3, 0x4c4, 0x3, 0x2, 0x2, 0x2, 0x4c4, 0x4d6, 0x3, 0x2, 0x2, 0x2, 
       0x4c5, 0x4c7, 0x5, 0x66, 0x34, 0x2, 0x4c6, 0x4c8, 0x7, 0x85, 0x2, 
       0x2, 0x4c7, 0x4c6, 0x3, 0x2, 0x2, 0x2, 0x4c7, 0x4c8, 0x3, 0x2, 0x2, 
       0x2, 0x4c8, 0x4d3, 0x3, 0x2, 0x2, 0x2, 0x4c9, 0x4cb, 0x7, 0x4, 0x2, 
       0x2, 0x4ca, 0x4cc, 0x7, 0x85, 0x2, 0x2, 0x4cb, 0x4ca, 0x3, 0x2, 0x2, 
       0x2, 0x4cb, 0x4cc, 0x3, 0x2, 0x2, 0x2, 0x4cc, 0x4cd, 0x3, 0x2, 0x2, 
       0x2, 0x4cd, 0x4cf, 0x5, 0x66, 0x34, 0x2, 0x4ce, 0x4d0, 0x7, 0x85, 
       0x2, 0x2, 0x4cf, 0x4ce, 0x3, 0x2, 0x2, 0x2, 0x4cf, 0x4d0, 0x3, 0x2, 
       0x2, 0x2, 0x4d0, 0x4d2, 0x3, 0x2, 0x2, 0x2, 0x4d1, 0x4c9, 0x3, 0x2, 
       0x2, 0x2, 0x4d2, 0x4d5, 0x3, 0x2, 0x2, 0x2, 0x4d3, 0x4d1, 0x3, 0x2, 
       0x2, 0x2, 0x4d3, 0x4d4, 0x3, 0x2, 0x2, 0x2, 0x4d4, 0x4d7, 0x3, 0x2, 
       0x2, 0x2, 0x4d5, 0x4d3, 0x3, 0x2, 0x2, 0x2, 0x4d6, 0x4c5, 0x3, 0x2, 
       0x2, 0x2, 0x4d6, 0x4d7, 0x3, 0x2, 0x2, 0x2, 0x4d7, 0x4d8, 0x3, 0x2, 
       0x2, 0x2, 0x4d8, 0x4d9, 0x7, 0xb, 0x2, 0x2, 0x4d9, 0x8b, 0x3, 0x2, 
       0x2, 0x2, 0x4da, 0x4dc, 0x7, 0x5, 0x2, 0x2, 0x4db, 0x4dd, 0x7, 0x85, 
       0x2, 0x2, 0x4dc, 0x4db, 0x3, 0x2, 0x2, 0x2, 0x4dc, 0x4dd, 0x3, 0x2, 
       0x2, 0x2, 0x4dd, 0x4de, 0x3, 0x2, 0x2, 0x2, 0x4de, 0x4f9, 0x5, 0x72, 
       0x3a, 0x2, 0x4df, 0x4e1, 0x7, 0x14, 0x2, 0x2, 0x4e0, 0x4e2, 0x7, 
       0x85, 0x2, 0x2, 0x4e1, 0x4e0, 0x3, 0x2, 0x2, 0x2, 0x4e1, 0x4e2, 0x3, 
       0x2, 0x2, 0x2, 0x4e2, 0x4e3, 0x3, 0x2, 0x2, 0x2, 0x4e3, 0x4f9, 0x5, 
       0x72, 0x3a, 0x2, 0x4e4, 0x4e6, 0x7, 0x15, 0x2, 0x2, 0x4e5, 0x4e7, 
       0x7, 0x85, 0x2, 0x2, 0x4e6, 0x4e5, 0x3, 0x2, 0x2, 0x2, 0x4e6, 0x4e7, 
       0x3, 0x2, 0x2, 0x2, 0x4e7, 0x4e8, 0x3, 0x2, 0x2, 0x2, 0x4e8, 0x4f9, 
       0x5, 0x72, 0x3a, 0x2, 0x4e9, 0x4eb, 0x7, 0x16, 0x2, 0x2, 0x4ea, 0x4ec, 
       0x7, 0x85, 0x2, 0x2, 0x4eb, 0x4ea, 0x3, 0x2, 0x2, 0x2, 0x4eb, 0x4ec, 
       0x3, 0x2, 0x2, 0x2, 0x4ec, 0x4ed, 0x3, 0x2, 0x2, 0x2, 0x4ed, 0x4f9, 
       0x5, 0x72, 0x3a, 0x2, 0x4ee, 0x4f0, 0x7, 0x17, 0x2, 0x2, 0x4ef, 0x4f1, 
       0x7, 0x85, 0x2, 0x2, 0x4f0, 0x4ef, 0x3, 0x2, 0x2, 0x2, 0x4f0, 0x4f1, 
       0x3, 0x2, 0x2, 0x2, 0x4f1, 0x4f2, 0x3, 0x2, 0x2, 0x2, 0x4f2, 0x4f9, 
       0x5, 0x72, 0x3a, 0x2, 0x4f3, 0x4f5, 0x7, 0x18, 0x2, 0x2, 0x4f4, 0x4f6, 
       0x7, 0x85, 0x2, 0x2, 0x4f5, 0x4f4, 0x3, 0x2, 0x2, 0x2, 0x4f5, 0x4f6, 
       0x3, 0x2, 0x2, 0x2, 0x4f6, 0x4f7, 0x3, 0x2, 0x2, 0x2, 0x4f7, 0x4f9, 
       0x5, 0x72, 0x3a, 0x2, 0x4f8, 0x4da, 0x3, 0x2, 0x2, 0x2, 0x4f8, 0x4df, 
       0x3, 0x2, 0x2, 0x2, 0x4f8, 0x4e4, 0x3, 0x2, 0x2, 0x2, 0x4f8, 0x4e9, 
       0x3, 0x2, 0x2, 0x2, 0x4f8, 0x4ee, 0x3, 0x2, 0x2, 0x2, 0x4f8, 0x4f3, 
       0x3, 0x2, 0x2, 0x2, 0x4f9, 0x8d, 0x3, 0x2, 0x2, 0x2, 0x4fa, 0x4fc, 
       0x7, 0x8, 0x2, 0x2, 0x4fb, 0x4fd, 0x7, 0x85, 0x2, 0x2, 0x4fc, 0x4fb, 
       0x3, 0x2, 0x2, 0x2, 0x4fc, 0x4fd, 0x3, 0x2, 0x2, 0x2, 0x4fd, 0x4fe, 
       0x3, 0x2, 0x2, 0x2, 0x4fe, 0x500, 0x5, 0x66, 0x34, 0x2, 0x4ff, 0x501, 
       0x7, 0x85, 0x2, 0x2, 0x500, 0x4ff, 0x3, 0x2, 0x2, 0x2, 0x500, 0x501, 
       0x3, 0x2, 0x2, 0x2, 0x501, 0x502, 0x3, 0x2, 0x2, 0x2, 0x502, 0x503, 
       0x7, 0x9, 0x2, 0x2, 0x503, 0x8f, 0x3, 0x2, 0x2, 0x2, 0x504, 0x509, 
       0x5, 0x50, 0x29, 0x2, 0x505, 0x507, 0x7, 0x85, 0x2, 0x2, 0x506, 0x505, 
       0x3, 0x2, 0x2, 0x2, 0x506, 0x507, 0x3, 0x2, 0x2, 0x2, 0x507, 0x508, 
       0x3, 0x2, 0x2, 0x2, 0x508, 0x50a, 0x5, 0x52, 0x2a, 0x2, 0x509, 0x506, 
       0x3, 0x2, 0x2, 0x2, 0x50a, 0x50b, 0x3, 0x2, 0x2, 0x2, 0x50b, 0x509, 
       0x3, 0x2, 0x2, 0x2, 0x50b, 0x50c, 0x3, 0x2, 0x2, 0x2, 0x50c, 0x91, 
       0x3, 0x2, 0x2, 0x2, 0x50d, 0x512, 0x5, 0x94, 0x4b, 0x2, 0x50e, 0x510, 
       0x7, 0x85, 0x2, 0x2, 0x50f, 0x50e, 0x3, 0x2, 0x2, 0x2, 0x50f, 0x510, 
       0x3, 0x2, 0x2, 0x2, 0x510, 0x511, 0x3, 0x2, 0x2, 0x2, 0x511, 0x513, 
       0x5, 0x46, 0x24, 0x2, 0x512, 0x50f, 0x3, 0x2, 0x2, 0x2, 0x512, 0x513, 
       0x3, 0x2, 0x2, 0x2, 0x513, 0x93, 0x3, 0x2, 0x2, 0x2, 0x514, 0x515, 
       0x5, 0xae, 0x58, 0x2, 0x515, 0x516, 0x7, 0x85, 0x2, 0x2, 0x516, 0x517, 
       0x7, 0x54, 0x2, 0x2, 0x517, 0x518, 0x7, 0x85, 0x2, 0x2, 0x518, 0x519, 
       0x5, 0x66, 0x34, 0x2, 0x519, 0x95, 0x3, 0x2, 0x2, 0x2, 0x51a, 0x51c, 
       0x5, 0x98, 0x4d, 0x2, 0x51b, 0x51d, 0x7, 0x85, 0x2, 0x2, 0x51c, 0x51b, 
       0x3, 0x2, 0x2, 0x2, 0x51c, 0x51d, 0x3, 0x2, 0x2, 0x2, 0x51d, 0x51e, 
       0x3, 0x2, 0x2, 0x2, 0x51e, 0x520, 0x7, 0x8, 0x2, 0x2, 0x51f, 0x521, 
       0x7, 0x85, 0x2, 0x2, 0x520, 0x51f, 0x3, 0x2, 0x2, 0x2, 0x520, 0x521, 
       0x3, 0x2, 0x2, 0x2, 0x521, 0x526, 0x3, 0x2, 0x2, 0x2, 0x522, 0x524, 
       0x7, 0x42, 0x2, 0x2, 0x523, 0x525, 0x7, 0x85, 0x2, 0x2, 0x524, 0x523, 
       0x3, 0x2, 0x2, 0x2, 0x524, 0x525, 0x3, 0x2, 0x2, 0x2, 0x525, 0x527, 
       0x3, 0x2, 0x2, 0x2, 0x526, 0x522, 0x3, 0x2, 0x2, 0x2, 0x526, 0x527, 
       0x3, 0x2, 0x2, 0x2, 0x527, 0x539, 0x3, 0x2, 0x2, 0x2, 0x528, 0x52a, 
       0x5, 0x66, 0x34, 0x2, 0x529, 0x52b, 0x7, 0x85, 0x2, 0x2, 0x52a, 0x529, 
       0x3, 0x2, 0x2, 0x2, 0x52a, 0x52b, 0x3, 0x2, 0x2, 0x2, 0x52b, 0x536, 
       0x3, 0x2, 0x2, 0x2, 0x52c, 0x52e, 0x7, 0x4, 0x2, 0x2, 0x52d, 0x52f, 
       0x7, 0x85, 0x2, 0x2, 0x52e, 0x52d, 0x3, 0x2, 0x2, 0x2, 0x52e, 0x52f, 
       0x3, 0x2, 0x2, 0x2, 0x52f, 0x530, 0x3, 0x2, 0x2, 0x2, 0x530, 0x532, 
       0x5, 0x66, 0x34, 0x2, 0x531, 0x533, 0x7, 0x85, 0x2, 0x2, 0x532, 0x531, 
       0x3, 0x2, 0x2, 0x2, 0x532, 0x533, 0x3, 0x2, 0x2, 0x2, 0x533, 0x535, 
       0x3, 0x2, 0x2, 0x2, 0x534, 0x52c, 0x3, 0x2, 0x2, 0x2, 0x535, 0x538, 
       0x3, 0x2, 0x2, 0x2, 0x536, 0x534, 0x3, 0x2, 0x2, 0x2, 0x536, 0x537, 
       0x3, 0x2, 0x2, 0x2, 0x537, 0x53a, 0x3, 0x2, 0x2, 0x2, 0x538, 0x536, 
       0x3, 0x2, 0x2, 0x2, 0x539, 0x528, 0x3, 0x2, 0x2, 0x2, 0x539, 0x53a, 
       0x3, 0x2, 0x2, 0x2, 0x53a, 0x53b, 0x3, 0x2, 0x2, 0x2, 0x53b, 0x53c, 
       0x7, 0x9, 0x2, 0x2, 0x53c, 0x97, 0x3, 0x2, 0x2, 0x2, 0x53d, 0x53e, 
       0x5, 0xa2, 0x52, 0x2, 0x53e, 0x53f, 0x5, 0xc0, 0x61, 0x2, 0x53f, 
       0x542, 0x3, 0x2, 0x2, 0x2, 0x540, 0x542, 0x7, 0x61, 0x2, 0x2, 0x541, 
       0x53d, 0x3, 0x2, 0x2, 0x2, 0x541, 0x540, 0x3, 0x2, 0x2, 0x2, 0x542, 
       0x99, 0x3, 0x2, 0x2, 0x2, 0x543, 0x545, 0x5, 0xa0, 0x51, 0x2, 0x544, 
       0x546, 0x7, 0x85, 0x2, 0x2, 0x545, 0x544, 0x3, 0x2, 0x2, 0x2, 0x545, 
       0x546, 0x3, 0x2, 0x2, 0x2, 0x546, 0x547, 0x3, 0x2, 0x2, 0x2, 0x547, 
       0x549, 0x7, 0x8, 0x2, 0x2, 0x548, 0x54a, 0x7, 0x85, 0x2, 0x2, 0x549, 
       0x548, 0x3, 0x2, 0x2, 0x2, 0x549, 0x54a, 0x3, 0x2, 0x2, 0x2, 0x54a, 
       0x55c, 0x3, 0x2, 0x2, 0x2, 0x54b, 0x54d, 0x5, 0x66, 0x34, 0x2, 0x54c, 
       0x54e, 0x7, 0x85, 0x2, 0x2, 0x54d, 0x54c, 0x3, 0x2, 0x2, 0x2, 0x54d, 
       0x54e, 0x3, 0x2, 0x2, 0x2, 0x54e, 0x559, 0x3, 0x2, 0x2, 0x2, 0x54f, 
       0x551, 0x7, 0x4, 0x2, 0x2, 0x550, 0x552, 0x7, 0x85, 0x2, 0x2, 0x551, 
       0x550, 0x3, 0x2, 0x2, 0x2, 0x551, 0x552, 0x3, 0x2, 0x2, 0x2, 0x552, 
       0x553, 0x3, 0x2, 0x2, 0x2, 0x553, 0x555, 0x5, 0x66, 0x34, 0x2, 0x554, 
       0x556, 0x7, 0x85, 0x2, 0x2, 0x555, 0x554, 0x3, 0x2, 0x2, 0x2, 0x555, 
       0x556, 0x3, 0x2, 0x2, 0x2, 0x556, 0x558, 0x3, 0x2, 0x2, 0x2, 0x557, 
       0x54f, 0x3, 0x2, 0x2, 0x2, 0x558, 0x55b, 0x3, 0x2, 0x2, 0x2, 0x559, 
       0x557, 0x3, 0x2, 0x2, 0x2, 0x559, 0x55a, 0x3, 0x2, 0x2, 0x2, 0x55a, 
       0x55d, 0x3, 0x2, 0x2, 0x2, 0x55b, 0x559, 0x3, 0x2, 0x2, 0x2, 0x55c, 
       0x54b, 0x3, 0x2, 0x2, 0x2, 0x55c, 0x55d, 0x3, 0x2, 0x2, 0x2, 0x55d, 
       0x55e, 0x3, 0x2, 0x2, 0x2, 0x55e, 0x55f, 0x7, 0x9, 0x2, 0x2, 0x55f, 
       0x9b, 0x3, 0x2, 0x2, 0x2, 0x560, 0x561, 0x5, 0xa0, 0x51, 0x2, 0x561, 
       0x9d, 0x3, 0x2, 0x2, 0x2, 0x562, 0x563, 0x5, 0xc0, 0x61, 0x2, 0x563, 
       0x9f, 0x3, 0x2, 0x2, 0x2, 0x564, 0x565, 0x5, 0xa2, 0x52, 0x2, 0x565, 
       0x566, 0x5, 0xc0, 0x61, 0x2, 0x566, 0xa1, 0x3, 0x2, 0x2, 0x2, 0x567, 
       0x568, 0x5, 0xc0, 0x61, 0x2, 0x568, 0x569, 0x7, 0x19, 0x2, 0x2, 0x569, 
       0x56b, 0x3, 0x2, 0x2, 0x2, 0x56a, 0x567, 0x3, 0x2, 0x2, 0x2, 0x56b, 
       0x56e, 0x3, 0x2, 0x2, 0x2, 0x56c, 0x56a, 0x3, 0x2, 0x2, 0x2, 0x56c, 
       0x56d, 0x3, 0x2, 0x2, 0x2, 0x56d, 0xa3, 0x3, 0x2, 0x2, 0x2, 0x56e, 
       0x56c, 0x3, 0x2, 0x2, 0x2, 0x56f, 0x571, 0x7, 0xa, 0x2, 0x2, 0x570, 
       0x572, 0x7, 0x85, 0x2, 0x2, 0x571, 0x570, 0x3, 0x2, 0x2, 0x2, 0x571, 
       0x572, 0x3, 0x2, 0x2, 0x2, 0x572, 0x573, 0x3, 0x2, 0x2, 0x2, 0x573, 
       0x57c, 0x5, 0x92, 0x4a, 0x2, 0x574, 0x576, 0x7, 0x85, 0x2, 0x2, 0x575, 
       0x574, 0x3, 0x2, 0x2, 0x2, 0x575, 0x576, 0x3, 0x2, 0x2, 0x2, 0x576, 
       0x577, 0x3, 0x2, 0x2, 0x2, 0x577, 0x579, 0x7, 0xd, 0x2, 0x2, 0x578, 
       0x57a, 0x7, 0x85, 0x2, 0x2, 0x579, 0x578, 0x3, 0x2, 0x2, 0x2, 0x579, 
       0x57a, 0x3, 0x2, 0x2, 0x2, 0x57a, 0x57b, 0x3, 0x2, 0x2, 0x2, 0x57b, 
       0x57d, 0x5, 0x66, 0x34, 0x2, 0x57c, 0x575, 0x3, 0x2, 0x2, 0x2, 0x57c, 
       0x57d, 0x3, 0x2, 0x2, 0x2, 0x57d, 0x57f, 0x3, 0x2, 0x2, 0x2, 0x57e, 
       0x580, 0x7, 0x85, 0x2, 0x2, 0x57f, 0x57e, 0x3, 0x2, 0x2, 0x2, 0x57f, 
       0x580, 0x3, 0x2, 0x2, 0x2, 0x580, 0x581, 0x3, 0x2, 0x2, 0x2, 0x581, 
       0x582, 0x7, 0xb, 0x2, 0x2, 0x582, 0xa5, 0x3, 0x2, 0x2, 0x2, 0x583, 
       0x585, 0x7, 0xa, 0x2, 0x2, 0x584, 0x586, 0x7, 0x85, 0x2, 0x2, 0x585, 
       0x584, 0x3, 0x2, 0x2, 0x2, 0x585, 0x586, 0x3, 0x2, 0x2, 0x2, 0x586, 
       0x58f, 0x3, 0x2, 0x2, 0x2, 0x587, 0x589, 0x5, 0xae, 0x58, 0x2, 0x588, 
       0x58a, 0x7, 0x85, 0x2, 0x2, 0x589, 0x588, 0x3, 0x2, 0x2, 0x2, 0x589, 
       0x58a, 0x3, 0x2, 0x2, 0x2, 0x58a, 0x58b, 0x3, 0x2, 0x2, 0x2, 0x58b, 
       0x58d, 0x7, 0x5, 0x2, 0x2, 0x58c, 0x58e, 0x7, 0x85, 0x2, 0x2, 0x58d, 
       0x58c, 0x3, 0x2, 0x2, 0x2, 0x58d, 0x58e, 0x3, 0x2, 0x2, 0x2, 0x58e, 
       0x590, 0x3, 0x2, 0x2, 0x2, 0x58f, 0x587, 0x3, 0x2, 0x2, 0x2, 0x58f, 
       0x590, 0x3, 0x2, 0x2, 0x2, 0x590, 0x591, 0x3, 0x2, 0x2, 0x2, 0x591, 
       0x593, 0x5, 0x90, 0x49, 0x2, 0x592, 0x594, 0x7, 0x85, 0x2, 0x2, 0x593, 
       0x592, 0x3, 0x2, 0x2, 0x2, 0x593, 0x594, 0x3, 0x2, 0x2, 0x2, 0x594, 
       0x59d, 0x3, 0x2, 0x2, 0x2, 0x595, 0x597, 0x7, 0x4f, 0x2, 0x2, 0x596, 
       0x598, 0x7, 0x85, 0x2, 0x2, 0x597, 0x596, 0x3, 0x2, 0x2, 0x2, 0x597, 
       0x598, 0x3, 0x2, 0x2, 0x2, 0x598, 0x599, 0x3, 0x2, 0x2, 0x2, 0x599, 
       0x59b, 0x5, 0x66, 0x34, 0x2, 0x59a, 0x59c, 0x7, 0x85, 0x2, 0x2, 0x59b, 
       0x59a, 0x3, 0x2, 0x2, 0x2, 0x59b, 0x59c, 0x3, 0x2, 0x2, 0x2, 0x59c, 
       0x59e, 0x3, 0x2, 0x2, 0x2, 0x59d, 0x595, 0x3, 0x2, 0x2, 0x2, 0x59d, 
       0x59e, 0x3, 0x2, 0x2, 0x2, 0x59e, 0x59f, 0x3, 0x2, 0x2, 0x2, 0x59f, 
       0x5a1, 0x7, 0xd, 0x2, 0x2, 0x5a0, 0x5a2, 0x7, 0x85, 0x2, 0x2, 0x5a1, 
       0x5a0, 0x3, 0x2, 0x2, 0x2, 0x5a1, 0x5a2, 0x3, 0x2, 0x2, 0x2, 0x5a2, 
       0x5a3, 0x3, 0x2, 0x2, 0x2, 0x5a3, 0x5a5, 0x5, 0x66, 0x34, 0x2, 0x5a4, 
       0x5a6, 0x7, 0x85, 0x2, 0x2, 0x5a5, 0x5a4, 0x3, 0x2, 0x2, 0x2, 0x5a5, 
       0x5a6, 0x3, 0x2, 0x2, 0x2, 0x5a6, 0x5a7, 0x3, 0x2, 0x2, 0x2, 0x5a7, 
       0x5a8, 0x7, 0xb, 0x2, 0x2, 0x5a8, 0xa7, 0x3, 0x2, 0x2, 0x2, 0x5a9, 
       0x5ab, 0x7, 0x19, 0x2, 0x2, 0x5aa, 0x5ac, 0x7, 0x85, 0x2, 0x2, 0x5ab, 
       0x5aa, 0x3, 0x2, 0x2, 0x2, 0x5ab, 0x5ac, 0x3, 0x2, 0x2, 0x2, 0x5ac, 
       0x5ad, 0x3, 0x2, 0x2, 0x2, 0x5ad, 0x5ae, 0x5, 0xb8, 0x5d, 0x2, 0x5ae, 
       0xa9, 0x3, 0x2, 0x2, 0x2, 0x5af, 0x5b4, 0x7, 0x62, 0x2, 0x2, 0x5b0, 
       0x5b2, 0x7, 0x85, 0x2, 0x2, 0x5b1, 0x5b0, 0x3, 0x2, 0x2, 0x2, 0x5b1, 
       0x5b2, 0x3, 0x2, 0x2, 0x2, 0x5b2, 0x5b3, 0x3, 0x2, 0x2, 0x2, 0x5b3, 
       0x5b5, 0x5, 0xac, 0x57, 0x2, 0x5b4, 0x5b1, 0x3, 0x2, 0x2, 0x2, 0x5b5, 
       0x5b6, 0x3, 0x2, 0x2, 0x2, 0x5b6, 0x5b4, 0x3, 0x2, 0x2, 0x2, 0x5b6, 
       0x5b7, 0x3, 0x2, 0x2, 0x2, 0x5b7, 0x5c6, 0x3, 0x2, 0x2, 0x2, 0x5b8, 
       0x5ba, 0x7, 0x62, 0x2, 0x2, 0x5b9, 0x5bb, 0x7, 0x85, 0x2, 0x2, 0x5ba, 
       0x5b9, 0x3, 0x2, 0x2, 0x2, 0x5ba, 0x5bb, 0x3, 0x2, 0x2, 0x2, 0x5bb, 
       0x5bc, 0x3, 0x2, 0x2, 0x2, 0x5bc, 0x5c1, 0x5, 0x66, 0x34, 0x2, 0x5bd, 
       0x5bf, 0x7, 0x85, 0x2, 0x2, 0x5be, 0x5bd, 0x3, 0x2, 0x2, 0x2, 0x5be, 
       0x5bf, 0x3, 0x2, 0x2, 0x2, 0x5bf, 0x5c0, 0x3, 0x2, 0x2, 0x2, 0x5c0, 
       0x5c2, 0x5, 0xac, 0x57, 0x2, 0x5c1, 0x5be, 0x3, 0x2, 0x2, 0x2, 0x5c2, 
       0x5c3, 0x3, 0x2, 0x2, 0x2, 0x5c3, 0x5c1, 0x3, 0x2, 0x2, 0x2, 0x5c3, 
       0x5c4, 0x3, 0x2, 0x2, 0x2, 0x5c4, 0x5c6, 0x3, 0x2, 0x2, 0x2, 0x5c5, 
       0x5af, 0x3, 0x2, 0x2, 0x2, 0x5c5, 0x5b8, 0x3, 0x2, 0x2, 0x2, 0x5c6, 
       0x5cf, 0x3, 0x2, 0x2, 0x2, 0x5c7, 0x5c9, 0x7, 0x85, 0x2, 0x2, 0x5c8, 
       0x5c7, 0x3, 0x2, 0x2, 0x2, 0x5c8, 0x5c9, 0x3, 0x2, 0x2, 0x2, 0x5c9, 
       0x5ca, 0x3, 0x2, 0x2, 0x2, 0x5ca, 0x5cc, 0x7, 0x63, 0x2, 0x2, 0x5cb, 
       0x5cd, 0x7, 0x85, 0x2, 0x2, 0x5cc, 0x5cb, 0x3, 0x2, 0x2, 0x2, 0x5cc, 
       0x5cd, 0x3, 0x2, 0x2, 0x2, 0x5cd, 0x5ce, 0x3, 0x2, 0x2, 0x2, 0x5ce, 
       0x5d0, 0x5, 0x66, 0x34, 0x2, 0x5cf, 0x5c8, 0x3, 0x2, 0x2, 0x2, 0x5cf, 
       0x5d0, 0x3, 0x2, 0x2, 0x2, 0x5d0, 0x5d2, 0x3, 0x2, 0x2, 0x2, 0x5d1, 
       0x5d3, 0x7, 0x85, 0x2, 0x2, 0x5d2, 0x5d1, 0x3, 0x2, 0x2, 0x2, 0x5d2, 
       0x5d3, 0x3, 0x2, 0x2, 0x2, 0x5d3, 0x5d4, 0x3, 0x2, 0x2, 0x2, 0x5d4, 
       0x5d5, 0x7, 0x64, 0x2, 0x2, 0x5d5, 0xab, 0x3, 0x2, 0x2, 0x2, 0x5d6, 
       0x5d8, 0x7, 0x65, 0x2, 0x2, 0x5d7, 0x5d9, 0x7, 0x85, 0x2, 0x2, 0x5d8, 
       0x5d7, 0x3, 0x2, 0x2, 0x2, 0x5d8, 0x5d9, 0x3, 0x2, 0x2, 0x2, 0x5d9, 
       0x5da, 0x3, 0x2, 0x2, 0x2, 0x5da, 0x5dc, 0x5, 0x66, 0x34, 0x2, 0x5db, 
       0x5dd, 0x7, 0x85, 0x2, 0x2, 0x5dc, 0x5db, 0x3, 0x2, 0x2, 0x2, 0x5dc, 
       0x5dd, 0x3, 0x2, 0x2, 0x2, 0x5dd, 0x5de, 0x3, 0x2, 0x2, 0x2, 0x5de, 
       0x5e0, 0x7, 0x66, 0x2, 0x2, 0x5df, 0x5e1, 0x7, 0x85, 0x2, 0x2, 0x5e0, 
       0x5df, 0x3, 0x2, 0x2, 0x2, 0x5e0, 0x5e1, 0x3, 0x2, 0x2, 0x2, 0x5e1, 
       0x5e2, 0x3, 0x2, 0x2, 0x2, 0x5e2, 0x5e3, 0x5, 0x66, 0x34, 0x2, 0x5e3, 
       0xad, 0x3, 0x2, 0x2, 0x2, 0x5e4, 0x5e5, 0x5, 0xc0, 0x61, 0x2, 0x5e5, 
       0xaf, 0x3, 0x2, 0x2, 0x2, 0x5e6, 0x5e9, 0x5, 0xbc, 0x5f, 0x2, 0x5e7, 
       0x5e9, 0x5, 0xba, 0x5e, 0x2, 0x5e8, 0x5e6, 0x3, 0x2, 0x2, 0x2, 0x5e8, 
       0x5e7, 0x3, 0x2, 0x2, 0x2, 0x5e9, 0xb1, 0x3, 0x2, 0x2, 0x2, 0x5ea, 
       0x5ec, 0x7, 0x1a, 0x2, 0x2, 0x5eb, 0x5ed, 0x7, 0x85, 0x2, 0x2, 0x5ec, 
       0x5eb, 0x3, 0x2, 0x2, 0x2, 0x5ec, 0x5ed, 0x3, 0x2, 0x2, 0x2, 0x5ed, 
       0x60f, 0x3, 0x2, 0x2, 0x2, 0x5ee, 0x5f0, 0x5, 0xb8, 0x5d, 0x2, 0x5ef, 
       0x5f1, 0x7, 0x85, 0x2, 0x2, 0x5f0, 0x5ef, 0x3, 0x2, 0x2, 0x2, 0x5f0, 
       0x5f1, 0x3, 0x2, 0x2, 0x2, 0x5f1, 0x5f2, 0x3, 0x2, 0x2, 0x2, 0x5f2, 
       0x5f4, 0x7, 0xc, 0x2, 0x2, 0x5f3, 0x5f5, 0x7, 0x85, 0x2, 0x2, 0x5f4, 
       0x5f3, 0x3, 0x2, 0x2, 0x2, 0x5f4, 0x5f5, 0x3, 0x2, 0x2, 0x2, 0x5f5, 
       0x5f6, 0x3, 0x2, 0x2, 0x2, 0x5f6, 0x5f8, 0x5, 0x66, 0x34, 0x2, 0x5f7, 
       0x5f9, 0x7, 0x85, 0x2, 0x2, 0x5f8, 0x5f7, 0x3, 0x2, 0x2, 0x2, 0x5f8, 
       0x5f9, 0x3, 0x2, 0x2, 0x2, 0x5f9, 0x60c, 0x3, 0x2, 0x2, 0x2, 0x5fa, 
       0x5fc, 0x7, 0x4, 0x2, 0x2, 0x5fb, 0x5fd, 0x7, 0x85, 0x2, 0x2, 0x5fc, 
       0x5fb, 0x3, 0x2, 0x2, 0x2, 0x5fc, 0x5fd, 0x3, 0x2, 0x2, 0x2, 0x5fd, 
       0x5fe, 0x3, 0x2, 0x2, 0x2, 0x5fe, 0x600, 0x5, 0xb8, 0x5d, 0x2, 0x5ff, 
       0x601, 0x7, 0x85, 0x2, 0x2, 0x600, 0x5ff, 0x3, 0x2, 0x2, 0x2, 0x600, 
       0x601, 0x3, 0x2, 0x2, 0x2, 0x601, 0x602, 0x3, 0x2, 0x2, 0x2, 0x602, 
       0x604, 0x7, 0xc, 0x2, 0x2, 0x603, 0x605, 0x7, 0x85, 0x2, 0x2, 0x604, 
       0x603, 0x3, 0x2, 0x2, 0x2, 0x604, 0x605, 0x3, 0x2, 0x2, 0x2, 0x605, 
       0x606, 0x3, 0x2, 0x2, 0x2, 0x606, 0x608, 0x5, 0x66, 0x34, 0x2, 0x607, 
       0x609, 0x7, 0x85, 0x2, 0x2, 0x608, 0x607, 0x3, 0x2, 0x2, 0x2, 0x608, 
       0x609, 0x3, 0x2, 0x2, 0x2, 0x609, 0x60b, 0x3, 0x2, 0x2, 0x2, 0x60a, 
       0x5fa, 0x3, 0x2, 0x2, 0x2, 0x60b, 0x60e, 0x3, 0x2, 0x2, 0x2, 0x60c, 
       0x60a, 0x3, 0x2, 0x2, 0x2, 0x60c, 0x60d, 0x3, 0x2, 0x2, 0x2, 0x60d, 
       0x610, 0x3, 0x2, 0x2, 0x2, 0x60e, 0x60c, 0x3, 0x2, 0x2, 0x2, 0x60f, 
       0x5ee, 0x3, 0x2, 0x2, 0x2, 0x60f, 0x610, 0x3, 0x2, 0x2, 0x2, 0x610, 
       0x611, 0x3, 0x2, 0x2, 0x2, 0x611, 0x612, 0x7, 0x1b, 0x2, 0x2, 0x612, 
       0xb3, 0x3, 0x2, 0x2, 0x2, 0x613, 0x616, 0x7, 0x1c, 0x2, 0x2, 0x614, 
       0x617, 0x5, 0xc0, 0x61, 0x2, 0x615, 0x617, 0x7, 0x6a, 0x2, 0x2, 0x616, 
       0x614, 0x3, 0x2, 0x2, 0x2, 0x616, 0x615, 0x3, 0x2, 0x2, 0x2, 0x617, 
       0xb5, 0x3, 0x2, 0x2, 0x2, 0x618, 0x61d, 0x5, 0x84, 0x43, 0x2, 0x619, 
       0x61b, 0x7, 0x85, 0x2, 0x2, 0x61a, 0x619, 0x3, 0x2, 0x2, 0x2, 0x61a, 
       0x61b, 0x3, 0x2, 0x2, 0x2, 0x61b, 0x61c, 0x3, 0x2, 0x2, 0x2, 0x61c, 
       0x61e, 0x5, 0xa8, 0x55, 0x2, 0x61d, 0x61a, 0x3, 0x2, 0x2, 0x2, 0x61e, 
       0x61f, 0x3, 0x2, 0x2, 0x2, 0x61f, 0x61d, 0x3, 0x2, 0x2, 0x2, 0x61f, 
       0x620, 0x3, 0x2, 0x2, 0x2, 0x620, 0xb7, 0x3, 0x2, 0x2, 0x2, 0x621, 
       0x622, 0x5, 0xbe, 0x60, 0x2, 0x622, 0xb9, 0x3, 0x2, 0x2, 0x2, 0x623, 
       0x624, 0x9, 0x5, 0x2, 0x2, 0x624, 0xbb, 0x3, 0x2, 0x2, 0x2, 0x625, 
       0x626, 0x9, 0x6, 0x2, 0x2, 0x626, 0xbd, 0x3, 0x2, 0x2, 0x2, 0x627, 
       0x62a, 0x5, 0xc0, 0x61, 0x2, 0x628, 0x62a, 0x5, 0xc2, 0x62, 0x2, 
       0x629, 0x627, 0x3, 0x2, 0x2, 0x2, 0x629, 0x628, 0x3, 0x2, 0x2, 0x2, 
       0x62a, 0xbf, 0x3, 0x2, 0x2, 0x2, 0x62b, 0x62c, 0x9, 0x7, 0x2, 0x2, 
       0x62c, 0xc1, 0x3, 0x2, 0x2, 0x2, 0x62d, 0x62e, 0x9, 0x8, 0x2, 0x2, 
       0x62e, 0xc3, 0x3, 0x2, 0x2, 0x2, 0x62f, 0x630, 0x9, 0x9, 0x2, 0x2, 
       0x630, 0xc5, 0x3, 0x2, 0x2, 0x2, 0x631, 0x632, 0x9, 0xa, 0x2, 0x2, 
       0x632, 0xc7, 0x3, 0x2, 0x2, 0x2, 0x633, 0x634, 0x9, 0xb, 0x2, 0x2, 
       0x634, 0xc9, 0x3, 0x2, 0x2, 0x2, 0x125, 0xcb, 0xcf, 0xd2, 0xd5, 0xdc, 
       0xe1, 0xe4, 0xe8, 0xec, 0xf1, 0xf8, 0xfd, 0x100, 0x104, 0x108, 0x10c, 
       0x112, 0x116, 0x11b, 0x120, 0x124, 0x127, 0x129, 0x12d, 0x131, 0x136, 
       0x13a, 0x13f, 0x143, 0x14c, 0x151, 0x155, 0x159, 0x15d, 0x162, 0x166, 
       0x169, 0x16d, 0x177, 0x17e, 0x18b, 0x18f, 0x195, 0x19c, 0x1a1, 0x1a5, 
       0x1ab, 0x1af, 0x1b5, 0x1b9, 0x1bf, 0x1c3, 0x1c7, 0x1cb, 0x1cf, 0x1d3, 
       0x1d8, 0x1df, 0x1e3, 0x1e8, 0x1ef, 0x1f5, 0x1fa, 0x200, 0x206, 0x20b, 
       0x20f, 0x214, 0x217, 0x21a, 0x21d, 0x224, 0x22a, 0x22d, 0x232, 0x235, 
       0x239, 0x23c, 0x244, 0x248, 0x24c, 0x250, 0x254, 0x259, 0x25e, 0x262, 
       0x267, 0x26a, 0x273, 0x27c, 0x281, 0x28e, 0x291, 0x2a1, 0x2a9, 0x2ad, 
       0x2b2, 0x2b7, 0x2bb, 0x2c0, 0x2c6, 0x2cb, 0x2d2, 0x2d6, 0x2da, 0x2dc, 
       0x2e0, 0x2e2, 0x2e6, 0x2e8, 0x2ee, 0x2f4, 0x2f8, 0x2fb, 0x2fe, 0x302, 
       0x308, 0x30c, 0x30f, 0x312, 0x318, 0x31b, 0x31e, 0x322, 0x328, 0x32b, 
       0x32e, 0x332, 0x336, 0x33a, 0x33c, 0x340, 0x342, 0x345, 0x349, 0x34b, 
       0x351, 0x355, 0x359, 0x35d, 0x360, 0x365, 0x36a, 0x36f, 0x374, 0x37a, 
       0x37e, 0x380, 0x384, 0x388, 0x38a, 0x38c, 0x39b, 0x3a5, 0x3af, 0x3b4, 
       0x3b8, 0x3bf, 0x3c4, 0x3c9, 0x3cd, 0x3d1, 0x3d5, 0x3d8, 0x3da, 0x3df, 
       0x3e3, 0x3e7, 0x3eb, 0x3ef, 0x3f3, 0x3f6, 0x3f8, 0x3fd, 0x401, 0x406, 
       0x40b, 0x40f, 0x418, 0x41a, 0x420, 0x424, 0x42b, 0x42f, 0x433, 0x436, 
       0x444, 0x447, 0x455, 0x459, 0x45e, 0x462, 0x465, 0x46c, 0x470, 0x474, 
       0x47b, 0x47f, 0x483, 0x489, 0x48d, 0x491, 0x497, 0x49b, 0x49f, 0x4a5, 
       0x4a9, 0x4ad, 0x4b5, 0x4bd, 0x4c3, 0x4c7, 0x4cb, 0x4cf, 0x4d3, 0x4d6, 
       0x4dc, 0x4e1, 0x4e6, 0x4eb, 0x4f0, 0x4f5, 0x4f8, 0x4fc, 0x500, 0x506, 
       0x50b, 0x50f, 0x512, 0x51c, 0x520, 0x524, 0x526, 0x52a, 0x52e, 0x532, 
       0x536, 0x539, 0x541, 0x545, 0x549, 0x54d, 0x551, 0x555, 0x559, 0x55c, 
       0x56c, 0x571, 0x575, 0x579, 0x57c, 0x57f, 0x585, 0x589, 0x58d, 0x58f, 
       0x593, 0x597, 0x59b, 0x59d, 0x5a1, 0x5a5, 0x5ab, 0x5b1, 0x5b6, 0x5ba, 
       0x5be, 0x5c3, 0x5c5, 0x5c8, 0x5cc, 0x5cf, 0x5d2, 0x5d8, 0x5dc, 0x5e0, 
       0x5e8, 0x5ec, 0x5f0, 0x5f4, 0x5f8, 0x5fc, 0x600, 0x604, 0x608, 0x60c, 
       0x60f, 0x616, 0x61a, 0x61f, 0x629, 
  };

  _serializedATN.insert(_serializedATN.end(), serializedATNSegment0,
    serializedATNSegment0 + sizeof(serializedATNSegment0) / sizeof(serializedATNSegment0[0]));


  atn::ATNDeserializer deserializer;
  _atn = deserializer.deserialize(_serializedATN);

  size_t count = _atn.getNumberOfDecisions();
  _decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    _decisionToDFA.emplace_back(_atn.getDecisionState(i), i);
  }
}

LcypherParser::Initializer LcypherParser::_init;
