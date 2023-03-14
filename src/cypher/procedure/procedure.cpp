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
#include "server/state_machine.h"
#include "restful/server/json_convert.h"
#include "cypher/graph/common.h"
#include "procedure.h"
#include "utils.h"
#include "butil/endpoint.h"
#include "cypher/monitor/memory_monitor_allocator.h"
#include "fma-common/encrypt.h"

namespace cypher {

#define CYPHER_ARG_CHECK(pred, msg)           \
    if (!(pred)) {                            \
        throw lgraph::ReminderException(msg); \
    }

#define CYPHER_DB_PROCEDURE_GRAPH_CHECK()                                \
    do {                                                                 \
        if (ctx->graph_.empty())                                         \
            throw lgraph::CypherException("graph name cannot be empty"); \
        if (!ctx->ac_db_) CYPHER_INTL_ERR();                             \
        if (!ctx->txn_) CYPHER_INTL_ERR();                               \
    } while(0)

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

void BuiltinProcedure::DbSubgraph(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                  const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 1, "This function takes 1 argrument. e.g. db.DbSubGraph(vids)");
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::LIST, "db.DbSubGraph(vids): `vids` must be list");
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    std::set<lgraph::VertexId> set_vids;
    for (auto &vid : args[0].List()) {
        CYPHER_ARG_CHECK(vid.type == parser::Expression::INT, "db.DbSubGraph(vids): `vid` must be int");
        set_vids.emplace(vid.Int());
    }
    auto vit = ctx->txn_->GetVertexIterator();
    std::vector<nlohmann::json> vertices;
    std::vector<nlohmann::json> relationships;
    for (auto vid : set_vids) {
        if (!vit.Goto(vid, false)) continue;
        lgraph_api::lgraph_result::Node node;
        node.id = vid;
        node.label = ctx->txn_->GetVertexLabel(vit);
        for (auto &property : ctx->txn_->GetVertexFields(vit)) {
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
            repl.label = ctx->txn_->GetEdgeLabel(eit);
            repl.forward = true;
            repl.tid = uid.tid;
            auto rel_fields = ctx->txn_->GetEdgeFields(eit);
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

void BuiltinProcedure::DbVertexLabels(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                      const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.vertexLabels()",
                                           args.size()))
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    auto labels = ctx->txn_->GetAllLabels(true);
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
    auto labels = ctx->txn_->GetAllLabels(false);
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
    auto indexes = ctx->txn_->ListVertexIndexes();
    for (auto &i : indexes) {
        Record r;
        r.AddConstant(lgraph::FieldData(i.label));
        r.AddConstant(lgraph::FieldData(i.field));
        r.AddConstant(lgraph::FieldData(i.unique));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbListLabelIndexes(RTContext *ctx, const Record *record,
                                          const VEC_EXPR &args, const VEC_STR &yield_items,
                                          std::vector<cypher::Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 1, FMA_FMT("Function requires 1 arguments, but {} are "
                                               "given. Usage: db.listLabelIndexes(label_name)",
                                               args.size()))

    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING,
                     FMA_FMT("{} has to be a string ", args[0].String()))

    auto label = args[0].String();
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    auto indexes = ctx->txn_->ListVertexIndexByLabel(label);
    for (auto &i : indexes) {
        if (i.label != label) continue;
        Record r;
        r.AddConstant(lgraph::FieldData(i.label));
        r.AddConstant(lgraph::FieldData(i.field));
        r.AddConstant(lgraph::FieldData(i.unique));
        records->emplace_back(r.Snapshot());
    }
}
void BuiltinProcedure::DbPropertyKeys(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                      const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_TODO();
}

void BuiltinProcedure::DbWarmUp(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                const VEC_STR &yield_items, std::vector<cypher::Record> *records) {
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

void BuiltinProcedure::DbmsProcedures(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                      const VEC_STR &yield_items,
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
        {"name", [](const Procedure &p, Record &r) { r.AddConstant(lgraph::FieldData(p.signature.proc_name)); }},
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

void BuiltinProcedure::_ExtractFds(const VEC_EXPR &args, std::string &label, std::string &extra,
                                   std::vector<lgraph::FieldSpec> &fds) {
    using namespace parser;
    CYPHER_ARG_CHECK(args.size() % SPEC_MEMBER_SIZE == 2,
                     "Not enough arguments. This function takes 2 or more arguments.")
    CYPHER_ARG_CHECK(args[0].type == Expression::STRING,
                     FMA_FMT("{} has to be a string ", args[0].ToString()))
    CYPHER_ARG_CHECK(args[1].type == Expression::STRING,
                     FMA_FMT("{} has to be a string ", args[0].ToString()))

    label = args[0].String();
    extra = args[1].String();
    for (int i = 0; i < (int)args.size() / 3; i++) {
        CYPHER_ARG_CHECK(
            args[i * SPEC_MEMBER_SIZE + 2].type == Expression::STRING,
            FMA_FMT("{} has to be a string ", args[i * SPEC_MEMBER_SIZE + 2].ToString()))
        CYPHER_ARG_CHECK(
            args[i * SPEC_MEMBER_SIZE + 3].type == Expression::VARIABLE,
            FMA_FMT("{} has to be a variable，alternative value is "
                    "(NUL,BOOL,INT8,INT16,INT32,INT64,FLOAT,DOUBLE,DATE,DATETIME,STRING,BLOB) ",
                    args[i * SPEC_MEMBER_SIZE + 3].ToString()))
        CYPHER_ARG_CHECK(
            args[i * SPEC_MEMBER_SIZE + 4].type == Expression::BOOL,
            FMA_FMT("{} has to be a boolean ", args[i * SPEC_MEMBER_SIZE + 4].ToString()))
        auto name = args[i * SPEC_MEMBER_SIZE + 2].String();
        auto type_name = args[i * SPEC_MEMBER_SIZE + 3].String();
        auto nullable = args[i * SPEC_MEMBER_SIZE + 4].Bool();

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
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING,
                     "wrong arguments type，has to be a string");
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::MAP,
                     "wrong arguments type， has to be a map");
    role = args[0].String();
    std::unordered_map<std::string, lgraph::AccessLevel> temp_levels;
    for (auto &kv : args[1].Map()) {
        auto it = ValidAccessLevels.find(kv.second.String());
        CYPHER_ARG_CHECK(it != ValidAccessLevels.end(), "unknown access level");
        temp_levels[kv.first] = it->second;
    }
    std::swap(leves, temp_levels);
}

static ::std::vector<::lgraph::FieldSpec> ParseFieldSpecs(const VEC_EXPR &args, size_t start_from) {
    using namespace parser;
    std::vector<lgraph::FieldSpec> ret;
    for (size_t i = start_from; i < args.size(); i++) {
        CYPHER_ARG_CHECK(args[i].type == Expression::LIST && args[i].List().size() == 3,
                         "Each FieldSpec must be a list of ('name', 'type' 'optional')");
        const auto &list = args[i].List();
        CYPHER_ARG_CHECK(list[0].type == Expression::STRING &&
                             list[1].type == Expression::VARIABLE &&
                             list[2].type == Expression::BOOL,
                         "Each FieldSpec must be a list of (name, type [,optional])")
        lgraph::FieldType ft;
        if (!lgraph::field_data_helper::TryGetFieldType(list[1].String(), ft))
            throw lgraph::InputError(FMA_FMT("Illegal field type:{}", list[1]));
        ret.emplace_back(list[0].String(), ft, list[2].Bool());
    }
    return ret;
}

static bool ParseIsVertex(const parser::Expression &arg) {
    if (arg.type == parser::Expression::STRING) {
        if (fma_common::StringEqual(arg.String(), "vertex", false)) return true;
        if (fma_common::StringEqual(arg.String(), "edge", false)) return false;
    }
    throw lgraph::InputError("Wrong argument given for label type: must be 'vertex' or 'edge'.");
    return false;
}

static std::vector<std::string> ParseStringList(const parser::Expression &arg,
                                                const std::string &param_name) {
    std::vector<std::string> ret;
    if (arg.type == parser::Expression::STRING) {
        ret.push_back(arg.String());
    } else if (arg.type == parser::Expression::LIST) {
        for (auto &a : arg.List()) {
            if (_F_UNLIKELY(a.type != parser::Expression::STRING))
                throw lgraph::InputError(FMA_FMT(
                    "Illegal value for parameter [{}]: must be a string or list of strings.",
                    param_name));
            ret.push_back(a.String());
        }
    } else {
        throw lgraph::InputError(FMA_FMT(
            "Illegal value for parameter [{}]: must be a string or list of strings.", param_name));
    }
    return ret;
}

static std::string ParseStringArg(const parser::Expression &arg, const std::string &param_name) {
    if (_F_UNLIKELY(arg.type != parser::Expression::STRING))
        throw lgraph::InputError(
            FMA_FMT("Illegal value for parameter [{}]: must be a string.", param_name));
    return arg.String();
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
    auto ret = ctx->ac_db_->AddLabel(true, label, fds, primary_fd, {});
    if (!ret) {
        throw lgraph::LabelExistException(label, true);
    }
}

// params: vertex, label, primary_field,  [fieldspec1], [fieldspec2]...
// params: edge, label, edge_constraints, [fieldspec1], [fieldspec2]...
void BuiltinProcedure::DbCreateLabel(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                     const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    if (args.size() < 2)
        throw lgraph::InputError("Not enough arguments. This function takes 2 or more arguments.");
    bool is_vertex = ParseIsVertex(args[0]);
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
                lgraph::InputError("The size of each constraint tuple should be 2");
            }
            if (!item[0].is_string() || !item[1].is_string()) {
                lgraph::InputError("The element of each constraint tuple should be string type");
            }
            edge_constraints.push_back(std::make_pair(item[0], item[1]));
        }
    }
    auto field_specs = ParseFieldSpecs(args, 3);
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    auto ret = ac_db.AddLabel(is_vertex, label, field_specs, primary_fd, edge_constraints);
    if (!ret) {
        throw lgraph::LabelExistException(label, is_vertex);
    }
}

