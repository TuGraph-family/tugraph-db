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
// Created by wt on 6/14/18.
//
#pragma once

#include <regex>
#include "cypher/execution_plan/ops/op.h"
#include "lgraph/lgraph_types.h"
#include "resultset/record.h"
#include "server/json_convert.h"
#include "server/bolt_session.h"

/* Runtime Record to User Record */
static void RRecordToURecord(
    lgraph_api::Transaction *txn,
    const std::vector<std::pair<std::string, lgraph_api::LGraphType>> &header,
    const std::shared_ptr<cypher::Record> &record_ptr, lgraph_api::Record &record) {
    if (header.empty()) {
        return;
    }

    for (size_t index = 0; index < header.size(); index++) {
        auto &v = record_ptr->values[index];
        const auto &entry_type = v.type;
        const auto &header_type = header[index].second;

        if (entry_type == cypher::Entry::NODE_SNAPSHOT) {
            if (header_type == lgraph_api::LGraphType::NODE ||
                header_type == lgraph_api::LGraphType::ANY) {
                std::regex regex_word("(V)\\[([0-9]+)\\]");
                std::smatch match_group;
                auto node_str = v.ToString();
                CYPHER_THROW_ASSERT(std::regex_match(node_str, match_group, regex_word));
                auto vid = static_cast<size_t>(std::stoll(match_group[2].str()));
                record.InsertVertexByID(header[index].first, vid);
                continue;
            } else {
                throw lgraph::CypherException(
                    "unhandled record entry type: " + cypher::Entry::ToString(v.type) +
                    ", header type: " + std::to_string(uint16_t(header_type)));
            }
        }

        if (entry_type == cypher::Entry::NODE) {
            if (header_type == lgraph_api::LGraphType::NODE ||
                header_type == lgraph_api::LGraphType::ANY) {
                auto vid = v.node->PullVid();
                if (vid >= 0) {
                    record.Insert(header[index].first, vid, txn);
                } else {
                    // OPTIONAL MATCH return null
                    record.InsertVertexByID(header[index].first, vid);
                }
                continue;
            } else {
                throw lgraph::CypherException(
                    "unhandled record entry type: " + cypher::Entry::ToString(v.type) +
                    ", header type: " + std::to_string(uint16_t(header_type)));
            }
        }

        if (entry_type == cypher::Entry::RELP_SNAPSHOT) {
            if (header_type == lgraph_api::LGraphType::RELATIONSHIP ||
                header_type == lgraph_api::LGraphType::ANY) {
                std::regex regex_word("(E)\\[([0-9]+_[0-9]+_[0-9]+_[0-9]+_[0-9]+)\\]");
                std::smatch match_group;
                auto repl_str = v.ToString();
                CYPHER_THROW_ASSERT(std::regex_match(repl_str, match_group, regex_word));
                auto ids = match_group[2].str();
                lgraph_api::EdgeUid edge_uid;
                std::regex split_word("_");
                std::sregex_token_iterator id(ids.begin(), ids.end(), split_word, -1);
                // WARNING: This depend on EUID order.
                auto start = static_cast<size_t>(std::stoll(id++->str()));
                auto end = static_cast<size_t>(std::stoll(id++->str()));
                auto lid = static_cast<uint16_t>(std::stoll(id++->str()));
                auto tid = static_cast<int64_t>(std::stoll(id++->str()));
                auto eid = static_cast<uint16_t>(std::stoll(id++->str()));
                record.InsertEdgeByID(header[index].first,
                                      lgraph_api::EdgeUid(start, end, lid, tid, eid));
                continue;
            } else {
                throw lgraph::CypherException(
                    "unhandled record entry type: " + cypher::Entry::ToString(v.type) +
                    ", header type: " + std::to_string(uint16_t(header_type)));
            }
        }

        if (entry_type == cypher::Entry::RELATIONSHIP) {
            if (header_type == lgraph_api::LGraphType::RELATIONSHIP ||
                header_type == lgraph_api::LGraphType::ANY) {
                auto uit = v.relationship->ItRef();
                auto uid = uit->GetUid();
                record.Insert(header[index].first, uid, txn);
                continue;
            } else {
                throw lgraph::CypherException(
                    "unhandled record entry type: " + cypher::Entry::ToString(v.type) +
                    ", header type: " + std::to_string(uint16_t(header_type)));
            }
        }

        if (entry_type == cypher::Entry::CONSTANT) {
            if (header_type == lgraph_api::LGraphType::PATH) {
                using Vertex = lgraph_api::traversal::Vertex;
                using Path = lgraph_api::traversal::Path;
                using Edge = lgraph_api::traversal::Edge;
                std::regex regex_word("(E|V)\\[([0-9]+|([0-9]+_[0-9]+_[0-9]+_[0-9]+_[0-9]+))\\]");
                std::smatch match_group;
                auto node_str = v.constant.array->at(0).ToString();
                CYPHER_THROW_ASSERT(std::regex_match(node_str, match_group, regex_word));
                auto start = static_cast<size_t>(std::stoll(match_group[2].str()));
                Path path{Vertex(start)};
                std::regex split_word("_");
                for (auto &path_pattern : *v.constant.array) {
                    auto path_pattern_str = path_pattern.ToString();
                    CYPHER_THROW_ASSERT(
                        std::regex_match(path_pattern_str, match_group, regex_word));
                    auto type = match_group[1].str();
                    if (type == "V") continue;
                    auto ids = match_group[2].str();
                    std::sregex_token_iterator id(ids.begin(), ids.end(), split_word, -1);
                    auto start = static_cast<size_t>(std::stoll(id++->str()));
                    auto end = static_cast<size_t>(std::stoll(id++->str()));
                    auto lid = static_cast<uint16_t>(std::stoll(id++->str()));
                    auto tid = static_cast<int64_t>(std::stoll(id++->str()));
                    auto eid = static_cast<uint16_t>(std::stoll(id++->str()));
                    bool forward = true;
                    if (path.GetEndVertex().GetId() != start) {
                        auto tmp_id = start;
                        forward = false;
                        start = end;
                        end = tmp_id;
                    }
                    path.Append(Edge(start, lid, tid, end, eid, forward));
                }
                record.Insert(header[index].first, path, txn);
                continue;
            } else {
                if (v.constant.array != nullptr) {
                    record.Insert(header[index].first, lgraph_api::FieldData(v.ToString()));
                } else {
                    record.Insert(header[index].first, v.constant.scalar);
                }
                continue;
            }
        }

        if (entry_type == cypher::Entry::UNKNOWN) {
            if (v.constant.array != nullptr) {
                record.Insert(header[index].first, lgraph_api::FieldData(v.ToString()));
            } else {
                record.Insert(header[index].first, v.constant.scalar);
            }
            continue;
        }

        throw lgraph::CypherException(
            "unhandled record entry type: " + cypher::Entry::ToString(v.type) +
            ", header type: " + std::to_string(uint16_t(header_type)));
    }
}

