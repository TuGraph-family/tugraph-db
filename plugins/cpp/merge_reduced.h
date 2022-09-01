/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include "lgraph/lgraph_olap.h"

/**
 * We assume in this file:
 * 1. The given conditions can match no more than one vertex/edge.
 * 2. There is no duplicated edges.
 */

/**
 * return value:
 * -1: no vertex found
 * -2: no index ready
 * -3: number of fields and data values do not match
 * >=0: the first matched vertex id
 */
int64_t MatchVertex(lgraph_api::Transaction &txn, const std::string &label,
                    const std::vector<std::string> &field_names,
                    const std::vector<lgraph_api::FieldData> &field_values, std::string &err) {
    if (field_names.size() != field_values.size()) {
        err = "number of fields and data values do not match";
        return -3;
    }
    bool index_ready = false;
    for (int i = 0; i < (int)field_names.size(); i++) {
        if (!txn.IsVertexIndexed(label, field_names[i])) continue;
        auto iit =
            txn.GetVertexIndexIterator(label, field_names[i], field_values[i], field_values[i]);
        while (iit.IsValid()) {
            auto vit = txn.GetVertexIterator(iit.GetVid());
            size_t j = 0;
            for (auto &fn : field_names) {
                if (vit[fn] != field_values[j]) break;
                j++;
            }
            if (vit.GetLabel() == label && j == field_names.size()) {
                return vit.GetId();
            }
            iit.Next();
        }
        index_ready = true;
        break;
    }
    if (index_ready) {
        err = "no vertex found";
        return -1;
    } else {
        err = "no index ready";
        return -2;
    }
}

/**
 * std::pair<bool, int64_t> MergeVertex(
 * const std::string& label_name,
 * const std::vector<std::pair<std::string, FieldData>> match_fields,
 * const std::vector<std::pair<std::string, FieldData>> set_fields_on_create,
 * const std::vector<std::pair<std::string, FieldData>> set_fields_on_match)
 */
std::pair<bool, int64_t> MergeVertex(
    lgraph_api::Transaction &txn, const std::string &label,
    const std::vector<std::string> &field_names,
    const std::vector<lgraph_api::FieldData> &field_values,
    const std::vector<std::string> &field_names_on_create,
    const std::vector<lgraph_api::FieldData> &field_values_on_create,
    const std::vector<std::string> &field_names_on_match,
    const std::vector<lgraph_api::FieldData> &field_values_on_match, std::string &err) {
    if (!txn.IsValid() || txn.IsReadOnly()) {
        err = "invalid txn";
        return std::make_pair(false, -1);
    }
    auto match_vid = MatchVertex(txn, label, field_names, field_values, err);
    if (match_vid >= 0) {
        /* ON MATCH */
        auto vit = txn.GetVertexIterator(match_vid);
        vit.SetFields(field_names_on_match, field_values_on_match);
        return std::make_pair(true, match_vid);
    } else if (match_vid == -1) {
        /* ON CREATE */
        std::vector<std::string> fns;
        std::vector<lgraph_api::FieldData> fvs;
        fns.insert(fns.end(), field_names.begin(), field_names.end());
        fns.insert(fns.end(), field_names_on_create.begin(), field_names_on_create.end());
        fvs.insert(fvs.end(), field_values.begin(), field_values.end());
        fvs.insert(fvs.end(), field_values_on_create.begin(), field_values_on_create.end());
        int64_t vid = txn.AddVertex(label, fns, fvs);
        return std::make_pair(true, vid);
    } else {
        return std::make_pair(false, match_vid);
    }
}

/**
 * std::pair<bool, EdgeUid> MergeEdge(
 * const std::int64_t& verId1,
 * const std::int64_t& verId2,
 * const std::string& edge_direction,
 * const std::vector<std::pair<std::string, FieldData>> match_fields,
 * const std::vector<std::pair<std::string, FieldData>> set_fields_on_create,
 * const std::vector<std::pair<std::string, FieldData>> set_fields_on_match)
 */