// params: vertex/edge, label
void BuiltinProcedure::DbGetLabelSchema(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                        const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (args.size() != 2)
        throw lgraph::InputError(
            "Wrong number of arguments. This function takes exactly 2 arguments.");
    bool is_vertex = ParseIsVertex(args[0]);
    std::string label = ParseStringArg(args[1], "label_name");
    auto fs = ctx->txn_->GetSchema(is_vertex, label);
    for (auto &f : fs) {
        Record r;
        r.AddConstant(lgraph::FieldData(f.name));
        r.AddConstant(lgraph::FieldData(lgraph::field_data_helper::FieldTypeName(f.type)));
        r.AddConstant(lgraph::FieldData(f.optional));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbGetVertexSchema(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                         const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 1, "need one parameters, e.g. db.getVertexSchema(label)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "label type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    const lgraph::Schema *schema = ctx->txn_->GetSchema(args[0].String(), true);
    Record r;
    r.AddConstant(lgraph::FieldData(ValueToJson(schema).serialize()));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbGetEdgeSchema(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                       const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 1, "need one parameters, e.g. db.getEdgeSchema(label)");
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "label type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    const lgraph::Schema *schema = ctx->txn_->GetSchema(args[0].String(), false);
    Record r;
    r.AddConstant(lgraph::FieldData(ValueToJson(schema).serialize()));
    records->emplace_back(r.Snapshot());
}

// params: vertex/edge, label
void BuiltinProcedure::DbDeleteLabel(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                     const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. db.deleteLabel(label_type, label_name)");
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    bool is_vertex = ParseIsVertex(args[0]);
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
                                             const VEC_EXPR &args, const VEC_STR &yield_items,
                                             std::vector<Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    if (args.size() != 3)
        throw lgraph::InputError(
            "Wrong number of arguments. This function takes exactly 3 arguments.");
    bool is_vertex = ParseIsVertex(args[0]);
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
                                             const VEC_EXPR &args, const VEC_STR &yield_items,
                                             std::vector<Record> *records) {
    using namespace parser;
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    if (args.size() < 3)
        throw lgraph::InputError("Too few arguments. This function takes 3 or more arguments.");
    bool is_vertex = ParseIsVertex(args[0]);
    std::string label = ParseStringArg(args[1], "label_name");
    // get field_spec_value
    std::vector<lgraph::FieldSpec> fields;
    std::vector<lgraph::FieldData> values;
    for (size_t i = 2; i < args.size(); i++) {
        CYPHER_ARG_CHECK(
            args[i].type == Expression::LIST && args[i].List().size() == 4,
            "Each FieldSpec must be a list of ('name', 'type', default_value, 'optional')");
        const auto &list = args[i].List();
        CYPHER_ARG_CHECK(list[0].type == Expression::STRING &&
                             list[1].type == Expression::VARIABLE &&
                             list[3].type == Expression::BOOL,
                         "Each FieldSpec must be a list of (name, type, default_value ,optional)");
        const std::string &name = list[0].String();
        lgraph::FieldType ft;
        if (!lgraph::field_data_helper::TryGetFieldType(list[1].String(), ft))
            throw lgraph::InputError(FMA_FMT("Illegal field type:{}", list[1]));
        lgraph::FieldData default_value = parser::MakeFieldData(list[2]);
        fields.emplace_back(name, ft, list[3].Bool());
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
                                             const VEC_EXPR &args, const VEC_STR &yield_items,
                                             std::vector<Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    if (args.size() < 3)
        throw lgraph::InputError(
            "Wrong number of arguments. This function takes 3 or more arguments.");
    bool is_vertex = ParseIsVertex(args[0]);
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

void BuiltinProcedure::DbCreateEdgeLabel(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                         const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() % SPEC_MEMBER_SIZE == 2,
        "e.g. db.createEdgeLabel(label_name, extra, field_name, field_type, is_unique)")
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
    auto ret = ac_db.AddLabel(false, label, fds, {}, edge_constraints);
    if (!ret) {
        throw lgraph::LabelExistException(label, true);
    }
}

void BuiltinProcedure::DbAddVertexIndex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                        const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 3,
                     "need 3 parameters, e.g. db.addIndex(label_name, field_name, is_unique)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "label_name type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING, "field_name type should be string")
    CYPHER_ARG_CHECK(args[2].type == parser::Expression::BOOL, "is_unique type should be boolean")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    auto label = args[0].String();
    auto field = args[1].String();
    auto unique = args[2].Bool();
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    bool success = ac_db.AddVertexIndex(label, field, unique);
    if (!success) {
        throw lgraph::IndexExistException(label, field);
    }
}

void BuiltinProcedure::DbAddEdgeIndex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                      const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 3,
                     "need 3 parameters, e.g. db.addIndex(label_name, field_name, is_unique)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "label_name type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING, "field_name type should be string")
    CYPHER_ARG_CHECK(args[2].type == parser::Expression::BOOL, "is_unique type should be boolean")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    auto label = args[0].String();
    auto field = args[1].String();
    auto unique = args[2].Bool();
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    bool success = ac_db.AddEdgeIndex(label, field, unique);
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
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::BOOL, "is_vertex type should be boolean")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING, "label_name type should be string")
    CYPHER_ARG_CHECK(args[2].type == parser::Expression::STRING, "field_name type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    auto is_vertex = args[0].Bool();
    auto label = args[1].String();
    auto field = args[2].String();
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    bool success = ac_db.AddFullTextIndex(is_vertex, label, field);
    if (!success) {
        throw lgraph::FullTextIndexExistException(label, field);
    }
}

