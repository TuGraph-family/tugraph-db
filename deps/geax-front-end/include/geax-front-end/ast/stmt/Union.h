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
 *         Yaochi <boyao.zby@alibaba-inc.com>
 */

#ifndef GEAXFRONTEND_AST_STMT_UNION_H_
#define GEAXFRONTEND_AST_STMT_UNION_H_

#include "geax-front-end/ast/stmt/QueryConjunctionType.h"

namespace geax {
namespace frontend {

class Union : public QueryConjunctionType {
public:
    Union()
        : QueryConjunctionType(AstNodeType::kUnion),
          isDistinct_(true) {}
    ~Union() = default;

    void setDistince(const bool& isDistinct) { isDistinct_ = isDistinct; }
    const bool distinct() const { return isDistinct_; }

    std::any accept(AstNodeVisitor& visitor) override {
        return visitor.visit(this);
    }

    DEFINE_FROM_AND_TO_JSON

private:
    DECLARE_JSON_FIELDS() {
        JSON_FIELD(type);
        JSON_FIELD(isDistinct);
    }

    bool isDistinct_;
};  // class Union

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_UNION_H_
