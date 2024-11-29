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

#include "core/data_type.h"
#include "cypher/execution_plan/optimization/optimization_filter_visitor_impl.h"

namespace cypher {

class PropertyFilterDetector : public cypher::OptimizationFilterVisitorImpl {
 public:
    PropertyFilterDetector() : isValidDetector(false) {}

    virtual ~PropertyFilterDetector() = default;

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

    std::unordered_map<std::string, std::unordered_map<std::string, std::set<lgraph::FieldData>>>&
    GetProperties() {
        return properties_;
    }

 private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<lgraph::FieldData>>>
        properties_;
    bool isValidDetector;
    std::string cur_symbol_;
    std::string cur_field_;
    std::set<lgraph::FieldData> cur_properties_;
    bool is_or_{false};
    bool is_and_{false};

    std::any visit(geax::frontend::BOr* node) override {
        if (is_and_) {
            return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_NOT_PASS;
        }
        is_or_ = true;
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::BAnd* node) override {
        if (is_or_) {
            return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_NOT_PASS;
        }
        is_and_ = true;
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::GetField* node) override {
        isValidDetector = true;
        cur_field_ = node->fieldName();
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::Ref* node) override {
        cur_symbol_ = node->name();
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::BEqual* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        if (!cur_properties_.empty()) {
            auto& fields_map = properties_[cur_symbol_];
            if (is_and_) {
                auto& fields = fields_map[cur_field_];
                if (fields.size() == 0) {
                    fields_map[cur_field_].insert(cur_properties_.begin(), cur_properties_.end());
                } else {
                    std::set<lgraph::FieldData> intersection;
                    for (auto& property : cur_properties_) {
                        if (fields.count(property)) {
                            intersection.insert(property);
                        }
                    }
                    fields_map[cur_field_] = intersection;
                }
            } else {
                if (fields_map.size() > 1 ||
                    (fields_map.size() == 1 && fields_map.count(cur_field_) == 0)) {
                    return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_NOT_PASS;
                }
                fields_map[cur_field_].insert(cur_properties_.begin(), cur_properties_.end());
            }
            cur_properties_.clear();
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
        if (!cur_properties_.empty()) {
            auto& fields_map = properties_[cur_symbol_];
            if (is_and_) {
                auto& fields = fields_map[cur_field_];
                if (fields.size() == 0) {
                    fields_map[cur_field_].insert(cur_properties_.begin(), cur_properties_.end());
                } else {
                    std::set<lgraph::FieldData> intersection;
                    for (auto& property : cur_properties_) {
                        if (fields.count(property)) {
                            intersection.insert(property);
                        }
                    }
                    fields_map[cur_field_] = intersection;
                }
            } else {
                if (fields_map.size() > 1 ||
                    (fields_map.size() == 1 && fields_map.count(cur_field_) == 0)) {
                    return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_NOT_PASS;
                }
                fields_map[cur_field_].insert(cur_properties_.begin(), cur_properties_.end());
            }
            cur_properties_.clear();
        }
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::VInt* node) override {
        if (isValidDetector) cur_properties_.insert(lgraph::FieldData(node->val()));
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::VBool* node) override {
        if (isValidDetector) cur_properties_.insert(lgraph::FieldData(node->val()));
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::VString* node) override {
        if (isValidDetector) cur_properties_.insert(lgraph::FieldData(node->val()));
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::VDouble* node) override {
        if (isValidDetector) cur_properties_.insert(lgraph::FieldData(node->val()));
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }
};

}  // namespace cypher
