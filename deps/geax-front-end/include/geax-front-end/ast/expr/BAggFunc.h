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

#ifndef GEAXFRONTEND_AST_EXPR_BAGGFUNC_H_
#define GEAXFRONTEND_AST_EXPR_BAGGFUNC_H_

#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

enum class BinarySetFunction { kPercentileCont, kPercentileDisc, kLastButNotUse };

inline const char* ToString(BinarySetFunction type) {
    static const StrArray<enumNum(BinarySetFunction::kLastButNotUse)> kDict = {
        "percentileCont",
        "percentileDisc",
    };
    const auto idx = static_cast<size_t>(type);
    return idx < kDict.size() ? kDict[idx] : geax::frontend::kUnknown;
}

inline bool ToEnum(std::string_view sv, BinarySetFunction& type) {
    static const std::unordered_map<std::string_view, BinarySetFunction> kTypeMap = {
        {"percentileCont", BinarySetFunction::kPercentileCont},
        {"percentileDisc", BinarySetFunction::kPercentileDisc},
    };
    auto it = kTypeMap.find(sv);
    return it != kTypeMap.end() && (type = it->second, true);
}

class BAggFunc : public Expr {
public:
    BAggFunc() : Expr(AstNodeType::kBAggFunc), rExpr_(nullptr) {}
    ~BAggFunc() = default;

    void setFuncName(BinarySetFunction funcName) { funcName_ = funcName; }
    BinarySetFunction funcName() const { return funcName_; }

    void setLExpr(bool isDistinct, Expr* expr) { lExpr_ = std::make_tuple(isDistinct, expr); }
    const std::tuple<bool, Expr*>& lExpr() const { return lExpr_; }

    void setRExpr(Expr* expr) { rExpr_ = expr; }
    Expr* rExpr() const { return rExpr_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool equals(const Expr& other) const override;

    BinarySetFunction funcName_;
    std::tuple<bool, Expr*> lExpr_;
    Expr* rExpr_;
};  // class BAggFunc

inline bool BAggFunc::equals(const Expr& other) const {
    const auto& expr = static_cast<const BAggFunc&>(other);
    bool ret = (nullptr != rExpr_) && (nullptr != expr.rExpr_) &&
               (nullptr != std::get<1>(lExpr_)) && (nullptr != std::get<1>(expr.lExpr_));
    ret = ret && funcName_ == expr.funcName_ && std::get<0>(lExpr_) == std::get<0>(expr.lExpr_) &&
          *std::get<1>(lExpr_) == *std::get<1>(expr.lExpr_) && *rExpr_ == *expr.rExpr_;
    return ret;
}

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_EXPR_BAGGFUNC_H_
