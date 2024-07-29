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
#include <boost/algorithm/string.hpp>
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
            if (constant.type != cypher::FieldData::MAP) {
                THROW_CODE(CypherException, "Only support for map type");
            }
            auto it = constant.map->find(fd);
            if (it == constant.map->end() ||
                it->second.type != cypher::FieldData::SCALAR) {
                THROW_CODE(CypherException, "Not found or type mismatch");
            }
            return it->second.scalar;
        }
    case RELP_SNAPSHOT:
    default:
        CYPHER_TODO();
    }
}

bool Entry::CheckEntityEfficient(RTContext *ctx) const {
    switch (type) {
    case NODE:
        {
            auto vit = node->ItRef();
            CYPHER_THROW_ASSERT(node && vit);
            return node->IsValidAfterMaterialize(ctx);
        }
    case RELATIONSHIP:
        {
            if (relationship->VarLen()) {
                auto &eits = relationship->ItsRef();
                for (auto &it : eits) {
                    if (!it.IsValid()) return false;
                }
                return true;
            } else {
                auto eit = relationship->ItRef();
                CYPHER_THROW_ASSERT(relationship && eit);
                return eit->IsValid();
            }
        }
    case NODE_SNAPSHOT:
        {
            CYPHER_THROW_ASSERT(constant.type == cypher::FieldData::SCALAR &&
                                constant.scalar.type == lgraph::FieldType::STRING);
            auto vid =
                std::stoi(constant.scalar.string().substr(2, constant.scalar.string().size() - 3));
            auto vit = ctx->txn_->GetVertexIterator(vid, true);
            return vit.IsValid();
        }
    case RELP_SNAPSHOT:
        {
            CYPHER_THROW_ASSERT(constant.type == cypher::FieldData::SCALAR &&
                                constant.scalar.type == lgraph::FieldType::STRING);
            auto str = constant.scalar.string().substr(1, constant.scalar.string().size() - 2);
            std::vector<std::string> euid;
            boost::split(euid, str, boost::is_any_of("_"));
            int64_t src = std::stoi(euid[0]);
            int64_t dst = std::stoi(euid[1]);
            uint16_t lid = std::stoi(euid[2]);
            int64_t tid = std::stoi(euid[3]);
            int64_t eid = std::stoi(euid[4]);
            auto eit = ctx->txn_->GetOutEdgeIterator({src, dst, lid, tid, eid}, true);
            return eit.IsValid();
        }
    case VAR_LEN_RELP:
        {
            auto paths = relationship->path_;
            auto len = paths.Length();
            for (size_t idx = 0; idx < len; ++idx) {
                auto euid = paths.GetNthEdge(idx);
                int64_t vid;
                if (paths.dirs_[idx]) {
                    vid = euid.dst;
                } else {
                    vid = euid.src;
                }
                auto vit = ctx->txn_->GetVertexIterator(vid, true);
                if (!vit.IsValid()) return false;
                auto eit = ctx->txn_->GetOutEdgeIterator(euid, true);
                if (!eit.IsValid()) return false;
                if (idx == len - 1) {
                    int64_t last_vid;
                    if (paths.dirs_[idx]) {
                        last_vid = euid.dst;
                    } else {
                        last_vid = euid.src;
                    }
                    auto vit = ctx->txn_->GetVertexIterator(last_vid, true);
                    if (!vit.IsValid()) return false;
                }
            }
            return true;
        }
    default:
        return false;
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
