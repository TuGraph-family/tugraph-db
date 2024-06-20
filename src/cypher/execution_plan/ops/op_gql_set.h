/**
 * Copyright 2022 AntGroup CO., Ltd.
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

#pragma once

#include "geax-front-end/ast/clause/ClauseNodeFwd.h"
#include "geax-front-end/ast/expr/ExprNodeFwd.h"
#include "cypher/arithmetic/arithmetic_expression.h"
#include "cypher/execution_plan/ops/op.h"

// TODO(anyone) get resources then set all
namespace cypher {

class OpGqlSet : public OpBase {
    const std::vector<geax::frontend::SetItem*>& set_items_;
    std::vector<lgraph::VertexId> vertices_to_delete_;
    std::vector<lgraph::EdgeUid> edges_to_delete_;
    bool summary_ = false;

    size_t GetRecordIdx(const std::string &var) {
        auto it = record->symbol_table->symbols.find(var);
        CYPHER_THROW_ASSERT(it != record->symbol_table->symbols.end());
        return it->second.id;
    }

    void SetVertex(RTContext *ctx, const std::string& variable,
                    const geax::frontend::PropStruct* fields) {
        auto properties = fields->properties();
        std::vector<std::string> fields_names;
        std::vector<lgraph_api::FieldData> fields_values;
        for (auto property : properties) {
            fields_names.emplace_back(std::get<0>(property));
            ArithExprNode ae(std::get<1>(property), *record->symbol_table);
            auto val = ae.Evaluate(ctx, *record);
            if (!val.IsScalar()) CYPHER_TODO();
            fields_values.emplace_back(val.constant.scalar);
        }
        auto idx = GetRecordIdx(variable);
        auto vid = record->values[idx].node->PullVid();
        ctx->txn_->GetTxn()->SetVertexProperty(vid, fields_names, fields_values);
        ctx->txn_->GetTxn()->RefreshIterators();
        ctx->result_info_->statistics.properties_set += fields_values.size();
    }

    void SetEdge(RTContext *ctx, const std::string& variable,
                    const geax::frontend::PropStruct* fields) {
        auto properties = fields->properties();
        std::vector<std::string> fields_names;
        std::vector<lgraph_api::FieldData> fields_values;
        for (auto property : properties) {
            fields_names.emplace_back(std::get<0>(property));
            ArithExprNode ae(std::get<1>(property), *record->symbol_table);
            auto val = ae.Evaluate(ctx, *record);
            if (!val.IsScalar()) CYPHER_TODO();
            fields_values.emplace_back(val.constant.scalar);
        }
        auto idx = GetRecordIdx(variable);
        ctx->txn_->GetTxn()->SetEdgeProperty(record->values[idx].relationship->ItRef()->GetUid(),
                                             fields_names, fields_values);
        ctx->txn_->GetTxn()->RefreshIterators();
        ctx->result_info_->statistics.properties_set++;
    }

    void SetVE(RTContext *ctx) {
        for (auto & item : set_items_) {
            if (typeid(*item) != typeid(geax::frontend::UpdateProperties))
                CYPHER_TODO();
            geax::frontend::UpdateProperties* props = nullptr;
            checkedCast(item, props);
            auto& variable = props->v();
            auto it = record->symbol_table->symbols.find(variable);
            CYPHER_THROW_ASSERT(it != record->symbol_table->symbols.end());
            if (it->second.type == SymbolNode::NODE) {
                SetVertex(ctx, variable, props->structs());
            } else if (it->second.type == SymbolNode::RELATIONSHIP) {
                SetEdge(ctx, variable, props->structs());
            } else {
                    CYPHER_TODO();
            }
        }
    }

    void ResultSummary(RTContext *ctx) {
                if (summary_) {
            std::string summary;
            summary.append("set ")
                .append(std::to_string(ctx->result_info_->statistics.properties_set))
                .append(" properties.");
            // ctx->result_info_->header.colums.emplace_back("<SUMMARY>");
            auto header = ctx->result_->Header();
            header.clear();
            header.emplace_back(std::make_pair("<SUMMARY>", lgraph_api::LGraphType::STRING));
            ctx->result_->ResetHeader(header);
            CYPHER_THROW_ASSERT(record);
            record->values.clear();
            record->AddConstant(lgraph::FieldData(summary));
        } else {
            /* There should be a "Project" operation on top of this "Set",
             * so leave result set to it. */
        }
    }

 public:
    OpGqlSet(const std::vector<geax::frontend::SetItem*>& set_items,
            PatternGraph *pattern_graph)
        : OpBase(OpType::GQL_UPDATE, "GqlSet")
        , set_items_(set_items) {
        state = StreamUnInitialized;
    }

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(parent && children.size() < 2);
        summary_ = !parent->parent;
        for (auto child : children) {
            child->Initialize(ctx);
        }
        record = children[0]->record;
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        if (state == StreamDepleted) {
            return OP_DEPLETED;
        }
        if (children.size() > 1) CYPHER_TODO();
        auto child = children[0];
        if (summary_) {
            while (child->Consume(ctx) == OP_OK) SetVE(ctx);
            ResultSummary(ctx);
            state = StreamDepleted;
            return OP_OK;
        } else {
            if (child->Consume(ctx) == OP_OK) {
                SetVE(ctx);
                return OP_OK;
            } else {
                return OP_DEPLETED;
            }
        }
        return OP_OK;
    }

    OpResult ResetImpl(bool complete) override {
        state = StreamUnInitialized;
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [");
        for (auto &m : modifies) str.append(m).append(",");
        if (!modifies.empty()) str.pop_back();
        str.append("]");
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
