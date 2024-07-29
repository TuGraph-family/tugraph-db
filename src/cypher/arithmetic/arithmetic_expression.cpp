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
// Created by wt on 6/15/18.
//
#include <cmath>
#include <cstdint>
#include <random>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <regex>
#include "cypher/cypher_exception.h"
#include "cypher/cypher_types.h"
#include "cypher/procedure/utils.h"
#include "cypher/parser/expression.h"
#include "cypher/parser/clause.h"
#include "cypher/parser/symbol_table.h"
#include "cypher/arithmetic/arithmetic_expression.h"
#include "cypher/filter/filter.h"
#include "db/galaxy.h"

#define CHECK_NODE(e)                                                                      \
    do {                                                                                   \
        if (!e.IsNode())                                                                   \
            throw lgraph::CypherException("Invalid argument of " + std::string(__func__)); \
    } while (0)

#define CHECK_RELP(e)                                                                      \
    do {                                                                                   \
        if (!e.IsRelationship())                                                           \
            throw lgraph::CypherException("Invalid argument of " + std::string(__func__)); \
    } while (0)

#define CHECK_CONSTANT(e)                                                                  \
    do {                                                                                   \
        if (e.type != Entry::CONSTANT)                                                     \
            throw lgraph::CypherException("Invalid argument of " + std::string(__func__)); \
    } while (0)

#define VALIDATE_IT(e) (e.node->ItRef() && e.node->PullVid() >= 0)

#define VALIDATE_EIT(e) (e.relationship->ItRef() && e.relationship->ItRef()->IsValid())

