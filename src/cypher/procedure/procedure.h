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
#pragma once

#include <iostream>
#include <utility>

#include "arithmetic/arithmetic_expression.h"
#include "parser/data_typedef.h"
#include "execution_plan/runtime_context.h"
#include "graph/graph.h"
#include "lgraph/lgraph_types.h"

namespace cypher {

/* argument and return value type */
static const std::map<lgraph_api::LGraphType, std::string> ResultTypeNames = {
    {lgraph_api::LGraphType::NUL, "NUL"},
    {lgraph_api::LGraphType::INTEGER, "INTEGER"},
    {lgraph_api::LGraphType::FLOAT, "FLOAT"},
    {lgraph_api::LGraphType::DOUBLE, "DOUBLE"},
    {lgraph_api::LGraphType::BOOLEAN, "BOOLEAN"},
    {lgraph_api::LGraphType::STRING, "STRING"},
    {lgraph_api::LGraphType::NODE, "NODE"},
    {lgraph_api::LGraphType::RELATIONSHIP, "RELATIONSHIP"},
    {lgraph_api::LGraphType::PATH, "PATH"},
    {lgraph_api::LGraphType::LIST, "LIST"},
    {lgraph_api::LGraphType::MAP, "MAP"},
    {lgraph_api::LGraphType::ANY, "ANY"}};

static const std::unordered_map<std::string, lgraph::AccessLevel> ValidAccessLevels = {
    {"NONE", lgraph::AccessLevel::NONE},
    {"READ", lgraph::AccessLevel::READ},
    {"WRITE", lgraph::AccessLevel::WRITE},
    {"FULL", lgraph::AccessLevel::FULL}};

static const std::unordered_map<std::string, lgraph::FieldAccessLevel> ValidFieldAccessLevels = {
    {"NONE", lgraph::FieldAccessLevel::NONE},
    {"READ", lgraph::FieldAccessLevel::READ},
    {"WRITE", lgraph::FieldAccessLevel::WRITE}};

static const std::unordered_map<std::string, lgraph::PluginManager::PluginType> ValidPluginType = {
    {"CPP", lgraph::PluginManager::PluginType::CPP},
    {"PY", lgraph::PluginManager::PluginType::PYTHON}};

static const std::unordered_map<std::string, lgraph::plugin::CodeType> ValidPluginCodeType = {
    {"PY", lgraph::plugin::CodeType::PY},
    {"SO", lgraph::plugin::CodeType::SO},
    {"CPP", lgraph::plugin::CodeType::CPP},
    {"ZIP", lgraph::plugin::CodeType::ZIP}};

static const int SPEC_MEMBER_SIZE = 3;

typedef std::vector<Entry> VEC_EXPR;
typedef std::vector<std::string> VEC_STR;
// TODO(anyone) procedure context
typedef std::function<void(RTContext *, const Record *, const VEC_EXPR &, const VEC_STR &,
                           std::vector<Record> *records)>
    SA_FUNC;

class BuiltinProcedure {
    static const std::unordered_map<std::string, lgraph::FieldType> type_map_;

    static void _ExtractFds(const VEC_EXPR &args, std::string &label, std::string &extra,
                            std::vector<lgraph::FieldSpec> &fds);

    static void _ExtractAccessLevel(const VEC_EXPR &args, std::string &label,
                                    std::unordered_map<std::string, lgraph::AccessLevel> &leves);

