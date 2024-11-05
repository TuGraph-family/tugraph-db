/**
 * Copyright 2022 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License") {}
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#pragma once

#include "geax-front-end/ast/clause/ElementFiller.h"
#include "cypher/utils/ast_node_visitor_impl.h"
#include "cypher/parser/data_typedef.h"

namespace cypher {

class GenAnonymousAliasRewriter : public AstNodeVisitorImpl {
 public:
    std::any visit(geax::frontend::ElementFiller* node) override {
        if (!node->v().has_value() || (node->v().has_value() && node->v().value().empty())) {
            node->setV(parser::ANONYMOUS + std::to_string(idx_++));
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

 private:
    size_t idx_ = 0;
};

}  // namespace cypher
