/**
 * Copyright 2024 AntGroup CO., Ltd.
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

#pragma once

#include "lgraph/olap_on_db.h"

struct MatchIterator {
    std::string label;
    std::vector<std::string> field_names;
    std::vector<lgraph_api::FieldData> field_values;

    MatchIterator(lgraph_api::Transaction &txn, const std::string &l,
                  const std::vector<std::string> &fns,
                  const std::vector<lgraph_api::FieldData> &fvs)
        : label(l), field_names(fns), field_values(fvs), txn_(&txn) {
        if (field_names.size() == field_values.size()) {
            /* there must be at least 1 valid index */
            for (int i = 0; i < (int)field_names.size(); i++) {
                if (!txn_->IsVertexIndexed(label, field_names[i])) continue;
                iit_ = new lgraph_api::VertexIndexIterator(txn_->GetVertexIndexIterator(
                    label, field_names[i], field_values[i], field_values[i]));
                while (iit_->IsValid()) {
                    if (CheckFields()) {
                        /* ON MATCH */
                        valid_ = true;
                        break;
                    }
                    iit_->Next();
                }
                break;
            }
        }
    }

    ~MatchIterator() {
        delete iit_;
        iit_ = nullptr;
    }

    bool Next() {
        if (!iit_) return false;
        valid_ = false;
        while (iit_->IsValid()) {
            iit_->Next();
            if (iit_->IsValid() && CheckFields()) {
                valid_ = true;
                break;
            }
        }
        return valid_;
    }

    bool IsValid() const { return iit_ && valid_; }

    bool IsIndexValid() const { return iit_ != nullptr; }

    int64_t GetVid() const { return IsValid() ? iit_->GetVid() : -1; }

 private:
    lgraph_api::Transaction *txn_ = nullptr;
    lgraph_api::VertexIndexIterator *iit_ = nullptr;
    bool valid_ = false;

    bool CheckFields() const {
        if (!iit_ || !iit_->IsValid()) return false;
        auto vit = txn_->GetVertexIterator(iit_->GetVid());
        size_t j = 0;
        for (auto &fn : field_names) {
            if (vit[fn] != field_values[j]) break;
            j++;
        }
        return j == field_names.size();
    }
};

/**
 * Merge Vertex Clause
 * MERGE (<node-name>:<label-name>
 * {<Property1-name>:<Property1-Value>[,<PropertyN-name>:<PropertyN-Value>] ON CREATE SET
 * <node-name>.<Property1-name>=<Property1-Value> [,<node-name>.<PropertyN-name>=<PropertyN-Value>]
 * ON MATCH SET <node-name>.<PropertyX-name>=<PropertyX-Value>
 * [,<node-name>.<PropertyN-name>=<PropertyN-Value>]
 */
extern "C" bool MergeVertex(lgraph_api::Transaction &txn, MatchIterator vertex,
                            const std::vector<std::string> &field_names_on_create,
                            const std::vector<lgraph_api::FieldData> &field_values_on_create,
                            const std::vector<std::string> &field_names_on_match,
                            const std::vector<lgraph_api::FieldData> &field_values_on_match,
                            std::string &err) {
    /* check txn */
    if (!txn.IsValid() || txn.IsReadOnly()) {
        err = "invalid txn";
        return false;
    }
    if (vertex.IsValid()) {
        /* ON MATCH */
        std::vector<int64_t> match_vids;
        do {
            match_vids.emplace_back(vertex.GetVid());
            vertex.Next();
        } while (vertex.IsValid());
        auto vit = txn.GetVertexIterator();
        for (auto vid : match_vids) {
            vit.Goto(vid);
            vit.SetFields(field_names_on_match, field_values_on_match);
        }
    } else if (vertex.IsIndexValid()) {
        /* ON CREATE */
        std::vector<std::string> fns;
        std::vector<lgraph_api::FieldData> fvs;
        fns.insert(fns.end(), vertex.field_names.begin(), vertex.field_names.end());
        fns.insert(fns.end(), field_names_on_create.begin(), field_names_on_create.end());
        fvs.insert(fvs.end(), vertex.field_values.begin(), vertex.field_values.end());
        fvs.insert(fvs.end(), field_values_on_create.begin(), field_values_on_create.end());
        txn.AddVertex(vertex.label, fns, fvs);
    } else {
        err = "invalid index";
        return false;
    }
    return true;
}

