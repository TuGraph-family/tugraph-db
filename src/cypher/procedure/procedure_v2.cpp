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

#include "import/import_online.h"
#include "core/field_data_helper.h"
#include "core/thread_id.h"
#include "db/galaxy.h"
#include "parser/data_typedef.h"
#include "resultset/record.h"
#include "server/state_machine.h"
#include "restful/server/json_convert.h"
#include "cypher/graph/common.h"
#include "cypher/procedure/procedure_v2.h"
#include "cypher/procedure/utils.h"
#include "butil/endpoint.h"
#include "cypher/monitor/memory_monitor_allocator.h"
#include "fma-common/encrypt.h"
#include "import/import_v3.h"

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
    EDGE_FILTER_T;

const std::unordered_map<std::string, lgraph::FieldType> BuiltinProcedureV2::type_map_ =
    lgraph::field_data_helper::_detail::_FieldName2TypeDict_();

cypher::VEC_STR_V2 ProcedureTitlesV2(const std::string &procedure_name,
                                     const cypher::VEC_STR_V2 &yield_items) {
    cypher::VEC_STR_V2 titles;
    auto pp = global_ptable_v2.GetProcedureV2(procedure_name);

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

void BuiltinProcedureV2::DbSubgraph(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                    const VEC_STR_V2 &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 1, "This function takes 1 argrument. e.g. db.DbSubGraph(vids)");
    CYPHER_ARG_CHECK(args[0].IsArray(), "db.DbSubGraph(vids): `vids` must be list");
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    std::set<lgraph::VertexId> set_vids;
    for (auto &vid : *args[0].constant.array) {
        CYPHER_ARG_CHECK(vid.IsInteger(), "db.DbSubGraph(vids): `vid` must be int");
        set_vids.emplace(vid.integer());
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

void BuiltinProcedureV2::DbVertexLabels(RTContext *ctx, const Record *record,
                                        const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbEdgeLabels(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                      const VEC_STR_V2 &yield_items, std::vector<Record> *records) {
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

void BuiltinProcedureV2::DbIndexes(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                   const VEC_STR_V2 &yield_items, std::vector<Record> *records) {
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

void BuiltinProcedureV2::DbListLabelIndexes(RTContext *ctx, const Record *record,
                                            const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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
    if (is_vertex) {
        indexes = ctx->txn_->GetTxn()->ListVertexIndexByLabel(label);
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
}
void BuiltinProcedureV2::DbPropertyKeys(RTContext *ctx, const Record *record,
                                        const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                        std::vector<Record> *records) {
    CYPHER_TODO();
}

void BuiltinProcedureV2::DbWarmUp(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                  const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsProcedures(RTContext *ctx, const Record *record,
                                        const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                        std::vector<cypher::Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.procedures()",
                                           args.size()))
    auto pp = global_ptable.GetProcedure("dbms.procedures");
    CYPHER_THROW_ASSERT(pp && pp->ContainsYieldItem("name") && pp->ContainsYieldItem("signature"));
    cypher::VEC_STR_V2 titles;
    if (yield_items.empty()) {
        titles.push_back("name");
        titles.push_back("signature");
        titles.push_back("read_only");
    } else {
        for (auto &item : yield_items) titles.emplace_back(item);
    }
    std::unordered_map<std::string, std::function<void(const ProcedureV2 &, Record &)>> lmap = {
        {"name",
         [](const ProcedureV2 &p, Record &r) { r.AddConstant(lgraph::FieldData(p.proc_name)); }},
        {"signature",
         [](const ProcedureV2 &p, Record &r) { r.AddConstant(lgraph::FieldData(p.Signature())); }},
        {"read_only",
         [](const ProcedureV2 &p, Record &r) { r.AddConstant(lgraph::FieldData(p.read_only)); }}};
    for (auto &p : global_procedures_v2) {
        Record r;
        for (auto &title : titles) {
            lmap.find(title)->second(p, r);
        }
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedureV2::_ExtractFds(const VEC_EXPR_V2 &args, std::string &label,
                                     std::string &extra, std::vector<lgraph::FieldSpec> &fds) {
    using namespace parser;
    CYPHER_ARG_CHECK(args.size() % SPEC_MEMBER_SIZE_V2 == 2,
                     "Not enough arguments. This function takes 2 or more arguments.")
    CYPHER_ARG_CHECK(args[0].IsString(), FMA_FMT("{} has to be a string ", args[0].ToString()))
    CYPHER_ARG_CHECK(args[1].IsString(), FMA_FMT("{} has to be a string ", args[0].ToString()))

    label = args[0].constant.scalar.AsString();
    extra = args[1].constant.scalar.AsString();
    for (int i = 0; i < (int)args.size() / 3; i++) {
        CYPHER_ARG_CHECK(
            args[i * SPEC_MEMBER_SIZE_V2 + 2].IsString(),
            FMA_FMT("{} has to be a string ", args[i * SPEC_MEMBER_SIZE_V2 + 2].ToString()))
        CYPHER_ARG_CHECK(
            args[i * SPEC_MEMBER_SIZE_V2 + 3].IsScalar(),
            FMA_FMT("{} has to be a variable，alternative value is "
                    "(NUL,BOOL,INT8,INT16,INT32,INT64,FLOAT,DOUBLE,DATE,DATETIME,STRING,BLOB) ",
                    args[i * SPEC_MEMBER_SIZE_V2 + 3].ToString()))
        CYPHER_ARG_CHECK(
            args[i * SPEC_MEMBER_SIZE_V2 + 4].IsBool(),
            FMA_FMT("{} has to be a boolean ", args[i * SPEC_MEMBER_SIZE_V2 + 4].ToString()))
        auto name = args[i * SPEC_MEMBER_SIZE_V2 + 2].constant.scalar.AsString();
        auto type_name = args[i * SPEC_MEMBER_SIZE_V2 + 3].constant.scalar.AsString();
        auto nullable = args[i * SPEC_MEMBER_SIZE_V2 + 4].constant.scalar.AsBool();

        std::transform(type_name.begin(), type_name.end(), type_name.begin(), ::tolower);
        auto it = type_map_.find(type_name);
        if (it == type_map_.end()) {
            throw lgraph::ReminderException("Unknown type name: " + type_name);
        }
        fds.emplace_back(name, it->second, nullable);
    }
}

void BuiltinProcedureV2::_ExtractAccessLevel(
    const VEC_EXPR_V2 &args, std::string &role,
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

static ::std::vector<::lgraph::FieldSpec> ParseFieldSpecs(const VEC_EXPR_V2 &args,
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

void BuiltinProcedureV2::DbCreateVertexLabel(RTContext *ctx, const Record *record,
                                             const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                             std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() % SPEC_MEMBER_SIZE_V2 == 2,
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

// params: vertex, label, primary_field,  [fieldspec1], [fieldspec2]...
// params: edge, label, edge_constraints, [fieldspec1], [fieldspec2]...
void BuiltinProcedureV2::DbCreateLabel(RTContext *ctx, const Record *record,
                                       const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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
void BuiltinProcedureV2::DbGetLabelSchema(RTContext *ctx, const Record *record,
                                          const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbGetVertexSchema(RTContext *ctx, const Record *record,
                                           const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbGetEdgeSchema(RTContext *ctx, const Record *record,
                                         const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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
void BuiltinProcedureV2::DbDeleteLabel(RTContext *ctx, const Record *record,
                                       const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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
void BuiltinProcedureV2::DbAlterLabelDelFields(RTContext *ctx, const Record *record,
                                               const VEC_EXPR_V2 &args,
                                               const VEC_STR_V2 &yield_items,
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
void BuiltinProcedureV2::DbAlterLabelAddFields(RTContext *ctx, const Record *record,
                                               const VEC_EXPR_V2 &args,
                                               const VEC_STR_V2 &yield_items,
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
        lgraph::FieldData default_value = list[2];
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
void BuiltinProcedureV2::DbAlterLabelModFields(RTContext *ctx, const Record *record,
                                               const VEC_EXPR_V2 &args,
                                               const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbCreateEdgeLabel(RTContext *ctx, const Record *record,
                                           const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                           std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() % SPEC_MEMBER_SIZE_V2 == 2,
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

void BuiltinProcedureV2::DbAddVertexIndex(RTContext *ctx, const Record *record,
                                          const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbAddEdgeIndex(RTContext *ctx, const Record *record,
                                        const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbAddFullTextIndex(RTContext *ctx, const Record *record,
                                            const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbDeleteFullTextIndex(RTContext *ctx, const Record *record,
                                               const VEC_EXPR_V2 &args,
                                               const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbRebuildFullTextIndex(RTContext *ctx, const Record *record,
                                                const VEC_EXPR_V2 &args,
                                                const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbFullTextIndexes(RTContext *ctx, const Record *record,
                                           const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbClearEdgeConstraints(RTContext *ctx, const Record *record,
                                                const VEC_EXPR_V2 &args,
                                                const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbAddEdgeConstraints(RTContext *ctx, const Record *record,
                                              const VEC_EXPR_V2 &args,
                                              const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsMetaCountDetail(RTContext *ctx, const Record *record,
                                             const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsMetaCount(RTContext *ctx, const Record *record,
                                       const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsMetaRefreshCount(RTContext *ctx, const Record *record,
                                              const VEC_EXPR_V2 &args,
                                              const VEC_STR_V2 &yield_items,
                                              std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.meta.refreshCount()",
                                           args.size()))
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    ac_db.RefreshCount();
}

void BuiltinProcedureV2::DbmsSecurityChangePassword(RTContext *ctx, const cypher::Record *record,
                                                    const cypher::VEC_EXPR_V2 &args,
                                                    const cypher::VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityChangeUserPassword(RTContext *ctx,
                                                        const cypher::Record *record,
                                                        const cypher::VEC_EXPR_V2 &args,
                                                        const cypher::VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecuritySetUserMemoryLimit(RTContext *ctx,
                                                        const cypher::Record *record,
                                                        const cypher::VEC_EXPR_V2 &args,
                                                        const cypher::VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityCreateUser(RTContext *ctx, const cypher::Record *record,
                                                const cypher::VEC_EXPR_V2 &args,
                                                const cypher::VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityDeleteUser(RTContext *ctx, const cypher::Record *record,
                                                const cypher::VEC_EXPR_V2 &args,
                                                const cypher::VEC_STR_V2 &yield_items,
                                                std::vector<cypher::Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 1,
                     "need 1 parameters, e.g. dbms.security.deleteUser(user_name)")
    CYPHER_ARG_CHECK(args[0].IsString(), "user_name type should be string")
    bool success = ctx->galaxy_->DeleteUser(ctx->user_, args[0].constant.scalar.AsString());
    if (!success) {
        throw lgraph::UserNotExistException(args[0].constant.scalar.AsString());
    }
}

void BuiltinProcedureV2::DbmsSecurityListUsers(RTContext *ctx, const cypher::Record *record,
                                               const cypher::VEC_EXPR_V2 &args,
                                               const cypher::VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityShowCurrentUser(RTContext *ctx, const cypher::Record *record,
                                                     const cypher::VEC_EXPR_V2 &args,
                                                     const cypher::VEC_STR_V2 &yield_items,
                                                     std::vector<cypher::Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.security.showCurrentUser()",
                                           args.size()))
    Record r;
    r.AddConstant(lgraph::FieldData(ctx->user_));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedureV2::DbmsSecurityHostWhitelistList(RTContext *ctx, const Record *record,
                                                       const VEC_EXPR_V2 &args,
                                                       const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityHostWhitelistAdd(RTContext *ctx, const Record *record,
                                                      const VEC_EXPR_V2 &args,
                                                      const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityHostWhitelistDelete(RTContext *ctx, const Record *record,
                                                         const VEC_EXPR_V2 &args,
                                                         const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsGraphCreateGraph(RTContext *ctx, const cypher::Record *record,
                                              const cypher::VEC_EXPR_V2 &args,
                                              const cypher::VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsGraphModGraph(RTContext *ctx, const cypher::Record *record,
                                           const cypher::VEC_EXPR_V2 &args,
                                           const cypher::VEC_STR_V2 &yield_items,
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
            act.max_size = kv.second.integer() << 30;
        } else if (kv.first == "description") {
            act.mod_desc = true;
            if (kv.second.IsString())
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

void BuiltinProcedureV2::DbmsGraphDeleteGraph(RTContext *ctx, const cypher::Record *record,
                                              const cypher::VEC_EXPR_V2 &args,
                                              const cypher::VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsGraphListGraphs(RTContext *ctx, const cypher::Record *record,
                                             const cypher::VEC_EXPR_V2 &args,
                                             const cypher::VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsGraphListUserGraphs(RTContext *ctx, const cypher::Record *record,
                                                 const cypher::VEC_EXPR_V2 &args,
                                                 const cypher::VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsGraphGetGraphInfo(RTContext *ctx, const Record *record,
                                               const VEC_EXPR_V2 &args,
                                               const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSystemInfo(RTContext *ctx, const cypher::Record *record,
                                        const cypher::VEC_EXPR_V2 &args,
                                        const cypher::VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsConfigList(RTContext *ctx, const cypher::Record *record,
                                        const cypher::VEC_EXPR_V2 &args,
                                        const cypher::VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsConfigUpdate(RTContext *ctx, const cypher::Record *record,
                                          const cypher::VEC_EXPR_V2 &args,
                                          const cypher::VEC_STR_V2 &yield_items,
                                          std::vector<cypher::Record> *records) {
    // ctx->txn_.reset();
    // ctx->ac_db_.reset();
    // CYPHER_ARG_CHECK(args.size() == 1,
    //                  "need exactly one parameter. "
    //                  "e.g. dbms.config.update({durable: false, enable_audit_log: false})")
    // CYPHER_ARG_CHECK(args[0].type == parser::Expression::MAP, "role type should be map")
    // std::map<std::string, lgraph::FieldData> kvs;
    // for (auto &kv : args[0].Map()) {
    //     kvs[kv.first] = parser::MakeFieldData(kv.second);
    // }
    // bool need_reload = ctx->galaxy_->UpdateConfig(ctx->user_, kvs);
    // // restart galaxy in a separate thread
    // if (need_reload) ctx->galaxy_->ReloadFromDisk();
}

void BuiltinProcedureV2::DbmsListBackupLogFiles(RTContext *ctx, const cypher::Record *record,
                                                const cypher::VEC_EXPR_V2 &args,
                                                const cypher::VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsTakeSnapshot(RTContext *ctx, const cypher::Record *record,
                                          const cypher::VEC_EXPR_V2 &args,
                                          const cypher::VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityListRoles(RTContext *ctx, const cypher::Record *record,
                                               const cypher::VEC_EXPR_V2 &args,
                                               const cypher::VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityCreateRole(RTContext *ctx, const Record *record,
                                                const VEC_EXPR_V2 &args,
                                                const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityDeleteRole(RTContext *ctx, const Record *record,
                                                const VEC_EXPR_V2 &args,
                                                const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityGetUserInfo(RTContext *ctx, const Record *record,
                                                 const VEC_EXPR_V2 &args,
                                                 const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityGetUserPermissions(RTContext *ctx, const Record *record,
                                                        const VEC_EXPR_V2 &args,
                                                        const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityGetUserMemoryUsage(RTContext *ctx, const Record *record,
                                                        const VEC_EXPR_V2 &args,
                                                        const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityGetRoleInfo(RTContext *ctx, const Record *record,
                                                 const VEC_EXPR_V2 &args,
                                                 const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityDisableRole(RTContext *ctx, const Record *record,
                                                 const VEC_EXPR_V2 &args,
                                                 const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityModRoleDesc(RTContext *ctx, const Record *record,
                                                 const VEC_EXPR_V2 &args,
                                                 const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityRebuildRoleAccessLevel(RTContext *ctx, const Record *record,
                                                            const VEC_EXPR_V2 &args,
                                                            const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityModRoleAccessLevel(RTContext *ctx, const Record *record,
                                                        const VEC_EXPR_V2 &args,
                                                        const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityModRoleFieldAccessLevel(RTContext *ctx, const Record *record,
                                                             const VEC_EXPR_V2 &args,
                                                             const VEC_STR_V2 &yield_items,
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
    cypher::VEC_STR_V2 titles =
        ProcedureTitlesV2("dbms.security.modRoleFieldAccessLevel", yield_items);
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

void BuiltinProcedureV2::DbmsSecurityDisableUser(RTContext *ctx, const Record *record,
                                                 const VEC_EXPR_V2 &args,
                                                 const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecuritySetCurrentDesc(RTContext *ctx, const Record *record,
                                                    const VEC_EXPR_V2 &args,
                                                    const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecuritySetUserDesc(RTContext *ctx, const Record *record,
                                                 const VEC_EXPR_V2 &args,
                                                 const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityDeleteUserRoles(RTContext *ctx, const Record *record,
                                                     const VEC_EXPR_V2 &args,
                                                     const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityRebuildUserRoles(RTContext *ctx, const Record *record,
                                                      const VEC_EXPR_V2 &args,
                                                      const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbmsSecurityAddUserRoles(RTContext *ctx, const Record *record,
                                                  const VEC_EXPR_V2 &args,
                                                  const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbPluginLoadPlugin(RTContext *ctx, const Record *record,
                                            const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbPluginDeletePlugin(RTContext *ctx, const Record *record,
                                              const VEC_EXPR_V2 &args,
                                              const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbPluginGetPluginInfo(RTContext *ctx, const Record *record,
                                               const VEC_EXPR_V2 &args,
                                               const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbPluginListPlugin(RTContext *ctx, const Record *record,
                                            const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbPluginListUserPlugins(RTContext *ctx, const Record *record,
                                                 const VEC_EXPR_V2 &args,
                                                 const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbPluginCallPlugin(RTContext *ctx, const Record *record,
                                            const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbImportorDataImportor(RTContext *ctx, const Record *record,
                                                const VEC_EXPR_V2 &args,
                                                const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbImportorFullImportor(RTContext *ctx, const Record *record,
                                                const VEC_EXPR_V2 &args,
                                                const VEC_STR_V2 &yield_items,
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
        full_import_conf[kv.first] = kv.second;
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

void BuiltinProcedureV2::DbImportorFullFileImportor(RTContext *ctx, const Record *record,
                                                    const VEC_EXPR_V2 &args,
                                                    const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbImportorSchemaImportor(RTContext *ctx, const Record *record,
                                                  const VEC_EXPR_V2 &args,
                                                  const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbDeleteIndex(RTContext *ctx, const Record *record,
                                       const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbDeleteEdgeIndex(RTContext *ctx, const Record *record,
                                           const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbFlushDB(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                   const VEC_STR_V2 &yield_items, std::vector<Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.flushDB()",
                                           args.size()))
    ctx->ac_db_->Flush();
}

void BuiltinProcedureV2::DbDropDB(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                  const VEC_STR_V2 &yield_items, std::vector<Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    if (!ctx->galaxy_->IsAdmin(ctx->user_))
        THROW_CODE(Unauthorized, "Admin access right required.");
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.dropDB()",
                                           args.size()))
    ctx->ac_db_->DropAllData();
}

void BuiltinProcedureV2::DbTaskListTasks(RTContext *ctx, const Record *record,
                                         const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbTaskTerminateTask(RTContext *ctx, const Record *record,
                                             const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbMonitorServerInfo(RTContext *ctx, const Record *record,
                                             const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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

void BuiltinProcedureV2::DbMonitorTuGraphInfo(RTContext *ctx, const Record *record,
                                              const VEC_EXPR_V2 &args,
                                              const VEC_STR_V2 &yield_items,
                                              std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.monitor.tuGraphInfo()",
                                           args.size()))
    if (!ctx) CYPHER_INTL_ERR();
    Record r;
    r.AddConstant(lgraph::FieldData(ValueToJson(ctx->sm_->GetStats()).serialize()));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedureV2::DbmsHaClusterInfo(RTContext *ctx, const Record *record,
                                           const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
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
static bool _Filter(lgraph::Transaction &txn, const EIT &eit, const EDGE_FILTER_T &edge_filter) {
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

static std::vector<lgraph::VertexId> _GetNeighbors(lgraph::Transaction &txn, lgraph::VertexId vid,
                                                   const std::string &edge_label,
                                                   const EDGE_FILTER_T &edge_filter,
                                                   const parser::LinkDirection &direction,
                                                   std::optional<size_t> per_node_limit) {
    auto vit = txn.GetVertexIterator(vid);
    CYPHER_THROW_ASSERT(vit.IsValid());
    std::vector<lgraph::VertexId> neighbors;
    if (edge_label.empty()) {
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
        auto edge_lid = txn.GetLabelId(false, edge_label);
        if (direction == parser::LinkDirection::LEFT_TO_RIGHT ||
            direction == parser::LinkDirection::DIR_NOT_SPECIFIED) {
            size_t count = 0;
            for (auto eit = vit.GetOutEdgeIterator(lgraph::EdgeUid(vid, 0, edge_lid, 0, 0), true);
                 eit.IsValid(); eit.Next()) {
                if (eit.GetLabelId() != edge_lid) break;
                count += 1;
                if (per_node_limit.has_value() && count > per_node_limit.value()) {
                    break;
                }
                if (!_Filter(txn, eit, edge_filter)) continue;
                neighbors.push_back(eit.GetDst());
            }
        }
        if (direction == parser::LinkDirection::RIGHT_TO_LEFT ||
            direction == parser::LinkDirection::DIR_NOT_SPECIFIED) {
            size_t count = 0;
            for (auto eit = vit.GetInEdgeIterator(lgraph::EdgeUid(0, vid, edge_lid, 0, 0), true);
                 eit.IsValid(); eit.Next()) {
                if (eit.GetLabelId() != edge_lid) break;
                count += 1;
                if (per_node_limit.has_value() && count > per_node_limit.value()) {
                    break;
                }
                if (!_Filter(txn, eit, edge_filter)) continue;
                neighbors.push_back(eit.GetSrc());
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
                                       lgraph::VertexId end_vid, const std::string &edge_label,
                                       size_t max_hops, cypher::Path &path,
                                       const EDGE_FILTER_T &edge_filter, parser::LinkDirection &dir,
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
                    _GetNeighbors(txn, vid, edge_label, edge_filter, forward_dir, per_node_limit);
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
                    _GetNeighbors(txn, vid, edge_label, edge_filter, backward_dir, per_node_limit);
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

void _EnumeratePartialPaths_v2(lgraph::Transaction &txn,
                               const std::unordered_map<int64_t, int> &hop_info, const int64_t vid,
                               const int depth, const std::string &edge_label, cypher::Path &path,
                               std::vector<cypher::Path> &paths) {
    if (depth != 0) {
        if (edge_label.empty()) {
            for (auto eit = txn.GetOutEdgeIterator(vid); eit.IsValid(); eit.Next()) {
                int64_t nbr = eit.GetDst();
                auto it = hop_info.find(nbr);
                if (it != hop_info.end() && it->second == depth - 1) {
                    path.Append(eit.GetUid());
                    _EnumeratePartialPaths_v2(txn, hop_info, nbr, depth - 1, edge_label, path,
                                              paths);
                    path.PopBack();
                }
            }
            for (auto eit = txn.GetInEdgeIterator(vid); eit.IsValid(); eit.Next()) {
                int64_t nbr = eit.GetSrc();
                auto it = hop_info.find(nbr);
                if (it != hop_info.end() && it->second == depth - 1) {
                    path.Append(eit.GetUid());
                    _EnumeratePartialPaths_v2(txn, hop_info, nbr, depth - 1, edge_label, path,
                                              paths);
                    path.PopBack();
                }
            }
        } else {
            auto edge_lid = txn.GetLabelId(false, edge_label);
            for (auto eit = txn.GetOutEdgeIterator(lgraph::EdgeUid(vid, 0, edge_lid, 0, 0), true);
                 eit.IsValid(); eit.Next()) {
                if (eit.GetLabelId() != edge_lid) break;
                int64_t nbr = eit.GetDst();
                auto it = hop_info.find(nbr);
                if (it != hop_info.end() && it->second == depth - 1) {
                    path.Append(eit.GetUid());
                    _EnumeratePartialPaths_v2(txn, hop_info, nbr, depth - 1, edge_label, path,
                                              paths);
                    path.PopBack();
                }
            }
            for (auto eit = txn.GetInEdgeIterator(lgraph::EdgeUid(0, vid, edge_lid, 0, 0), true);
                 eit.IsValid(); eit.Next()) {
                if (eit.GetLabelId() != edge_lid) break;
                int64_t nbr = eit.GetSrc();
                auto it = hop_info.find(nbr);
                if (it != hop_info.end() && it->second == depth - 1) {
                    path.Append(eit.GetUid());
                    _EnumeratePartialPaths_v2(txn, hop_info, nbr, depth - 1, edge_label, path,
                                              paths);
                    path.PopBack();
                }
            }
        }
    } else {
        paths.emplace_back(path);
    }
}

static void _P2PUnweightedAllShortestPaths(lgraph::Transaction &txn, lgraph::VertexId start_vid,
                                           lgraph::VertexId end_vid, const std::string &edge_label,
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
                if (edge_label.empty()) {
                    for (auto eit = txn.GetOutEdgeIterator(vid); eit.IsValid(); eit.Next()) {
                        _EmplaceForwardNeighbor(eit, fhop, child, parent, hits, next_q);
                    }
                    for (auto eit = txn.GetInEdgeIterator(vid); eit.IsValid(); eit.Next()) {
                        _EmplaceForwardNeighbor(eit, fhop, child, parent, hits, next_q);
                    }
                } else {
                    auto edge_lid = txn.GetLabelId(false, edge_label);
                    for (auto eit =
                             txn.GetOutEdgeIterator(lgraph::EdgeUid(vid, 0, edge_lid, 0, 0), true);
                         eit.IsValid(); eit.Next()) {
                        if (eit.GetLabelId() != edge_lid) break;
                        _EmplaceForwardNeighbor(eit, fhop, child, parent, hits, next_q);
                    }
                    for (auto eit =
                             txn.GetInEdgeIterator(lgraph::EdgeUid(0, vid, edge_lid, 0, 0), true);
                         eit.IsValid(); eit.Next()) {
                        if (eit.GetLabelId() != edge_lid) break;
                        _EmplaceForwardNeighbor(eit, fhop, child, parent, hits, next_q);
                    }
                }
            }
            std::sort(next_q.begin(), next_q.end());
            forward_q.swap(next_q);
        } else {
            bhop++;
            for (auto vid : backward_q) {
                if (edge_label.empty()) {
                    for (auto eit = txn.GetOutEdgeIterator(vid); eit.IsValid(); eit.Next()) {
                        _EmplaceBackwardNeighbor(eit, bhop, parent, child, hits, next_q);
                    }
                    for (auto eit = txn.GetInEdgeIterator(vid); eit.IsValid(); eit.Next()) {
                        _EmplaceBackwardNeighbor(eit, bhop, parent, child, hits, next_q);
                    }
                } else {
                    auto edge_lid = txn.GetLabelId(false, edge_label);
                    for (auto eit =
                             txn.GetOutEdgeIterator(lgraph::EdgeUid(vid, 0, edge_lid, 0, 0), true);
                         eit.IsValid(); eit.Next()) {
                        if (eit.GetLabelId() != edge_lid) break;
                        _EmplaceBackwardNeighbor(eit, bhop, parent, child, hits, next_q);
                    }
                    for (auto eit =
                             txn.GetInEdgeIterator(lgraph::EdgeUid(0, vid, edge_lid, 0, 0), true);
                         eit.IsValid(); eit.Next()) {
                        if (eit.GetLabelId() != edge_lid) break;
                        _EmplaceBackwardNeighbor(eit, bhop, parent, child, hits, next_q);
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
        _EnumeratePartialPaths_v2(txn, parent, fvid, parent[fvid], edge_label, path, fpaths);
        path.Clear();
        path.SetStart(bvid);
        _EnumeratePartialPaths_v2(txn, child, bvid, child[bvid], edge_label, path, bpaths);
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

void AlgoFuncV2::ShortestPath(RTContext *ctx, const Record *record, const cypher::VEC_EXPR_V2 &args,
                              const cypher::VEC_STR_V2 &yield_items,
                              std::vector<cypher::Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    CYPHER_ARG_CHECK(args.size() / 2 == 1, "wrong arguments number")
    CYPHER_ARG_CHECK(
        args[0].IsString() && args[1].IsString() && (args.size() == 2 || args[2].IsMap()),
        "wrong type")
    std::string edge_label;
    EDGE_FILTER_T edge_filter;
    parser::LinkDirection direction = parser::LinkDirection::DIR_NOT_SPECIFIED;
    size_t max_hops = 20;
    if (args.size() == 3) {
        auto &map = *args[2].constant.map;
        auto it = map.find("relationshipQuery");
        if (it != map.end()) {
            if (it->second.IsString()) CYPHER_TODO();
            edge_label = it->second.AsString();
        }
        auto max = map.find("maxHops");
        if (max != map.end()) {
            if (max->second.IsInteger()) CYPHER_TODO();
            max_hops = max->second.integer();
        }
        auto map_edge_filter = map.find("edgeFilter");
        if (map_edge_filter != map.end()) {
            CYPHER_TODO();
            // if (map_edge_filter->second.type != parser::Expression::MAP) CYPHER_TODO();
            // for (auto &kv : map_edge_filter->second.Map()) {
            //     if (kv.second.type != parser::Expression::MAP) CYPHER_TODO();
            //     std::unordered_map<std::string, lgraph::FieldData> filter_t;
            //     for (auto filter_pair : kv.second.Map()) {
            //         lgraph::FieldData field;
            //         if (!filter_pair.second.IsLiteral()) CYPHER_TODO();
            //         field = parser::MakeFieldData(filter_pair.second);
            //         filter_t.emplace(filter_pair.first, field);
            //     }
            //     edge_filter.emplace(kv.first, filter_t);
            // }
        }
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
    auto it1 = record->symbol_table->symbols.find(args[0].constant.scalar.AsString());
    auto it2 = record->symbol_table->symbols.find(args[1].constant.scalar.AsString());
    if (it1 == record->symbol_table->symbols.end() || it2 == record->symbol_table->symbols.end()) {
        CYPHER_TODO();
    }
    auto start_vid = record->values[it1->second.id].node->PullVid();
    auto end_vid = record->values[it2->second.id].node->PullVid();
    cypher::Path path;
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    _P2PUnweightedShortestPath(*ctx->txn_->GetTxn(), start_vid, end_vid, edge_label, max_hops, path,
                               edge_filter, direction, std::nullopt);

    auto pp = global_ptable.GetProcedure("algo.shortestPath");
    CYPHER_THROW_ASSERT(pp && pp->ContainsYieldItem("nodeCount") &&
                        pp->ContainsYieldItem("totalCost") && pp->ContainsYieldItem("path"));
    cypher::VEC_STR_V2 titles = ProcedureTitlesV2("algo.shortestPath", yield_items);
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

void AlgoFuncV2::AllShortestPaths(RTContext *ctx, const Record *record,
                                  const cypher::VEC_EXPR_V2 &args,
                                  const cypher::VEC_STR_V2 &yield_items,
                                  std::vector<cypher::Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    CYPHER_ARG_CHECK(args.size() / 2 == 1, "wrong arguments number")
    CYPHER_ARG_CHECK(
        args[0].IsString() && args[1].IsString() && (args.size() == 2 || args[2].IsMap()),
        "wrong type")
    std::string edge_label;
    if (args.size() == 3) {
        auto &map = *args[2].constant.map;
        auto it = map.find("relationshipQuery");
        if (it != map.end()) {
            if (!it->second.IsString()) CYPHER_TODO();
            edge_label = it->second.AsString();
        }
    }
    CYPHER_THROW_ASSERT(record);
    auto it1 = record->symbol_table->symbols.find(args[0].constant.scalar.AsString());
    auto it2 = record->symbol_table->symbols.find(args[1].constant.scalar.AsString());
    if (it1 == record->symbol_table->symbols.end() || it2 == record->symbol_table->symbols.end()) {
        CYPHER_TODO();
    }
    auto start_vid = record->values[it1->second.id].node->PullVid();
    auto end_vid = record->values[it2->second.id].node->PullVid();
    std::vector<cypher::Path> paths;
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    _P2PUnweightedAllShortestPaths(*ctx->txn_->GetTxn(), start_vid, end_vid, edge_label, paths);

    auto pp = global_ptable.GetProcedure("algo.allShortestPaths");
    CYPHER_THROW_ASSERT(pp && pp->ContainsYieldItem("nodeIds") &&
                        pp->ContainsYieldItem("relationshipIds") && pp->ContainsYieldItem("cost"));
    cypher::VEC_STR_V2 titles = ProcedureTitlesV2("algo.allShortestPaths", yield_items);
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

void AlgoFuncV2::NativeExtract(RTContext *ctx, const cypher::Record *record,
                               const cypher::VEC_EXPR_V2 &args,
                               const cypher::VEC_STR_V2 &yield_items,
                               struct std::vector<cypher::Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    CYPHER_ARG_CHECK(args.size() / 2 == 1, "wrong arguments number")
    CYPHER_ARG_CHECK(args[0].IsString() && args[1].IsMap(), "wrong type")
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
        auto i = record->symbol_table->symbols.find(args[0].constant.scalar.AsString());
        if (i == record->symbol_table->symbols.end()) CYPHER_TODO();
        auto &vid = record->values[i->second.id];
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
        auto i = record->symbol_table->symbols.find(args[0].constant.scalar.AsString());
        if (i == record->symbol_table->symbols.end()) CYPHER_TODO();
        auto &eid = record->values[i->second.id];
        if (!eid.IsString()) CYPHER_TODO();
        auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
        value = ctx->txn_->GetTxn()->GetEdgeField(
            _detail::ExtractEdgeUid(eid.constant.scalar.AsString()), it2->second.AsString());
    }

    auto pp = global_ptable.GetProcedure("algo.native.extract");
    CYPHER_THROW_ASSERT(pp && pp->ContainsYieldItem("value"));
    cypher::VEC_STR_V2 titles;
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
    std::unordered_map<std::string, std::function<void(const cypher::FieldData &, Record &)>> lmap =
        { {"value", [&](const cypher::FieldData &d, Record &r) { r.AddConstant(d); }},
        };
    Record r;
    for (auto &title : titles) {
        lmap.find(title)->second(value, r);
    }
    records->emplace_back(r.Snapshot());
}

void AlgoFuncV2::PageRank(RTContext *ctx, const cypher::Record *record,
                          const cypher::VEC_EXPR_V2 &args, const cypher::VEC_STR_V2 &yield_items,
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
    cypher::VEC_STR_V2 titles = ProcedureTitlesV2("algo.pagerank", yield_items);
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

void AlgoFuncV2::Jaccard(RTContext *ctx, const cypher::Record *record,
                         const cypher::VEC_EXPR_V2 &args, const cypher::VEC_STR_V2 &yield_items,
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
    cypher::VEC_STR_V2 titles = ProcedureTitlesV2("algo.jaccard", yield_items);
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
}  // namespace cypher
