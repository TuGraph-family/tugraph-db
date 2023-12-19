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
 *         Qingye <liubingye.lby@antgroup.com>
 */

#ifndef GEAXFRONTEND_AST_CLAUSE_EDGEONJOIN_H_
#define GEAXFRONTEND_AST_CLAUSE_EDGEONJOIN_H_

#include "geax-front-end/ast/clause/Hint.h"

namespace geax {
namespace frontend {

class EdgeOnJoin : public Hint {
public:
    EdgeOnJoin() : Hint(AstNodeType::kEdgeOnJoin) {}
    ~EdgeOnJoin() = default;

    void setEdge(std::string&& edge) { edge_ = std::move(edge); }
    const std::string& edge() const { return edge_; }

    void setJoinKey(std::string&& joinKey) { joinKey_ = std::move(joinKey); }
    const std::string& joinKey() const { return joinKey_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::string edge_;
    std::string joinKey_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_EDGEONJOIN_H_
