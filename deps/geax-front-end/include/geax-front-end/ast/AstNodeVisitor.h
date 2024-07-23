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

#ifndef GEAXFRONTEND_AST_ASTNODEVISITOR_H_
#define GEAXFRONTEND_AST_ASTNODEVISITOR_H_

#include <any>

#include "geax-front-end/GEAXErrorCode.h"

namespace geax::frontend {

class PathModePrefix;
class PathSearchPrefix;
class SingleLabel;
class LabelOr;
class LabelAnd;
class LabelNot;
class PropStruct;
class YieldField;
class OrderByField;
class Node;
class Edge;
class ElementFiller;
class PathPattern;
class PathChain;
class GraphPattern;
class WhereClause;
class TableFunctionClause;
class ReadConsistency;
class AllowAnonymousTable;
class OpConcurrent;
class EdgeOnJoin;
class SetAllProperties;
class UpdateProperties;
class SetSingleProperty;
class SetLabel;
class OtherWise;
class Union;
class Except;
class Intersect;
class SchemaFromPath;
class BindingTable;
class BindingValue;
class BindingGraph;
class SetSchemaClause;
class SetGraphClause;
class SetTimeZoneClause;
class SetParamClause;
class ResetAll;
class ResetSchema;
class ResetTimeZone;
class ResetGraph;
class ResetParam;

class BEqual;
class BNotEqual;
class BGreaterThan;
class BNotSmallerThan;
class BSmallerThan;
class BNotGreaterThan;
class BSafeEqual;
class BAdd;
class BSub;
class BMod;
class BMul;
class BDiv;
class BSquare;
class BOr;
class BXor;
class BBitAnd;
class BBitOr;
class BBitXor;
class BBitLeftShift;
class BBitRightShift;
class GetField;
class TupleGet;
class Not;
class Neg;
class Tilde;
class VSome;
class BAnd;
class BConcat;
class BIndex;
class BLike;
class BIn;
class If;
class Function;
class Case;
class Cast;
class MatchCase;
class AggFunc;
class BAggFunc;
class MultiCount;
class Windowing;
class MkList;
class MkMap;
class MkRecord;
class MkSet;
class MkTuple;
class VBool;
class VInt;
class VDouble;
class VString;
class VDate;
class VDatetime;
class VDuration;
class VTime;
class VNull;
class VNone;
class Ref;
class Param;
class IsNull;
class IsDirected;
class IsNormalized;
class IsSourceOf;
class IsDestinationOf;
class IsLabeled;
class Same;
class AllDifferent;
class Exists;

class ExplainActivity;
class SessionActivity;
class SessionSet;
class SessionReset;
class TransactionActivity;
class FullTransaction;
class NormalTransaction;
class StartTransaction;
class CommitTransaction;
class RollBackTransaction;
class ProcedureBody;
class BindingTable;
class BindingValue;
class BindingGraph;
class BindingTableInnerQuery;
class BindingTableInnerExpr;
class StatementWithYield;
class QueryStatement;
class StandaloneCallStatement;
class JoinQueryExpression;
class JoinRightPart;
class CompositeQueryStatement;
class AmbientLinearQueryStatement;
class SelectStatement;
class FocusedQueryStatement;
class FocusedResultStatement;
class MatchStatement;
class FilterStatement;
class ForStatement;
class CallQueryStatement;
class CallProcedureStatement;
class InlineProcedureCall;
class NamedProcedureCall;
class PrimitiveResultStatement;
class CatalogModifyStatement;
class LinearDataModifyingStatement;
class InsertStatement;
class ReplaceStatement;
class SetStatement;
class DeleteStatement;
class RemoveStatement;
class MergeStatement;
class ShowProcessListStatement;
class KillStatement;
class ManagerStatement;
class UnwindStatement;
class InQueryProcedureCall;

class DummyNode;

/**
 * An AstNodeVisitor is designed to traverse the AST tree.
 *
 * You can define your own AstNodeVisitor by overriding each visit functions.
 */
class AstNodeVisitor {
 public:
    AstNodeVisitor() = default;
    virtual ~AstNodeVisitor() = default;

