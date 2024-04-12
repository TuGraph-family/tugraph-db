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

#include <exception>
#include <iostream>
#include <unordered_set>
#include "lgraph/lgraph.h"
#include "lgraph/lgraph_types.h"
#include "lgraph/lgraph_utils.h"
#include "lgraph/lgraph_result.h"
#include "tools/json.hpp"

using namespace lgraph_api;
using json = nlohmann::json;

template <typename EIT>
class LabeledEdgeIterator {
 public:
    LabeledEdgeIterator(EIT&& eit, const int64_t src, const int64_t dst,
                        const std::vector<int16_t>& lids, int64_t per_node_limit)
        : eit_(std::move(eit)) {
        if (lids.empty()) {
            valid_ = false;
            return;
        }
        valid_ = true;
        lid_pos_ = 0;
        lids_ = lids;
        src_ = src;
        dst_ = dst;
        per_node_limit_ = per_node_limit;
        count_ = 1;
        eit_.Goto(EdgeUid(src_, dst_, lids_[lid_pos_], 0, 0), true);
        while (!_IsCurLabelValid() && _NextLabel()) {
        }
    }

    bool IsValid() { return valid_; }

    void Next() {
        count_ += 1;
        eit_.Next();
        while (!_IsCurLabelValid() && _NextLabel()) {
        }
    }

    EdgeUid GetUid() { return eit_.GetUid(); }

    EIT& Eit() { return eit_; }

 private:
    bool _IsCurLabelValid() {
        return lid_pos_ < lids_.size() && eit_.IsValid() && eit_.GetLabelId() == lids_[lid_pos_] &&
               (per_node_limit_ < 0 || count_ <= per_node_limit_);
    }

    bool _NextLabel() {
        count_ = 1;
        if (++lid_pos_ >= lids_.size()) {
            valid_ = false;
            return valid_;
        }
        eit_.Goto(EdgeUid(src_, dst_, lids_[lid_pos_], 0, 0), true);
        return true;
    }

    EIT eit_;
    bool valid_;
    int64_t src_, dst_;
    int64_t per_node_limit_;
    size_t count_;
    size_t lid_pos_;
    std::vector<int16_t> lids_;
};

typedef LabeledEdgeIterator<OutEdgeIterator> LabeledOutEdgeIterator;
typedef LabeledEdgeIterator<InEdgeIterator> LabeledInEdgeIterator;

extern "C" bool Process(GraphDB& db, const std::string& request, std::string& response) {
    static const std::string ACCOUNT_LABEL = "Account";
    static const std::string ACCOUNT_ID = "id";
    static const std::string ACCOUNT_ISBLOCKED = "isBlocked";
    static const std::string TRANSFER_LABEL = "transfer";
    static const std::string TRANSFER_TIMESTAMP = "timestamp";
    static const std::vector<std::string> TRANSFER_FIELD_NAMES = {"timestamp", "amount"};
    lgraph_api::Result api_result({{"msg", LGraphType::STRING}, {"txn", LGraphType::STRING}});
    auto record = api_result.MutableRecord();
    record->Insert("txn", FieldData::String("abort"));
    int64_t src_id, dst_id, time, amt, start_time, end_time;
    int64_t limit = -1;
    try {
        json input = json::parse(request);
        parse_from_json(src_id, "srcId", input);
        parse_from_json(dst_id, "dstId", input);
        parse_from_json(time, "time", input);
        parse_from_json(amt, "amt", input);
        parse_from_json(start_time, "startTime", input);
        parse_from_json(end_time, "endTime", input);
        parse_from_json(limit, "limit", input);
    } catch (std::exception& e) {
        record->Insert("msg", FieldData::String("json parse error: " + std::string(e.what())));
        response = api_result.Dump();
        return false;
    }
    auto txn = db.CreateWriteTxn();
    auto src = txn.GetVertexByUniqueIndex(ACCOUNT_LABEL, ACCOUNT_ID, FieldData(src_id));
    auto dst = txn.GetVertexByUniqueIndex(ACCOUNT_LABEL, ACCOUNT_ID, FieldData(dst_id));
    std::vector<int16_t> transfer_id = {
        (int16_t)txn.GetEdgeLabelId(TRANSFER_LABEL),
    };
    if (!src.IsValid() || !dst.IsValid()) {
        record->Insert("msg", FieldData::String("src/dst invalid"));
        response = api_result.Dump();
        txn.Abort();
        return false;
    }
    if (src.GetField(ACCOUNT_ISBLOCKED).AsBool() || dst.GetField(ACCOUNT_ISBLOCKED).AsBool()) {
        record->Insert("msg", FieldData::String("src/dst is blocked"));
        response = api_result.Dump();
        txn.Abort();
        return true;
    }
    txn.AddEdge(
        src.GetId(), dst.GetId(), TRANSFER_LABEL, TRANSFER_FIELD_NAMES,
        std::vector<FieldData>{FieldData(time), FieldData(amt)});
    std::unordered_set<int64_t> src_in;
    for (auto src_eit =
             LabeledInEdgeIterator(src.GetInEdgeIterator(), 0, src.GetId(), transfer_id, limit);
         src_eit.IsValid(); src_eit.Next()) {
        auto timestamp = src_eit.Eit().GetField(TRANSFER_TIMESTAMP);
        if (timestamp.AsInt64() > start_time && timestamp.AsInt64() < end_time) {
            src_in.emplace(src_eit.Eit().GetSrc());
        }
    }
    if (src_in.empty()) {
        record->Insert("msg", FieldData::String("not detected"));
        record->Insert("txn", FieldData::String("commit"));
        response = api_result.Dump();
        txn.Commit();
        return true;
    }
    for (auto dst_eit =
             LabeledOutEdgeIterator(dst.GetOutEdgeIterator(), dst.GetId(), 0, transfer_id, limit);
         dst_eit.IsValid(); dst_eit.Next()) {
        auto timestamp = dst_eit.Eit().GetField(TRANSFER_TIMESTAMP);
        if (timestamp.AsInt64() > start_time && timestamp.AsInt64() < end_time &&
            src_in.find(dst_eit.Eit().GetDst()) != src_in.end()) {
            txn.Abort();
            break;
        }
    }
    if (txn.IsValid()) {
        record->Insert("msg", FieldData::String("not detected"));
        record->Insert("txn", FieldData::String("commit"));
        response = api_result.Dump();
        txn.Commit();
        return true;
    }
    txn = db.CreateWriteTxn();
    src = txn.GetVertexByUniqueIndex(ACCOUNT_LABEL, ACCOUNT_ID, FieldData(src_id));
    dst = txn.GetVertexByUniqueIndex(ACCOUNT_LABEL, ACCOUNT_ID, FieldData(dst_id));
    if (!src.IsValid() || !dst.IsValid()) {
        txn.Abort();
        record->Insert("msg", FieldData::String("src/dst invalid"));
        response = api_result.Dump();
        return false;
    }
    src.SetField(ACCOUNT_ISBLOCKED, FieldData(true));
    dst.SetField(ACCOUNT_ISBLOCKED, FieldData(true));
    record->Insert("msg", FieldData::String("block src/dst"));
    record->Insert("txn", FieldData::String("commit"));
    response = api_result.Dump();
    txn.Commit();
    return true;
}
