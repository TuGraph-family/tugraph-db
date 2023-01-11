﻿/**
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
// Created by wt on 18-7-31.
//
#include "procedure/utils.h"
#include "op_standalone_call.h"
#include "procedure/procedure.h"
#include "server/json_convert.h"

cypher::OpBase::OpResult cypher::StandaloneCall::RealConsume(RTContext *ctx) {
    auto &call = call_clause_;
    auto &procedure_name = std::get<0>(call);
    auto &parameters = std::get<1>(call);
    auto &yield_items = std::get<2>(call);
    auto names = fma_common::Split(procedure_name, ".");
    if (names.size() > 2 && names[0] == "plugin") {
        std::string input, output;
        auto type = names[1] == "cpp" ? lgraph::PluginManager::PluginType::CPP
                                      : lgraph::PluginManager::PluginType::PYTHON;
        if (type == lgraph::PluginManager::PluginType::PYTHON) {
            throw lgraph::EvaluationException(
                "Calling python plugin in CYPHER is disabled in this release.");
        }
        auto token = "A_DUMMY_TOKEN_FOR_CPP_PLUGIN";
        auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
        if (names[2] == "list") {
            auto plugins = ac_db.ListPlugins(type, token);
            // todo: return records other than just strings
            auto &header = ctx->result_->Header();
            if (header.size() > 1)
                throw lgraph::InputError(FMA_FMT("Plugin [{}] header is excaption.", names[2]));
            auto title = header[0].first;
            for (auto &p : plugins) {
                std::string s;
                s.append(p.name).append(" | ").append(p.desc).append(" | ").append(
                    p.read_only ? "READ" : "WRITE");

                auto &r = ctx->result_->NewRecord();
                r.Insert(title, lgraph::FieldData(s));
            }
        } else {
            if (ctx->txn_) ctx->txn_->Abort();
            bool exists = Utils::CallPlugin(*ctx, type, names[2], parameters, output);
            if (!exists) throw lgraph::InputError("Plugin [{}] does not exist.", names[2]);
            ctx->result_->Load(output);
#if 0
            /* todo: redundant parse */
            try {
                std::vector<std::string> headers;
                std::vector<Record> records;
                size_t i = 0, size = 0;
                auto res = nlohmann::json::parse(output);
                CYPHER_THROW_ASSERT(res.is_array());
                for (auto it = res.begin(); it != res.end(); it++) {
                    CYPHER_THROW_ASSERT(it.value().is_object());
                    Record r;
                    bool fill_header = headers.empty();
                    for (auto it2 = it.value().begin(); it2 != it.value().end(); it2++) {
                        if (fill_header) headers.emplace_back(it2.key());
                        std::stringstream ss;
                        ss << it2.value();
                        r.AddConstant(lgraph::FieldData(ss.str()));
                    }
                    records.emplace_back(r);
                }
                ctx->result_info_->header.colums.clear();
                for (auto &h : headers) {
                    ctx->result_info_->header.colums.emplace_back(h);
                }
                for (auto &r : records) ctx->result_info_->AddRecord(r);
            } catch (std::exception &e) {
                throw lgraph::EvaluationException(std::string("parse json error: ") + e.what());
            }
#endif
        }
    } else {
        SetFunc(procedure_name);
        std::vector<Record> records;
        procedure_->function(ctx, nullptr, parameters, yield_items, &records);
        auto &header = ctx->result_->Header();
        for (auto &r : records) {
            auto &record = ctx->result_->NewRecord();
            int idx = 0;
            for (auto &v : r.values) {
                auto title = header[idx].first;
                auto type = header[idx].second;
                switch (type) {
                case lgraph::ResultElementType::NODE:
                    CYPHER_TODO();
                    break;
                case lgraph::ResultElementType::RELATIONSHIP:
                    CYPHER_TODO();
                    break;
                case lgraph::ResultElementType::ANY:
                case lgraph::ResultElementType::PATH:
                    // TODO PATH is undefine
                    record.Insert(title, lgraph::FieldData(v.ToString()));
                    break;
                case lgraph::ResultElementType::MAP:
                    {
                        auto obj = lgraph_rfc::FieldDataToJson(v.constant.scalar);
                        std::map<std::string, lgraph::FieldData> map;
                        for (auto &o : obj.items()) {
                            map[o.key()] = lgraph::FieldData(o.value().dump());
                        }
                        record.Insert(title, map);
                        break;
                    }
                case lgraph::ResultElementType::LIST:
                    {
                        auto obj = lgraph_rfc::FieldDataToJson(v.constant.scalar);
                        std::vector<lgraph::FieldData> list;
                        for (auto &o : obj) {
                            list.emplace_back(lgraph::FieldData(o.dump()));
                        }
                        record.Insert(title, list);
                        break;
                    }
                default:
                    if (v.constant.array != nullptr) {
                        record.Insert(title, lgraph::FieldData(v.ToString()));
                    }
                    else {
                        record.Insert(title, v.constant.scalar);
                    }
                }
                idx++;
            }
        }
    }
    return OP_DEPLETED;
}