    //---------------------------------------------------------------------------------
    // patterns
    virtual std::any visit(GraphPattern* node) = 0;
    virtual std::any visit(PathPattern* node) = 0;
    virtual std::any visit(PathChain* node) = 0;
    virtual std::any visit(Node* node) = 0;
    virtual std::any visit(Edge* node) = 0;
    virtual std::any visit(ElementFiller* node) = 0;

    //---------------------------------------------------------------------------------
    // clauses
    virtual std::any visit(WhereClause* node) = 0;
    virtual std::any visit(OrderByField* node) = 0;
    virtual std::any visit(PathModePrefix* node) = 0;
    virtual std::any visit(PathSearchPrefix* node) = 0;
    virtual std::any visit(SingleLabel* node) = 0;
    virtual std::any visit(LabelOr* node) = 0;
    virtual std::any visit(LabelAnd* node) = 0;
    virtual std::any visit(LabelNot* node) = 0;
    virtual std::any visit(PropStruct* node) = 0;
    virtual std::any visit(YieldField* node) = 0;
    virtual std::any visit(TableFunctionClause* node) = 0;
    virtual std::any visit(ReadConsistency* node) = 0;
    virtual std::any visit(AllowAnonymousTable* node) = 0;
    virtual std::any visit(OpConcurrent* node) = 0;
    virtual std::any visit(EdgeOnJoin* node) = 0;
    virtual std::any visit(SetAllProperties* node) = 0;
    virtual std::any visit(UpdateProperties* node) = 0;
    virtual std::any visit(SetLabel* node) = 0;
    virtual std::any visit(SetSingleProperty* node) = 0;
    virtual std::any visit(SetSchemaClause* node) = 0;
    virtual std::any visit(SetGraphClause* node) = 0;
    virtual std::any visit(SetTimeZoneClause* node) = 0;
    virtual std::any visit(SetParamClause* node) = 0;
    virtual std::any visit(ResetAll* node) = 0;
    virtual std::any visit(ResetSchema* node) = 0;
    virtual std::any visit(ResetTimeZone* node) = 0;
    virtual std::any visit(ResetGraph* node) = 0;
    virtual std::any visit(ResetParam* node) = 0;

    //---------------------------------------------------------------------------------
    // exprs
    virtual std::any visit(GetField* node) = 0;
    virtual std::any visit(TupleGet* node) = 0;
    virtual std::any visit(Not* node) = 0;
    virtual std::any visit(Neg* node) = 0;
    virtual std::any visit(Tilde* node) = 0;
    virtual std::any visit(VSome* node) = 0;

    virtual std::any visit(BEqual* node) = 0;
    virtual std::any visit(BNotEqual* node) = 0;
    virtual std::any visit(BGreaterThan* node) = 0;
    virtual std::any visit(BNotSmallerThan* node) = 0;
    virtual std::any visit(BSmallerThan* node) = 0;
    virtual std::any visit(BNotGreaterThan* node) = 0;
    virtual std::any visit(BSafeEqual* node) = 0;
    virtual std::any visit(BAdd* node) = 0;
    virtual std::any visit(BSub* node) = 0;
    virtual std::any visit(BDiv* node) = 0;
    virtual std::any visit(BMul* node) = 0;
    virtual std::any visit(BMod* node) = 0;
    virtual std::any visit(BSquare* node) = 0;
    virtual std::any visit(BAnd* node) = 0;
    virtual std::any visit(BOr* node) = 0;
    virtual std::any visit(BXor* node) = 0;
    virtual std::any visit(BBitAnd* node) = 0;
    virtual std::any visit(BBitOr* node) = 0;
    virtual std::any visit(BBitXor* node) = 0;
    virtual std::any visit(BBitLeftShift* node) = 0;
    virtual std::any visit(BBitRightShift* node) = 0;
    virtual std::any visit(BConcat* node) = 0;
    virtual std::any visit(BIndex* node) = 0;
    virtual std::any visit(BLike* node) = 0;
    virtual std::any visit(BIn* node) = 0;