void BuiltinProcedure::DbDeleteFullTextIndex(RTContext *ctx, const Record *record,
                                             const VEC_EXPR &args, const VEC_STR &yield_items,
                                             std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() == 3,
        "need 3 parameters, e.g. db.deleteFullTextIndex(is_vertex, label_name, field_name)");

    CYPHER_ARG_CHECK(args[0].type == parser::Expression::BOOL, "is_vertex type should be boolean")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING, "label_name type should be string")
    CYPHER_ARG_CHECK(args[2].type == parser::Expression::STRING, "field_name type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    bool success = db.DeleteFullTextIndex(args[0].Bool(), args[1].String(), args[2].String());
    if (!success) {
        throw lgraph::FullTextIndexNotExistException(args[1].String(), args[2].String());
    }
}

void BuiltinProcedure::DbRebuildFullTextIndex(RTContext *ctx, const Record *record,
                                              const VEC_EXPR &args, const VEC_STR &yield_items,
                                              std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need 2 parameters, e.g. db.rebuildFullTextIndex(vertex_labels, edge_labels)");
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING,
                     "vertex_labels should be a json array string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING,
                     "edge_labels should be a json array string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    std::set<std::string> v_labels, e_labels;
    auto vs = nlohmann::json::parse(args[0].String());
    if (!vs.is_array()) {
        throw lgraph::InputError("vertex_labels should be a json array string");
    }
    for (auto &item : vs) {
        v_labels.emplace(item);
    }
    auto es = nlohmann::json::parse(args[1].String());
    if (!es.is_array()) {
        throw lgraph::InputError("edge_labels should be a json array string");
    }
    for (auto &item : es) {
        e_labels.emplace(item);
    }
    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    db.RebuildFullTextIndex(v_labels, e_labels);
}

void BuiltinProcedure::DbFullTextIndexes(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                         const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.FullTextIndexes()",
                                           args.size()))
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    const auto &ft_indexs = ctx->txn_->ListFullTextIndexes();
    for (const auto &ft_index : ft_indexs) {
        Record r;
        r.AddConstant(lgraph::FieldData(std::get<0>(ft_index)));
        r.AddConstant(lgraph::FieldData(std::get<1>(ft_index)));
        r.AddConstant(lgraph::FieldData(std::get<2>(ft_index)));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbClearEdgeConstraints(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                            const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 1,
                     "need 1 parameters, e.g. db.clearEdgeConstraints(label_name)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING,
                     "label_name type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    auto label = args[0].String();
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    ac_db.ClearEdgeConstraints(label);
}

void BuiltinProcedure::DbAddEdgeConstraints(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                        const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need 2 parameters, e.g. db.addEdgeConstraints(label_name, constraints)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING,
                     "label_name type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING,
                     "constraints type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    auto label = args[0].String();
    auto constraints = args[1].String();
    std::vector<std::pair<std::string, std::string>> edge_constraints;
    auto ec = nlohmann::json::parse(constraints);
    for (auto &item : ec) {
        edge_constraints.emplace_back(item[0], item[1]);
    }
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    ac_db.AddEdgeConstraints(label, edge_constraints);
}

void BuiltinProcedure::DbmsSecurityChangePassword(RTContext *ctx, const cypher::Record *record,
                                                  const cypher::VEC_EXPR &args,
                                                  const cypher::VEC_STR &yield_items,
                                                  std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() == 2,
        "need 2 parameters, e.g. dbms.security.changePassword(current_password, new_password)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING,
                     "current_password type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING,
                     "new_password type should be string")
    if (ctx->txn_) ctx->txn_->Abort();
    bool success =
        ctx->galaxy_->ChangeCurrentPassword(ctx->user_, args[0].String(), args[1].String());
    if (!success) {
        throw lgraph::UserNotExistException(ctx->user_);
    }
}

void BuiltinProcedure::DbmsSecurityChangeUserPassword(RTContext *ctx, const cypher::Record *record,
                                                      const cypher::VEC_EXPR &args,
                                                      const cypher::VEC_STR &yield_items,
                                                      std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() == 2,
        "need 2 parameters, e.g. dbms.security.changeUserPassword(current_password, new_password)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING,
                     "current_password type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING,
                     "new_password type should be string")

    bool success = ctx->galaxy_->ChangeUserPassword(ctx->user_, args[0].String(), args[1].String());
    if (!success) {
        throw lgraph::UserNotExistException(args[0].String());
    }
}

void BuiltinProcedure::DbmsSecuritySetUserMemoryLimit(RTContext *ctx, const cypher::Record *record,
                                                      const cypher::VEC_EXPR &args,
                                                      const cypher::VEC_STR &yield_items,
                                                      std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() == 2,
        "need 2 parameters, e.g. dbms.security.setUserMemoryLimit(user_name, memory_limit)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "user_name should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::INT,
                     "User_Memory_Limit must be an integer");

    bool success = ctx->galaxy_->SetUserMemoryLimit(ctx->user_, args[0].String(), args[1].Int());
    if (!success) {
        throw lgraph::UserNotExistException(args[0].String());
    }
}

void BuiltinProcedure::DbmsSecurityCreateUser(RTContext *ctx, const cypher::Record *record,
                                              const cypher::VEC_EXPR &args,
                                              const cypher::VEC_STR &yield_items,
                                              std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need 2 parameters, e.g. dbms.security.createUser(user_name, password)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "user_name type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING, "password type should be string")
    bool success = ctx->galaxy_->CreateUser(ctx->user_, args[0].String(), args[1].String(),
                                            args.size() == 2 ? "" : args[2].String());
    if (!success) {
        throw lgraph::UserExistException(args[0].String());
    }
}

void BuiltinProcedure::DbmsSecurityDeleteUser(RTContext *ctx, const cypher::Record *record,
                                              const cypher::VEC_EXPR &args,
                                              const cypher::VEC_STR &yield_items,
                                              std::vector<cypher::Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 1,
                     "need 1 parameters, e.g. dbms.security.deleteUser(user_name)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "user_name type should be string")
    bool success = ctx->galaxy_->DeleteUser(ctx->user_, args[0].String());
    if (!success) {
        throw lgraph::UserNotExistException(args[0].String());
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
        CYPHER_ARG_CHECK(ip.type == parser::Expression::STRING, "Host names must be strings.")
        CYPHER_ARG_CHECK(butil::str2ip(ip.String().c_str(), &my_ip) == 0, "Invalid host");
        ips.push_back(ip.String());
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
        CYPHER_ARG_CHECK(ip.type == parser::Expression::STRING, "Host names must be strings.")
        ips.push_back(ip.String());
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
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "graph_name must be string");

    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    lgraph::DBConfig config;
    if (args.size() >= 2) {
        CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING, "description gmust be string");
        config.desc = args[1].String();
    }
    if (args.size() >= 3) {
        CYPHER_ARG_CHECK(args[2].type == parser::Expression::INT, "Max_size_GB must be an integer");
        config.db_size = ((size_t)args[2].Int()) << 30;
    }
    bool success = ctx->galaxy_->CreateGraph(ctx->user_, args[0].String(), config);
    if (!success) {
        throw lgraph::GraphExistException(args[0].String());
    }
}

