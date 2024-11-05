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

#include "cypher/execution_plan/ops/op_delete.h"
#include "cypher/parser/symbol_table.h"
#include "cypher/execution_plan/runtime_context.h"
namespace cypher {

void OpGqlDelete::DoDeleteVE(RTContext *ctx) {
    for (auto &item : items_) {
        auto it = record->symbol_table->symbols.find(item);
        CYPHER_THROW_ASSERT(it != record->symbol_table->symbols.end());
        if (it->second.type == SymbolNode::NODE) {
            auto node = record->values[it->second.id].node;
            int deleted_edge = node->vertex_->Delete();
            ctx->result_info_->statistics.vertices_deleted++;
            ctx->result_info_->statistics.edges_deleted += deleted_edge;
        } else if (it->second.type == SymbolNode::RELATIONSHIP) {
            auto relp = record->values[it->second.id].relationship;
            relp->edge_->Delete();
            ctx->result_info_->statistics.edges_deleted++;
        } else {
            CYPHER_TODO();
        }
    }
}

void OpGqlDelete::ResultSummary(RTContext *ctx) {
    if (summary_) {
        std::string summary;
        summary.append("deleted ")
            .append(std::to_string(ctx->result_info_->statistics.vertices_deleted))
            .append(" vertices, deleted ")
            .append(std::to_string(ctx->result_info_->statistics.edges_deleted))
            .append(" edges.");
        //auto header = ctx->result_->Header();
        //header.clear();
        //header.emplace_back(std::make_pair("<SUMMARY>", lgraph_api::LGraphType::STRING));
        //ctx->result_->ResetHeader(header);
        CYPHER_THROW_ASSERT(record);
        record->values.clear();
        record->AddConstant(Value(summary));
    } else {
        /* There should be a "Project" operation on top of this "Delete",
         * so leave result set to it. */
    }
}

}
