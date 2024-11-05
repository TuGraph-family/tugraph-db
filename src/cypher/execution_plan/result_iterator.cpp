/**
* Copyright 2024 AntGroup CO., Ltd.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*/


#include "result_iterator.h"
#include <antlr4-runtime/antlr4-runtime.h>
#include "cypher/parser/generated/LcypherLexer.h"
#include "cypher/parser/generated/LcypherParser.h"
#include "cypher/parser/cypher_base_visitor_v2.h"
#include "cypher/parser/cypher_error_listener.h"
#include "cypher/rewriter/StandaloneCallYieldRewriter.h"
#include "cypher/rewriter/GenAnonymousAliasRewriter.h"
#include "cypher/rewriter/MultiPathPatternRewriter.h"
#include "common/logger.h"

ResultIterator::ResultIterator(void* ctx, txn::Transaction *txn, std::string cypher)
    : Iterator(txn), ctx_((cypher::RTContext*)ctx), cypher_(std::move(cypher)) {
    antlr4::ANTLRInputStream input(cypher_);
    parser::LcypherLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    parser::LcypherParser parser(&tokens);
    parser.addErrorListener(&parser::CypherErrorListener::INSTANCE);
    parser::CypherBaseVisitorV2 visitor(objAlloc_, parser.oC_Cypher(), ctx_);
    geax::frontend::AstNode* node = visitor.result();
    cypher::StandaloneCallYieldRewriter standalone_call_yield_rewriter(objAlloc_);
    node->accept(standalone_call_yield_rewriter);
    cypher::GenAnonymousAliasRewriter gen_anonymous_alias_rewriter;
    node->accept(gen_anonymous_alias_rewriter);
    cypher::MultiPathPatternRewriter multi_path_pattern_rewriter(objAlloc_);
    node->accept(multi_path_pattern_rewriter);
    ctx_->txn_ = txn;
    auto ret = execution_plan_v2_.Build(node, ctx_);
    if (ret != geax::frontend::GEAXErrorCode::GEAX_SUCCEED) {
        LOG_ERROR("Failed to build execution plan, ret:{}, msg:{}",
                  geax::frontend::ToString(ret), execution_plan_v2_.ErrorMsg());
        THROW_CODE(CypherException, "Failed to build execution plan");
    }
    if (visitor.CommandType() != parser::CmdType::QUERY) {
        header_.emplace_back("@plan");
        common::Result res;
        res.type = common::ResultType::Value;
        auto desc = execution_plan_v2_.DumpPlan(0, false);
        res.data = Value::String(desc);
        record_.emplace_back(std::move(res));
        valid_ = true;
    } else {
        for (const auto& col : ctx_->result_info_->header.colums) {
            header_.push_back(col.alias.empty() ? col.name : col.alias);
        }
        root_ = execution_plan_v2_.Root();
        res_ = root_->Consume(ctx_);
        if (res_ == cypher::OpBase::OpResult::OP_OK) {
            valid_ = true;
            ReFillRecord();
        }
    }
}

void ResultIterator::ReFillRecord() {
    record_.clear();
    assert(header_.size() == root_->record->values.size());
    for (auto& entry : root_->record->values) {
        common::Result res;
        if (entry.IsNode()) {
            if (entry.node->vertex_) {
                common::Node n;
                n.id = entry.node->vertex_->GetNativeId();
                n.labels = entry.node->vertex_->GetLabels();
                n.properties = entry.node->vertex_->GetAllProperty();
                res.type = common::ResultType::Node;
                res.data = std::move(n);
            } else {
                res.type = common::ResultType::Value;
                res.data = Value();
            }
        } else if (entry.IsRelationship()) {
            if (entry.relationship->edge_) {
                common::Relationship r;
                r.id = entry.relationship->edge_->GetNativeId();
                r.src = entry.relationship->edge_->GetNativeStartId();
                r.dst = entry.relationship->edge_->GetNativeEndId();
                r.type = entry.relationship->edge_->GetType();
                r.properties = entry.relationship->edge_->GetAllProperty();
                res.type = common::ResultType::Relationship;
                res.data = std::move(r);
            } else {
                res.type = common::ResultType::Value;
                res.data = Value();
            }
        } else if (entry.IsConstant()) {
            res.type = common::ResultType::Value;
            res.data = entry.constant;
        } else if (entry.IsPath()) {
            if (entry.path.empty()) {
                res.type = common::ResultType::Value;
                res.data = Value();
            } else {
                res.type = common::ResultType::Path;
                common::Path path;
                for (auto& item : entry.path) {
                    if (item.is_node) {
                        common::Node n;
                        auto& v = std::any_cast<graphdb::Vertex&>(item.element);
                        n.id = v.GetNativeId();
                        n.labels = v.GetLabels();
                        n.properties = v.GetAllProperty();
                        path.data.emplace_back(common::PathElement{true, std::move(n)});
                    } else {
                        common::Relationship r;
                        auto& e = std::any_cast<graphdb::Edge&>(item.element);
                        r.id = e.GetNativeId();
                        r.src = e.GetNativeStartId();
                        r.dst = e.GetNativeEndId();
                        r.type = e.GetType();
                        r.properties = e.GetAllProperty();
                        path.data.emplace_back(common::PathElement{false, std::move(r)});
                    }
                }
                res.data = std::move(path);
            }
        } else {
            LOG_ERROR("entry.type: {}", (int)entry.type);
            CYPHER_TODO();
        }
        record_.emplace_back(std::move(res));
    }
}

void ResultIterator::Next() {
    assert(valid_);
    valid_ = false;
    if (root_ && res_ == cypher::OpBase::OpResult::OP_OK) {
        res_ = root_->Consume(ctx_);
        if (res_ == cypher::OpBase::OpResult::OP_OK) {
            valid_ = true;
            ReFillRecord();
        }
    }
}