//
// Created by lipanpan on 2024/6/18.
//

#ifndef GEAXFRONTEND_AST_CLAUSE_REMOVESINGLEPROPERTY_H_
#define GEAXFRONTEND_AST_CLAUSE_REMOVESINGLEPROPERTY_H_

#include "geax-front-end/ast/clause/RemoveItem.h"
#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

class RemoveSingleProperty : public RemoveItem {
 public:
    RemoveSingleProperty() : RemoveItem(AstNodeType::kRemoveSingleProperty) {}
    ~RemoveSingleProperty() = default;

    void setV(std::string&& v) { v_ = std::move(v); }
    const std::string& v() const { return v_; }

    void setProperty(std::string&& property) { property_ = property; }
    const std::string& property() const { return property_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

 private:
    std::string v_;
    std::string property_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_REMOVESINGLEPROPERTY_H_
