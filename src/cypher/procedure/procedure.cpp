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
// Created by wt on 19-1-10.
//

#include "import/import_online.h"
#include "core/field_data_helper.h"
#include "core/thread_id.h"
#include "db/galaxy.h"
#include "parser/data_typedef.h"
#include "resultset/record.h"
#include "server/state_machine.h"
#include "restful/server/json_convert.h"
#include "cypher/graph/common.h"
#include "cypher/procedure/procedure.h"
#include "cypher/procedure/utils.h"
#include "butil/endpoint.h"
#include "cypher/monitor/memory_monitor_allocator.h"
#include "fma-common/encrypt.h"
#include "import/import_v3.h"
#include "server/bolt_session.h"

namespace cypher {

#define CYPHER_ARG_CHECK(pred, msg)           \
    if (!(pred)) {                            \
        throw lgraph::ReminderException(msg); \
    }

#define CYPHER_DB_PROCEDURE_GRAPH_CHECK()                                                     \
    do {                                                                                      \
        if (ctx->graph_.empty()) throw lgraph::CypherException("graph name cannot be empty"); \
        if (!ctx->ac_db_) CYPHER_INTL_ERR();                                                  \
        if (!ctx->txn_) CYPHER_INTL_ERR();                                                    \
    } while (0)

typedef std::unordered_map<std::string, std::unordered_map<std::string, lgraph::FieldData>>
    EDGE_PROPERTY_FILTER_T;

const std::unordered_map<std::string, lgraph::FieldType> BuiltinProcedure::type_map_ =
    lgraph::field_data_helper::_detail::_FieldName2TypeDict_();

cypher::VEC_STR ProcedureTitles(const std::string &procedure_name,
                                const cypher::VEC_STR &yield_items) {
    cypher::VEC_STR titles;
    auto pp = global_ptable.GetProcedure(procedure_name);

    if (yield_items.empty()) {
        for (auto &res : pp->signature.result_list) titles.emplace_back(res.name);
    } else {
        for (auto &item : yield_items) {
            if (!pp->ContainsYieldItem(item)) {
                throw lgraph::CypherException("Unknown procedure output: " + item);
            }
            titles.emplace_back(item);
        }
    }
    return titles;
}

static bool ParseIsVertex(const std::string &token) {
    if (fma_common::StringEqual(token, "vertex", false)) return true;
    if (fma_common::StringEqual(token, "edge", false)) return false;
    THROW_CODE(InputError, "Wrong argument given for label type: must be 'vertex' or 'edge'.");
    return false;
}

void BuiltinProcedure::DbSubgraph(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                  const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 1, "This function takes 1 argrument. e.g. db.DbSubGraph(vids)");
    CYPHER_ARG_CHECK(args[0].IsArray(), "db.DbSubGraph(vids): `vids` must be list");
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    std::set<lgraph::VertexId> set_vids;
    for (auto &vid : *args[0].constant.array) {
        CYPHER_ARG_CHECK(vid.IsInteger(), "db.DbSubGraph(vids): `vid` must be int");
        set_vids.emplace(vid.scalar.integer());
    }
    auto vit = ctx->txn_->GetTxn()->GetVertexIterator();
    std::vector<nlohmann::json> vertices;
    std::vector<nlohmann::json> relationships;
    for (auto vid : set_vids) {
        if (!vit.Goto(vid, false)) continue;
        lgraph_api::lgraph_result::Node node;
        node.id = vid;
        node.label = ctx->txn_->GetTxn()->GetVertexLabel(vit);
        for (auto &property : ctx->txn_->GetTxn()->GetVertexFields(vit)) {
            node.properties.insert(property);
        }
        vertices.push_back(node.ToJson());
        for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
            lgraph_api::lgraph_result::Relationship repl;
            auto uid = eit.GetUid();
            if (set_vids.find(uid.dst) == set_vids.end()) continue;
            repl.id = uid.eid;
            repl.src = uid.src;
            repl.dst = uid.dst;
            repl.label_id = uid.lid;
            repl.label = ctx->txn_->GetTxn()->GetEdgeLabel(eit);
            repl.forward = true;
            repl.tid = uid.tid;
            auto rel_fields = ctx->txn_->GetTxn()->GetEdgeFields(eit);
            for (auto &property : rel_fields) {
                repl.properties.insert(property);
            }
            relationships.push_back(repl.ToJson());
        }
    }
    Record r;
    nlohmann::json j;
    j["nodes"] = vertices;
    j["relationships"] = relationships;
    r.AddConstant(lgraph::FieldData(j.dump()));
    records->push_back(r.Snapshot());
}

void BuiltinProcedure::DbVertexLabels(RTContext *ctx, const Record *record,
                                      const VEC_EXPR &args, const VEC_STR &yield_items,
                                      std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.vertexLabels()",
                                           args.size()))
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    auto labels = ctx->txn_->GetTxn()->GetAllLabels(true);
    for (auto &l : labels) {
        Record r;
        r.AddConstant(lgraph::FieldData(l));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbEdgeLabels(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                    const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.edgeLabels()",
                                           args.size()))
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    auto labels = ctx->txn_->GetTxn()->GetAllLabels(false);
    for (auto &l : labels) {
        Record r;
        r.AddConstant(lgraph::FieldData(l));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbIndexes(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                 const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.indexes()",
                                           args.size()))
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    auto vertex_indexes = ctx->txn_->GetTxn()->ListVertexIndexes();
    for (auto &i : vertex_indexes) {
        Record r;
        r.AddConstant(lgraph::FieldData(i.label));
        r.AddConstant(lgraph::FieldData(i.field));
        r.AddConstant(lgraph::FieldData("vertex"));
        bool unique = false, pair_unique = false;
        switch (i.type) {
        case lgraph::IndexType::GlobalUniqueIndex:
            unique = true;
            break;
        case lgraph::IndexType::PairUniqueIndex:
            pair_unique = true;
            break;
        case lgraph::IndexType::NonuniqueIndex:
            // just to pass the compilation
            break;
        }
        r.AddConstant(lgraph::FieldData(unique));
        r.AddConstant(lgraph::FieldData(pair_unique));
        records->emplace_back(r.Snapshot());
    }

    auto vertex_composite_indexes = ctx->txn_->GetTxn()->ListVertexCompositeIndexes();
    for (auto &i : vertex_composite_indexes) {
        Record r;
        r.AddConstant(lgraph::FieldData(i.label));
        r.AddConstant(lgraph::FieldData(boost::join(i.fields, ",")));
        r.AddConstant(lgraph::FieldData("vertex"));
        bool unique = false, pair_unique = false;
        switch (i.type) {
        case lgraph::CompositeIndexType::UniqueIndex:
            {
                unique = true;
                break;
            }
        case lgraph::CompositeIndexType::NonUniqueIndex:
            {
                unique = false;
                break;
            }
        }
        r.AddConstant(lgraph::FieldData(unique));
        r.AddConstant(lgraph::FieldData(pair_unique));
        records->emplace_back(r.Snapshot());
    }

    auto edge_indexes = ctx->txn_->GetTxn()->ListEdgeIndexes();
    for (auto &i : edge_indexes) {
        Record r;
        r.AddConstant(lgraph::FieldData(i.label));
        r.AddConstant(lgraph::FieldData(i.field));
        r.AddConstant(lgraph::FieldData("edge"));
        bool unique = false, pair_unique = false;
        switch (i.type) {
        case lgraph::IndexType::GlobalUniqueIndex:
            unique = true;
            break;
        case lgraph::IndexType::PairUniqueIndex:
            pair_unique = true;
            break;
        case lgraph::IndexType::NonuniqueIndex:
            // just to pass the compilation
            break;
        }
        r.AddConstant(lgraph::FieldData(unique));
        r.AddConstant(lgraph::FieldData(pair_unique));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbListLabelIndexes(RTContext *ctx, const Record *record,
                                          const VEC_EXPR &args, const VEC_STR &yield_items,
                                          std::vector<cypher::Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     FMA_FMT("Function requires 2 arguments, but {} are "
                             "given. Usage: db.listLabelIndexes(label_name, label_type)",
                             args.size()))
    CYPHER_ARG_CHECK(args[0].IsString(), FMA_FMT("{} has to be a string ", args[0].ToString()))
    CYPHER_ARG_CHECK(args[1].IsString(), FMA_FMT("{} has to be a string ", args[1].ToString()))
    auto label = args[0].constant.scalar.AsString();
    bool is_vertex = ParseIsVertex(args[1].constant.scalar.AsString());
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    std::vector<lgraph::IndexSpec> indexes;
    std::vector<lgraph::CompositeIndexSpec> compositeIndexes;
    if (is_vertex) {
        indexes = ctx->txn_->GetTxn()->ListVertexIndexByLabel(label);
        compositeIndexes = ctx->txn_->GetTxn()->ListVertexCompositeIndexByLabel(label);
    } else {
        indexes = ctx->txn_->GetTxn()->ListEdgeIndexByLabel(label);
    }
    for (auto &i : indexes) {
        if (i.label != label) continue;
        Record r;
        r.AddConstant(lgraph::FieldData(i.label));
        r.AddConstant(lgraph::FieldData(i.field));
        bool unique = false, pair_unique = false;
        switch (i.type) {
        case lgraph::IndexType::GlobalUniqueIndex:
            unique = true;
            break;
        case lgraph::IndexType::PairUniqueIndex:
            pair_unique = true;
            break;
        case lgraph::IndexType::NonuniqueIndex:
            // just to pass the compilation
            break;
        }
        r.AddConstant(lgraph::FieldData(unique));
        r.AddConstant(lgraph::FieldData(pair_unique));
        records->emplace_back(r.Snapshot());
    }
    for (auto &i : compositeIndexes) {
        if (i.label != label) continue;
        Record r;
        r.AddConstant(lgraph::FieldData(i.label));
        r.AddConstant(lgraph::FieldData(boost::join(i.fields, ",")));
        bool unique = false, pair_unique = false;
        switch (i.type) {
        case lgraph::CompositeIndexType::UniqueIndex:
            {
                unique = true;
                break;
            }
        case lgraph::CompositeIndexType::NonUniqueIndex:
            {
                unique = false;
                break;
            }
        }
        r.AddConstant(lgraph::FieldData(unique));
        r.AddConstant(lgraph::FieldData(pair_unique));
        records->emplace_back(r.Snapshot());
    }
}
void BuiltinProcedure::DbPropertyKeys(RTContext *ctx, const Record *record,
                                      const VEC_EXPR &args, const VEC_STR &yield_items,
                                      std::vector<Record> *records) {
    CYPHER_TODO();
}

void BuiltinProcedure::DbWarmUp(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                const VEC_STR &yield_items,
                                std::vector<cypher::Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.warmup()",
                                           args.size()))
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    double t1 = fma_common::GetTime();
    ctx->txn_.reset();
    ctx->ac_db_->WarmUp();
    double t2 = fma_common::GetTime();
    Record r;
    r.AddConstant(
        lgraph::FieldData("Warm up successful in " + std::to_string(t2 - t1) + " seconds."));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsProcedures(RTContext *ctx, const Record *record,
                                      const VEC_EXPR &args, const VEC_STR &yield_items,
                                      std::vector<cypher::Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.procedures()",
                                           args.size()))
    auto pp = global_ptable.GetProcedure("dbms.procedures");
    CYPHER_THROW_ASSERT(pp && pp->ContainsYieldItem("name") && pp->ContainsYieldItem("signature"));
    cypher::VEC_STR titles;
    if (yield_items.empty()) {
        titles.push_back("name");
        titles.push_back("signature");
        titles.push_back("read_only");
    } else {
        for (auto &item : yield_items) titles.emplace_back(item);
    }
    std::unordered_map<std::string, std::function<void(const Procedure &, Record &)>> lmap = {
        {"name",
         [](const Procedure &p, Record &r) { r.AddConstant(lgraph::FieldData(p.proc_name)); }},
        {"signature",
         [](const Procedure &p, Record &r) { r.AddConstant(lgraph::FieldData(p.Signature())); }},
        {"read_only",
         [](const Procedure &p, Record &r) { r.AddConstant(lgraph::FieldData(p.read_only)); }}};
    for (auto &p : global_procedures) {
        Record r;
        for (auto &title : titles) {
            lmap.find(title)->second(p, r);
        }
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::_ExtractFds(const VEC_EXPR &args, std::string &label,
                                   std::string &extra, std::vector<lgraph::FieldSpec> &fds) {
    using namespace parser;
    CYPHER_ARG_CHECK(args.size() % SPEC_MEMBER_SIZE == 2,
                     "Not enough arguments. This function takes 2 or more arguments.")
    CYPHER_ARG_CHECK(args[0].IsString(), FMA_FMT("{} has to be a string ", args[0].ToString()))
    CYPHER_ARG_CHECK(args[1].IsString(), FMA_FMT("{} has to be a string ", args[0].ToString()))

    label = args[0].constant.scalar.AsString();
    extra = args[1].constant.scalar.AsString();
    for (int i = 0; i < (int)args.size() / 3; i++) {
        CYPHER_ARG_CHECK(
            args[i * SPEC_MEMBER_SIZE + 2].IsString(),
            FMA_FMT("{} has to be a string ", args[i * SPEC_MEMBER_SIZE + 2].ToString()))
        CYPHER_ARG_CHECK(
            args[i * SPEC_MEMBER_SIZE + 3].IsScalar(),
            FMA_FMT("{} has to be a variable，alternative value is "
                    "(NUL,BOOL,INT8,INT16,INT32,INT64,FLOAT,DOUBLE,DATE,DATETIME,STRING,BLOB) ",
                    args[i * SPEC_MEMBER_SIZE + 3].ToString()))
        CYPHER_ARG_CHECK(
            args[i * SPEC_MEMBER_SIZE + 4].IsBool(),
            FMA_FMT("{} has to be a boolean ", args[i * SPEC_MEMBER_SIZE + 4].ToString()))
        auto name = args[i * SPEC_MEMBER_SIZE + 2].constant.scalar.AsString();
        auto type_name = args[i * SPEC_MEMBER_SIZE + 3].constant.scalar.AsString();
        auto nullable = args[i * SPEC_MEMBER_SIZE + 4].constant.scalar.AsBool();

        std::transform(type_name.begin(), type_name.end(), type_name.begin(), ::tolower);
        auto it = type_map_.find(type_name);
        if (it == type_map_.end()) {
            throw lgraph::ReminderException("Unknown type name: " + type_name);
        }
        fds.emplace_back(name, it->second, nullable);
    }
}

void BuiltinProcedure::_ExtractAccessLevel(
    const VEC_EXPR &args, std::string &role,
    std::unordered_map<std::string, lgraph::AccessLevel> &leves) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "Not enough arguments. This function takes 2 or "
                     "more arguments. first is a string, other arguments are map");
    CYPHER_ARG_CHECK(args[0].IsString(), "wrong arguments type，has to be a string");
    CYPHER_ARG_CHECK(args[1].IsMap(), "wrong arguments type， has to be a map");
    role = args[0].constant.scalar.AsString();
    std::unordered_map<std::string, lgraph::AccessLevel> temp_levels;
    for (auto &kv : *args[1].constant.map) {
        auto it = ValidAccessLevels.find(kv.second.AsString());
        CYPHER_ARG_CHECK(it != ValidAccessLevels.end(), "unknown access level");
        temp_levels[kv.first] = it->second;
    }
    std::swap(leves, temp_levels);
}

static ::std::vector<::lgraph::FieldSpec> ParseFieldSpecs(const VEC_EXPR &args,
                                                          size_t start_from) {
    using namespace parser;
    std::vector<lgraph::FieldSpec> ret;
    for (size_t i = start_from; i < args.size(); i++) {
        CYPHER_ARG_CHECK(args[i].IsArray() && args[i].constant.array->size() == 3,
                         "Each FieldSpec must be a list of ('name', 'type' 'optional')");
        const auto &list = *args[i].constant.array;
        CYPHER_ARG_CHECK(list[0].IsString() && list[1].IsString() && list[2].IsBool(),
                         "Each FieldSpec must be a list of (name, type [,optional])")
        lgraph::FieldType ft;
        if (!lgraph::field_data_helper::TryGetFieldType(list[1].AsString(), ft))
            THROW_CODE(InputError, "Illegal field type:{}", list[1]);
        ret.emplace_back(list[0].AsString(), ft, list[2].AsBool());
    }
    return ret;
}

static std::vector<std::string> ParseStringList(const Entry &arg, const std::string &param_name) {
    std::vector<std::string> ret;
    if (arg.IsString()) {
        ret.push_back(arg.constant.scalar.AsString());
    } else if (arg.IsArray()) {
        for (auto &a : *arg.constant.array) {
            if (_F_UNLIKELY(!a.IsString()))
                THROW_CODE(InputError,
                           "Illegal value for parameter [{}]: must be a string or list of strings.",
                           param_name);
            ret.push_back(a.AsString());
        }
    } else {
        THROW_CODE(InputError,
                   "Illegal value for parameter [{}]: must be a string or list of strings.",
                   param_name);
    }
    return ret;
}

static std::string ParseStringArg(const Entry &entry, const std::string &param_name) {
    if (_F_UNLIKELY(!entry.IsString()))
        THROW_CODE(InputError, "Illegal value for parameter [{}]: must be a string.", param_name);
    return entry.constant.scalar.AsString();
}

std::optional<lgraph_api::FieldData> JsonToFieldData(const nlohmann::json& j_object,
                                                     const lgraph_api::FieldSpec& fs) {
    if (j_object.is_null()) {
        if (fs.optional) {
            return lgraph_api::FieldData();
        } else {
            return {};
        }
    }
    try {
        switch (fs.type) {
        case lgraph_api::FieldType::BOOL:
            return lgraph_api::FieldData(j_object.get<bool>());
        case lgraph_api::FieldType::INT8:
            return lgraph_api::FieldData(j_object.get<int8_t>());
        case lgraph_api::FieldType::INT16:
            return lgraph_api::FieldData(j_object.get<int16_t>());
        case lgraph_api::FieldType::INT32:
            return lgraph_api::FieldData(j_object.get<int32_t>());
        case lgraph_api::FieldType::INT64:
            return lgraph_api::FieldData(j_object.get<int64_t>());
        case lgraph_api::FieldType::FLOAT:
            return lgraph_api::FieldData(j_object.get<float>());
        case lgraph_api::FieldType::DOUBLE:
            return lgraph_api::FieldData(j_object.get<double>());
        case lgraph_api::FieldType::STRING:
            return lgraph_api::FieldData(j_object.get<std::string>());
        case lgraph_api::FieldType::DATE:
            return lgraph_api::FieldData(lgraph_api::Date(j_object.get<std::string>()));
        case lgraph_api::FieldType::DATETIME:
            return lgraph_api::FieldData(lgraph_api::DateTime(j_object.get<std::string>()));
        default:
            return {};
        }
    } catch (...) {
        return {};
    }
}

