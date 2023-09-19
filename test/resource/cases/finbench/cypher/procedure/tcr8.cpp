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

#include <algorithm>
#include <exception>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "fma-common/logger.h"
#include "lgraph/lgraph.h"
#include "lgraph/lgraph_edge_iterator.h"
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
    static const std::string LOAN_LABEL = "Loan";
    static const std::string LOAN_ID = "id";
    static const std::string LOAN_AMOUNT = "loanAmount";
    static const std::string ACCOUNT_ID = "id";
    static const std::string DEPOSIT_LABEL = "deposit";
    static const std::string TRANSFER_LABEL = "transfer";
    static const std::string WITHDRAW_LABEL = "withdraw";
    static const std::string TIMESTAMP = "timestamp";
    static const std::string AMOUNT = "amount";
    json output;
    int64_t id, start_time, end_time;
    int64_t limit = -1;
    float threshold;
    try {
        json input = json::parse(request);
        parse_from_json(id, "id", input);
        parse_from_json(threshold, "threshold", input);
        parse_from_json(start_time, "startTime", input);
        parse_from_json(end_time, "endTime", input);
        parse_from_json(limit, "limit", input);
    } catch (std::exception& e) {
        output["msg"] = "json parse error: " + std::string(e.what());
        response = output.dump();
        return false;
    }
    auto add_amount = [](std::unordered_map<int64_t, double>& m, int64_t vid, double amount) {
        auto it = m.find(vid);
        if (it == m.end() || it->second > amount) {
            m.emplace(vid, amount);
        }
    };
    auto add_dst =
        [](std::unordered_map<int64_t, std::unordered_map<std::string, std::pair<double, size_t>>>&
               merged_in,
           std::unordered_map<int64_t, double>& m, int64_t dst, int64_t src, const std::string& eid,
           double amount, float threshold, size_t hop) {
            if (amount > threshold * m.find(src)->second) {
                if (merged_in.find(dst) == merged_in.end()) {
                    merged_in[dst] = {};
                }
                if (merged_in[dst].find(eid) == merged_in[dst].end()) {
                    merged_in[dst][eid] = {amount, hop};
                }
            }
        };
    auto txn = db.CreateReadTxn();
    std::vector<int16_t> deposit_id = {
        (int16_t)txn.GetEdgeLabelId(DEPOSIT_LABEL),
    };
    std::vector<int16_t> edge_label_ids = {
        (int16_t)txn.GetEdgeLabelId(TRANSFER_LABEL),
        (int16_t)txn.GetEdgeLabelId(WITHDRAW_LABEL),
    };
    auto loan = txn.GetVertexByUniqueIndex(LOAN_LABEL, LOAN_ID, FieldData(id));
    auto loan_amount = loan.GetField(LOAN_AMOUNT).AsDouble();
    auto vit = txn.GetVertexIterator();
    std::unordered_map<int64_t, std::unordered_map<std::string, std::pair<double, size_t>>>
        merged_in;
    std::unordered_map<int64_t, double> min_amount;
    std::unordered_set<int64_t> src_set, dst_set;

    for (auto deposit =
             LabeledOutEdgeIterator(loan.GetOutEdgeIterator(), loan.GetId(), 0, deposit_id, limit);
         deposit.IsValid(); deposit.Next()) {
        auto timestamp = deposit.Eit().GetField(TIMESTAMP).AsInt64();
        auto amount = deposit.Eit().GetField(AMOUNT).AsDouble();
        if (timestamp > start_time && timestamp < end_time) {
            auto dst = deposit.Eit().GetDst();
            src_set.emplace(dst);
            add_amount(min_amount, dst, amount);
        }
    }
    for (size_t i = 1; i <= 3; i++) {
        for (auto& vid : src_set) {
            vit.Goto(vid);
            for (auto eit = LabeledOutEdgeIterator(vit.GetOutEdgeIterator(), vit.GetId(), 0,
                                                   edge_label_ids, limit);
                 eit.IsValid(); eit.Next()) {
                auto timestamp = eit.Eit().GetField(TIMESTAMP).AsInt64();
                auto amount = eit.Eit().GetField(AMOUNT).AsDouble();
                auto dst_vid = eit.Eit().GetDst();
                if (timestamp > start_time && timestamp < end_time) {
                    add_amount(min_amount, dst_vid, amount);
                    add_dst(merged_in, min_amount, dst_vid, vid, eit.Eit().GetUid().ToString(),
                            amount, threshold, i);

                    dst_set.emplace(dst_vid);
                }
            }
        }
        std::swap(src_set, dst_set);
        dst_set.clear();
    }
    // ratio, hop, dst
    std::vector<std::tuple<double, size_t, int64_t>> result;
    for (auto& kv1 : merged_in) {
        double sum = 0;
        size_t hop = std::numeric_limits<size_t>::max();
        vit.Goto(kv1.first);
        for (auto& kv2 : kv1.second) {
            sum += kv2.second.first;
            hop = std::min(kv2.second.second + 1, hop);
        }
        result.emplace_back(std::round(1000.0 * sum / loan_amount) / 1000, hop,
                            vit.GetField(ACCOUNT_ID).AsInt64());
    }
    std::sort(result.begin(), result.end(),
              [=](std::tuple<double, size_t, int64_t>& l, std::tuple<double, size_t, int64_t>& r) {
                  return std::get<1>(l) == std::get<1>(r)
                             ? (std::get<0>(l) == std::get<0>(r) ? std::get<2>(l) < std::get<2>(r)
                                                                 : std::get<0>(l) > std::get<0>(r))
                             : std::get<1>(l) > std::get<1>(r);
              });
    lgraph_api::Result api_result(
        {{"i", LGraphType::INTEGER}, {"r", LGraphType::DOUBLE}, {"d", LGraphType::INTEGER}});
    for (auto& item : result) {
        auto r = api_result.MutableRecord();
        r->Insert("i", FieldData::Int64(std::get<2>(item)));
        r->Insert("r", FieldData::Double(std::get<0>(item)));
        r->Insert("d", FieldData::Int64(std::get<1>(item)));
    }
    response = api_result.Dump();
    return true;
}
