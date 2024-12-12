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

#include "cypher/procedure/procedure.h"
#include "common/logger.h"
#include "common/flags.h"
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace cypher {

#define CYPHER_ARG_CHECK(pred, msg)           \
    if (!(pred)) {                            \
        THROW_CODE(ReminderException, msg);   \
    }

#define CYPHER_DB_PROCEDURE_GRAPH_CHECK()                                                     \
    do {                                                                                      \
        if (ctx->graph_.empty()) throw lgraph::CypherException("graph name cannot be empty"); \
        if (!ctx->ac_db_) CYPHER_INTL_ERR();                                                  \
        if (!ctx->txn_) CYPHER_INTL_ERR();                                                    \
    } while (0)

/* argument and return value type */
const std::map<ProcedureResultType, std::string> ResultTypeNames = {
    {ProcedureResultType::Value, "Value"},
    {ProcedureResultType::Node, "Node"},
    {ProcedureResultType::Relationship, "Relationship"}
};

std::vector<Procedure> global_procedures = {
    Procedure(
        "log.setLevel", LogFunc::SetLevel,
        Procedure::SIG_SPEC{{"level", {0, ProcedureResultType::Value}}},
        Procedure::SIG_SPEC{}
        ),
    Procedure(
        "log.queryLog", LogFunc::QueryLog,
        Procedure::SIG_SPEC{{"enable", {0, ProcedureResultType::Value}}},
        Procedure::SIG_SPEC{}
        ),
    Procedure(
        "algo.pagerank", AlgoFunc::PageRank,
        Procedure::SIG_SPEC{{"config", {0, ProcedureResultType::Value}}},
        Procedure::SIG_SPEC{{"node", {0, ProcedureResultType::Node}},
                            {"score", {1, ProcedureResultType::Value}}}
    ),
    Procedure(
    "dbms.procedures", BuiltinProcedure::DbmsProcedures,
        Procedure::SIG_SPEC{},
    Procedure::SIG_SPEC{{"name", {0, ProcedureResultType::Value}},
                        {"signature", {1, ProcedureResultType::Value}}}
    ),
    Procedure(
        "db.labels", BuiltinProcedure::DbLabels,
        Procedure::SIG_SPEC{},
        Procedure::SIG_SPEC{{"label", {0, ProcedureResultType::Value}}}
    ),
    Procedure(
        "db.relationshipTypes", BuiltinProcedure::DbRelationshipTypes,
        Procedure::SIG_SPEC{},
        Procedure::SIG_SPEC{{"relationshipType", {0, ProcedureResultType::Value}}}
        ),
    Procedure(
        "db.createUniquePropertyConstraint", BuiltinProcedure::DbCreateUniquePropertyConstraint,
        Procedure::SIG_SPEC{{"index_name", {0, ProcedureResultType::Value}},
                            {"label", {1, ProcedureResultType::Value}},
                            {"property", {2, ProcedureResultType::Value}}},
        Procedure::SIG_SPEC{}),
    Procedure(
        "db.deleteUniquePropertyConstraint", BuiltinProcedure::DbDeleteUniquePropertyConstraint,
        Procedure::SIG_SPEC{{"index_name", {0, ProcedureResultType::Value}}},
        Procedure::SIG_SPEC{}),
    Procedure(
        "db.index.fulltext.createNodeIndex", BuiltinProcedure::DbIndexFullTextCreateNodeIndex,
        Procedure::SIG_SPEC{{"index_name", {0, ProcedureResultType::Value}},
                            {"labels", {1, ProcedureResultType::Value}},
                            {"properties", {2, ProcedureResultType::Value}}},
        Procedure::SIG_SPEC{}),
    Procedure(
        "db.index.fulltext.queryNodes", BuiltinProcedure::DbIndexFullTextQueryNodes,
        Procedure::SIG_SPEC{{"index_name", {0, ProcedureResultType::Value}},
                            {"query", {1, ProcedureResultType::Value}}},
        Procedure::SIG_SPEC{{"node", {0, ProcedureResultType::Node}},
                            {"score", {1, ProcedureResultType::Value}}}),
    Procedure(
        "db.index.fulltext.deleteIndex", BuiltinProcedure::DbIndexFullTextDeleteIndex,
        Procedure::SIG_SPEC{{"index_name", {0, ProcedureResultType::Value}}},
        Procedure::SIG_SPEC{}),
    Procedure(
        "db.index.fulltext.applyWal", BuiltinProcedure::DbIndexFullTextApplyWal,
        Procedure::SIG_SPEC{},
        Procedure::SIG_SPEC{}
        ),
    Procedure(
        "db.index.vector.createNodeIndex", BuiltinProcedure::DbIndexVectorCreateNodeIndex,
        Procedure::SIG_SPEC{{"index_name", {0, ProcedureResultType::Value}},
                            {"label", {1, ProcedureResultType::Value}},
                            {"property", {2, ProcedureResultType::Value}},
                            {"parameter", {3, ProcedureResultType::Value}}},
        Procedure::SIG_SPEC{}),
    Procedure(
        "db.index.vector.knnSearchNodes", BuiltinProcedure::DbIndexVectorKnnSearchNodes,
        Procedure::SIG_SPEC{{"index_name", {0, ProcedureResultType::Value}},
                            {"query", {1, ProcedureResultType::Value}},
                            {"parameter", {2, ProcedureResultType::Value}}},
        Procedure::SIG_SPEC{{"node", {0, ProcedureResultType::Node}},
                            {"distance", {1, ProcedureResultType::Value}}}),
    Procedure(
        "db.index.vector.applyWal", BuiltinProcedure::DbIndexVectorApplyWal,
        Procedure::SIG_SPEC{},
        Procedure::SIG_SPEC{}
        ),
    Procedure(
        "db.index.vector.deleteIndex", BuiltinProcedure::DbIndexVectorDeleteIndex,
        Procedure::SIG_SPEC{{"index_name", {0, ProcedureResultType::Value}}},
        Procedure::SIG_SPEC{}),
    Procedure(
        "dbms.graph.createGraph", BuiltinProcedure::DbmsGraphCreateGraph,
        Procedure::SIG_SPEC{{"graph_name", {0, ProcedureResultType::Value}}},
        Procedure::SIG_SPEC{}
    ),
    Procedure(
        "dbms.graph.deleteGraph", BuiltinProcedure::DbmsGraphDeleteGraph,
        Procedure::SIG_SPEC{{"graph_name", {0, ProcedureResultType::Value}}},
        Procedure::SIG_SPEC{}
    ),
    Procedure(
        "db.dropDB", BuiltinProcedure::DbDropDB,
        Procedure::SIG_SPEC{},
        Procedure::SIG_SPEC{}
    ),
    Procedure(
        "dbms.graph.listGraph", BuiltinProcedure::DbmsGraphListGraph,
        Procedure::SIG_SPEC{},
        Procedure::SIG_SPEC{{"id", {0, ProcedureResultType::Value}},
                            {"name", {1, ProcedureResultType::Value}}}
    ),
    Procedure(
        "db.showIndexes", BuiltinProcedure::DbShowIndexes,
        Procedure::SIG_SPEC{},
        Procedure::SIG_SPEC{{"name", {0, ProcedureResultType::Value}},
                            {"type", {1, ProcedureResultType::Value}},
                            {"entityType", {2, ProcedureResultType::Value}},
                            {"labelsOrTypes", {3, ProcedureResultType::Value}},
                            {"properties", {4, ProcedureResultType::Value}},
                            {"otherInfo", {4, ProcedureResultType::Value}}}
        ),
};

ProcedureTable global_ptable;
std::string Procedure::Signature() const {
    std::string s;
    std::map<int, std::string> args, res;
    for (auto &p : signature.input_list) {
        std::string str;
        str.append(p.name).append("::").append(ResultTypeNames.at(p.type));
        args.emplace(p.index, str);
    }
    for (auto &r : signature.result_list) {
        std::string str;
        str.append(r.name).append("::").append(ResultTypeNames.at(r.type));
        res.emplace(r.index, str);
    }
    s.append(proc_name).append("(");
    size_t i = 0;
    for (auto &a : args) {
        s.append(a.second);
        if (++i < signature.input_list.size()) s.append(",");
    }
    s.append(") :: (");
    i = 0;
    for (auto &r : res) {
        s.append(r.second);
        if (++i < signature.result_list.size()) s.append(",");
    }
    s.append(")");
    return s;
}

void ProcedureTable::RegisterStandaloneFuncs() {
    for (auto &p : global_procedures) {
        ptable_.emplace(p.proc_name, p);
    }
}

void LogFunc::SetLevel(cypher::RTContext *ctx, const cypher::Record *record,
                       const cypher::VEC_EXPR &args,
                       const cypher::VEC_STR &yield_items,
                       std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.size() == 1, fmt::format("Function requires 1 arguments, but {} are "
                                                   "given. Usage: log.setLevel(level)",
                                                   args.size()))
    CYPHER_ARG_CHECK(args[0].IsString(), "level type should be String")
    auto level = args[0].constant.AsString();
    std::set<std::string> levels = {"trace", "debug", "info", "warning", "error", "critical", "off", "warn", "err"};
    if (!levels.count(level)) {
        THROW_CODE(InvalidParameter, "Invalid level string:{}", level);
    }
    LOG_INFO("Set the log level to {}", level);
    spdlog::default_logger()->set_level(spdlog::level::from_str(level));
}

