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

#include <any>
#include "cypher/arithmetic/arithmetic_expression.h"
#include "cypher/arithmetic/ast_agg_expr_detector.h"

namespace cypher {

std::vector<geax::frontend::Expr*>& AstAggExprDetector::AggExprs() { return agg_exprs_; }

bool AstAggExprDetector::Validate() {
    if (std::any_cast<geax::frontend::GEAXErrorCode>(expr_->accept(*this)) !=
        geax::frontend::GEAXErrorCode::GEAX_SUCCEED) {
        return false;
    }
    if (agg_exprs_.size() > 0 && outside_var_) {
        error_msg_ =
            "Variables cannot be used outside of the Agg function, for example: sum(n) + m";
        return false;
    }
    return true;
}

bool AstAggExprDetector::HasValidAggFunc() {
        return agg_exprs_.size() > 0;
    }

std::vector<geax::frontend::Expr*> AstAggExprDetector::GetAggExprs(geax::frontend::Expr* expr) {
    AstAggExprDetector detector(expr);
    expr->accept(detector);
    return detector.AggExprs();
}

std::any AstAggExprDetector::visit(geax::frontend::AggFunc* node) {
    if (in_agg_func_ > 0) {
        nested_agg_func_ = true;
        error_msg_ = "Agg function cannot be nested";
        return geax::frontend::GEAXErrorCode::GEAX_ERROR;
    }
    in_agg_func_ += 1;
    auto agg_funcs = ArithOpNode::RegisterAggFuncs();
    std::string func_name = ToString(node->funcName());
    std::transform(func_name.begin(), func_name.end(), func_name.begin(), ::tolower);
    if (agg_funcs.find(func_name) != agg_funcs.end()) {
        agg_exprs_.emplace_back(node);
    }
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
    in_agg_func_ -= 1;
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any AstAggExprDetector::visit(geax::frontend::BAggFunc* node) {
    if (in_agg_func_ > 0) {
        nested_agg_func_ = true;
        error_msg_ = "Agg function cannot be nested";
        return geax::frontend::GEAXErrorCode::GEAX_ERROR;
    }
    in_agg_func_ += 1;
    auto agg_funcs = ArithOpNode::RegisterAggFuncs();
    std::string func_name = ToString(node->funcName());
    std::transform(func_name.begin(), func_name.end(), func_name.begin(), ::tolower);
    if (agg_funcs.find(func_name) != agg_funcs.end()) {
        agg_exprs_.emplace_back(node);
    }
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(std::get<1>(node->lExpr()));
    ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->rExpr());
    in_agg_func_ -= 1;
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

std::any AstAggExprDetector::visit(geax::frontend::GetField* node) {
    if (in_agg_func_ == 0) {
        outside_var_ = true;
    }
    return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
}

}  // namespace cypher
