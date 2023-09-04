/**
 * Copyright 2022 AntGroup CO., Ltd.
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
#include <string>
#include <utility>

#include "execution_plan/visitor/visitor.h"
#include "cypher/resultset/record.h"
#include "cypher/utils/geax_util.h"
#include "parser/symbol_table.h"
#include "cypher/cypher_types.h"
#include "core/data_type.h"
#include "geax-front-end/ast/Ast.h"


#ifndef BINARY_EXPR_TOSTRING
#define BINARY_EXPR_TOSTRING(op)                                              \
    auto lef = std::any_cast<std::string>(node->left()->accept(*this));       \
    auto rig = std::any_cast<std::string>(node->right()->accept(*this));      \
    return "(" + lef + op + rig + ")";
#endif

#ifndef UNARY_EXPR_TOSTRING
#define UNARY_EXPR_TOSTRING(op)                                               \
    auto expr = std::any_cast<std::string>(node->expr()->accept(*this));      \
    return "(" + std::string(op) + expr + ")";
#endif

namespace cypher {

class AstExprToString : public geax::frontend::AstNodeVisitor {
 public:
    std::string dump(geax::frontend::Expr* node) {
        return std::any_cast<std::string>(this->visit(node));
    }

 private:
    std::any visit(geax::frontend::Expr* node) {
        return node->accept(*this);
    }

    std::any visit(geax::frontend::GetField* node) override {
        return std::any_cast<std::string>(node->expr()->accept(*this)) + "." + node->fieldName();
    }

    std::any visit(geax::frontend::TupleGet* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::Not* node) override {
        UNARY_EXPR_TOSTRING("!");
    }
    std::any visit(geax::frontend::Neg* node) override {
        UNARY_EXPR_TOSTRING("-");
    }
    std::any visit(geax::frontend::Tilde* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::VSome* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::BEqual* node) override {
        BINARY_EXPR_TOSTRING("=");
    }
    std::any visit(geax::frontend::BNotEqual* node) override {
        BINARY_EXPR_TOSTRING("!=");
    }
    std::any visit(geax::frontend::BGreaterThan* node) override {
        BINARY_EXPR_TOSTRING(">");
    }
    std::any visit(geax::frontend::BNotSmallerThan* node) override {
        BINARY_EXPR_TOSTRING(">=");
    }
    std::any visit(geax::frontend::BSmallerThan* node) override {
        BINARY_EXPR_TOSTRING("<");
    }
    std::any visit(geax::frontend::BNotGreaterThan* node) override {
        BINARY_EXPR_TOSTRING("<=");
    }
    std::any visit(geax::frontend::BSafeEqual* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::BAdd* node) override {
        BINARY_EXPR_TOSTRING("+");
    }
    std::any visit(geax::frontend::BSub* node) override {
        BINARY_EXPR_TOSTRING("-");
    }
    std::any visit(geax::frontend::BDiv* node) override {
        BINARY_EXPR_TOSTRING("/");
    }
    std::any visit(geax::frontend::BMul* node) override {
        BINARY_EXPR_TOSTRING("*");;
    }
    std::any visit(geax::frontend::BMod* node) override {
        BINARY_EXPR_TOSTRING("%");;
    }
    std::any visit(geax::frontend::BAnd* node) override {
        BINARY_EXPR_TOSTRING(" and ");
    }
    std::any visit(geax::frontend::BOr* node) override {
        BINARY_EXPR_TOSTRING(" or ");
    }
    std::any visit(geax::frontend::BXor* node) override {
        BINARY_EXPR_TOSTRING(" xor ");
    }
    std::any visit(geax::frontend::BBitAnd* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::BBitOr* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::BBitXor* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::BBitLeftShift* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::BBitRightShift* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::BConcat* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::BIndex* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::BLike* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::BIn* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::If* node) override {
        NOT_SUPPORT_AND_THROW();
    }

    std::any visit(geax::frontend::Function* node) override {
        std::string str = node->name() + "(";
        for (auto& expr : node->args()) {
            str += std::any_cast<std::string>(expr->accept(*this));
            str += ",";
        }
        if (fma_common::EndsWith(str, ",")) {
            str.resize(str.size()-1);
        }
        str += ")";
        return str;
    }

    std::any visit(geax::frontend::Case* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::Cast* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::MatchCase* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::AggFunc* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::BAggFunc* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::MultiCount* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::Windowing* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::MkList* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::MkMap* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::MkRecord* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::MkSet* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::MkTuple* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::VBool* node) override {
        return std::to_string(node->val());
    }
    std::any visit(geax::frontend::VInt* node) override {
        return std::to_string(node->val());
    }
    std::any visit(geax::frontend::VDouble* node) override {
        return std::to_string(node->val());
    }
    std::any visit(geax::frontend::VString* node) override {
        return "\"" + node->val() + "\"";
    }
    std::any visit(geax::frontend::VDate* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::VDatetime* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::VDuration* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::VTime* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::VNull* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::VNone* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::Ref* node) override {
        return node->name();
    }
    std::any visit(geax::frontend::GraphPattern* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::PathPattern* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::PathChain* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::Node* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::Edge* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::ElementFiller* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::WhereClause* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::OrderByField* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::PathModePrefix* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::PathSearchPrefix* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::SingleLabel* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::LabelOr* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::LabelAnd* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::LabelNot* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::PropStruct* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::YieldField* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::TableFunctionClause* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::ReadConsistency* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::AllowAnonymousTable* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::OpConcurrent* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::SetAllProperties* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::SetLabel* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::IsNull* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::IsDirected* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::IsNormalized* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::IsSourceOf* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::IsDestinationOf* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::IsLabeled* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::Same* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::AllDifferent* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::Exists* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::ProcedureBody* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::SchemaFromPath* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::BindingValue* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::BindingGraph* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::BindingTable* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::StatementWithYield* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::QueryStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::JoinQueryExpression* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::JoinRightPart* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::CompositeQueryStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::AmbientLinearQueryStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::SelectStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::FocusedQueryStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::FocusedResultStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::MatchStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::FilterStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::PrimitiveResultStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::CatalogModifyStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::LinearDataModifyingStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::InsertStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::ReplaceStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::SetStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::DeleteStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::RemoveStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::OtherWise* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::Union* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::Except* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::Intersect* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::UpdateProperties* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::SetSingleProperty* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::Param* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::ShowProcessListStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::KillStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::ManagerStatement* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::DummyNode* node) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any reportError() override { return error_msg_; }

 private:
    std::string error_msg_;
};

//
// https://stackoverflow.com/questions/59958785/unused-static-inline-functions-generate-warnings-with-clang
//
// gcc would flag an unused static function, but not an inline static one. For clang though, the
// outcome is different.
//
[[maybe_unused]]
inline static std::string ToString(geax::frontend::Expr* node) {
    return AstExprToString().dump(node);
}

}  // namespace cypher
