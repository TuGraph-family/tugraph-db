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

#ifndef GEAXFRONTEND_AST_CLAUSE_EXCEPT_H_
#define GEAXFRONTEND_AST_CLAUSE_EXCEPT_H_

#include "geax-front-end/ast/clause/QueryConjunctionType.h"

namespace geax {
namespace frontend {

class Except : public QueryConjunctionType {
public:
    Except() : QueryConjunctionType(AstNodeType::kExcept), isDistinct_(true) {}
    ~Except() = default;

    void setDistinct(bool isDistinct) { isDistinct_ = isDistinct; }
    bool distinct() const { return isDistinct_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool isDistinct_;
};  // class Except

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_EXCEPT_H_
