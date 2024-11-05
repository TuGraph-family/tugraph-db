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
// Created by wt on 19-7-18.
//

#include "cypher/execution_plan/ops/op_cartesian_product.h"
#include "cypher/parser/symbol_table.h"
namespace cypher {

OpBase::OpResult CartesianProduct::Initialize(RTContext *ctx) {
    CYPHER_THROW_ASSERT(!children.empty());
    const SymbolTable *sym_tab = nullptr;
    for (auto child : children) {
        auto res = child->Initialize(ctx);
        if (res != OP_OK) return res;
        /* take the 1st non-null sym_tab of children
         * TODO(anyone) check more  */
        if (!sym_tab && child->record->symbol_table) {
            sym_tab = child->record->symbol_table;
        }
    }
    if (!sym_tab) THROW_CODE(CypherException, "CartesianProduct initialize failed");
    record = std::make_shared<Record>(sym_tab->symbols.size(), sym_tab);
    return OP_OK;
}

OpBase::OpResult CartesianProduct::RealConsume(RTContext *ctx) {
    if (init) {
        init = false;
        for (auto child : children) {
            auto res = child->Consume(ctx);
            if (res != OP_OK) return res;
            record->Merge(*child->record);
        }
        return OP_OK;
    }
    // Pull from first stream.
    auto child = children[0];
    if (child->Consume(ctx) == OP_OK) {
        // Managed to get data from first stream.
        record->Merge(*child->record);
    } else {
        // Failed to get data from first stream,
        // try pulling other streams for data.
        auto res = PullFromStreams(ctx);
        if (res != OP_OK) return res;
    }
    return OP_OK;
}

OpBase::OpResult CartesianProduct::PullFromStreams(RTContext *ctx) {
    CYPHER_THROW_ASSERT(children.size() > 1);
    for (int i = 1; i < (int)children.size(); i++) {
        auto child = children[i];
        if (child->Consume(ctx) == OP_OK) {
            record->Merge(*child->record);
            /* Managed to get new data.
             * Reset streams [0-i] */
            for (int ii = 0; ii < i; ii++) ResetStream(children[ii]);
            // Pull from resetted streams.
            for (int j = 0; j < i; j++) {
                auto c = children[j];
                if (c->Consume(ctx) == OP_OK) {
                    record->Merge(*c->record);
                } else {
                    return OP_ERR;
                }
            }
            // Ready to continue.
            return OP_OK;
        }
    }
    /* If we're here, then we didn't manged to get new data.
     * Last stream depleted. */
    return OP_DEPLETED;
}


}