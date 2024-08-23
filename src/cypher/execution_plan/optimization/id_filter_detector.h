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

class IDFilterDetector : public cypher::OptimizationFilterVisitorImpl {
 public:
    IDFilterDetector() : isValidDetector(false) {}

    virtual ~IDFilterDetector() = default;

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

    std::unordered_map<std::string, std::set<uint64_t>>& GetVids() { return vids_; }

 private:
    std::unordered_map<std::string, std::set<uint64_t>> vids_;
    bool isValidDetector;
    std::string cur_symbol_;
    std::set<uint64_t> cur_vids_;

    std::any visit(geax::frontend::BOr* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::BAnd* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Function* node) override {
        if (node->name() == "id") isValidDetector = true;
        for (auto& arg : node->args()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(arg);
        }
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::Ref* node) override {
        cur_symbol_ = node->name();
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::BEqual* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        if (!cur_vids_.empty()) {
            auto& vids = vids_[cur_symbol_];
            vids.insert(cur_vids_.begin(), cur_vids_.end());
            cur_vids_.clear();
        }
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::MkList* node) override {
        for (auto& expr : node->elems()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(expr);
        }
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::BIn* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        if (!cur_vids_.empty()) {
            auto& vids = vids_[cur_symbol_];
            vids.insert(cur_vids_.begin(), cur_vids_.end());
            cur_vids_.clear();
        }
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::VInt* node) override {
        if (isValidDetector) cur_vids_.insert(node->val());
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }
};

}  // namespace cypher