void BuiltinProcedure::DbUpsertVertex(RTContext *ctx, const Record *record,
                                      const VEC_EXPR &args, const VEC_STR &yield_items,
                                      std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. db.upsertVertex(label_name, list_data)");
    CYPHER_ARG_CHECK(args[0].IsString(), "label_name type should be string")
    CYPHER_ARG_CHECK(args[1].IsArray(), "list_data type should be list")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    const auto& list = args[1].constant.array;
    lgraph_api::GraphDB db(ctx->ac_db_.get(), false);
    auto txn = db.CreateReadTxn();
    auto label_id = txn.GetVertexLabelId(args[0].constant.AsString());
    auto vertex_fields = txn.GetVertexSchema(args[0].constant.AsString());
    std::unordered_map<std::string, std::pair<size_t, lgraph_api::FieldSpec>> fd_type;
    for (const auto& fd : vertex_fields) {
        fd_type[fd.name] = std::make_pair(txn.GetVertexFieldId(label_id, fd.name), fd);
    }
    std::string pf = txn.GetVertexPrimaryField(args[0].constant.AsString());
    auto index_fds = txn.GetTxn()->ListVertexIndexByLabel(args[0].constant.AsString());
    std::unordered_map<size_t, bool> unique_indexs;
    for (auto& index : index_fds) {
        if (index.label == pf) {
            continue;
        }
        if (index.type == lgraph_api::IndexType::GlobalUniqueIndex) {
            unique_indexs[txn.GetVertexFieldId(label_id, index.field)] = true;
        }
    }

    txn.Abort();
    int64_t total = list->size();
    int64_t data_error = 0;
    int64_t index_conflict = 0;
    int64_t insert = 0;
    int64_t update = 0;
    std::vector<std::tuple<int, std::vector<size_t>, std::vector<size_t>,
                           std::vector<lgraph_api::FieldData>>> lines;
    for (auto& line : *list) {
        int primary_pos = -1;
        std::vector<size_t> unique_pos;
        std::vector<size_t> field_ids;
        std::vector<lgraph_api::FieldData> fds;
        bool success = true;
        if (line.IsMap()) {
            THROW_CODE(InputError, "The type of the elements in the list must be map");
        }
        for (auto& item : *line.map) {
            auto iter = fd_type.find(item.first);
            if (iter != fd_type.end()) {
                auto fd = item.second.scalar;
                if (fd.IsNull()) {
                    success = false;
                    break;
                }
                fds.emplace_back(std::move(fd));
                field_ids.push_back(iter->second.first);
                if (unique_indexs.count(iter->second.first)) {
                    unique_pos.push_back(field_ids.size() - 1);
                }
                if (item.first == pf) {
                    primary_pos = (int)field_ids.size() - 1;
                }
            }
        }
        if (success && primary_pos >= 0) {
            lines.emplace_back(primary_pos, std::move(unique_pos),
                               std::move(field_ids), std::move(fds));
        } else {
            data_error++;
        }
    }
    txn = db.CreateWriteTxn();
    for (auto& l : lines) {
        int ret = txn.UpsertVertex(label_id, std::get<0>(l),
                                   std::get<1>(l), std::get<2>(l), std::get<3>(l));
        if (ret == 0) {
            index_conflict++;
        } else if (ret == 1) {
            insert++;
        } else {
            update++;
        }
    }
    txn.Commit();
    Record r;
    r.AddConstant(lgraph::FieldData(total));
    r.AddConstant(lgraph::FieldData(data_error));
    r.AddConstant(lgraph::FieldData(index_conflict));
    r.AddConstant(lgraph::FieldData(insert));
    r.AddConstant(lgraph::FieldData(update));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbUpsertVertexByJson(RTContext *ctx, const Record *record,
                                            const VEC_EXPR &args, const VEC_STR &yield_items,
                                            std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. db.upsertVertexByJson(label_name, list_data)");
    CYPHER_ARG_CHECK(args[0].IsString(), "label_name type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(),
                     "list_data type should be json string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    nlohmann::json json_data = nlohmann::json::parse(args[1].constant.AsString());
    if (!json_data.is_array()) {
        THROW_CODE(InputError, "The json data should be array type");
    }
    lgraph_api::GraphDB db(ctx->ac_db_.get(), false);
    auto txn = db.CreateReadTxn();
    auto label_id = txn.GetVertexLabelId(args[0].constant.AsString());
    auto vertex_fields = txn.GetVertexSchema(args[0].constant.AsString());
    std::unordered_map<std::string, std::pair<size_t, lgraph_api::FieldSpec>> fd_type;
    for (const auto& fd : vertex_fields) {
        fd_type[fd.name] = std::make_pair(txn.GetVertexFieldId(label_id, fd.name), fd);
    }
    std::string pf = txn.GetVertexPrimaryField(args[0].constant.AsString());
    auto index_fds = txn.GetTxn()->ListVertexIndexByLabel(args[0].constant.AsString());
    std::unordered_map<size_t, bool> unique_indexs;
    for (auto& index : index_fds) {
        if (index.label == pf) {
            continue;
        }
        if (index.type == lgraph_api::IndexType::GlobalUniqueIndex) {
            unique_indexs[txn.GetVertexFieldId(label_id, index.field)] = true;
        }
    }

    txn.Abort();
    int64_t json_total = json_data.size();
    int64_t json_error = 0;
    int64_t index_conflict = 0;
    int64_t insert = 0;
    int64_t update = 0;
    std::vector<std::tuple<int, std::vector<size_t>, std::vector<size_t>,
                           std::vector<lgraph_api::FieldData>>> lines;
    for (auto& line : json_data) {
        int primary_pos = -1;
        std::vector<size_t> unique_pos;
        std::vector<size_t> field_ids;
        std::vector<lgraph_api::FieldData> fds;
        bool success = true;
        for (auto& item : line.items()) {
            auto iter = fd_type.find(item.key());
            if (iter != fd_type.end()) {
                auto fd = JsonToFieldData(item.value(), iter->second.second);
                if (!fd) {
                    success = false;
                    break;
                }
                fds.emplace_back(std::move(fd.value()));
                field_ids.push_back(iter->second.first);
                if (unique_indexs.count(iter->second.first)) {
                    unique_pos.push_back(field_ids.size() - 1);
                }
                if (item.key() == pf) {
                    primary_pos = (int)field_ids.size() - 1;
                }
            }
        }
        if (success && primary_pos >= 0) {
            lines.emplace_back(primary_pos, std::move(unique_pos),
                               std::move(field_ids), std::move(fds));
        } else {
            json_error++;
        }
    }
    txn = db.CreateWriteTxn();
    for (auto& l : lines) {
        int ret = txn.UpsertVertex(label_id, std::get<0>(l),
                                   std::get<1>(l), std::get<2>(l), std::get<3>(l));
        if (ret == 0) {
            index_conflict++;
        } else if (ret == 1) {
            insert++;
        } else {
            update++;
        }
    }
    txn.Commit();
    Record r;
    r.AddConstant(lgraph::FieldData(json_total));
    r.AddConstant(lgraph::FieldData(json_error));
    r.AddConstant(lgraph::FieldData(index_conflict));
    r.AddConstant(lgraph::FieldData(insert));
    r.AddConstant(lgraph::FieldData(update));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbUpsertEdge(RTContext *ctx, const Record *record,
                                    const VEC_EXPR &args, const VEC_STR &yield_items,
                                    std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 4,
                     "need 4 parameters, "
                     "e.g. db.upsertEdge(label_name, start_spec, end_spec, list_data)")
    CYPHER_ARG_CHECK(args[0].IsString(), "label_name type should be string")
    CYPHER_ARG_CHECK(args[1].IsMap(), "start_spec type should be map")
    CYPHER_ARG_CHECK(args[2].IsMap(), "end_spec type should be map")
    CYPHER_ARG_CHECK(args[3].IsArray(), "list_data type should be list")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    const auto& start = *args[1].constant.map;
    if (!start.count("type") || !start.count("key")) {
        THROW_CODE(InputError, "start_spec missing 'type' or 'key'");
    }
    const auto& end = *args[2].constant.map;
    if (!end.count("type") || !end.count("key")) {
        THROW_CODE(InputError, "end_spec missing 'type' or 'key'");
    }
    std::string start_type = start.at("type").AsString();
    std::string start_json_key = start.at("key").AsString();
    std::string end_type = end.at("type").AsString();
    std::string end_json_key = end.at("key").AsString();
    lgraph_api::GraphDB db(ctx->ac_db_.get(), false);
    auto txn = db.CreateReadTxn();
    auto label_id = txn.GetEdgeLabelId(args[0].constant.AsString());

    auto start_label_id = txn.GetVertexLabelId(start_type);
    auto start_pf = txn.GetVertexPrimaryField(start_type);
    auto start_pf_id = txn.GetVertexFieldId(start_label_id, start_pf);
    lgraph_api::FieldSpec start_pf_fs;
    auto vertex_fields = txn.GetVertexSchema(start_type);
    for (auto& item : vertex_fields) {
        if (item.name == start_pf) {
            start_pf_fs = item;
            break;
        }
    }

    auto end_label_id = txn.GetVertexLabelId(end_type);
    auto end_pf = txn.GetVertexPrimaryField(end_type);
    auto end_pf_id = txn.GetVertexFieldId(end_label_id, end_pf);
    lgraph_api::FieldSpec end_pf_fs;
    vertex_fields = txn.GetVertexSchema(end_type);
    for (auto& item : vertex_fields) {
        if (item.name == end_pf) {
            end_pf_fs = item;
            break;
        }
    }

    auto edge_fields = txn.GetEdgeSchema(args[0].constant.AsString());
    std::unordered_map<std::string, std::pair<size_t, lgraph_api::FieldSpec>> fd_type;
    for (const auto& fd : edge_fields) {
        fd_type[fd.name] = std::make_pair(txn.GetEdgeFieldId(label_id, fd.name), fd);
    }

    auto index_fds = txn.GetTxn()->ListEdgeIndexByLabel(args[0].constant.AsString());
    std::unordered_map<size_t, bool> unique_indexs;
    for (auto& index : index_fds) {
        if (index.type == lgraph_api::IndexType::GlobalUniqueIndex) {
            unique_indexs[txn.GetEdgeFieldId(label_id, index.field)] = true;
        }
    }

    const auto& list = *args[3].constant.array;
    int64_t json_total = list.size();
    int64_t json_error = 0;
    int64_t index_conflict = 0;
    int64_t insert = 0;
    int64_t update = 0;
    std::vector<std::tuple<int64_t , int64_t, std::vector<size_t>, std::vector<size_t>,
                           std::vector<lgraph_api::FieldData>>> lines;
    for (auto& line : list) {
        int64_t start_vid = -1;
        int64_t end_vid = -1;
        std::vector<size_t> unique_pos;
        std::vector<size_t> field_ids;
        std::vector<lgraph_api::FieldData> fds;
        bool success = true;
        if (!line.IsMap()) {
            THROW_CODE(InputError, "The type of the elements in the list must be map");
        }
        for (auto& item : *line.map) {
            if (item.first == start_json_key) {
                auto fd = item.second.scalar;
                if (fd.IsNull()) {
                    success = false;
                    break;
                }
                auto iiter = txn.GetVertexIndexIterator(
                    start_label_id, start_pf_id, fd, fd);
                if (!iiter.IsValid()) {
                    success = false;
                    break;
                }
                start_vid = iiter.GetVid();
                continue;
            } else if (item.first == end_json_key) {
                auto fd = item.second.scalar;
                if (fd.IsNull()) {
                    success = false;
                    break;
                }
                auto iiter = txn.GetVertexIndexIterator(
                    end_label_id, end_pf_id, fd, fd);
                if (!iiter.IsValid()) {
                    success = false;
                    break;
                }
                end_vid = iiter.GetVid();
                continue;
            } else {
                auto iter = fd_type.find(item.first);
                if (iter != fd_type.end()) {
                    auto fd = item.second.scalar;
                    if (fd.IsNull()) {
                        success = false;
                        break;
                    }
                    fds.emplace_back(std::move(fd));
                    field_ids.push_back(iter->second.first);
                    if (unique_indexs.count(iter->second.first)) {
                        unique_pos.push_back(field_ids.size() - 1);
                    }
                }
            }
        }
        if (success && start_vid >= 0 && end_vid >= 0) {
            lines.emplace_back(start_vid, end_vid, std::move(unique_pos),
                               std::move(field_ids), std::move(fds));
        } else {
            json_error++;
        }
    }
    txn.Abort();
    txn = db.CreateWriteTxn();
    for (auto& l : lines) {
        int ret = txn.UpsertEdge(std::get<0>(l), std::get<1>(l),
                                 label_id, std::get<2>(l), std::get<3>(l), std::get<4>(l));
        if (ret == 0) {
            index_conflict++;
        } else if (ret == 1) {
            insert++;
        } else {
            update++;
        }
    }
    txn.Commit();
    Record r;
    r.AddConstant(lgraph::FieldData(json_total));
    r.AddConstant(lgraph::FieldData(json_error));
    r.AddConstant(lgraph::FieldData(index_conflict));
    r.AddConstant(lgraph::FieldData(insert));
    r.AddConstant(lgraph::FieldData(update));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbUpsertEdgeByJson(RTContext *ctx, const Record *record,
                                          const VEC_EXPR &args, const VEC_STR &yield_items,
                                          std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 4,
                     "need 4 parameters, "
                     "e.g. db.upsertEdgeByJson(label_name, start_spec, end_spec, list_data)")
    CYPHER_ARG_CHECK(args[0].IsString(), "label_name type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "start_spec type should be json string")
    CYPHER_ARG_CHECK(args[2].IsString(), "end_spec type should be json string")
    CYPHER_ARG_CHECK(args[3].IsString(), "list_data type should be json string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    nlohmann::json json_data = nlohmann::json::parse(args[3].constant.AsString());
    if (!json_data.is_array()) {
        THROW_CODE(InputError, "The json data should be array type");
    }
    nlohmann::json start = nlohmann::json::parse(args[1].constant.AsString());
    if (!start.contains("type") || !start.contains("key")) {
        THROW_CODE(InputError, "start_spec missing 'type' or 'key'");
    }
    nlohmann::json end = nlohmann::json::parse(args[2].constant.AsString());
    if (!end.contains("type") || !end.contains("key")) {
        THROW_CODE(InputError, "end_spec missing 'type' or 'key'");
    }

    std::string start_type = start["type"].get<std::string>();
    std::string start_json_key = start["key"].get<std::string>();
    std::string end_type = end["type"].get<std::string>();
    std::string end_json_key = end["key"].get<std::string>();
    lgraph_api::GraphDB db(ctx->ac_db_.get(), false);
    auto txn = db.CreateReadTxn();
    auto label_id = txn.GetEdgeLabelId(args[0].constant.AsString());

    auto start_label_id = txn.GetVertexLabelId(start_type);
    auto start_pf = txn.GetVertexPrimaryField(start_type);
    auto start_pf_id = txn.GetVertexFieldId(start_label_id, start_pf);
    lgraph_api::FieldSpec start_pf_fs;
    auto vertex_fields = txn.GetVertexSchema(start_type);
    for (auto& item : vertex_fields) {
        if (item.name == start_pf) {
            start_pf_fs = item;
            break;
        }
    }

    auto end_label_id = txn.GetVertexLabelId(end_type);
    auto end_pf = txn.GetVertexPrimaryField(end_type);
    auto end_pf_id = txn.GetVertexFieldId(end_label_id, end_pf);
    lgraph_api::FieldSpec end_pf_fs;
    vertex_fields = txn.GetVertexSchema(end_type);
    for (auto& item : vertex_fields) {
        if (item.name == end_pf) {
            end_pf_fs = item;
            break;
        }
    }

    auto edge_fields = txn.GetEdgeSchema(args[0].constant.AsString());
    std::unordered_map<std::string, std::pair<size_t, lgraph_api::FieldSpec>> fd_type;
    for (const auto& fd : edge_fields) {
        fd_type[fd.name] = std::make_pair(txn.GetEdgeFieldId(label_id, fd.name), fd);
    }

    auto index_fds = txn.GetTxn()->ListEdgeIndexByLabel(args[0].constant.AsString());
    std::unordered_map<size_t, bool> unique_indexs;
    for (auto& index : index_fds) {
        if (index.type == lgraph_api::IndexType::GlobalUniqueIndex) {
            unique_indexs[txn.GetEdgeFieldId(label_id, index.field)] = true;
        }
    }

    int64_t json_total = json_data.size();
    int64_t json_error = 0;
    int64_t index_conflict = 0;
    int64_t insert = 0;
    int64_t update = 0;
    std::vector<std::tuple<int64_t , int64_t, std::vector<size_t>, std::vector<size_t>,
                           std::vector<lgraph_api::FieldData>>> lines;
    for (auto& line : json_data) {
        int64_t start_vid = -1;
        int64_t end_vid = -1;
        std::vector<size_t> unique_pos;
        std::vector<size_t> field_ids;
        std::vector<lgraph_api::FieldData> fds;
        bool success = true;
        for (auto& item : line.items()) {
            if (item.key() == start_json_key) {
                auto fd = JsonToFieldData(item.value(), start_pf_fs);
                if (!fd) {
                    success = false;
                    break;
                }
                auto iiter = txn.GetVertexIndexIterator(
                    start_label_id, start_pf_id, fd.value(), fd.value());
                if (!iiter.IsValid()) {
                    success = false;
                    break;
                }
                start_vid = iiter.GetVid();
                continue;
            } else if (item.key() == end_json_key) {
                auto fd = JsonToFieldData(item.value(), end_pf_fs);
                if (!fd) {
                    success = false;
                    break;
                }
                auto iiter = txn.GetVertexIndexIterator(
                    end_label_id, end_pf_id, fd.value(), fd.value());
                if (!iiter.IsValid()) {
                    success = false;
                    break;
                }
                end_vid = iiter.GetVid();
                continue;
            } else {
                auto iter = fd_type.find(item.key());
                if (iter != fd_type.end()) {
                    auto fd = JsonToFieldData(item.value(), iter->second.second);
                    if (!fd) {
                        success = false;
                        break;
                    }
                    fds.emplace_back(std::move(fd.value()));
                    field_ids.push_back(iter->second.first);
                    if (unique_indexs.count(iter->second.first)) {
                        unique_pos.push_back(field_ids.size() - 1);
                    }
                }
            }
        }
        if (success && start_vid >= 0 && end_vid >= 0) {
            lines.emplace_back(start_vid, end_vid, std::move(unique_pos),
                               std::move(field_ids), std::move(fds));
        } else {
            json_error++;
        }
    }
    txn.Abort();
    txn = db.CreateWriteTxn();
    for (auto& l : lines) {
        int ret = txn.UpsertEdge(std::get<0>(l), std::get<1>(l),
                                 label_id, std::get<2>(l), std::get<3>(l), std::get<4>(l));
        if (ret == 0) {
            index_conflict++;
        } else if (ret == 1) {
            insert++;
        } else {
            update++;
        }
    }
    txn.Commit();
    Record r;
    r.AddConstant(lgraph::FieldData(json_total));
    r.AddConstant(lgraph::FieldData(json_error));
    r.AddConstant(lgraph::FieldData(index_conflict));
    r.AddConstant(lgraph::FieldData(insert));
    r.AddConstant(lgraph::FieldData(update));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbCreateVertexLabel(RTContext *ctx, const Record *record,
                                           const VEC_EXPR &args, const VEC_STR &yield_items,
                                           std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() % SPEC_MEMBER_SIZE == 2,
        "e.g. db.createVertexLabel(label_name, primary_fd, field_name, field_type, is_unique)");
    std::string label;
    std::string primary_fd;
    std::vector<lgraph::FieldSpec> fds;
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    _ExtractFds(args, label, primary_fd, fds);
    auto ret = ctx->ac_db_->AddLabel(true, label, fds, lgraph::VertexOptions(primary_fd));
    if (!ret) {
        throw lgraph::LabelExistException(label, true);
    }
}

void BuiltinProcedure::DbCreateEdgeLabelByJson(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args,
                                               const VEC_STR &yield_items,
                                               std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() % SPEC_MEMBER_SIZE == 1,
                     "e.g. db.createEdgeLabelByJson(json_data)");
    CYPHER_ARG_CHECK(args[0].IsString(), "json_data type should be json string")
    nlohmann::json schema;
    schema["schema"] = nlohmann::json::array();
    auto ec = nlohmann::json::parse(args[0].constant.AsString());
    schema["schema"].push_back(ec);
    auto schema_desc = lgraph::import_v2::ImportConfParser::ParseSchema(schema);
    if (schema_desc.label_desc.size() != 1) {
        THROW_CODE(InputError, "json schema size error, size:{}", schema_desc.label_desc.size());
    }
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    std::vector<lgraph_api::FieldSpec> fds;
    const auto& ld = schema_desc.label_desc[0];
    if (ld.is_vertex) {
        THROW_CODE(InputError, "is not edge json schema");
    }
    for (auto& p : ld.GetSchemaDef())
        fds.emplace_back(p.second);
    lgraph_api::EdgeOptions eo;
    if (ld.HasTemporalField()) {
        auto tf = ld.GetTemporalField();
        eo.temporal_field = tf.name;
        eo.temporal_field_order = tf.temporal_order;
    }
    eo.edge_constraints = ld.edge_constraints;
    eo.detach_property = ld.detach_property;
    bool ok = ctx->ac_db_->AddLabel(ld.is_vertex, ld.name, fds, eo);
    if (ok) {
        LOG_INFO() << FMA_FMT("Add {} label:{}", ld.is_vertex ? "vertex" : "edge",
                              ld.name);
    } else {
        throw lgraph::LabelExistException(ld.name, false);
    }
    for (auto& spec : ld.columns) {
        if (spec.index) {
            ctx->ac_db_->AddEdgeIndex(ld.name, spec.name, spec.idxType);
            LOG_INFO() << FMA_FMT("Add edge index [label:{}, field:{}, type:{}]",
                                  ld.name, spec.name, static_cast<int>(spec.idxType));
        }
    }
}

void BuiltinProcedure::DbCreateVertexLabelByJson(RTContext *ctx, const Record *record,
                                                 const VEC_EXPR &args,
                                                 const VEC_STR &yield_items,
                                                 std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() % SPEC_MEMBER_SIZE == 1,
                     "e.g. db.createVertexLabelByJson(json_data)");
    CYPHER_ARG_CHECK(args[0].IsString(), "json_data type should be json string")
    nlohmann::json schema;
    schema["schema"] = nlohmann::json::array();
    auto ec = nlohmann::json::parse(args[0].constant.AsString());
    schema["schema"].push_back(ec);
    auto schema_desc = lgraph::import_v2::ImportConfParser::ParseSchema(schema);
    if (schema_desc.label_desc.size() != 1) {
        THROW_CODE(InputError, "json schema size error, size:{}", schema_desc.label_desc.size());
    }
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    std::vector<lgraph_api::FieldSpec> fds;
    const auto& ld = schema_desc.label_desc[0];
    if (!ld.is_vertex) {
        THROW_CODE(InputError, "is not vertex json schema");
    }
    for (auto& p : ld.GetSchemaDef())
        fds.emplace_back(p.second);
    lgraph_api::VertexOptions vo;
    vo.primary_field = ld.GetPrimaryField().name;
    vo.detach_property = ld.detach_property;
    bool ok = ctx->ac_db_->AddLabel(ld.is_vertex, ld.name, fds, vo);
    if (ok) {
        LOG_INFO() << FMA_FMT("Add {} label:{}", ld.is_vertex ? "vertex" : "edge",
                              ld.name);
    } else {
        throw lgraph::LabelExistException(ld.name, true);
    }
    for (auto& spec : ld.columns) {
        if (spec.index && !spec.primary) {
            ctx->ac_db_->AddVertexIndex(ld.name, spec.name, spec.idxType);
            LOG_INFO() << FMA_FMT("Add vertex index [label:{}, field:{}, type:{}]",
                                  ld.name, spec.name, static_cast<int>(spec.idxType));
        }
    }
}

// params: vertex, label, primary_field,  [fieldspec1], [fieldspec2]...
// params: edge, label, edge_constraints, [fieldspec1], [fieldspec2]...
void BuiltinProcedure::DbCreateLabel(RTContext *ctx, const Record *record,
                                     const VEC_EXPR &args, const VEC_STR &yield_items,
                                     std::vector<Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    if (args.size() < 2)
        THROW_CODE(InputError, "Not enough arguments. This function takes 2 or more arguments.");
    bool is_vertex = ParseIsVertex(args[0].constant.scalar.AsString());
    std::string label = ParseStringArg(args[1], "label_name");
    std::string primary_fd;
    std::vector<std::pair<std::string, std::string>> edge_constraints;
    if (is_vertex) {
        primary_fd = ParseStringArg(args[2], "primary_field");
    } else {
        auto str = ParseStringArg(args[2], "edge_constraints");
        auto ec = nlohmann::json::parse(str);
        for (auto &item : ec) {
            if (item.size() != 2) {
                THROW_CODE(InputError, "The size of each constraint tuple should be 2");
            }
            if (!item[0].is_string() || !item[1].is_string()) {
                THROW_CODE(InputError,
                           "The element of each constraint tuple should be string type");
            }
            edge_constraints.push_back(std::make_pair(item[0], item[1]));
        }
    }
    auto field_specs = ParseFieldSpecs(args, 3);
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    std::unique_ptr<lgraph::LabelOptions> options;
    if (is_vertex) {
        auto vo = std::make_unique<lgraph::VertexOptions>();
        vo->primary_field = primary_fd;
        options = std::move(vo);
    } else {
        auto eo = std::make_unique<lgraph::EdgeOptions>();
        eo->edge_constraints = edge_constraints;
        options = std::move(eo);
    }
    auto ret = ac_db.AddLabel(is_vertex, label, field_specs, *options);
    if (!ret) {
        throw lgraph::LabelExistException(label, is_vertex);
    }
}

// params: vertex/edge, label
void BuiltinProcedure::DbGetLabelSchema(RTContext *ctx, const Record *record,
                                        const VEC_EXPR &args, const VEC_STR &yield_items,
                                        std::vector<Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (args.size() != 2)
        THROW_CODE(InputError,
                   "Wrong number of arguments. This function takes exactly 2 arguments.");
    bool is_vertex = ParseIsVertex(args[0].constant.scalar.AsString());
    std::string label = ParseStringArg(args[1], "label_name");
    auto fs = ctx->txn_->GetTxn()->GetSchema(is_vertex, label);
    for (auto &f : fs) {
        Record r;
        r.AddConstant(lgraph::FieldData(f.name));
        r.AddConstant(lgraph::FieldData(lgraph::field_data_helper::FieldTypeName(f.type)));
        r.AddConstant(lgraph::FieldData(f.optional));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbGetVertexSchema(RTContext *ctx, const Record *record,
                                         const VEC_EXPR &args, const VEC_STR &yield_items,
                                         std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 1, "need one parameters, e.g. db.getVertexSchema(label)")
    CYPHER_ARG_CHECK(args[0].IsString(), "label type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    const lgraph::Schema *schema =
        ctx->txn_->GetTxn()->GetSchema(args[0].constant.scalar.AsString(), true);
    if (!schema) {
        throw lgraph::CypherException(
            FMA_FMT("vertex label {} does not exist", args[0].constant.scalar.AsString()));
    }
    Record r;
    r.AddConstant(lgraph::FieldData(ValueToJson(schema).serialize()));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbGetEdgeSchema(RTContext *ctx, const Record *record,
                                       const VEC_EXPR &args, const VEC_STR &yield_items,
                                       std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 1, "need one parameters, e.g. db.getEdgeSchema(label)");
    CYPHER_ARG_CHECK(args[0].IsString(), "label type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    const lgraph::Schema *schema =
        ctx->txn_->GetTxn()->GetSchema(args[0].constant.scalar.AsString(), false);
    if (!schema) {
        throw lgraph::CypherException(
            FMA_FMT("edge label {} does not exist", args[0].constant.scalar.AsString()));
    }
    Record r;
    r.AddConstant(lgraph::FieldData(ValueToJson(schema).serialize()));
    records->emplace_back(r.Snapshot());
}

// params: vertex/edge, label
void BuiltinProcedure::DbDeleteLabel(RTContext *ctx, const Record *record,
                                     const VEC_EXPR &args, const VEC_STR &yield_items,
                                     std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. db.deleteLabel(label_type, label_name)");
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    bool is_vertex = ParseIsVertex(args[0].constant.scalar.AsString());
    std::string label = ParseStringArg(args[1], "label_name");
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    size_t affected = 0;
    auto ret = ac_db.DeleteLabel(is_vertex, label, &affected);
    if (!ret) {
        throw lgraph::LabelNotExistException(label);
    }
}

// params: vertex/edge, label, field_names
void BuiltinProcedure::DbAlterLabelDelFields(RTContext *ctx, const Record *record,
                                             const VEC_EXPR &args,
                                             const VEC_STR &yield_items,
                                             std::vector<Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    if (args.size() != 3)
        THROW_CODE(InputError,
                   "Wrong number of arguments. This function takes exactly 3 arguments.");
    bool is_vertex = ParseIsVertex(args[0].constant.scalar.AsString());
    std::string label = ParseStringArg(args[1], "label_name");
    std::vector<std::string> fields = ParseStringList(args[2], "field_names");
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    size_t affected = 0;
    auto ret = ac_db.AlterLabelDelFields(label, fields, is_vertex, &affected);
    if (ret) {
        Record r;
        r.AddConstant(lgraph::FieldData::Int64(static_cast<int64_t>(affected)));
        records->emplace_back(r.Snapshot());
    } else {
        throw lgraph::LabelNotExistException(label);
    }
}

// params: vertex/edge, label, [field_spec_value_1], [field_spec_value_2]...
void BuiltinProcedure::DbAlterLabelAddFields(RTContext *ctx, const Record *record,
                                             const VEC_EXPR &args,
                                             const VEC_STR &yield_items,
                                             std::vector<Record> *records) {
    using namespace parser;
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    if (args.size() < 3)
        THROW_CODE(InputError, "Too few arguments. This function takes 3 or more arguments.");
    bool is_vertex = ParseIsVertex(args[0].constant.scalar.AsString());
    std::string label = ParseStringArg(args[1], "label_name");
    // get field_spec_value
    std::vector<lgraph::FieldSpec> fields;
    std::vector<lgraph::FieldData> values;
    for (size_t i = 2; i < args.size(); i++) {
        CYPHER_ARG_CHECK(
            args[i].IsArray() && args[i].constant.array->size() == 4,
            "Each FieldSpec must be a list of ('name', 'type', default_value, 'optional')");
        const auto &list = *args[i].constant.array;
        CYPHER_ARG_CHECK(list[0].IsString() && list[1].IsString() && list[3].IsBool(),
                         "Each FieldSpec must be a list of (name, type, default_value ,optional)");
        const std::string &name = list[0].AsString();
        lgraph::FieldType ft;
        if (!lgraph::field_data_helper::TryGetFieldType(list[1].AsString(), ft))
            THROW_CODE(InputError, "Illegal field type:{}", list[1]);
        lgraph::FieldData default_value = list[2].scalar;
        fields.emplace_back(name, ft, list[3].AsBool());
        values.emplace_back(std::move(default_value));
    }
    // now alter label
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    size_t affected = 0;
    auto ret = ac_db.AlterLabelAddFields(label, fields, values, is_vertex, &affected);
    if (ret) {
        Record r;
        r.AddConstant(lgraph::FieldData::Int64(static_cast<int64_t>(affected)));
        records->emplace_back(r.Snapshot());
    } else {
        throw lgraph::LabelNotExistException(label);
    }
}

// params: vertex/edge, label, [field_spec_1], [field_spec_2],...
void BuiltinProcedure::DbAlterLabelModFields(RTContext *ctx, const Record *record,
                                             const VEC_EXPR &args,
                                             const VEC_STR &yield_items,
                                             std::vector<Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    if (args.size() < 3)
        THROW_CODE(InputError,
                   "Wrong number of arguments. This function takes 3 or more arguments.");
    bool is_vertex = ParseIsVertex(args[0].constant.scalar.AsString());
    std::string label = ParseStringArg(args[1], "label_name");
    std::vector<lgraph::FieldSpec> fields = ParseFieldSpecs(args, 2);
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    size_t affected = 0;
    auto ret = ac_db.AlterLabelModFields(label, fields, is_vertex, &affected);
    if (ret) {
        Record r;
        r.AddConstant(lgraph::FieldData::Int64(static_cast<int64_t>(affected)));
        records->emplace_back(r.Snapshot());
    } else {
        throw lgraph::LabelNotExistException(label);
    }
}

void BuiltinProcedure::DbCreateEdgeLabel(RTContext *ctx, const Record *record,
                                         const VEC_EXPR &args, const VEC_STR &yield_items,
                                         std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() % SPEC_MEMBER_SIZE == 2,
        "e.g. db.createEdgeLabel(label_name, extra, field_name, field_type, is_optional)")
    std::string label;
    std::vector<lgraph::FieldSpec> fds;
    std::vector<std::pair<std::string, std::string>> edge_constraints;
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    std::string extra;
    _ExtractFds(args, label, extra, fds);
    auto ec = nlohmann::json::parse(extra);
    for (auto &item : ec) {
        edge_constraints.push_back(std::make_pair(item[0], item[1]));
    }
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    auto ret = ac_db.AddLabel(false, label, fds, lgraph::EdgeOptions(edge_constraints));
    if (!ret) {
        throw lgraph::LabelExistException(label, true);
    }
}

void BuiltinProcedure::DbAddVertexIndex(RTContext *ctx, const Record *record,
                                        const VEC_EXPR &args, const VEC_STR &yield_items,
                                        std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 3,
                     "need 3 parameters, e.g. db.addIndex(label_name, field_name, unique)")
    CYPHER_ARG_CHECK(args[0].IsString(), "label_name type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "field_name type should be string")
    CYPHER_ARG_CHECK(args[2].IsBool(), "unique type should be boolean")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    auto label = args[0].constant.scalar.AsString();
    auto field = args[1].constant.scalar.AsString();
    auto unique = args[2].constant.scalar.AsBool();
    lgraph::IndexType type =
        unique ? lgraph::IndexType::GlobalUniqueIndex : lgraph::IndexType::NonuniqueIndex;
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    bool success = ac_db.AddVertexIndex(label, field, type);
    if (!success) {
        throw lgraph::IndexExistException(label, field);
    }
}

void BuiltinProcedure::DbAddVertexCompositeIndex(cypher::RTContext *ctx,
                                                 const cypher::Record *record,
                                                 const cypher::VEC_EXPR &args,
                                                 const cypher::VEC_STR &yield_items,
                                                 std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 3,
                     "need 3 parameters, e.g. db.addIndex(label_name, field_name, unique)")
    CYPHER_ARG_CHECK(args[0].IsString(), "label_name type should be string")
    CYPHER_ARG_CHECK(args[1].IsArray(), "field_names type should be list")
    CYPHER_ARG_CHECK(args[2].IsBool(), "unique type should be boolean")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    auto label = args[0].constant.AsString();
    auto fields_args = *args[1].constant.array;
    std::vector<std::string> fields;
    for (auto &arg : fields_args) {
        fields.push_back(arg.AsString());
    }
    auto unique = args[2].constant.AsBool();
    lgraph::CompositeIndexType type = unique ? lgraph::CompositeIndexType::UniqueIndex :
                                             lgraph::CompositeIndexType::NonUniqueIndex;
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    bool success = ac_db.AddVertexCompositeIndex(label, fields, type);
    if (!success) {
        std::string field_strings = boost::algorithm::join(fields, ",");
        THROW_CODE(IndexExist, "VertexCompositeIndex [{}:{}] already exist.", label, field_strings);
    }
}

void BuiltinProcedure::DbAddEdgeIndex(RTContext *ctx, const Record *record,
                                      const VEC_EXPR &args, const VEC_STR &yield_items,
                                      std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 4,
                     "need 3 parameters, e.g. "
                     "db.addEdgeIndex(label_name, field_name, unique, pair_unique)")
    CYPHER_ARG_CHECK(args[0].IsString(), "label_name type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "field_name type should be string")
    CYPHER_ARG_CHECK(args[2].IsBool(), "unique type should be boolean")
    CYPHER_ARG_CHECK(args[3].IsBool(), "pair_unique type should be boolean")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    auto label = args[0].constant.scalar.AsString();
    auto field = args[1].constant.scalar.AsString();
    auto unique = args[2].constant.scalar.AsBool();
    auto pair_unique = args[3].constant.scalar.AsBool();
    if (unique && pair_unique) {
        THROW_CODE(InputError, "pair_unique and unique configuration cannot occur simultaneously)");
    }
    lgraph::IndexType type;
    if (unique) {
        type = lgraph::IndexType::GlobalUniqueIndex;
    } else if (pair_unique) {
        type = lgraph::IndexType::PairUniqueIndex;
    } else {
        type = lgraph::IndexType::NonuniqueIndex;
    }
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    bool success = ac_db.AddEdgeIndex(label, field, type);
    if (!success) {
        throw lgraph::IndexExistException(label, field);
    }
}

void BuiltinProcedure::DbAddFullTextIndex(RTContext *ctx, const Record *record,
                                          const VEC_EXPR &args, const VEC_STR &yield_items,
                                          std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() == 3,
        "need 3 parameters, e.g. db.addFullTextIndex(is_vertex, label_name, field_name)")
    CYPHER_ARG_CHECK(args[0].IsBool(), "is_vertex type should be boolean")
    CYPHER_ARG_CHECK(args[1].IsString(), "label_name type should be string")
    CYPHER_ARG_CHECK(args[2].IsString(), "field_name type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    auto is_vertex = args[0].constant.scalar.AsBool();
    auto label = args[1].constant.scalar.AsString();
    auto field = args[2].constant.scalar.AsString();
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    bool success = ac_db.AddFullTextIndex(is_vertex, label, field);
    if (!success) {
        throw lgraph::FullTextIndexExistException(label, field);
    }
}

void BuiltinProcedure::DbDeleteFullTextIndex(RTContext *ctx, const Record *record,
                                             const VEC_EXPR &args,
                                             const VEC_STR &yield_items,
                                             std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() == 3,
        "need 3 parameters, e.g. db.deleteFullTextIndex(is_vertex, label_name, field_name)");

    CYPHER_ARG_CHECK(args[0].IsBool(), "is_vertex type should be boolean")
    CYPHER_ARG_CHECK(args[1].IsString(), "label_name type should be string")
    CYPHER_ARG_CHECK(args[2].IsString(), "field_name type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    bool success =
        db.DeleteFullTextIndex(args[0].constant.scalar.AsBool(), args[1].constant.scalar.AsString(),
                               args[2].constant.scalar.AsString());
    if (!success) {
        throw lgraph::FullTextIndexNotExistException(args[1].constant.scalar.AsString(),
                                                     args[2].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbRebuildFullTextIndex(RTContext *ctx, const Record *record,
                                              const VEC_EXPR &args,
                                              const VEC_STR &yield_items,
                                              std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need 2 parameters, e.g. db.rebuildFullTextIndex(vertex_labels, edge_labels)");
    CYPHER_ARG_CHECK(args[0].IsString(), "vertex_labels should be a json array string")
    CYPHER_ARG_CHECK(args[1].IsString(), "edge_labels should be a json array string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    std::set<std::string> v_labels, e_labels;
    auto vs = nlohmann::json::parse(args[0].constant.scalar.AsString());
    if (!vs.is_array()) {
        THROW_CODE(InputError, "vertex_labels should be a json array string");
    }
    for (auto &item : vs) {
        v_labels.emplace(item);
    }
    auto es = nlohmann::json::parse(args[1].constant.scalar.AsString());
    if (!es.is_array()) {
        THROW_CODE(InputError, "edge_labels should be a json array string");
    }
    for (auto &item : es) {
        e_labels.emplace(item);
    }
    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    db.RebuildFullTextIndex(v_labels, e_labels);
}

void BuiltinProcedure::DbFullTextIndexes(RTContext *ctx, const Record *record,
                                         const VEC_EXPR &args, const VEC_STR &yield_items,
                                         std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.FullTextIndexes()",
                                           args.size()))
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    const auto &ft_indexs = ctx->txn_->GetTxn()->ListFullTextIndexes();
    for (const auto &ft_index : ft_indexs) {
        Record r;
        r.AddConstant(lgraph::FieldData(std::get<0>(ft_index)));
        r.AddConstant(lgraph::FieldData(std::get<1>(ft_index)));
        r.AddConstant(lgraph::FieldData(std::get<2>(ft_index)));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbClearEdgeConstraints(RTContext *ctx, const Record *record,
                                              const VEC_EXPR &args,
                                              const VEC_STR &yield_items,
                                              std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 1,
                     "need 1 parameters, e.g. db.clearEdgeConstraints(label_name)")
    CYPHER_ARG_CHECK(args[0].IsString(), "label_name type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    auto label = args[0].constant.scalar.AsString();
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    ac_db.ClearEdgeConstraints(label);
}

void BuiltinProcedure::DbAddEdgeConstraints(RTContext *ctx, const Record *record,
                                            const VEC_EXPR &args,
                                            const VEC_STR &yield_items,
                                            std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need 2 parameters, e.g. db.addEdgeConstraints(label_name, constraints)")
    CYPHER_ARG_CHECK(args[0].IsString(), "label_name type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "constraints type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    auto label = args[0].constant.scalar.AsString();
    auto constraints = args[1].constant.scalar.AsString();
    std::vector<std::pair<std::string, std::string>> edge_constraints;
    auto ec = nlohmann::json::parse(constraints);
    for (auto &item : ec) {
        edge_constraints.emplace_back(item[0], item[1]);
    }
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    ac_db.AddEdgeConstraints(label, edge_constraints);
}

void BuiltinProcedure::DbmsMetaCountDetail(RTContext *ctx, const Record *record,
                                           const VEC_EXPR &args, const VEC_STR &yield_items,
                                           std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.meta.countDetail()",
                                           args.size()))
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    const auto &counts = ctx->txn_->GetTxn()->countDetail();
    for (const auto &count : counts) {
        Record r;
        r.AddConstant(lgraph::FieldData(std::get<0>(count)));
        r.AddConstant(lgraph::FieldData(std::get<1>(count)));
        r.AddConstant(lgraph::FieldData(std::get<2>(count)));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbmsMetaCount(RTContext *ctx, const Record *record,
                                     const VEC_EXPR &args, const VEC_STR &yield_items,
                                     std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.meta.count()",
                                           args.size()))
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    const auto &counts = ctx->txn_->GetTxn()->countDetail();
    int64_t vertex_num = 0;
    int64_t edge_num = 0;
    for (const auto &count : counts) {
        if (std::get<0>(count)) {
            vertex_num += std::get<2>(count);
        } else {
            edge_num += std::get<2>(count);
        }
    }
    {
        Record r;
        r.AddConstant(lgraph::FieldData("vertex"));
        r.AddConstant(lgraph::FieldData(vertex_num));
        records->emplace_back(r.Snapshot());
    }
    {
        Record r;
        r.AddConstant(lgraph::FieldData("edge"));
        r.AddConstant(lgraph::FieldData(edge_num));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbmsMetaRefreshCount(RTContext *ctx, const Record *record,
                                            const VEC_EXPR &args,
                                            const VEC_STR &yield_items,
                                            std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.meta.refreshCount()",
                                           args.size()))
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    ac_db.RefreshCount();
}

void BuiltinProcedure::DbmsSecurityIsDefaultUserPassword(RTContext *ctx,
                                                         const cypher::Record *record,
                                                         const cypher::VEC_EXPR &args,
                                                         const cypher::VEC_STR &yield_items,
                                                         std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() == 0,
        "need 0 parameters, e.g. dbms.security.isDefaultUserPassword()")
    if (ctx->txn_) ctx->txn_->Abort();
    bool is_default_user_password = false;
    if (ctx->bolt_conn_) {
        auto session = (bolt::BoltSession*)(ctx->bolt_conn_->GetContext());
        is_default_user_password = session->using_default_user_password;
    }
    Record r;
    r.AddConstant(lgraph::FieldData(is_default_user_password));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsSecurityChangePassword(RTContext *ctx, const cypher::Record *record,
                                                  const cypher::VEC_EXPR &args,
                                                  const cypher::VEC_STR &yield_items,
                                                  std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() == 2,
        "need 2 parameters, e.g. dbms.security.changePassword(current_password, new_password)")
    CYPHER_ARG_CHECK(args[0].IsString(), "current_password type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "new_password type should be string")
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->ChangeCurrentPassword(
        ctx->user_, args[0].constant.scalar.AsString(), args[1].constant.scalar.AsString());
    if (!success) {
        throw lgraph::UserNotExistException(ctx->user_);
    }
}

void BuiltinProcedure::DbmsSecurityChangeUserPassword(RTContext *ctx,
                                                      const cypher::Record *record,
                                                      const cypher::VEC_EXPR &args,
                                                      const cypher::VEC_STR &yield_items,
                                                      std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() == 2,
        "need 2 parameters, e.g. dbms.security.changeUserPassword(user_name, new_password)")
    CYPHER_ARG_CHECK(args[0].IsString(), "user_name type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "new_password type should be string")

    bool success = ctx->galaxy_->ChangeUserPassword(ctx->user_, args[0].constant.scalar.AsString(),
                                                    args[1].constant.scalar.AsString());
    if (!success) {
        throw lgraph::UserNotExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbmsSecuritySetUserMemoryLimit(RTContext *ctx,
                                                      const cypher::Record *record,
                                                      const cypher::VEC_EXPR &args,
                                                      const cypher::VEC_STR &yield_items,
                                                      std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() == 2,
        "need 2 parameters, e.g. dbms.security.setUserMemoryLimit(user_name, memory_limit)")
    CYPHER_ARG_CHECK(args[0].IsString(), "user_name should be string")
    CYPHER_ARG_CHECK(args[1].IsInteger(), "User_Memory_Limit must be an integer");

    bool success = ctx->galaxy_->SetUserMemoryLimit(ctx->user_, args[0].constant.scalar.AsString(),
                                                    args[1].constant.scalar.integer());
    if (!success) {
        throw lgraph::UserNotExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbmsSecurityCreateUser(RTContext *ctx, const cypher::Record *record,
                                              const cypher::VEC_EXPR &args,
                                              const cypher::VEC_STR &yield_items,
                                              std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need 2 parameters, e.g. dbms.security.createUser(user_name, password)")
    CYPHER_ARG_CHECK(args[0].IsString(), "user_name type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "password type should be string")
    bool success = ctx->galaxy_->CreateUser(
        ctx->user_, args[0].constant.scalar.AsString(), args[1].constant.scalar.AsString(),
        args.size() == 2 ? "" : args[2].constant.scalar.AsString());
    if (!success) {
        throw lgraph::UserExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbmsSecurityDeleteUser(RTContext *ctx, const cypher::Record *record,
                                              const cypher::VEC_EXPR &args,
                                              const cypher::VEC_STR &yield_items,
                                              std::vector<cypher::Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 1,
                     "need 1 parameters, e.g. dbms.security.deleteUser(user_name)")
    CYPHER_ARG_CHECK(args[0].IsString(), "user_name type should be string")
    bool success = ctx->galaxy_->DeleteUser(ctx->user_, args[0].constant.scalar.AsString());
    if (!success) {
        throw lgraph::UserNotExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbmsSecurityListUsers(RTContext *ctx, const cypher::Record *record,
                                             const cypher::VEC_EXPR &args,
                                             const cypher::VEC_STR &yield_items,
                                             std::vector<cypher::Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.security.listUsers()",
                                           args.size()))
    std::map<std::string, lgraph::AclManager::UserInfo> us = ctx->galaxy_->ListUsers(ctx->user_);
    for (auto &u : us) {
        Record r;
        r.AddConstant(lgraph::FieldData(u.first));
        r.AddConstant(lgraph::FieldData(ValueToJson(u.second).serialize()));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbmsSecurityShowCurrentUser(RTContext *ctx, const cypher::Record *record,
                                                   const cypher::VEC_EXPR &args,
                                                   const cypher::VEC_STR &yield_items,
                                                   std::vector<cypher::Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.security.showCurrentUser()",
                                           args.size()))
    Record r;
    r.AddConstant(lgraph::FieldData(ctx->user_));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsSecurityHostWhitelistList(RTContext *ctx, const Record *record,
                                                     const VEC_EXPR &args,
                                                     const VEC_STR &yield_items,
                                                     std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.security.listAllowedHosts()",
                                           args.size()))
    for (auto &ip : ctx->galaxy_->GetIpWhiteList(ctx->user_)) {
        Record r;
        r.AddConstant(lgraph::FieldData(ip));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbmsSecurityHostWhitelistAdd(RTContext *ctx, const Record *record,
                                                    const VEC_EXPR &args,
                                                    const VEC_STR &yield_items,
                                                    std::vector<Record> *records) {
    CYPHER_ARG_CHECK(!args.empty(), "This function takes one or more string arguments.")

    std::vector<std::string> ips;
    butil::ip_t my_ip;
    for (auto &ip : args) {
        CYPHER_ARG_CHECK(ip.IsString(), "Host names must be strings.")
        CYPHER_ARG_CHECK(butil::str2ip(ip.constant.scalar.AsString().c_str(), &my_ip) == 0,
                         "Invalid host");
        ips.push_back(ip.constant.scalar.AsString());
    }
    Record r;
    r.AddConstant(lgraph::FieldData((int64_t)ctx->galaxy_->AddIpsToWhitelist(ctx->user_, ips)));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsSecurityHostWhitelistDelete(RTContext *ctx, const Record *record,
                                                       const VEC_EXPR &args,
                                                       const VEC_STR &yield_items,
                                                       std::vector<Record> *records) {
    CYPHER_ARG_CHECK(!args.empty(), "This function takes one or more string arguments.")

    std::vector<std::string> ips;
    for (auto &ip : args) {
        CYPHER_ARG_CHECK(ip.IsString(), "Host names must be strings.")
        ips.push_back(ip.constant.scalar.AsString());
    }
    Record r;
    r.AddConstant(
        lgraph::FieldData((int64_t)ctx->galaxy_->RemoveIpsFromWhitelist(ctx->user_, ips)));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsGraphCreateGraph(RTContext *ctx, const cypher::Record *record,
                                            const cypher::VEC_EXPR &args,
                                            const cypher::VEC_STR &yield_items,
                                            std::vector<cypher::Record> *records) {
    CYPHER_ARG_CHECK(args.size() >= 1, "This function takes one or more arguments");
    CYPHER_ARG_CHECK(args[0].IsString(), "graph_name must be string");

    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    lgraph::DBConfig config;
    if (args.size() >= 2) {
        CYPHER_ARG_CHECK(args[1].IsString(), "description gmust be string");
        config.desc = args[1].constant.scalar.AsString();
    }
    if (args.size() >= 3) {
        CYPHER_ARG_CHECK(args[2].IsInteger(), "Max_size_GB must be an integer");
        config.db_size = ((size_t)args[2].constant.scalar.integer()) << 30;
    }
    bool success =
        ctx->galaxy_->CreateGraph(ctx->user_, args[0].constant.scalar.AsString(), config);
    if (!success) {
        throw lgraph::GraphExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbmsGraphModGraph(RTContext *ctx, const cypher::Record *record,
                                         const cypher::VEC_EXPR &args,
                                         const cypher::VEC_STR &yield_items,
                                         std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() == 2,
        "This function takes exactly 2 arguments. e.g.  dbms.graph.modGraph(graph_name,config)");
    CYPHER_ARG_CHECK(args[0].IsString(), "graph_name must be string");
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    lgraph::DBConfig config;
    CYPHER_ARG_CHECK(args[1].IsMap(), "Illega must be map");
    lgraph::GraphManager::ModGraphActions act;
    act.mod_size = false;
    act.mod_desc = false;
    for (auto &kv : *args[1].constant.map) {
        if (kv.first == "max_size_GB") {
            act.mod_size = true;
            if (!kv.second.IsInteger())
                THROW_CODE(InputError, "Invalid value for max_size_GB: must be integer");
            act.max_size = kv.second.scalar.integer() << 30;
        } else if (kv.first == "description") {
            act.mod_desc = true;
            if (!kv.second.IsString())
                THROW_CODE(InputError, "Invalid value for description: must be string");
            act.desc = kv.second.AsString();
        } else {
            THROW_CODE(InputError, "Invalid config key: " + kv.first);
        }
    }
    bool success = ctx->galaxy_->ModGraph(ctx->user_, args[0].constant.scalar.AsString(), act);
    if (!success) {
        throw lgraph::GraphNotExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbmsGraphDeleteGraph(RTContext *ctx, const cypher::Record *record,
                                            const cypher::VEC_EXPR &args,
                                            const cypher::VEC_STR &yield_items,
                                            std::vector<cypher::Record> *records) {
    if (ctx->txn_) ctx->txn_->Abort();
    CYPHER_ARG_CHECK(
        args.size() == 1,
        "This function takes exactly 1 arguments. e.g.  dbms.graph.deleteGraph(graph_name)")
    CYPHER_ARG_CHECK(args[0].IsString(), "graph_name must be string")

    /* close the previous txn first, in case of nested transaction */
    bool success = ctx->galaxy_->DeleteGraph(ctx->user_, args[0].constant.scalar.AsString());
    if (!success) {
        throw lgraph::GraphNotExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbmsGraphListGraphs(RTContext *ctx, const cypher::Record *record,
                                           const cypher::VEC_EXPR &args,
                                           const cypher::VEC_STR &yield_items,
                                           std::vector<cypher::Record> *records) {
    if (ctx->txn_) ctx->txn_->Abort();
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.graph.listGraphs()",
                                           args.size()))
    auto graphs = ctx->galaxy_->ListGraphs(ctx->user_);
    for (auto &g : graphs) {
        Record r;
        r.AddConstant(lgraph::FieldData(g.first));
        r.AddConstant(lgraph::FieldData(lgraph::ValueToJson(g.second).serialize()));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbmsGraphListUserGraphs(RTContext *ctx, const cypher::Record *record,
                                               const cypher::VEC_EXPR &args,
                                               const cypher::VEC_STR &yield_items,
                                               std::vector<cypher::Record> *records) {
    if (ctx->txn_) ctx->txn_->Abort();
    CYPHER_ARG_CHECK(args.size() == 1, FMA_FMT("Function requires 1 arguments, but {} are "
                                               "given. Usage: dbms.graph.listUserGraphs(user_name)",
                                               args.size()))
    std::string user_name = args[0].constant.scalar.AsString();
    if ((ctx->user_ != user_name) && !ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    const std::map<std::string, lgraph::DBConfig> &graphs = ctx->galaxy_->ListGraphsInternal();
    std::map<std::string, lgraph::DBConfig> userGraphs;
    for (auto graph : graphs) {
        lgraph_api::AccessLevel acl = ctx->galaxy_->GetAcl(ctx->user_, user_name, graph.first);
        if (acl == lgraph_api::AccessLevel::NONE) {
            continue;
        }
        userGraphs.emplace(graph.first, graph.second);
    }
    for (auto &g : userGraphs) {
        Record r;
        r.AddConstant(lgraph::FieldData(g.first));
        r.AddConstant(lgraph::FieldData(lgraph::ValueToJson(g.second).serialize()));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbmsGraphGetGraphInfo(RTContext *ctx, const Record *record,
                                             const VEC_EXPR &args,
                                             const VEC_STR &yield_items,
                                             std::vector<cypher::Record> *records) {
    if (ctx->txn_) ctx->txn_->Abort();
    CYPHER_ARG_CHECK(args.size() == 1, FMA_FMT("This function takes exactly 1 arguments, but {} "
                                               "given. Usage: dbms.graph.getGraphInfo(graph_name)",
                                               args.size()))

    auto graph_ref = ctx->galaxy_->OpenGraph(ctx->user_, args[0].constant.scalar.AsString());
    const lgraph::DBConfig &conf = graph_ref.GetLightningGraph()->GetConfig();
    Record r;
    r.AddConstant(lgraph::FieldData(args[0].constant.scalar.AsString()));
    r.AddConstant(lgraph::FieldData(lgraph::ValueToJson(conf).serialize()));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsGraphGetGraphSchema(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args,
                                               const VEC_STR &yield_items,
                                               std::vector<cypher::Record> *records) {
    if (ctx->txn_) ctx->txn_->Abort();
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("This function takes exactly 0 arguments, but {} "
                                           "given. Usage: dbms.graph.getGraphSchema()",
                                           args.size()))
    auto db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    auto txn = db.CreateReadTxn();
    nlohmann::json graph_schema;
    graph_schema["schema"] = nlohmann::json::array();
    for (auto& name : txn.GetAllLabels(true)) {
        nlohmann::json node;
        node["label"] = name;
        node["type"] = "VERTEX";
        auto s = txn.GetSchema(name, true);
        node["primary"] = s->GetPrimaryField();
        node["detach_property"] = s->DetachProperty();
        for (auto& fd : s->GetFields()) {
            nlohmann::json property;
            property["name"] = fd.Name();
            property["type"] = lgraph_api::to_string(fd.Type());
            property["optional"] = fd.IsOptional();
            auto vi = fd.GetVertexIndex();
            if (vi) {
                property["index"] = true;
                property["unique"] = vi->IsUnique();
            }
            node["properties"].push_back(property);
        }
        graph_schema["schema"].push_back(node);
    }
    for (auto& name : txn.GetAllLabels(false)) {
        nlohmann::json edge;
        edge["label"] = name;
        edge["type"] = "EDGE";
        auto s = txn.GetSchema(name, false);
        edge["detach_property"] = s->DetachProperty();
        edge["constraints"] = nlohmann::json::array();
        for (auto& pairs : s->GetEdgeConstraints()) {
            edge["constraints"].push_back(std::vector<std::string>{pairs.first, pairs.second});
        }
        for (auto& fd : s->GetFields()) {
            nlohmann::json property;
            property["name"] = fd.Name();
            property["type"] = lgraph_api::to_string(fd.Type());
            property["optional"] = fd.IsOptional();
            auto vi = fd.GetEdgeIndex();
            if (vi) {
                property["index"] = true;
                property["unique"] = vi->IsUnique();
            }
            edge["properties"].push_back(property);
        }
        graph_schema["schema"].push_back(edge);
    }
    Record r;
    r.AddConstant(lgraph::FieldData(graph_schema.dump()));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsSystemInfo(RTContext *ctx, const cypher::Record *record,
                                      const cypher::VEC_EXPR &args,
                                      const cypher::VEC_STR &yield_items,
                                      std::vector<cypher::Record> *records) {
    if (ctx->txn_) ctx->txn_->Abort();
    std::string version;
    version.append(std::to_string(lgraph::_detail::VER_MAJOR))
        .append(".")
        .append(std::to_string(lgraph::_detail::VER_MINOR))
        .append(".")
        .append(std::to_string(lgraph::_detail::VER_PATCH));
    std::vector<std::pair<std::string, lgraph::FieldData>> info = {
        {lgraph::RestStrings::VER, lgraph::FieldData(version)},
        {lgraph::RestStrings::UP_TIME,
         lgraph::FieldData(ctx->sm_ ? ctx->sm_->GetUpTimeInSeconds() : 0.0)},
        {lgraph::RestStrings::BRANCH, lgraph::FieldData(GIT_BRANCH)},
        {lgraph::RestStrings::COMMIT, lgraph::FieldData(GIT_COMMIT_HASH)},
        {lgraph::RestStrings::WEB_COMMIT, lgraph::FieldData(WEB_GIT_COMMIT_HASH)},
        {lgraph::RestStrings::CPP_ID, lgraph::FieldData(CXX_COMPILER_ID)},
        {lgraph::RestStrings::CPP_VERSION, lgraph::FieldData(CXX_COMPILER_VERSION)},
        {lgraph::RestStrings::PYTHON_VERSION, lgraph::FieldData(PYTHON_LIB_VERSION)},
    };
    for (auto &i : info) {
        Record r;
        r.AddConstant(lgraph::FieldData(i.first));
        r.AddConstant(i.second);
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbmsConfigList(RTContext *ctx, const cypher::Record *record,
                                      const cypher::VEC_EXPR &args,
                                      const cypher::VEC_STR &yield_items,
                                      std::vector<cypher::Record> *records) {
    if (ctx->txn_) ctx->txn_->Abort();
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.config.list()",
                                           args.size()))
    const std::shared_ptr<lgraph::GlobalConfig> gc = ctx->galaxy_->GetGlobalConfigPtr();
    if (!gc) return;
    for (auto &kv : gc->ToFieldDataMap()) {
        Record r;
        r.AddConstant(lgraph::FieldData(kv.first));
        r.AddConstant(kv.second);
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbmsConfigUpdate(RTContext *ctx, const cypher::Record *record,
                                        const cypher::VEC_EXPR &args,
                                        const cypher::VEC_STR &yield_items,
                                        std::vector<cypher::Record> *records) {
     ctx->txn_.reset();
     ctx->ac_db_.reset();
     CYPHER_ARG_CHECK(args.size() == 1,
                      "need exactly one parameter. "
                      "e.g. dbms.config.update({durable: false, enable_audit_log: false})")
     CYPHER_ARG_CHECK(args[0].IsMap(), "role type should be map")
     std::map<std::string, lgraph::FieldData> kvs;
     for (auto &kv : *args[0].constant.map) {
         kvs[kv.first] = kv.second.scalar;
     }
     bool need_reload = ctx->galaxy_->UpdateConfig(ctx->user_, kvs);
     // restart galaxy in a separate thread
     if (need_reload) ctx->galaxy_->ReloadFromDisk();
}

void BuiltinProcedure::DbmsListBackupLogFiles(RTContext *ctx, const cypher::Record *record,
                                              const cypher::VEC_EXPR &args,
                                              const cypher::VEC_STR &yield_items,
                                              std::vector<cypher::Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.listBackupFiles()",
                                           args.size()))
    if (!ctx->sm_) THROW_CODE(InputError, "Cannot be called in embedded mode.");
    for (auto &f : ctx->sm_->ListBackupLogFiles()) {
        Record r;
        r.AddConstant(lgraph::FieldData(f));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbmsTakeSnapshot(RTContext *ctx, const cypher::Record *record,
                                        const cypher::VEC_EXPR &args,
                                        const cypher::VEC_STR &yield_items,
                                        std::vector<cypher::Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.takeSnapshot()",
                                           args.size()))
    if (!ctx->sm_) THROW_CODE(InputError, "Cannot be called in embedded mode.");
    std::string path = ctx->sm_->TakeSnapshot();
    Record r;
    r.AddConstant(lgraph::FieldData(path));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsSecurityListRoles(RTContext *ctx, const cypher::Record *record,
                                             const cypher::VEC_EXPR &args,
                                             const cypher::VEC_STR &yield_items,
                                             std::vector<cypher::Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.security.listRoles()",
                                           args.size()))
    if (ctx->txn_) ctx->txn_->Abort();
    std::map<std::string, lgraph::AclManager::RoleInfo> rs = ctx->galaxy_->ListRoles(ctx->user_);
    for (auto &u : rs) {
        Record r;
        r.AddConstant(lgraph::FieldData(u.first));
        auto &table = u.second.graph_access;
        auto it = table.begin();
        while (it != table.end()) {
            if (it->first == lgraph::_detail::META_GRAPH)
                table.erase(it++);
            else
                ++it;
        }
        r.AddConstant(lgraph::FieldData(ValueToJson(u.second).serialize()));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbmsSecurityCreateRole(RTContext *ctx, const Record *record,
                                              const VEC_EXPR &args,
                                              const VEC_STR &yield_items,
                                              std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. dbms.security.createRole(role, desc)");
    CYPHER_ARG_CHECK(args[0].IsString(), "role type should be string");
    CYPHER_ARG_CHECK(args[1].IsString(), "desc type should be string");
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->CreateRole(ctx->user_, args[0].constant.scalar.AsString(),
                                            args[1].constant.scalar.AsString());
    if (!success) {
        throw lgraph::RoleExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbmsSecurityDeleteRole(RTContext *ctx, const Record *record,
                                              const VEC_EXPR &args,
                                              const VEC_STR &yield_items,
                                              std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 1, "need one parameters, e.g. dbms.security.deleteRole(role)");
    CYPHER_ARG_CHECK(args[0].IsString(), "role type should be string");
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->DeleteRole(ctx->user_, args[0].constant.scalar.AsString());
    if (!success) {
        throw lgraph::RoleNotExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbmsSecurityGetUserInfo(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args,
                                               const VEC_STR &yield_items,
                                               std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 1, "need one parameters, e.g. dbms.security.getUserInfo(user)");
    CYPHER_ARG_CHECK(args[0].IsString(), "user type should be string");
    if (ctx->txn_) ctx->txn_->Abort();
    auto uinfo = ctx->galaxy_->GetUserInfo(ctx->user_, args[0].constant.scalar.AsString());
    Record r;
    r.AddConstant(lgraph::FieldData(ValueToJson(uinfo).serialize()));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsSecurityGetUserPermissions(RTContext *ctx, const Record *record,
                                                      const VEC_EXPR &args,
                                                      const VEC_STR &yield_items,
                                                      std::vector<cypher::Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 1,
                     "need one parameters, e.g. dbms.security.getUserPermissions(user_name)");
    CYPHER_ARG_CHECK(args[0].IsString(), "user_name type should be string");
    auto uinfo = ctx->galaxy_->ListUserGraphs(ctx->user_, args[0].constant.scalar.AsString());
    if (ctx->txn_) ctx->txn_->Abort();
    Record r;
    r.AddConstant(lgraph::FieldData(lgraph::ValueToJson(uinfo).serialize()));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsSecurityGetUserMemoryUsage(RTContext *ctx, const Record *record,
                                                      const VEC_EXPR &args,
                                                      const VEC_STR &yield_items,
                                                      std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 1, "need one parameters, e.g. dbms.security.getUserInfo(user)");
    CYPHER_ARG_CHECK(args[0].IsString(), "user type should be string");
    if (ctx->txn_) ctx->txn_->Abort();
    int64_t usage = AllocatorManager.GetMemoryUsage(ctx->user_);
    Record r;
    r.AddConstant(lgraph::FieldData(usage));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsSecurityGetRoleInfo(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args,
                                               const VEC_STR &yield_items,
                                               std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 1, "need one parameters, e.g. dbms.security.getRoleInfo(role)");
    CYPHER_ARG_CHECK(args[0].IsString(), "role type should be string");
    auto rinfo = ctx->galaxy_->GetRoleInfo(ctx->user_, args[0].constant.scalar.AsString());
    if (ctx->txn_) ctx->txn_->Abort();
    Record r;
    r.AddConstant(lgraph::FieldData(ValueToJson(rinfo).serialize()));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsSecurityDisableRole(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args,
                                               const VEC_STR &yield_items,
                                               std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. dbms.security.disableRole(role, disable)")
    CYPHER_ARG_CHECK(args[0].IsString(), "role type should be string")
    CYPHER_ARG_CHECK(args[1].IsBool(), "disable type should be boolean")
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->ModRoleDisable(args[0].constant.scalar.AsString(),
                                                args[1].constant.scalar.AsBool());
    if (!success) {
        throw lgraph::RoleNotExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbmsSecurityModRoleDesc(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args,
                                               const VEC_STR &yield_items,
                                               std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. dbms.security.modRoleDesc(role, description)")
    CYPHER_ARG_CHECK(args[0].IsString(), "role type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "description type should be string")
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->ModRoleDesc(args[0].constant.scalar.AsString(),
                                             args[1].constant.scalar.AsString());
    if (!success) {
        throw lgraph::RoleNotExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbmsSecurityRebuildRoleAccessLevel(RTContext *ctx, const Record *record,
                                                          const VEC_EXPR &args,
                                                          const VEC_STR &yield_items,
                                                          std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(
        args.size() == 2,
        "need two parameters, e.g. dbms.security.modAllRoleAccessLevel(role, access_level)")
    CYPHER_ARG_CHECK(args[0].IsString(), "role type should be string")
    CYPHER_ARG_CHECK(args[1].IsMap(), "access_level type should be map, key and val both string")
    std::string role;
    std::unordered_map<std::string, lgraph::AccessLevel> levels;
    _ExtractAccessLevel(args, role, levels);
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->ModAllRoleAccessLevel(role, levels);
    if (!success) {
        throw lgraph::RoleNotExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbmsSecurityModRoleAccessLevel(RTContext *ctx, const Record *record,
                                                      const VEC_EXPR &args,
                                                      const VEC_STR &yield_items,
                                                      std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(
        args.size() == 2,
        "need two parameters, e.g. dbms.security.modRoleAccessLevel(role, access_level)")
    CYPHER_ARG_CHECK(args[0].IsString(), "role type should be string")
    CYPHER_ARG_CHECK(args[1].IsMap(), "access_level type should be map, key and val both string")
    std::string role;
    std::unordered_map<std::string, lgraph::AccessLevel> levels;
    _ExtractAccessLevel(args, role, levels);
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->ModRoleAccessLevel(role, levels);
    if (!success) {
        throw lgraph::RoleNotExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbmsSecurityModRoleFieldAccessLevel(RTContext *ctx, const Record *record,
                                                           const VEC_EXPR &args,
                                                           const VEC_STR &yield_items,
                                                           std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 6,
                     "need five parameters, "
                     "e.g. dbms.security.modRoleFieldAccessLevel"
                     "(role, graph, label, field, label_type, field_access_level)")
    CYPHER_ARG_CHECK(args[0].IsString(), "role type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "graph type should be string")
    CYPHER_ARG_CHECK(args[2].IsString(), "label type should be string")
    CYPHER_ARG_CHECK(args[3].IsString(), "field type should be string")
    CYPHER_ARG_CHECK(args[4].IsString(), "label_type type should be string")
    CYPHER_ARG_CHECK(args[5].IsString(), "field_access_level type should be string")
    cypher::VEC_STR titles =
        ProcedureTitles("dbms.security.modRoleFieldAccessLevel", yield_items);
    std::string role = args[0].constant.scalar.AsString();
    std::string graph = args[1].constant.scalar.AsString();
    std::string label = args[2].constant.scalar.AsString();
    std::string field = args[3].constant.scalar.AsString();
    std::string label_type = args[4].constant.scalar.AsString();
    std::string field_access_level = args[5].constant.scalar.AsString();
    // is_vertex
    bool is_vertex = true;
    if (label_type == "VERTEX" || label_type == "vertex")
        is_vertex = true;
    else if (label_type == "EDGE" || label_type == "edge")
        is_vertex = false;
    else
        CYPHER_ARG_CHECK(false, "label_type should be VERTEX or EDGE.");
    // field_access_level
    auto it = ValidFieldAccessLevels.find(field_access_level);
    CYPHER_ARG_CHECK(it != ValidFieldAccessLevels.end(), "unknown access level");
    auto level = it->second;
    lgraph::AclManager::FieldAccessTable acs_table;
    lgraph::AclManager::FieldAccess acs;
    acs[lgraph::AclManager::LabelFieldSpec(is_vertex, label, field)] = level;
    acs_table[graph] = acs;
    if (ctx->txn_) ctx->txn_->Abort();
    if (!ctx->galaxy_->ModRoleFieldAccessLevel(role, acs_table))
        throw lgraph::RoleNotExistException(role);
}

void BuiltinProcedure::DbmsSecurityDisableUser(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args,
                                               const VEC_STR &yield_items,
                                               std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. dbms.security.disableUser(user, disable)");
    CYPHER_ARG_CHECK(args[0].IsString(), "user type should be string");
    CYPHER_ARG_CHECK(args[1].IsBool(), "disable type should be boolean");
    std::string modified_user = args[0].constant.scalar.AsString();
    if (ctx->txn_) ctx->txn_->Abort();
    bool success =
        ctx->galaxy_->ModUserDisable(ctx->user_, modified_user, args[1].constant.scalar.AsBool());
    if (!success) {
        throw lgraph::UserNotExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbmsSecuritySetCurrentDesc(RTContext *ctx, const Record *record,
                                                  const VEC_EXPR &args,
                                                  const VEC_STR &yield_items,
                                                  std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 1,
                     "need one parameters, e.g. dbms.security.setCurrentDesc(description)")
    CYPHER_ARG_CHECK(args[0].IsString(), "description type should be string")

    std::string modified_user = args[0].constant.scalar.AsString();
    if (modified_user != ctx->user_ && !ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Non-admin user cannot modify other users.");
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->SetUserDescription(ctx->user_, ctx->user_,
                                                    args[0].constant.scalar.AsString());
    if (!success) {
        throw lgraph::UserNotExistException(ctx->user_);
    }
}

void BuiltinProcedure::DbmsSecuritySetUserDesc(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args,
                                               const VEC_STR &yield_items,
                                               std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. dbms.security.setUserDesc(user, description)")
    CYPHER_ARG_CHECK(args[0].IsString(), "user type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "description type should be string")

    std::string modified_user = args[0].constant.scalar.AsString();
    if (modified_user != ctx->user_ && !ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Non-admin user cannot modify other users.");
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->SetUserDescription(ctx->user_, args[0].constant.scalar.AsString(),
                                                    args[1].constant.scalar.AsString());
    if (!success) {
        throw lgraph::UserNotExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbmsSecurityDeleteUserRoles(RTContext *ctx, const Record *record,
                                                   const VEC_EXPR &args,
                                                   const VEC_STR &yield_items,
                                                   std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. dbms.security.deleteUserRoles(user, roles)")
    CYPHER_ARG_CHECK(args[0].IsString(), "user type should be string")
    CYPHER_ARG_CHECK(args[1].IsArray(), "roles type should be list")
    std::vector<std::string> roles;
    for (auto &a : *args[1].constant.array) {
        if (_F_UNLIKELY(!a.IsString()))
            THROW_CODE(InputError, "Illegal value for roles: must be a string list .");
        roles.push_back(a.AsString());
    }
    if (ctx->txn_) ctx->txn_->Abort();
    bool success =
        ctx->galaxy_->DeleteUserRoles(ctx->user_, args[0].constant.scalar.AsString(), roles);
    if (!success) {
        throw lgraph::UserNotExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbmsSecurityRebuildUserRoles(RTContext *ctx, const Record *record,
                                                    const VEC_EXPR &args,
                                                    const VEC_STR &yield_items,
                                                    std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. dbms.security.rebuildUserRoles(user, roles)")
    CYPHER_ARG_CHECK(args[0].IsString(), "user type should be string")
    CYPHER_ARG_CHECK(args[1].IsArray(), "roles type should be list")
    std::vector<std::string> roles;
    for (auto &a : *args[1].constant.array) {
        if (_F_UNLIKELY(!a.IsString()))
            THROW_CODE(InputError, "Illegal value for roles: must be a string list .");
        roles.push_back(a.AsString());
    }
    if (ctx->txn_) ctx->txn_->Abort();
    bool success =
        ctx->galaxy_->RebuildUserRoles(ctx->user_, args[0].constant.scalar.AsString(), roles);
    if (!success) {
        throw lgraph::UserNotExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbmsSecurityAddUserRoles(RTContext *ctx, const Record *record,
                                                const VEC_EXPR &args,
                                                const VEC_STR &yield_items,
                                                std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. dbms.security.addUserRoles(user, roles)")
    CYPHER_ARG_CHECK(args[0].IsString(), "user type should be string")
    CYPHER_ARG_CHECK(args[1].IsArray(), "roles type should be list")
    std::vector<std::string> roles;
    for (auto &a : *args[1].constant.array) {
        if (_F_UNLIKELY(!a.IsString()))
            THROW_CODE(InputError, "Illegal value for roles: must be a string list .");
        roles.push_back(a.AsString());
    }
    if (ctx->txn_) ctx->txn_->Abort();
    bool success =
        ctx->galaxy_->AddUserRoles(ctx->user_, args[0].constant.scalar.AsString(), roles);
    if (!success) {
        throw lgraph::UserNotExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbPluginLoadPlugin(RTContext *ctx, const Record *record,
                                          const VEC_EXPR &args, const VEC_STR &yield_items,
                                          std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() == 7,
        "need seven parameters, e.g. db.plugin.loadPlugin(plugin_type,"
        "plugin_name, plugin_content, code_type, plugin_description, read_only, version)")
    CYPHER_ARG_CHECK(args[0].IsString(), "plugin_type type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "plugin_name type should be string")
    CYPHER_ARG_CHECK(args[3].IsString(), "code_type type should be string")
    CYPHER_ARG_CHECK(args[4].IsString(), "plugin_description type should be string")
    CYPHER_ARG_CHECK(args[5].IsBool(), "read_only type should be boolean")
    CYPHER_ARG_CHECK(args[6].IsString(), "version type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    auto plugin_type_it = ValidPluginType.find(args[0].constant.scalar.AsString());
    CYPHER_ARG_CHECK(plugin_type_it != ValidPluginType.end(),
                     "unknown plugin_type, one of ('CPP', 'PY')");
    auto code_type_it = ValidPluginCodeType.find(args[3].constant.scalar.AsString());
    CYPHER_ARG_CHECK(code_type_it != ValidPluginCodeType.end(),
                     "unknown plugin_type, one of ('PY', 'SO', 'CPP', 'ZIP')");
    bool success = false;
    fma_common::encrypt::Base64 base64;
    if (args[2].IsString()) {
        std::string content = base64.Decode(args[2].constant.scalar.AsString());
        success =
            db.LoadPlugin(plugin_type_it->second, ctx->user_, args[1].constant.scalar.AsString(),
                          std::vector<std::string>{content}, std::vector<std::string>{},
                          code_type_it->second, args[4].constant.scalar.AsString(),
                          args[5].constant.scalar.AsBool(), args[6].constant.scalar.AsString());
    } else if (args[2].IsMap()) {
        std::vector<std::string> filenames;
        std::vector<std::string> codes;
        for (auto &kv : *args[2].constant.map) {
            if (kv.first[0] == '`' && kv.first.back() == '`') {
                filenames.push_back(kv.first.substr(1, kv.first.size() - 2));
            } else {
                filenames.push_back(kv.first);
            }
            codes.push_back(base64.Decode(kv.second.AsString()));
        }
        success = db.LoadPlugin(
            plugin_type_it->second, ctx->user_, args[1].constant.scalar.AsString(), codes,
            filenames, code_type_it->second, args[4].constant.scalar.AsString(),
            args[5].constant.scalar.AsBool(), args[6].constant.scalar.AsString());
    } else {
        throw lgraph::ReminderException("plugin_content should be string or map");
    }

    if (!success) {
        throw lgraph::PluginExistException(args[1].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbPluginDeletePlugin(RTContext *ctx, const Record *record,
                                            const VEC_EXPR &args,
                                            const VEC_STR &yield_items,
                                            std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. db.plugin.deletePlugin(plugin_type, plugin_name)")
    CYPHER_ARG_CHECK(args[0].IsString(), "plugin_type type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "plugin_name type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    auto plugin_type_it = ValidPluginType.find(args[0].constant.scalar.AsString());
    CYPHER_ARG_CHECK(plugin_type_it != ValidPluginType.end(),
                     "unknown plugin_type, one of ('CPP', 'PY')")
    bool success =
        db.DelPlugin(plugin_type_it->second, ctx->user_, args[1].constant.scalar.AsString());
    if (!success) {
        throw lgraph::PluginNotExistException(args[1].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbPluginGetPluginInfo(RTContext *ctx, const Record *record,
                                             const VEC_EXPR &args,
                                             const VEC_STR &yield_items,
                                             std::vector<cypher::Record> *records) {
    CYPHER_ARG_CHECK((args.size() == 2 || args.size() == 3),
                     "need two or three parameters, e.g. db.plugin.getPluginInfo(plugin_type, "
                     "plugin_name,show_code=false)")
    CYPHER_ARG_CHECK(args[0].IsString(), "plugin_type type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "plugin_name type should be string")
    bool show_code = false;
    if (args.size() == 3) {
        CYPHER_ARG_CHECK(args[2].IsBool(), "show_code type should be boolean")
        show_code = args[2].constant.scalar.AsBool();
    }
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    auto plugin_type_it = ValidPluginType.find(args[0].constant.scalar.AsString());
    CYPHER_ARG_CHECK(plugin_type_it != ValidPluginType.end(),
                     "unknown plugin_type, one of ('CPP', 'PY')")
    lgraph::PluginCode co;
    bool success = db.GetPluginCode(plugin_type_it->second, ctx->user_,
                                    args[1].constant.scalar.AsString(), co);
    if (!success) {
        throw lgraph::PluginNotExistException(args[1].constant.scalar.AsString());
    }
    if (show_code) {
        std::string encoded = lgraph_api::base64::Encode(co.code);
        co.code = encoded;
    } else {
        co.code = "Not show code here.";
    }
    Record r;
    r.AddConstant(lgraph::FieldData(lgraph::ValueToJson(co).serialize()));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbPluginListPlugin(RTContext *ctx, const Record *record,
                                          const VEC_EXPR &args, const VEC_STR &yield_items,
                                          std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need 2 parameters, e.g. db.plugin.listPlugin(plugin_type, plugin_version)")
    CYPHER_ARG_CHECK(args[0].IsString(), "plugin_type type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    auto plugin_type_it = ValidPluginType.find(args[0].constant.scalar.AsString());
    CYPHER_ARG_CHECK(plugin_type_it != ValidPluginType.end(),
                     "unknown plugin_type, one of ('CPP', 'PY')")

    CYPHER_ARG_CHECK(args[1].IsString(), "plugin_version type should be string")
    std::string version = args[1].constant.scalar.AsString();
    CYPHER_ARG_CHECK(version == lgraph::plugin::PLUGIN_VERSION_1 ||
                         version == lgraph::plugin::PLUGIN_VERSION_2 ||
                         version == lgraph::plugin::PLUGIN_VERSION_ANY,
                     "unknown plugin_version, one of ('V1', 'V2', 'Any')")

    std::vector<lgraph::PluginDesc> descs = db.ListPlugins(plugin_type_it->second, ctx->user_);
    for (auto &d : descs) {
        if (version != lgraph::plugin::PLUGIN_VERSION_ANY && d.version != version) continue;
        Record r;
        r.AddConstant(lgraph::FieldData(ValueToJson(d).serialize()));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbPluginListUserPlugins(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args,
                                               const VEC_STR &yield_items,
                                               std::vector<Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.graph.listUserPlugins()",
                                           args.size()))
    std::unordered_map<std::string, lgraph::AccessControlledDB> dbs =
        ctx->galaxy_->OpenUserGraphs(ctx->user_, ctx->user_);
    for (auto &kv : ValidPluginType) {
        for (auto &db : dbs) {
            std::vector<lgraph::PluginDesc> descs = db.second.ListPlugins(kv.second, ctx->user_);
            for (auto &d : descs) {
                Record r;
                r.AddConstant(lgraph::FieldData(db.first));
                r.AddConstant(lgraph::FieldData(ValueToJson(d).serialize()));
                records->emplace_back(r.Snapshot());
            }
        }
    }
}

void BuiltinProcedure::DbPluginCallPlugin(RTContext *ctx, const Record *record,
                                          const VEC_EXPR &args, const VEC_STR &yield_items,
                                          std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 5,
                     "need five parameters, e.g. "
                     "db.plugin.callPlugin(plugin_type, plugin_name, param, timeout, in_process)")
    CYPHER_ARG_CHECK(args[0].IsString(), "plugin_type type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "plugin_name type should be string")
    CYPHER_ARG_CHECK(args[2].IsString(), "param type should be string")
    CYPHER_ARG_CHECK(args[3].IsReal(), "timeout type should be double")
    CYPHER_ARG_CHECK(args[4].IsBool(), "in_process type should be boolean")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    auto plugin_type_it = ValidPluginType.find(args[0].constant.scalar.AsString());
    CYPHER_ARG_CHECK(plugin_type_it != ValidPluginType.end(),
                     "unknown plugin_type, one of ('CPP', 'PY')")
    lgraph::PluginManager::PluginType type = plugin_type_it->second;
    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    std::string name = args[1].constant.scalar.AsString();
    lgraph::TimeoutTaskKiller timeout_killer;
    if (args[3].constant.scalar.AsDouble() > 0) {
        timeout_killer.SetTimeout(args[3].constant.scalar.AsDouble());
    }
    std::string res;
    if (!db.CallPlugin(ctx->txn_.get(), type, ctx->user_, name, args[2].constant.scalar.AsString(),
                       args[3].constant.scalar.AsDouble(), args[4].constant.scalar.AsBool(), res)) {
        throw lgraph::PluginNotExistException(name);
    }
    Record r;
    r.AddConstant(lgraph::FieldData(res));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbImportorDataImportor(RTContext *ctx, const Record *record,
                                              const VEC_EXPR &args,
                                              const VEC_STR &yield_items,
                                              std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 5,
                     "need five parameters, e.g. db.importor.dataImportor"
                     "(description, content, continue_on_error, thread_nums, delimiter)")
    CYPHER_ARG_CHECK(args[0].IsString(), "description type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "content type should be string")
    CYPHER_ARG_CHECK(args[2].IsBool(), "continue_on_error type should be boolean");
    CYPHER_ARG_CHECK(args[3].IsInteger(), "thread_nums type should be integer")
    CYPHER_ARG_CHECK(args[4].IsString(), "delimiter type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();

    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    if (db.GetAccessLevel() < lgraph::AccessLevel::WRITE)
        throw lgraph::CypherException("Need write permission to do import.");
    lgraph::import_v2::ImportOnline::Config config;
    config.continue_on_error = args[2].constant.scalar.AsBool();
    config.n_threads = args[3].constant.scalar.integer();
    config.delimiter = args[4].constant.scalar.AsString();
    fma_common::encrypt::Base64 base64;
    std::string desc = base64.Decode(args[0].constant.scalar.AsString());
    std::string content = base64.Decode(args[1].constant.scalar.AsString());
    std::string log = lgraph::import_v2::ImportOnline::HandleOnlineTextPackage(
        std::move(desc), std::move(content), db.GetLightningGraph(), config);
}

void BuiltinProcedure::DbImportorFullImportor(RTContext *ctx, const Record *record,
                                              const VEC_EXPR &args,
                                              const VEC_STR &yield_items,
                                              std::vector<Record> *records) {
    ctx->txn_.reset();
    ctx->ac_db_.reset();
    CYPHER_ARG_CHECK(args.size() == 1,
                     "need exactly one parameter. "
                     "e.g. db.importor.fullImportor({})")
    CYPHER_ARG_CHECK(args[0].IsMap(), "conf type should be map")
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");

    // parse parameter
    lgraph::import_v3::Importer::Config import_config_v3;
    std::map<std::string, lgraph::FieldData> full_import_conf;
    for (auto &kv : *args[0].constant.map) {
        full_import_conf[kv.first] = kv.second.scalar;
    }
    auto check = [&full_import_conf](const std::string &name, lgraph::FieldType type) {
        if (full_import_conf.count(name) && full_import_conf[name].GetType() != type)
            throw lgraph::CypherException("Option " + name + " is not " + to_string(type));
        return full_import_conf.count(name);
    };

    if (!full_import_conf.count("config_file"))
        THROW_CODE(InputError, "Option config_file does not exist.");
    if (!check("graph_name", lgraph::FieldType::STRING))
        full_import_conf["graph_name"] = lgraph::FieldData(lgraph::_detail::DEFAULT_GRAPH_DB_NAME);
    if (!check("delete_if_exists", lgraph::FieldType::BOOL))
        full_import_conf["delete_if_exists"] = lgraph::FieldData(false);
    if (!check("username", lgraph::FieldType::STRING))
        full_import_conf["username"] = lgraph::FieldData(lgraph::_detail::DEFAULT_ADMIN_NAME);
    if (!full_import_conf["delete_if_exists"].AsBool() &&
        ctx->galaxy_->ListGraphsInternal().count(full_import_conf["graph_name"].string())) {
        throw lgraph::CypherException("DB exists and cannot be deleted.");
    }

    std::mutex mu;
    std::condition_variable cv;
    bool ok_to_leave = false;
    std::string data_file_path, log, error;
    std::thread worker([&]() {
        std::lock_guard<std::mutex> l(mu);
        try {
            import_config_v3.db_dir = ctx->galaxy_->GetConfig().dir + "/.import_tmp";
            import_config_v3.graph = lgraph::_detail::DEFAULT_GRAPH_DB_NAME;
            import_config_v3.config_file = full_import_conf["config_file"].string();
            import_config_v3.user = full_import_conf["username"].string();
            if (check("password", lgraph::FieldType::STRING))
                import_config_v3.password = full_import_conf["password"].string();
            if (check("continue_on_error", lgraph::FieldType::BOOL))
                import_config_v3.continue_on_error = full_import_conf["continue_on_error"].AsBool();
            if (check("delimiter", lgraph::FieldType::STRING))
                import_config_v3.delimiter = full_import_conf["delimiter"].string();
            import_config_v3.delete_if_exists = true;
            if (check("parse_block_size", lgraph::FieldType::INT64))
                import_config_v3.parse_block_size = full_import_conf["parse_block_size"].AsInt64();
            if (check("parse_block_threads", lgraph::FieldType::INT64))
                import_config_v3.parse_block_threads =
                    full_import_conf["parse_block_threads"].AsInt64();
            if (check("parse_file_threads", lgraph::FieldType::INT64))
                import_config_v3.parse_file_threads =
                    full_import_conf["parse_file_threads"].AsInt64();
            if (check("generate_sst_threads", lgraph::FieldType::INT64))
                import_config_v3.generate_sst_threads =
                    full_import_conf["generate_sst_threads"].AsInt64();
            if (check("read_rocksdb_threads", lgraph::FieldType::INT64))
                import_config_v3.read_rocksdb_threads =
                    full_import_conf["read_rocksdb_threads"].AsInt64();
            if (check("vid_num_per_reading", lgraph::FieldType::INT64))
                import_config_v3.vid_num_per_reading =
                    full_import_conf["vid_num_per_reading"].AsInt64();
            if (check("max_size_per_reading", lgraph::FieldType::INT64))
                import_config_v3.max_size_per_reading =
                    full_import_conf["max_size_per_reading"].AsInt64();
            if (check("compact", lgraph::FieldType::BOOL))
                import_config_v3.compact = full_import_conf["compact"].AsBool();
            if (check("keep_vid_in_memory", lgraph::FieldType::BOOL))
                import_config_v3.keep_vid_in_memory =
                    full_import_conf["keep_vid_in_memory"].AsBool();
            if (check("enable_fulltext_index", lgraph::FieldType::BOOL))
                import_config_v3.enable_fulltext_index =
                    full_import_conf["enable_fulltext_index"].AsBool();
            if (check("fulltext_index_analyzer", lgraph::FieldType::STRING))
                import_config_v3.fulltext_index_analyzer =
                    full_import_conf["fulltext_index_analyzer"].string();
            import_config_v3.import_online = true;
            lgraph::import_v3::Importer importer(import_config_v3);
            importer.DoImportOffline();
            log = importer.OnlineFullImportLog();
            for (const auto &entry : std::filesystem::directory_iterator(import_config_v3.db_dir)) {
                if (entry.is_directory() && entry.path().filename().string() != ".meta" &&
                    std::filesystem::exists(entry.path().string() + "/data.mdb")) {
                    data_file_path = entry.path().string() + "/data.mdb";
                    break;
                }
            }
        } catch (std::exception &e) {
            error = e.what();
        }
        ok_to_leave = true;
        cv.notify_all();
    });
    worker.detach();
    std::unique_lock<std::mutex> l(mu);
    while (!ok_to_leave) cv.wait(l);
    auto &fs =
        fma_common::FileSystem::GetFileSystem(ctx->galaxy_->GetConfig().dir + "/.import_tmp");
    if (error.empty()) {
        lgraph::DBConfig dbConfig;
        dbConfig.dir = ctx->galaxy_->GetConfig().dir;
        dbConfig.name = full_import_conf["graph_name"].string();
        dbConfig.create_if_not_exist = true;
        ctx->galaxy_->CreateGraph(full_import_conf["username"].string(),
                                  full_import_conf["graph_name"].string(), dbConfig,
                                  data_file_path);
        fs.RemoveDir(ctx->galaxy_->GetConfig().dir + "/.import_tmp");
        Record r;
        r.AddConstant(lgraph::FieldData(log));
        records->emplace_back(r.Snapshot());
    } else {
        fs.RemoveDir(ctx->galaxy_->GetConfig().dir + "/.import_tmp");
        throw lgraph::CypherException(error);
    }
}

void BuiltinProcedure::DbImportorFullFileImportor(RTContext *ctx, const Record *record,
                                                  const VEC_EXPR &args,
                                                  const VEC_STR &yield_items,
                                                  std::vector<Record> *records) {
    ctx->txn_.reset();
    ctx->ac_db_.reset();
    CYPHER_ARG_CHECK((args.size() == 2 || args.size() == 3),
                     "need no more than three parameters and no less than two parameters. "
                     "e.g. db.importor.fullFileImportor({\"ha\",\"data.mdb\",[false]})")
    CYPHER_ARG_CHECK(args[0].IsString(), "graph_name type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "path type should be string")
    bool remote = false;
    if (args.size() == 3) {
        CYPHER_ARG_CHECK(args[2].IsBool(), "remote type should be bool")
        remote = args[2].constant.scalar.AsBool();
    }
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    std::string graph_name = args[0].constant.scalar.AsString(),
                path = args[1].constant.scalar.AsString();
    if (remote) {
        std::string outputFilename = ctx->galaxy_->GetConfig().dir + "/.import.file.data.mdb";
        std::string command = "wget -O " + outputFilename + " " + path;
        int result = std::system(command.c_str());
        if (result == 0) {
            LOG_INFO() << "Downloaded file saved as: " << outputFilename;
        } else {
            LOG_ERROR() << "Failed to download file";
            throw lgraph::CypherException("Failed to download file");
        }
        path = ctx->galaxy_->GetConfig().dir + "/.import.file.data.mdb";
    }

    lgraph::DBConfig dbConfig;
    dbConfig.dir = ctx->galaxy_->GetConfig().dir;
    dbConfig.name = graph_name;
    dbConfig.create_if_not_exist = true;
    bool success = ctx->galaxy_->CreateGraph(ctx->user_, graph_name, dbConfig, path);
    if (remote) {
        auto &fs_download = fma_common::FileSystem::GetFileSystem(ctx->galaxy_->GetConfig().dir +
                                                                  "/.import.file.data.mdb");
        fs_download.Remove(ctx->galaxy_->GetConfig().dir + "/.import.file.data.mdb");
    }
    if (!success) throw lgraph::GraphCreateException(args[0].constant.scalar.AsString());
}

void BuiltinProcedure::DbImportorSchemaImportor(RTContext *ctx, const Record *record,
                                                const VEC_EXPR &args,
                                                const VEC_STR &yield_items,
                                                std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 1,
                     "need one parameters, e.g. db.importor.schemaImportor(description)")
    CYPHER_ARG_CHECK(args[0].IsString(), "description type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();

    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    if (db.GetAccessLevel() < lgraph::AccessLevel::WRITE)
        throw lgraph::CypherException("Need write permission to do import.");
    fma_common::encrypt::Base64 base64;
    std::string desc = base64.Decode(args[0].constant.scalar.AsString());
    std::string log = ::lgraph::import_v2::ImportOnline::HandleOnlineSchema(std::move(desc), db);
}

void BuiltinProcedure::DbDeleteIndex(RTContext *ctx, const Record *record,
                                     const VEC_EXPR &args, const VEC_STR &yield_items,
                                     std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. db.deleteIndex(label_name, field_name)")
    CYPHER_ARG_CHECK(args[0].IsString(), "label_name type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "field_name type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->ac_db_->DeleteVertexIndex(args[0].constant.scalar.AsString(),
                                                  args[1].constant.scalar.AsString());
    if (!success) {
        throw lgraph::IndexNotExistException(args[0].constant.scalar.AsString(),
                                             args[1].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbDeleteEdgeIndex(RTContext *ctx, const Record *record,
                                         const VEC_EXPR &args, const VEC_STR &yield_items,
                                         std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. db.deleteIndex(label_name, field_name)")
    CYPHER_ARG_CHECK(args[0].IsString(), "label_name type should be string")
    CYPHER_ARG_CHECK(args[1].IsString(), "field_name type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->ac_db_->DeleteEdgeIndex(args[0].constant.scalar.AsString(),
                                                args[1].constant.scalar.AsString());
    if (!success) {
        throw lgraph::IndexNotExistException(args[0].constant.scalar.AsString(),
                                             args[1].constant.scalar.AsString());
    }
}

void BuiltinProcedure::DbDeleteCompositeIndex(RTContext *ctx, const Record *record,
                                              const VEC_EXPR &args,
                                              const VEC_STR &yield_items,
                                              std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. db.deleteIndex(label_name, field_name)")
    CYPHER_ARG_CHECK(args[0].IsString(), "label_name type should be string")
    CYPHER_ARG_CHECK(args[1].IsArray(), "field_names type should be list<string>")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    std::vector<std::string> fields;
    for (const auto &v : *args[1].constant.array) {
        fields.push_back(v.AsString());
    }
    bool success = ctx->ac_db_->DeleteVertexCompositeIndex(args[0].constant.AsString(), fields);
    if (!success) {
        throw lgraph::IndexNotExistException(args[0].constant.AsString(),
                                             args[1].constant.AsString());
    }
}

void BuiltinProcedure::DbFlushDB(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                 const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.flushDB()",
                                           args.size()))
    ctx->ac_db_->Flush();
}

void BuiltinProcedure::DbDropDB(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.dropDB()",
                                           args.size()))
    ctx->ac_db_->DropAllData();
}

void BuiltinProcedure::DbDropAllVertex(RTContext *ctx, const Record *record,
                                       const VEC_EXPR &args,
                                       const VEC_STR &yield_items,
                                       std::vector<Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.dropAllVertex()",
                                           args.size()))
    ctx->ac_db_->DropAllVertex();
}

void BuiltinProcedure::DbTaskListTasks(RTContext *ctx, const Record *record,
                                       const VEC_EXPR &args, const VEC_STR &yield_items,
                                       std::vector<cypher::Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.task.listTasks()",
                                           args.size()))
    if (ctx->txn_) ctx->txn_->Abort();
    std::map<std::string, lgraph::AclManager::RoleInfo> rs = ctx->galaxy_->ListRoles(ctx->user_);

    auto tasksInfo = lgraph::TaskTracker::GetInstance().ListRunningTasks();
    for (auto &t : tasksInfo) {
        Record r;
        r.AddConstant(lgraph::FieldData(ValueToJson(t).serialize()));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbTaskTerminateTask(RTContext *ctx, const Record *record,
                                           const VEC_EXPR &args, const VEC_STR &yield_items,
                                           std::vector<cypher::Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 1, "need one parameters, e.g. dbms.task.terminateTask(task_id)")
    CYPHER_ARG_CHECK(args[0].IsString(), "task_id type should be string")
    lgraph::TaskTracker::TaskId task_id = lgraph::TaskTracker::TaskId();
    task_id.FromString(args[0].constant.scalar.AsString());
    auto r = lgraph::TaskTracker::GetInstance().KillTask(task_id);
    switch (r) {
    case lgraph::TaskTracker::SUCCESS:
        return;
    case lgraph::TaskTracker::NOTFOUND:
        throw lgraph::TaskNotExistException(args[0].constant.scalar.AsString());
    case lgraph::TaskTracker::FAIL_TO_KILL:
        throw lgraph::TaskKilledFailedException(args[0].constant.scalar.AsString());
    default:
        return;
    }
}

void BuiltinProcedure::DbMonitorServerInfo(RTContext *ctx, const Record *record,
                                           const VEC_EXPR &args, const VEC_STR &yield_items,
                                           std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.monitor.serverInfo()",
                                           args.size()))
    Record r;
    fma_common::HardwareInfo::CPURate cpuRate = fma_common::HardwareInfo::GetCPURate();
    r.AddConstant(lgraph::FieldData(lgraph::ValueToJson(cpuRate).serialize()));

    struct fma_common::HardwareInfo::MemoryInfo memoryInfo;
    fma_common::HardwareInfo::GetMemoryInfo(memoryInfo);
    r.AddConstant(lgraph::FieldData(lgraph::ValueToJson(memoryInfo).serialize()));

    fma_common::HardwareInfo::DiskRate diskRate = fma_common::HardwareInfo::GetDiskRate();
    r.AddConstant(lgraph::FieldData(lgraph::ValueToJson(diskRate).serialize()));

    struct fma_common::DiskInfo diskInfo;
    fma_common::GetDiskInfo(diskInfo, ctx->galaxy_->GetConfig().dir.data());
    r.AddConstant(lgraph::FieldData(
        lgraph::ValueToJson(diskInfo, fma_common::GetDirSpace(ctx->galaxy_->GetConfig().dir.data()))
            .serialize()));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbMonitorTuGraphInfo(RTContext *ctx, const Record *record,
                                            const VEC_EXPR &args,
                                            const VEC_STR &yield_items,
                                            std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.monitor.tuGraphInfo()",
                                           args.size()))
    if (!ctx) CYPHER_INTL_ERR();
    Record r;
    r.AddConstant(lgraph::FieldData(ValueToJson(ctx->sm_->GetStats()).serialize()));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsHaClusterInfo(RTContext *ctx, const Record *record,
                                         const VEC_EXPR &args, const VEC_STR &yield_items,
                                         std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.ha.clusterInfo()",
                                           args.size()))
    if (ctx->txn_) ctx->txn_->Abort();
    if (!ctx->sm_->IsInHaMode())
        THROW_CODE(InputError, "The service should be started as a high availability cluster .");
    auto peers = ctx->sm_->ListPeers();
    Record r;
    r.AddConstant(lgraph::FieldData(lgraph::ValueToJson(peers).serialize()));
    r.AddConstant(lgraph::FieldData(ctx->sm_->IsCurrentMaster()));
    records->emplace_back(r.Snapshot());
}

static void _FetchPath(lgraph::Transaction &txn, size_t hops,
                       std::unordered_map<lgraph::VertexId, lgraph::VertexId> &parent,
                       std::unordered_map<lgraph::VertexId, lgraph::VertexId> &child,
                       lgraph::VertexId vid_from, lgraph::VertexId vid_a, lgraph::VertexId vid_b,
                       lgraph::VertexId vid_to, cypher::Path &path) {
    std::vector<lgraph::VertexId> vids;
    auto vid = vid_a;
    while (vid != vid_from) {
        vids.push_back(vid);
        vid = parent[vid];
    }
    vids.push_back(vid);
    std::reverse(vids.begin(), vids.end());
    vid = vid_b;
    while (vid != vid_to) {
        vids.push_back(vid);
        vid = child[vid];
    }
    vids.push_back(vid);
    if (hops != 0 && vids.size() != hops + 1) {
        throw lgraph::ReminderException("_FetchPath failed");
    }
    for (size_t i = 0; i < hops; i++) {
        // TODO(any): fake edges!
        path.Append(lgraph::EdgeUid(vids[i], vids[i + 1], 0, 0, 0));
    }
}

template <typename EIT>
static bool _Filter(lgraph::Transaction &txn, const EIT &eit,
                    const EDGE_PROPERTY_FILTER_T &edge_filter) {
    for (auto &kv1 : edge_filter) {
        auto field = txn.GetEdgeField(eit.GetUid(), kv1.first);
        for (auto &kv2 : kv1.second) {
            if ((kv2.first == "smaller_than" && field >= kv2.second) ||
                (kv2.first == "greater_than" && field <= kv2.second)) {
                return false;
            }
        }
    }
    return true;
}

struct EdgeFilter {
    std::string label;
    EDGE_PROPERTY_FILTER_T property_filter;
};

static std::vector<lgraph::VertexId> _GetNeighbors(lgraph::Transaction &txn, lgraph::VertexId vid,
                                                   const std::vector<EdgeFilter>& edge_filters,
                                                   const parser::LinkDirection &direction,
                                                   std::optional<size_t> per_node_limit) {
    auto vit = txn.GetVertexIterator(vid);
    CYPHER_THROW_ASSERT(vit.IsValid());
    std::vector<lgraph::VertexId> neighbors;
    if (edge_filters.empty()) {
        bool out_edge_left = false, in_edge_left = false;
        neighbors = vit.ListDstVids(nullptr, nullptr,
                                    per_node_limit.has_value() ? per_node_limit.value() : 10000,
                                    &out_edge_left, nullptr);
        auto srcIds = vit.ListSrcVids(nullptr, nullptr,
                                      per_node_limit.has_value() ? per_node_limit.value() : 10000,
                                      &in_edge_left, nullptr);
        if (out_edge_left || in_edge_left) {
            LOG_WARN() << "Result trimmed down because " << vid
                       << " has more than 10000 in/out neighbours";
        }
        if (direction == parser::LinkDirection::RIGHT_TO_LEFT) {
            neighbors = srcIds;
        } else if (direction == parser::LinkDirection::DIR_NOT_SPECIFIED) {
            neighbors.reserve(neighbors.size() + srcIds.size());
            neighbors.insert(neighbors.end(), srcIds.begin(), srcIds.end());
        }
    } else {
        if (direction == parser::LinkDirection::LEFT_TO_RIGHT ||
            direction == parser::LinkDirection::DIR_NOT_SPECIFIED) {
            for (auto& edge_filter : edge_filters) {
                size_t count = 0;
                auto edge_lid = txn.GetLabelId(false, edge_filter.label);
                for (auto eit = vit.GetOutEdgeIterator(
                         lgraph::EdgeUid(vid, 0, edge_lid, 0, 0), true);
                     eit.IsValid(); eit.Next()) {
                    if (eit.GetLabelId() != edge_lid) break;
                    count += 1;
                    if (per_node_limit.has_value() && count > per_node_limit.value()) {
                        break;
                    }
                    if (!_Filter(txn, eit, edge_filter.property_filter)) continue;
                    neighbors.push_back(eit.GetDst());
                }
            }
        }
        if (direction == parser::LinkDirection::RIGHT_TO_LEFT ||
            direction == parser::LinkDirection::DIR_NOT_SPECIFIED) {
            for (auto& edge_filter : edge_filters) {
                size_t count = 0;
                auto edge_lid = txn.GetLabelId(false, edge_filter.label);
                for (auto eit = vit.GetInEdgeIterator(
                         lgraph::EdgeUid(0, vid, edge_lid, 0, 0), true);
                     eit.IsValid(); eit.Next()) {
                    if (eit.GetLabelId() != edge_lid) break;
                    count += 1;
                    if (per_node_limit.has_value() && count > per_node_limit.value()) {
                        break;
                    }
                    if (!_Filter(txn, eit, edge_filter.property_filter)) continue;
                    neighbors.push_back(eit.GetSrc());
                }
            }
        }
    }
    return neighbors;
}

/* All the shortestPath procedures assume that they are being executed on undirected graphs.
 * Just like described in neo4j:
 *   As of the 3.4.7.0 release, all the Shortest Path procedures assume that they are being executed
 * on undirected graphs. Therefore, the direction of a relationship can be ignored. If you are
 * running these algorithms on a graph where the direction is important, you can use the direction
 * parameter. For example, direction:"INCOMING" or direction:"OUTGOING".
 */
static void _P2PUnweightedShortestPath(lgraph::Transaction &txn, lgraph::VertexId start_vid,
                                       lgraph::VertexId end_vid,
                                       const std::vector<EdgeFilter> &edge_filters,
                                       size_t max_hops, cypher::Path &path,
                                       parser::LinkDirection &dir,
                                       std::optional<size_t> per_node_limit) {
    path.Clear();
    path.SetStart(start_vid);
    if (start_vid == end_vid) return;
    std::unordered_map<lgraph::VertexId, lgraph::VertexId> parent{{start_vid, start_vid}};
    std::unordered_map<lgraph::VertexId, lgraph::VertexId> child{{end_vid, end_vid}};
    std::vector<lgraph::VertexId> forward_q{start_vid};
    std::vector<lgraph::VertexId> backward_q{end_vid};
    size_t hops = 0;
    parser::LinkDirection forward_dir = dir;
    parser::LinkDirection backward_dir =
        dir == parser::LinkDirection::RIGHT_TO_LEFT   ? parser::LinkDirection::LEFT_TO_RIGHT
        : dir == parser::LinkDirection::LEFT_TO_RIGHT ? parser::LinkDirection::RIGHT_TO_LEFT
                                                      : parser::LinkDirection::DIR_NOT_SPECIFIED;
    while (hops++ < max_hops) {
        std::vector<lgraph::VertexId> next_q;
        // decide which way to search first
        if (forward_q.size() <= backward_q.size()) {
            // search forward
            for (auto vid : forward_q) {
                auto nbrs =
                    _GetNeighbors(txn, vid, edge_filters, forward_dir, per_node_limit);
                for (auto nbr : nbrs) {
                    if (child.find(nbr) != child.end()) {
                        // found the path
                        _FetchPath(txn, hops, parent, child, start_vid, vid, nbr, end_vid, path);
                        return;
                    }
                    auto it = parent.find(nbr);
                    if (it == parent.end()) {
                        parent.emplace_hint(it, nbr, vid);
                        next_q.push_back(nbr);
                    }
                }
            }
            if (next_q.empty()) break;
            forward_q = std::move(next_q);
        } else {
            for (auto vid : backward_q) {
                auto nbrs =
                    _GetNeighbors(txn, vid, edge_filters, backward_dir, per_node_limit);
                for (auto nbr : nbrs) {
                    if (parent.find(nbr) != parent.end()) {
                        // found the path
                        _FetchPath(txn, hops, parent, child, start_vid, nbr, vid, end_vid, path);
                        return;
                    }
                    auto it = child.find(nbr);
                    if (it == child.end()) {
                        child.emplace_hint(it, nbr, vid);
                        next_q.push_back(nbr);
                    }
                }
            }
            if (next_q.empty()) break;
            backward_q = std::move(next_q);
        }
    }
}

template <typename EIT>
static void _EmplaceForwardNeighbor(
    const EIT &eit, const int fhop, const std::unordered_map<lgraph::VertexId, int> &child,
    std::unordered_map<lgraph::VertexId, int> &parent,
    std::vector<std::tuple<lgraph::VertexId, lgraph::VertexId, lgraph::LabelId, lgraph::EdgeId,
                           bool>> &hits,
    std::vector<lgraph::VertexId> &next_q) {
    lgraph::VertexId vid, nbr;
    bool direction;
    if (std::is_same<EIT, ::lgraph::graph::OutEdgeIterator>::value) {
        vid = eit.GetSrc();
        nbr = eit.GetDst();
        direction = true;
    } else {
        vid = eit.GetDst();
        nbr = eit.GetSrc();
        direction = false;
    }
    if (child.find(nbr) != child.end()) {
        hits.emplace_back(vid, nbr, eit.GetLabelId(), eit.GetEdgeId(), direction);
    } else {
        auto it = parent.find(nbr);
        if (it == parent.end()) {
            parent.emplace_hint(it, nbr, fhop);
            next_q.emplace_back(nbr);
        }
    }
}

template <typename EIT>
static void _EmplaceBackwardNeighbor(
    const EIT &eit, const int bhop, const std::unordered_map<lgraph::VertexId, int> &parent,
    std::unordered_map<lgraph::VertexId, int> &child,
    std::vector<std::tuple<lgraph::VertexId, lgraph::VertexId, lgraph::LabelId, lgraph::EdgeId,
                           bool>> &hits,
    std::vector<lgraph::VertexId> &next_q) {
    lgraph::VertexId vid, nbr;
    bool direction;
    if (std::is_same<EIT, ::lgraph::graph::OutEdgeIterator>::value) {
        vid = eit.GetSrc();
        nbr = eit.GetDst();
        direction = false;
    } else {
        vid = eit.GetDst();
        nbr = eit.GetSrc();
        direction = true;
    }
    if (parent.find(nbr) != parent.end()) {
        hits.emplace_back(nbr, vid, eit.GetLabelId(), eit.GetEdgeId(), direction);
    } else {
        auto it = child.find(nbr);
        if (it == child.end()) {
            child.emplace_hint(it, nbr, bhop);
            next_q.emplace_back(nbr);
        }
    }
}

void _EnumeratePartialPaths(lgraph::Transaction &txn,
                            const std::unordered_map<int64_t, int> &hop_info, const int64_t vid,
                            const int depth, const std::vector<std::string> &edge_labels,
                            cypher::Path &path,
                            std::vector<cypher::Path> &paths) {
    if (depth != 0) {
        if (edge_labels.empty()) {
            for (auto eit = txn.GetOutEdgeIterator(vid); eit.IsValid(); eit.Next()) {
                int64_t nbr = eit.GetDst();
                auto it = hop_info.find(nbr);
                if (it != hop_info.end() && it->second == depth - 1) {
                    path.Append(eit.GetUid());
                    _EnumeratePartialPaths(txn, hop_info, nbr, depth - 1, edge_labels,
                                              path, paths);
                    path.PopBack();
                }
            }
            for (auto eit = txn.GetInEdgeIterator(vid); eit.IsValid(); eit.Next()) {
                int64_t nbr = eit.GetSrc();
                auto it = hop_info.find(nbr);
                if (it != hop_info.end() && it->second == depth - 1) {
                    path.Append(eit.GetUid());
                    _EnumeratePartialPaths(txn, hop_info, nbr, depth - 1, edge_labels,
                                              path, paths);
                    path.PopBack();
                }
            }
        } else {
            for (auto& edge_label : edge_labels) {
                auto edge_lid = txn.GetLabelId(false, edge_label);
                for (auto eit =
                         txn.GetOutEdgeIterator(lgraph::EdgeUid(vid, 0, edge_lid, 0, 0), true);
                     eit.IsValid(); eit.Next()) {
                    if (eit.GetLabelId() != edge_lid) break;
                    int64_t nbr = eit.GetDst();
                    auto it = hop_info.find(nbr);
                    if (it != hop_info.end() && it->second == depth - 1) {
                        path.Append(eit.GetUid());
                        _EnumeratePartialPaths(txn, hop_info, nbr, depth - 1, edge_labels, path,
                                               paths);
                        path.PopBack();
                    }
                }
                for (auto eit =
                         txn.GetInEdgeIterator(lgraph::EdgeUid(0, vid, edge_lid, 0, 0), true);
                     eit.IsValid(); eit.Next()) {
                    if (eit.GetLabelId() != edge_lid) break;
                    int64_t nbr = eit.GetSrc();
                    auto it = hop_info.find(nbr);
                    if (it != hop_info.end() && it->second == depth - 1) {
                        path.Append(eit.GetUid());
                        _EnumeratePartialPaths(txn, hop_info, nbr, depth - 1, edge_labels, path,
                                                  paths);
                        path.PopBack();
                    }
                }
            }
        }
    } else {
        paths.emplace_back(path);
    }
}

static void _P2PUnweightedAllShortestPaths(lgraph::Transaction &txn, lgraph::VertexId start_vid,
                                           lgraph::VertexId end_vid,
                                           const std::vector<std::string> &edge_labels,
                                           std::vector<cypher::Path> &paths) {
    // TODO(heng): fix PrimaryId
    if (start_vid == end_vid) {
        paths.emplace_back(cypher::Path());
        paths.back().SetStart(start_vid);
        return;
    }
    std::unordered_map<lgraph::VertexId, int> parent{{start_vid, 0}};
    std::unordered_map<lgraph::VertexId, int> child{{end_vid, 0}};
    std::vector<lgraph::VertexId> forward_q{start_vid};
    std::vector<lgraph::VertexId> backward_q{end_vid};
    int fhop = 0;
    int bhop = 0;
    // <fvid, bvid, lid, eid, direction>
    std::vector<
        std::tuple<lgraph::VertexId, lgraph::VertexId, lgraph::LabelId, lgraph::EdgeId, bool>>
        hits;
    for (int hop = 0; !forward_q.empty() && !backward_q.empty() && hits.empty(); hop++) {
        std::vector<lgraph::VertexId> next_q;
        if (forward_q.size() <= backward_q.size()) {
            fhop++;
            for (auto vid : forward_q) {
                if (edge_labels.empty()) {
                    for (auto eit = txn.GetOutEdgeIterator(vid); eit.IsValid(); eit.Next()) {
                        _EmplaceForwardNeighbor(eit, fhop, child, parent, hits, next_q);
                    }
                    for (auto eit = txn.GetInEdgeIterator(vid); eit.IsValid(); eit.Next()) {
                        _EmplaceForwardNeighbor(eit, fhop, child, parent, hits, next_q);
                    }
                } else {
                    for (auto& edge_label : edge_labels) {
                        auto edge_lid = txn.GetLabelId(false, edge_label);
                        for (auto eit = txn.GetOutEdgeIterator(
                                 lgraph::EdgeUid(vid, 0, edge_lid, 0, 0), true);
                             eit.IsValid(); eit.Next()) {
                            if (eit.GetLabelId() != edge_lid) break;
                            _EmplaceForwardNeighbor(eit, fhop, child, parent, hits, next_q);
                        }
                        for (auto eit = txn.GetInEdgeIterator(
                                 lgraph::EdgeUid(0, vid, edge_lid, 0, 0), true);
                             eit.IsValid(); eit.Next()) {
                            if (eit.GetLabelId() != edge_lid) break;
                            _EmplaceForwardNeighbor(eit, fhop, child, parent, hits, next_q);
                        }
                    }
                }
            }
            std::sort(next_q.begin(), next_q.end());
            forward_q.swap(next_q);
        } else {
            bhop++;
            for (auto vid : backward_q) {
                if (edge_labels.empty()) {
                    for (auto eit = txn.GetOutEdgeIterator(vid); eit.IsValid(); eit.Next()) {
                        _EmplaceBackwardNeighbor(eit, bhop, parent, child, hits, next_q);
                    }
                    for (auto eit = txn.GetInEdgeIterator(vid); eit.IsValid(); eit.Next()) {
                        _EmplaceBackwardNeighbor(eit, bhop, parent, child, hits, next_q);
                    }
                } else {
                    for (auto& edge_label : edge_labels) {
                        auto edge_lid = txn.GetLabelId(false, edge_label);
                        for (auto eit = txn.GetOutEdgeIterator(
                                 lgraph::EdgeUid(vid, 0, edge_lid, 0, 0), true);
                             eit.IsValid(); eit.Next()) {
                            if (eit.GetLabelId() != edge_lid) break;
                            _EmplaceBackwardNeighbor(eit, bhop, parent, child, hits, next_q);
                        }
                        for (auto eit = txn.GetInEdgeIterator(
                                 lgraph::EdgeUid(0, vid, edge_lid, 0, 0), true);
                             eit.IsValid(); eit.Next()) {
                            if (eit.GetLabelId() != edge_lid) break;
                            _EmplaceBackwardNeighbor(eit, bhop, parent, child, hits, next_q);
                        }
                    }
                }
            }
            std::sort(next_q.begin(), next_q.end());
            backward_q.swap(next_q);
        }
    }
    for (auto &hit : hits) {
        std::vector<cypher::Path> fpaths;
        std::vector<cypher::Path> bpaths;
        auto fvid = std::get<0>(hit);
        auto bvid = std::get<1>(hit);
        cypher::Path path;
        path.SetStart(fvid);
        _EnumeratePartialPaths(txn, parent, fvid, parent[fvid], edge_labels, path, fpaths);
        path.Clear();
        path.SetStart(bvid);
        _EnumeratePartialPaths(txn, child, bvid, child[bvid], edge_labels, path, bpaths);
        for (auto &fpath : fpaths) {
            fpath.Reverse();
            fpath.Append(std::get<4>(hit)
                             ? lgraph::EdgeUid(fvid, bvid, std::get<2>(hit), 0, std::get<3>(hit))
                             : lgraph::EdgeUid(bvid, fvid, std::get<2>(hit), 0, std::get<3>(hit)));
            for (auto &bpath : bpaths) {
                paths.emplace_back(fpath);
                for (int i = 0; i < (int)bpath.Length(); i++)
                    paths.back().Append(bpath.GetNthEdge(i));
            }
        }
    }
}

void AlgoFunc::ShortestPath(RTContext *ctx, const Record *record, const cypher::VEC_EXPR &args,
                            const cypher::VEC_STR &yield_items,
                            std::vector<cypher::Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    CYPHER_ARG_CHECK(args.size() / 2 == 1, "wrong arguments number")
    CYPHER_ARG_CHECK(args[0].IsNode() && args[1].IsNode() && (args.size() == 2 || args[2].IsMap()),
                     "wrong type")
    std::vector<EdgeFilter> edge_filters;
    parser::LinkDirection direction = parser::LinkDirection::DIR_NOT_SPECIFIED;
    size_t max_hops = 20;
    if (args.size() == 3) {
        auto &map = *args[2].constant.map;
        // edgeFilter
        auto iter = map.find("relationshipQuery");
        if (iter != map.end()) {
            if (!iter->second.IsArray() && !iter->second.IsString()) CYPHER_TODO();
            if (iter->second.IsString()) {
                EdgeFilter e_filter;
                e_filter.label = iter->second.AsString();
                edge_filters.emplace_back(std::move(e_filter));
            } else {
                for (auto& item : *iter->second.array) {
                    if (!item.IsMap()) CYPHER_TODO();
                    auto edge_filter = item.map;
                    auto label_iter = edge_filter->find("label");
                    if (label_iter == edge_filter->end()) CYPHER_TODO();
                    EdgeFilter e_filter;
                    e_filter.label = label_iter->second.AsString();
                    auto property_filter_iter = edge_filter->find("property_filter");
                    if (property_filter_iter != edge_filter->end()) {
                        if (!property_filter_iter->second.IsMap()) CYPHER_TODO();
                        for (auto &kv : *property_filter_iter->second.map) {
                            if (!kv.second.IsMap()) CYPHER_TODO();
                            std::unordered_map<std::string, lgraph::FieldData> filter_t;
                            for (auto& filter_pair : *kv.second.map) {
                                filter_t.emplace(filter_pair.first, filter_pair.second.scalar);
                            }
                            e_filter.property_filter.emplace(kv.first, filter_t);
                        }
                    }
                    edge_filters.emplace_back(std::move(e_filter));
                }
            }
        }
        auto max = map.find("maxHops");
        if (max != map.end()) {
            if (!max->second.IsInteger()) CYPHER_TODO();
            max_hops = max->second.scalar.integer();
        }
        // direction
        auto it_dir = map.find("direction");
        if (it_dir != map.end()) {
            if (!it_dir->second.IsString()) CYPHER_TODO();
            if (it_dir->second.AsString() == "PointingLeft") {
                direction = parser::LinkDirection::RIGHT_TO_LEFT;
            } else if (it_dir->second.AsString() == "PointingRight") {
                direction = parser::LinkDirection::LEFT_TO_RIGHT;
            } else if (it_dir->second.AsString() == "AnyDirection") {
                direction = parser::LinkDirection::DIR_NOT_SPECIFIED;
            } else {
                CYPHER_TODO();
            }
        }
    }
    CYPHER_THROW_ASSERT(record);
    auto start_vid = args[0].node->PullVid();
    auto end_vid = args[1].node->PullVid();
    cypher::Path path;
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    _P2PUnweightedShortestPath(*ctx->txn_->GetTxn(), start_vid, end_vid,
                               edge_filters, max_hops, path,
                                direction, std::nullopt);

    auto pp = global_ptable.GetProcedure("algo.shortestPath");
    CYPHER_THROW_ASSERT(pp && pp->ContainsYieldItem("nodeCount") &&
                        pp->ContainsYieldItem("totalCost") && pp->ContainsYieldItem("path"));
    cypher::VEC_STR titles = ProcedureTitles("algo.shortestPath", yield_items);
    std::unordered_map<std::string, std::function<void(Record &)>> lmap = {
        {"nodeCount",
         [&](Record &r) {
             r.AddConstant(lgraph::FieldData(
                 static_cast<int32_t>(path.Length() == 0 ? 0 : path.Length() + 1)));
         }},
        {"totalCost",
         [&](Record &r) { r.AddConstant(lgraph::FieldData(static_cast<float>(path.Length()))); }},
        {"path", [&](Record &r) { r.AddConstant(lgraph::FieldData(path.ToString())); }}};
    Record r;
    for (auto &title : titles) {
        lmap.find(title)->second(r);
    }
    records->emplace_back(r.Snapshot());
}

void AlgoFunc::AllShortestPaths(RTContext *ctx, const Record *record,
                                const cypher::VEC_EXPR &args,
                                const cypher::VEC_STR &yield_items,
                                std::vector<cypher::Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    CYPHER_ARG_CHECK(args.size() / 2 == 1, "wrong arguments number")
    CYPHER_ARG_CHECK(args[0].IsNode() && args[1].IsNode() && (args.size() == 2 || args[2].IsMap()),
                     "wrong type")
    std::vector<std::string> edge_labels;
    if (args.size() == 3) {
        auto &map = *args[2].constant.map;
        auto it = map.find("relationshipQuery");
        if (it != map.end()) {
            if (!it->second.IsArray() && !it->second.IsString()) CYPHER_TODO();
            if (it->second.IsString()) {
                edge_labels.push_back(it->second.AsString());
            } else {
                for (auto &item : *it->second.array) {
                    if (!item.IsMap()) CYPHER_TODO();
                    auto &edge_filter = item.map;
                    auto label_iter = edge_filter->find("label");
                    if (label_iter == edge_filter->end()) CYPHER_TODO();
                    edge_labels.push_back(label_iter->second.AsString());
                }
            }
        }
    }
    CYPHER_THROW_ASSERT(record);
    auto start_vid = args[0].node->PullVid();
    auto end_vid = args[1].node->PullVid();
    std::vector<cypher::Path> paths;
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    _P2PUnweightedAllShortestPaths(*ctx->txn_->GetTxn(), start_vid, end_vid, edge_labels, paths);

    auto pp = global_ptable.GetProcedure("algo.allShortestPaths");
    CYPHER_THROW_ASSERT(pp && pp->ContainsYieldItem("nodeIds") &&
                        pp->ContainsYieldItem("relationshipIds") && pp->ContainsYieldItem("cost"));
    cypher::VEC_STR titles = ProcedureTitles("algo.allShortestPaths", yield_items);
    std::unordered_map<std::string, std::function<void(const cypher::Path &, Record &)>> lmap = {
        {"nodeIds",
         [&](const cypher::Path &path, Record &r) {
             auto ids = cypher::FieldData::Array(0);
             for (int i = 0; i <= (int)path.Length(); i++) {
                 ids.array->emplace_back(lgraph::FieldData(static_cast<int64_t>(path.ids_[i * 2])));
             }
             r.AddConstant(ids);
         }},
        {"relationshipIds",
         [&](const cypher::Path &path, Record &r) {
             auto ids = cypher::FieldData::Array(0);
             for (int i = 0; i < (int)path.Length(); i++) {
                 ids.array->emplace_back(
                     lgraph::FieldData(_detail::EdgeUid2String(path.GetNthEdge(i))));
             }
             r.AddConstant(ids);
         }},
        {"cost",
         [&](const cypher::Path &path, Record &r) {
             r.AddConstant(lgraph::FieldData(static_cast<float>(path.Length())));
         }},
    };
    for (auto &path : paths) {
        Record r;
        for (auto &title : titles) {
            lmap.find(title)->second(path, r);
        }
        records->emplace_back(r.Snapshot());
    }
}

void AlgoFunc::NativeExtract(RTContext *ctx, const cypher::Record *record,
                             const cypher::VEC_EXPR &args,
                             const cypher::VEC_STR &yield_items,
                             struct std::vector<cypher::Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    CYPHER_ARG_CHECK(args.size() / 2 == 1, "wrong arguments number")
    CYPHER_ARG_CHECK(args[0].type == Entry::RecordEntryType::CONSTANT && args[1].IsMap(),
                     "wrong type")
    auto &config = *args[1].constant.map;
    auto it1 = config.find("isNode");
    auto it2 = config.find("field");
    if (it1 == config.end() || !it1->second.IsBool() || it2 == config.end() ||
        !it2->second.IsString()) {
        CYPHER_TODO();
    }
    cypher::FieldData value;
    if (it1->second.AsBool()) {
        CYPHER_THROW_ASSERT(record);
        auto &vid = args[0];
        auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
        if (vid.IsArray()) {
            value = cypher::FieldData::Array(0);
            for (auto &id : *vid.constant.array) {
                value.array->emplace_back(
                    ctx->txn_->GetTxn()->GetVertexField(id.AsInt64(), it2->second.AsString()));
            }
        } else {
            CYPHER_TODO();
        }
    } else {
        CYPHER_THROW_ASSERT(record);
        auto &eid = args[0];
        if (!eid.IsString()) CYPHER_TODO();
        auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
        value = ctx->txn_->GetTxn()->GetEdgeField(
            _detail::ExtractEdgeUid(eid.constant.scalar.AsString()), it2->second.AsString());
    }

    auto pp = global_ptable.GetProcedure("algo.native.extract");
    CYPHER_THROW_ASSERT(pp && pp->ContainsYieldItem("value"));
    cypher::VEC_STR titles;
    if (yield_items.empty()) {
        for (auto &res : pp->signature.result_list) titles.emplace_back(res.name);
    } else {
        for (auto &item : yield_items) {
            if (!pp->ContainsYieldItem(item)) {
                throw lgraph::CypherException("Unknown procedure output: " + item);
            }
            titles.emplace_back(item);
        }
    }
    std::unordered_map<std::string, std::function<
        void(const cypher::FieldData &, Record &)>> lmap = {
            {"value", [&](const cypher::FieldData &d, Record &r) { r.AddConstant(d); }},
        };
    Record r;
    for (auto &title : titles) {
        lmap.find(title)->second(value, r);
    }
    records->emplace_back(r.Snapshot());
}

void AlgoFunc::PageRank(RTContext *ctx, const cypher::Record *record,
                        const cypher::VEC_EXPR &args, const cypher::VEC_STR &yield_items,
                        struct std::vector<cypher::Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    CYPHER_ARG_CHECK(args.size() == 1, "args number should be 1");
    CYPHER_ARG_CHECK(args[0].IsInteger(), "arg is not int");

    std::map<int64_t, std::pair<double, int>> node_prs_curr;
    std::map<int64_t, std::pair<double, int>> node_prs_next;
    auto num_iterations = args[0].constant.scalar.integer();
    auto vertex_num = 0;
    double d = (double)0.85;
    for (auto v_it = ctx->txn_->GetVertexIterator(); v_it.IsValid(); v_it.Next()) {
        if (v_it.GetNumOutEdges() == 0) {
            node_prs_curr[v_it.GetId()] = std::make_pair(double(1), v_it.GetNumOutEdges());

        } else {
            node_prs_curr[v_it.GetId()] =
                std::make_pair(double(1 / v_it.GetNumOutEdges()), v_it.GetNumOutEdges());
        }
        vertex_num++;
    }
    if (vertex_num == 0) {
        return;
    }

    while (num_iterations > 0) {
        for (auto v_it = ctx->txn_->GetVertexIterator(); v_it.IsValid(); v_it.Next()) {
            auto vid = v_it.GetId();
            double sum = 0;
            for (auto edge_it = v_it.GetInEdgeIterator(); edge_it.IsValid(); edge_it.Next()) {
                if (node_prs_curr[edge_it.GetSrc()].second == 0) {
                    sum += node_prs_curr[edge_it.GetSrc()].first;
                } else {
                    sum += node_prs_curr[edge_it.GetSrc()].first /
                           node_prs_curr[edge_it.GetSrc()].second;
                }
            }
            node_prs_next[vid].first = (1 - d) / vertex_num + d * sum;
            node_prs_next[vid].second = node_prs_curr[vid].second;
        }
        node_prs_curr = node_prs_next;
        num_iterations--;
    }
    cypher::VEC_STR titles = ProcedureTitles("algo.pagerank", yield_items);
    std::unordered_map<std::string, bool> title_exsited;
    std::transform(titles.begin(), titles.end(), std::inserter(title_exsited, title_exsited.end()),
                   [](const std::string &title) { return std::make_pair(title, true); });
    for (auto &node_pr : node_prs_curr) {
        cypher::Record r;
        cypher::Node n;
        if (title_exsited.find("node") != title_exsited.end()) {
            n.SetVid(node_pr.first);
            r.AddNode(&n);
        }
        if (title_exsited.find("pr") != title_exsited.end()) {
            r.AddConstant(lgraph::FieldData(node_pr.second.first));
        }
        records->emplace_back(r.Snapshot());
    }
}

void AlgoFunc::Jaccard(RTContext *ctx, const cypher::Record *record,
                       const cypher::VEC_EXPR &args, const cypher::VEC_STR &yield_items,
                       struct std::vector<cypher::Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    CYPHER_ARG_CHECK(args.size() == 2, "args number should be 2");
    auto lef_entry = args[0];
    auto rig_entry = args[1];
    CYPHER_ARG_CHECK(lef_entry.IsArray(), "args[0] is not list");
    CYPHER_ARG_CHECK(rig_entry.IsArray(), "args[1] is not list");
    std::unordered_set<int64_t> union_set, intersection;
    for (auto &item : *lef_entry.constant.array) {
        CYPHER_ARG_CHECK(item.IsInteger(), "item is not integer");
        union_set.emplace(item.AsInt64());
    }
    for (auto &item : *rig_entry.constant.array) {
        CYPHER_ARG_CHECK(item.IsInteger(), "item is not integer");
        if (union_set.find(item.AsInt64()) != union_set.end()) {
            intersection.emplace(item.AsInt64());
        }
    }
    for (auto &item : *rig_entry.constant.array) {
        CYPHER_ARG_CHECK(item.IsInteger(), "item is not integer");
        union_set.emplace(item.AsInt64());
    }
    std::unordered_set<std::string> title_exsited;
    cypher::VEC_STR titles = ProcedureTitles("algo.jaccard", yield_items);
    std::transform(titles.begin(), titles.end(), std::inserter(title_exsited, title_exsited.end()),
                   [](const std::string &title) { return title; });
    if (title_exsited.find("similarity") != title_exsited.end()) {
        auto f = union_set.empty()
                     ? lgraph::FieldData()
                     : lgraph::FieldData(float(intersection.size()) / union_set.size());
        cypher::Record r;
        r.AddConstant(f);
        records->emplace_back(r.Snapshot());
    }
}

void SpatialFunc::Distance(RTContext *ctx, const Record *record,
                           const cypher::VEC_EXPR &args,
                           const cypher::VEC_STR &yield_items,
                           struct std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2, "wrong arguments number");
    CYPHER_ARG_CHECK(args[0].IsSpatial(),
                     FMA_FMT("{} has to be spatial data",
                             args[0].ToString()));
    CYPHER_ARG_CHECK(args[1].IsSpatial(),
                     FMA_FMT("{} has to be spatial data",
                             args[1].ToString()));

    ::lgraph_api::SRID srid1 = args[0].constant.scalar.GetSRID();
    ::lgraph_api::SRID srid2 = args[1].constant.scalar.GetSRID();
    CYPHER_THROW_ASSERT(srid1 == srid2);
    double d = 0;
    switch (srid1) {
    case ::lgraph_api::SRID::WGS84:
        {
            auto Spatial1 = args[0].constant.scalar.AsWgsSpatial();
            auto Spatial2 = args[1].constant.scalar.AsWgsSpatial();
            d = Spatial1.Distance(Spatial2);
            break;
        }

    case ::lgraph_api::SRID::CARTESIAN:
        {
            auto Spatial1 = args[0].constant.scalar.AsCartesianSpatial();
            auto Spatial2 = args[1].constant.scalar.AsCartesianSpatial();
            d = Spatial1.Distance(Spatial2);
            break;
        }

    default:
        throw std::runtime_error("unsupported srid type!");
    }

    Record r;
    r.AddConstant(::lgraph::FieldData(d));
    records->emplace_back(r.Snapshot());
}

}  // namespace cypher
