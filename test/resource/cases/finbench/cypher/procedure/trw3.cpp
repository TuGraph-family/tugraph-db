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
    static const std::string PERSON_LABEL = "Person";
    static const std::string PERSON_ID = "id";
    static const std::string PERSON_ISBLOCKED = "isBlocked";
    static const std::string GUARANTEE_LABEL = "guarantee";
    static const std::string GUARANTEE_TIMESTAMP = "timestamp";
    static const std::string LOAN_LOANAMOUNT = "loanAmount";
    static const std::vector<std::string> GUARANTEE_FIELD_NAMES = {"timestamp"};
    static const std::string APPLY_LABEL = "apply";
    lgraph_api::Result api_result({{"msg", LGraphType::STRING}, {"txn", LGraphType::STRING}});
    auto record = api_result.MutableRecord();
    record->Insert("txn", FieldData::String("abort"));
    int64_t src_id, dst_id, time, threshold, start_time, end_time;
    int64_t limit = -1;
    try {
        json input = json::parse(request);
        parse_from_json(src_id, "srcId", input);
        parse_from_json(dst_id, "dstId", input);
        parse_from_json(time, "time", input);
        parse_from_json(threshold, "threshold", input);
        parse_from_json(start_time, "startTime", input);
        parse_from_json(end_time, "endTime", input);
        parse_from_json(limit, "limit", input);
    } catch (std::exception& e) {
        record->Insert("msg", FieldData::String("json parse error: " + std::string(e.what())));
        response = api_result.Dump();
        return false;
    }
    auto txn = db.CreateWriteTxn();
    auto src = txn.GetVertexByUniqueIndex(PERSON_LABEL, PERSON_ID, FieldData(src_id));
    auto dst = txn.GetVertexByUniqueIndex(PERSON_LABEL, PERSON_ID, FieldData(dst_id));
    std::vector<int16_t> guarantee_id = {
        (int16_t)txn.GetEdgeLabelId(GUARANTEE_LABEL),
    };
    std::vector<int16_t> apply_id = {
        (int16_t)txn.GetEdgeLabelId(APPLY_LABEL),
    };

    if (!src.IsValid() || !dst.IsValid()) {
        record->Insert("msg", FieldData::String("src/dst invalid"));
        response = api_result.Dump();
        txn.Abort();
        return false;
    }
    if (src.GetField(PERSON_ISBLOCKED).AsBool() || dst.GetField(PERSON_ISBLOCKED).AsBool()) {
        record->Insert("msg", FieldData::String("src/dst is blocked"));
        response = api_result.Dump();
        txn.Abort();
        return true;
    }
    txn.AddEdge(src.GetId(), dst.GetId(), GUARANTEE_LABEL, GUARANTEE_FIELD_NAMES,
                std::vector<FieldData>{FieldData(time)});

    // expand src
    std::unordered_set<int64_t> visited, dst_set, src_set{src.GetId()}, loans;
    while (!src_set.empty()) {
        for (auto& vid : src_set) {
            src.Goto(vid);
            for (auto eit = LabeledOutEdgeIterator(src.GetOutEdgeIterator(), src.GetId(), 0,
                                                   guarantee_id, limit);
                 eit.IsValid(); eit.Next()) {
                auto timestamp = eit.Eit().GetField(GUARANTEE_TIMESTAMP).AsInt64();
                if (timestamp > start_time && timestamp < end_time &&
                    visited.find(eit.Eit().GetDst()) == visited.end()) {
                    dst_set.emplace(eit.Eit().GetDst());
                    visited.emplace(eit.Eit().GetDst());
                }
            }
        }
        swap(src_set, dst_set);
        dst_set.clear();
    }
    for (auto& vid : visited) {
        src.Goto(vid);
        for (auto eit =
                 LabeledOutEdgeIterator(src.GetOutEdgeIterator(), src.GetId(), 0, apply_id, limit);
             eit.IsValid(); eit.Next()) {
            loans.emplace(eit.Eit().GetDst());
        }
    }
    double loan_sum = 0;
    for (auto& loan : loans) {
        src.Goto(loan);
        loan_sum += src.GetField(LOAN_LOANAMOUNT).AsDouble();
        if (loan_sum > threshold) {
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
    src = txn.GetVertexByUniqueIndex(PERSON_LABEL, PERSON_ID, FieldData(src_id));
    dst = txn.GetVertexByUniqueIndex(PERSON_LABEL, PERSON_ID, FieldData(dst_id));
    if (!src.IsValid() || !dst.IsValid()) {
        txn.Abort();
        record->Insert("msg", FieldData::String("src/dst invalid"));
        response = api_result.Dump();
        return false;
    }
    src.SetField(PERSON_ISBLOCKED, FieldData(true));
    dst.SetField(PERSON_ISBLOCKED, FieldData(true));
    record->Insert("msg", FieldData::String("block src/dst"));
    record->Insert("txn", FieldData::String("commit"));
    response = api_result.Dump();
    txn.Commit();
    return true;
}
