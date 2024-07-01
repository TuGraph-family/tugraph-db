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

#ifndef GEAXFRONTEND_AST_UTILS_ASTUTIL_H_
#define GEAXFRONTEND_AST_UTILS_ASTUTIL_H_

#include <any>
#include <string>
#include <utility>

#include "geax-front-end/GEAXErrorCode.h"
#include "geax-front-end/ast/Ast.h"

#ifndef BINARY_EXPR_TOSTRING
#define BINARY_EXPR_TOSTRING(op)         \
    str_ += "(";                         \
    node->left()->accept(*this);         \
    str_ += " " + std::string(op) + " "; \
    node->right()->accept(*this);        \
    str_ += ")";                         \
    return GEAXErrorCode::GEAX_SUCCEED;
#endif

#ifndef UNARY_EXPR_TOSTRING
#define UNARY_EXPR_TOSTRING(op)          \
    str_ += "(" + std::string(op) + " "; \
    node->expr()->accept(*this);         \
    str_ += ")";                         \
    return GEAXErrorCode::GEAX_SUCCEED;
#endif

namespace geax::frontend {

class AstExprToString : public AstExprNodeVisitorImpl {
public:
    std::string dump(geax::frontend::Expr* node) {
        if (node) {
            node->accept(*this);
            return str_;
        } else {
            return "";
        }
    }

private:
    std::any visit(geax::frontend::GetField* node) override {
        node->expr()->accept(*this);
        str_ += "." + node->fieldName();
        return GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::TupleGet* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    std::any visit(geax::frontend::Not* node) override { UNARY_EXPR_TOSTRING("!"); }
    std::any visit(geax::frontend::Neg* node) override { UNARY_EXPR_TOSTRING("-"); }
    std::any visit(geax::frontend::Tilde* node) override {
        UNARY_EXPR_TOSTRING("~");
    }
    std::any visit(geax::frontend::VSome* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    std::any visit(geax::frontend::BEqual* node) override { BINARY_EXPR_TOSTRING("="); }
    std::any visit(geax::frontend::BNotEqual* node) override { BINARY_EXPR_TOSTRING("!="); }
    std::any visit(geax::frontend::BGreaterThan* node) override { BINARY_EXPR_TOSTRING(">"); }
    std::any visit(geax::frontend::BNotSmallerThan* node) override { BINARY_EXPR_TOSTRING(">="); }
    std::any visit(geax::frontend::BSmallerThan* node) override { BINARY_EXPR_TOSTRING("<"); }
    std::any visit(geax::frontend::BNotGreaterThan* node) override { BINARY_EXPR_TOSTRING("<="); }
    std::any visit(geax::frontend::BSafeEqual* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }
    std::any visit(geax::frontend::BAdd* node) override { BINARY_EXPR_TOSTRING("+"); }
    std::any visit(geax::frontend::BSub* node) override { BINARY_EXPR_TOSTRING("-"); }
    std::any visit(geax::frontend::BDiv* node) override { BINARY_EXPR_TOSTRING("/"); }
    std::any visit(geax::frontend::BMul* node) override { BINARY_EXPR_TOSTRING("*"); }
    std::any visit(geax::frontend::BMod* node) override { BINARY_EXPR_TOSTRING("%"); }
    std::any visit(geax::frontend::BSquare* node) override { BINARY_EXPR_TOSTRING("^"); }
    std::any visit(geax::frontend::BAnd* node) override { BINARY_EXPR_TOSTRING("and"); }
    std::any visit(geax::frontend::BOr* node) override { BINARY_EXPR_TOSTRING("or"); }
    std::any visit(geax::frontend::BXor* node) override { BINARY_EXPR_TOSTRING("xor"); }
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
        str_ += node->name() + "(";
        for (auto& expr : node->args()) {
            expr->accept(*this);
            str_ += ",";
        }
        if (!node->args().empty()) {
            str_.resize(str_.size() - 1);
        }
        str_ += ")";
        return GEAXErrorCode::GEAX_SUCCEED;
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
        str_ += std::to_string(node->val());
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(geax::frontend::VInt* node) override {
        str_ += std::to_string(node->val());
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(geax::frontend::VDouble* node) override {
        str_ += std::to_string(node->val());
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(geax::frontend::VString* node) override {
        str_ += "\"" + node->val() + "\"";
        return GEAXErrorCode::GEAX_SUCCEED;
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
        str_ += node->name();
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(geax::frontend::Param* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
    }

    std::any reportError() override { return error_msg_; }

private:
    std::string error_msg_;
    std::string str_;
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

class AstLabelTreeToString : public AstLabelTreeNodeVisitorImpl {
public:
    std::string dump(geax::frontend::LabelTree* node) {
        if (node) {
            node->accept(*this);
            return str_;
        } else {
            return "";
        }
    }

private:
    std::any visit(geax::frontend::SingleLabel* node) override {
        str_ += node->label();
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(geax::frontend::LabelOr* node) override { BINARY_EXPR_TOSTRING("or"); }
    std::any visit(geax::frontend::LabelAnd* node) override { BINARY_EXPR_TOSTRING("and"); }
    std::any visit(geax::frontend::LabelNot* node) override { UNARY_EXPR_TOSTRING("not"); }
    std::any reportError() override { return error_msg_; }

private:
    std::string error_msg_;
    std::string str_;
};

[[maybe_unused]] inline static std::string ToString(geax::frontend::LabelTree* node) {
    return AstLabelTreeToString().dump(node);
}

}  // namespace geax::frontend

#endif  // GEAXFRONTEND_AST_UTILS_ASTUTIL_H_
