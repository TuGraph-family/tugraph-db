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

#include "cypher/execution_plan/visitor/visitor.h"
#include "cypher/resultset/record.h"
#include "cypher/utils/geax_util.h"
#include "cypher/parser/symbol_table.h"
#include "cypher/cypher_types.h"
//#include "core/data_type.h"
#include "geax-front-end/ast/Ast.h"
#include <boost/algorithm/string/predicate.hpp>

#ifndef EXP_BINARY_EXPR_TOSTRING
#define EXP_BINARY_EXPR_TOSTRING(op)                                     \
    auto lef = std::any_cast<std::string>(node->left()->accept(*this));  \
    auto rig = std::any_cast<std::string>(node->right()->accept(*this)); \
    return "(" + lef + op + rig + ")";
#endif

#ifndef EXP_UNARY_EXPR_TOSTRING
#define EXP_UNARY_EXPR_TOSTRING(op)                                      \
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
        std::string str;
        str = std::any_cast<std::string>(node->expr()->accept(*this));
        return str + "." + node->fieldName();
    }

    std::any visit(geax::frontend::TupleGet* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::Not* node) override { EXP_UNARY_EXPR_TOSTRING("!"); }
    std::any visit(geax::frontend::Neg* node) override { EXP_UNARY_EXPR_TOSTRING("-"); }
    std::any visit(geax::frontend::Tilde* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::VSome* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BEqual* node) override { EXP_BINARY_EXPR_TOSTRING("="); }
    std::any visit(geax::frontend::BNotEqual* node) override { EXP_BINARY_EXPR_TOSTRING("!="); }
    std::any visit(geax::frontend::BGreaterThan* node) override { EXP_BINARY_EXPR_TOSTRING(">"); }
    std::any visit(geax::frontend::BNotSmallerThan* node) override {
        EXP_BINARY_EXPR_TOSTRING(">=");
    }
    std::any visit(geax::frontend::BSmallerThan* node) override { EXP_BINARY_EXPR_TOSTRING("<"); }
    std::any visit(geax::frontend::BNotGreaterThan* node) override {
        EXP_BINARY_EXPR_TOSTRING("<=");
    }
    std::any visit(geax::frontend::BSafeEqual* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BAdd* node) override { EXP_BINARY_EXPR_TOSTRING("+"); }
    std::any visit(geax::frontend::BSub* node) override { EXP_BINARY_EXPR_TOSTRING("-"); }
    std::any visit(geax::frontend::BDiv* node) override { EXP_BINARY_EXPR_TOSTRING("/"); }
    std::any visit(geax::frontend::BMul* node) override { EXP_BINARY_EXPR_TOSTRING("*"); }
    std::any visit(geax::frontend::BMod* node) override { EXP_BINARY_EXPR_TOSTRING("%"); }
    std::any visit(geax::frontend::BSquare* node) override { EXP_BINARY_EXPR_TOSTRING(" ^ "); }
    std::any visit(geax::frontend::BAnd* node) override { EXP_BINARY_EXPR_TOSTRING(" and "); }
    std::any visit(geax::frontend::BOr* node) override { EXP_BINARY_EXPR_TOSTRING(" or "); }
    std::any visit(geax::frontend::BXor* node) override { EXP_BINARY_EXPR_TOSTRING(" xor "); }
    std::any visit(geax::frontend::IsLabeled* node) override {
        EXP_UNARY_EXPR_TOSTRING(" isLabel ");
    }
    std::any visit(geax::frontend::BBitAnd* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BBitOr* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BBitXor* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BBitLeftShift* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BBitRightShift* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BConcat* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BIndex* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BLike* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BIn* node) override {
        std::string str = std::any_cast<std::string>(node->left()->accept(*this));
        str += " In ";
        str += std::any_cast<std::string>(node->right()->accept(*this));
        return str;
    }
    std::any visit(geax::frontend::If* ) override { NOT_SUPPORT_AND_THROW(); }

    std::any visit(geax::frontend::Function* node) override {
        std::string str = node->name() + "(";
        for (auto& expr : node->args()) {
            str += std::any_cast<std::string>(expr->accept(*this));
            str += ",";
        }
        if (boost::algorithm::ends_with(str, ",")) {
            str.resize(str.size() - 1);
        }
        str += ")";
        return str;
    }

    std::any visit(geax::frontend::Case* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::Cast* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::MatchCase* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::AggFunc* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::BAggFunc* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::MultiCount* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::Windowing* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::MkList* node) override {
        std::string res("{");
        const auto& exprs = node->elems();
        for (size_t idx = 0; idx < exprs.size(); ++idx) {
            std::string temp;
            temp = std::any_cast<std::string>(exprs[idx]->accept(*this));
            res += temp;
            if (idx != exprs.size() - 1) {
                res += ", ";
            } else {
                res += "}";
            }
        }
        return res;
    }
    std::any visit(geax::frontend::ListComprehension* node) override {
        std::string res("{");
        const auto & exprs = std::vector{ node->getVariable(),
                             node->getInExpression(), node->getOpExpression()};
        for (size_t idx = 0; idx < exprs.size(); ++idx) {
            std::string temp;
            temp = std::any_cast<std::string>(exprs[idx]->accept(*this));
            res += temp;
            if (idx != exprs.size() - 1) {
                res += ", ";
            } else {
                res += "}";
            }
        }
        return res;
    }

    std::any visit(geax::frontend::MkMap* ) override {
        NOT_SUPPORT_AND_THROW();
    }
    std::any visit(geax::frontend::MkRecord* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::MkSet* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::MkTuple* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::VBool* node) override { return std::to_string(node->val()); }
    std::any visit(geax::frontend::VInt* node) override { return std::to_string(node->val()); }
    std::any visit(geax::frontend::VDouble* node) override { return std::to_string(node->val()); }
    std::any visit(geax::frontend::VString* node) override { return "\"" + node->val() + "\""; }
    std::any visit(geax::frontend::VDate* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::VDatetime* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::VDuration* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::VTime* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::VNull* ) override { return std::string("__nul__"); }
    std::any visit(geax::frontend::VNone* ) override { NOT_SUPPORT_AND_THROW(); }
    std::any visit(geax::frontend::Ref* node) override { return node->name(); }
    std::any visit(geax::frontend::Param* node) override { return node->name(); }
    std::any visit(geax::frontend::IsNull* node) override {
        std::string str = std::any_cast<std::string>(node->expr()->accept(*this));
        str += "IS NULL";
        return str;
    }
    std::any visit(geax::frontend::Exists*) override {
        std::string str("Exists");
        return str;
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
[[maybe_unused]] inline static std::string ToString(geax::frontend::Expr* node) {
    return AstExprToString().dump(node);
}

}  // namespace cypher