namespace cypher {

cypher::FieldData BuiltinFunction::Id(RTContext *ctx, const Record &record,
                                      const std::vector<ArithExprNode> &args) {
    /* note the 1st argument is DISTINCT_OR_NOT */
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    // TODO(anyone) handle snapshot of node/relp
    if (r.IsNode()) {
        if (!VALIDATE_IT(r)) return {};
        return cypher::FieldData(lgraph::FieldData(r.node->PullVid()));
    } else if (r.IsRelationship()) {
        if (!VALIDATE_EIT(r)) return {};
        return cypher::FieldData(lgraph::FieldData(r.relationship->ItRef()->GetId()));
    } else {
        throw lgraph::CypherException("Invalid argument of " + std::string(__func__));
    }
}

cypher::FieldData BuiltinFunction::EUid(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_RELP(r);
    if (!VALIDATE_EIT(r)) return {};
    return cypher::FieldData(
        lgraph::FieldData(cypher::_detail::EdgeUid2String(r.relationship->ItRef()->GetUid())));
}

cypher::FieldData BuiltinFunction::Label(RTContext *ctx, const Record &record,
                                         const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    lgraph::FieldData res;
    if (r.IsNode()) {
        if (!r.node->IsValidAfterMaterialize(ctx)) CYPHER_INTL_ERR();
        res = lgraph::FieldData(r.node->ItRef()->GetLabel());
    } else if (r.IsRelationship() && VALIDATE_EIT(r)) {
        res = lgraph::FieldData(r.relationship->ItRef()->GetLabel());
    } else {
        throw lgraph::CypherException("Invalid argument of " + std::string(__func__));
    }
    return cypher::FieldData(res);
}

cypher::FieldData BuiltinFunction::Type(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_RELP(r);
    if (!VALIDATE_EIT(r)) return cypher::FieldData();
    return cypher::FieldData(lgraph::FieldData(r.relationship->ItRef()->GetLabel()));
}

cypher::FieldData BuiltinFunction::EndNode(RTContext *ctx, const Record &record,
                                           const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_RELP(r);
    return cypher::FieldData(lgraph::FieldData(r.relationship->ItRef()->GetDst()));
}

cypher::FieldData BuiltinFunction::StartNode(RTContext *ctx, const Record &record,
                                             const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_RELP(r);
    return cypher::FieldData(lgraph::FieldData(r.relationship->ItRef()->GetSrc()));
}

cypher::FieldData BuiltinFunction::Properties(RTContext *ctx, const Record &record,
                                              const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    lgraph::FieldData res;
    if (r.type == Entry::NODE && r.node && VALIDATE_IT(r)) {
        if (!r.node->IsValidAfterMaterialize(ctx)) CYPHER_INTL_ERR();
        res = lgraph::FieldData(r.node->ItRef()->Properties());
    } else if (r.type == Entry::RELATIONSHIP && r.relationship && VALIDATE_EIT(r)) {
        res = lgraph::FieldData(r.relationship->ItRef()->Properties());
    } else if (r.type == Entry::CONSTANT && r.constant.array) {
        auto ret = cypher::FieldData::Array(0);
        lgraph::VIter vit;
        lgraph::EIter eit;
        lgraph::EdgeUid euid;
        for (size_t i = 0; i < r.constant.array->size(); i++) {
            std::string s = r.constant.array->at(i).AsString();
            if (s.rfind("V[", 0) == 0) {
                auto vid = static_cast<int64_t>(std::stoll(s.substr(2, s.size() - 3)));
                vit.Initialize(ctx->txn_->GetTxn().get(), lgraph::VIter::VERTEX_ITER, vid);
                ret.array->emplace_back(lgraph::FieldData(vit.Properties()));
            } else if (s.rfind("E[", 0) == 0) {
                auto euidVec = fma_common::Split(s.substr(2, s.size() - 3), "_");
                euid.src = static_cast<int64_t>(std::stoll(euidVec[0]));
                euid.dst = static_cast<int64_t>(std::stoll(euidVec[1]));
                euid.lid = static_cast<uint16_t>(std::stoll(euidVec[2]));
                euid.eid = static_cast<int64_t>(std::stoll(euidVec[3]));
                eit.Initialize(ctx->txn_->GetTxn().get(), euid);
                ret.array->emplace_back(lgraph::FieldData(eit.Properties()));
            } else {
                throw lgraph::CypherException("Invalid argument of " + std::string(__func__));
            }
        }
        return ret;
    } else {
        throw lgraph::CypherException("Invalid argument of " + std::string(__func__));
    }
    return cypher::FieldData(res);
}

cypher::FieldData BuiltinFunction::Head(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto r = args[1].Evaluate(ctx, record);
    if (!r.IsArray()) throw lgraph::CypherException("List expected in head(): " + r.ToString());
    return r.constant.array->empty() ? cypher::FieldData()
                                     : cypher::FieldData(r.constant.array->front());
}

cypher::FieldData BuiltinFunction::Last(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto r = args[1].Evaluate(ctx, record);
    if (!r.IsArray()) throw lgraph::CypherException("List expected in last(): " + r.ToString());
    return r.constant.array->empty() ? cypher::FieldData()
                                     : cypher::FieldData(r.constant.array->back());
}

cypher::FieldData BuiltinFunction::MaxInList(RTContext *ctx, const Record &record,
                                             const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto r = args[1].Evaluate(ctx, record);
    if (!r.IsArray())
        throw lgraph::CypherException("List expected in maxInList(): " + r.ToString());
    if (r.constant.array->empty()) {
        throw lgraph::CypherException("List cannot be empty in maxInList()");
    }
    size_t pos = 0;
    for (size_t i = 0; i < r.constant.array->size(); i++) {
        if ((*r.constant.array)[i] > (*r.constant.array)[pos]) {
            pos = i;
        }
    }
    return cypher::FieldData((*r.constant.array)[pos]);
}

cypher::FieldData BuiltinFunction::MinInList(RTContext *ctx, const Record &record,
                                             const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto r = args[1].Evaluate(ctx, record);
    if (!r.IsArray())
        throw lgraph::CypherException("List expected in minInList(): " + r.ToString());
    if (r.constant.array->empty()) {
        throw lgraph::CypherException("List cannot be empty in minInList()");
    }
    size_t pos = 0;
    for (size_t i = 0; i < r.constant.array->size(); i++) {
        if ((*r.constant.array)[i] < (*r.constant.array)[pos]) {
            pos = i;
        }
    }
    return cypher::FieldData((*r.constant.array)[pos]);
}

cypher::FieldData BuiltinFunction::Size(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto r = args[1].Evaluate(ctx, record);
    if (r.IsArray()) {
        return cypher::FieldData(
            ::lgraph::FieldData(static_cast<int64_t>(r.constant.array->size())));
    } else if (r.IsString()) {
        return cypher::FieldData(
            ::lgraph::FieldData(static_cast<int64_t>(r.constant.scalar.AsString().size())));
    }
    throw lgraph::CypherException("Type mismatch: expected String or List<T>: " + r.ToString());
}

cypher::FieldData BuiltinFunction::Length(RTContext *ctx, const Record &record,
                                          const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();

    auto arg1 = args[1].Evaluate(ctx, record);
    switch (arg1.type) {
        case Entry::CONSTANT:
            if (arg1.constant.IsString()) {
                auto len = arg1.constant.ToString("").length();
                return cypher::FieldData(lgraph::FieldData(static_cast<int64_t>(len)));

            } else if (arg1.constant.IsArray()) {
                return cypher::FieldData(
                    lgraph::FieldData(static_cast<int64_t>(arg1.constant.array->size() / 2)));
            }
            break;
        default:
            break;
    }

    throw lgraph::CypherException("Function `LENGTH()` is not supported for: " + arg1.ToString());
}

// TODO(jinyejun.jyj): support native path in FieldData
cypher::FieldData BuiltinFunction::Nodes(RTContext *ctx, const Record &record,
                                         const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto r = args[1].Evaluate(ctx, record);
    if (!r.IsArray()) throw lgraph::CypherException("Path expected in nodes(): " + r.ToString());
    std::vector<lgraph::FieldData> vids;
    for (size_t i = 0; i < r.constant.array->size(); i += 2) {
        std::string vid_str = (*r.constant.array)[i].AsString(); /* V[0] */
        auto open_bracket_pos = vid_str.find('[');
        auto close_bracket_pos = vid_str.find(']');
        auto vid_number_str =
            vid_str.substr(open_bracket_pos + 1, close_bracket_pos - open_bracket_pos - 1);
        vids.emplace_back(
            lgraph::FieldData(static_cast<int64_t>(std::stoll(vid_number_str.c_str()))));
    }
    return cypher::FieldData(vids);
}

// TODO(huangke): support native path in FieldData
cypher::FieldData BuiltinFunction::Relationships(RTContext *ctx, const Record &record,
                                         const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto r = args[1].Evaluate(ctx, record);
    if (!r.IsArray()) throw lgraph::CypherException("Path expected in nodes(): " + r.ToString());
    std::vector<lgraph::FieldData> euids;
    for (size_t i = 1; i < r.constant.array->size(); i += 2) {
        std::string euid_str = (*r.constant.array)[i].AsString(); /* E[0_0_0_0_0] */
        auto open_bracket_pos = euid_str.find('[');
        auto close_bracket_pos = euid_str.find(']');
        auto euid_tuple_str =
            euid_str.substr(open_bracket_pos + 1, close_bracket_pos - open_bracket_pos - 1);
        euids.emplace_back(
            lgraph::FieldData(euid_tuple_str));
    }
    return cypher::FieldData(euids);
}

cypher::FieldData BuiltinFunction::Labels(RTContext *ctx, const Record &record,
                                          const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    auto ret = cypher::FieldData::Array(0);
    if (r.IsNode()) {
        if (!r.node->IsValidAfterMaterialize(ctx)) CYPHER_INTL_ERR();
        ret.array->emplace_back(lgraph::FieldData(r.node->ItRef()->GetLabel()));
    } else if (r.IsRelationship() && VALIDATE_EIT(r)) {
        ret.array->emplace_back(lgraph::FieldData(r.relationship->ItRef()->GetLabel()));
    } else {
        throw lgraph::CypherException("Invalid argument of " + std::string(__func__));
    }
    return ret;
}

cypher::FieldData BuiltinFunction::Keys(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_NODE(r);
    if (!r.node->IsValidAfterMaterialize(ctx)) CYPHER_INTL_ERR();
    auto ret = cypher::FieldData::Array(0);
    for (auto &k : r.node->ItRef()->Keys()) {
        ret.array->emplace_back(lgraph::FieldData(k));
    }
    return ret;
}

cypher::FieldData BuiltinFunction::Range(RTContext *ctx, const Record &record,
                                         const std::vector<ArithExprNode> &args) {
    /* Arguments:
     * start    An expression that returns an integer value.
     * end      An expression that returns an integer value.
     * step     A numeric expression defining the difference between any two consecutive values,
     * with a default of 1.
     */
    if (args.size() < 3 || args.size() > 4) CYPHER_ARGUMENT_ERROR();
    auto start = args[1].Evaluate(ctx, record);
    auto end = args[2].Evaluate(ctx, record);
    int64_t step_i = 1;
    if (args.size() == 4) {
        auto step = args[3].Evaluate(ctx, record);
        if (!step.IsInteger()) CYPHER_ARGUMENT_ERROR();
        step_i = step.constant.scalar.integer();
    }
    if (!start.IsInteger() || !end.IsInteger()) CYPHER_ARGUMENT_ERROR();
    auto start_i = start.constant.scalar.integer();
    auto end_i = end.constant.scalar.integer();
    cypher::FieldData ret = cypher::FieldData::Array(0);
    for (auto i = start_i; i <= end_i; i += step_i) {
        ret.array->emplace_back(i);
    }
    return ret;
}

cypher::FieldData BuiltinFunction::Subscript(RTContext *ctx, const Record &record,
                                             const std::vector<cypher::ArithExprNode> &args) {
    /* Arguments:
     * expr_node
     * start_idx
     * end_idx
     */
    if (args.size() < 3 || args.size() > 4) CYPHER_ARGUMENT_ERROR();
    auto list = args[1].Evaluate(ctx, record);
    auto start = args[2].Evaluate(ctx, record);
    if (!list.IsArray() || !start.IsInteger()) CYPHER_ARGUMENT_ERROR();
    auto len = list.constant.array->size();
    if (args.size() == 3) {
        auto start_i = start.constant.scalar.integer();
        start_i = start_i < 0 ? start_i + len : start_i;
        if (start_i < 0 || (size_t)start_i >= len) return cypher::FieldData();
        return cypher::FieldData(list.constant.array->at(start_i));
    } else if (args.size() == 4) {
        auto end = args[3].Evaluate(ctx, record);
        auto start_i = start.constant.scalar.integer();
        auto end_i = end.constant.scalar.integer();
        start_i = start_i < 0 ? start_i + len : start_i;
        end_i = end_i < 0 ? end_i + len : end_i;
        if (start_i < 0 || (size_t)start_i >= len || end_i < 0 || (size_t)end_i >= len ||
            start_i >= end_i) {
            return cypher::FieldData::Array(0);
        }
        cypher::FieldData ret = cypher::FieldData::Array(0);
        for (auto i = start_i; i < end_i; i++) {
            ret.array->emplace_back(list.constant.array->at(i));
        }
        return ret;
    }
    CYPHER_ARGUMENT_ERROR();
}

cypher::FieldData BuiltinFunction::IsAsc(
    RTContext *ctx, const Record &record,
    const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    if (r.IsArray()) {
        for (size_t i = 1; i < r.constant.array->size(); i++) {
            if ((*r.constant.array)[i] <= (*r.constant.array)[i-1]) {
                return cypher::FieldData(lgraph::FieldData(false));
            }
        }
        return cypher::FieldData(lgraph::FieldData(true));
    }
    CYPHER_ARGUMENT_ERROR();
}

cypher::FieldData BuiltinFunction::IsDesc(
    RTContext *ctx, const Record &record,
    const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    if (r.IsArray()) {
        for (size_t i = 1; i < r.constant.array->size(); i++) {
            if ((*r.constant.array)[i] >= (*r.constant.array)[i-1]) {
                return cypher::FieldData(lgraph::FieldData(false));
            }
        }
        return cypher::FieldData(lgraph::FieldData(true));
    }
    CYPHER_ARGUMENT_ERROR();
}

cypher::FieldData BuiltinFunction::HasDuplicates(RTContext *ctx, const Record &record,
                                                 const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    if (r.IsArray()) {
        std::unordered_set<int64_t> s;
        for (auto item : *r.constant.array) {
            if (!item.IsInteger()) {
                CYPHER_ARGUMENT_ERROR();
            }
            if (s.find(item.AsInt64()) != s.end()) {
                return cypher::FieldData(lgraph::FieldData(true));
            }
            s.emplace(item.AsInt64());
        }
        return cypher::FieldData(lgraph::FieldData(false));
    }
    CYPHER_ARGUMENT_ERROR();
}

cypher::FieldData BuiltinFunction::GetMemberProp(RTContext *ctx, const Record &record,
                                                 const std::vector<ArithExprNode> &args) {
    if (args.empty() || args.size() > 3) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto ret = cypher::FieldData::Array(0);
    std::string field_name;
    if (args.size() == 3) {
        auto field_name_entry = args[2].Evaluate(ctx, record);
        if (field_name_entry.IsString()) {
            field_name = field_name_entry.constant.scalar.AsString();
        } else {
            CYPHER_ARGUMENT_ERROR();
        }
    } else {
        CYPHER_ARGUMENT_ERROR();
    }

    const auto &entry = args[1].Evaluate(ctx, record);
    if (entry.type == Entry::VAR_LEN_RELP) {
        for (auto &eit : entry.relationship->ItsRef()) {
            if (eit.IsValid()) {
                ret.array->emplace_back(lgraph::FieldData(eit.GetField(field_name)));
            }
        }
    } else if (entry.type == Entry::CONSTANT && entry.constant.IsArray()) {
        for (auto &item : *entry.constant.array) {
            // TODO(huangke): support native nodes in FieldData
            if (item.IsInteger()) {
                auto vit = ctx->txn_->GetVertexIterator();
                vit.Goto(item.AsInt64());
                if (vit.IsValid()) {
                    ret.array->emplace_back(lgraph::FieldData(vit.GetField(field_name)));
                } else {
                    ret.array->emplace_back(lgraph::FieldData());
                }
            } else {
                ret.array->emplace_back(lgraph::FieldData());
            }
        }
    } else {
        CYPHER_ARGUMENT_ERROR();
    }
    return ret;
}

cypher::FieldData BuiltinFunction::Abs(RTContext *ctx, const Record &record,
                                       const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_CONSTANT(r);
    if (r.constant.IsInteger()) {
        return r.constant.scalar.integer() < 0
                   ? cypher::FieldData(lgraph::FieldData(-r.constant.scalar.integer()))
                   : r.constant;
    } else if (r.constant.IsReal()) {
        return r.constant.scalar.real() < 0
                   ? cypher::FieldData(lgraph::FieldData(-r.constant.scalar.real()))
                   : r.constant;
    }
    CYPHER_ARGUMENT_ERROR();
}

cypher::FieldData BuiltinFunction::Ceil(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_CONSTANT(r);
    if (r.constant.IsInteger()) {
        return r.constant;
    } else if (r.constant.IsReal()) {
        return cypher::FieldData(lgraph::FieldData(std::ceil(r.constant.scalar.real())));
    }
    CYPHER_ARGUMENT_ERROR();
}

cypher::FieldData BuiltinFunction::Floor(RTContext *ctx, const Record &record,
                                         const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_CONSTANT(r);
    if (r.constant.IsInteger()) {
        return r.constant;
    } else if (r.constant.IsReal()) {
        return cypher::FieldData(lgraph::FieldData(std::floor(r.constant.scalar.real())));
    }
    CYPHER_ARGUMENT_ERROR();
}

cypher::FieldData BuiltinFunction::Rand(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args) {
    if (args.size() != 1) CYPHER_ARGUMENT_ERROR();
    std::random_device rd;                        // device
    std::mt19937 rng(rd());                       // engine
    std::uniform_real_distribution<> dist(0, 1);  // distribution
    return cypher::FieldData(lgraph::FieldData(dist(rng)));
}

cypher::FieldData BuiltinFunction::Round(RTContext *ctx, const Record &record,
                                         const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_CONSTANT(r);
    if (r.constant.IsInteger()) {
        return r.constant;
    } else if (r.constant.IsReal()) {
        return cypher::FieldData(lgraph::FieldData(std::round(r.constant.scalar.real())));
    }
    CYPHER_ARGUMENT_ERROR();
}

cypher::FieldData BuiltinFunction::Sign(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    CHECK_CONSTANT(r);
    if (r.constant.IsInteger()) {
#ifdef _WIN32
        // workaround vs library bug
        int8_t sign = std::signbit((double)r.constant.scalar.integer()) ? -1 : 1;
#else
        int8_t sign = std::signbit(r.constant.scalar.integer()) ? -1 : 1;
#endif
        return cypher::FieldData(lgraph::FieldData(sign));
    } else if (r.constant.IsReal()) {
        int8_t sign = std::signbit(r.constant.scalar.real()) ? -1 : 1;
        return cypher::FieldData(lgraph::FieldData(sign));
    }
    CYPHER_ARGUMENT_ERROR();
}

cypher::FieldData BuiltinFunction::ToBoolean(RTContext *ctx, const Record &record,
                                             const std::vector<cypher::ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    switch (r.type) {
    case Entry::CONSTANT:
        if (r.constant.IsBool()) {
            return r.constant;
        } else if (r.constant.IsString()) {
            std::string d = r.constant.scalar.string();
            if (d == "True" || d == "true") return cypher::FieldData(lgraph::FieldData(true));
            if (d == "False" || d == "false") return cypher::FieldData(lgraph::FieldData(false));
        }
    default:
        break;
    }
    CYPHER_ARGUMENT_ERROR();
}

cypher::FieldData BuiltinFunction::ToFloat(RTContext *ctx, const Record &record,
                                           const std::vector<cypher::ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    switch (r.type) {
    case Entry::CONSTANT:
        if (r.constant.IsInteger()) {
            return cypher::FieldData(
                lgraph::FieldData(static_cast<double>(r.constant.scalar.integer())));
        }
        if (r.constant.IsReal()) {
            return r.constant;
        }
        if (r.constant.IsString()) {
            try {
                return cypher::FieldData(lgraph::FieldData(std::stod(r.constant.scalar.string())));
            } catch (std::exception &e) {
                LOG_WARN() << e.what();
                break;
            }
        }
    default:
        break;
    }
    CYPHER_ARGUMENT_ERROR();
}

cypher::FieldData BuiltinFunction::ToString(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    switch (r.type) {
    case Entry::CONSTANT:
        if (r.constant.IsString()) {
            return r.constant;
        }
        if (r.constant.IsInteger()) {
            try {
                return cypher::FieldData(
                    lgraph::FieldData(std::to_string(r.constant.scalar.integer())));
            } catch (std::exception &e) {
                LOG_WARN() << e.what();
                break;
            }
        }
        if (r.constant.IsReal()) {
            try {
                return cypher::FieldData(
                    lgraph::FieldData(std::to_string(r.constant.scalar.real())));
            } catch (std::exception &e) {
                LOG_WARN() << e.what();
                break;
            }
        }
        if (r.constant.IsBool()) {
            bool d = r.constant.scalar.AsBool();
            if (d)
                return cypher::FieldData(lgraph::FieldData("true"));
            else
                return cypher::FieldData(lgraph::FieldData("false"));
        }
    default:
        break;
    }
    CYPHER_ARGUMENT_ERROR();
}

cypher::FieldData BuiltinFunction::ToInteger(RTContext *ctx, const Record &record,
                                             const std::vector<cypher::ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto operand = args[1];
    auto r = operand.Evaluate(ctx, record);
    switch (r.type) {
    case Entry::CONSTANT:
        if (r.constant.IsInteger()) {
            return r.constant;
        }
        if (r.constant.IsReal()) {
            return cypher::FieldData(
                lgraph::FieldData(static_cast<int64_t>(r.constant.scalar.real())));
        }
        if (r.constant.IsString()) {
            try {
                return cypher::FieldData(lgraph::FieldData(
                    static_cast<int64_t>(std::stoll(r.constant.scalar.string()))));
            } catch (std::exception &e) {
                LOG_WARN() << e.what();
                break;
            }
        }
    default:
        break;
    }
    CYPHER_ARGUMENT_ERROR();
}

cypher::FieldData BuiltinFunction::Date(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() > 2) CYPHER_ARGUMENT_ERROR();
    if (args.size() == 1) {
        // date() Returns the current date;
        return cypher::FieldData(::lgraph::FieldData(::lgraph::Date::Now()));
    } else {
        CYPHER_THROW_ASSERT(args.size() == 2);
        // date(string) Returns a Date by parsing a string.
        auto r = args[1].Evaluate(ctx, record);
        if (!r.IsString()) CYPHER_ARGUMENT_ERROR();
        auto dt = ::lgraph::FieldData::Date(r.constant.scalar.AsString());
        return cypher::FieldData(dt);
    }
}