void LogFunc::QueryLog(cypher::RTContext *ctx, const cypher::Record *record,
                        const cypher::VEC_EXPR &args,
                        const cypher::VEC_STR &yield_items,
                        std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.size() == 1, fmt::format("Function requires 1 arguments, but {} are "
                                                   "given. Usage: log.queryLog(bool)",
                                                   args.size()))
    CYPHER_ARG_CHECK(args[0].IsBool(), "enable type should be Bool")
    auto enable = args[0].constant.AsBool();
    if (enable) {
        if (!FLAGS_enable_query_log) {
            LOG_INFO("Enable query log");
            SetupQueryLogger();
            FLAGS_enable_query_log = true;
        }
    } else {
        if (FLAGS_enable_query_log) {
            LOG_INFO("Disable query log");
            FLAGS_enable_query_log = false;
            spdlog::drop("query_logger");
        }
    }
}

void AlgoFunc::PageRank(cypher::RTContext *ctx, const cypher::Record *record,
                        const cypher::VEC_EXPR &args,
                        const cypher::VEC_STR &yield_items,
                        std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.size() == 1, fmt::format("Function requires 1 arguments, but {} are "
                                               "given. Usage: algo.pagerank(config)",
                                               args.size()))
    CYPHER_ARG_CHECK(args[0].IsMap(), "config type should be Map")
    auto config = args[0].constant.AsMap();
    if (!config.count("num_iterations")) {
        THROW_CODE(InvalidParameter, "num_iterations is required");
    }
    auto num_iterations = config.at("num_iterations").AsInteger();
    auto d = (double)0.85;
    if (config.count("damping_factor")) {
        d = config.at("damping_factor").AsDouble();
    }
    CYPHER_ARG_CHECK((d < 1 && d >= 0), "damping_factor should be in [0, 1)");

    std::map<int64_t, std::pair<double, int>> node_prs_curr;
    std::map<int64_t, std::pair<double, int>> node_prs_next;
    auto vertex_num = 0;
    for (auto viter = ctx->txn_->NewVertexIterator(); viter->Valid(); viter->Next()) {
        auto vertex = viter->GetVertex();
        auto out_degree = vertex.GetDegree(graphdb::EdgeDirection::OUTGOING);
        if (out_degree == 0) {
            node_prs_curr[vertex.GetId()] = std::make_pair(double(1), out_degree);
        } else {
            node_prs_curr[vertex.GetId()] =
                std::make_pair(double(1.0 / out_degree), out_degree);
        }
        vertex_num++;
    }
    if (vertex_num == 0) {
        return;
    }
    while (num_iterations > 0) {
        for (auto viter = ctx->txn_->NewVertexIterator(); viter->Valid(); viter->Next()) {
            double sum = 0;
            auto vertex = viter->GetVertex();
            auto vid = vertex.GetId();
            for (auto eiter = vertex.NewEdgeIterator(graphdb::EdgeDirection::INCOMING, {}, {}); eiter->Valid(); eiter->Next()) {
                auto src = eiter->GetEdge().GetStartId();
                if (node_prs_curr[src].second == 0) {
                    sum += node_prs_curr[src].first;
                } else {
                    sum += node_prs_curr[src].first / node_prs_curr[src].second;
                }
            }
            node_prs_next[vid].first = (1 - d) / vertex_num + d * sum;
            node_prs_next[vid].second = node_prs_curr[vid].second;
        }
        node_prs_curr = node_prs_next;
        num_iterations--;
    }
    for (auto &node_pr : node_prs_curr) {
        std::vector<ProcedureResult> r;
        for (auto& yield : yield_items) {
            if (yield == "node") {
                r.emplace_back(graphdb::Vertex(ctx->txn_,node_pr.first));
            } else if (yield == "score") {
                r.emplace_back(Value(node_pr.second.first));
            }
        }
        records->emplace_back(std::move(r));
    }
}

