/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

//
// Created by wt on 6/14/18.
//
#pragma once

#include "op.h"

/* Runtime Record to User Record */
static void RRecordToURecord(
    const std::vector<std::pair<std::string, lgraph_api::ResultElementType>> &header,
    const std::shared_ptr<cypher::Record> &record_ptr, lgraph_api::Record &record) {
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
                } else if (v.type == cypher::Entry::NODE) {
                    vid = v.node->PullVid();
                } else {
                    throw lgraph::CypherException("unhandled record entry type: " +
                                                  cypher::Entry::ToString(v.type));
                }
                record.InsertVertexByID(header[index].first, vid);
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
                    record.InsertEdgeByID(header[index].first, v.relationship->ItRef()->GetUid());
                } else {
                    throw lgraph::CypherException("unhandled record entry type: " +
                                                  cypher::Entry::ToString(v.type));
                }
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
        RRecordToURecord(ctx->result_->Header(), child->record, record);
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