cypher::FieldData BuiltinFunction::DateComponent(RTContext *ctx, const Record &record,
                                                 const std::vector<ArithExprNode> &args) {
    if (args.size() != 3) CYPHER_ARGUMENT_ERROR();
    static const int64_t MAX_DAYS_EPOCH = 2000000;
    static const std::unordered_map<std::string, int> COMPONENT_MAP{
        {"year", 0}, {"month", 1}, {"day", 2}
    };
    auto days_stamp = args[1].Evaluate(ctx, record);
    auto component = args[2].Evaluate(ctx, record);

    if (!days_stamp.IsInteger() || !component.IsString()) CYPHER_ARGUMENT_ERROR();
    auto days_epoch = days_stamp.constant.scalar.AsInt64();
    if (days_epoch > MAX_DAYS_EPOCH) days_epoch /= 365;

    auto d = lgraph::Date(static_cast<int32_t>(days_epoch));
    auto YMD = d.GetYearMonthDay();
    auto it = COMPONENT_MAP.find(component.constant.scalar.AsString());
    if (it == COMPONENT_MAP.end())
        THROW_CODE(InputError, "Invalid input: " + component.constant.scalar.AsString());
    switch (it->second) {
    case 0:
        return cypher::FieldData(::lgraph::FieldData(YMD.year));
    case 1:
        return cypher::FieldData(::lgraph::FieldData(static_cast<int64_t>(YMD.month)));
    case 2:
        return cypher::FieldData(::lgraph::FieldData(static_cast<int64_t>(YMD.day)));
    default:
        THROW_CODE(InternalError);
    }
}

