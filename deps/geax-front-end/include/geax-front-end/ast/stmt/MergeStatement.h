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

#ifndef GEAXFRONTEND_AST_STMT_MERGESTATEMENT_H_
#define GEAXFRONTEND_AST_STMT_MERGESTATEMENT_H_

#include "geax-front-end/ast/clause/PathPattern.h"
#include "geax-front-end/ast/stmt/PrimitiveDataModifyStatement.h"
#include "geax-front-end/ast/stmt/SetStatement.h"

namespace geax {
namespace frontend {

class MergeStatement : public PrimitiveDataModifyStatement {
 public:
    MergeStatement()
        : PrimitiveDataModifyStatement(AstNodeType::kMergeStatement) {}
    ~MergeStatement() = default;

    void setPathPattern(PathPattern* pathPattern) {
        pathPattern_ = pathPattern;
    }
    PathPattern* pathPattern() const { return pathPattern_; }

    void appendOnMatch(SetStatement* set) { onMatch_.emplace_back(set); }
    void setOnMatch(std::vector<SetStatement*>&& onMatch) {
        onMatch_ = std::move(onMatch);
    }
    const std::vector<SetStatement*>& onMatch() const { return onMatch_; }

    void appendOnCreate(SetStatement* set) { onCreate_.emplace_back(set); }
    void setOnCreate(std::vector<SetStatement*>&& onCreate) {
        onCreate_ = std::move(onCreate);
    }
    const std::vector<SetStatement*>& onCreate() const { return onCreate_; }

    std::any accept(AstNodeVisitor& visitor) override {
        return visitor.visit(this);
    }

 private:
    PathPattern* pathPattern_;
    std::vector<SetStatement*> onMatch_;
    std::vector<SetStatement*> onCreate_;
};  // class MergeStatement

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_MERGESTATEMENT_H_