    virtual std::any visit(If* node) = 0;
    virtual std::any visit(Function* node) = 0;
    virtual std::any visit(Case* node) = 0;
    virtual std::any visit(Cast* node) = 0;
    virtual std::any visit(MatchCase* node) = 0;
    virtual std::any visit(AggFunc* node) = 0;
    virtual std::any visit(BAggFunc* node) = 0;
    virtual std::any visit(MultiCount* node) = 0;
    virtual std::any visit(Windowing* node) = 0;

    virtual std::any visit(MkList* node) = 0;
    virtual std::any visit(MkMap* node) = 0;
    virtual std::any visit(MkRecord* node) = 0;
    virtual std::any visit(MkSet* node) = 0;
    virtual std::any visit(MkTuple* node) = 0;

    virtual std::any visit(VBool* node) = 0;
    virtual std::any visit(VInt* node) = 0;
    virtual std::any visit(VDouble* node) = 0;
    virtual std::any visit(VString* node) = 0;
    virtual std::any visit(VDate* node) = 0;
    virtual std::any visit(VDatetime* node) = 0;
    virtual std::any visit(VDuration* node) = 0;
    virtual std::any visit(VTime* node) = 0;
    virtual std::any visit(VNull* node) = 0;
    virtual std::any visit(VNone* node) = 0;
    virtual std::any visit(Ref* node) = 0;
    virtual std::any visit(Param* node) = 0;

    // predicates
    virtual std::any visit(IsNull* node) = 0;
    virtual std::any visit(IsDirected* node) = 0;
    virtual std::any visit(IsNormalized* node) = 0;
    virtual std::any visit(IsSourceOf* node) = 0;
    virtual std::any visit(IsDestinationOf* node) = 0;
    virtual std::any visit(IsLabeled* node) = 0;
    virtual std::any visit(Same* node) = 0;
    virtual std::any visit(AllDifferent* node) = 0;
    virtual std::any visit(Exists* node) = 0;

    //---------------------------------------------------------------------------------
    // stmt
    virtual std::any visit(ExplainActivity* node) = 0;
    virtual std::any visit(SessionActivity* node) = 0;
    virtual std::any visit(TransactionActivity* node) = 0;
    virtual std::any visit(FullTransaction* node) = 0;
    virtual std::any visit(NormalTransaction* node) = 0;
    virtual std::any visit(StartTransaction* node) = 0;
    virtual std::any visit(CommitTransaction* node) = 0;
    virtual std::any visit(RollBackTransaction* node) = 0;
    virtual std::any visit(SessionSet* node) = 0;
    virtual std::any visit(SessionReset* node) = 0;
    virtual std::any visit(ProcedureBody* node) = 0;
    virtual std::any visit(SchemaFromPath* node) = 0;
    virtual std::any visit(BindingValue* node) = 0;
    virtual std::any visit(BindingGraph* node) = 0;
    virtual std::any visit(BindingTable* node) = 0;
    virtual std::any visit(BindingTableInnerQuery* node) = 0;
    virtual std::any visit(BindingTableInnerExpr* node) = 0;
    virtual std::any visit(StatementWithYield* node) = 0;
    virtual std::any visit(QueryStatement* node) = 0;
    virtual std::any visit(StandaloneCallStatement* node) = 0;
    virtual std::any visit(JoinQueryExpression* node) = 0;
    virtual std::any visit(JoinRightPart* node) = 0;
    virtual std::any visit(CompositeQueryStatement* node) = 0;
    virtual std::any visit(AmbientLinearQueryStatement* node) = 0;
    virtual std::any visit(SelectStatement* node) = 0;
    virtual std::any visit(FocusedQueryStatement* node) = 0;
    virtual std::any visit(FocusedResultStatement* node) = 0;
    virtual std::any visit(MatchStatement* node) = 0;
    virtual std::any visit(FilterStatement* node) = 0;
    virtual std::any visit(CallQueryStatement* node) = 0;
    virtual std::any visit(CallProcedureStatement* node) = 0;
    virtual std::any visit(InlineProcedureCall* node) = 0;
    virtual std::any visit(NamedProcedureCall* node) = 0;
    virtual std::any visit(ForStatement* node) = 0;
    virtual std::any visit(PrimitiveResultStatement* node) = 0;
    virtual std::any visit(CatalogModifyStatement* node) = 0;
    virtual std::any visit(LinearDataModifyingStatement* node) = 0;
    virtual std::any visit(InsertStatement* node) = 0;
    virtual std::any visit(ReplaceStatement* node) = 0;
    virtual std::any visit(SetStatement* node) = 0;
    virtual std::any visit(DeleteStatement* node) = 0;
    virtual std::any visit(RemoveStatement* node) = 0;
    virtual std::any visit(MergeStatement* node) = 0;
    virtual std::any visit(OtherWise* node) = 0;
    virtual std::any visit(Union* node) = 0;
    virtual std::any visit(Except* node) = 0;
    virtual std::any visit(Intersect* node) = 0;
    virtual std::any visit(ShowProcessListStatement* node) = 0;
    virtual std::any visit(KillStatement* node) = 0;
    virtual std::any visit(ManagerStatement* node) = 0;
    virtual std::any visit(UnwindStatement* node) = 0;
    virtual std::any visit(InQueryProcedureCall* node) = 0;