cypher::FieldData BuiltinFunction::DateTime(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() > 2) CYPHER_ARGUMENT_ERROR();
    if (args.size() == 1) {
        // datetime() Returns the current DateTime.
        return cypher::FieldData(::lgraph::FieldData(::lgraph::DateTime::Now()));
    } else {
        CYPHER_THROW_ASSERT(args.size() == 2);
        // datetime(string) Returns a DateTime by parsing a string.
        auto r = args[1].Evaluate(ctx, record);
        if (!r.IsString()) CYPHER_ARGUMENT_ERROR();
        auto dt = ::lgraph::FieldData::DateTime(r.constant.scalar.AsString());
        return cypher::FieldData(dt);
    }
}

/* TODO(anyone) Consider the following 2 style:
 * 1.
 * RETURN datetimeComponent({epochMillis:1347062400000, component:year}),
 * datetimeComponent({epochMillis:1347062400, component:day}) 2. WITH datetime({ year:1984,
 * month:11, day:11, hour:12, minute:31, second:14, nanosecond: 645876123,
 * timezone:'Europe/Stockholm' }) AS d RETURN d.year, d.quarter, d.month, d.week, d.weekYear, d.day,
 * d.ordinalDay, d.dayOfWeek, d.dayOfQuarter, d.hour, d.minute, d.second, d.millisecond,
 * d.microsecond, d.nanosecond, d.timezone, d.offset, d.offsetMinutes, d.epochSeconds, d.epochMillis
 */
cypher::FieldData BuiltinFunction::DateTimeComponent(RTContext *ctx, const Record &record,
                                                     const std::vector<ArithExprNode> &args) {
    if (args.size() != 3) CYPHER_ARGUMENT_ERROR();
    static const int64_t MAX_MICROSECONDS_EPOCH = 100000000000000000;  // 5138-11-16 09:46:40
    static const std::unordered_map<std::string, int> COMPONENT_MAP{
        {"year", 0}, {"month", 1}, {"day", 2}, {"hour", 3}, {"minute", 4}, {"second", 5},
        {"microsecond", 6}
    };
    auto time_stamp = args[1].Evaluate(ctx, record);
    auto component = args[2].Evaluate(ctx, record);
    if (!time_stamp.IsInteger() || !component.IsString()) CYPHER_ARGUMENT_ERROR();
    auto microseconds_epoch = time_stamp.constant.scalar.AsInt64();
    if (microseconds_epoch > MAX_MICROSECONDS_EPOCH) microseconds_epoch /= 1000;
    auto dt = lgraph::DateTime(microseconds_epoch);
    auto ymdhmsf = dt.GetYMDHMSF();
    auto it = COMPONENT_MAP.find(component.constant.scalar.AsString());
    if (it == COMPONENT_MAP.end())
        THROW_CODE(InputError, "Invalid input: " + component.constant.scalar.AsString());
    switch (it->second) {
    case 0:
        return cypher::FieldData(::lgraph::FieldData(ymdhmsf.year));
    case 1:
        return cypher::FieldData(::lgraph::FieldData(static_cast<int64_t>(ymdhmsf.month)));
    case 2:
        return cypher::FieldData(::lgraph::FieldData(static_cast<int64_t>(ymdhmsf.day)));
    case 3:
        return cypher::FieldData(::lgraph::FieldData(static_cast<int64_t>(ymdhmsf.hour)));
    case 4:
        return cypher::FieldData(::lgraph::FieldData(static_cast<int64_t>(ymdhmsf.minute)));
    case 5:
        return cypher::FieldData(::lgraph::FieldData(static_cast<int64_t>(ymdhmsf.second)));
    case 6:
        return cypher::FieldData(::lgraph::FieldData(static_cast<int64_t>(ymdhmsf.fraction)));
    default:
        THROW_CODE(InternalError);
    }
}

cypher::FieldData BuiltinFunction::SubString(RTContext *ctx, const Record &record,
                                             const std::vector<ArithExprNode> &args) {
    /* Arguments:
     * start    An expression that returns an integer value.
     * length   An expression that returns an integer value.
     */
    if (args.size() != 4) CYPHER_ARGUMENT_ERROR();

    auto arg1 = args[1].Evaluate(ctx, record);
    switch (arg1.type) {
        case Entry::CONSTANT:
            if (arg1.constant.IsString()) {
                auto arg2 = args[2].Evaluate(ctx, record);
                auto arg3 = args[3].Evaluate(ctx, record);
                if (!arg2.IsInteger())
                    throw lgraph::CypherException("Argument 2 of `SUBSTRING()` expects int type: "
                                                  + arg2.ToString());
                if (!arg3.IsInteger())
                    throw lgraph::CypherException("Argument 3 of `SUBSTRING()` expects int type: "
                                                  + arg3.ToString());

                auto origin = arg1.constant.ToString("");
                auto size = static_cast<int64_t>(origin.length());
                auto start = arg2.constant.scalar.integer();
                auto length = arg3.constant.scalar.integer();
                if (start < 1 || start > size)
                    throw lgraph::CypherException("Invalid argument 2 of `SUBSTRING()`: "
                                                  + arg2.ToString());
                if (length < 1 || start - 1 + length > size)
                    throw lgraph::CypherException("Invalid argument 3 of `SUBSTRING()`: "
                                                  + arg3.ToString());

                auto result = origin.substr(start - 1, length);
                return cypher::FieldData(lgraph::FieldData(result));
            }
            break;
        default:
            break;
    }

    throw lgraph::CypherException("Function `SUBSTRING()` is not supported for: "
                                  + arg1.ToString());
}

cypher::FieldData BuiltinFunction::Mask(RTContext *ctx, const Record &record,
                                        const std::vector<ArithExprNode> &args) {
    /* Arguments:
     * start    An expression that returns an integer value.
     * end   An expression that returns an integer value.
     * mask_char An expression that returns a char value.
     */
    if (args.size() != 4 && args.size() != 5) CYPHER_ARGUMENT_ERROR();
    auto extractChineseCharacters = [](const std::string& text) {
        auto isUtf8StartByte = [](unsigned char c) {
            return (c & 0xC0) != 0x80;
        };
        std::vector<std::string> characters;
        std::string currentChar;
        for (unsigned char c : text) {
            if (isUtf8StartByte(c)) {
                if (!currentChar.empty()) {
                    characters.push_back(currentChar);
                    currentChar.clear();
                }
            }
            currentChar += c;
        }
        if (!currentChar.empty()) {
            characters.push_back(currentChar);
        }
        return characters;
    };
    auto arg1 = args[1].Evaluate(ctx, record);
    switch (arg1.type) {
        case Entry::CONSTANT:
            if (arg1.constant.IsString()) {
                auto arg2 = args[2].Evaluate(ctx, record);
                auto arg3 = args[3].Evaluate(ctx, record);
                if (!arg2.IsInteger())
                    throw lgraph::CypherException("Argument 2 of `MASK()` expects int type: "
                                                  + arg2.ToString());
                if (!arg3.IsInteger())
                    throw lgraph::CypherException("Argument 3 of `MASK()` expects int type: "
                                                  + arg3.ToString());

                std::string ss = "*";
                if (args.size() == 5) {
                    auto arg4 = args[4].Evaluate(ctx, record);
                    ss = arg4.constant.ToString("");
                }
                auto origin = arg1.constant.ToString("");
                auto origin_strings = extractChineseCharacters(origin);
                auto size = static_cast<int64_t>(origin_strings.size());
                auto start = arg2.constant.scalar.integer();
                auto end = arg3.constant.scalar.integer();
                if (start < 1 || start > size)
                    throw lgraph::CypherException("Invalid argument 2 of `MASK()`: "
                                                  + arg2.ToString());
                if (end < start || end > size)
                    throw lgraph::CypherException("Invalid argument 3 of `MASK()`: "
                                                  + arg3.ToString());

                std::string result;
                for (int i = 0; i < start - 1; ++i)
                    result.append(origin_strings[i]);
                for (int i = start; i <= end; i++)
                    result.append(ss);
                for (int i = end; i < size; ++i)
                    result.append(origin_strings[i]);
                return cypher::FieldData(lgraph::FieldData(result));
            }
            break;
    default:
            break;
    }

    throw lgraph::CypherException("Function `MASK()` is not supported for: "
                                  + arg1.ToString());
}

cypher::FieldData BuiltinFunction::Concat(RTContext *ctx, const Record &record,
                                             const std::vector<ArithExprNode> &args) {
    /* Arguments:
     * others[1...n]    At least one expression that returns a string value.
     */
    if (args.size() < 3) CYPHER_ARGUMENT_ERROR();

    auto arg1 = args[1].Evaluate(ctx, record);
    switch (arg1.type) {
        case Entry::CONSTANT:
            if (arg1.constant.IsString()) {
                auto result = arg1.constant.ToString("");

                for (int i = 2; i < (int)args.size(); ++i) {
                    auto arg = args[i].Evaluate(ctx, record);
                    if (!arg.IsString())
                        throw lgraph::CypherException("Argument " + std::to_string(i)
                                                      + " of `CONCAT()` expects string type: "
                                                      + arg.ToString());

                    result.append(arg.constant.ToString(""));
                }

                return cypher::FieldData(lgraph::FieldData(result));
            }
            break;
        default:
            break;
    }

    throw lgraph::CypherException("Function `CONCAT()` is not supported for: " + arg1.ToString());
}

