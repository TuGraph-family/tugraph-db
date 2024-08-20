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
#include "cypher/cypher_exception.h"
#include "utils/geax_util.h"

namespace cypher {

class OptimizationFilterVisitorImpl : public cypher::AstNodeVisitorImpl {
 public:
    OptimizationFilterVisitorImpl() {}

    virtual ~OptimizationFilterVisitorImpl() {};

 private:
    std::any visit(geax::frontend::AggFunc* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->expr());
        for (auto& expr : node->distinctBy()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(expr);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::AllDifferent* node) override {
        for (auto item : node->items()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(item);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BAdd* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BAggFunc* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->rExpr());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(std::get<1>(node->lExpr()));
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BAnd* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BBitAnd* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BBitLeftShift* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BBitOr* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BBitRightShift* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BBitXor* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BConcat* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BDiv* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BEqual* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BGreaterThan* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BIn* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BIndex* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }
    
    std::any visit(geax::frontend::BLike* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BMod* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BMul* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BNotEqual* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BNotGreaterThan* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BNotSmallerThan* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BOr* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BSafeEqual* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BSmallerThan* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BSquare* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }
    
    std::any visit(geax::frontend::BSub* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BXor* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Case* node) override {
        if (node->input().has_value()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(node->input().value());
        }
        for (auto& case_body : node->caseBodies()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(std::get<0>(case_body));
            ACCEPT_AND_CHECK_WITH_PASS_MSG(std::get<1>(case_body));
        }
        if (node->elseBody().has_value()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(node->elseBody().value());
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Cast* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Exists* node) override {
        for (auto path_chain : node->pathChains()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(path_chain);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Function* node) override {
        for (auto& arg : node->args()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(arg);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::GetField* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::If* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->condition());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->trueBody());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->falseBody());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::IsDestinationOf* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::IsDirected* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::IsLabeled* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->expr());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->labelTree());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::IsNormalized* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::IsNull* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::IsSourceOf* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::ListComprehension* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MatchCase* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->input());
        for (auto& case_body : node->cases()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(std::get<0>(case_body));
            ACCEPT_AND_CHECK_WITH_PASS_MSG(std::get<1>(case_body));
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MkList* node) override {
        for (auto elem : node->elems()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(elem);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MkMap* node) override {
        for (auto [elem1, elem2] : node->elems()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(elem1);
            ACCEPT_AND_CHECK_WITH_PASS_MSG(elem2);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MkRecord* node) override {
        for (auto [elem1, elem2] : node->elems()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(elem2);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MkSet* node) override {
        for (auto elem : node->elems()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(elem);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MkTuple* node) override {
        for (auto elem : node->elems()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(elem);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MultiCount* node) override {
        for (auto& arg : node->args()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(arg);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Neg* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Not* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Param* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Ref* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Same* node) override {
        for (auto item : node->items()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(item);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Tilde* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::TupleGet* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VBool* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VDate* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VDatetime* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }
        
    std::any visit(geax::frontend::VDouble* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }
    
    std::any visit(geax::frontend::VDuration* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }
    
    std::any visit(geax::frontend::VInt* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VNone* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VNull* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VSome* node) override {
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VString* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VTime* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Windowing* node) override {
        for (auto expr : node->partitionBy()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(expr);
        }
        for (auto [expr, b] : node->orderByClause()) {
            ACCEPT_AND_CHECK_WITH_PASS_MSG(expr);
        }
        ACCEPT_AND_CHECK_WITH_PASS_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }


    std::any visit(geax::frontend::DummyNode* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }
};

}  // namespace cypher
