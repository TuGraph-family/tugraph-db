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

namespace geax {
namespace frontend {

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

class ProcedureBody;
class StatementWithYield;
class QueryStatement;
class JoinQueryExpression;
class JoinRightPart;
class CompositeQueryStatement;
class AmbientLinearQueryStatement;
class SelectStatement;
class FocusedQueryStatement;
class FocusedResultStatement;
class MatchStatement;
class FilterStatement;
class PrimitiveResultStatement;
class CatalogModifyStatement;
class LinearDataModifyingStatement;
class InsertStatement;
class ReplaceStatement;
class SetStatement;
class DeleteStatement;
class RemoveStatement;
class ShowProcessListStatement;
class KillStatement;
class ManagerStatement;

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
    virtual std::any visit(SetAllProperties* node) = 0;
    virtual std::any visit(UpdateProperties* node) = 0;
    virtual std::any visit(SetLabel* node) = 0;
    virtual std::any visit(SetSingleProperty* node) = 0;

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
    virtual std::any visit(ProcedureBody* node) = 0;
    virtual std::any visit(SchemaFromPath* node) = 0;
    virtual std::any visit(BindingValue* node) = 0;
    virtual std::any visit(BindingGraph* node) = 0;
    virtual std::any visit(BindingTable* node) = 0;
    virtual std::any visit(StatementWithYield* node) = 0;
    virtual std::any visit(QueryStatement* node) = 0;
    virtual std::any visit(JoinQueryExpression* node) = 0;
    virtual std::any visit(JoinRightPart* node) = 0;
    virtual std::any visit(CompositeQueryStatement* node) = 0;
    virtual std::any visit(AmbientLinearQueryStatement* node) = 0;
    virtual std::any visit(SelectStatement* node) = 0;
    virtual std::any visit(FocusedQueryStatement* node) = 0;
    virtual std::any visit(FocusedResultStatement* node) = 0;
    virtual std::any visit(MatchStatement* node) = 0;
    virtual std::any visit(FilterStatement* node) = 0;
    virtual std::any visit(PrimitiveResultStatement* node) = 0;
    virtual std::any visit(CatalogModifyStatement* node) = 0;
    virtual std::any visit(LinearDataModifyingStatement* node) = 0;
    virtual std::any visit(InsertStatement* node) = 0;
    virtual std::any visit(ReplaceStatement* node) = 0;
    virtual std::any visit(SetStatement* node) = 0;
    virtual std::any visit(DeleteStatement* node) = 0;
    virtual std::any visit(RemoveStatement* node) = 0;
    virtual std::any visit(OtherWise* node) = 0;
    virtual std::any visit(Union* node) = 0;
    virtual std::any visit(Except* node) = 0;
    virtual std::any visit(Intersect* node) = 0;
    virtual std::any visit(ShowProcessListStatement* node) = 0;
    virtual std::any visit(KillStatement* node) = 0;
    virtual std::any visit(ManagerStatement* node) = 0;

    virtual std::any visit(DummyNode* node) = 0;

protected:
    virtual std::any reportError() = 0;
};  // class AstNodeVisitor

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_ASTNODEVISITOR_H_