cypher::FieldData BuiltinFunction::Point(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() == 1 || args.size() > 4) CYPHER_ARGUMENT_ERROR();
    CYPHER_THROW_ASSERT(args.size() >= 2);
    // Point(string) Returns a Point by parsing a string.
    auto arg1 = args[1].Evaluate(ctx, record);
    if (args.size() == 2) {
        if (!arg1.IsString()) CYPHER_ARGUMENT_ERROR();
        auto pt = ::lgraph::FieldData::Point(arg1.constant.scalar.AsString());
        return cypher::FieldData(pt);
    }
    auto arg2 = args[2].Evaluate(ctx, record);
    lgraph_api::SRID s;
    double arg1_, arg2_;
    if (arg1.IsInteger())
        arg1_ = static_cast<double>(arg1.constant.scalar.AsInt64());
    else if (arg1.IsReal())
        arg1_ = arg1.constant.scalar.AsDouble();
    else
        CYPHER_ARGUMENT_ERROR();

    if (arg2.IsInteger())
        arg2_ = static_cast<double>(arg2.constant.scalar.AsInt64());
    else if (arg2.IsReal())
        arg2_ = arg2.constant.scalar.AsDouble();
    else
        CYPHER_ARGUMENT_ERROR();

    // return point(2.32, 4.96)  srid = 4326 by default;
    if (args.size() == 3) {
        s = lgraph_api::SRID::WGS84;
        auto pt = ::lgraph::FieldData::Point(lgraph_api::Point<lgraph_api::Wgs84>
                (arg1_, arg2_, s));
        return cypher::FieldData(pt);
    }

    // return point(arg1, arg2, 7203);
    auto arg3 = args[3].Evaluate(ctx, record);
    if (!arg3.IsInteger())
        CYPHER_ARGUMENT_ERROR();

    switch (arg3.constant.scalar.AsInt64()) {
        case 4326:
            s = lgraph_api::SRID::WGS84;
            return cypher::FieldData(::lgraph::FieldData::Point
            (lgraph_api::Point<lgraph_api::Wgs84>(arg1_, arg2_, s)));
        case 7203:
            s = lgraph_api::SRID::CARTESIAN;
            return cypher::FieldData(::lgraph::FieldData::Point
            (lgraph_api::Point<lgraph_api::Cartesian>(arg1_, arg2_, s)));
        default:
            CYPHER_ARGUMENT_ERROR();
    }
}

cypher::FieldData BuiltinFunction::PointWKB(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() > 3) CYPHER_ARGUMENT_ERROR();
    CYPHER_THROW_ASSERT(args.size() <= 3);
    auto wkb = args[1].Evaluate(ctx, record);
    if (!wkb.IsString()) CYPHER_ARGUMENT_ERROR();

    lgraph_api::SRID s = lgraph_api::SRID::WGS84;
    lgraph_api::SpatialType t = lgraph_api::SpatialType::POINT;
    std::string wkb_ =  wkb.constant.scalar.AsString();
    if (args.size() == 2) {
        auto pt = ::lgraph::FieldData::Point(lgraph_api::Point<lgraph_api::Wgs84>
                (s, t, 0, wkb_));
        return cypher::FieldData(pt);
    }

    auto srid = args[2].Evaluate(ctx, record);
    if (!srid.IsInteger()) CYPHER_ARGUMENT_ERROR();
    switch (srid.constant.scalar.AsInt64()) {
        case 4326:
            return cypher::FieldData(::lgraph::FieldData::Point(
                lgraph_api::Point<lgraph_api::Wgs84>(s, t, 0, wkb_)));
        case 7203:
            s = lgraph_api::SRID::CARTESIAN;
            return cypher::FieldData(::lgraph::FieldData::Point(
                lgraph_api::Point<lgraph_api::Cartesian>(s, t, 0, wkb_)));
        default:
            CYPHER_ARGUMENT_ERROR();
    }
}

cypher::FieldData BuiltinFunction::PointWKT(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() > 3) CYPHER_ARGUMENT_ERROR();
    CYPHER_THROW_ASSERT(args.size() <= 3);
    auto wkt = args[1].Evaluate(ctx, record);
    if (!wkt.IsString()) CYPHER_ARGUMENT_ERROR();

    lgraph_api::SRID s = lgraph_api::SRID::WGS84;
    lgraph_api::SpatialType t = lgraph_api::SpatialType::POINT;
    std::string wkt_ = wkt.constant.scalar.AsString();
    if (args.size() == 2) {
        auto pt = ::lgraph::FieldData::Point(lgraph_api::Point<lgraph_api::Wgs84>
                (s, t, 1, wkt_));
        return cypher::FieldData(pt);
    }

    auto srid = args[2].Evaluate(ctx, record);
    if (!srid.IsInteger()) CYPHER_ARGUMENT_ERROR();
    switch (srid.constant.scalar.AsInt64()) {
        case 4326:
            return cypher::FieldData(::lgraph::FieldData::Point(
                lgraph_api::Point<lgraph_api::Wgs84>(s, t, 1, wkt_)));
        case 7203:
            s = lgraph_api::SRID::CARTESIAN;
            return cypher::FieldData(::lgraph::FieldData::Point(
                lgraph_api::Point<lgraph_api::Cartesian>(s, t, 1, wkt_)));
        default:
            CYPHER_ARGUMENT_ERROR();
    }
}

cypher::FieldData BuiltinFunction::LineString(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    CYPHER_THROW_ASSERT(args.size() == 2);
    // LineString(string) Returns a LineString by parsing a string.
    auto r = args[1].Evaluate(ctx, record);
    if (!r.IsString()) CYPHER_ARGUMENT_ERROR();
    auto l = ::lgraph::FieldData::LineString(r.constant.scalar.AsString());
    return cypher::FieldData(l);
}

cypher::FieldData BuiltinFunction::LineStringWKB(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() > 3) CYPHER_ARGUMENT_ERROR();
    CYPHER_THROW_ASSERT(args.size() <= 3);
    auto wkb = args[1].Evaluate(ctx, record);
    if (!wkb.IsString()) CYPHER_ARGUMENT_ERROR();

    lgraph_api::SRID s = lgraph_api::SRID::WGS84;
    lgraph_api::SpatialType t = lgraph_api::SpatialType::LINESTRING;
    std::string wkb_ = wkb.constant.scalar.AsString();
    if (args.size() == 2) {
        auto pt = ::lgraph::FieldData::LineString(lgraph_api::LineString<lgraph_api::Wgs84>
                (s, t, 0, wkb_));
        return cypher::FieldData(pt);
    }

    auto srid = args[2].Evaluate(ctx, record);
    if (!srid.IsInteger()) CYPHER_ARGUMENT_ERROR();
    switch (srid.constant.scalar.AsInt64()) {
        case 4326:
            return cypher::FieldData(::lgraph::FieldData::LineString(
                lgraph_api::LineString<lgraph_api::Wgs84>
                (s, t, 0, wkb_)));
        case 7203:
            s = lgraph_api::SRID::CARTESIAN;
            return cypher::FieldData(::lgraph::FieldData::LineString(
                lgraph_api::LineString<lgraph_api::Cartesian>
                (s, t, 0, wkb_)));
        default:
            CYPHER_ARGUMENT_ERROR();
    }
}

cypher::FieldData BuiltinFunction::LineStringWKT(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() > 3) CYPHER_ARGUMENT_ERROR();
    CYPHER_THROW_ASSERT(args.size() <= 3);
    auto wkt = args[1].Evaluate(ctx, record);
    if (!wkt.IsString()) CYPHER_ARGUMENT_ERROR();

    lgraph_api::SRID s = lgraph_api::SRID::WGS84;
    lgraph_api::SpatialType t = lgraph_api::SpatialType::LINESTRING;
    std::string wkt_ = wkt.constant.scalar.AsString();
    if (args.size() == 2) {
        auto pt = ::lgraph::FieldData::LineString(lgraph_api::LineString<lgraph_api::Wgs84>
                (s, t, 1, wkt_));
        return cypher::FieldData(pt);
    }

    auto srid = args[2].Evaluate(ctx, record);
    if (!srid.IsInteger()) CYPHER_ARGUMENT_ERROR();
    switch (srid.constant.scalar.AsInt64()) {
        case 4326:
            return cypher::FieldData(::lgraph::FieldData::LineString(
                lgraph_api::LineString<lgraph_api::Wgs84>
                (s, t, 1, wkt_)));
        case 7203:
            s = lgraph_api::SRID::CARTESIAN;
            return cypher::FieldData(::lgraph::FieldData::LineString(
                lgraph_api::LineString<lgraph_api::Cartesian>
                (s, t, 1, wkt_)));
        default:
            CYPHER_ARGUMENT_ERROR();
    }
}

