/**
 * Copyright 2022 AntGroup CO., Ltd.
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

//
// Created by wt on 6/29/18.
//
#pragma once

#include <memory>
#include "cypher/arithmetic/arithmetic_expression.h"
#include "cypher/execution_plan/ops/op.h"
#include "cypher/utils/geax_util.h"

namespace lgraph {

enum CompareOp { LBR_EQ = 0, LBR_NEQ = 1, LBR_LT = 2, LBR_LE = 3, LBR_GT = 4, LBR_GE = 5 };

enum LogicalOp { LBR_EMPTY = 0, LBR_AND = 1, LBR_OR = 2, LBR_NOT = 3, LBR_XOR = 4 };

class Filter {
 public:
    enum Type {
        EMPTY,
        UNARY,
        BINARY,
        RANGE_FILTER,
        TEST_NULL_FILTER,
        TEST_IN_FILTER,
        TEST_EXISTS_FILTER,
        LABEL_FILTER,
        STRING_FILTER,
        // todo (kehuang): AstExpr is currently temporarily used as a type of Filter, and will be
        // replaced with AstExpr in the future.
        GEAX_EXPR_FILTER,
    };

    static inline std::string ToString(const Type &type) {
        switch (type) {
        case Type::EMPTY:
            return "EMPTY";
        case Type::UNARY:
            return "UNARY";
        case Type::BINARY:
            return "BINARY";
        case Type::RANGE_FILTER:
            return "RANGE_FILTER";
        case Type::TEST_NULL_FILTER:
            return "TEST_NULL_FILTER";
        case Type::TEST_IN_FILTER:
            return "TEST_IN_FILTER";
        case Type::TEST_EXISTS_FILTER:
            return "TEST_EXISTS_FILTER";
        case Type::LABEL_FILTER:
            return "LABEL_FILTER";
        case Type::STRING_FILTER:
            return "STRING_FILTER";
        case Type::GEAX_EXPR_FILTER:
            return "GEAX_EXPR_FILTER";
        default:
            THROW_CODE(CypherException, "unknown RecordEntryType");
        }
    }

    // disable copy constructor & assignment
    Filter(const Filter &rhs) = delete;

    Filter &operator=(const Filter &rhs) = delete;

    Filter &operator=(Filter &&rhs) = delete;

    Filter() : _type(EMPTY), _logical_op(lgraph::LogicalOp::LBR_EMPTY) {}

    Filter(lgraph::LogicalOp logical_op, const std::shared_ptr<Filter> &unary)
        : _type(UNARY), _logical_op(logical_op), _left(unary) {}

    Filter(lgraph::LogicalOp logical_op, const std::shared_ptr<Filter> &left,
           const std::shared_ptr<Filter> &right)
        : _type(BINARY), _logical_op(logical_op), _left(left), _right(right) {}

    Filter(Filter &&rhs) noexcept
        : _type(rhs._type),
          _logical_op(rhs._logical_op),
          _left(std::move(rhs._left)),
          _right(std::move(rhs._right)) {
        rhs._type = EMPTY;
    }

    virtual ~Filter() = default;

    virtual std::shared_ptr<Filter> Clone() const {
        auto clone = std::make_shared<Filter>();
        clone->_type = _type;
        clone->_logical_op = _logical_op;
        clone->_left = _left == nullptr ? nullptr : _left->Clone();
        clone->_right = _right == nullptr ? nullptr : _right->Clone();
        return clone;
    }

    const std::shared_ptr<Filter> &Left() const { return _left; }
    std::shared_ptr<Filter> &Left() { return _left; }

    const std::shared_ptr<Filter> &Right() const { return _right; }
    std::shared_ptr<Filter> &Right() { return _right; }

    Type Type() const { return _type; }

    lgraph::LogicalOp LogicalOp() const { return _logical_op; }

    virtual std::set<std::string> Alias() const {
        std::set<std::string> ret;
        switch (_type) {
        case EMPTY:
            return ret;
        case UNARY:
            return _left->Alias();
        case BINARY:
            {
                ret = _left->Alias();
                auto ret_r = _right->Alias();
                ret.insert(ret_r.begin(), ret_r.end());
                return ret;
            }
        default:
            return ret;
        }
    }

    virtual std::set<std::pair<std::string, std::string>> VisitedFields() const {
        THROW_CODE(CypherException,
            "Filter Type:{} VisitedFields not implemented yet", (int)Type());
    }

    /* For filter tree: If there are sub-filter(s) completely contained in ALIASES.
     * For leaf filter: If the filter is completely contained in ALIASES.
     * */
    virtual bool ContainAlias(const std::vector<std::string> &aliases) const {
        switch (_logical_op) {
        case lgraph::LBR_EMPTY:
        case lgraph::LBR_NOT:
            return _left && _left->ContainAlias(aliases);
        case lgraph::LBR_AND:
        case lgraph::LBR_OR:
        case lgraph::LBR_XOR:
            return _left && _right &&
                   (_left->ContainAlias(aliases) || _right->ContainAlias(aliases));
        default:
            return false;
        }
    }

    virtual void RealignAliasId(const cypher::SymbolTable &sym_tab) {
        if (_left) _left->RealignAliasId(sym_tab);
        if (_right) _right->RealignAliasId(sym_tab);
    }

    bool Empty() const { return _type == EMPTY; }

    bool IsLeaf() const { return _type != EMPTY && _type != UNARY && _type != BINARY; }

    // Test whether only contains logical AND for all binary logical op.
    bool BinaryOnlyContainsAND() const {
        if (_type == BINARY && LogicalOp() != lgraph::LogicalOp::LBR_AND) {
            return false;
        }
        return (_left == nullptr || _left->BinaryOnlyContainsAND()) &&
               (_right == nullptr || _right->BinaryOnlyContainsAND());
    }

    /* Remove sub-filter nodes that are completely contained by the ALIAS.
     * e.g.
     * input filter: {a.uid > d.uid}&&{b.uid < c.uid}
     * alias: {a, b, c}
     * output filter: {a.uid > d.uid}
     * */
    static void RemoveAlias(std::shared_ptr<Filter> &f, const std::vector<std::string> &alias) {
        RemoveFilterWhen(f, [&alias](const auto &b, const auto &e) {
            std::vector<std::string> vs;
            std::set_intersection(b, e, alias.cbegin(), alias.cend(), std::back_inserter(vs));
            return vs.size() == (size_t)std::distance(b, e);
        });
    }

    /// Remove sub-filter nodes that doesn't meet condition `apply`
    template <typename F>
    static void RemoveFilterWhen(std::shared_ptr<Filter> &f, F &&apply) {
        if (f->_type == EMPTY) return;
        if (f->IsLeaf()) {
            auto fa = f->Alias();
            if (apply(fa.cbegin(), fa.cend())) f = nullptr;
            return;
        }
        if (f->_type == UNARY) {
            RemoveFilterWhen(f->_left, apply);
            /* squash */
            if (f->_left == nullptr) f = nullptr;
            return;
        }
        if (f->_type == BINARY) {
            // if (f->LogicalOp() != lgraph::LogicalOp::LBR_AND)
            //     CYPHER_TODO();  // only handle AND-tree
            RemoveFilterWhen(f->_left, apply);
            RemoveFilterWhen(f->_right, apply);
            /* squash */
            if (!f->_left && !f->_right) {
                f = nullptr;
            } else if (f->_left && !f->_right) {
                f = f->_left;
            } else if (!f->_left && f->_right) {
                f = f->_right;
            }
            return;
        }
    }

    virtual bool DoFilter(cypher::RTContext *ctx, const cypher::Record &record) {
        switch (_logical_op) {
        case lgraph::LBR_EMPTY:
            return _left && _left->DoFilter(ctx, record);
        case lgraph::LBR_NOT:
            return _left && !_left->DoFilter(ctx, record);
        case lgraph::LBR_AND:
            return _left && _right &&
                   (_left->DoFilter(ctx, record) && _right->DoFilter(ctx, record));
        case lgraph::LBR_OR:
            return _left && _right &&
                   (_left->DoFilter(ctx, record) || _right->DoFilter(ctx, record));
        case lgraph::LBR_XOR:
            return _left && _right &&
                   (_left->DoFilter(ctx, record) != _right->DoFilter(ctx, record));
        default:
            return false;
        }
    }

    virtual const std::string ToString() const {
        static const std::string null_str = "null";
        std::string left_str, right_str;
        switch (_logical_op) {
        case lgraph::LBR_EMPTY:
            return _left == nullptr ? null_str : _left->ToString();
        case lgraph::LBR_NOT:
            left_str = _left == nullptr ? null_str : _left->ToString();
            return "!" + left_str;
        case lgraph::LBR_AND:
            left_str = _left == nullptr ? null_str : _left->ToString();
            right_str = _right == nullptr ? null_str : _right->ToString();
            return ("(" + left_str + "&&" + right_str + ")");
        case lgraph::LBR_OR:
            left_str = _left == nullptr ? null_str : _left->ToString();
            right_str = _right == nullptr ? null_str : _right->ToString();
            return ("(" + left_str + "||" + right_str + ")");
        case lgraph::LBR_XOR:
            left_str = _left == nullptr ? null_str : _left->ToString();
            right_str = _right == nullptr ? null_str : _right->ToString();
            return ("(" + left_str + "^" + right_str + ")");
        default:
            return "";
        }
    }

 protected:
    enum Type _type = EMPTY;

 private:
    lgraph::LogicalOp _logical_op;
    std::shared_ptr<Filter> _left = nullptr;  // also used for unary operation
    std::shared_ptr<Filter> _right = nullptr;
    const cypher::SymbolTable *sym_tab_ = nullptr;
};

