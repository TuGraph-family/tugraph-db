//
// Created by lipanpan on 2024/6/18.
//

#ifndef GEAXFRONTEND_AST_CLAUSE_REMOVEITEM_H_
#define GEAXFRONTEND_AST_CLAUSE_REMOVEITEM_H_

#include "geax-front-end/ast/AstNode.h"

namespace geax {
namespace frontend {

class RemoveItem : public AstNode {
 public:
    explicit RemoveItem(AstNodeType type) : AstNode(type) {}
    virtual ~RemoveItem() = default;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_REMOVEITEM_H_