cypher::FieldData BuiltinFunction::Polygon(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    CYPHER_THROW_ASSERT(args.size() == 2);
    // Polygon(string) Returns a Point by parsing a string.
    auto r = args[1].Evaluate(ctx, record);
    if (!r.IsString()) CYPHER_ARGUMENT_ERROR();
    auto pl = ::lgraph::FieldData::Polygon(r.constant.scalar.AsString());
    return cypher::FieldData(pl);
}

cypher::FieldData BuiltinFunction::PolygonWKB(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() > 3) CYPHER_ARGUMENT_ERROR();
    CYPHER_THROW_ASSERT(args.size() <= 3);
    auto wkb = args[1].Evaluate(ctx, record);
    if (!wkb.IsString()) CYPHER_ARGUMENT_ERROR();

    lgraph_api::SRID s = lgraph_api::SRID::WGS84;
    lgraph_api::SpatialType t = lgraph_api::SpatialType::POLYGON;
    std::string wkb_ = wkb.constant.scalar.AsString();
    if (args.size() == 2) {
        auto pt = ::lgraph::FieldData::Polygon(lgraph_api::Polygon<lgraph_api::Wgs84>
                (s, t, 0, wkb_));
        return cypher::FieldData(pt);
    }

    auto srid = args[2].Evaluate(ctx, record);
    if (!srid.IsInteger()) CYPHER_ARGUMENT_ERROR();
    switch (srid.constant.scalar.AsInt64()) {
        case 4326:
            return cypher::FieldData(::lgraph::FieldData::Polygon(
                lgraph_api::Polygon<lgraph_api::Wgs84>
                (s, t, 0, wkb_)));
        case 7203:
            s = lgraph_api::SRID::CARTESIAN;
            return cypher::FieldData(::lgraph::FieldData::Polygon(
                lgraph_api::Polygon<lgraph_api::Cartesian>
                (s, t, 0, wkb_)));
        default:
            CYPHER_ARGUMENT_ERROR();
    }
}

cypher::FieldData BuiltinFunction::PolygonWKT(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() > 3) CYPHER_ARGUMENT_ERROR();
    CYPHER_THROW_ASSERT(args.size() <= 3);
    auto wkt = args[1].Evaluate(ctx, record);
    if (!wkt.IsString()) CYPHER_ARGUMENT_ERROR();

    lgraph_api::SRID s = lgraph_api::SRID::WGS84;
    lgraph_api::SpatialType t = lgraph_api::SpatialType::LINESTRING;
    std::string wkt_ = wkt.constant.scalar.AsString();
    if (args.size() == 2) {
        auto pt = ::lgraph::FieldData::Polygon(lgraph_api::Polygon<lgraph_api::Wgs84>
                (s, t, 1, wkt_));
        return cypher::FieldData(pt);
    }

    auto srid = args[2].Evaluate(ctx, record);
    if (!srid.IsInteger()) CYPHER_ARGUMENT_ERROR();
    switch (srid.constant.scalar.AsInt64()) {
        case 4326:
            return cypher::FieldData(::lgraph::FieldData::Polygon(
                lgraph_api::Polygon<lgraph_api::Wgs84>
                (s, t, 1, wkt_)));
        case 7203:
            s = lgraph_api::SRID::CARTESIAN;
            return cypher::FieldData(::lgraph::FieldData::Polygon(
                lgraph_api::Polygon<lgraph_api::Cartesian>
                (s, t, 1, wkt_)));
        default:
            CYPHER_ARGUMENT_ERROR();
    }
}

cypher::FieldData BuiltinFunction::StartsWith(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() != 3) CYPHER_ARGUMENT_ERROR();
    auto prefix = args[1].Evaluate(ctx, record);
    std::string prefix_str = prefix.constant.scalar.AsString();
    auto variable = args[2].Evaluate(ctx, record);
    std::string variable_str = variable.constant.scalar.AsString();
    bool ret = variable_str.compare(0, prefix_str.size(), prefix_str) == 0;
    return cypher::FieldData(lgraph_api::FieldData(ret));
}

cypher::FieldData BuiltinFunction::EndsWith(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() != 3) CYPHER_ARGUMENT_ERROR();
    auto postfix = args[1].Evaluate(ctx, record);
    std::string postfix_str = postfix.constant.scalar.AsString();
    auto variable = args[2].Evaluate(ctx, record);
    std::string variable_str = variable.constant.scalar.AsString();
    bool ret = variable_str.size() >= postfix_str.size() &&
               variable_str.compare(variable_str.size() - postfix_str.size(),
               postfix_str.size(), postfix_str) == 0;
    return cypher::FieldData(lgraph_api::FieldData(ret));
}

cypher::FieldData BuiltinFunction::Contains(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() != 3) CYPHER_ARGUMENT_ERROR();
    auto substr = args[1].Evaluate(ctx, record);
    std::string substr_str = substr.constant.scalar.AsString();
    auto variable = args[2].Evaluate(ctx, record);
    std::string variable_str = variable.constant.scalar.AsString();
    bool ret = variable_str.find(substr_str) != std::string::npos;
    return cypher::FieldData(lgraph_api::FieldData(ret));
}

cypher::FieldData BuiltinFunction::Regexp(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() != 3) CYPHER_ARGUMENT_ERROR();
    auto regexp = args[1].Evaluate(ctx, record);
    std::string regexp_str = regexp.constant.scalar.AsString();
    auto variable = args[2].Evaluate(ctx, record);
    std::string variable_str = variable.constant.scalar.AsString();
    if (variable_str.empty()) return cypher::FieldData(lgraph_api::FieldData(false));
    bool ret = std::regex_match(variable_str, std::regex(regexp_str));
    return cypher::FieldData(lgraph_api::FieldData(ret));
}

cypher::FieldData BuiltinFunction::Exists(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    auto res = args[1].Evaluate(ctx, record);
    return cypher::FieldData(lgraph_api::FieldData(!res.IsNull()));
}

cypher::FieldData BuiltinFunction::Bin(RTContext *ctx, const Record &record,
                                       const std::vector<ArithExprNode> &args) {
    if (args.size() != 2) CYPHER_ARGUMENT_ERROR();
    auto r = args[1].Evaluate(ctx, record);
    if (!r.IsString()) CYPHER_ARGUMENT_ERROR();
    auto bin = ::lgraph::FieldData::BlobFromBase64(r.constant.scalar.AsString());
    return cypher::FieldData(bin);
}

cypher::FieldData BuiltinFunction::NativeGetEdgeField(RTContext *ctx, const Record &record,
                                                      const std::vector<ArithExprNode> &args) {
    if (args.size() != 3) CYPHER_ARGUMENT_ERROR();
    auto eid = args[1].Evaluate(ctx, record);
    auto field = args[2].Evaluate(ctx, record);
    if (!eid.IsString() || !field.IsString()) CYPHER_ARGUMENT_ERROR();
    // need transaction
    CYPHER_TODO();
    return cypher::FieldData();
}

const std::string BuiltinFunction::INTL_TO_PATH = "_to_path_";                        // NOLINT
const std::string BuiltinFunction::INTL_TO_LIST = "_to_list_";                        // NOLINT
const std::string BuiltinFunction::INTL_LIST_COMPREHENSION = "_list_comprehension_";  // NOLINT

/* Creates a path from a given sequence of graph entities.
 * The arguments are the sequence of graph entities combines the path.
 * The sequence is always in odd length and defined as:
 * Odd indices members are always representing the value of a single node.
 * Even indices members are either representing the value of a single edge,
 * or an sipath, in case of variable length traversal.  */
cypher::FieldData BuiltinFunction::_ToPath(RTContext *ctx, const Record &record,
                                           const std::vector<ArithExprNode> &args) {
    if (args.size() % 2 == 0) CYPHER_ARGUMENT_ERROR();
    auto ret = cypher::FieldData::Array(0);
    bool merge_next_node = false;
    for (auto &arg : args) {
        if (merge_next_node) {
            merge_next_node = false;
            continue;
        }
        auto r = arg.Evaluate(ctx, record);
        if (r.type == Entry::VAR_LEN_RELP) {
            auto &path = r.relationship->path_;
            merge_next_node = path.Length() == 0;
            for (int l = 0; l < (int)path.Length(); l++) {
                ret.array->emplace_back(std::string("E[")
                                            .append(_detail::EdgeUid2String(path.GetNthEdge(l)))
                                            .append("]"));
                if (l == (int)path.Length() - 1) break;
                ret.array->emplace_back(
                    std::string("V[").append(std::to_string(path.ids_[l * 2 + 2])).append("]"));
            }
        } else {
            ret.array->emplace_back(r.ToString());
        }
    }
    return ret;
}

cypher::FieldData BuiltinFunction::_ToList(RTContext *ctx, const Record &record,
                                           const std::vector<ArithExprNode> &args) {
    auto ret = cypher::FieldData::Array(0);
    for (auto &arg : args) {
        auto r = arg.Evaluate(ctx, record);
        // CYPHER_THROW_ASSERT(r.IsScalar());
        ret.array->emplace_back(r.constant.scalar);
    }
    return ret;
}