 public:
    static void DbSubgraph(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                           const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbVertexLabels(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                               const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbEdgeLabels(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                             const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbIndexes(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                          const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbPropertyKeys(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                               const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbWarmUp(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                         const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbUpsertVertex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                               const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbUpsertEdge(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                             const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbUpsertVertexByJson(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                     const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbUpsertEdgeByJson(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                     const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbCreateVertexLabel(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                    const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbCreateVertexLabelByJson(RTContext *ctx, const Record *record,
                                          const VEC_EXPR &args, const VEC_STR &yield_items,
                                          std::vector<Record> *records);

    static void DbCreateEdgeLabelByJson(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                          const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbGetLabelSchema(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                 const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbGetVertexSchema(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                  const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbGetEdgeSchema(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbCreateLabel(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                              const VEC_STR &yield_items, std::vector<Record> *records);
    static void DbDeleteLabel(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                              const VEC_STR &yield_items, std::vector<Record> *records);
    static void DbAlterLabelDelFields(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                      const VEC_STR &yield_items, std::vector<Record> *records);
    static void DbAlterLabelAddFields(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                      const VEC_STR &yield_items, std::vector<Record> *records);
    static void DbAlterLabelModFields(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                      const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbCreateEdgeLabel(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                  const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbAddVertexIndex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                 const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbAddVertexCompositeIndex(RTContext *ctx, const Record *record,
                                          const VEC_EXPR &args, const VEC_STR &yield_items,
                                          std::vector<Record> *records);

    static void DbAddEdgeIndex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                               const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbAddFullTextIndex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                   const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbDeleteFullTextIndex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                      const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbRebuildFullTextIndex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                       const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbFullTextIndexes(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                  const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbAddEdgeConstraints(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                     const VEC_STR &yield_items, std::vector<Record> *records);
    static void DbClearEdgeConstraints(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                       const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsProcedures(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                               const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsMetaCountDetail(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                    const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsMetaCount(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                              const VEC_STR &yield_items, std::vector<Record> *records);
    static void DbmsMetaRefreshCount(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                     const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsSecurityIsDefaultUserPassword(RTContext *ctx, const Record *record,
                                           const VEC_EXPR &args, const VEC_STR &yield_items,
                                           std::vector<Record> *records);

    static void DbmsSecurityChangePassword(RTContext *ctx, const Record *record,
                                           const VEC_EXPR &args, const VEC_STR &yield_items,
                                           std::vector<Record> *records);

    static void DbmsSecurityChangeUserPassword(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args, const VEC_STR &yield_items,
                                               std::vector<Record> *records);

    static void DbmsSecuritySetUserMemoryLimit(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args, const VEC_STR &yield_items,
                                               std::vector<Record> *records);

    static void DbmsSecurityCreateUser(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                       const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsSecurityDeleteUser(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                       const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsSecurityListUsers(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                      const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsSecurityShowCurrentUser(RTContext *ctx, const Record *record,
                                            const VEC_EXPR &args, const VEC_STR &yield_items,
                                            std::vector<Record> *records);

    static void DbmsSecurityHostWhitelistList(RTContext *ctx, const Record *record,
                                              const VEC_EXPR &args, const VEC_STR &yield_items,
                                              std::vector<Record> *records);

    static void DbmsSecurityHostWhitelistDelete(RTContext *ctx, const Record *record,
                                                const VEC_EXPR &args, const VEC_STR &yield_items,
                                                std::vector<Record> *records);

    static void DbmsSecurityHostWhitelistAdd(RTContext *ctx, const Record *record,
                                             const VEC_EXPR &args, const VEC_STR &yield_items,
                                             std::vector<Record> *records);

    static void DbmsGraphCreateGraph(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                     const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsGraphModGraph(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                  const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsGraphDeleteGraph(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                     const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsGraphListGraphs(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                    const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsGraphListUserGraphs(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                        const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsGraphGetGraphInfo(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                      const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsGraphGetGraphSchema(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                        const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsSystemInfo(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                               const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsConfigList(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                               const VEC_STR &yield_items, std::vector<Record> *records);
    static void DbmsConfigUpdate(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                 const VEC_STR &yield_items, std::vector<Record> *records);
    // take snapshot
    static void DbmsTakeSnapshot(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                 const VEC_STR &yield_items, std::vector<Record> *records);
    // list existing backup logs
    static void DbmsListBackupLogFiles(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                       const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsSecurityListRoles(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                      const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsSecurityCreateRole(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                       const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsSecurityDeleteRole(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                       const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsSecurityGetUserInfo(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                        const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsSecurityGetUserMemoryUsage(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args, const VEC_STR &yield_items,
                                               std::vector<Record> *records);

    static void DbmsSecurityGetUserPermissions(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args, const VEC_STR &yield_items,
                                               std::vector<Record> *records);

    static void DbmsSecurityGetRoleInfo(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                        const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsSecurityDisableRole(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                        const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsSecurityModRoleDesc(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                        const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsSecurityRebuildRoleAccessLevel(RTContext *ctx, const Record *record,
                                                   const VEC_EXPR &args, const VEC_STR &yield_items,
                                                   std::vector<Record> *records);

    static void DbmsSecurityModRoleAccessLevel(RTContext *ctx, const Record *record,
                                               const VEC_EXPR &args, const VEC_STR &yield_items,
                                               std::vector<Record> *records);

    static void DbmsSecurityModRoleFieldAccessLevel(RTContext *ctx, const Record *record,
                                                    const VEC_EXPR &args,
                                                    const VEC_STR &yield_items,
                                                    std::vector<Record> *records);

    static void DbmsSecurityDisableUser(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                        const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsSecuritySetCurrentDesc(RTContext *ctx, const Record *record,
                                           const VEC_EXPR &args, const VEC_STR &yield_items,
                                           std::vector<Record> *records);

    static void DbmsSecuritySetUserDesc(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                        const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsSecurityDeleteUserRoles(RTContext *ctx, const Record *record,
                                            const VEC_EXPR &args, const VEC_STR &yield_items,
                                            std::vector<Record> *records);

    static void DbmsSecurityRebuildUserRoles(RTContext *ctx, const Record *record,
                                             const VEC_EXPR &args, const VEC_STR &yield_items,
                                             std::vector<Record> *records);

    static void DbmsSecurityAddUserRoles(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                         const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbPluginLoadPlugin(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                   const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbPluginDeletePlugin(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                     const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbPluginListPlugin(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                   const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbPluginListUserPlugins(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                        const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbPluginGetPluginInfo(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                      const VEC_STR &yield_items, std::vector<Record> *records);
    static void DbPluginCallPlugin(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                   const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbImportorDataImportor(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                       const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbImportorFullImportor(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                       const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbImportorFullFileImportor(RTContext *ctx, const Record *record,
                                           const VEC_EXPR &args, const VEC_STR &yield_items,
                                           std::vector<Record> *records);

    static void DbImportorSchemaImportor(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                         const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbDeleteIndex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                              const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbDeleteEdgeIndex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                  const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbDeleteCompositeIndex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                              const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbFlushDB(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                          const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbDropDB(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                         const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbDropAllVertex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                         const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbTaskListTasks(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbTaskTerminateTask(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                    const VEC_STR &yield_items, std::vector<Record> *records);
    static void DbListLabelIndexes(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                   const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbMonitorServerInfo(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                    const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbMonitorTuGraphInfo(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                     const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbmsHaClusterInfo(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                  const VEC_STR &yield_items, std::vector<Record> *records);
};

class AlgoFunc {
 public:
    static void ShortestPath(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                             const VEC_STR &yield_items, std::vector<Record> *records);

    static void AllShortestPaths(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                 const VEC_STR &yield_items, std::vector<Record> *records);

    static void NativeExtract(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                              const VEC_STR &yield_items, std::vector<Record> *records);

    static void PageRank(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                         const VEC_STR &yield_items, std::vector<Record> *records);

    static void Jaccard(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                        const VEC_STR &yield_items, std::vector<Record> *records);
};

class SpatialFunc {
 public:
    static void Distance(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                          const VEC_STR &yield_items, std::vector<Record> *records);
};

struct Procedure {
    /* <name, <index, type>> */
    typedef std::vector<std::pair<std::string, std::pair<int, lgraph_api::LGraphType>>> SIG_SPEC;
    typedef SA_FUNC FUNC;
    std::string proc_name;
    lgraph_api::SigSpec signature;
    FUNC function;
    bool read_only;
    bool separate_txn;  // do this procedure in separate txn

    Procedure(std::string name, FUNC func, const SIG_SPEC &args, const SIG_SPEC &results,
              bool ro = true, bool st = false)
        : proc_name(std::move(name)), function(std::move(func)), read_only(ro), separate_txn(st) {
        std::vector<lgraph_api::Parameter> input_list;
        std::transform(args.cbegin(), args.cend(), std::back_inserter(input_list),
                       [](auto arg) -> lgraph_api::Parameter {
                           return {
                               .name = arg.first,
                               .index = arg.second.first,
                               .type = arg.second.second,
                           };
                       });

        std::vector<lgraph_api::Parameter> yield_items;
        std::transform(results.cbegin(), results.cend(), std::back_inserter(yield_items),
                       [](auto result) -> lgraph_api::Parameter {
                           return {
                               .name = result.first,
                               .index = result.second.first,
                               .type = result.second.second,
                           };
                       });

        signature = {
            .input_list = std::move(input_list),
            .result_list = std::move(yield_items),
        };
    }

    Procedure(std::string name, FUNC func, bool ro = true, bool st = false)
        : proc_name(std::move(name)), function(std::move(func)), read_only(ro), separate_txn(st) {
        signature = {
            .input_list = {},
            .result_list = {},
        };
    }

    bool ContainsYieldItem(const std::string &item) {
        for (auto &r : signature.result_list) {
            if (r.name == item) {
                return true;
            }
        }
        return false;
    }

    std::string Signature() const {
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
};

static std::vector<Procedure> global_procedures = {
    Procedure("db.subgraph", BuiltinProcedure::DbSubgraph,
              Procedure::SIG_SPEC{{"vids", {0, lgraph_api::LGraphType::LIST}}},
              Procedure::SIG_SPEC{{"subgraph", {0, lgraph_api::LGraphType::STRING}}}),

    Procedure("db.vertexLabels", BuiltinProcedure::DbVertexLabels, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"label", {0, lgraph_api::LGraphType::STRING}}}),

    Procedure("db.edgeLabels", BuiltinProcedure::DbEdgeLabels, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"label", {0, lgraph_api::LGraphType::STRING}}}),

    Procedure("db.indexes", BuiltinProcedure::DbIndexes, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{
                  {"label", {0, lgraph_api::LGraphType::STRING}},
                  {"field", {1, lgraph_api::LGraphType::STRING}},
                  {"label_type", {2, lgraph_api::LGraphType::STRING}},
                  {"unique", {3, lgraph_api::LGraphType::BOOLEAN}},
                  {"pair_unique", {4, lgraph_api::LGraphType::BOOLEAN}},
              }),

    Procedure("db.listLabelIndexes", BuiltinProcedure::DbListLabelIndexes,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"label_type", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{
                  {"label", {0, lgraph_api::LGraphType::STRING}},
                  {"field", {1, lgraph_api::LGraphType::STRING}},
                  {"unique", {2, lgraph_api::LGraphType::BOOLEAN}},
                  {"pair_unique", {3, lgraph_api::LGraphType::BOOLEAN}},
              }),

    Procedure("db.propertyKeys", BuiltinProcedure::DbPropertyKeys, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"propertyKey", {0, lgraph_api::LGraphType::STRING}}}),

    Procedure("db.warmup", BuiltinProcedure::DbWarmUp, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"time_used", {0, lgraph_api::LGraphType::STRING}}}, true, true),

    Procedure("db.createVertexLabelByJson", BuiltinProcedure::DbCreateVertexLabelByJson,
              Procedure::SIG_SPEC{{"json_data", {0, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.createEdgeLabelByJson", BuiltinProcedure::DbCreateEdgeLabelByJson,
              Procedure::SIG_SPEC{{"json_data", {0, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.createVertexLabel", BuiltinProcedure::DbCreateVertexLabel,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"field_specs", {1, lgraph_api::LGraphType::LIST}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.createLabel", BuiltinProcedure::DbCreateLabel,
              Procedure::SIG_SPEC{{"label_type", {0, lgraph_api::LGraphType::STRING}},
                                  {"label_name", {1, lgraph_api::LGraphType::STRING}},
                                  {"extra", {2, lgraph_api::LGraphType::STRING}},
                                  {"field_specs", {3, lgraph_api::LGraphType::LIST}}},
              Procedure::SIG_SPEC{}, false, true),

    Procedure("db.getLabelSchema", BuiltinProcedure::DbGetLabelSchema,
              Procedure::SIG_SPEC{{"label_type", {0, lgraph_api::LGraphType::STRING}},
                                  {"label_name", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"name", {0, lgraph_api::LGraphType::STRING}},
                                  {"type", {1, lgraph_api::LGraphType::STRING}},
                                  {"optional", {2, lgraph_api::LGraphType::BOOLEAN}}},
              true, false),

    Procedure("db.getVertexSchema", BuiltinProcedure::DbGetVertexSchema,
              Procedure::SIG_SPEC{{"label", {0, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"schema", {0, lgraph_api::LGraphType::MAP}}}, true, false),

    Procedure("db.getEdgeSchema", BuiltinProcedure::DbGetEdgeSchema,
              Procedure::SIG_SPEC{{"label", {0, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"schema", {0, lgraph_api::LGraphType::MAP}}}, true, false),

    Procedure("db.deleteLabel", BuiltinProcedure::DbDeleteLabel,
              Procedure::SIG_SPEC{{"label_type", {0, lgraph_api::LGraphType::STRING}},
                                  {"label_name", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{
                  {"", {0, lgraph_api::LGraphType::NUL}},
              },
              false, true),

    Procedure("db.alterLabelDelFields", BuiltinProcedure::DbAlterLabelDelFields,
              Procedure::SIG_SPEC{{"label_type", {0, lgraph_api::LGraphType::STRING}},
                                  {"label_name", {1, lgraph_api::LGraphType::STRING}},
                                  {"del_fields", {2, lgraph_api::LGraphType::LIST}}},
              Procedure::SIG_SPEC{
                  {"record_affected", {0, lgraph_api::LGraphType::INTEGER}},
              },
              false, true),

    Procedure("db.alterLabelAddFields", BuiltinProcedure::DbAlterLabelAddFields,
              Procedure::SIG_SPEC{
                  {"label_type", {0, lgraph_api::LGraphType::STRING}},
                  {"label_name", {1, lgraph_api::LGraphType::STRING}},
                  {"add_field_spec_values", {2, lgraph_api::LGraphType::LIST}},
              },
              Procedure::SIG_SPEC{
                  {"record_affected", {0, lgraph_api::LGraphType::INTEGER}},
              },
              false, true),

    Procedure("db.upsertVertex", BuiltinProcedure::DbUpsertVertex,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"list_data", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{
                  {"total", {0, lgraph_api::LGraphType::INTEGER}},
                  {"data_error", {1, lgraph_api::LGraphType::INTEGER}},
                  {"index_conflict", {2, lgraph_api::LGraphType::INTEGER}},
                  {"insert", {3, lgraph_api::LGraphType::INTEGER}},
                  {"update", {4, lgraph_api::LGraphType::INTEGER}},
              },
              false, true),

    Procedure("db.upsertVertexByJson", BuiltinProcedure::DbUpsertVertexByJson,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"list_data", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{
                  {"total", {0, lgraph_api::LGraphType::INTEGER}},
                  {"data_error", {1, lgraph_api::LGraphType::INTEGER}},
                  {"index_conflict", {2, lgraph_api::LGraphType::INTEGER}},
                  {"insert", {3, lgraph_api::LGraphType::INTEGER}},
                  {"update", {4, lgraph_api::LGraphType::INTEGER}},
              },
              false, true),

    Procedure("db.upsertEdge", BuiltinProcedure::DbUpsertEdge,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"start_spec", {1, lgraph_api::LGraphType::STRING}},
                                  {"end_spec", {2, lgraph_api::LGraphType::STRING}},
                                  {"list_data", {3, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{
                  {"total", {0, lgraph_api::LGraphType::INTEGER}},
                  {"data_error", {1, lgraph_api::LGraphType::INTEGER}},
                  {"index_conflict", {2, lgraph_api::LGraphType::INTEGER}},
                  {"insert", {3, lgraph_api::LGraphType::INTEGER}},
                  {"update", {4, lgraph_api::LGraphType::INTEGER}},
              },
              false, true),

    Procedure("db.upsertEdgeByJson", BuiltinProcedure::DbUpsertEdgeByJson,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"start_spec", {1, lgraph_api::LGraphType::STRING}},
                                  {"end_spec", {2, lgraph_api::LGraphType::STRING}},
                                  {"list_data", {3, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{
                  {"total", {0, lgraph_api::LGraphType::INTEGER}},
                  {"data_error", {1, lgraph_api::LGraphType::INTEGER}},
                  {"index_conflict", {2, lgraph_api::LGraphType::INTEGER}},
                  {"insert", {3, lgraph_api::LGraphType::INTEGER}},
                  {"update", {4, lgraph_api::LGraphType::INTEGER}},
              },
              false, true),

    Procedure("db.alterLabelModFields", BuiltinProcedure::DbAlterLabelModFields,
              Procedure::SIG_SPEC{{"label_type", {0, lgraph_api::LGraphType::STRING}},
                                  {"label_name", {1, lgraph_api::LGraphType::STRING}},
                                  {"mod_field_specs", {2, lgraph_api::LGraphType::LIST}}},
              Procedure::SIG_SPEC{
                  {"record_affected", {0, lgraph_api::LGraphType::INTEGER}},
              },
              false, true),

    Procedure("db.createEdgeLabel", BuiltinProcedure::DbCreateEdgeLabel,
              Procedure::SIG_SPEC{{"type_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"field_specs", {1, lgraph_api::LGraphType::LIST}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.addIndex", BuiltinProcedure::DbAddVertexIndex,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"field_name", {1, lgraph_api::LGraphType::STRING}},
                                  {"unique", {2, lgraph_api::LGraphType::BOOLEAN}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.addVertexCompositeIndex", BuiltinProcedure::DbAddVertexCompositeIndex,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"field_names", {1, lgraph_api::LGraphType::LIST}},
                                  {"unique", {2, lgraph_api::LGraphType::BOOLEAN}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.addEdgeIndex", BuiltinProcedure::DbAddEdgeIndex,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"field_name", {1, lgraph_api::LGraphType::STRING}},
                                  {"unique", {2, lgraph_api::LGraphType::BOOLEAN}},
                                  {"pair_unique", {2, lgraph_api::LGraphType::BOOLEAN}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.addFullTextIndex", BuiltinProcedure::DbAddFullTextIndex,
              Procedure::SIG_SPEC{{"is_vertex", {0, lgraph_api::LGraphType::BOOLEAN}},
                                  {"label_name", {1, lgraph_api::LGraphType::STRING}},
                                  {"field_name", {2, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.deleteFullTextIndex", BuiltinProcedure::DbDeleteFullTextIndex,
              Procedure::SIG_SPEC{{"is_vertex", {0, lgraph_api::LGraphType::BOOLEAN}},
                                  {"label_name", {1, lgraph_api::LGraphType::STRING}},
                                  {"field_name", {2, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.rebuildFullTextIndex", BuiltinProcedure::DbRebuildFullTextIndex,
              Procedure::SIG_SPEC{{"vertex_labels", {0, lgraph_api::LGraphType::STRING}},
                                  {"edge_labels", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.fullTextIndexes", BuiltinProcedure::DbFullTextIndexes, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"is_vertex", {0, lgraph_api::LGraphType::BOOLEAN}},
                                  {"label", {1, lgraph_api::LGraphType::STRING}},
                                  {"field", {2, lgraph_api::LGraphType::STRING}}}),

    Procedure("db.addEdgeConstraints", BuiltinProcedure::DbAddEdgeConstraints,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"constraints", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.clearEdgeConstraints", BuiltinProcedure::DbClearEdgeConstraints,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.procedures", BuiltinProcedure::DbmsProcedures, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"name", {0, lgraph_api::LGraphType::STRING}},
                                  {"signature", {1, lgraph_api::LGraphType::STRING}},
                                  {"read_only", {2, lgraph_api::LGraphType::BOOLEAN}}},
              true, false),

    Procedure("dbms.meta.countDetail", BuiltinProcedure::DbmsMetaCountDetail, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"is_vertex", {0, lgraph_api::LGraphType::BOOLEAN}},
                                  {"label", {1, lgraph_api::LGraphType::STRING}},
                                  {"count", {2, lgraph_api::LGraphType::INTEGER}}},
              true, false),
    Procedure("dbms.meta.count", BuiltinProcedure::DbmsMetaCount, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"type", {1, lgraph_api::LGraphType::STRING}},
                                  {"number", {2, lgraph_api::LGraphType::INTEGER}}},
              true, false),
    Procedure("dbms.meta.refreshCount", BuiltinProcedure::DbmsMetaRefreshCount,
              Procedure::SIG_SPEC{}, Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}},
              false, true),
    Procedure("dbms.security.isDefaultUserPassword",
              BuiltinProcedure::DbmsSecurityIsDefaultUserPassword,
              Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{
                  {"isDefaultUserPassword", {0, lgraph_api::LGraphType::BOOLEAN}}
              }, true, false),
    Procedure("dbms.security.changePassword", BuiltinProcedure::DbmsSecurityChangePassword,
              Procedure::SIG_SPEC{
                  {"current_password", {0, lgraph_api::LGraphType::STRING}},
                  {"new_password", {1, lgraph_api::LGraphType::STRING}},
              },
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.security.changeUserPassword", BuiltinProcedure::DbmsSecurityChangeUserPassword,
              Procedure::SIG_SPEC{
                  {"user_name", {0, lgraph_api::LGraphType::STRING}},
                  {"new_password", {1, lgraph_api::LGraphType::STRING}},
              },
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.security.createUser", BuiltinProcedure::DbmsSecurityCreateUser,
              Procedure::SIG_SPEC{{"user_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"password", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.security.deleteUser", BuiltinProcedure::DbmsSecurityDeleteUser,
              Procedure::SIG_SPEC{{"user_name", {0, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.security.setUserMemoryLimit", BuiltinProcedure::DbmsSecuritySetUserMemoryLimit,
              Procedure::SIG_SPEC{
                  {"user_name", {0, lgraph_api::LGraphType::STRING}},
                  {"MemoryLimit", {1, lgraph_api::LGraphType::INTEGER}},
              },
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.security.listUsers", BuiltinProcedure::DbmsSecurityListUsers,
              Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{
                  {"user_name", {0, lgraph_api::LGraphType::STRING}},
                  {"user_info", {1, lgraph_api::LGraphType::MAP}},
              },
              true, true),

    Procedure("dbms.security.showCurrentUser", BuiltinProcedure::DbmsSecurityShowCurrentUser,
              Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"current_user", {0, lgraph_api::LGraphType::STRING}}}, true,
              true),

    Procedure("dbms.security.listAllowedHosts", BuiltinProcedure::DbmsSecurityHostWhitelistList,
              Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"host", {0, lgraph_api::LGraphType::STRING}}}, true, true),

    Procedure("dbms.security.deleteAllowedHosts", BuiltinProcedure::DbmsSecurityHostWhitelistDelete,
              Procedure::SIG_SPEC{{"hosts", {0, lgraph_api::LGraphType::LIST}}},
              Procedure::SIG_SPEC{{"record_affected", {0, lgraph_api::LGraphType::INTEGER}}}, false,
              true),

    Procedure("dbms.security.addAllowedHosts", BuiltinProcedure::DbmsSecurityHostWhitelistAdd,
              Procedure::SIG_SPEC{{"hosts", {0, lgraph_api::LGraphType::LIST}}},
              Procedure::SIG_SPEC{{"num_added", {0, lgraph_api::LGraphType::INTEGER}}}, false,
              true),

    Procedure("dbms.graph.createGraph", BuiltinProcedure::DbmsGraphCreateGraph,
              Procedure::SIG_SPEC{{"graph_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"description", {1, lgraph_api::LGraphType::STRING}},
                                  {"max_size_GB", {2, lgraph_api::LGraphType::INTEGER}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.graph.deleteGraph", BuiltinProcedure::DbmsGraphDeleteGraph,
              Procedure::SIG_SPEC{{"graph_name", {0, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.graph.modGraph", BuiltinProcedure::DbmsGraphModGraph,
              Procedure::SIG_SPEC{{"graph_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"config", {1, lgraph_api::LGraphType::MAP}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.graph.listGraphs", BuiltinProcedure::DbmsGraphListGraphs, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"graph_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"configuration", {1, lgraph_api::LGraphType::MAP}}},
              true, true),

    Procedure("dbms.graph.listUserGraphs", BuiltinProcedure::DbmsGraphListUserGraphs,
              Procedure::SIG_SPEC{{"user_name", {0, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"graph_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"configuration", {1, lgraph_api::LGraphType::MAP}}},
              true, true),

    Procedure("dbms.graph.getGraphInfo", BuiltinProcedure::DbmsGraphGetGraphInfo,
              Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"graph_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"configuration", {1, lgraph_api::LGraphType::MAP}}},
              true, true),

    Procedure("dbms.graph.getGraphSchema", BuiltinProcedure::DbmsGraphGetGraphSchema,
              Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"schema", {0, lgraph_api::LGraphType::STRING}}},
              true, true),

    Procedure("dbms.system.info", BuiltinProcedure::DbmsSystemInfo, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"name", {0, lgraph_api::LGraphType::STRING}},
                                  {"value", {1, lgraph_api::LGraphType::ANY}}},
              true, true),

    Procedure("dbms.config.list", BuiltinProcedure::DbmsConfigList, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"name", {0, lgraph_api::LGraphType::STRING}},
                                  {"value", {1, lgraph_api::LGraphType::ANY}}},
              true, true),

    Procedure("dbms.config.update", BuiltinProcedure::DbmsConfigUpdate,
              Procedure::SIG_SPEC{{"updates", {0, lgraph_api::LGraphType::MAP}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.takeSnapshot", BuiltinProcedure::DbmsTakeSnapshot, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"path", {0, lgraph_api::LGraphType::STRING}}}, false),

    Procedure("dbms.listBackupFiles", BuiltinProcedure::DbmsListBackupLogFiles,
              Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"file", {0, lgraph_api::LGraphType::STRING}}}, true),
    // algo
    Procedure("algo.shortestPath", AlgoFunc::ShortestPath,
              Procedure::SIG_SPEC{
                  {"startNode", {0, lgraph_api::LGraphType::NODE}},
                  {"endNode", {1, lgraph_api::LGraphType::NODE}},
                  {"config", {2, lgraph_api::LGraphType::MAP}},
              },
              Procedure::SIG_SPEC{
                  {"nodeCount", {0, lgraph_api::LGraphType::INTEGER}},
                  {"totalCost", {1, lgraph_api::LGraphType::FLOAT}},
                  {"path", {2, lgraph_api::LGraphType::STRING}},
              }),

    Procedure("algo.allShortestPaths", AlgoFunc::AllShortestPaths,
              Procedure::SIG_SPEC{
                  {"startNode", {0, lgraph_api::LGraphType::NODE}},
                  {"endNode", {1, lgraph_api::LGraphType::NODE}},
                  {"config", {2, lgraph_api::LGraphType::MAP}},
              },
              Procedure::SIG_SPEC{
                  {"nodeIds", {0, lgraph_api::LGraphType::LIST}},
                  {"relationshipIds", {1, lgraph_api::LGraphType::LIST}},
                  {"cost", {2, lgraph_api::LGraphType::LIST}},
              }),

    Procedure("algo.native.extract", AlgoFunc::NativeExtract,
              Procedure::SIG_SPEC{
                  {"id", {0, lgraph_api::LGraphType::ANY}},
                  {"config", {1, lgraph_api::LGraphType::MAP}},
              },
              Procedure::SIG_SPEC{
                  {"value", {0, lgraph_api::LGraphType::ANY}},
              }),

    Procedure("algo.pagerank", AlgoFunc::PageRank,
              Procedure::SIG_SPEC{
                  {"num_iterations", {0, lgraph_api::LGraphType::INTEGER}},
              },
              Procedure::SIG_SPEC{{"node", {0, lgraph_api::LGraphType::NODE}},
                                  {"pr", {1, lgraph_api::LGraphType::FLOAT}}}),

    Procedure("algo.jaccard", AlgoFunc::Jaccard,
              Procedure::SIG_SPEC{
                  {"lhs", {0, lgraph_api::LGraphType::ANY}},
                  {"rhs", {0, lgraph_api::LGraphType::ANY}},
              },
              Procedure::SIG_SPEC{
                  {"similarity", {0, lgraph_api::LGraphType::FLOAT}},
              }),
    // spatial
    Procedure("spatial.distance", SpatialFunc::Distance,
              Procedure::SIG_SPEC{
                 {"Spatial1", {0, lgraph_api::LGraphType::STRING}},
                 {"Spatial2", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"distance", {0, lgraph_api::LGraphType::DOUBLE}}}),

    Procedure("dbms.security.listRoles", BuiltinProcedure::DbmsSecurityListRoles,
              Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{
                  {"role_name", {0, lgraph_api::LGraphType::STRING}},
                  {"role_info", {1, lgraph_api::LGraphType::MAP}},
              },
              true, true),

    Procedure("dbms.security.createRole", BuiltinProcedure::DbmsSecurityCreateRole,
              Procedure::SIG_SPEC{{"role_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"desc", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.security.deleteRole", BuiltinProcedure::DbmsSecurityDeleteRole,
              Procedure::SIG_SPEC{{"role_name", {0, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.security.getUserInfo", BuiltinProcedure::DbmsSecurityGetUserInfo,
              Procedure::SIG_SPEC{{"user", {0, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"user_info", {0, lgraph_api::LGraphType::MAP}}}, true, true),

    Procedure("dbms.security.getUserMemoryUsage", BuiltinProcedure::DbmsSecurityGetUserMemoryUsage,
              Procedure::SIG_SPEC{{"user", {0, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"memory_usage", {0, lgraph_api::LGraphType::INTEGER}}}, true,
              true),

    Procedure("dbms.security.getUserPermissions", BuiltinProcedure::DbmsSecurityGetUserPermissions,
              Procedure::SIG_SPEC{{"user", {0, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"user_info", {0, lgraph_api::LGraphType::MAP}}}, true, true),

    Procedure("dbms.security.getRoleInfo", BuiltinProcedure::DbmsSecurityGetRoleInfo,
              Procedure::SIG_SPEC{{"role", {0, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"role_info", {0, lgraph_api::LGraphType::MAP}}}, true, true),

    Procedure("dbms.security.disableRole", BuiltinProcedure::DbmsSecurityDisableRole,
              Procedure::SIG_SPEC{{"role", {0, lgraph_api::LGraphType::STRING}},
                                  {"disable", {1, lgraph_api::LGraphType::BOOLEAN}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.security.modRoleDesc", BuiltinProcedure::DbmsSecurityModRoleDesc,
              Procedure::SIG_SPEC{{"role", {0, lgraph_api::LGraphType::STRING}},
                                  {"description", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.security.rebuildRoleAccessLevel",
              BuiltinProcedure::DbmsSecurityRebuildRoleAccessLevel,
              Procedure::SIG_SPEC{{"role", {0, lgraph_api::LGraphType::STRING}},
                                  {"access_level", {1, lgraph_api::LGraphType::MAP}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.security.modRoleAccessLevel", BuiltinProcedure::DbmsSecurityModRoleAccessLevel,
              Procedure::SIG_SPEC{{"role", {0, lgraph_api::LGraphType::STRING}},
                                  {"access_level", {1, lgraph_api::LGraphType::MAP}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.security.modRoleFieldAccessLevel",
              BuiltinProcedure::DbmsSecurityModRoleFieldAccessLevel,
              Procedure::SIG_SPEC{{"role", {0, lgraph_api::LGraphType::STRING}},
                                  {"graph", {0, lgraph_api::LGraphType::STRING}},
                                  {"label", {0, lgraph_api::LGraphType::STRING}},
                                  {"field", {0, lgraph_api::LGraphType::STRING}},
                                  {"label_type", {0, lgraph_api::LGraphType::STRING}},
                                  {"field_access_level", {0, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.security.disableUser", BuiltinProcedure::DbmsSecurityDisableUser,
              Procedure::SIG_SPEC{{"user", {0, lgraph_api::LGraphType::STRING}},
                                  {"disable", {1, lgraph_api::LGraphType::BOOLEAN}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.security.setCurrentDesc", BuiltinProcedure::DbmsSecuritySetCurrentDesc,
              Procedure::SIG_SPEC{{"description", {0, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.security.setUserDesc", BuiltinProcedure::DbmsSecuritySetUserDesc,
              Procedure::SIG_SPEC{{"user", {0, lgraph_api::LGraphType::STRING}},
                                  {"description", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.security.deleteUserRoles", BuiltinProcedure::DbmsSecurityDeleteUserRoles,
              Procedure::SIG_SPEC{{"user", {0, lgraph_api::LGraphType::STRING}},
                                  {"roles", {1, lgraph_api::LGraphType::LIST}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.security.rebuildUserRoles", BuiltinProcedure::DbmsSecurityRebuildUserRoles,
              Procedure::SIG_SPEC{{"user", {0, lgraph_api::LGraphType::STRING}},
                                  {"roles", {1, lgraph_api::LGraphType::LIST}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.security.addUserRoles", BuiltinProcedure::DbmsSecurityAddUserRoles,
              Procedure::SIG_SPEC{{"user", {0, lgraph_api::LGraphType::STRING}},
                                  {"roles", {1, lgraph_api::LGraphType::LIST}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.plugin.loadPlugin", BuiltinProcedure::DbPluginLoadPlugin,
              Procedure::SIG_SPEC{{"plugin_type", {0, lgraph_api::LGraphType::STRING}},
                                  {"plugin_name", {1, lgraph_api::LGraphType::STRING}},
                                  {"plugin_content", {2, lgraph_api::LGraphType::ANY}},
                                  {"code_type", {3, lgraph_api::LGraphType::STRING}},
                                  {"plugin_description", {4, lgraph_api::LGraphType::STRING}},
                                  {"read_only", {5, lgraph_api::LGraphType::BOOLEAN}},
                                  {"version", {6, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.plugin.deletePlugin", BuiltinProcedure::DbPluginDeletePlugin,
              Procedure::SIG_SPEC{{"plugin_type", {0, lgraph_api::LGraphType::STRING}},
                                  {"plugin_name", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.plugin.getPluginInfo", BuiltinProcedure::DbPluginGetPluginInfo,
              Procedure::SIG_SPEC{{"plugin_type", {0, lgraph_api::LGraphType::STRING}},
                                  {"plugin_name", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"plugin_description", {0, lgraph_api::LGraphType::MAP}}}, true,
              true),

    Procedure("db.plugin.listPlugin", BuiltinProcedure::DbPluginListPlugin,
              Procedure::SIG_SPEC{{"plugin_type", {0, lgraph_api::LGraphType::STRING}},
                                  {"plugin_version", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"plugin_description", {0, lgraph_api::LGraphType::MAP}}}, false,
              true),

    Procedure("db.plugin.listUserPlugins", BuiltinProcedure::DbPluginListUserPlugins,
              Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"graph", {0, lgraph_api::LGraphType::STRING}},
                                  {"plugins", {1, lgraph_api::LGraphType::MAP}}},
              true, true),

    Procedure("db.plugin.callPlugin", BuiltinProcedure::DbPluginCallPlugin,
              Procedure::SIG_SPEC{{"plugin_type", {0, lgraph_api::LGraphType::STRING}},
                                  {"plugin_name", {1, lgraph_api::LGraphType::STRING}},
                                  {"param", {2, lgraph_api::LGraphType::STRING}},
                                  {"timeout", {3, lgraph_api::LGraphType::DOUBLE}},
                                  {"in_process", {4, lgraph_api::LGraphType::BOOLEAN}}},
              Procedure::SIG_SPEC{{"result", {0, lgraph_api::LGraphType::STRING}}}, false, true),

    Procedure("db.importor.dataImportor", BuiltinProcedure::DbImportorDataImportor,
              Procedure::SIG_SPEC{{"description", {0, lgraph_api::LGraphType::STRING}},
                                  {"content", {1, lgraph_api::LGraphType::STRING}},
                                  {"continue_on_error", {2, lgraph_api::LGraphType::BOOLEAN}},
                                  {"thread_nums", {3, lgraph_api::LGraphType::INTEGER}},
                                  {"delimiter", {4, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.importor.fullImportor", BuiltinProcedure::DbImportorFullImportor,
              Procedure::SIG_SPEC{{"conf", {0, lgraph_api::LGraphType::MAP}}},
              Procedure::SIG_SPEC{{"result", {0, lgraph_api::LGraphType::STRING}}}, false, true),

    Procedure("db.importor.fullFileImportor", BuiltinProcedure::DbImportorFullFileImportor,
              Procedure::SIG_SPEC{{"graph_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"path", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.importor.schemaImportor", BuiltinProcedure::DbImportorSchemaImportor,
              Procedure::SIG_SPEC{{"description", {0, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.deleteIndex", BuiltinProcedure::DbDeleteIndex,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"field_name", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.deleteEdgeIndex", BuiltinProcedure::DbDeleteEdgeIndex,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"field_name", {1, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.deleteCompositeIndex", BuiltinProcedure::DbDeleteCompositeIndex,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"field_name", {1, lgraph_api::LGraphType::LIST}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.flushDB", BuiltinProcedure::DbFlushDB, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.dropDB", BuiltinProcedure::DbDropDB, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.dropAllVertex", BuiltinProcedure::DbDropAllVertex, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("dbms.task.listTasks", BuiltinProcedure::DbTaskListTasks, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{
                  {"tasks_info", {1, lgraph_api::LGraphType::MAP}},
              },
              true, true),

    Procedure("dbms.task.terminateTask", BuiltinProcedure::DbTaskTerminateTask,
              Procedure::SIG_SPEC{{"task_id", {0, lgraph_api::LGraphType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    Procedure("db.monitor.tuGraphInfo", BuiltinProcedure::DbMonitorTuGraphInfo,
              Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"request", {0, lgraph_api::LGraphType::STRING}}}, false, true),

    Procedure("db.monitor.serverInfo", BuiltinProcedure::DbMonitorServerInfo, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"cpu", {0, lgraph_api::LGraphType::STRING}},
                                  {"memory", {1, lgraph_api::LGraphType::STRING}},
                                  {"disk_rate", {2, lgraph_api::LGraphType::STRING}},
                                  {"disk_storage", {3, lgraph_api::LGraphType::STRING}}},
              false, true),

    Procedure("dbms.ha.clusterInfo", BuiltinProcedure::DbmsHaClusterInfo, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"cluster_info", {0, lgraph_api::LGraphType::LIST}},
                                  {"is_master", {1, lgraph_api::LGraphType::BOOLEAN}}
              }, true, true)};

class ProcedureTable {
    std::unordered_map<std::string, Procedure> ptable_;

 public:
    ProcedureTable() { RegisterStandaloneFuncs(); }

    void RegisterStandaloneFuncs() {
        for (auto &p : global_procedures) {
            ptable_.emplace(p.proc_name, p);
        }
    }

    Procedure *GetProcedure(const std::string &name) {
        auto it = ptable_.find(name);
        if (it == ptable_.end()) return nullptr;
        return &it->second;
    }
};

static ProcedureTable global_ptable;
}  // namespace cypher