void BuiltinProcedure::DbmsGraphModGraph(RTContext *ctx, const cypher::Record *record,
                                         const cypher::VEC_EXPR &args,
                                         const cypher::VEC_STR &yield_items,
                                         std::vector<Record> *records) {
    CYPHER_ARG_CHECK(
        args.size() == 2,
        "This function takes exactly 2 arguments. e.g.  dbms.graph.modGraph(graph_name,config)");
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "graph_name must be string");
    /* close the previous txn first, in case of nested transaction */
    if (ctx->txn_) ctx->txn_->Abort();
    lgraph::DBConfig config;
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::MAP, "Illega must be map");
    lgraph::GraphManager::ModGraphActions act;
    act.mod_size = false;
    act.mod_desc = false;
    for (auto &kv : args[1].Map()) {
        if (kv.first == "max_size_GB") {
            act.mod_size = true;
            if (kv.second.type != parser::Expression::INT)
                throw lgraph::InputError("Invalid value for max_size_GB: must be integer");
            act.max_size = kv.second.Int() << 30;
        } else if (kv.first == "description") {
            act.mod_desc = true;
            if (kv.second.type != parser::Expression::STRING)
                throw lgraph::InputError("Invalid value for description: must be string");
            act.desc = kv.second.String();
        } else {
            throw lgraph::InputError("Invalid config key: " + kv.first);
        }
    }
    bool success = ctx->galaxy_->ModGraph(ctx->user_, args[0].String(), act);
    if (!success) {
        throw lgraph::GraphNotExistException(args[0].String());
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
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "graph_name must be string")

    /* close the previous txn first, in case of nested transaction */
    bool success = ctx->galaxy_->DeleteGraph(ctx->user_, args[0].String());
    if (!success) {
        throw lgraph::GraphNotExistException(args[0].String());
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

void BuiltinProcedure::DbmsGraphGetGraphInfo(RTContext *ctx, const Record *record,
                                             const VEC_EXPR &args, const VEC_STR &yield_items,
                                             std::vector<cypher::Record> *records) {
    if (ctx->txn_) ctx->txn_->Abort();
    CYPHER_ARG_CHECK(args.size() == 1, FMA_FMT("This function takes exactly 1 arguments, but {} "
                                               "given. Usage: dbms.graph.getGraphInfo(graph_name)",
                                               args.size()))

    auto graph_ref = ctx->galaxy_->OpenGraph(ctx->user_, args[0].String());
    const lgraph::DBConfig &conf = graph_ref.GetLightningGraph()->GetConfig();
    Record r;
    r.AddConstant(lgraph::FieldData(args[0].String()));
    r.AddConstant(lgraph::FieldData(lgraph::ValueToJson(conf).serialize()));
    records->emplace_back(r.Snapshot());
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
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::MAP, "role type should be map")
    std::map<std::string, lgraph::FieldData> kvs;
    for (auto &kv : args[0].Map()) {
        kvs[kv.first] = parser::MakeFieldData(kv.second);
    }
    bool need_reload = ctx->galaxy_->UpdateConfig(ctx->user_, kvs);
    // restart galaxy in a separate thread
    if (need_reload) ctx->galaxy_->ReloadFromDisk();
}

void BuiltinProcedure::DbmsListBackupLogFiles(RTContext *ctx, const cypher::Record *record,
                                              const cypher::VEC_EXPR &args,
                                              const cypher::VEC_STR &yield_items,
                                              std::vector<cypher::Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.listBackupFiles()",
                                           args.size()))
    if (!ctx->sm_) throw lgraph::InputError("Cannot be called in embedded mode.");
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
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.takeSnapshot()",
                                           args.size()))
    if (!ctx->sm_) throw lgraph::InputError("Cannot be called in embedded mode.");
    std::string path = ctx->sm_->TakeSnapshot();
    Record r;
    r.AddConstant(lgraph::FieldData(path));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsSecurityListRoles(RTContext *ctx, const cypher::Record *record,
                                             const cypher::VEC_EXPR &args,
                                             const cypher::VEC_STR &yield_items,
                                             std::vector<cypher::Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.security.listRoles()",
                                           args.size()))
    if (ctx->txn_) ctx->txn_->Abort();
    std::map<std::string, lgraph::AclManager::RoleInfo> rs = ctx->galaxy_->ListRoles(ctx->user_);
    for (auto &u : rs) {
        Record r;
        r.AddConstant(lgraph::FieldData(u.first));
        r.AddConstant(lgraph::FieldData(ValueToJson(u.second).serialize()));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbmsSecurityCreateRole(RTContext *ctx, const Record *record,
                                              const VEC_EXPR &args, const VEC_STR &yield_items,
                                              std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. dbms.security.createRole(role, desc)");
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "role type should be string");
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING, "desc type should be string");
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->CreateRole(ctx->user_, args[0].String(), args[1].String());
    if (!success) {
        throw lgraph::RoleExistException(args[0].String());
    }
}

void BuiltinProcedure::DbmsSecurityDeleteRole(RTContext *ctx, const Record *record,
                                              const VEC_EXPR &args, const VEC_STR &yield_items,
                                              std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 1, "need one parameters, e.g. dbms.security.deleteRole(role)");
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "role type should be string");
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->DeleteRole(ctx->user_, args[0].String());
    if (!success) {
        throw lgraph::RoleNotExistException(args[0].String());
    }
}

void BuiltinProcedure::DbmsSecurityGetUserInfo(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args, const VEC_STR &yield_items,
                                               std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 1, "need one parameters, e.g. dbms.security.getUserInfo(user)");
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "user type should be string");
    if (ctx->txn_) ctx->txn_->Abort();
    auto uinfo = ctx->galaxy_->GetUserInfo(ctx->user_, args[0].String());
    Record r;
    r.AddConstant(lgraph::FieldData(ValueToJson(uinfo).serialize()));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsSecurityGetUserPermissions(RTContext *ctx, const Record *record,
                                                      const VEC_EXPR &args,
                                                      const VEC_STR &yield_items,
                                                      std::vector<cypher::Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 1,
                     "need one parameters, e.g. dbms.security.getUserPermissions(user_name)");
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "user_name type should be string");
    auto uinfo = ctx->galaxy_->ListUserGraphs(ctx->user_, args[0].String());
    if (ctx->txn_) ctx->txn_->Abort();
    Record r;
    r.AddConstant(lgraph::FieldData(lgraph::ValueToJson(uinfo).serialize()));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsSecurityGetUserMemoryUsage(RTContext *ctx, const Record *record,
                                                      const VEC_EXPR &args,
                                                      const VEC_STR &yield_items,
                                                      std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 1, "need one parameters, e.g. dbms.security.getUserInfo(user)");
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "user type should be string");
    if (ctx->txn_) ctx->txn_->Abort();
    int64_t usage = AllocatorManager.GetMemoryUsage(ctx->user_);
    Record r;
    r.AddConstant(lgraph::FieldData(usage));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsSecurityGetRoleInfo(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args, const VEC_STR &yield_items,
                                               std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 1, "need one parameters, e.g. dbms.security.getRoleInfo(role)");
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "role type should be string");
    auto rinfo = ctx->galaxy_->GetRoleInfo(ctx->user_, args[0].String());
    if (ctx->txn_) ctx->txn_->Abort();
    Record r;
    r.AddConstant(lgraph::FieldData(ValueToJson(rinfo).serialize()));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsSecurityDisableRole(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args, const VEC_STR &yield_items,
                                               std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. dbms.security.disableRole(role, disable)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "role type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::BOOL, "disable type should be boolean")
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->ModRoleDisable(args[0].String(), args[1].Bool());
    if (!success) {
        throw lgraph::RoleNotExistException(args[0].String());
    }
}

