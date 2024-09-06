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

#include "cypher/execution_plan/optimization/optimization_filter_visitor_impl.h"

namespace cypher {

class TraversalFilterDetector : public cypher::OptimizationFilterVisitorImpl {
 public:
    TraversalFilterDetector() {}

    virtual ~TraversalFilterDetector() = default;

    bool Build(geax::frontend::AstNode* astNode) {
        try {
            if (std::any_cast<geax::frontend::GEAXErrorCode>(astNode->accept(*this)) !=
                geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS) {
                return false;
            }
        } catch (const lgraph::CypherException& e) {
            return false;
        }
        return true;
    }

 private:
    std::any visit(geax::frontend::BAnd* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::BOr* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::GetField* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::Ref* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::BNotEqual* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::BEqual* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::BGreaterThan* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::BSmallerThan* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::BNotGreaterThan* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::BNotSmallerThan* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::VInt* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::VBool* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::VString* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::VDouble* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }
};

}  // namespace cypher
