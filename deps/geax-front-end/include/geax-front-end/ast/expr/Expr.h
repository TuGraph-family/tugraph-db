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

#ifndef GEAXFRONTEND_AST_EXPR_EXPR_H_
#define GEAXFRONTEND_AST_EXPR_EXPR_H_

#include "geax-front-end/ast/AstNode.h"

namespace geax {
namespace frontend {

class GQLExpr;

/**
 * An Expr stands for an expression node.
 */
class Expr : public AstNode {
public:
    explicit Expr(AstNodeType type) : AstNode(type) {}
    virtual ~Expr() = default;

    friend bool operator==(const Expr& lhs, const Expr& rhs) {
        return lhs.type() == rhs.type() && lhs.equals(rhs);
    }

protected:
    virtual bool equals(const Expr& other) const = 0;

    // Representation of expressions in json could be various, one solution is
    // like Lisp's 'S-expression'
    // (https://stackoverflow.com/questions/20737045/representing-logic-as-data-in-json)
    //
    // But we employ another here:
    // Say we have an expression (a and b) or c, and we express it in json like
    //
    // {
    //  "type": "OR",
    //  "left": {
    //          "type": "IntegerLiteral"
    //          "IntegerLiteral": c
    //  },
    //  "right": {
    //          "type": "AND",
    //          "left": {
    //                  "type": "StringLiteral",
    //                  "val": "a"
    //          },
    //          "right": {
    //                  "type": "BoolLiteral",
    //                  "val": true
    //          }
    //      }
    // }
    //
    // I think this is expressive and easy to parse, and let's see if this format is good enough.
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_EXPR_EXPR_H_
