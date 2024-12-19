/**
 * Copyright 2023 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License") {}
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

#include "cypher/utils/ast_node_visitor_impl.h"
#include "cypher/cypher_exception.h"
#include "utils/geax_util.h"

namespace cypher {

class OptimizationFilterVisitorImpl : public geax::frontend::AstNodeVisitor {
 public:
    OptimizationFilterVisitorImpl() = default;

    ~OptimizationFilterVisitorImpl() override = default;

    std::any reportError() override {
        return error_msg_;
    }

 private:
    std::any visit(geax::frontend::GetField* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::TupleGet* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::Not* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::Neg* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::Tilde* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::VSome* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BEqual* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BNotEqual* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BGreaterThan* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BNotSmallerThan* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BSmallerThan* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BNotGreaterThan* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BSafeEqual* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BAdd* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BSub* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BDiv* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BMul* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BMod* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BSquare* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BAnd* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BOr* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BXor* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BBitAnd* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BBitOr* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BBitXor* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BBitLeftShift* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BBitRightShift* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BConcat* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BIndex* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BLike* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BIn* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::If* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::Function* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::Case* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::Cast* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::MatchCase* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::AggFunc* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BAggFunc* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::MultiCount* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::Windowing* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::MkList* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::MkMap* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::MkRecord* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::MkSet* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::MkTuple* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::VBool* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::VInt* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::VDouble* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::VString* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::VDate* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::VDatetime* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::VDuration* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::VTime* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::VNull* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::VNone* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::Ref* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::GraphPattern* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::PathPattern* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::PathChain* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::Node* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::Edge* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::ElementFiller* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::WhereClause* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::OrderByField* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::PathModePrefix* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::PathSearchPrefix* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::SingleLabel* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::LabelOr* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::LabelAnd* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::LabelNot* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::PropStruct* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::YieldField* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::TableFunctionClause* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::ReadConsistency* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::AllowAnonymousTable* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::OpConcurrent* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::EdgeOnJoin* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::SetAllProperties* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::SetLabel* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::IsNull* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::IsDirected* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::IsNormalized* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::IsSourceOf* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::IsDestinationOf* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::IsLabeled* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::Same* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::AllDifferent* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::Exists* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::ProcedureBody* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::SchemaFromPath* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BindingValue* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BindingGraph* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BindingTable* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::StatementWithYield* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::QueryStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::JoinQueryExpression* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::JoinRightPart* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::CompositeQueryStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::AmbientLinearQueryStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::SelectStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::FocusedQueryStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::FocusedResultStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::MatchStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::FilterStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::PrimitiveResultStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::CatalogModifyStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::LinearDataModifyingStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::InsertStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::ReplaceStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::SetStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::DeleteStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::RemoveStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::OtherWise* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::Union* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::Except* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::Intersect* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::UpdateProperties* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::SetSingleProperty* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::RemoveSingleProperty* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::SetSchemaClause* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::SetGraphClause* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::SetTimeZoneClause* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::SetParamClause* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::ResetAll* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::ResetSchema* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::ResetTimeZone* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::ResetGraph* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::ResetParam* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::ExplainActivity* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::SessionActivity* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::TransactionActivity* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::FullTransaction* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::NormalTransaction* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::StartTransaction* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::CommitTransaction* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::RollBackTransaction* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::SessionSet* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::SessionReset* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BindingTableInnerQuery* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::BindingTableInnerExpr* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::StandaloneCallStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::CallQueryStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::CallProcedureStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::InlineProcedureCall* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::NamedProcedureCall* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::ForStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::MergeStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::Param* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::ShowProcessListStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::KillStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::ManagerStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::UnwindStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::InQueryProcedureCall* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::DummyNode* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any visit(geax::frontend::ListComprehension* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

 protected:
    std::string error_msg_;
};

}  // namespace cypher
