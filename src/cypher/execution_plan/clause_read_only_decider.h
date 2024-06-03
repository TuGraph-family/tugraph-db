/**
 * Copyright 2023 AntGroup CO., Ltd.
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

#include "cypher/utils/ast_node_visitor_impl.h"

namespace cypher {

class ClauseReadOnlyDecider : public cypher::AstNodeVisitorImpl {
 public:
    ClauseReadOnlyDecider() {}

    virtual ~ClauseReadOnlyDecider() = default;

    /**
     *  Determine if there are any invalid aggregate functions.
     *  Return true if there are none.
     */
    bool IsReadOnly() {
        return read_only_;
    }

    geax::frontend::GEAXErrorCode Build(geax::frontend::AstNode* root) {
        return std::any_cast<geax::frontend::GEAXErrorCode>(root->accept(*this));
    }

 private:
    std::any visit(geax::frontend::AmbientLinearQueryStatement* node) override {
        read_only_ = true;
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::LinearDataModifyingStatement* node) override {
        read_only_ = false;
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    bool read_only_ = true;
};

}  // namespace cypher
