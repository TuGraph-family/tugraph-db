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
#include "procedure/procedure.h"

namespace cypher {

class ClauseReadOnlyDecider : public cypher::AstNodeVisitorImpl {
 public:
    ClauseReadOnlyDecider() = default;

    ~ClauseReadOnlyDecider() override = default;

    [[nodiscard]] bool IsReadOnly() const {
        return read_only_;
    }

    geax::frontend::GEAXErrorCode Build(geax::frontend::AstNode* root, cypher::RTContext* ctx) {
        ctx_ = ctx;
        return std::any_cast<geax::frontend::GEAXErrorCode>(root->accept(*this));
    }

 private:
    std::any visit(geax::frontend::AmbientLinearQueryStatement* node) override {
        read_only_ = true;
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->resultStatement());
        for (auto query_statement : node->queryStatements()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(query_statement);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::LinearDataModifyingStatement* node) override {
        read_only_ = false;
        for (auto query_statement : node->queryStatements()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(query_statement);
        }
        for (auto modify_statement : node->modifyStatements()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(modify_statement);
        }
        if (node->resultStatement().has_value()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->resultStatement().value());
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::NamedProcedureCall* node) override;

    std::any visit(geax::frontend::InQueryProcedureCall* node) override;

    bool read_only_ = true;
    cypher::RTContext* ctx_{};
};

}  // namespace cypher