void BuiltinProcedure::DbmsSecurityModRoleDesc(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args, const VEC_STR &yield_items,
                                               std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. dbms.security.modRoleDesc(role, description)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "role type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING,
                     "description type should be string")
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->ModRoleDesc(args[0].String(), args[1].String());
    if (!success) {
        throw lgraph::RoleNotExistException(args[0].String());
    }
}

void BuiltinProcedure::DbmsSecurityRebuildRoleAccessLevel(RTContext *ctx, const Record *record,
                                                          const VEC_EXPR &args,
                                                          const VEC_STR &yield_items,
                                                          std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(
        args.size() == 2,
        "need two parameters, e.g. dbms.security.modAllRoleAccessLevel(role, access_level)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "role type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::MAP,
                     "access_level type should be map, key and val both string")
    std::string role;
    std::unordered_map<std::string, lgraph::AccessLevel> levels;
    _ExtractAccessLevel(args, role, levels);
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->ModAllRoleAccessLevel(role, levels);
    if (!success) {
        throw lgraph::RoleNotExistException(args[0].String());
    }
}

void BuiltinProcedure::DbmsSecurityModRoleAccessLevel(RTContext *ctx, const Record *record,
                                                      const VEC_EXPR &args,
                                                      const VEC_STR &yield_items,
                                                      std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(
        args.size() == 2,
        "need two parameters, e.g. dbms.security.modRoleAccessLevel(role, access_level)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "role type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::MAP,
                     "access_level type should be map, key and val both string")
    std::string role;
    std::unordered_map<std::string, lgraph::AccessLevel> levels;
    _ExtractAccessLevel(args, role, levels);
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->ModRoleAccessLevel(role, levels);
    if (!success) {
        throw lgraph::RoleNotExistException(args[0].String());
    }
}

void BuiltinProcedure::DbmsSecurityModRoleFieldAccessLevel(RTContext *ctx, const Record *record,
                                                           const VEC_EXPR &args,
                                                           const VEC_STR &yield_items,
                                                           std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 6,
                     "need five parameters, "
                     "e.g. dbms.security.modRoleFieldAccessLevel"
                     "(role, graph, label, field, label_type, field_access_level)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "role type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING, "graph type should be string")
    CYPHER_ARG_CHECK(args[2].type == parser::Expression::STRING, "label type should be string")
    CYPHER_ARG_CHECK(args[3].type == parser::Expression::STRING, "field type should be string")
    CYPHER_ARG_CHECK(args[4].type == parser::Expression::STRING, "label_type type should be string")
    CYPHER_ARG_CHECK(args[5].type == parser::Expression::STRING,
                     "field_access_level type should be string")
    cypher::VEC_STR titles = ProcedureTitles("dbms.security.modRoleFieldAccessLevel", yield_items);
    std::string role = args[0].String();
    std::string graph = args[1].String();
    std::string label = args[2].String();
    std::string field = args[3].String();
    std::string label_type = args[4].String();
    std::string field_access_level = args[5].String();
    // is_vertex
    bool is_vertex = true;
    if (label_type == "VERTEX" or label_type == "vertex")
        is_vertex = true;
    else if (label_type == "EDGE" or label_type == "edge")
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
                                               const VEC_EXPR &args, const VEC_STR &yield_items,
                                               std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. dbms.security.disableUser(user, disable)");
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "user type should be string");
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::BOOL, "disable type should be boolean");
    std::string modified_user = args[0].String();
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->ModUserDisable(ctx->user_, modified_user, args[1].Bool());
    if (!success) {
        throw lgraph::UserNotExistException(args[0].String());
    }
}

void BuiltinProcedure::DbmsSecuritySetCurrentDesc(RTContext *ctx, const Record *record,
                                                  const VEC_EXPR &args, const VEC_STR &yield_items,
                                                  std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 1,
                     "need one parameters, e.g. dbms.security.setCurrentDesc(description)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING,
                     "description type should be string")

    std::string modified_user = args[0].String();
    if (modified_user != ctx->user_ && !ctx->galaxy_->IsAdmin(ctx->user_))
        throw lgraph::AuthError("Non-admin user cannot modify other users.");
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->SetUserDescription(ctx->user_, ctx->user_, args[0].String());
    if (!success) {
        throw lgraph::UserNotExistException(ctx->user_);
    }
}

void BuiltinProcedure::DbmsSecuritySetUserDesc(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args, const VEC_STR &yield_items,
                                               std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. dbms.security.setUserDesc(user, description)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "user type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING,
                     "description type should be string")

    std::string modified_user = args[0].String();
    if (modified_user != ctx->user_ && !ctx->galaxy_->IsAdmin(ctx->user_))
        throw lgraph::AuthError("Non-admin user cannot modify other users.");
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->SetUserDescription(ctx->user_, args[0].String(), args[1].String());
    if (!success) {
        throw lgraph::UserNotExistException(args[0].String());
    }
}

void BuiltinProcedure::DbmsSecurityDeleteUserRoles(RTContext *ctx, const Record *record,
                                                   const VEC_EXPR &args, const VEC_STR &yield_items,
                                                   std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. dbms.security.deleteUserRoles(user, roles)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "user type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::LIST, "roles type should be list")
    std::vector<std::string> roles;
    for (auto &a : args[1].List()) {
        if (_F_UNLIKELY(a.type != parser::Expression::STRING))
            throw lgraph::InputError("Illegal value for roles: must be a string list .");
        roles.push_back(a.String());
    }
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->DeleteUserRoles(ctx->user_, args[0].String(), roles);
    if (!success) {
        throw lgraph::UserNotExistException(args[0].String());
    }
}

void BuiltinProcedure::DbmsSecurityRebuildUserRoles(RTContext *ctx, const Record *record,
                                                    const VEC_EXPR &args,
                                                    const VEC_STR &yield_items,
                                                    std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. dbms.security.rebuildUserRoles(user, roles)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "user type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::LIST, "roles type should be list")
    std::vector<std::string> roles;
    for (auto &a : args[1].List()) {
        if (_F_UNLIKELY(a.type != parser::Expression::STRING))
            throw lgraph::InputError("Illegal value for roles: must be a string list .");
        roles.push_back(a.String());
    }
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->RebuildUserRoles(ctx->user_, args[0].String(), roles);
    if (!success) {
        throw lgraph::UserNotExistException(args[0].String());
    }
}

