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
 *         Lili <liangjingru.ljr@antgroup.com>
 */

#ifndef GEAXFRONTEND_AST_EXPR_EXISTS_H_
#define GEAXFRONTEND_AST_EXPR_EXISTS_H_

#include "geax-front-end/ast/clause/PathChain.h"
#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

class Exists : public Expr {
public:
    Exists() : Expr(AstNodeType::kExists) {}
    ~Exists() = default;

    void appendPathChain(PathChain* pathChain) { pathChains_.emplace_back(pathChain); }
    void setPathChains(std::vector<PathChain*>&& pathChains) {
        pathChains_ = std::move(pathChains);
    }
    const std::vector<PathChain*>& pathChains() const { return pathChains_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool equals(const Expr& other) const override;

    std::vector<PathChain*> pathChains_;
};  // class Exists

inline bool Exists::equals(const Expr& other) const {
    const auto& expr = static_cast<const Exists&>(other);
    // TODO(ljr): PathChain equals
    bool ret = pathChains_.size() == expr.pathChains_.size();
    return ret;
}

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_EXPR_EXISTS_H_