void BuiltinProcedure::DbmsProcedures(RTContext *ctx, const Record *record,
                                      const VEC_EXPR &args, const VEC_STR &yield_items,
                                      std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.empty(), fmt::format("Function requires 0 arguments, but {} are "
                                           "given. Usage: dbms.procedures()",
                                           args.size()))
    auto pp = global_ptable.GetProcedure("dbms.procedures");
    CYPHER_THROW_ASSERT(pp && pp->ContainsYieldItem("name") && pp->ContainsYieldItem("signature"));
    cypher::VEC_STR titles;
    if (yield_items.empty()) {
        titles.emplace_back("name");
        titles.emplace_back("signature");
    } else {
        for (auto &item : yield_items) titles.emplace_back(item);
    }
    std::unordered_map<std::string, std::function<void(const Procedure &, std::vector<ProcedureResult> &)>> lmap = {
        {"name",
         [](const Procedure &p, std::vector<ProcedureResult> &r) { r.emplace_back(Value(p.proc_name)); }},
        {"signature",
         [](const Procedure &p, std::vector<ProcedureResult> &r) { r.emplace_back(Value(p.Signature())); }}
    };
    for (auto &p : global_procedures) {
        std::vector<ProcedureResult> r;
        for (auto &title : titles) {
            auto iter = lmap.find(title);
            if (iter == lmap.end()) {
                THROW_CODE(CypherException, "No such yield item: {}", title);
            }
            iter->second(p, r);
        }
        records->emplace_back(std::move(r));
    }
}

