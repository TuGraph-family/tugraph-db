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
 *
 *  Author:
 *         lili <liangjingru.ljr@antgroup.com>
 */

#ifndef GEAXFRONTEND_AST_EXPR_AGGFUNC_H_
#define GEAXFRONTEND_AST_EXPR_AGGFUNC_H_

#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

enum class GeneralSetFunction {
    kAvg,
    kCount,
    kMax,
    kMin,
    kSum,
    kCollect,
    kStdDevSamp,
    kStdDevPop,
    kGroupConcat,
    kStDev,
    kStDevP,
    kVariance,
    kVarianceP,
    kLastButNotUse
};

inline const char* ToString(GeneralSetFunction type) {
    static const StrArray<enumNum(GeneralSetFunction::kLastButNotUse)> kDict = {
        "avg", "count", "max", "min", "sum", "collect", "stdDevSamp", "stdDevPop", "groupConcat",
        "stDev", "stDevP", "variance", "varianceP"};
    const auto idx = static_cast<size_t>(type);
    return idx < kDict.size() ? kDict[idx] : geax::frontend::kUnknown;
}

inline bool ToEnum(std::string_view sv, GeneralSetFunction& type) {
    static const std::unordered_map<std::string_view, GeneralSetFunction> kTypeMap = {
        {"avg", GeneralSetFunction::kAvg},
        {"count", GeneralSetFunction::kCount},
        {"max", GeneralSetFunction::kMax},
        {"min", GeneralSetFunction::kMin},
        {"sum", GeneralSetFunction::kSum},
        {"collect", GeneralSetFunction::kCollect},
        {"stdDevSamp", GeneralSetFunction::kStdDevSamp},
        {"stdDevPop", GeneralSetFunction::kStdDevPop},
        {"groutConcat", GeneralSetFunction::kGroupConcat},
        {"stDev", GeneralSetFunction::kStDev},
        {"stDevP", GeneralSetFunction::kStDevP},
        {"variance", GeneralSetFunction::kVariance},
        {"varianceP", GeneralSetFunction::kVarianceP}};
    auto it = kTypeMap.find(sv);
    return it != kTypeMap.end() && (type = it->second, true);
}

class AggFunc : public Expr {
public:
    AggFunc() : Expr(AstNodeType::kAggFunc), isDistinct_(false), expr_(nullptr) {}
    ~AggFunc() = default;

    void setFuncName(GeneralSetFunction funcName) { funcName_ = funcName; }
    GeneralSetFunction funcName() const { return funcName_; }

    void setDistinct(bool isDistinct) { isDistinct_ = isDistinct; }
    bool isDistinct() const { return isDistinct_; }

    void setExpr(Expr* expr) { expr_ = expr; }
    Expr* expr() const { return expr_; }

    void appendDistinctBy(Expr* expr) { distinctBy_.emplace_back(expr); }
    void setDistinctBy(std::vector<Expr*>&& distinctBy) { distinctBy_ = std::move(distinctBy); }
    const std::vector<Expr*>& distinctBy() const { return distinctBy_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool equals(const Expr& other) const override;

    GeneralSetFunction funcName_;
    bool isDistinct_;
    Expr* expr_;
    std::vector<Expr*> distinctBy_;
};  // class AggFunc

inline bool AggFunc::equals(const Expr& other) const {
    const auto& expr = static_cast<const AggFunc&>(other);
    bool ret = (nullptr != expr_) && (nullptr != expr.expr_);
    ret = ret && funcName_ == expr.funcName_ && isDistinct_ == expr.isDistinct_ &&
          *expr_ == *expr.expr_ && distinctBy_.size() == expr.distinctBy_.size();
    for (auto i = 0u; i < distinctBy_.size() && ret; ++i) {
        ret = (nullptr != expr_) && (nullptr != expr.expr_);
        ret = ret && *distinctBy_[i] == *expr.distinctBy_[i];
    }
    return ret;
}

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_EXPR_AGGFUNC_H_
