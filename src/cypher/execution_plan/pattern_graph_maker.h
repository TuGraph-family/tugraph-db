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
 */

#pragma once

#include <vector>
#include "geax-front-end/ast/AstNode.h"
#include "cypher/graph/graph.h"

namespace cypher {
// TODO(lingsu): rename in the future
class PatternGraphMaker : public geax::frontend::AstNodeVisitor {
 public:
    explicit PatternGraphMaker(std::vector<PatternGraph>& pattern_graphs)
        : pattern_graphs_(pattern_graphs) {}

    geax::frontend::GEAXErrorCode Build(geax::frontend::AstNode* astNode, RTContext* ctx);
    std::string ErrorMsg() { return error_msg_; }

 private:
    std::any visit(geax::frontend::GraphPattern* node) override;
    std::any visit(geax::frontend::PathPattern* node) override;
    std::any visit(geax::frontend::PathChain* node) override;
    std::any visit(geax::frontend::Node* node) override;
    std::any visit(geax::frontend::Edge* node) override;
    std::any visit(geax::frontend::ElementFiller* node) override;

    //---------------------------------------------------------------------------------
    // clauses
    std::any visit(geax::frontend::WhereClause* node) override;
    std::any visit(geax::frontend::OrderByField* node) override;
    std::any visit(geax::frontend::PathModePrefix* node) override;
    std::any visit(geax::frontend::PathSearchPrefix* node) override;
    std::any visit(geax::frontend::SingleLabel* node) override;
    std::any visit(geax::frontend::LabelOr* node) override;
    std::any visit(geax::frontend::LabelAnd* node) override;
    std::any visit(geax::frontend::LabelNot* node) override;
    std::any visit(geax::frontend::PropStruct* node) override;
    std::any visit(geax::frontend::YieldField* node) override;
    std::any visit(geax::frontend::TableFunctionClause* node) override;
    std::any visit(geax::frontend::ReadConsistency* node) override;
    std::any visit(geax::frontend::AllowAnonymousTable* node) override;
    std::any visit(geax::frontend::OpConcurrent* node) override;
    std::any visit(geax::frontend::EdgeOnJoin* node) override;
    std::any visit(geax::frontend::SetAllProperties* node) override;
    std::any visit(geax::frontend::UpdateProperties* node) override;
    std::any visit(geax::frontend::SetLabel* node) override;
    std::any visit(geax::frontend::SetSingleProperty* node) override;
    std::any visit(geax::frontend::SetSchemaClause* node) override;
    std::any visit(geax::frontend::SetGraphClause* node) override;
    std::any visit(geax::frontend::SetTimeZoneClause* node) override;
    std::any visit(geax::frontend::SetParamClause* node) override;
    std::any visit(geax::frontend::ResetAll* node) override;
    std::any visit(geax::frontend::ResetSchema* node) override;
    std::any visit(geax::frontend::ResetTimeZone* node) override;
    std::any visit(geax::frontend::ResetGraph* node) override;
    std::any visit(geax::frontend::ResetParam* node) override;

    //---------------------------------------------------------------------------------
    // exprs
    std::any visit(geax::frontend::GetField* node) override;
    std::any visit(geax::frontend::TupleGet* node) override;
    std::any visit(geax::frontend::Not* node) override;
    std::any visit(geax::frontend::Neg* node) override;
    std::any visit(geax::frontend::Tilde* node) override;
    std::any visit(geax::frontend::VSome* node) override;