    virtual std::any visit(DummyNode* node) = 0;

 protected:
    virtual std::any reportError() = 0;
};  // class AstNodeVisitor

class AstExprNodeVisitorImpl : public AstNodeVisitor {
 public:
    AstExprNodeVisitorImpl() = default;
    virtual ~AstExprNodeVisitorImpl() = default;

    //---------------------------------------------------------------------------------
    // patterns
    virtual std::any visit(GraphPattern*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(PathPattern*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(PathChain*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Node*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Edge*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ElementFiller*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    //---------------------------------------------------------------------------------
    // clauses
    virtual std::any visit(WhereClause*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(OrderByField*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(PathModePrefix*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(PathSearchPrefix*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SingleLabel*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(LabelOr*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(LabelAnd*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(LabelNot*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(PropStruct*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(YieldField*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(TableFunctionClause*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ReadConsistency*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(AllowAnonymousTable*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(OpConcurrent*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(EdgeOnJoin*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SetAllProperties*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(UpdateProperties*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SetLabel*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SetSingleProperty*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SetSchemaClause*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SetGraphClause*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SetTimeZoneClause*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SetParamClause*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ResetAll*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ResetSchema*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ResetTimeZone*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ResetGraph*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ResetParam*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    //---------------------------------------------------------------------------------
    // exprs
    virtual std::any visit(GetField* node) override = 0;
    virtual std::any visit(TupleGet* node) override = 0;
    virtual std::any visit(Not* node) override = 0;
    virtual std::any visit(Neg* node) override = 0;
    virtual std::any visit(Tilde* node) override = 0;
    virtual std::any visit(VSome* node) override = 0;

    virtual std::any visit(BEqual* node) override = 0;
    virtual std::any visit(BNotEqual* node) override = 0;
    virtual std::any visit(BGreaterThan* node) override = 0;
    virtual std::any visit(BNotSmallerThan* node) override = 0;
    virtual std::any visit(BSmallerThan* node) override = 0;
    virtual std::any visit(BNotGreaterThan* node) override = 0;
    virtual std::any visit(BSafeEqual* node) override = 0;
    virtual std::any visit(BAdd* node) override = 0;
    virtual std::any visit(BSub* node) override = 0;
    virtual std::any visit(BDiv* node) override = 0;
    virtual std::any visit(BMul* node) override = 0;
    virtual std::any visit(BMod* node) override = 0;
    virtual std::any visit(BSquare* node) override = 0;
    virtual std::any visit(BAnd* node) override = 0;
    virtual std::any visit(BOr* node) override = 0;
    virtual std::any visit(BXor* node) override = 0;
    virtual std::any visit(BBitAnd* node) override = 0;
    virtual std::any visit(BBitOr* node) override = 0;
    virtual std::any visit(BBitXor* node) override = 0;
    virtual std::any visit(BBitLeftShift* node) override = 0;
    virtual std::any visit(BBitRightShift* node) override = 0;
    virtual std::any visit(BConcat* node) override = 0;
    virtual std::any visit(BIndex* node) override = 0;
    virtual std::any visit(BLike* node) override = 0;
    virtual std::any visit(BIn* node) override = 0;

    virtual std::any visit(If* node) override = 0;
    virtual std::any visit(Function* node) override = 0;
    virtual std::any visit(Case* node) override = 0;
    virtual std::any visit(Cast* node) override = 0;
    virtual std::any visit(MatchCase* node) override = 0;
    virtual std::any visit(AggFunc* node) override = 0;
    virtual std::any visit(BAggFunc* node) override = 0;
    virtual std::any visit(MultiCount* node) override = 0;
    virtual std::any visit(Windowing* node) override = 0;

    virtual std::any visit(MkList* node) override = 0;
    virtual std::any visit(MkMap* node) override = 0;
    virtual std::any visit(MkRecord* node) override = 0;
    virtual std::any visit(MkSet* node) override = 0;
    virtual std::any visit(MkTuple* node) override = 0;

    virtual std::any visit(VBool* node) override = 0;
    virtual std::any visit(VInt* node) override = 0;
    virtual std::any visit(VDouble* node) override = 0;
    virtual std::any visit(VString* node) override = 0;
    virtual std::any visit(VDate* node) override = 0;
    virtual std::any visit(VDatetime* node) override = 0;
    virtual std::any visit(VDuration* node) override = 0;
    virtual std::any visit(VTime* node) override = 0;
    virtual std::any visit(VNull* node) override = 0;
    virtual std::any visit(VNone* node) override = 0;
    virtual std::any visit(Ref* node) override = 0;
    virtual std::any visit(Param* node) override = 0;

    // predicates
    virtual std::any visit(IsNull*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(IsDirected*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(IsNormalized*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(IsSourceOf*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(IsDestinationOf*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(IsLabeled*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Same*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(AllDifferent*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Exists*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    //---------------------------------------------------------------------------------
    // stmt
    virtual std::any visit(ExplainActivity*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SessionActivity*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(TransactionActivity*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(FullTransaction*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(NormalTransaction*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(StartTransaction*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(CommitTransaction*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(RollBackTransaction*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SessionSet*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SessionReset*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ProcedureBody*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SchemaFromPath*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BindingValue*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BindingGraph*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BindingTable*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BindingTableInnerQuery*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BindingTableInnerExpr*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(StatementWithYield*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(QueryStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(StandaloneCallStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(JoinQueryExpression*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(JoinRightPart*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(CompositeQueryStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(AmbientLinearQueryStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SelectStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(FocusedQueryStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(FocusedResultStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(MatchStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(FilterStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(CallQueryStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(CallProcedureStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(InlineProcedureCall*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(NamedProcedureCall*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ForStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(PrimitiveResultStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(CatalogModifyStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(LinearDataModifyingStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(InsertStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ReplaceStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(MergeStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SetStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(DeleteStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(RemoveStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(OtherWise*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Union*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Except*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Intersect*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ShowProcessListStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(KillStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ManagerStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(UnwindStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(InQueryProcedureCall*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    virtual std::any visit(DummyNode*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

 protected:
    virtual std::any reportError() override = 0;
};  // class AstExprNodeVisitorImpl

class AstLabelTreeNodeVisitorImpl : public AstNodeVisitor {
 public:
    AstLabelTreeNodeVisitorImpl() = default;
    virtual ~AstLabelTreeNodeVisitorImpl() = default;

    //---------------------------------------------------------------------------------
    // patterns
    virtual std::any visit(GraphPattern*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(PathPattern*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(PathChain*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Node*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Edge*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ElementFiller*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    //---------------------------------------------------------------------------------
    // clauses
    virtual std::any visit(WhereClause*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(OrderByField*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(PathModePrefix*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(PathSearchPrefix*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SingleLabel* node) override = 0;
    virtual std::any visit(LabelOr* node) override = 0;
    virtual std::any visit(LabelAnd* node) override = 0;
    virtual std::any visit(LabelNot* node) override = 0;
    virtual std::any visit(PropStruct*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(YieldField*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(TableFunctionClause*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ReadConsistency*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(AllowAnonymousTable*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(OpConcurrent*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(EdgeOnJoin*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SetAllProperties*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(UpdateProperties*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SetLabel*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SetSingleProperty*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SetSchemaClause*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SetGraphClause*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SetTimeZoneClause*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SetParamClause*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ResetAll*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ResetSchema*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ResetTimeZone*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ResetGraph*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ResetParam*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    //---------------------------------------------------------------------------------
    // exprs
    virtual std::any visit(GetField*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(TupleGet*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Not*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Neg*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Tilde*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(VSome*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    virtual std::any visit(BEqual*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BNotEqual*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BGreaterThan*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BNotSmallerThan*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BSmallerThan*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BNotGreaterThan*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BSafeEqual*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BAdd*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BSub*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BDiv*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BMul*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BMod*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BSquare*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BAnd*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BOr*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BXor*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BBitAnd*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BBitOr*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BBitXor*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BBitLeftShift*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BBitRightShift*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BConcat*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BIndex*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BLike*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BIn*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    virtual std::any visit(If*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Function*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Case*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Cast*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(MatchCase*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(AggFunc*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BAggFunc*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(MultiCount*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Windowing*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    virtual std::any visit(MkList*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(MkMap*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(MkRecord*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(MkSet*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(MkTuple*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    virtual std::any visit(VBool*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(VInt*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(VDouble*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(VString*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(VDate*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(VDatetime*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(VDuration*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(VTime*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(VNull*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(VNone*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Ref*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Param*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    // predicates
    virtual std::any visit(IsNull*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(IsDirected*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(IsNormalized*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(IsSourceOf*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(IsDestinationOf*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(IsLabeled*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Same*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(AllDifferent*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Exists*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    //---------------------------------------------------------------------------------
    // stmt
    virtual std::any visit(ExplainActivity*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SessionActivity*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(TransactionActivity*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(FullTransaction*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(NormalTransaction*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(StartTransaction*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(CommitTransaction*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(RollBackTransaction*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SessionSet*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SessionReset*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ProcedureBody*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SchemaFromPath*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BindingValue*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BindingGraph*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BindingTable*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BindingTableInnerQuery*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(BindingTableInnerExpr*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(StatementWithYield*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(QueryStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(StandaloneCallStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(JoinQueryExpression*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(JoinRightPart*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(CompositeQueryStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(AmbientLinearQueryStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SelectStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(FocusedQueryStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(FocusedResultStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(MatchStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(FilterStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(CallQueryStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(CallProcedureStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(InlineProcedureCall*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(NamedProcedureCall*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ForStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(PrimitiveResultStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(CatalogModifyStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(LinearDataModifyingStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(InsertStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ReplaceStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(MergeStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(SetStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(DeleteStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(RemoveStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(OtherWise*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Union*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Except*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(Intersect*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ShowProcessListStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(KillStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(ManagerStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(UnwindStatement*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    virtual std::any visit(InQueryProcedureCall*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    virtual std::any visit(DummyNode*) override {
        return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

 protected:
    virtual std::any reportError() override = 0;
};  // class AstLabelTreeNodeVisitorImpl

}  // namespace geax::frontend

#endif  // GEAXFRONTEND_AST_ASTNODEVISITOR_H_
