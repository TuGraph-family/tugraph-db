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
*         botu.wzy@antgroup.com
*/

#ifndef GEAXFRONTEND_AST_CLAUSE_REMOVELABEL_H_
#define GEAXFRONTEND_AST_CLAUSE_REMOVELABEL_H_
#include <vector>
#include "geax-front-end/ast/clause/RemoveItem.h"

namespace geax {
namespace frontend {

class RemoveLabel : public RemoveItem {
  public:
   RemoveLabel() : RemoveItem(AstNodeType::kRemoveLabel) {}
   ~RemoveLabel() = default;

   void setV(std::string&& v) { v_ = std::move(v); }
   const std::string& v() const { return v_; }

   void setLabels(std::vector<std::string> labels) {
       labels_ = std::move(labels);
   }
   const std::vector<std::string>& labels() const { return labels_; }

   std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

  private:
   std::string v_;
   std::vector<std::string> labels_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_REMOVELABEL_H_