    std::any visit(geax::frontend::BEqual* node) override;
    std::any visit(geax::frontend::BNotEqual* node) override;
    std::any visit(geax::frontend::BGreaterThan* node) override;
    std::any visit(geax::frontend::BNotSmallerThan* node) override;
    std::any visit(geax::frontend::BSmallerThan* node) override;
    std::any visit(geax::frontend::BNotGreaterThan* node) override;
    std::any visit(geax::frontend::BSafeEqual* node) override;
    std::any visit(geax::frontend::BAdd* node) override;
    std::any visit(geax::frontend::BSub* node) override;
    std::any visit(geax::frontend::BDiv* node) override;
    std::any visit(geax::frontend::BMul* node) override;
    std::any visit(geax::frontend::BMod* node) override;
    std::any visit(geax::frontend::BSquare* node) override;
    std::any visit(geax::frontend::BAnd* node) override;
    std::any visit(geax::frontend::BOr* node) override;
    std::any visit(geax::frontend::BXor* node) override;
    std::any visit(geax::frontend::BBitAnd* node) override;
    std::any visit(geax::frontend::BBitOr* node) override;
    std::any visit(geax::frontend::BBitXor* node) override;
    std::any visit(geax::frontend::BBitLeftShift* node) override;
    std::any visit(geax::frontend::BBitRightShift* node) override;
    std::any visit(geax::frontend::BConcat* node) override;
    std::any visit(geax::frontend::BIndex* node) override;
    std::any visit(geax::frontend::BLike* node) override;
    std::any visit(geax::frontend::BIn* node) override;

    std::any visit(geax::frontend::If* node) override;
    std::any visit(geax::frontend::Function* node) override;
    std::any visit(geax::frontend::Case* node) override;
    std::any visit(geax::frontend::Cast* node) override;
    std::any visit(geax::frontend::MatchCase* node) override;
    std::any visit(geax::frontend::AggFunc* node) override;
    std::any visit(geax::frontend::BAggFunc* node) override;
    std::any visit(geax::frontend::MultiCount* node) override;
    std::any visit(geax::frontend::Windowing* node) override;

    std::any visit(geax::frontend::MkList* node) override;
    std::any visit(geax::frontend::MkMap* node) override;
    std::any visit(geax::frontend::MkRecord* node) override;
    std::any visit(geax::frontend::MkSet* node) override;
    std::any visit(geax::frontend::MkTuple* node) override;

    std::any visit(geax::frontend::VBool* node) override;
    std::any visit(geax::frontend::VInt* node) override;
    std::any visit(geax::frontend::VDouble* node) override;
    std::any visit(geax::frontend::VString* node) override;
    std::any visit(geax::frontend::VDate* node) override;
    std::any visit(geax::frontend::VDatetime* node) override;
    std::any visit(geax::frontend::VDuration* node) override;
    std::any visit(geax::frontend::VTime* node) override;
    std::any visit(geax::frontend::VNull* node) override;
    std::any visit(geax::frontend::VNone* node) override;
    std::any visit(geax::frontend::Ref* node) override;
    std::any visit(geax::frontend::Param* node) override;

    // predicates
    std::any visit(geax::frontend::IsNull* node) override;
    std::any visit(geax::frontend::IsDirected* node) override;
    std::any visit(geax::frontend::IsNormalized* node) override;
    std::any visit(geax::frontend::IsSourceOf* node) override;
    std::any visit(geax::frontend::IsDestinationOf* node) override;
    std::any visit(geax::frontend::IsLabeled* node) override;
    std::any visit(geax::frontend::Same* node) override;
    std::any visit(geax::frontend::AllDifferent* node) override;
    std::any visit(geax::frontend::Exists* node) override;

