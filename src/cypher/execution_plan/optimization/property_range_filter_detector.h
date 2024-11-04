/**
 * Copyright 2024 AntGroup CO., Ltd.
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

class PropertyRangeFilterDetector : public cypher::OptimizationFilterVisitorImpl {
 public:
    PropertyRangeFilterDetector() {}

    virtual ~PropertyRangeFilterDetector() = default;

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

    std::unordered_map<
        std::string,
        std::unordered_map<std::string, std::pair<lgraph::FieldData, lgraph::FieldData>>>&
    GetProperties() {
        return properties_;
    }

 private:
    std::unordered_map<
        std::string,
        std::unordered_map<std::string, std::pair<lgraph::FieldData, lgraph::FieldData>>>
        properties_;
    std::string cur_symbol_;
    std::string cur_field_;
    lgraph::FieldData cur_value_;
    bool is_conjunction_ = false;
    bool is_disjunction_ = false;
    std::pair<lgraph::FieldData, lgraph::FieldData> cur_bounds_;

    bool in_bounds(const std::pair<lgraph::FieldData, lgraph::FieldData>& bounds,
                   const lgraph::FieldData& value) {
        CYPHER_THROW_ASSERT(!bounds.first.is_null() || !bounds.second.is_null());
        if (!bounds.first.is_null() && !bounds.second.is_null()) {
            return value > bounds.first && value < bounds.second;
        }
        if (!bounds.first.is_null()) {
            return value > bounds.first;
        }
        if (!bounds.second.is_null()) {
            return value < bounds.second;
        }
        return false;
    }

    /* merge cur_bounds_ into bounds, return true if the merge succeed, false else */
    bool merge_bounds(std::pair<lgraph::FieldData, lgraph::FieldData>& bounds) {
        CYPHER_THROW_ASSERT(is_conjunction_ != is_disjunction_);
        if (is_conjunction_) {
            if (!cur_bounds_.first.is_null()) {
                if (in_bounds(bounds, cur_bounds_.first)) {
                    bounds.first = cur_bounds_.first;
                } else if (cur_bounds_.first >= bounds.second) {
                    return false;
                }
            } else {
                if (in_bounds(bounds, cur_bounds_.second)) {
                    bounds.second = cur_bounds_.second;
                } else if (cur_bounds_.second <= bounds.first) {
                    return false;
                }
            }
        } else {
            if (!cur_bounds_.first.is_null()) {
                if (bounds.first.is_null()) {
                    return false;
                } else if (cur_bounds_.first < bounds.first) {
                    bounds.first = cur_bounds_.first;
                }
            } else {
                if (bounds.second.is_null()) {
                    return false;
                } else if (cur_bounds_.second > bounds.second) {
                    bounds.second = cur_bounds_.second;
                }
            }
        }
        // reset cur_bounds_
        cur_bounds_.first = lgraph::FieldData();
        cur_bounds_.second = lgraph::FieldData();
        return true;
    }

    std::any visit(geax::frontend::BOr* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        if (is_conjunction_) {
            return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_NOT_PASS;
        }
        is_disjunction_ = true;
        if (!cur_bounds_.first.is_null() || !cur_bounds_.second.is_null()) {
            // get the bookkeeping bounds and do the merge
            auto& fields_map = properties_[cur_symbol_];
            auto& field_bounds = fields_map[cur_field_];
            // if merge failed, optimization not pass
            if (!merge_bounds(field_bounds)) {
                return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_NOT_PASS;
            }
        }
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::BAnd* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        if (is_disjunction_) {
            return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;
        }
        is_conjunction_ = true;
        if (!cur_bounds_.first.is_null() || !cur_bounds_.second.is_null()) {
            // get the bookkeeping bounds and do the merge
            auto& fields_map = properties_[cur_symbol_];
            auto& field_bounds = fields_map[cur_field_];
            // if merge failed, optimization not pass
            if (!merge_bounds(field_bounds)) {
                return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_NOT_PASS;
            }
        }
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::GetField* node) override {
        cur_field_ = node->fieldName();
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::Ref* node) override {
        cur_symbol_ = node->name();
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::BSmallerThan* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        auto& fields_map = properties_[cur_symbol_];
        auto& field_bounds = fields_map[cur_field_];
        if (field_bounds.first.is_null() && field_bounds.second.is_null()) {
            // left bound and right bound both not set, set one
            field_bounds.second = cur_value_;
        } else {
            // set cur_bounds and do the merge later
            cur_bounds_.first = lgraph::FieldData();
            cur_bounds_.second = cur_value_;
        }
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::BGreaterThan* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        auto& fields_map = properties_[cur_symbol_];
        auto& field_bounds = fields_map[cur_field_];
        if (field_bounds.first.is_null() && field_bounds.second.is_null()) {
            // left bound and right bound both not set, set one
            field_bounds.first = cur_value_;
        } else {
            // set cur_bounds and do the merge later
            cur_bounds_.first = cur_value_;
            cur_bounds_.second = lgraph::FieldData();
        }

        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::VInt* node) override {
        cur_value_ = lgraph::FieldData(node->val());
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }

    std::any visit(geax::frontend::VDouble* node) override {
        cur_value_ = lgraph::FieldData(node->val());
        return geax::frontend::GEAXErrorCode::GEAX_OPTIMIZATION_PASS;
    }
};

}  // namespace cypher