void BuiltinProcedure::DbLabels(cypher::RTContext *ctx, const cypher::Record *record,
                                const cypher::VEC_EXPR &args, const cypher::VEC_STR &yield_items,
                                std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.empty(), fmt::format("Function requires 0 arguments, but {} are "
                                               "given. Usage: db.labels()",
                                               args.size()))
    auto labels = ctx->txn_->db()->id_generator().GetVertexLabels();
    for (auto& label : labels) {
        std::vector<ProcedureResult> r;
        r.emplace_back(Value(label));
        records->emplace_back(std::move(r));
    }
}

void BuiltinProcedure::DbRelationshipTypes(cypher::RTContext *ctx, const cypher::Record *record,
                                const cypher::VEC_EXPR &args, const cypher::VEC_STR &yield_items,
                                std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.empty(), fmt::format("Function requires 0 arguments, but {} are "
                                               "given. Usage: db.relationshipTypes()",
                                               args.size()))
    auto types = ctx->txn_->db()->id_generator().GetEdgeTypes();
    for (auto& type : types) {
        std::vector<ProcedureResult> r;
        r.emplace_back(Value(type));
        records->emplace_back(std::move(r));
    }
}

void BuiltinProcedure::DbCreateUniquePropertyConstraint(cypher::RTContext *ctx, const cypher::Record *record,
                                           const cypher::VEC_EXPR &args, const cypher::VEC_STR &yield_items,
                                           std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.size() == 3, fmt::format("Function requires 3 arguments, but {} are "
                                               "given. Usage: db.createUniquePropertyConstraint(constraintName, label, property)",
                                               args.size()))
    CYPHER_ARG_CHECK(args[0].IsString(), "index_name type should be String")
    CYPHER_ARG_CHECK(args[1].IsString(), "label type should be String")
    CYPHER_ARG_CHECK(args[2].IsString(), "property type should be String")
    auto index_name = args[0].constant.AsString();
    auto label = args[1].constant.AsString();
    auto property = args[2].constant.AsString();
    LOG_INFO("Create unique property constraint, name:{}, label:{}, property:{}", index_name, label, property);
    ctx->txn_->db()->AddVertexPropertyIndex(index_name, true, label, property);
}

