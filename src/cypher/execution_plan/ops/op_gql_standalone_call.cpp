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

#include "procedure/utils.h"
#include "cypher/execution_plan/ops/op_gql_standalone_call.h"
#include "procedure/procedure.h"
#include "resultset/record.h"
#include "server/json_convert.h"
#include "arithmetic/arithmetic_expression.h"
#include "tools/lgraph_log.h"

cypher::OpBase::OpResult cypher::GqlStandaloneCall::RealConsume(RTContext *ctx) {
    auto names = fma_common::Split(func_name_, ".");
    if (names.size() > 2 && names[0] == "plugin") {
        CYPHER_TODO();
    } else {
        std::vector<Record> records;
        std::vector<std::string> _yield_items;
        std::vector<Entry> parameters;
        parameters.reserve(args_.size());
        for (auto expr : args_) {
            ArithExprNode node(expr, symbol_table_);
            // TODO(lingsu): dummy record
            parameters.emplace_back(node.Evaluate(ctx, Record(1)));
        }
        procedure_->function(ctx, nullptr, parameters, _yield_items, &records);
        auto &header = ctx->result_->Header();
        for (auto &r : records) {
            auto record = ctx->result_->MutableRecord();
            int idx = 0;
            for (auto &v : r.values) {
                auto title = header[idx].first;
                auto type = header[idx].second;
                switch (type) {
                case lgraph_api::LGraphType::NODE:
                    CYPHER_TODO();
                    break;
                case lgraph_api::LGraphType::RELATIONSHIP:
                    CYPHER_TODO();
                    break;
                case lgraph_api::LGraphType::ANY:
                    if (v.type == Entry::RecordEntryType::CONSTANT &&
                        v.constant.type == cypher::FieldData::FieldType::SCALAR) {
                        record->Insert(title, lgraph::FieldData(v.constant.scalar));
                    } else {
                        record->Insert(title, lgraph::FieldData(v.ToString()));
                    }
                    break;
                case lgraph_api::LGraphType::PATH:
                    // TODO(anyone): PATH is undefine
                    record->Insert(title, lgraph::FieldData(v.ToString()));
                    break;
                case lgraph_api::LGraphType::MAP:
                    {
                        auto obj = lgraph_rfc::FieldDataToJson(v.constant.scalar);
                        std::map<std::string, lgraph::FieldData> map;
                        for (auto &o : obj.items()) {
                            map[o.key()] = lgraph_rfc::JsonToFieldData(o.value());
                        }
                        record->Insert(title, map);
                        break;
                    }
                case lgraph_api::LGraphType::LIST:
                    {
                        auto obj = lgraph_rfc::FieldDataToJson(v.constant.scalar);
                        std::vector<lgraph::FieldData> list;
                        for (auto &o : obj) {
                            list.emplace_back(lgraph_rfc::JsonToFieldData(o));
                        }
                        record->Insert(title, list);
                        break;
                    }
                default:
                    if (v.constant.array != nullptr) {
                        record->Insert(title, lgraph::FieldData(v.ToString()));
                    } else {
                        record->Insert(title, v.constant.scalar);
                    }
                }
                idx++;
            }
        }
    }
    return OP_DEPLETED;
}