void BuiltinProcedure::DbmsSecurityAddUserRoles(RTContext *ctx, const Record *record,
                                                const VEC_EXPR &args, const VEC_STR &yield_items,
                                                std::vector<Record> *records) {
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. dbms.security.addUserRoles(user, roles)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "user type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::LIST, "roles type should be list")
    std::vector<std::string> roles;
    for (auto &a : args[1].List()) {
        if (_F_UNLIKELY(a.type != parser::Expression::STRING))
            throw lgraph::InputError("Illegal value for roles: must be a string list .");
        roles.push_back(a.String());
    }
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->galaxy_->AddUserRoles(ctx->user_, args[0].String(), roles);
    if (!success) {
        throw lgraph::UserNotExistException(args[0].String());
    }
}

void BuiltinProcedure::DbPluginLoadPlugin(RTContext *ctx, const Record *record,
                                          const VEC_EXPR &args, const VEC_STR &yield_items,
                                          std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 6,
                     "need six parameters, e.g. db.plugin.loadPlugin(plugin_type,"
                     "plugin_name, plugin_content, code_type, plugin_description, read_only)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING,
                     "plugin_type type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING,
                     "plugin_name type should be string")
    CYPHER_ARG_CHECK(args[2].type == parser::Expression::STRING,
                     "plugin_content type should be string")
    CYPHER_ARG_CHECK(args[3].type == parser::Expression::STRING, "code_type type should be string")
    CYPHER_ARG_CHECK(args[4].type == parser::Expression::STRING,
                     "plugin_description type should be string")
    CYPHER_ARG_CHECK(args[5].type == parser::Expression::BOOL, "read_only type should be boolean")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    auto plugin_type_it = ValidPluginType.find(args[0].String());
    CYPHER_ARG_CHECK(plugin_type_it != ValidPluginType.end(),
                     "unknown plugin_type, one of ('CPP', 'PY')");
    auto code_type_it = ValidPluginCodeType.find(args[3].String());
    CYPHER_ARG_CHECK(code_type_it != ValidPluginCodeType.end(),
                     "unknown plugin_type, one of ('PY', 'SO', 'CPP', 'ZIP')");
    fma_common::encrypt::Base64 base64;
    std::string content = base64.Decode(args[2].String());
    bool success = db.LoadPlugin(plugin_type_it->second, ctx->user_, args[1].String(), content,
                                 code_type_it->second, args[4].String(), args[5].Bool());
    if (!success) {
        throw lgraph::PluginExistException(args[1].String());
    }
}

void BuiltinProcedure::DbPluginDeletePlugin(RTContext *ctx, const Record *record,
                                            const VEC_EXPR &args, const VEC_STR &yield_items,
                                            std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. db.plugin.deletePlugin(plugin_type, plugin_name)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING,
                     "plugin_type type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING,
                     "plugin_name type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    auto plugin_type_it = ValidPluginType.find(args[0].String());
    CYPHER_ARG_CHECK(plugin_type_it != ValidPluginType.end(),
                     "unknown plugin_type, one of ('CPP', 'PY')")
    bool success = db.DelPlugin(plugin_type_it->second, ctx->user_, args[1].String());
    if (!success) {
        throw lgraph::PluginNotExistException(args[1].String());
    }
}

void BuiltinProcedure::DbPluginGetPluginInfo(RTContext *ctx, const Record *record,
                                             const VEC_EXPR &args, const VEC_STR &yield_items,
                                             std::vector<cypher::Record> *records) {
    CYPHER_ARG_CHECK((args.size() == 2 || args.size() == 3),
                     "need two or three parameters, e.g. db.plugin.getPluginInfo(plugin_type, "
                     "plugin_name,show_code=false)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING,
                     "plugin_type type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING,
                     "plugin_name type should be string")
    bool show_code = false;
    if (args.size() == 3) {
        CYPHER_ARG_CHECK(args[2].type == parser::Expression::BOOL,
                         "show_code type should be boolean")
        show_code = args[2].Bool();
    }
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    auto plugin_type_it = ValidPluginType.find(args[0].String());
    CYPHER_ARG_CHECK(plugin_type_it != ValidPluginType.end(),
                     "unknown plugin_type, one of ('CPP', 'PY')")
    lgraph::PluginCode co;
    bool success = db.GetPluginCode(plugin_type_it->second, ctx->user_, args[1].String(), co);
    if (!success) {
        throw lgraph::PluginNotExistException(args[1].String());
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
    CYPHER_ARG_CHECK(args.size() == 1,
                     "need one parameters, e.g. db.plugin.listPlugin(plugin_type)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING,
                     "plugin_type type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    auto plugin_type_it = ValidPluginType.find(args[0].String());
    CYPHER_ARG_CHECK(plugin_type_it != ValidPluginType.end(),
                     "unknown plugin_type, one of ('CPP', 'PY')")
    std::vector<lgraph::PluginDesc> descs = db.ListPlugins(plugin_type_it->second, ctx->user_);
    for (auto &d : descs) {
        Record r;
        r.AddConstant(lgraph::FieldData(ValueToJson(d).serialize()));
        records->emplace_back(r.Snapshot());
    }
}

void BuiltinProcedure::DbPluginListUserPlugins(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args, const VEC_STR &yield_items,
                                               std::vector<Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.graph.listGraphs()",
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
                     "db.plugin.loadPlugin(plugin_type, plugin_name, param, timeout, in_process)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING,
                     "plugin_type type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING,
                     "plugin_name type should be string")
    CYPHER_ARG_CHECK(args[2].type == parser::Expression::STRING, "param type should be string")
    CYPHER_ARG_CHECK(args[3].type == parser::Expression::DOUBLE, "timeout type should be double")
    CYPHER_ARG_CHECK(args[4].type == parser::Expression::BOOL, "in_process type should be boolean")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    auto plugin_type_it = ValidPluginType.find(args[0].String());
    CYPHER_ARG_CHECK(plugin_type_it != ValidPluginType.end(),
                     "unknown plugin_type, one of ('CPP', 'PY')")
    lgraph::PluginManager::PluginType type = plugin_type_it->second;
    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    std::string name = args[1].String();
    lgraph::TimeoutTaskKiller timeout_killer;
    timeout_killer.SetTimeout(args[3].Double());
    std::string res;
    bool success = db.CallPlugin(type, ctx->user_, name, args[2].String(), args[3].Double(),
                                 args[4].Bool(), res);
    Record r;
    r.AddConstant(lgraph::FieldData(res));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbImportorDataImportor(RTContext *ctx, const Record *record,
                                              const VEC_EXPR &args, const VEC_STR &yield_items,
                                              std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 5,
                     "need five parameters, e.g. db.importor.dataImportor"
                     "(description, content, continue_on_error, thread_nums, delimiter)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING,
                     "description type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING, "content type should be string")
    CYPHER_ARG_CHECK(args[2].type == parser::Expression::BOOL,
                     "continue_on_error type should be boolean");
    CYPHER_ARG_CHECK(args[3].type == parser::Expression::INT, "thread_nums type should be integer")
    CYPHER_ARG_CHECK(args[4].type == parser::Expression::STRING, "delimiter type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();

    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    if (db.GetAccessLevel() < lgraph::AccessLevel::WRITE)
        throw lgraph::CypherException("Need write permission to do import.");
    lgraph::import_v2::ImportOnline::Config config;
    config.continue_on_error = args[2].Bool();
    config.n_threads = args[3].Int();
    config.delimiter = args[4].String();
    fma_common::encrypt::Base64 base64;
    std::string desc = base64.Decode(args[0].String());
    std::string content = base64.Decode(args[1].String());
    std::string log = lgraph::import_v2::ImportOnline::HandleOnlineTextPackage(
        std::move(desc), std::move(content), db.GetLightningGraph(), config);
}

void BuiltinProcedure::DbImportorSchemaImportor(RTContext *ctx, const Record *record,
                                                const VEC_EXPR &args, const VEC_STR &yield_items,
                                                std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 1,
                     "need one parameters, e.g. db.importor.schemaImportor(description)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING,
                     "description type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();

    lgraph::AccessControlledDB db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    if (db.GetAccessLevel() < lgraph::AccessLevel::WRITE)
        throw lgraph::CypherException("Need write permission to do import.");
    fma_common::encrypt::Base64 base64;
    std::string desc = base64.Decode(args[0].String());
    std::string log = ::lgraph::import_v2::ImportOnline::HandleOnlineSchema(std::move(desc), db);
}

void BuiltinProcedure::DbDeleteIndex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                     const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. db.deleteIndex(label_name, field_name)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "label_name type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING, "field_name type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->ac_db_->DeleteVertexIndex(args[0].String(), args[1].String());
    if (!success) {
        throw lgraph::IndexNotExistException(args[0].String(), args[1].String());
    }
}

void BuiltinProcedure::DbDeleteEdgeIndex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                         const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.size() == 2,
                     "need two parameters, e.g. db.deleteIndex(label_name, field_name)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "label_name type should be string")
    CYPHER_ARG_CHECK(args[1].type == parser::Expression::STRING, "field_name type should be string")
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    bool success = ctx->ac_db_->DeleteEdgeIndex(args[0].String(), args[1].String());
    if (!success) {
        throw lgraph::IndexNotExistException(args[0].String(), args[1].String());
    }
}

void BuiltinProcedure::DbFlushDB(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                 const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.flushDB()",
                                           args.size()))
    ctx->ac_db_->Flush();
}