cypher::FieldData BuiltinFunction::_ListComprehension(RTContext *ctx, const Record &record,
                                                      const std::vector<ArithExprNode> &args) {
    auto ret = cypher::FieldData::Array(0);
    Record nested_record_for_list_comprehension;
    nested_record_for_list_comprehension.values = record.values;
    nested_record_for_list_comprehension.values.emplace_back(Entry());
    CYPHER_THROW_ASSERT(args.size() == 4);
    auto range = args[1].Evaluate(ctx, record);
    CYPHER_THROW_ASSERT(range.IsArray());
    for (auto &a : *range.constant.array) {
        nested_record_for_list_comprehension.values.back() = Entry(cypher::FieldData(a));
        auto v = args[3].Evaluate(ctx, nested_record_for_list_comprehension);
        ret.array->emplace_back(v.constant.scalar);
    }
    return ret;
}

cypher::FieldData BuiltinFunction::Coalesce(RTContext *ctx, const Record &record,
                                            const std::vector<ArithExprNode> &args) {
    if (args.size() == 1) CYPHER_ARGUMENT_ERROR();
    for (int i = 1; i < (int)args.size(); i++) {
        auto r = args[i].Evaluate(ctx, record);
        CHECK_CONSTANT(r);
        if (r.constant.scalar.type != lgraph_api::NUL) {
            return r.constant;
        }
    }
    return cypher::FieldData();
}

ArithOperandNode::ArithOperandNode(const parser::Expression &expr, const SymbolTable &sym_tab) {
    Set(expr, sym_tab);
}

void ArithOperandNode::SetEntity(const std::string &alias, const SymbolTable &sym_tab) {
    type = AR_OPERAND_VARIADIC;
    variadic.alias = alias;
    auto it = sym_tab.symbols.find(alias);
    if (it == sym_tab.symbols.end()) {
        THROW_CODE(InputError,
            "Variable `{}` not defined", alias);
    }
    // CYPHER_THROW_ASSERT(it != sym_tab.symbols.end());
    variadic.alias_idx = it->second.id;
}

void ArithOperandNode::SetEntity(const std::string &alias, const std::string &property,
                                 const SymbolTable &sym_tab) {
    SetEntity(alias, sym_tab);
    variadic.entity_prop = property;
}

void ArithOperandNode::SetParameter(const std::string &param, const SymbolTable &sym_tab) {
    type = AR_OPERAND_PARAMETER;
    variadic.alias = param;
    auto it = sym_tab.symbols.find(param);
    if (it == sym_tab.symbols.end()) {
        throw lgraph::CypherException("Parameter not defined: " + param);
    }
    variadic.alias_idx = it->second.id;
}

void ArithOperandNode::RealignAliasId(const SymbolTable &sym_tab) {
    if (type == AR_OPERAND_CONSTANT) return;
    auto it = sym_tab.symbols.find(variadic.alias);
    CYPHER_THROW_ASSERT(it != sym_tab.symbols.end());
    variadic.alias_idx = it->second.id;
}

cypher::FieldData GenerateCypherFieldData(const parser::Expression &expr) {
    if (expr.type != parser::Expression::LIST && expr.type != parser::Expression::MAP) {
        return cypher::FieldData(parser::MakeFieldData(expr));
    }
    if (expr.type == parser::Expression::LIST) {
        std::vector<cypher::FieldData> list;
        for (auto &e : expr.List()) list.emplace_back(GenerateCypherFieldData(e));
        return cypher::FieldData(std::move(list));
    } else {
        std::unordered_map<std::string, cypher::FieldData> map;
        for (auto &e : expr.Map()) map.emplace(e.first, GenerateCypherFieldData(e.second));
        return cypher::FieldData(std::move(map));
    }
}

void ArithOperandNode::Set(const parser::Expression &expr, const SymbolTable &sym_tab) {
    switch (expr.type) {
    case parser::Expression::VARIABLE:
        /* e.g.
         * 1. node/relationship: n, r
         * 2. constant value: count(*) AS f WHERE f > 1 */
        SetEntity(expr.String(), sym_tab);
        break;
    case parser::Expression::PROPERTY:
        {
            /* e.g. n.name */
            auto &alias = expr.Property().first;
            auto &property = expr.Property().second;
            CYPHER_THROW_ASSERT(alias.type == parser::Expression::VARIABLE);
            SetEntity(alias.String(), property, sym_tab);
            break;
        }
    case parser::Expression::BOOL:
    case parser::Expression::INT:
    case parser::Expression::DOUBLE:
    case parser::Expression::STRING:
        /* e.g. -2, 3.141592, 'im a string' etc */
        type = ArithOperandNode::AR_OPERAND_CONSTANT;
        constant = parser::MakeFieldData(expr);
        break;
    case parser::Expression::PARAMETER:
        /* e.g. $name */
        SetParameter(expr.String(), sym_tab);
        break;
    case parser::Expression::NULL_:
        /* e.g. CASE r WHEN null THEN false */
        type = ArithOperandNode::AR_OPERAND_CONSTANT;
        // constant.type = lgraph::FieldData::NUL;
        break;
    case parser::Expression::LIST:
        {
            /* e.g. [1,3,5,7], [1,3,5.55,'seven'] */
            type = ArithOperandNode::AR_OPERAND_CONSTANT;
            constant = GenerateCypherFieldData(expr);
            break;
        }
    case parser::Expression::MAP:
        {
            type = ArithOperandNode::AR_OPERAND_CONSTANT;
            constant = GenerateCypherFieldData(expr);
            break;
        }
    default:
        CYPHER_TODO();
    }
}

ArithExprNode::ArithExprNode(const parser::Expression &expr, const SymbolTable &sym_tab) {
    expression_ = expr;
    Set(expression_, sym_tab);
}

void ArithExprNode::SetOperand(ArithOperandNode::ArithOperandType operand_type,
                               const std::string &opd, const SymbolTable &sym_tab) {
    if (operand_type == ArithOperandNode::AR_OPERAND_VARIADIC) {
        operand.SetEntity(opd, sym_tab);
    } else if (operand_type == ArithOperandNode::AR_OPERAND_PARAMETER) {
        operand.SetParameter(opd, sym_tab);
    } else {
        throw lgraph::CypherException(std::string(__func__) + ": operand type error");
    }
    type = AR_EXP_OPERAND;
}

void ArithExprNode::SetOperand(ArithOperandNode::ArithOperandType operand_type,
                               const std::string &alias, const std::string &property,
                               const SymbolTable &sym_tab) {
    SetOperand(operand_type, alias, property);
    CYPHER_THROW_ASSERT(sym_tab.symbols.find(alias) != sym_tab.symbols.end());
    operand.variadic.alias_idx = sym_tab.symbols.find(alias)->second.id;
}