void BuiltinProcedure::DbDeleteUniquePropertyConstraint(cypher::RTContext *ctx, const cypher::Record *record,
                                                        const cypher::VEC_EXPR &args, const cypher::VEC_STR &yield_items,
                                                        std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.size() == 1, fmt::format("Function requires 1 arguments, but {} are "
                                                   "given. Usage: db.deleteUniquePropertyConstraint(constraintName)",
                                                   args.size()))
    CYPHER_ARG_CHECK(args[0].IsString(), "index_name type should be String")
    auto index_name = args[0].constant.AsString();
    LOG_INFO("Delete unique property constraint {}", index_name);
    ctx->txn_->db()->DeleteVertexPropertyIndex(index_name);
}

void BuiltinProcedure::DbIndexVectorCreateNodeIndex(
    cypher::RTContext *ctx, const cypher::Record *record,
    const cypher::VEC_EXPR &args, const cypher::VEC_STR &yield_items,
    std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.size() == 4, fmt::format("Function requires 4 arguments, but {} are "
                                                   "given. Usage: db.index.vector.createNodeIndex(index_name, label, property, parameter)",
                                                   args.size()))
    CYPHER_ARG_CHECK(args[0].IsString(), "index_name type should be String")
    CYPHER_ARG_CHECK(args[1].IsString(), "label type should be String")
    CYPHER_ARG_CHECK(args[2].IsString(), "property type should be String")
    CYPHER_ARG_CHECK(args[3].IsMap(), "parameter type should be Map")
    auto index_name = args[0].constant.AsString();
    auto label = args[1].constant.AsString();
    auto property = args[2].constant.AsString();
    auto parameter = args[3].constant.AsMap();
    int dimension = 128;
    if (parameter.count("dimension")) {
        dimension = (int)parameter.at("dimension").AsInteger();
    } else {
        THROW_CODE(InvalidParameter, "dimension is required");
    }
    CYPHER_ARG_CHECK(dimension >= 1 && dimension <= 4096,
                     "dimension should be an integer in the range [1, 4096]");
    std::string distance_type = "l2";
    if (parameter.count("distance_type")) {
        distance_type = parameter.at("distance_type").AsString();
    }
    CYPHER_ARG_CHECK((distance_type == "l2" || distance_type == "ip"),
                     "Distance type should be one of them : l2, ip");
    int hnsw_m = 16;
    if (parameter.count("hnsw_m")) {
        hnsw_m = (int)parameter.at("hnsw_m").AsInteger();
    }
    CYPHER_ARG_CHECK((hnsw_m <= 64 && hnsw_m >= 5),
                     "hnsw.m should be an integer in the range [5, 64]");
    int hnsw_ef_construction = 100;
    if (parameter.count("hnsw_ef_construction")) {
        hnsw_ef_construction = (int)parameter.at("hnsw_ef_construction").AsInteger();
    }
    CYPHER_ARG_CHECK((hnsw_ef_construction <= 1000 && hnsw_ef_construction >= hnsw_m),
                     "hnsw.efConstruction should be an integer in the range [hnsw.m,1000]");
    LOG_INFO("Create node vector index, name:{}, label:{}, property:{}, parameter:{}",
             index_name, label, property, parameter);
    ctx->txn_->db()->AddVertexVectorIndex(index_name, label, property,
                                          dimension, distance_type, hnsw_m, hnsw_ef_construction);
}

