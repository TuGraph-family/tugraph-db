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

#include <cstdint>
#include "core/data_type.h"
#include "cypher/utils/ast_node_visitor_impl.h"
#include "geax-front-end/ast/expr/BIn.h"
#include "geax-front-end/ast/expr/BNotEqual.h"
#include "geax-front-end/ast/expr/GetField.h"
#include "geax-front-end/ast/expr/MkList.h"
#include "geax-front-end/ast/expr/Ref.h"
#include "geax-front-end/ast/expr/VBool.h"
#include "geax-front-end/ast/expr/VDouble.h"
#include "geax-front-end/ast/expr/VInt.h"
#include "geax-front-end/ast/expr/VString.h"

namespace cypher {

class PropertyFilterDetector : public cypher::AstNodeVisitorImpl {
 public:
    PropertyFilterDetector() : isValidDetector(false) {}

    virtual ~PropertyFilterDetector() = default;

    bool Build(geax::frontend::AstNode* astNode) {
        LOG_INFO() << "------------into FilterDetector::Build";
        if (std::any_cast<geax::frontend::GEAXErrorCode>(astNode->accept(*this)) !=
            geax::frontend::GEAXErrorCode::GEAX_SUCCEED) {
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

    std::any visit(geax::frontend::BAnd* node) override {
        LOG_INFO() << "-----------FilterDetector BAnd";
        return geax::frontend::GEAXErrorCode::GEAX_ERROR;
    }

    std::any visit(geax::frontend::BOr* node) override {
        LOG_INFO() << "-----------FilterDetector BOr";
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::GetField* node) override {
        LOG_INFO() << "-----------FilterDetector GetField";
        isValidDetector = true;
        cur_field_ = node->fieldName();
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Ref* node) override {
        LOG_INFO() << "-----------FilterDetector Ref";
        cur_symbol_ = node->name();
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BNotEqual* node) override {
        LOG_INFO() << "-----------FilterDetector BNotEqual";
        return geax::frontend::GEAXErrorCode::GEAX_ERROR;
    }

    std::any visit(geax::frontend::BEqual* node) override {
        LOG_INFO() << "-----------FilterDetector BEqual";
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        if (!cur_properties_.empty()) {
            auto& fields_map = properties_[cur_symbol_];
            auto& fields = fields_map[cur_field_];
            fields.insert(cur_properties_.begin(), cur_properties_.end());
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MkList* node) override {
        LOG_INFO() << "-----------FilterDetector MkList";
        for (auto& expr : node->elems()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(expr);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BIn* node) override {
        LOG_INFO() << "-----------FilterDetector BIn";
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        if (!cur_properties_.empty()) {
            auto& fields_map = properties_[cur_symbol_];
            auto& fields = fields_map[cur_field_];
            fields.insert(cur_properties_.begin(), cur_properties_.end());
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VInt* node) override {
        LOG_INFO() << "-----------FilterDetector VInt";
        if (isValidDetector) cur_properties_.insert(lgraph::FieldData(node->val()));
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VBool* node) override {
        LOG_INFO() << "-----------FilterDetector VInt";
        if (isValidDetector) cur_properties_.insert(lgraph::FieldData(node->val()));
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VString* node) override {
        LOG_INFO() << "-----------FilterDetector VInt";
        if (isValidDetector) cur_properties_.insert(lgraph::FieldData(node->val()));
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VDouble* node) override {
        LOG_INFO() << "-----------FilterDetector VInt";
        if (isValidDetector) cur_properties_.insert(lgraph::FieldData(node->val()));
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

};

}  // namespace cypher