class GeaxExprFilter : public Filter {
 private:
    cypher::ArithExprNode arith_expr_;

 public:
    GeaxExprFilter(geax::frontend::Expr *expr, const cypher::SymbolTable &sym_tab) {
        arith_expr_.SetAstExp(expr, sym_tab);
        _type = GEAX_EXPR_FILTER;
    }

    cypher::ArithExprNode &GetArithExpr() { return arith_expr_; }

    std::shared_ptr<Filter> Clone() const override { NOT_SUPPORT_AND_THROW(); }

    std::set<std::string> Alias() const override { NOT_SUPPORT_AND_THROW(); }

    std::set<std::pair<std::string, std::string>> VisitedFields() const override {
        NOT_SUPPORT_AND_THROW();
    }

    bool ContainAlias(const std::vector<std::string> &alias) const override {
        NOT_SUPPORT_AND_THROW();
    }

    void RealignAliasId(const cypher::SymbolTable &sym_tab) override { NOT_SUPPORT_AND_THROW(); }

    bool DoFilter(cypher::RTContext *ctx, const cypher::Record &record) override {
        auto res = arith_expr_.Evaluate(ctx, record);
        if (res.IsBool()) {
            return res.constant.AsBool();
        }
        NOT_SUPPORT_AND_THROW();
    }

    const std::string ToString() const override { return arith_expr_.ToString(); }
};

}  // namespace lgraph