void ArithExprNode::Set(const parser::Expression &expr, const SymbolTable &sym_tab) {
    switch (expr.type) {
    case parser::Expression::FUNCTION:
        {
            /* e.g. id(n) */
            type = AR_EXP_OP;
            auto &list = expr.List();
            CYPHER_THROW_ASSERT(list.at(0).type == parser::Expression::STRING);
            op.SetFunc(list.at(0).String());
            for (int i = 1; i < (int)list.size(); i++) {
                auto &e = list.at(i);
                ArithExprNode child;
                child.Set(e, sym_tab);
                op.children.emplace_back(child);
            }
            break;
        }
    case parser::Expression::CASE:
        {
            /* e.g. CASE WHEN xx THEN xx END */
            type = AR_EXP_OP;
            op.type = ArithOpNode::AR_OP_CASE;
            auto &case_expr = expr.Case();
            ArithExprNode case_type;
            case_type.SetOperand(ArithOperandNode::AR_OPERAND_CONSTANT,
                                 cypher::FieldData(lgraph::FieldData(case_expr.second)));
            op.children.emplace_back(case_type);
            for (auto &e : case_expr.first) {
                ArithExprNode child;
                child.Set(e, sym_tab);
                op.children.emplace_back(child);
            }
            break;
        }
    case parser::Expression::FILTER:
        /* e.g. n.age > 30 */
        type = AR_EXP_OP;
        op.type = ArithOpNode::AR_OP_FILTER;
        op.fp = expr.Filter();
        break;
    case parser::Expression::MATH:
        {
            /* e.g. (a+b)/c-d*e%f */
            type = AR_EXP_OP;
            op.type = ArithOpNode::AR_OP_MATH;
            auto &math = expr.List();
            for (auto &e : math) {
                ArithExprNode child;
                child.Set(e, sym_tab);
                op.children.emplace_back(child);
            }
            break;
        }
    case parser::Expression::VARIABLE:
        {
            // Check if the identifier is a named path
            auto it = sym_tab.symbols.find(expr.String());
            if (it != sym_tab.symbols.end() && it->second.type == SymbolNode::NAMED_PATH) {
                auto pit = sym_tab.anot_collection.named_paths.find(it->first);
                if (pit == sym_tab.anot_collection.named_paths.end()) CYPHER_TODO();
                type = AR_EXP_OP;
                op.SetFunc(BuiltinFunction::INTL_TO_PATH);
                auto &pattern_element = *pit->second;
                auto &node_pattern = std::get<0>(pattern_element);
                ArithExprNode child;
                child.SetOperand(ArithOperandNode::AR_OPERAND_VARIADIC, std::get<0>(node_pattern),
                                 sym_tab);
                op.children.emplace_back(child);
                for (auto &pec : std::get<1>(pattern_element)) {
                    auto &rp = std::get<0>(pec);
                    auto &np = std::get<1>(pec);
                    ArithExprNode c1;
                    c1.SetOperand(ArithOperandNode::AR_OPERAND_VARIADIC,
                                  std::get<0>(std::get<1>(rp)), sym_tab);
                    op.children.emplace_back(c1);
                    ArithExprNode c2;
                    c2.SetOperand(ArithOperandNode::AR_OPERAND_VARIADIC, std::get<0>(np), sym_tab);
                    op.children.emplace_back(c2);
                }
            } else {
                // default
                type = AR_EXP_OPERAND;
                operand.Set(expr, sym_tab);
            }
            break;
        }
    case parser::Expression::LIST_COMPREHENSION:
        {
            /* e.g. [x IN range(0,10) | x] */
            type = AR_EXP_OP;
            auto &lc = expr.ListComprehension();
            op.SetFunc(BuiltinFunction::INTL_LIST_COMPREHENSION);
            /* The nested symbol table used in list comprehension contains ONE EXTRA symbol,
             * we will construct it again when evaluate list comprehension.  */
            SymbolTable nested_sym_tab_for_list_comprehension(sym_tab);
            nested_sym_tab_for_list_comprehension.symbols.emplace(
                lc[0].String(),
                SymbolNode(sym_tab.symbols.size(), SymbolNode::CONSTANT, SymbolNode::LOCAL));
            for (auto &e : expr.ListComprehension()) {
                ArithExprNode child;
                if (e.type != parser::Expression::NA)
                    child.Set(e, nested_sym_tab_for_list_comprehension);
                op.children.emplace_back(child);
            }
            break;
        }
    case parser::Expression::RELATIONSHIPS_PATTERN:
        /* e.g. exists((n)-[]->(m)) */
        CYPHER_TODO();
    case parser::Expression::LIST:
        {
            /* We treat:
             * [1, 2, 'three'] as operand
             * [{a: "Christopher Nolan", b: "Dennis Quaid"}] as operand
             * [n.name, n.age] as op  */
            bool is_operand = true;
            for (auto &e : expr.List()) {
                if (!e.IsLiteral()) is_operand = false;
                if (e.type == parser::Expression::MAP) is_operand = true;
            }
            if (!is_operand) {
                type = AR_EXP_OP;
                op.SetFunc(BuiltinFunction::INTL_TO_LIST);
                for (auto &e : expr.List()) {
                    ArithExprNode child;
                    child.Set(e, sym_tab);
                    CYPHER_THROW_ASSERT(child.type == AR_EXP_OPERAND);
                    op.children.emplace_back(child);
                }
                break;
            }
        }
    default:
        type = AR_EXP_OPERAND;
        operand.Set(expr, sym_tab);
    }
}

void ArithExprNode::RealignAliasId(const SymbolTable &sym_tab) {
    if (type == AR_EXP_OPERAND) {
        return operand.RealignAliasId(sym_tab);
    } else {
        return op.RealignAliasId(sym_tab);
    }
}

static bool IsMathOperator(const ArithExprNode &node) {
    if (node.type != ArithExprNode::AR_EXP_OPERAND ||
        node.operand.type != ArithOperandNode::AR_OPERAND_CONSTANT ||
        !node.operand.constant.IsString()) {
        return false;
    }
    auto &s = node.operand.constant.scalar.string();
    static const int8_t operator_table[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, /* %, *, +, -, / */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, /* ^ */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    return s.size() == 1 && operator_table[(size_t)(s[0])] == (int8_t)1;
}

void ArithOpNode::RealignAliasId(const cypher::SymbolTable &sym_tab) {
    for (auto &c : children) c.RealignAliasId(sym_tab);
}

Entry ArithOpNode::Evaluate(RTContext *ctx, const Record &record) const {
    switch (type) {
    case AR_OP_AGGREGATE:
        /* Aggregation function should be reduced by now.
         * TODO(anyone) verify above statement. */
        return agg_func->result;
    case AR_OP_FUNC:
        {
            if (func) return Entry(func(ctx, record, children));
            /* if !func, custom function */
            std::string input, output;
            std::string name = func_name.substr(std::string(CUSTOM_FUNCTION_PREFIX).size());
            // evaluate every child and add to input for output
            for (int i = 1; i < (int)children.size(); i++) {
                auto v = children[i].Evaluate(ctx, record);
                input.append(v.ToString());
                if (i != (int)children.size() - 1) input.append(",");
            }
            auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
            bool exists =
                ac_db.CallPlugin(ctx->txn_.get(), lgraph::plugin::Type::CPP,
                                 "A_DUMMY_TOKEN_FOR_CPP_PLUGIN", name, input, 0, false, output);
            if (!exists) {
                THROW_CODE(InputError, "Plugin [{}] does not exist.", name);
            }
            return Entry(cypher::FieldData(lgraph::FieldData(output)));
        }
    case AR_OP_FILTER:
        return Entry(cypher::FieldData(lgraph::FieldData(fp->DoFilter(ctx, record))));
    case AR_OP_CASE:
        {
            CYPHER_THROW_ASSERT(!children.empty() &&
                                children[0].type == ArithExprNode::AR_EXP_OPERAND &&
                                children[0].operand.type == ArithOperandNode::AR_OPERAND_CONSTANT);
            auto case_type = children[0].operand.constant.scalar.integer();
            switch (case_type) {
            case 0:
            case 1:
                for (int i = 1; i < (int)children.size() - 1; i += 2) {
                    CYPHER_THROW_ASSERT(children[i].op.type == ArithOpNode::AR_OP_FILTER);
                    auto pred = children[i].Evaluate(ctx, record);
                    CYPHER_THROW_ASSERT(pred.IsBool());
                    if (pred.constant.scalar.AsBool()) return children[i + 1].Evaluate(ctx, record);
                }
                return case_type == 0 ? Entry() : children.back().Evaluate(ctx, record);
            case 2:
                {
                    auto test = children.back().Evaluate(ctx, record);
                    for (int i = 1; i < (int)children.size() - 1; i += 2) {
                        auto value = children[i].Evaluate(ctx, record);
                        if (value == test) return children[i + 1].Evaluate(ctx, record);
                    }
                    return Entry();
                }
            case 3:
                {
                    auto test = children[children.size() - 2].Evaluate(ctx, record);
                    for (int i = 1; i < (int)children.size() - 2; i += 2) {
                        auto value = children[i].Evaluate(ctx, record);
                        if (value == test) return children[i + 1].Evaluate(ctx, record);
                    }
                    return children.back().Evaluate(ctx, record);
                }
            default:
                return Entry();
            }
        }
    case AR_OP_MATH:
        {
            /* TODO(anyone) move out to AR_OP_FUNC */
            std::stack<cypher::FieldData> s;
            for (auto &c : children) {
                if (!IsMathOperator(c)) {
                    auto v = c.Evaluate(ctx, record);
                    s.emplace(v.constant);
                } else {
                    CYPHER_THROW_ASSERT(!s.empty());
                    auto y = s.top();
                    s.pop();
                    CYPHER_THROW_ASSERT(!s.empty());
                    auto x = s.top();
                    s.pop();
                    char token = c.operand.constant.scalar.string().at(0);
                    cypher::FieldData ret;
                    switch (token) {
                    case '+':
                        ret = Add(x, y);
                        break;
                    case '-':
                        ret = Sub(x, y);
                        break;
                    case '*':
                        ret = Mul(x, y);
                        break;
                    case '/':
                        ret = Div(x, y);
                        break;
                    case '%':
                        ret = Mod(x, y);
                        break;
                    case '^':
                        ret = Pow(x, y);
                        break;
                    default:
                        throw lgraph::CypherException("Invalid mathematical operator: " +
                                                      c.operand.constant.scalar.string());
                    }
                    s.emplace(ret);
                }
            }
            CYPHER_THROW_ASSERT(!s.empty());
            return Entry(s.top());
        }
    default:
        CYPHER_TODO();
    }
}

std::string ArithOpNode::ToString() const {
    std::string str;
    str.append(func_name).append("(");
    for (auto &c : children) str.append(c.ToString()).append(",");
    if (str[str.size() - 1] == ',') str.pop_back();
    str.append(")");
    return str;
}
}  // namespace cypher
