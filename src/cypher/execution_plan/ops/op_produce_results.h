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
#include "lgraph/lgraph_result.h"
#include "lgraph/lgraph_types.h"
#include "lgraph_api/result_element.h"
#include "resultset/record.h"
#include "server/json_convert.h"
#include "server/bolt_session.h"
#include "boost/regex.hpp"

/* Runtime Record to User Record */
static void RRecordToURecord(
    lgraph_api::Transaction *txn,
    const std::vector<std::pair<std::string, lgraph_api::LGraphType>> &header,
    const std::shared_ptr<cypher::Record> &record_ptr, lgraph_api::Record &record,
    lgraph_api::NODEMAP& node_map, lgraph_api::RELPMAP& relp_map
    ) {
    if (header.empty()) {
        return;
    }

    for (size_t index = 0; index < header.size(); index++) {
        auto &v = record_ptr->values[index];
        const auto &entry_type = v.type;
        const auto &header_type = header[index].second;
        auto InsertVertex = [](lgraph_api::Record &record, const lgraph::VertexId &vid,
                               const std::string &name, lgraph_api::Transaction *txn) {
            if (vid >= 0) {
                record.Insert(name, vid, txn);
            } else {
                record.InsertVertexByID(name, vid);
            }
        };
        if (entry_type == cypher::Entry::NODE_SNAPSHOT) {
            if (header_type == lgraph_api::LGraphType::NODE ||
                header_type == lgraph_api::LGraphType::ANY) {
                std::regex regex_word("(V)\\[([0-9]+)\\]");
                std::smatch match_group;
                auto node_str = v.ToString();
                CYPHER_THROW_ASSERT(std::regex_match(node_str, match_group, regex_word));
                auto vid = static_cast<size_t>(std::stoll(match_group[2].str()));
                InsertVertex(record, vid, header[index].first, txn);
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
                InsertVertex(record, vid, header[index].first, txn);
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
                lgraph::EdgeUid uid = lgraph::EdgeUid(start, end, lid, tid, eid);
                record.Insert(header[index].first, uid, txn);
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
                if (uit->IsValid()) {
                    auto uid = uit->GetUid();
                    record.Insert(header[index].first, uid, txn);
                } else {
                    lgraph::EdgeUid euid;
                    euid.eid = -1;
                    record.InsertEdgeByID(header[index].first, euid);
                }
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
                boost::regex regex_word("(E|V)\\[([0-9]+|([0-9]+_[0-9]+_[0-9]+_[0-9]+_[0-9]+))\\]");
                boost::smatch match_group;
                auto node_str = v.constant.array->at(0).ToString();
                CYPHER_THROW_ASSERT(boost::regex_match(node_str, match_group, regex_word));
                auto start = static_cast<size_t>(std::stoll(match_group[2].str()));
                Path path{Vertex(start)};
                boost::regex split_word("_");
                for (auto &path_pattern : *v.constant.array) {
                    auto path_pattern_str = path_pattern.ToString();
                    CYPHER_THROW_ASSERT(
                        boost::regex_match(path_pattern_str, match_group, regex_word));
                    auto type = match_group[1].str();
                    if (type == "V") continue;
                    auto ids = match_group[2].str();
                    boost::sregex_token_iterator id(ids.begin(), ids.end(), split_word, -1);
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
                record.Insert(header[index].first, path, txn, node_map, relp_map);
                continue;
            } else {
                if (v.constant.array != nullptr || v.constant.map != nullptr) {
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
    lgraph_api::NODEMAP node_map_;
    lgraph_api::RELPMAP relp_map_;

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
            while (session->state == bolt::SessionState::STREAMING && !session->streaming_msg) {
                session->streaming_msg = session->msgs.Pop(std::chrono::milliseconds(100));
                if (ctx->bolt_conn_->has_closed()) {
                    LOG_INFO() << "The bolt connection is closed, cancel the op execution.";
                    return OP_ERR;
                }
                if (!session->streaming_msg) {
                    continue;
                }
                if (session->streaming_msg.value().type == bolt::BoltMsg::PullN ||
                    session->streaming_msg.value().type == bolt::BoltMsg::DiscardN) {
                    const auto &fields = session->streaming_msg.value().fields;
                    if (fields.size() != 1) {
                        std::string err =
                            FMA_FMT("{} msg fields size error, size: {}",
                                    bolt::ToString(session->streaming_msg.value().type).c_str(),
                                    fields.size());
                        LOG_ERROR() << err;
                        bolt::PackStream ps;
                        ps.AppendFailure({{"code", "error"}, {"message", err}});
                        ctx->bolt_conn_->PostResponse(std::move(ps.MutableBuffer()));
                        session->state = bolt::SessionState::FAILED;
                        return OP_ERR;
                    }
                    auto &val =
                        std::any_cast<const std::unordered_map<std::string, std::any> &>(fields[0]);
                    auto n = std::any_cast<int64_t>(val.at("n"));
                    session->streaming_msg.value().n = n;
                } else if (session->streaming_msg.value().type == bolt::BoltMsg::Reset) {
                    LOG_INFO() << "Receive RESET, cancel the op execution.";
                    bolt::PackStream ps;
                    ps.AppendSuccess();
                    ctx->bolt_conn_->PostResponse(std::move(ps.MutableBuffer()));
                    session->state = bolt::SessionState::READY;
                    return OP_ERR;
                } else {
                    LOG_ERROR() << FMA_FMT(
                        "Unexpected msg:{} in STREAMING state, cancel the op execution, "
                        "close the connection.",
                        bolt::ToString(session->streaming_msg.value().type));
                    ctx->bolt_conn_->Close();
                    return OP_ERR;
                }
                break;
            }
            if (session->state == bolt::SessionState::INTERRUPTED) {
                LOG_WARN() << "The session state is INTERRUPTED, cancel the op execution.";
                return OP_ERR;
            } else if (session->state != bolt::SessionState::STREAMING) {
                LOG_ERROR() << "Unexpected state: {} in op execution, close the connection.";
                ctx->bolt_conn_->Close();
                return OP_ERR;
            } else if (session->streaming_msg.value().type != bolt::BoltMsg::PullN &&
                       session->streaming_msg.value().type != bolt::BoltMsg::DiscardN) {
                LOG_ERROR() << FMA_FMT("Unexpected msg: {} in op execution, "
                    "cancel the op execution, close the connection.",
                    bolt::ToString(session->streaming_msg.value().type));
                ctx->bolt_conn_->Close();
                return OP_ERR;
            }
            auto child = children[0];
            auto res = child->Consume(ctx);
            if (res != OP_OK) {
                if (ctx->result_->Size() > 0 &&
                    session->streaming_msg.value().type == bolt::BoltMsg::PullN) {
                    session->ps.AppendRecords(ctx->result_->BoltRecords());
                }
                session->ps.AppendSuccess();
                session->state = bolt::SessionState::READY;
                ctx->bolt_conn_->PostResponse(std::move(session->ps.MutableBuffer()));
                session->ps.Reset();
                return res;
            }
            if (session->streaming_msg.value().type == bolt::BoltMsg::PullN) {
                auto record = ctx->result_->MutableRecord();
                RRecordToURecord(ctx->txn_.get(), ctx->result_->Header(), child->record,
                                *record, node_map_, relp_map_);
                session->ps.AppendRecords(ctx->result_->BoltRecords());
                ctx->result_->ClearRecords();
                bool sync = false;
                if (--session->streaming_msg.value().n == 0) {
                    std::unordered_map<std::string, std::any> meta;
                    meta["has_more"] = true;
                    session->ps.AppendSuccess(meta);
                    session->state = bolt::SessionState::STREAMING;
                    session->streaming_msg.reset();
                    sync = true;
                }
                if (sync || session->ps.ConstBuffer().size() > 1024) {
                    ctx->bolt_conn_->PostResponse(std::move(session->ps.MutableBuffer()));
                    session->ps.Reset();
                }
            } else if (session->streaming_msg.value().type == bolt::BoltMsg::DiscardN) {
                if (--session->streaming_msg.value().n == 0) {
                    std::unordered_map<std::string, std::any> meta;
                    meta["has_more"] = true;
                    session->ps.AppendSuccess(meta);
                    session->state = bolt::SessionState::STREAMING;
                    session->streaming_msg.reset();
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
            RRecordToURecord(ctx->txn_.get(), ctx->result_->Header(),
                            child->record, *record, node_map_, relp_map_);
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
