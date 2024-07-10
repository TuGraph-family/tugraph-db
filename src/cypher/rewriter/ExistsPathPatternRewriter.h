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
#include "geax-front-end/ast/clause/GraphPattern.h"
#include "geax-front-end/ast/clause/PathChain.h"
#include "geax-front-end/ast/clause/PathPattern.h"
#include "geax-front-end/ast/clause/WhereClause.h"
#include "geax-front-end/ast/stmt/PrimitiveResultStatement.h"
#include "geax-front-end/common/ObjectAllocator.h"
#include "tools/lgraph_log.h"

namespace cypher {

class ExistsPathPatternRewriter : public AstNodeVisitorImpl {
 public:
    ExistsPathPatternRewriter(geax::common::ObjectArenaAllocator& allocator)
        : allocator_(allocator), has_exists_(false) {}
    ~ExistsPathPatternRewriter() = default;

 private:
    geax::frontend::PathPattern* path_pattern_;
    geax::common::ObjectArenaAllocator& allocator_;
    bool has_exists_;

    DISABLE_COPY(ExistsPathPatternRewriter);
    DISABLE_MOVE(ExistsPathPatternRewriter);

    std::any visit(geax::frontend::Exists* node) override {
        LOG_INFO() << "--------------ExistsPathPatternRewriter Exists";
        has_exists_ = true;
        path_pattern_ = allocator_.allocate<geax::frontend::PathPattern>();
        auto& path_chains = node->pathChains();
        for (auto& path_chain : path_chains) {
            path_pattern_->appendChain(path_chain);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::GraphPattern* node) override {
        LOG_INFO() << "--------------ExistsPathPatternRewriter GraphPattern";
        if (node->where().has_value()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->where().value());
        }
        if (!has_exists_) return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
        node->appendPathPattern(path_pattern_);
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MatchStatement* node) override {
        LOG_INFO() << "--------------ExistsPathPatternRewriter MatchStatement";
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->graphPattern());
        if (!has_exists_) return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
        node->setStatementMode(geax::frontend::StatementMode::kOptional);
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::PrimitiveResultStatement* node) override {
        LOG_INFO() << "--------------ExistsPathPatternRewriter PrimitiveResultStatement";
        for (auto item : node->items()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(std::get<1>(item));
        }
        if (!has_exists_) return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }
};

}  // namespace cypher
