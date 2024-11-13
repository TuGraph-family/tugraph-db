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

#include "cypher/execution_plan/ops/op_set.h"
#include "cypher/arithmetic/arithmetic_expression.h"
#include "common/logger.h"

namespace cypher {

void OpGqlSet::SetLabels(
    cypher::RTContext* ctx, geax::frontend::SetLabel* update) {
    auto idx = GetRecordIdx(update->v());
    if (!record->values[idx].IsNode()) {
        THROW_CODE(CypherException, "set labels, but not Node");
    }
    record->values[idx].node->vertex_->AddLabels(
        {update->labels().begin(), update->labels().end()});
}

void OpGqlSet::SetVertex(RTContext *ctx, const std::string& variable,
                         geax::frontend::UpdateProperties* update) {
    auto idx = GetRecordIdx(variable);
    if (update->variable()) {
        ArithExprNode ae(update->variable(), *record->symbol_table);
        auto val = ae.Evaluate(ctx, *record);
        if (!val.IsMap()) CYPHER_TODO();
        const auto& props = val.constant.AsMap();
        if (update->mode() == geax::frontend::UpdateProperties::Mode::Assign) {
            // SET n = properties
            record->values[idx].node->vertex_->RemoveAllProperty();
        } else {
            // SET n += properties
        }
        record->values[idx].node->vertex_->SetProperties(props);
        ctx->result_info_->statistics.properties_set += props.size();
    } else {
        auto properties = update->structs()->properties();
        std::unordered_map<std::string, Value> values;
        for (auto property : properties) {
            ArithExprNode ae(std::get<1>(property), *record->symbol_table);
            auto val = ae.Evaluate(ctx, *record);
            if (!val.IsConstant()) CYPHER_TODO();
            values.emplace(std::get<0>(property), std::move(val.constant));
        }
        record->values[idx].node->vertex_->SetProperties(values);
        ctx->result_info_->statistics.properties_set += values.size();
    }
}

void OpGqlSet::SetEdge(RTContext *ctx, const std::string& variable,
                       geax::frontend::UpdateProperties* update) {
    auto idx = GetRecordIdx(variable);
    if (update->variable()) {
        ArithExprNode ae(update->variable(), *record->symbol_table);
        auto val = ae.Evaluate(ctx, *record);
        if (!val.IsMap()) CYPHER_TODO();
        const auto& props = val.constant.AsMap();
        if (update->mode() == geax::frontend::UpdateProperties::Mode::Assign) {
            // SET n = properties
            record->values[idx].relationship->edge_->RemoveAllProperty();
        } else {
            // SET n += properties
        }
        record->values[idx].relationship->edge_->SetProperties(props);
        ctx->result_info_->statistics.properties_set += props.size();
    } else {
        auto properties = update->structs()->properties();
        std::unordered_map<std::string, Value> props;
        for (auto property : properties) {
            ArithExprNode ae(std::get<1>(property), *record->symbol_table);
            auto val = ae.Evaluate(ctx, *record);
            if (!val.IsScalar()) CYPHER_TODO();
            props.emplace(std::get<0>(property), std::move(val.constant));
        }
        record->values[idx].relationship->edge_->SetProperties(props);
        ctx->result_info_->statistics.properties_set += props.size();
    }
}

void OpGqlSet::SetVE(RTContext *ctx) {
    for (auto & item : set_items_) {
        geax::frontend::UpdateProperties* props = nullptr;
        props = dynamic_cast<geax::frontend::UpdateProperties *>(item);
        if (props) {
            auto& variable = props->v();
            auto it = record->symbol_table->symbols.find(variable);
            CYPHER_THROW_ASSERT(it != record->symbol_table->symbols.end());
            if (it->second.type == SymbolNode::NODE) {
                SetVertex(ctx, variable, props);
            } else if (it->second.type == SymbolNode::RELATIONSHIP) {
                SetEdge(ctx, variable, props);
            } else {
                CYPHER_TODO();
            }
        } else {
            geax::frontend::SetLabel* labels = nullptr;
            labels = dynamic_cast<geax::frontend::SetLabel *>(item);
            if (!labels) {
                CYPHER_TODO();
            }
            SetLabels(ctx, labels);
        }
    }
}

void OpGqlSet::ResultSummary(RTContext *ctx) {
    if (summary_) {
        std::string summary;
        summary.append("set ")
            .append(std::to_string(ctx->result_info_->statistics.properties_set))
            .append(" properties.");
        // ctx->result_info_->header.colums.emplace_back("<SUMMARY>");
        /*auto header = ctx->result_->Header();
        header.clear();
        header.emplace_back(std::make_pair("<SUMMARY>", lgraph_api::LGraphType::STRING));
        ctx->result_->ResetHeader(header);*/
        CYPHER_THROW_ASSERT(record);
        record->values.clear();
        record->AddConstant(Value(summary));
    } else {
        /* There should be a "Project" operation on top of this "Set",
         * so leave result set to it. */
    }
}

}