void BuiltinProcedure::DbDropDB(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    if (ctx->txn_) ctx->txn_->Abort();
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.dropDB()",
                                           args.size()))
    ctx->ac_db_->DropAllData();
}

void BuiltinProcedure::DbTaskListTasks(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                       const VEC_STR &yield_items,
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
    if (!ctx->galaxy_->IsAdmin(ctx->user_)) throw lgraph::AuthError("Admin access right required.");
    CYPHER_ARG_CHECK(args.size() == 1, "need one parameters, e.g. dbms.task.terminateTask(task_id)")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::STRING, "task_id type should be string")
    lgraph::TaskTracker::TaskId task_id = lgraph::TaskTracker::TaskId();
    task_id.FromString(args[0].String());
    auto r = lgraph::TaskTracker::GetInstance().KillTask(task_id);
    switch (r) {
    case lgraph::TaskTracker::SUCCESS:
        return;
    case lgraph::TaskTracker::NOTFOUND:
        throw lgraph::TaskNotExistException(args[0].String());
    case lgraph::TaskTracker::FAIL_TO_KILL:
        throw lgraph::TaskKilledFailedException(args[0].String());
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
                                            const VEC_EXPR &args, const VEC_STR &yield_items,
                                            std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: db.monitor.tuGraphInfo()",
                                           args.size()))
    if (!ctx) CYPHER_INTL_ERR();
    Record r;
    r.AddConstant(lgraph::FieldData(ValueToJson(ctx->sm_->GetStats()).serialize()));
    records->emplace_back(r.Snapshot());
}

void BuiltinProcedure::DbmsHaClusterInfo(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                         const VEC_STR &yield_items, std::vector<Record> *records) {
    CYPHER_ARG_CHECK(args.empty(), FMA_FMT("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.ha.clusterInfo()",
                                           args.size()))
    if (ctx->txn_) ctx->txn_->Abort();
    if (!ctx->sm_->IsInHaMode())
        throw lgraph::InputError("The service should be started as a high availability cluster .");
    auto peers = ctx->sm_->ListPeers();
    Record r;
    r.AddConstant(lgraph::FieldData(lgraph::ValueToJson(peers).serialize()));
    records->emplace_back(r.Snapshot());
}

static void _FetchPath(lgraph::Transaction &txn, size_t hops,
                       std::unordered_map<lgraph::VertexId, lgraph::VertexId> &parent,
                       std::unordered_map<lgraph::VertexId, lgraph::VertexId> &child,
                       lgraph::VertexId vid_from, lgraph::VertexId vid_a, lgraph::VertexId vid_b,
                       lgraph::VertexId vid_to, cypher::Path &path) {
    std::vector<lgraph::VertexId> vids(hops + 1);
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
    for (int i = 0; i < (int)hops; i++) {
        // TODO(any): fake edges!
        path.Append(lgraph::EdgeUid(vids[i], vids[i + 1], 0, 0, 0));
    }
}

