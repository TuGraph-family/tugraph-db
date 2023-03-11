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

#include "op.h"
#include "server/json_convert.h"
#include <regex>

/* Runtime Record to User Record */
static void RRecordToURecord(
    lgraph::Transaction* txn, const std::vector<std::pair<std::string, lgraph_api::LGraphType>> &header,
    const std::shared_ptr<cypher::Record> &record_ptr, lgraph_api::Record &record) {
    using unordered_json = nlohmann::ordered_json;
    if (header.empty()) {
        return;
    }
    int index = 0;
    for (auto &v : record_ptr->values) {
        switch (header[index].second) {
        case lgraph_api::LGraphType::NODE:
            {
                if (v.type == cypher::Entry::NODE_SNAPSHOT) {
                    std::regex regex_word("(V)\\[([0-9]+)\\]");
                    std::smatch match_group;
                    auto node_str = v.ToString();
                    CYPHER_THROW_ASSERT(std::regex_match(node_str, match_group, regex_word));
                    auto vid = static_cast<size_t>(std::stoll(match_group[2].str()));
                    record.InsertVertexByID(header[index].first, vid);
                } else if (v.type == cypher::Entry::NODE) {
                    auto vid = v.node->PullVid();
                    if (vid >= 0) {
                        record.Insert(header[index].first, vid, txn);
                    } else {
                        // OPTIONAL MATCH return null
                        record.InsertVertexByID(header[index].first, vid);
                    }
                } else {
                    throw lgraph::CypherException("unhandled record entry type: " +
                                                  cypher::Entry::ToString(v.type));
                }
                break;
            }
        case lgraph_api::LGraphType::RELATIONSHIP:
            {
                if (v.type == cypher::Entry::RELP_SNAPSHOT) {
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
                    record.InsertEdgeByID(header[index].first, lgraph_api::EdgeUid(start,
                                                                                   end,
                                                                                   lid,
                                                                                   tid,
                                                                                   eid));
                } else if (v.type == cypher::Entry::RELATIONSHIP) {
                    auto uit = v.relationship->ItRef();
                    auto uid = uit->GetUid();
                    record.Insert(header[index].first, uid, txn);
                } else {
                    throw lgraph::CypherException("unhandled record entry type: " +
                                                  cypher::Entry::ToString(v.type));
                }
                break;
            }
        case lgraph_api::LGraphType::PATH:
            {
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
                    CYPHER_THROW_ASSERT(std::regex_match(path_pattern_str, match_group, regex_word));
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
                        auto tmp_id =start;
                        forward = false;
                        start = end;
                        end = tmp_id;
                    }
                    path.Append(Edge(start, lid, tid, end, eid, forward));
                }
                record.Insert(header[index].first, path, txn);
                break;
            }
        default:
            if (v.constant.array != nullptr)
                record.Insert(header[index].first, lgraph_api::FieldData(v.ToString()));
            else
                record.Insert(header[index].first, v.constant.scalar);
            break;
        }
        index++;
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
    explicit ProduceResults() : OpBase(OpType::PRODUCE_RESULTS, "Produce Results") {
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
        auto child = children[0];
        auto res = child->Consume(ctx);
        if (res != OP_OK) return res;
        auto &record = ctx->result_->NewRecord();
        RRecordToURecord(ctx->txn_.get(), ctx->result_->Header(), child->record, record);
        return OP_OK;
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