    //---------------------------------------------------------------------------------
    // stmt
    std::any visit(geax::frontend::ExplainActivity* node) override;
    std::any visit(geax::frontend::SessionActivity* node) override;
    std::any visit(geax::frontend::TransactionActivity* node) override;
    std::any visit(geax::frontend::FullTransaction* node) override;
    std::any visit(geax::frontend::NormalTransaction* node) override;
    std::any visit(geax::frontend::StartTransaction* node) override;
    std::any visit(geax::frontend::CommitTransaction* node) override;
    std::any visit(geax::frontend::RollBackTransaction* node) override;
    std::any visit(geax::frontend::SessionSet* node) override;
    std::any visit(geax::frontend::SessionReset* node) override;
    std::any visit(geax::frontend::ProcedureBody* node) override;
    std::any visit(geax::frontend::SchemaFromPath* node) override;
    std::any visit(geax::frontend::BindingValue* node) override;
    std::any visit(geax::frontend::BindingGraph* node) override;
    std::any visit(geax::frontend::BindingTable* node) override;
    std::any visit(geax::frontend::BindingTableInnerQuery* node) override;
    std::any visit(geax::frontend::BindingTableInnerExpr* node) override;
    std::any visit(geax::frontend::StatementWithYield* node) override;
    std::any visit(geax::frontend::QueryStatement* node) override;
    std::any visit(geax::frontend::StandaloneCallStatement* node) override;
    std::any visit(geax::frontend::JoinQueryExpression* node) override;
    std::any visit(geax::frontend::JoinRightPart* node) override;
    std::any visit(geax::frontend::CompositeQueryStatement* node) override;
    std::any visit(geax::frontend::AmbientLinearQueryStatement* node) override;
    std::any visit(geax::frontend::SelectStatement* node) override;
    std::any visit(geax::frontend::FocusedQueryStatement* node) override;
    std::any visit(geax::frontend::FocusedResultStatement* node) override;
    std::any visit(geax::frontend::MatchStatement* node) override;
    std::any visit(geax::frontend::FilterStatement* node) override;
    std::any visit(geax::frontend::CallQueryStatement* node) override;
    std::any visit(geax::frontend::CallProcedureStatement* node) override;
    std::any visit(geax::frontend::InlineProcedureCall* node) override;
    std::any visit(geax::frontend::NamedProcedureCall* node) override;
    std::any visit(geax::frontend::ForStatement* node) override;
    std::any visit(geax::frontend::PrimitiveResultStatement* node) override;
    std::any visit(geax::frontend::CatalogModifyStatement* node) override;
    std::any visit(geax::frontend::LinearDataModifyingStatement* node) override;
    std::any visit(geax::frontend::InsertStatement* node) override;
    std::any visit(geax::frontend::ReplaceStatement* node) override;
    std::any visit(geax::frontend::SetStatement* node) override;
    std::any visit(geax::frontend::DeleteStatement* node) override;
    std::any visit(geax::frontend::RemoveStatement* node) override;
    std::any visit(geax::frontend::MergeStatement* node) override;
    std::any visit(geax::frontend::OtherWise* node) override;
    std::any visit(geax::frontend::Union* node) override;
    std::any visit(geax::frontend::Except* node) override;
    std::any visit(geax::frontend::Intersect* node) override;
    std::any visit(geax::frontend::ShowProcessListStatement* node) override;
    std::any visit(geax::frontend::KillStatement* node) override;
    std::any visit(geax::frontend::ManagerStatement* node) override;
    std::any visit(geax::frontend::UnwindStatement* node) override;
    std::any visit(geax::frontend::InQueryProcedureCall* node) override;

    std::any visit(geax::frontend::DummyNode* node) override;
    std::any reportError() override;

 private:
    void AddSymbol(const std::string& symbol_alias, cypher::SymbolNode::Type type,
                   cypher::SymbolNode::Scope scope);
    void AddNode(Node* node);
    void AddRelationship(Relationship* rel);
    std::vector<PatternGraph>& pattern_graphs_;
    std::vector<size_t> symbols_idx_;
    size_t cur_pattern_graph_;
    std::unordered_set<geax::frontend::AstNodeType> cur_types_;
    bool read_only_ = true;
    std::string error_msg_;
    std::string curr_procedure_name_;
    RTContext* ctx_;

    std::shared_ptr<Node> node_t_;
    std::shared_ptr<Node> start_t_;
    std::shared_ptr<Relationship> relp_t_;
};
}  // namespace cypher
