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

#include "cypher/resultset/record.h"

#include "execution_plan/runtime_context.h"
#include "parser/symbol_table.h"

namespace cypher {
lgraph::FieldData Entry::GetEntityField(RTContext *ctx, const std::string &fd) const {
    switch (type) {
    case NODE:
        {
            auto vit = node->ItRef();
            CYPHER_THROW_ASSERT(node && vit);
            return node->IsValidAfterMaterialize(ctx) ? vit->GetField(fd) : lgraph::FieldData();
        }
    case RELATIONSHIP:
        {
            auto eit = relationship->ItRef();
            CYPHER_THROW_ASSERT(relationship && eit);
            return eit->IsValid() ? eit->GetField(fd) : lgraph::FieldData();
        }
    case NODE_SNAPSHOT:
        {
            // extract vid from snapshot, "V[{id}]"
            CYPHER_THROW_ASSERT(constant.type == cypher::FieldData::SCALAR &&
                                constant.scalar.type == lgraph::FieldType::STRING);
            auto vid =
                std::stoi(constant.scalar.string().substr(2, constant.scalar.string().size() - 3));
            return ctx->txn_->GetTxn()->GetVertexField(vid, fd);
        }
    case CONSTANT:
        {
            CYPHER_THROW_ASSERT(constant.type == cypher::FieldData::MAP);
            auto it = constant.map->find(fd);
            CYPHER_THROW_ASSERT(it != constant.map->end() &&
                                it->second.type == cypher::FieldData::SCALAR);
            return it->second.scalar;
        }
    case RELP_SNAPSHOT:
    default:
        CYPHER_TODO();
    }
}

void Record::SetParameter(const PARAM_TAB &ptab) {
    if (!symbol_table || ptab.empty()) return;
    for (auto &param : ptab) {
        auto it = symbol_table->symbols.find(param.first);
        if (it != symbol_table->symbols.end()) {
            values[it->second.id] = Entry(param.second);
        } else {
            // LOG_WARN() << "Invalid parameter: " << param.first;
        }
    }
}
}  // namespace cypher