static std::vector<lgraph::VertexId> _GetNeighbors(lgraph::Transaction &txn, lgraph::VertexId vid,
                                                   const std::string &edge_label) {
    static fma_common::Logger &logger = fma_common::Logger::Get("cypher.algo.p2puwssp");
    auto vit = txn.GetVertexIterator(vid);
    CYPHER_THROW_ASSERT(vit.IsValid());
    std::vector<lgraph::VertexId> neighbors;
    if (edge_label.empty()) {
        bool out_edge_left = false, in_edge_left = false;
        neighbors = vit.ListDstVids(nullptr, nullptr, 10000, &out_edge_left, nullptr);
        auto srcIds = vit.ListSrcVids(nullptr, nullptr, 10000, &in_edge_left, nullptr);
        if (out_edge_left || in_edge_left) {
            FMA_WARN_STREAM(logger) << "Result trimmed down because " << vid
                                    << " has more than 10000 in/out neighbours";
        }
        neighbors.reserve(neighbors.size() + srcIds.size());
        neighbors.insert(neighbors.end(), srcIds.begin(), srcIds.end());
    } else {
        auto edge_lid = txn.GetLabelId(false, edge_label);
        for (auto eit = vit.GetOutEdgeIterator(lgraph::EdgeUid(vid, 0, edge_lid, 0, 0), true);
             eit.IsValid(); eit.Next()) {
            if (eit.GetLabelId() != edge_lid) break;
            neighbors.push_back(eit.GetDst());
        }
        for (auto eit = vit.GetInEdgeIterator(lgraph::EdgeUid(0, vid, edge_lid, 0, 0), true);
             eit.IsValid(); eit.Next()) {
            if (eit.GetLabelId() != edge_lid) break;
            neighbors.push_back(eit.GetSrc());
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
                                       size_t max_hops, cypher::Path &path) {
    path.Clear();
    path.SetStart(start_vid);
    if (start_vid == end_vid) return;
    std::unordered_map<lgraph::VertexId, lgraph::VertexId> parent{{start_vid, start_vid}};
    std::unordered_map<lgraph::VertexId, lgraph::VertexId> child{{end_vid, end_vid}};
    std::vector<lgraph::VertexId> forward_q{start_vid};
    std::vector<lgraph::VertexId> backward_q{end_vid};
    size_t hops = 0;
    while (hops++ < max_hops) {
        std::vector<lgraph::VertexId> next_q;
        // decide which way to search first
        if (forward_q.size() <= backward_q.size()) {
            // search forward
            for (auto vid : forward_q) {
                auto nbrs = _GetNeighbors(txn, vid, edge_label);
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
                auto nbrs = _GetNeighbors(txn, vid, edge_label);
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
                            const int depth, const std::string &edge_label, cypher::Path &path,
                            std::vector<cypher::Path> &paths) {
    if (depth != 0) {
        if (edge_label.empty()) {
            for (auto eit = txn.GetOutEdgeIterator(vid); eit.IsValid(); eit.Next()) {
                int64_t nbr = eit.GetDst();
                auto it = hop_info.find(nbr);
                if (it != hop_info.end() && it->second == depth - 1) {
                    path.Append(eit.GetUid());
                    _EnumeratePartialPaths(txn, hop_info, nbr, depth - 1, edge_label, path, paths);
                    path.PopBack();
                }
            }
            for (auto eit = txn.GetInEdgeIterator(vid); eit.IsValid(); eit.Next()) {
                int64_t nbr = eit.GetSrc();
                auto it = hop_info.find(nbr);
                if (it != hop_info.end() && it->second == depth - 1) {
                    path.Append(eit.GetUid());
                    _EnumeratePartialPaths(txn, hop_info, nbr, depth - 1, edge_label, path, paths);
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
                    _EnumeratePartialPaths(txn, hop_info, nbr, depth - 1, edge_label, path, paths);
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
                    _EnumeratePartialPaths(txn, hop_info, nbr, depth - 1, edge_label, path, paths);
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
        _EnumeratePartialPaths(txn, parent, fvid, parent[fvid], edge_label, path, fpaths);
        path.Clear();
        path.SetStart(bvid);
        _EnumeratePartialPaths(txn, child, bvid, child[bvid], edge_label, path, bpaths);
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
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::VARIABLE &&
                         args[1].type == parser::Expression::VARIABLE &&
                         (args.size() == 2 || args[2].type == parser::Expression::MAP),
                     "wrong type")
    std::string edge_label;
    size_t max_hops = 20;
    if (args.size() == 3) {
        auto &map = args[2].Map();
        auto it = map.find("relationshipQuery");
        if (it != map.end()) {
            if (it->second.type != parser::Expression::STRING) CYPHER_TODO();
            edge_label = it->second.String();
        }
        auto max = map.find("maxHops");
        if (max != map.end()) {
            if (max->second.type != parser::Expression::INT) CYPHER_TODO();
            max_hops = max->second.Int();
        }
    }
    CYPHER_THROW_ASSERT(record);
    auto it1 = record->symbol_table->symbols.find(args[0].String());
    auto it2 = record->symbol_table->symbols.find(args[1].String());
    if (it1 == record->symbol_table->symbols.end() || it2 == record->symbol_table->symbols.end()) {
        CYPHER_TODO();
    }
    auto start_vid = record->values[it1->second.id].node->PullVid();
    auto end_vid = record->values[it2->second.id].node->PullVid();
    cypher::Path path;
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    _P2PUnweightedShortestPath(*ctx->txn_, start_vid, end_vid, edge_label, max_hops, path);

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

void AlgoFunc::AllShortestPaths(RTContext *ctx, const Record *record, const cypher::VEC_EXPR &args,
                                const cypher::VEC_STR &yield_items,
                                std::vector<cypher::Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    CYPHER_ARG_CHECK(args.size() / 2 == 1, "wrong arguments number")
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::VARIABLE &&
                         args[1].type == parser::Expression::VARIABLE &&
                         (args.size() == 2 || args[2].type == parser::Expression::MAP),
                     "wrong type")
    std::string edge_label;
    if (args.size() == 3) {
        auto &map = args[2].Map();
        auto it = map.find("relationshipQuery");
        if (it != map.end()) {
            if (it->second.type != parser::Expression::STRING) CYPHER_TODO();
            edge_label = it->second.String();
        }
    }
    CYPHER_THROW_ASSERT(record);
    auto it1 = record->symbol_table->symbols.find(args[0].String());
    auto it2 = record->symbol_table->symbols.find(args[1].String());
    if (it1 == record->symbol_table->symbols.end() || it2 == record->symbol_table->symbols.end()) {
        CYPHER_TODO();
    }
    auto start_vid = record->values[it1->second.id].node->PullVid();
    auto end_vid = record->values[it2->second.id].node->PullVid();
    std::vector<cypher::Path> paths;
    auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    _P2PUnweightedAllShortestPaths(*ctx->txn_, start_vid, end_vid, edge_label, paths);

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
                             const cypher::VEC_EXPR &args, const cypher::VEC_STR &yield_items,
                             struct std::vector<cypher::Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    CYPHER_ARG_CHECK(args.size() / 2 == 1, "wrong arguments number")
    CYPHER_ARG_CHECK(
        args[0].type == parser::Expression::VARIABLE && args[1].type == parser::Expression::MAP,
        "wrong type")
    auto &config = args[1].Map();
    auto it1 = config.find("isNode");
    auto it2 = config.find("field");
    if (it1 == config.end() || it1->second.type != parser::Expression::BOOL ||
        it2 == config.end() || it2->second.type != parser::Expression::STRING) {
        CYPHER_TODO();
    }
    cypher::FieldData value;
    if (it1->second.Bool()) {
        CYPHER_THROW_ASSERT(record);
        auto i = record->symbol_table->symbols.find(args[0].String());
        if (i == record->symbol_table->symbols.end()) CYPHER_TODO();
        auto &vid = record->values[i->second.id];
        auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
        if (vid.IsArray()) {
            value = cypher::FieldData::Array(0);
            for (auto &id : *vid.constant.array) {
                value.array->emplace_back(
                    ctx->txn_->GetVertexField(id.AsInt64(), it2->second.String()));
            }
        } else {
            CYPHER_TODO();
        }
    } else {
        CYPHER_THROW_ASSERT(record);
        auto i = record->symbol_table->symbols.find(args[0].String());
        if (i == record->symbol_table->symbols.end()) CYPHER_TODO();
        auto &eid = record->values[i->second.id];
        if (!eid.IsString()) CYPHER_TODO();
        auto ac_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
        value = ctx->txn_->GetEdgeField(_detail::ExtractEdgeUid(eid.constant.scalar.AsString()),
                                        it2->second.String());
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
    std::unordered_map<std::string, std::function<void(const cypher::FieldData &, Record &)>> lmap =
        {
            // NOLINT
            {"value", [&](const cypher::FieldData &d, Record &r) { r.AddConstant(d); }},
        };
    Record r;
    for (auto &title : titles) {
        lmap.find(title)->second(value, r);
    }
    records->emplace_back(r.Snapshot());
}

void AlgoFunc::PageRank(RTContext *ctx, const cypher::Record *record, const cypher::VEC_EXPR &args,
                        const cypher::VEC_STR &yield_items,
                        struct std::vector<cypher::Record> *records) {
    CYPHER_DB_PROCEDURE_GRAPH_CHECK();
    CYPHER_ARG_CHECK(args.size() == 1, "args number should be 1");
    CYPHER_ARG_CHECK(args[0].type == parser::Expression::INT, "arg is not int");

    std::map<int64_t, std::pair<double, int>> node_prs_curr;
    std::map<int64_t, std::pair<double, int>> node_prs_next;
    auto num_iterations = args[0].Int();
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
        if (title_exsited.find("node") != title_exsited.end()) {
            cypher::Node n;
            n.SetVid(node_pr.first);
            r.AddNode(&n);
        }
        if (title_exsited.find("pr") != title_exsited.end()) {
            r.AddConstant(lgraph::FieldData(node_pr.second.first));
        }
        records->emplace_back(r.Snapshot());
    }
}
}  // namespace cypher
