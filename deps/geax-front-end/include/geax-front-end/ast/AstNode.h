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

#ifndef GEAXFRONTEND_AST_ASTNODE_H_
#define GEAXFRONTEND_AST_ASTNODE_H_

#include <array>
#include <string_view>
#include <unordered_map>

#include "geax-front-end/ast/AstCommon.h"
#include "geax-front-end/ast/AstNodeVisitor.h"

namespace geax {
namespace frontend {

template <typename E>
constexpr auto enumNum(E e) {
    return static_cast<std::underlying_type_t<E>>(e);
}

template <size_t N>
using StrArray = std::array<const char* const, N>;

// Please add your ast node types here
#define MAKE_AST_NODE_TYPE(TYPE)                                                          \
    TYPE(ExplainActivity, kExplainActivity, "ExplainActivity")                            \
    TYPE(SessionActivity, kSessionActivity, "SessionActivity")                            \
    TYPE(SessionSet, kSessionSet, "SessionSet")                                           \
    TYPE(SetSchemaClause, kSetSchema, "SetSchema")                                        \
    TYPE(SetGraphClause, kSetGraph, "SetGraph")                                           \
    TYPE(SetTimeZoneClause, kSetTimeZone, "SetTimeZone")                                  \
    TYPE(SetParamClause, kSetParam, "SetParam")                                           \
    TYPE(SessionReset, kSessionReset, "SessionReset")                                     \
    TYPE(ResetAll, kResetAll, "ResetAll")                                                 \
    TYPE(ResetGraph, kResetGraph, "ResetGraph")                                           \
    TYPE(ResetSchema, kResetSchema, "ResetSchema")                                        \
    TYPE(ResetTimeZone, kResetTimeZone, "ResetTimeZone")                                  \
    TYPE(ResetParam, kResetParam, "ResetParam")                                           \
    TYPE(TransactionActivity, kTransactionActivity, "TransactionActivity")                \
    TYPE(FullTransaction, kFullTransaction, "FullTransaction")                            \
    TYPE(NormalTransaction, kNormalTransaction, "NormalTransaction")                      \
    TYPE(StartTransaction, kStartTransaction, "StartTransaction")                         \
    TYPE(CommitTransaction, kCommitTransaction, "CommitTransaction")                      \
    TYPE(RollBackTransaction, kRollBackTransaction, "RollBackTransaction")                \
    TYPE(ProcedureBody, kProcedureBody, "ProcedureBody")                                  \
    TYPE(SchemaFromPath, kSchemaFromPath, "SchemaFromPath")                               \
    TYPE(BindingGraph, kBindingGraph, "BindingGraph")                                     \
    TYPE(BindingTable, kBindingTable, "BindingTable")                                     \
    TYPE(BindingValue, kBindingValue, "BindingValue")                                     \
    TYPE(BindingTableInnerQuery, kBindingTableInnerQuery, "BindingTableInnerQuery")       \
    TYPE(BindingTableInnerExpr, kBindingTableInnerExpr, "BindingTableInnerExpr")          \
    TYPE(StatementWithYield, kStatementWithYield, "StatementWithYield")                   \
    TYPE(QueryStatement, kQueryStatement, "QueryStatement")                               \
    TYPE(StandaloneCallStatement, kStandaloneCallStatement, "StandaloneCallStatement")    \
    TYPE(JoinQueryExpression, kJoinQuery, "JoinQuery")                                    \
    TYPE(JoinRightPart, kJoinRight, "JoinRight")                                          \
    TYPE(CompositeQueryStatement, kCompositeStatement, "CompositeStatement")              \
    TYPE(AmbientLinearQueryStatement, kAmbientStatement, "AmbientStatement")              \
    TYPE(SelectStatement, kSelectStatement, "SelectStatement")                            \
    TYPE(FocusedQueryStatement, kFocusedQueryStatement, "FocusedQueryStatement")          \
    TYPE(FocusedResultStatement, kFocusedResultStatement, "FocusedResultStatement")       \
    TYPE(MatchStatement, kMatchStatement, "MatchStatement")                               \
    TYPE(FilterStatement, kFilterStatement, "FilterStatement")                            \
    TYPE(ForStatement, kForStatement, "ForStatement")                                     \
    TYPE(CallQueryStatement, kCallQueryStatement, "CallQueryStatement")                   \
    TYPE(CallProcedureStatement, kCallProcedureStatement, "CallProcedureStatement")       \
    TYPE(InlineProcedureCall, kInlineProcedureCall, "InlineProcedureCall")                \
    TYPE(NamedProcedureCall, kNamedProcedureCall, "NamedProcedureCall")                   \
    TYPE(PrimitiveResultStatement, kPrimitiveResultStatement, "PrimitiveResultStatement") \
    TYPE(CatalogModifyStatement, kCatalogModifyStatement, "CatalogModifyStatement")       \
    TYPE(ManagerStatement, kManagerStatement, "ManagerStatement")                         \
    TYPE(ShowProcessListStatement, kShowProcessListStatement, "ShowProcessListStatement") \
    TYPE(KillStatement, kKillStatement, "KillStatement")                                  \
    TYPE(LinearDataModifyingStatement, kDataModifyStatement, "DataModifyStatement")       \
    TYPE(DeleteStatement, kDeleteStatement, "DeleteStatement")                            \
    TYPE(InsertStatement, kInsertStatement, "InsertStatement")                            \
    TYPE(ReplaceStatement, kReplaceStatement, "ReplaceStatement")                         \
    TYPE(SetStatement, kSetStatement, "SetStatement")                                     \
    TYPE(RemoveStatement, kRemoveStatement, "RemoveStatement")                            \
    TYPE(MergeStatement, kMergeStatement, "MergeStatement")                               \
    TYPE(OtherWise, kOtherWise, "OtherWise")                                              \
    TYPE(Union, kUnion, "Union")                                                          \
    TYPE(Except, kExcept, "Except")                                                       \
    TYPE(Intersect, kIntersect, "Intersect")                                              \
    TYPE(OrderByField, kOrderByField, "OrderByField")                                     \
    TYPE(ReadConsistency, kReadConsistency, "ReadConsistency")                            \
    TYPE(AllowAnonymousTable, kAllowAnonymousTable, "AllowAnonymousTable")                \
    TYPE(EdgeOnJoin, kEdgeOnJoin, "EdgeOnJoin")                                           \
    TYPE(OpConcurrent, kOpConcurrent, "OpConcurrent")                                     \
    TYPE(Node, kNode, "Node")                                                             \
    TYPE(Edge, kEdge, "Edge")                                                             \
    TYPE(PathPattern, kPathPattern, "PathPattern")                                        \
    TYPE(PathChain, kPathChain, "PathChain")                                              \
    TYPE(GraphPattern, kGraphPattern, "GraphPattern")                                     \
    TYPE(ElementFiller, kElementFiller, "ElementFiller")                                  \
    TYPE(PathModePrefix, kPathModePrefix, "PathModePrefix")                               \
    TYPE(PathSearchPrefix, kPathSearchPrefix, "PathSearchPrefix")                         \
    TYPE(SingleLabel, kSingleLabel, "SingleLabel")                                        \
    TYPE(LabelOr, kLabelOr, "LabelOr")                                                    \
    TYPE(LabelAnd, kLabelAnd, "LabelAnd")                                                 \
    TYPE(LabelNot, kLabelNot, "LabelNot")                                                 \
    TYPE(PropStruct, kPropStruct, "PropStruct")                                           \
    TYPE(YieldField, kYieldField, "YieldField")                                           \
    TYPE(WhereClause, kWhere, "Where")                                                    \
    TYPE(TableFunctionClause, kTableFunction, "TableFunctionClause")                      \
    TYPE(SetAllProperties, kSetAllProperties, "SetAllProperties")                         \
    TYPE(UpdateProperties, kUpdateProperties, "UpdateProperties")                         \
    TYPE(SetLabel, kSetLabel, "SetLabel")                                                 \
    TYPE(SetSingleProperty, kSetSingleProperty, "SetSingleProperty")                      \
    TYPE(Not, kNot, "Not")                                                                \
    TYPE(Neg, kNeg, "Neg")                                                                \
    TYPE(Tilde, kTilde, "Tilde")                                                          \
    TYPE(VSome, kVSome, "VSome")                                                          \
    TYPE(GetField, kGetField, "GetField")                                                 \
    TYPE(TupleGet, kTupleGet, "TupleGet")                                                 \
    TYPE(BEqual, kBEqual, "BEqual")                                                       \
    TYPE(BNotEqual, kBNotEqual, "BNotEqual")                                              \
    TYPE(BSafeEqual, kBSafeEqual, "BSafeEqual")                                           \
    TYPE(BSmallerThan, kBSmallerThan, "BSmallerThan")                                     \
    TYPE(BGreaterThan, kBGreaterThan, "BGreaterThan")                                     \
    TYPE(BNotGreaterThan, kBNotGreaterThan, "BNotGreaterThan")                            \
    TYPE(BNotSmallerThan, kBNotSmallerThan, "BNotSmallerThan")                            \
    TYPE(BAdd, kBAdd, "BAdd")                                                             \
    TYPE(BSub, kBSub, "BSub")                                                             \
    TYPE(BMul, kBMul, "BMul")                                                             \
    TYPE(BDiv, kBDiv, "BDiv")                                                             \
    TYPE(BMod, kBMod, "BMod")                                                             \
    TYPE(BSquare, kBSquare, "BSquare")                                                    \
    TYPE(BOr, kBOr, "BOr")                                                                \
    TYPE(BXor, kBXor, "BXor")                                                             \
    TYPE(BAnd, kBAnd, "BAnd")                                                             \
    TYPE(BBitAnd, kBBitAnd, "BBitAnd")                                                    \
    TYPE(BBitOr, kBBitOr, "BBitOr")                                                       \
    TYPE(BBitXor, kBBitXor, "BBitXor")                                                    \
    TYPE(BBitLeftShift, kBBitLeftShift, "BBitLeftShift")                                  \
    TYPE(BBitRightShift, kBBitRightShift, "BBitRightShift")                               \
    TYPE(BConcat, kBConcat, "BConcat")                                                    \
    TYPE(BIndex, kBIndex, "BIndex")                                                       \
    TYPE(BLike, kBLike, "BLike")                                                          \
    TYPE(BIn, kBIn, "BIn")                                                                \
    TYPE(IsNull, kIsNull, "IsNull")                                                       \
    TYPE(IsDirected, kIsDirected, "IsDirected")                                           \
    TYPE(IsNormalized, kIsNormalized, "IsNormalized")                                     \
    TYPE(IsSourceOf, kIsSourceOf, "IsSourceOf")                                           \
    TYPE(IsDestinationOf, kIsDestinationOf, "IsDestinationOf")                            \
    TYPE(AllDifferent, kAllDifferent, "AllDifferent")                                     \
    TYPE(IsLabeled, kIsLabeled, "IsLabeled")                                              \
    TYPE(Exists, kExists, "Exists")                                                       \
    TYPE(Same, kSame, "Same")                                                             \
    TYPE(Cast, kCast, "Cast")                                                             \
    TYPE(If, kIf, "If")                                                                   \
    TYPE(Function, kFunc, "Func")                                                         \
    TYPE(Case, kCase, "Case")                                                             \
    TYPE(MatchCase, kMatchCase, "MatchCase")                                              \
    TYPE(Windowing, kWindowing, "Windowing")                                              \
    TYPE(AggFunc, kAggFunc, "AggFunc")                                                    \
    TYPE(BAggFunc, kBAggFunc, "BAggFunc")                                                 \
    TYPE(MultiCount, kMultiCount, "MultiCount")                                           \
    TYPE(Ref, kRef, "Ref")                                                                \
    TYPE(Param, kParam, "Param")                                                          \
    TYPE(VString, kVString, "VString")                                                    \
    TYPE(VInt, kVInt, "VInt")                                                             \
    TYPE(VDouble, kVDouble, "VDouble")                                                    \
    TYPE(VBool, kVBool, "VBool")                                                          \
    TYPE(VDate, kVDate, "VDate")                                                          \
    TYPE(VDatetime, kVDatetime, "VDatetime")                                              \
    TYPE(VDuration, kVDuration, "VDuration")                                              \
    TYPE(VTime, kVTime, "VTime")                                                          \
    TYPE(VNull, kVNull, "VNull")                                                          \
    TYPE(VNone, kVNone, "VNone")                                                          \
    TYPE(MkList, kMkList, "MkList")                                                       \
    TYPE(MkMap, kMkMap, "MkMap")                                                          \
    TYPE(MkRecord, kMkRecord, "MkRecord")                                                 \
    TYPE(MkSet, kMkSet, "MkSet")                                                          \
    TYPE(MkTuple, kMkTuple, "MkTuple")                                                    \
    TYPE(UnwindStatement, kUnwindStatement, "UnwindStatement")                            \
    TYPE(InQueryProcedureCall, kInQueryProcedureCall, "InQueryProcedureCall")             \
    TYPE(DummyNode, kNotDefined, "NotDefined")
// This should always be the last one

/**
 * An enum to specify the type of an AstNode.
 *
 * It's automatically generated by above macro, so please add your
 * newly defined AstNodeType there.
 */
enum class AstNodeType : uint32_t {
#define TYPE(NODE, NAME, DOC) NAME,
    MAKE_AST_NODE_TYPE(TYPE)
#undef TYPE
};
inline const char* ToString(AstNodeType type) {
    static const StrArray<enumNum(AstNodeType::kNotDefined) + 1> kNameArray = {
#define TYPE(NODE, NAME, DOC) DOC,
        MAKE_AST_NODE_TYPE(TYPE)
#undef TYPE
    };
    const auto idx = static_cast<size_t>(type);
    return idx < kNameArray.size() ? kNameArray[idx] : geax::frontend::kUnknown;
}
inline bool ToEnum(std::string_view sv, AstNodeType& type) {
    static const std::unordered_map<std::string_view, AstNodeType> kTypeMap = {
#define TYPE(NODE, NAME, DOC) {DOC, AstNodeType::NAME},
        MAKE_AST_NODE_TYPE(TYPE)
#undef TYPE
    };
    auto it = kTypeMap.find(sv);
    return it == kTypeMap.end() ? false : (type = it->second, true);
}

/**
 * An AstNode represents a node in the AST.
 */
class AstNode {
public:
    explicit AstNode(AstNodeType type) : type_(type) {}
    virtual ~AstNode() = default;

    AstNodeType type() const { return type_; }

    virtual std::any accept(AstNodeVisitor& visitor) = 0;

    // Get AstNodeType from its name
    static AstNodeType getTypeByName(const std::string& name) {
        AstNodeType type = AstNodeType::kNotDefined;
        ToEnum(name, type);
        return type;
    }

    inline static constexpr const char* kType = "type";

protected:
    AstNodeType type_;
};

/**
 * A DummyNode is just a non-op node.
 */
class DummyNode : public AstNode {
public:
    DummyNode() : AstNode(AstNodeType::kNotDefined) {}
    ~DummyNode() = default;

    std::any accept(AstNodeVisitor&) override { return GEAXErrorCode::GEAX_SUCCEED; }
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_ASTNODE_H_