void BuiltinProcedure::DbIndexVectorKnnSearchNodes(cypher::RTContext *ctx, const Record *record,
                                                   const VEC_EXPR &args, const VEC_STR &yield_items,
                                                   std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.size() == 3, fmt::format("Function requires 3 arguments, but {} are "
                                                   "given. Usage: db.index.vector.knnSearchNodes(index_name, query, parameter)",
                                                   args.size()))
    CYPHER_ARG_CHECK(args[0].IsString(), "index_name type should be String")
    CYPHER_ARG_CHECK(args[1].IsArray(), "query type should be Array")
    CYPHER_ARG_CHECK(args[2].IsMap(), "parameter type should be Map")
    auto index_name = args[0].constant.AsString();
    auto query = args[1].constant.AsArray();
    std::vector<float> vectors;
    for (auto& v : query) {
        CYPHER_ARG_CHECK(v.IsDouble(), "parameter type should be DoubleArray")
        vectors.push_back((float)v.AsDouble());
    }
    int top_k = 10;
    int ef_search = 200;
    auto parameter = args[2].constant.AsMap();
    if (parameter.count("top_k")) {
        top_k = (int)parameter.at("top_k").AsInteger();
    }
    if (parameter.count("ef_search")) {
        ef_search = (int)parameter.at("ef_search").AsInteger();
    }
    for (auto viter = ctx->txn_->QueryVertexByKnnSearch(index_name, vectors, top_k, ef_search); viter->Valid(); viter->Next()) {
        std::vector<ProcedureResult> r;
        for (auto& yield : yield_items) {
            if (yield == "node") {
                r.emplace_back(viter->GetVertexScore().vertex);
            } else if (yield == "distance") {
                r.emplace_back(Value(viter->GetVertexScore().score));
            }
        }
        records->emplace_back(std::move(r));
    }
}

void BuiltinProcedure::DbIndexVectorApplyWal(
    RTContext *ctx, const Record *record, const VEC_EXPR &args,
    const VEC_STR &yield_items,
    std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.empty(), fmt::format("Function requires 0 arguments, but {} are "
                                               "given. Usage: db.index.vector.applyWal",
                                               args.size()))
    std::string name = ctx->txn_->db()->db_meta().graph_name();
    auto graphdb = server::g_galaxy->OpenGraph(name);
    for (auto& [_, index] : graphdb->meta_info().GetVertexVectorIndex()) {
        LOG_INFO("Manually apply WAL for Vector index {}", index->meta().name());
        index->ApplyWAL();
    }
}

void BuiltinProcedure::DbIndexVectorDeleteIndex(
    RTContext *ctx, const Record *record, const VEC_EXPR &args,
    const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.size() == 1, fmt::format("Function requires 1 arguments, but {} are "
                                                   "given. Usage: db.index.vector.deleteIndex(index_name)",
                                                   args.size()))
    CYPHER_ARG_CHECK(args[0].IsString(), "index_name type should be String")
    auto index_name = args[0].constant.AsString();
    LOG_INFO("Delete node vector index {}", index_name);
    ctx->txn_->db()->DeleteVertexVectorIndex(index_name);
}

void BuiltinProcedure::DbIndexFullTextCreateNodeIndex(
    cypher::RTContext *ctx, const cypher::Record *record,
    const cypher::VEC_EXPR &args, const cypher::VEC_STR &yield_items,
    std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.size() == 3, fmt::format("Function requires 3 arguments, but {} are "
                                               "given. Usage: db.index.fulltext.createNodeIndex(index_name, labels, properties)",
                                               args.size()))
    CYPHER_ARG_CHECK(args[0].IsString(), "index_name type should be String")
    CYPHER_ARG_CHECK(args[1].IsArray(), "labels type should be Array")
    CYPHER_ARG_CHECK(args[2].IsArray(), "properties type should be Array")
    auto index_name = args[0].constant.AsString();
    std::vector<std::string> labels, properties;
    for (auto& v : args[1].constant.AsArray()) {
        CYPHER_ARG_CHECK(v.IsString(), "labels type should be string Array")
        labels.push_back(v.AsString());
    }
    for (auto& v : args[2].constant.AsArray()) {
        CYPHER_ARG_CHECK(v.IsString(), "properties type should be StringArray")
        properties.push_back(v.AsString());
    }
    LOG_INFO("Create node fulltext index, name:{}, labels:{}, properties:{}",
             index_name, labels, properties);
    ctx->txn_->db()->AddVertexFullTextIndex(index_name, labels, properties);
}

