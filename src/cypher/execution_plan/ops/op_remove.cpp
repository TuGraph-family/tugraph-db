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

//
// Created by lpp on 24-6-19.
//

#include "cypher/execution_plan/ops/op_remove.h"
#include "geax-front-end/ast/clause/RemoveSingleProperty.h"
#include "cypher/utils/geax_util.h"
#include "cypher/execution_plan/runtime_context.h"

namespace cypher {

void OpGqlRemove::Remove(cypher::RTContext *ctx) {
    for (auto &item : remove_items_) {
        if (typeid(*item) != typeid(geax::frontend::RemoveSingleProperty))
            CYPHER_TODO();
        geax::frontend::RemoveSingleProperty* props = nullptr;
        checkedCast(item, props);
        auto &var = props->v();
        auto &key = props->property();
        auto it = record->symbol_table->symbols.find(var);
        if (it == record->symbol_table->symbols.end()) CYPHER_TODO();
        auto &entry = record->values[it->second.id];
        if (entry.type != Entry::NODE) CYPHER_TODO();
        entry.node->vertex_->RemoveProperty(key);

        ctx->result_info_->statistics.properties_remove++;
    }
}

OpBase::OpResult OpGqlRemove::RealConsume(RTContext *ctx) {
    if (state == StreamDepleted) {
        return OP_DEPLETED;
    }
    if (children.size() > 1) CYPHER_TODO();
    auto child = children[0];
    if (summary_) {
        while (child->Consume(ctx) == OP_OK) Remove(ctx);
        ResultSummary(ctx);
        state = StreamDepleted;
        return OP_OK;
    } else {
        if (child->Consume(ctx) == OP_OK) {
            Remove(ctx);
            return OP_OK;
        } else {
            return OP_DEPLETED;
        }
    }
}

void OpGqlRemove::ResultSummary(RTContext *ctx) {
    if (summary_) {
        std::string summary;
        summary.append("remove ")
            .append(std::to_string(ctx->result_info_->statistics.properties_remove))
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
        /* There should be a "Project" operation on top of this "Remove",
         * so leave result set to it. */
    }
}

}