namespace cypher {

class ProduceResults : public OpBase {
    enum {
        Uninitialized,
        RefreshAfterPass,
        Resetted,
        Consuming,
    } state_;

 public:
    ProduceResults() : OpBase(OpType::PRODUCE_RESULTS, "Produce Results") {
        state_ = Uninitialized;
    }

    OpResult Initialize(RTContext *ctx) override {
        if (!children.empty()) {
            children[0]->Initialize(ctx);
        }
        return OP_OK;
    }

    /* ProduceResults next operation
     * called each time a new result record is required */
    OpResult RealConsume(RTContext *ctx) override {
        if (state_ == Uninitialized) {
            Initialize(ctx);
            state_ = Consuming;
        }
        if (children.empty()) return OP_DEPLETED;
        if (ctx->bolt_conn_) {
            if (ctx->bolt_conn_->has_closed()) {
                LOG_INFO() << "The bolt connection is closed, cancel the op execution.";
                return OP_ERR;
            }
            auto session = (bolt::BoltSession *)ctx->bolt_conn_->GetContext();
            while (!session->current_msg) {
                session->current_msg = session->msgs.Pop(std::chrono::milliseconds(100));
                if (ctx->bolt_conn_->has_closed()) {
                    LOG_INFO() << "The bolt connection is closed, cancel the op execution.";
                    return OP_ERR;
                }
            }
            auto child = children[0];
            auto res = child->Consume(ctx);
            if (res != OP_OK) {
                session->ps.AppendSuccess();
                ctx->bolt_conn_->PostResponse(std::move(session->ps.MutableBuffer()));
                session->ps.Reset();
                return res;
            }
            if (session->current_msg.value().type == bolt::BoltMsg::PullN) {
                auto record = ctx->result_->MutableRecord();
                RRecordToURecord(ctx->txn_.get(), ctx->result_->Header(), child->record, *record);
                session->ps.AppendRecords(ctx->result_->BoltRecords());
                ctx->result_->ClearRecords();
                bool sync = false;
                if (--session->current_msg.value().n == 0) {
                    std::unordered_map<std::string, std::any> meta;
                    meta["has_more"] = true;
                    session->ps.AppendSuccess(meta);
                    session->current_msg.reset();
                    sync = true;
                }
                if (sync || session->ps.ConstBuffer().size() > 1024) {
                    ctx->bolt_conn_->PostResponse(std::move(session->ps.MutableBuffer()));
                    session->ps.Reset();
                }
            } else if (session->current_msg.value().type == bolt::BoltMsg::DiscardN) {
                if (--session->current_msg.value().n == 0) {
                    std::unordered_map<std::string, std::any> meta;
                    meta["has_more"] = true;
                    session->ps.AppendSuccess(meta);
                    session->current_msg.reset();
                    ctx->bolt_conn_->PostResponse(std::move(session->ps.MutableBuffer()));
                    session->ps.Reset();
                }
            }
            return OP_OK;
        } else {
            auto child = children[0];
            auto res = child->Consume(ctx);
            if (res != OP_OK) return res;
            auto record = ctx->result_->MutableRecord();
            RRecordToURecord(ctx->txn_.get(), ctx->result_->Header(), child->record, *record);
            return OP_OK;
        }
    }

    /* Restart */
    OpResult ResetImpl(bool complete) override {
        if (complete) state_ = Uninitialized;
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