std::pair<bool, lgraph_api::EdgeUid> MergeEdge(
    lgraph_api::Transaction &txn, int64_t src, int64_t dst, const std::string &label,
    const std::vector<std::string> &field_names,
    const std::vector<lgraph_api::FieldData> &field_values,
    const std::vector<std::string> &field_names_on_create,
    const std::vector<lgraph_api::FieldData> &field_values_on_create,
    const std::vector<std::string> &field_names_on_match,
    const std::vector<lgraph_api::FieldData> &field_values_on_match, std::string &err) {
    if (!txn.IsValid() || txn.IsReadOnly()) {
        err = "invalid txn";
        return std::make_pair(false, lgraph_api::EdgeUid());
    }
    if (field_names.size() != field_values.size()) {
        err = "number of fields and data values do not match";
        return std::make_pair(false, lgraph_api::EdgeUid());
    }
    auto eit = txn.GetVertexIterator(src).GetOutEdgeIterator();
    lgraph_api::EdgeUid match_euid(-1, -1, -1, -1, -1);
    while (eit.IsValid()) {
        if (eit.GetDst() == dst) {
            size_t j = 0;
            for (auto &fn : field_names) {
                if (eit[fn] != field_values[j]) break;
                j++;
            }
            if (eit.GetLabel() == label && j == field_names.size()) {
                match_euid.src = src;
                match_euid.dst = dst;
                match_euid.lid = eit.GetLabelId();
                match_euid.tid = eit.GetTemporalId();
                match_euid.eid = eit.GetEdgeId();
                break;
            }
        }
        eit.Next();
    }
    if (match_euid.src >= 0) {
        /* ON MATCH */
        eit.Goto(match_euid);
        eit.SetFields(field_names_on_match, field_values_on_match);
        return std::make_pair(true, match_euid);
    } else {
        /* ON CREATE */
        std::vector<std::string> fns;
        std::vector<lgraph_api::FieldData> fvs;
        fns.insert(fns.end(), field_names.begin(), field_names.end());
        fns.insert(fns.end(), field_names_on_create.begin(), field_names_on_create.end());
        fvs.insert(fvs.end(), field_values.begin(), field_values.end());
        fvs.insert(fvs.end(), field_values_on_create.begin(), field_values_on_create.end());
        auto eid = txn.AddEdge(src, dst, label, fns, fvs);
        return std::make_pair(true, eid);
    }
}

/**
 * std::pair<bool, EdgeUid> MergeEdge(
 * const std::string& label_name1,
 * const std::vector<std::pair<std::string, FieldData>> label1_match_fields,
 * const std::string& label_name2,
 * const std::vector<std::pair<std::string, FieldData>> label2_match_fields,
 * const std::string& edge_direction,
 * const std::vector<std::pair<std::string, FieldData>> match_fields,
 * const std::vector<std::pair<std::string, FieldData>> set_fields_on_create,
 * const std::vector<std::pair<std::string, FieldData>> set_fields_on_match)
 */
std::pair<bool, lgraph_api::EdgeUid> MergeEdge(
    lgraph_api::Transaction &txn, const std::string &src_label,
    const std::vector<std::string> &src_field_names,
    const std::vector<lgraph_api::FieldData> &src_field_values, const std::string &dst_label,
    const std::vector<std::string> &dst_field_names,
    const std::vector<lgraph_api::FieldData> &dst_field_values, const std::string &label,
    const std::vector<std::string> &field_names,
    const std::vector<lgraph_api::FieldData> &field_values,
    const std::vector<std::string> &field_names_on_create,
    const std::vector<lgraph_api::FieldData> &field_values_on_create,
    const std::vector<std::string> &field_names_on_match,
    const std::vector<lgraph_api::FieldData> &field_values_on_match, std::string &err) {
    if (!txn.IsValid() || txn.IsReadOnly()) {
        err = "invalid txn";
        return std::make_pair(false, lgraph_api::EdgeUid());
    }
    if (field_names.size() != field_values.size()) {
        err = "number of fields and data values do not match";
        return std::make_pair(false, lgraph_api::EdgeUid());
    }
    auto src_id = MatchVertex(txn, src_label, src_field_names, src_field_values, err);
    auto dst_id = MatchVertex(txn, dst_label, dst_field_names, dst_field_values, err);
    if (src_id < 0 || dst_id < 0) {
        err.append("invalid src/dst vertex");
        return std::make_pair(false, lgraph_api::EdgeUid());
    }
    return MergeEdge(txn, src_id, dst_id, label, field_names, field_values, field_names_on_create,
                     field_values_on_create, field_names_on_match, field_values_on_match, err);
}