void BuiltinProcedure::DbIndexFullTextDeleteIndex(
    RTContext *ctx,const Record *record, const VEC_EXPR &args,
    const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.size() == 1, fmt::format("Function requires 1 arguments, but {} are "
                                                   "given. Usage: db.index.fulltext.deleteIndex(index_name)",
                                                   args.size()))
    CYPHER_ARG_CHECK(args[0].IsString(), "index_name type should be String")
    auto index_name = args[0].constant.AsString();
    LOG_INFO("Delete node fulltext index {}", index_name);
    ctx->txn_->db()->DeleteVertexFullTextIndex(index_name);
}

void BuiltinProcedure::DbIndexFullTextQueryNodes(
    RTContext *ctx, const Record *record, const VEC_EXPR &args,
    const VEC_STR &yield_items,
    std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.size() == 3, fmt::format("Function requires 3 arguments, but {} are "
                                                   "given. Usage: db.index.fulltext.queryNodes(index_name, query, top_n)",
                                                   args.size()))
    CYPHER_ARG_CHECK(args[0].IsString(), "index_name type should be String")
    CYPHER_ARG_CHECK(args[1].IsString(), "query type should be String")
    CYPHER_ARG_CHECK(args[2].IsInteger(), "top_n type should be Integer")
    auto index_name = args[0].constant.AsString();
    auto query = args[1].constant.AsString();
    auto top_n = args[2].constant.AsInteger();
    for (auto viter = ctx->txn_->QueryVertexByFTIndex(index_name, query, top_n); viter->Valid(); viter->Next()) {
        std::vector<ProcedureResult> r;
        for (auto& yield : yield_items) {
            if (yield == "node") {
                r.emplace_back(viter->GetVertexScore().vertex);
            } else if (yield == "score") {
                r.emplace_back(Value(viter->GetVertexScore().score));
            }
        }
        records->emplace_back(std::move(r));
    }
}

void BuiltinProcedure::DbIndexFullTextApplyWal(
    RTContext *ctx, const Record *record, const VEC_EXPR &args,
    const VEC_STR &yield_items,
    std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.empty(), fmt::format("Function requires 0 arguments, but {} are "
                                                   "given. Usage: db.index.fulltext.applyWal",
                                                   args.size()))
    std::string name = ctx->txn_->db()->db_meta().graph_name();
    auto graphdb = server::g_galaxy->OpenGraph(name);
    for (auto& [_, index] : graphdb->meta_info().GetVertexFullTextIndex()) {
        LOG_INFO("Manually apply WAL for FullText index {}", index->Name());
        index->ApplyWAL();
    }
}

void BuiltinProcedure::DbmsGraphCreateGraph(
    RTContext *ctx, const Record *record, const VEC_EXPR &args,
    const VEC_STR &yield_items,
    std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.size() == 1, fmt::format("Function requires 3 arguments, but {} are "
                                                   "given. Usage: dbms.graph.createGraph('graph1')",
                                                   args.size()))
    CYPHER_ARG_CHECK(args[0].IsString(), "graph_name type should be String")
    auto name = args[0].constant.AsString();
    LOG_INFO("Create graph {}", name);
    server::g_galaxy->CreateGraph(name);
}

void BuiltinProcedure::DbmsGraphDeleteGraph(
    RTContext *ctx, const Record *record, const VEC_EXPR &args,
    const VEC_STR &yield_items,
    std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.size() == 1, fmt::format("Function requires 3 arguments, but {} are "
                                                   "given. Usage: dbms.graph.deleteGraph('graph1')",
                                                   args.size()))
    CYPHER_ARG_CHECK(args[0].IsString(), "graph_name type should be String")
    auto name = args[0].constant.AsString();
    LOG_INFO("Delete graph {}", name);
    server::g_galaxy->DeleteGraph(args[0].constant.AsString());
}

void BuiltinProcedure::DbmsGraphListGraph(
    RTContext *ctx, const Record *record, const VEC_EXPR &args,
    const VEC_STR &yield_items,
    std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.empty(), fmt::format("Function requires 0 arguments, but {} are "
                                                   "given. Usage: dbms.graph.listGraph()",
                                                   args.size()))
    for (auto& [_, graph] : server::g_galaxy->Graphs()) {
        std::vector<ProcedureResult> r;
        for (auto& yield : yield_items) {
            if (yield == "id") {
                r.emplace_back(Value::Integer(graph->db_meta().graph_id()));
            } else if (yield == "name") {
                r.emplace_back(Value::String(graph->db_meta().graph_name()));
            }
        }
        records->emplace_back(std::move(r));
    }
}

