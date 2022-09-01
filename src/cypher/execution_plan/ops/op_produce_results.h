/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

//
// Created by wt on 6/14/18.
//
#pragma once

#include "op.h"
#include "server/json_convert.h"
#include <regex>

/* Runtime Record to User Record */
static void RRecordToURecord(
    lgraph::Transaction* txn, const std::vector<std::pair<std::string, lgraph_api::ResultElementType>> &header,
    const std::shared_ptr<cypher::Record> &record_ptr, lgraph_api::Record &record) {
    using unordered_json = nlohmann::ordered_json;
    if (header.empty()) {
        return;
    }
    int index = 0;
    for (auto &v : record_ptr->values) {
        switch (header[index].second) {
        case lgraph_api::ResultElementType::NODE:
            {
                lgraph::VertexId vid = 0;
                if (v.type == cypher::Entry::NODE_SNAPSHOT) {
                    uint start = 2;
                    uint end = v.ToString().size() - 3;
                    vid = std::stoi(v.ToString().substr(start, end));
                    record.InsertVertexByID(header[index].first, vid);
                } else if (v.type == cypher::Entry::NODE) {
                    vid = v.node->PullVid();
                    if (vid >= 0) {
                        record.InsertVertex(header[index].first, vid, txn);
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
        case lgraph_api::ResultElementType::RELATIONSHIP:
            {
                if (v.type == cypher::Entry::RELP_SNAPSHOT) {
                    lgraph_api::EdgeUid edge_uid;
                    auto edge_uid_vec =
                        fma_common::Split(v.ToString().substr(2, v.ToString().size() - 3), "_");
                    edge_uid.src = static_cast<int64_t>(std::stoll(edge_uid_vec[0]));
                    edge_uid.dst = static_cast<int64_t>(std::stoll(edge_uid_vec[1]));
                    edge_uid.lid = static_cast<uint16_t>(std::stoll(edge_uid_vec[2]));
                    edge_uid.eid = static_cast<int64_t>(std::stoll(edge_uid_vec[3]));
                    record.InsertEdgeByID(header[index].first, edge_uid);
                } else if (v.type == cypher::Entry::RELATIONSHIP) {
                    auto uit = v.relationship->ItRef();
                    auto uid = uit->GetUid();
                    record.InsertEdge(header[index].first, uid, txn);
                } else {
                    throw lgraph::CypherException("unhandled record entry type: " +
                                                  cypher::Entry::ToString(v.type));
                }
                break;
            }
        case lgraph_api::ResultElementType::PATH:
            {
                // TODO: After merging the PATH type, need to modify the code here
                auto pathArrays = v.constant.array;
                unordered_json result;
                for (auto& pathArray : *pathArrays) {
                   std::string arr = pathArray.ToString("NUL");
                   unordered_json arrJson;
                   std::map<std::string, unordered_json> properties;
                   if (arr.at(0) == 'V') {
                        auto strVid = arr.substr(arr.find("[") + 1, arr.find("]") - 2);
                        lgraph::VertexId vid = std::atoll(strVid.c_str());
                        auto vit = std::make_unique<lgraph::VIter>(txn,
                                                               lgraph::VIter::VERTEX_ITER,
                                                               vid);
                        arrJson["identity"] = vid;
                        arrJson["label"] = vit->GetLabel();
                        auto node_fields = vit->GetFields();
                        for (const auto &n : node_fields) {
                            properties[n.first] = lgraph_rfc::FieldDataToJson(n.second);
                        }
                        if (!properties.empty()) {
                            arrJson["properties"] = properties;
                        }
                        result[arr] = arrJson;
                   } else if (arr.at(0) == 'E') {
                        std::regex regex_word("E\\[([0-9]+)_([0-9]+)_([0-9]+)_([0-9]+)\\]");
                        std::smatch match_group;
                        CYPHER_THROW_ASSERT(std::regex_match(arr, match_group, regex_word));
                        auto uid = lgraph::EdgeUid(static_cast<int64_t>(std::atoll(match_group[1].str().c_str())),
                                                    static_cast<int64_t>(std::atoll(match_group[2].str().c_str())),
                                                    static_cast<uint16_t>(std::atoll(match_group[3].str().c_str())),
                                                    0, 
                                                    static_cast<int64_t>(std::atoll(match_group[4].str().c_str())));
                        auto vit = std::make_unique<lgraph::EIter>(txn, uid);
                        auto rel_fields = vit->GetFields();
                        arrJson["identity"] = uid.eid;
                        arrJson["start"] = uid.src;
                        arrJson["end"] = uid.dst;
                        arrJson["label"] = vit->GetLabel();
                        arrJson["label_id"] = uid.lid;
                        for (const auto &r : rel_fields) {
                            properties[r.first] = lgraph_rfc::FieldDataToJson(r.second);
                        }
                        if(!properties.empty()) {
                            arrJson["properties"] = properties;
                        }
                        result[arr] = arrJson;
                   } else {
                        throw lgraph::CypherException("unhandled path array type: " + arr);
                   }
                }
                record.Insert(header[index].first, lgraph_api::FieldData(result.dump()));
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