/**
 * MATCH (<node-name1>:<label-name>
 * {<Property1-name>:<Property1-Value>[,<PropertyN-name>:<PropertyN-Value>]})
 * ,(<node-name2>:<label-name>
 * {<Property1-name>:<Property1-Value>[,<PropertyN-name>:<PropertyN-Value>]})
 * [,(<node-nameM>:<label-name>
 * {<Property1-name>:<Property1-Value>[,<PropertyN-name>:<PropertyN-Value>]})] MERGE
 * (node-name1)-[[<edge-name>]:<edge-type>]->(node-name2) ON CREATE SET
 * <edge-name>.<Property1-name>=<Property1-Value> [,<edge-name>.<PropertyN-name>=<PropertyN-Value>]
 * ON MATCH SET <node-name>.<Property1-name>=<Property1-Value>
 * [,<node-name>.<PropertyN-name>=<PropertyN-Value>]
 */
extern "C" bool MergeEdge(lgraph_api::Transaction &txn, MatchIterator src, MatchIterator dst,
                          const std::string &label, const std::vector<std::string> &field_names,
                          const std::vector<lgraph_api::FieldData> &field_values,
                          const std::vector<std::string> &field_names_on_create,
                          const std::vector<lgraph_api::FieldData> &field_values_on_create,
                          const std::vector<std::string> &field_names_on_match,
                          const std::vector<lgraph_api::FieldData> &field_values_on_match,
                          std::string &err) {
    /* check txn */
    if (!txn.IsValid() || txn.IsReadOnly()) {
        err = "invalid txn";
        return false;
    }
    if (!src.IsValid() || !dst.IsValid()) {
        err = "invalid vertex";
        return false;
    }
    if (field_names.size() != field_values.size()) {
        err = "number of fields and data values do not match";
        return false;
    }
    std::vector<int64_t> src_ids, dst_ids;
    while (src.IsValid()) {
        src_ids.emplace_back(src.GetVid());
        src.Next();
    }
    while (dst.IsValid()) {
        dst_ids.emplace_back(dst.GetVid());
        dst.Next();
    }
    for (auto src_id : src_ids) {
        for (auto dst_id : dst_ids) {
            auto eit = txn.GetVertexIterator(src_id).GetOutEdgeIterator();
            std::vector<lgraph::EdgeUid> match_eids;
            while (eit.IsValid()) {
                if (eit.GetDst() == dst_id) {
                    size_t j = 0;
                    for (auto &fn : field_names) {
                        if (eit[fn] != field_values[j]) break;
                        j++;
                    }
                    if (eit.GetLabel() == label && j == field_names.size()) {
                        match_eids.emplace_back(src_id, dst_id, eit.GetLabelId(),
                                                eit.GetTemporalId(), eit.GetEdgeId());
                    }
                }
                eit.Next();
            }
            if (!match_eids.empty()) {
                /* ON MATCH */
                for (auto uid : match_eids) {
                    eit.Goto(uid);
                    eit.SetFields(field_names_on_match, field_values_on_match);
                }
            } else {
                /* ON CREATE */
                std::vector<std::string> fns;
                std::vector<lgraph_api::FieldData> fvs;
                fns.insert(fns.end(), field_names.begin(), field_names.end());
                fns.insert(fns.end(), field_names_on_create.begin(), field_names_on_create.end());
                fvs.insert(fvs.end(), field_values.begin(), field_values.end());
                fvs.insert(fvs.end(), field_values_on_create.begin(), field_values_on_create.end());
                txn.AddEdge(src_id, dst_id, label, fns, fvs);
            }
        }
    }
    return true;
}