void BuiltinProcedure::DbDropDB(
    RTContext *ctx, const Record *record, const VEC_EXPR &args,
    const VEC_STR &yield_items,
    std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.empty(), fmt::format("Function requires 0 arguments, but {} are "
                                                   "given. Usage: db.dropDB()",
                                                   args.size()))
    std::string name = ctx->txn_->db()->db_meta().graph_name();
    LOG_INFO("dropDB {}", name);
    server::g_galaxy->DeleteGraph(name);
    server::g_galaxy->CreateGraph(name);
}

void BuiltinProcedure::DbShowIndexes(
    RTContext *ctx, const Record *record, const VEC_EXPR &args,
    const VEC_STR &yield_items,
    std::vector<std::vector<ProcedureResult>> *records) {
    CYPHER_ARG_CHECK(args.empty(), fmt::format("Function requires 0 arguments, but {} are "
                                               "given. Usage: db.showIndexes()",
                                               args.size()))
    std::string name = ctx->txn_->db()->db_meta().graph_name();
    auto graphdb = server::g_galaxy->OpenGraph(name);
    for (auto& [_, index] : graphdb->meta_info().GetVertexPropertyIndex()) {
        std::vector<ProcedureResult> r;
        for (auto& yield : yield_items) {
            if (yield == "name") {
                r.emplace_back(Value::String(index.meta().name()));
            } else if (yield == "type") {
                r.emplace_back(Value::String("Unique"));
            } else if (yield == "entityType") {
                r.emplace_back(Value::String("NODE"));
            } else if (yield == "labelsOrTypes") {
                r.emplace_back(Value::StringArray({index.meta().label()}));
            } else if (yield == "properties") {
                r.emplace_back(Value::StringArray({index.meta().property()}));
            } else if (yield == "otherInfo") {
                r.emplace_back(Value());
            }
        }
        records->emplace_back(std::move(r));
    }
    for (auto& [_, index] : graphdb->meta_info().GetVertexFullTextIndex()) {
        std::vector<ProcedureResult> r;
        for (auto& yield : yield_items) {
            if (yield == "name") {
                r.emplace_back(Value::String(index->meta().name()));
            } else if (yield == "type") {
                r.emplace_back(Value::String("FullText"));
            } else if (yield == "entityType") {
                r.emplace_back(Value::String("NODE"));
            } else if (yield == "labelsOrTypes") {
                r.emplace_back(Value::StringArray({index->meta().labels().begin(), index->meta().labels().end()}));
            } else if (yield == "properties") {
                r.emplace_back(Value::StringArray({index->meta().properties().begin(), index->meta().properties().end()}));
            } else if (yield == "otherInfo") {
                r.emplace_back(Value());
            }
        }
        records->emplace_back(std::move(r));
    }
    for (auto& [_, index] : graphdb->meta_info().GetVertexVectorIndex()) {
        std::vector<ProcedureResult> r;
        for (auto& yield : yield_items) {
            if (yield == "name") {
                r.emplace_back(Value::String(index->meta().name()));
            } else if (yield == "type") {
                r.emplace_back(Value::String("Vector"));
            } else if (yield == "entityType") {
                r.emplace_back(Value::String("NODE"));
            } else if (yield == "labelsOrTypes") {
                r.emplace_back(Value::StringArray({index->meta().label()}));
            } else if (yield == "properties") {
                r.emplace_back(Value::StringArray({index->meta().property()}));
            } else if (yield == "otherInfo") {
                std::unordered_map<std::string, Value> info;
                info["elementsNum"] = Value(index->GetElementsNum());
                info["deletedIdsNum"] = Value(index->GetDeletedIdsNum());
                info["shardingNum"] = Value((int64_t)index->meta().sharding_num());
                r.emplace_back(Value(std::move(info)));
            }
        }
        records->emplace_back(std::move(r));
    }
}

}  // namespace cypher
