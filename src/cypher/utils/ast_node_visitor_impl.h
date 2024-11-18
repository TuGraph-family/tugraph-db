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

#include "geax-front-end/GEAXErrorCode.h"
#include "geax-front-end/ast/Ast.h"
#include "utils/geax_util.h"

namespace cypher {

class AstNodeVisitorImpl : public geax::frontend::AstNodeVisitor {
 public:
    AstNodeVisitorImpl() = default;

    virtual ~AstNodeVisitorImpl() = default;

    std::any reportError() override {
        return error_msg_;
    }

 private:
    std::any visit(geax::frontend::GetField* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::TupleGet* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Not* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Neg* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Tilde* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VSome* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BEqual* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BNotEqual* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BGreaterThan* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BNotSmallerThan* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BSmallerThan* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BNotGreaterThan* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BSafeEqual* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BAdd* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BSub* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BDiv* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BMul* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BMod* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BSquare* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BAnd* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BOr* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BXor* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BBitAnd* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BBitOr* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BBitXor* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BBitLeftShift* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BBitRightShift* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BConcat* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BIndex* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BLike* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BIn* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::If* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->condition());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->trueBody());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->falseBody());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Function* node) override {
        for (auto& arg : node->args()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(arg);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Case* node) override {
        if (node->input().has_value()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->input().value());
        }
        for (auto& case_body : node->caseBodies()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(std::get<0>(case_body));
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(std::get<1>(case_body));
        }
        if (node->elseBody().has_value()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->elseBody().value());
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Cast* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MatchCase* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->input());
        for (auto& case_body : node->cases()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(std::get<0>(case_body));
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(std::get<1>(case_body));
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::AggFunc* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        for (auto& expr : node->distinctBy()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(expr);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BAggFunc* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->rExpr());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(std::get<1>(node->lExpr()));
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MultiCount* node) override {
        for (auto& arg : node->args()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(arg);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Windowing* node) override {
        for (auto expr : node->partitionBy()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(expr);
        }
        for (auto [expr, b] : node->orderByClause()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(expr);
        }
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MkList* node) override {
        for (auto elem : node->elems()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(elem);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MkMap* node) override {
        for (auto [elem1, elem2] : node->elems()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(elem1);
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(elem2);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MkRecord* node) override {
        for (auto [elem1, elem2] : node->elems()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(elem2);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MkSet* node) override {
        for (auto elem : node->elems()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(elem);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MkTuple* node) override {
        for (auto elem : node->elems()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(elem);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VBool* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VInt* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VDouble* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VString* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VDate* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VDatetime* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VDuration* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VTime* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VNull* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::VNone* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Ref* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::GraphPattern* node) override {
        for (auto path_pattern : node->pathPatterns()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(path_pattern);
        }
        if (node->keep().has_value()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->keep().value());
        }
        if (node->where().has_value()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->where().value());
        }
        if (node->yield().has_value()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->yield().value());
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::PathPattern* node) override {
        if (node->prefix().has_value()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->prefix().value());
        }
        for (auto path_chain : node->chains()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(path_chain);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::PathChain* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->head());
        for (auto [e, n] : node->tails()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(e);
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(n);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Node* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->filler());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Edge* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->filler());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::ElementFiller* node) override {
        if (node->label().has_value()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->label().value());
        }
        for (auto element_predicate : node->predicates()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(element_predicate);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::WhereClause* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->predicate());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::OrderByField* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->field());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::PathModePrefix* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::PathSearchPrefix* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::SingleLabel* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::LabelOr* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::LabelAnd* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::LabelNot* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::PropStruct* node) override {
        for (auto [s, property] : node->properties()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(property);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::YieldField* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::TableFunctionClause* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->function());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::ReadConsistency* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::AllowAnonymousTable* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::OpConcurrent* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::EdgeOnJoin* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::SetAllProperties* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->structs());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::SetLabel* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->label());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::IsNull* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::IsDirected* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::IsNormalized* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::IsSourceOf* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::IsDestinationOf* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->left());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->right());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::IsLabeled* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->labelTree());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Same* node) override {
        for (auto item : node->items()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(item);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::AllDifferent* node) override {
        for (auto item : node->items()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(item);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Exists* node) override {
        for (auto path_chain : node->pathChains()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(path_chain);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::ProcedureBody* node) override {
        if (node->schemaRef().has_value()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->schemaRef().value());
        }
        for (auto binding : node->bindingDefinitions()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(binding);
        }
        for (auto statement_with_yield : node->statements()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(statement_with_yield);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::SchemaFromPath* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BindingValue* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BindingGraph* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BindingTable* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::StatementWithYield* node) override {
        if (node->yield().has_value()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->yield().value());
        }
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->statement());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::QueryStatement* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->joinQuery());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::JoinQueryExpression* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->head());
        for (auto [part, result_statement] : node->body()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(part);
            if (result_statement.has_value()) {
                ACCEPT_AND_CHECK_WITH_ERROR_MSG(result_statement.value());
            }
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::JoinRightPart* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->rightBody());
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->onExpr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::CompositeQueryStatement* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->head());
        for (auto [conhjunction, query_statement] : node->body()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(conhjunction);
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(query_statement);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::AmbientLinearQueryStatement* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->resultStatement());
        for (auto query_statement : node->queryStatements()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(query_statement);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::SelectStatement* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->resultStatement());
        for (auto [s, query_statement] : node->fromClauses()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(query_statement);
        }
        if (node->where().has_value()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->where().value());
        }
        if (node->having().has_value()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->having().value());
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::FocusedQueryStatement* node) override {
        for (auto [s, query_statement] : node->queryList()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(query_statement);
        }
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->resultStatement());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::FocusedResultStatement* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->resultStatement());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MatchStatement* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->graphPattern());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::FilterStatement* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->predicate());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::PrimitiveResultStatement* node) override {
        for (auto item : node->items()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(std::get<1>(item));
        }
        for (auto key : node->groupKeys()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(key);
        }
        for (auto order_by_key : node->orderBys()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(order_by_key);
        }
        for (auto hint : node->hints()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(hint);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::CatalogModifyStatement* node) override {
        for (auto catalog_statement : node->statements()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(catalog_statement);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::LinearDataModifyingStatement* node) override {
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

    std::any visit(geax::frontend::InsertStatement* node) override {
        for (auto path : node->paths()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(path);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::ReplaceStatement* node) override {
        for (auto path : node->paths()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(path);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::SetStatement* node) override {
        for (auto item : node->items()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(item);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::DeleteStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::RemoveStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::OtherWise* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Union* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Except* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Intersect* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::UpdateProperties* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->structs());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::SetSingleProperty* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->value());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::RemoveSingleProperty* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::SetSchemaClause* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->initExpr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::SetGraphClause* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->initExpr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::SetTimeZoneClause* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->initExpr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::SetParamClause* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::ResetAll* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::ResetSchema* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::ResetTimeZone* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::ResetGraph* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::ResetParam* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::ExplainActivity* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->procedureBody());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::SessionActivity* node) override {
        for (auto& session : node->sessions()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(session);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::TransactionActivity* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->transaction());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::FullTransaction* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->startTransaction());
        if (node->normalTransaction().has_value()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->normalTransaction().value());
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::NormalTransaction* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->query());
        if (node->endTransaction().has_value()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->endTransaction().value());
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::StartTransaction* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::CommitTransaction* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::RollBackTransaction* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::SessionSet* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->command());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::SessionReset* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->command());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BindingTableInnerQuery* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->body());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::BindingTableInnerExpr* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::StandaloneCallStatement* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->procedureStatement());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::CallQueryStatement* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->procedureStatement());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::CallProcedureStatement* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->procedureCall());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::InlineProcedureCall* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::NamedProcedureCall* node) override {
        for (auto& arg : node->args()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(arg);
        }
        if (node->yield().has_value()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->yield().value());
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::ForStatement* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->expr());
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::MergeStatement* node) override {
        ACCEPT_AND_CHECK_WITH_ERROR_MSG(node->pathPattern());
        for (auto& onMatch : node->onMatch()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(onMatch);
        }
        for (auto& onCreate : node->onCreate()) {
            ACCEPT_AND_CHECK_WITH_ERROR_MSG(onCreate);
        }
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::Param* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::ShowProcessListStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::KillStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::ManagerStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::UnwindStatement* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::InQueryProcedureCall* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::DummyNode* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(geax::frontend::ListComprehension* node) override {
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

 protected:
    std::string error_msg_;
};

}  // namespace cypher
