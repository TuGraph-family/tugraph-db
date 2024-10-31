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

#include <algorithm>
#include <unordered_map>
#include "cypher/cypher_types.h"
#include "core/data_type.h"

#include "cypher/cypher_exception.h"
#include "cypher/experimental/data_type/field_data.h"
#include "cypher/experimental/data_type/record.h"
#include "cypher/utils/geax_util.h"
#include "cypher/experimental/expressions/cexpr.h"

namespace cypher {
namespace compilation {
CFieldData CFieldData::operator+(const CFieldData &other) const {
    if (is_null() || other.is_null()) return CFieldData();
    CFieldData ret;
    if (type == CFieldData::ARRAY || other.type == CFieldData::ARRAY) {
        CYPHER_TODO();
    } else if (is_string() || other.is_string()) {
        CYPHER_TODO();
    } else if ((is_integer() || is_real()) && (other.is_integer() || other.is_real())) {
        if (is_integer() && other.is_integer()) {
            ret.scalar = CScalarData(scalar.Int64() + other.scalar.Int64());
        } else {
            dyn_var<double> x_n = is_integer() ? (dyn_var<double>)scalar.integer()
                : scalar.real();
            dyn_var<double> y_n = is_integer()? (dyn_var<double>)other.scalar.integer()
                : other.scalar.real();
            ret.scalar = CScalarData(x_n + y_n);
        }
    }
    return ret;
}

CFieldData CFieldData::operator-(const CFieldData &other) const {
    if (is_null() || other.is_null()) return CFieldData();
    CFieldData ret;
    if (type == CFieldData::ARRAY || other.type == CFieldData::ARRAY) {
        CYPHER_TODO();
    } else if (is_string() || other.is_string()) {
        CYPHER_TODO();
    } else if ((is_integer() || is_real()) && (other.is_integer() || other.is_real())) {
        if (is_integer() && other.is_integer()) {
            ret.scalar = CScalarData(scalar.Int64() - other.scalar.Int64());
        } else {
            dyn_var<double> x_n = is_integer() ? (dyn_var<double>)scalar.integer()
                : scalar.real();
            dyn_var<double> y_n = is_integer()? (dyn_var<double>)other.scalar.integer()
                : other.scalar.real();
            ret.scalar = CScalarData(x_n - y_n);
        }
    }
    return ret;
}

static CFieldData add(const CFieldData& x, const CFieldData& y) {
    return x + y;
}

static CFieldData sub(const CFieldData& x, const CFieldData& y) {
    return x - y;
}

static CFieldData div(const CFieldData& x,  const CFieldData y) {
    if (x.is_null() || y.is_null()) return CFieldData();
    if (!(x.is_integer() || x.is_real()) || !(y.is_integer() || y.is_real()))
        throw lgraph::CypherException("Type mismatch: expect Integer or Float in div expr");
    CFieldData ret;
    if (x.is_integer() && y.is_integer()) {
        dyn_var<int64_t> x_n = x.scalar.integer();
        dyn_var<int64_t> y_n = y.scalar.integer();
        if (y_n == 0) throw lgraph::CypherException("divide by zero");
        ret.scalar = CScalarData(x_n / y_n);
    } else {
        dyn_var<double> x_n = x.is_integer() ? (dyn_var<double>) x.scalar.integer()
            : x.scalar.real();
        dyn_var<double> y_n = y.is_integer()? (dyn_var<double>) y.scalar.integer()
            : y.scalar.real();
        if (y_n == 0) CYPHER_TODO();
        ret.scalar = CScalarData(x_n - y_n);
    }
    return ret;
}

#ifndef DO_BINARY_EXPR
#define DO_BINARY_EXPR(func)                                       \
    auto lef = std::any_cast<CEntry>(node->left()->accept(*this));  \
    auto rig = std::any_cast<CEntry>(node->right()->accept(*this)); \
    if (lef.type_ != CEntry::RecordEntryType::CONSTANT ||            \
        rig.type_ != CEntry::RecordEntryType::CONSTANT) {            \
        NOT_SUPPORT_AND_THROW();                                   \
    }                                                              \
    return CEntry(func(lef.constant_, rig.constant_));
#endif

std::any ExprEvaluator::visit(geax::frontend::BAdd* node) { DO_BINARY_EXPR(add); }

std::any ExprEvaluator::visit(geax::frontend::Ref* node) {
    auto it = sym_tab_->symbols.find(node->name());
    if (it == sym_tab_->symbols.end()) NOT_SUPPORT_AND_THROW();
    switch (it->second.type) {
    case SymbolNode::NODE:
    case SymbolNode::RELATIONSHIP:
    case SymbolNode::CONSTANT:
    case SymbolNode::PARAMETER:
        return record_->values[it->second.id];
    case SymbolNode::NAMED_PATH:
        {
            // auto it = sym_tab_->anot_collection.path_elements.find(node->name());
            // if (it == sym_tab_->anot_collection.path_elements.end())
            //     throw lgraph::CypherException("path_elements error: " + node->name())
            // const std::vector<std::shared_ptr<geax::frontend::Ref>>& elements = it->second;
            // std::vector<CExprNode> params;
            // for (auto ref: elements) {
            //     params.emplace_back(ref.get(), *sym_tab_);
            // }
            CYPHER_TODO();
        }
    }
    return std::any();
}

std::any ExprEvaluator::visit(geax::frontend::GetField* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::TupleGet* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::Not* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::Neg* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::Tilde* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::VSome* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BEqual* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BNotEqual* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BGreaterThan* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BNotSmallerThan* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BSmallerThan* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BNotGreaterThan* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BSafeEqual* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BSub* node) { DO_BINARY_EXPR(sub); }
std::any ExprEvaluator::visit(geax::frontend::BDiv* node) { DO_BINARY_EXPR(div); }
std::any ExprEvaluator::visit(geax::frontend::BMul* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BMod* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BSquare* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BAnd* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BOr* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BXor* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BBitAnd* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BBitOr* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BBitXor* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BBitLeftShift* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BBitRightShift* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BConcat* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BIndex* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BLike* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BIn* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::If* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::Function* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::Case* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::Cast* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::MatchCase* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::AggFunc* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::BAggFunc* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::MultiCount* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::Windowing* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::MkList* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::MkMap* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::MkRecord* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::MkSet* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::MkTuple* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::VBool* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::VInt* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::VDouble* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::VString* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::VDate* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::VDatetime* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::VDuration* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::VTime* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::VNull* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::VNone* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::Param* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::SingleLabel* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::LabelOr* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::IsLabeled* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::IsNull* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::ListComprehension* node) { CYPHER_TODO(); }
std::any ExprEvaluator::visit(geax::frontend::Exists* node) { CYPHER_TODO(); }
std::any ExprEvaluator::reportError() { CYPHER_TODO(); }
}  // namespace compilation
}  // namespace cypher
