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

#include <any>
#include <memory>
#include <string>
#include <utility>

#include "geax-front-end/ast/Ast.h"
#include "geax-front-end/ast/expr/AggFunc.h"
#include "cypher/arithmetic/agg_ctx.h"
#include "cypher/arithmetic/ast_agg_expr_detector.h"
#include "cypher/execution_plan/visitor/visitor.h"
#include "cypher/resultset/record.h"
#include "cypher/parser/symbol_table.h"
#include "cypher/cypher_types.h"
#include "core/data_type.h"

namespace cypher {

class AstExprEvaluator : public geax::frontend::AstNodeVisitor {
 public:
    AstExprEvaluator() = delete;

    AstExprEvaluator(geax::frontend::Expr* expr, const SymbolTable* sym_tab)
        : expr_(expr), sym_tab_(sym_tab) {}

    ~AstExprEvaluator() = default;

    std::vector<geax::frontend::Expr*> agg_exprs_;
    std::vector<std::shared_ptr<AggCtx>> agg_ctxs_;
    size_t agg_pos_;

    enum class VisitMode {
        EVALUATE,
        AGGREGATE,
    } visit_mode_;

    Entry Evaluate(RTContext* ctx, const Record* record) {
        ctx_ = ctx;
        record_ = record;
        agg_pos_ = 0;
        visit_mode_ = VisitMode::EVALUATE;
        return std::any_cast<Entry>(expr_->accept(*this));
    }

    void Aggregate(RTContext* ctx, const Record* record) {
        ctx_ = ctx;
        record_ = record;
        visit_mode_ = VisitMode::AGGREGATE;
        if (agg_exprs_.empty()) {
            agg_exprs_ = AstAggExprDetector::GetAggExprs(expr_);
        }
        for (size_t i = 0; i < agg_exprs_.size(); i++) {
            agg_pos_ = i;
            agg_exprs_[i]->accept(*this);
        }
    }

    void Reduce() {
        for (auto agg_ctx : agg_ctxs_) {
            agg_ctx->ReduceNext();
        }
    }

 private:
    std::any visit(geax::frontend::Expr* node) {
        return node->accept(*this);
    }

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
    std::any visit(geax::frontend::GraphPattern* node) override;
    std::any visit(geax::frontend::PathPattern* node) override;
    std::any visit(geax::frontend::PathChain* node) override;
    std::any visit(geax::frontend::Node* node) override;
    std::any visit(geax::frontend::Edge* node) override;
    std::any visit(geax::frontend::ElementFiller* node) override;
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
    std::any visit(geax::frontend::SetAllProperties* node) override;
    std::any visit(geax::frontend::SetLabel* node) override;
    std::any visit(geax::frontend::IsNull* node) override;
    std::any visit(geax::frontend::IsDirected* node) override;
    std::any visit(geax::frontend::IsNormalized* node) override;
    std::any visit(geax::frontend::IsSourceOf* node) override;
    std::any visit(geax::frontend::IsDestinationOf* node) override;
    std::any visit(geax::frontend::IsLabeled* node) override;
    std::any visit(geax::frontend::Same* node) override;
    std::any visit(geax::frontend::AllDifferent* node) override;
    std::any visit(geax::frontend::Exists* node) override;
    std::any visit(geax::frontend::ProcedureBody* node) override;
    std::any visit(geax::frontend::SchemaFromPath* node) override;
    std::any visit(geax::frontend::BindingValue* node) override;
    std::any visit(geax::frontend::BindingGraph* node) override;
    std::any visit(geax::frontend::BindingTable* node) override;
    std::any visit(geax::frontend::StatementWithYield* node) override;
    std::any visit(geax::frontend::QueryStatement* node) override;
    std::any visit(geax::frontend::JoinQueryExpression* node) override;
    std::any visit(geax::frontend::JoinRightPart* node) override;
    std::any visit(geax::frontend::CompositeQueryStatement* node) override;
    std::any visit(geax::frontend::AmbientLinearQueryStatement* node) override;
    std::any visit(geax::frontend::SelectStatement* node) override;
    std::any visit(geax::frontend::FocusedQueryStatement* node) override;
    std::any visit(geax::frontend::FocusedResultStatement* node) override;
    std::any visit(geax::frontend::MatchStatement* node) override;
    std::any visit(geax::frontend::FilterStatement* node) override;
    std::any visit(geax::frontend::PrimitiveResultStatement* node) override;
    std::any visit(geax::frontend::CatalogModifyStatement* node) override;
    std::any visit(geax::frontend::LinearDataModifyingStatement* node) override;
    std::any visit(geax::frontend::InsertStatement* node) override;
    std::any visit(geax::frontend::ReplaceStatement* node) override;
    std::any visit(geax::frontend::SetStatement* node) override;
    std::any visit(geax::frontend::DeleteStatement* node) override;
    std::any visit(geax::frontend::RemoveStatement* node) override;
    std::any visit(geax::frontend::OtherWise* node) override;
    std::any visit(geax::frontend::Union* node) override;
    std::any visit(geax::frontend::Except* node) override;
    std::any visit(geax::frontend::Intersect* node) override;

    std::any visit(geax::frontend::UpdateProperties* node) override;
    std::any visit(geax::frontend::SetSingleProperty* node) override;
    std::any visit(geax::frontend::Param* node) override;
    std::any visit(geax::frontend::ShowProcessListStatement* node) override;
    std::any visit(geax::frontend::KillStatement* node) override;
    std::any visit(geax::frontend::ManagerStatement* node) override;
    std::any visit(geax::frontend::DummyNode* node) override;
    std::any reportError() override;

 private:
    std::string error_msg_;
    geax::frontend::Expr* expr_;
    RTContext* ctx_;
    const SymbolTable* sym_tab_;
    const Record* record_;
    std::shared_ptr<AggCtx> agg_func_;
};

}  // namespace cypher
