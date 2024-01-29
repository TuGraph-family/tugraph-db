/**
 * Copyright 2024 AntGroup CO., Ltd.
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
#define BINARY_EXPR_TOSTRING(op)                                         \
    auto lef = std::any_cast<std::string>(node->left()->accept(*this));  \
    auto rig = std::any_cast<std::string>(node->right()->accept(*this)); \
    return "(" + lef + op + rig + ")";
#endif

#ifndef UNARY_EXPR_TOSTRING
#define UNARY_EXPR_TOSTRING(op)                                          \
    auto expr = std::any_cast<std::string>(node->expr()->accept(*this)); \
    return "(" + std::string(op) + expr + ")";
#endif

namespace cypher {

class AstExprToString : public geax::frontend::AstExprNodeVisitorImpl {
 public:
    std::string dump(geax::frontend::Expr* node) {
        return std::any_cast<std::string>(node->accept(*this));
    }

 private:
    std::any visit(geax::frontend::GetField* node) override {
        return std::any_cast<std::string>(node->expr()->accept(*this)) + "." + node->fieldName();
    }

    std::any visit(geax::frontend::TupleGet* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::Not* node) override { UNARY_EXPR_TOSTRING("!"); }
    std::any visit(geax::frontend::Neg* node) override { UNARY_EXPR_TOSTRING("-"); }
    std::any visit(geax::frontend::Tilde* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::VSome* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BEqual* node) override { BINARY_EXPR_TOSTRING("="); }
    std::any visit(geax::frontend::BNotEqual* node) override { BINARY_EXPR_TOSTRING("!="); }
    std::any visit(geax::frontend::BGreaterThan* node) override { BINARY_EXPR_TOSTRING(">"); }
    std::any visit(geax::frontend::BNotSmallerThan* node) override { BINARY_EXPR_TOSTRING(">="); }
    std::any visit(geax::frontend::BSmallerThan* node) override { BINARY_EXPR_TOSTRING("<"); }
    std::any visit(geax::frontend::BNotGreaterThan* node) override { BINARY_EXPR_TOSTRING("<="); }
    std::any visit(geax::frontend::BSafeEqual* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BAdd* node) override { BINARY_EXPR_TOSTRING("+"); }
    std::any visit(geax::frontend::BSub* node) override { BINARY_EXPR_TOSTRING("-"); }
    std::any visit(geax::frontend::BDiv* node) override { BINARY_EXPR_TOSTRING("/"); }
    std::any visit(geax::frontend::BMul* node) override { BINARY_EXPR_TOSTRING("*"); }
    std::any visit(geax::frontend::BMod* node) override { BINARY_EXPR_TOSTRING("%"); }
    std::any visit(geax::frontend::BAnd* node) override { BINARY_EXPR_TOSTRING(" and "); }
    std::any visit(geax::frontend::BOr* node) override { BINARY_EXPR_TOSTRING(" or "); }
    std::any visit(geax::frontend::BXor* node) override { BINARY_EXPR_TOSTRING(" xor "); }
    std::any visit(geax::frontend::BBitAnd* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BBitOr* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BBitXor* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BBitLeftShift* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BBitRightShift* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BConcat* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BIndex* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BLike* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BIn* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::If* node) override { NOT_SUPPORT_AND_THROW(); }

    std::any visit(geax::frontend::Function* node) override {
        std::string str = node->name() + "(";
        for (auto& expr : node->args()) {
            str += std::any_cast<std::string>(expr->accept(*this));
            str += ",";
        }
        if (fma_common::EndsWith(str, ",")) {
            str.resize(str.size() - 1);
        }
        str += ")";
        return str;
    }

    std::any visit(geax::frontend::Case* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::Cast* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::MatchCase* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::AggFunc* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BAggFunc* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::MultiCount* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::Windowing* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::MkList* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::MkMap* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::MkRecord* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::MkSet* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::MkTuple* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::VBool* node) override { return std::to_string(node->val()); }
    std::any visit(geax::frontend::VInt* node) override { return std::to_string(node->val()); }
    std::any visit(geax::frontend::VDouble* node) override { return std::to_string(node->val()); }
    std::any visit(geax::frontend::VString* node) override { return "\"" + node->val() + "\""; }
    std::any visit(geax::frontend::VDate* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::VDatetime* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::VDuration* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::VTime* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::VNull* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::VNone* node) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::Ref* node) override { return node->name(); }
    std::any visit(geax::frontend::Param* node) override { NOT_SUPPORT_AND_THROW(); }
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
[[maybe_unused]] inline static std::string ToString(geax::frontend::Expr* node) {
    return AstExprToString().dump(node);
}

}  // namespace cypher
