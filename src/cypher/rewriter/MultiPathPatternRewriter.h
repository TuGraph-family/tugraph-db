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

#include <vector>
#include "geax-front-end/ast/clause/ElementFiller.h"

#include "cypher/utils/ast_node_visitor_impl.h"
#include "cypher/parser/data_typedef.h"
#include "geax-front-end/ast/expr/Expr.h"
#include "geax-front-end/common/ObjectAllocator.h"

namespace cypher {

class MultiPathPatternRewriter : public AstNodeVisitorImpl {
 public:
    explicit MultiPathPatternRewriter(geax::common::ObjectArenaAllocator& allocator)
        : allocator_(allocator) {}
    std::any visit(geax::frontend::GraphPattern* node) override {
        in_graph_pattern_ = true;
        node_alias_.clear();
        filter_.clear();
        for (auto path_pattern : node->pathPatterns()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(path_pattern);
        }
        if (node->where().has_value()) {
            filter_.emplace_back(node->where().value());
        }
        auto filter = MergeFilter(filter_, allocator_);
        if (filter) {
            node->setWhere(filter);
        }
        in_graph_pattern_ = false;
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Node* node) override {
        in_node_ = true;
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->filler());
        in_node_ = false;
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::ElementFiller* node) override {
        if (in_graph_pattern_ && in_node_ && node->v().has_value()) {
            if (node_alias_.find(node->v().value()) != node_alias_.end()) {
                std::string pre_name = node->v().value();
                node->setV(parser::ANONYMOUS + std::string("MULTI_") + std::to_string(idx_++));
                std::string new_name = node->v().value();
                auto lhs = allocator_.allocate<geax::frontend::Ref>();
                lhs->setName(std::move(pre_name));
                auto rhs = allocator_.allocate<geax::frontend::Ref>();
                rhs->setName(std::move(new_name));
                auto equal = allocator_.allocate<geax::frontend::BEqual>();
                equal->setLeft(lhs);
                equal->setRight(rhs);
                filter_.push_back(equal);
            }
            node_alias_.insert(node->v().value());
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

 private:
    static geax::frontend::Expr* MergeFilter(std::vector<geax::frontend::Expr*>& filter,
                                             geax::common::ObjectArenaAllocator& allocator) {
        geax::frontend::Expr* ret = nullptr;
        for (auto expr : filter) {
            if (ret == nullptr) {
                ret = expr;
            } else {
                auto and_expr = allocator.allocate<geax::frontend::BAnd>();
                and_expr->setLeft(ret);
                and_expr->setRight(expr);
                ret = and_expr;
            }
        }
        return ret;
    }
    size_t idx_ = 0;
    bool in_graph_pattern_ = false;
    bool in_node_ = false;
    geax::common::ObjectArenaAllocator& allocator_;
    std::set<std::string> node_alias_;
    std::vector<geax::frontend::Expr*> filter_;
};

}  // namespace cypher
