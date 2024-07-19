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
static const std::map<lgraph_api::LGraphType, std::string> ResultTypeNamesV2 = {
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

static const std::unordered_map<std::string, lgraph::AccessLevel> ValidAccessLevelsV2 = {
    {"NONE", lgraph::AccessLevel::NONE},
    {"READ", lgraph::AccessLevel::READ},
    {"WRITE", lgraph::AccessLevel::WRITE},
    {"FULL", lgraph::AccessLevel::FULL}};

static const std::unordered_map<std::string, lgraph::FieldAccessLevel> ValidFieldAccessLevelsV2 = {
    {"NONE", lgraph::FieldAccessLevel::NONE},
    {"READ", lgraph::FieldAccessLevel::READ},
    {"WRITE", lgraph::FieldAccessLevel::WRITE}};

static const std::unordered_map<std::string, lgraph::PluginManager::PluginType> ValidPluginTypeV2 =
    {{"CPP", lgraph::PluginManager::PluginType::CPP},
     {"PY", lgraph::PluginManager::PluginType::PYTHON}};

static const std::unordered_map<std::string, lgraph::plugin::CodeType> ValidPluginCodeTypeV2 = {
    {"PY", lgraph::plugin::CodeType::PY},
    {"SO", lgraph::plugin::CodeType::SO},
    {"CPP", lgraph::plugin::CodeType::CPP},
    {"ZIP", lgraph::plugin::CodeType::ZIP}};

static const int SPEC_MEMBER_SIZE_V2 = 3;

typedef std::vector<Entry> VEC_EXPR_V2;
typedef std::vector<std::string> VEC_STR_V2;
// TODO(anyone) procedure context
typedef std::function<void(RTContext *, const Record *, const VEC_EXPR_V2 &, const VEC_STR_V2 &,
                           std::vector<Record> *records)>
    SA_FUNC_V2;

class BuiltinProcedureV2 {
    static const std::unordered_map<std::string, lgraph::FieldType> type_map_;

    static void _ExtractFds(const VEC_EXPR_V2 &args, std::string &label, std::string &extra,
                            std::vector<lgraph::FieldSpec> &fds);

    static void _ExtractAccessLevel(const VEC_EXPR_V2 &args, std::string &label,
                                    std::unordered_map<std::string, lgraph::AccessLevel> &leves);

 public:
    static void DbSubgraph(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                           const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbVertexLabels(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                               const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbEdgeLabels(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                             const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbIndexes(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                          const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbPropertyKeys(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                               const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbWarmUp(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                         const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbCreateVertexLabel(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                    const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbGetLabelSchema(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                 const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbGetVertexSchema(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                  const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbGetEdgeSchema(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbCreateLabel(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                              const VEC_STR_V2 &yield_items, std::vector<Record> *records);
    static void DbDeleteLabel(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                              const VEC_STR_V2 &yield_items, std::vector<Record> *records);
    static void DbAlterLabelDelFields(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                      const VEC_STR_V2 &yield_items, std::vector<Record> *records);
    static void DbAlterLabelAddFields(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                      const VEC_STR_V2 &yield_items, std::vector<Record> *records);
    static void DbAlterLabelModFields(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                      const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbCreateEdgeLabel(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                  const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbAddVertexIndex(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                 const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbAddEdgeIndex(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                               const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbAddFullTextIndex(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                   const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbDeleteFullTextIndex(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                      const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbRebuildFullTextIndex(RTContext *ctx, const Record *record,
                                       const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                       std::vector<Record> *records);

    static void DbFullTextIndexes(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                  const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbAddEdgeConstraints(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                     const VEC_STR_V2 &yield_items, std::vector<Record> *records);
    static void DbClearEdgeConstraints(RTContext *ctx, const Record *record,
                                       const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                       std::vector<Record> *records);

    static void DbmsProcedures(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                               const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbmsMetaCountDetail(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                    const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbmsMetaCount(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                              const VEC_STR_V2 &yield_items, std::vector<Record> *records);
    static void DbmsMetaRefreshCount(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                     const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbmsSecurityChangePassword(RTContext *ctx, const Record *record,
                                           const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                           std::vector<Record> *records);

    static void DbmsSecurityChangeUserPassword(RTContext *ctx, const Record *record,
                                               const VEC_EXPR_V2 &args,
                                               const VEC_STR_V2 &yield_items,
                                               std::vector<Record> *records);

    static void DbmsSecuritySetUserMemoryLimit(RTContext *ctx, const Record *record,
                                               const VEC_EXPR_V2 &args,
                                               const VEC_STR_V2 &yield_items,
                                               std::vector<Record> *records);

    static void DbmsSecurityCreateUser(RTContext *ctx, const Record *record,
                                       const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                       std::vector<Record> *records);

    static void DbmsSecurityDeleteUser(RTContext *ctx, const Record *record,
                                       const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                       std::vector<Record> *records);

    static void DbmsSecurityListUsers(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                      const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbmsSecurityShowCurrentUser(RTContext *ctx, const Record *record,
                                            const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                            std::vector<Record> *records);

    static void DbmsSecurityHostWhitelistList(RTContext *ctx, const Record *record,
                                              const VEC_EXPR_V2 &args,
                                              const VEC_STR_V2 &yield_items,
                                              std::vector<Record> *records);

    static void DbmsSecurityHostWhitelistDelete(RTContext *ctx, const Record *record,
                                                const VEC_EXPR_V2 &args,
                                                const VEC_STR_V2 &yield_items,
                                                std::vector<Record> *records);

    static void DbmsSecurityHostWhitelistAdd(RTContext *ctx, const Record *record,
                                             const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                             std::vector<Record> *records);

    static void DbmsGraphCreateGraph(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                     const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbmsGraphModGraph(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                  const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbmsGraphDeleteGraph(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                     const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbmsGraphListGraphs(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                    const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbmsGraphListUserGraphs(RTContext *ctx, const Record *record,
                                        const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                        std::vector<Record> *records);

    static void DbmsGraphGetGraphInfo(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                      const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbmsSystemInfo(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                               const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbmsConfigList(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                               const VEC_STR_V2 &yield_items, std::vector<Record> *records);
    static void DbmsConfigUpdate(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                 const VEC_STR_V2 &yield_items, std::vector<Record> *records);
    // take snapshot
    static void DbmsTakeSnapshot(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                 const VEC_STR_V2 &yield_items, std::vector<Record> *records);
    // list existing backup logs
    static void DbmsListBackupLogFiles(RTContext *ctx, const Record *record,
                                       const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                       std::vector<Record> *records);

    static void DbmsSecurityListRoles(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                      const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbmsSecurityCreateRole(RTContext *ctx, const Record *record,
                                       const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                       std::vector<Record> *records);

    static void DbmsSecurityDeleteRole(RTContext *ctx, const Record *record,
                                       const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                       std::vector<Record> *records);

    static void DbmsSecurityGetUserInfo(RTContext *ctx, const Record *record,
                                        const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                        std::vector<Record> *records);

    static void DbmsSecurityGetUserMemoryUsage(RTContext *ctx, const Record *record,
                                               const VEC_EXPR_V2 &args,
                                               const VEC_STR_V2 &yield_items,
                                               std::vector<Record> *records);

    static void DbmsSecurityGetUserPermissions(RTContext *ctx, const Record *record,
                                               const VEC_EXPR_V2 &args,
                                               const VEC_STR_V2 &yield_items,
                                               std::vector<Record> *records);

    static void DbmsSecurityGetRoleInfo(RTContext *ctx, const Record *record,
                                        const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                        std::vector<Record> *records);

    static void DbmsSecurityDisableRole(RTContext *ctx, const Record *record,
                                        const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                        std::vector<Record> *records);

    static void DbmsSecurityModRoleDesc(RTContext *ctx, const Record *record,
                                        const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                        std::vector<Record> *records);

    static void DbmsSecurityRebuildRoleAccessLevel(RTContext *ctx, const Record *record,
                                                   const VEC_EXPR_V2 &args,
                                                   const VEC_STR_V2 &yield_items,
                                                   std::vector<Record> *records);

    static void DbmsSecurityModRoleAccessLevel(RTContext *ctx, const Record *record,
                                               const VEC_EXPR_V2 &args,
                                               const VEC_STR_V2 &yield_items,
                                               std::vector<Record> *records);

    static void DbmsSecurityModRoleFieldAccessLevel(RTContext *ctx, const Record *record,
                                                    const VEC_EXPR_V2 &args,
                                                    const VEC_STR_V2 &yield_items,
                                                    std::vector<Record> *records);

    static void DbmsSecurityDisableUser(RTContext *ctx, const Record *record,
                                        const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                        std::vector<Record> *records);

    static void DbmsSecuritySetCurrentDesc(RTContext *ctx, const Record *record,
                                           const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                           std::vector<Record> *records);

    static void DbmsSecuritySetUserDesc(RTContext *ctx, const Record *record,
                                        const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                        std::vector<Record> *records);

    static void DbmsSecurityDeleteUserRoles(RTContext *ctx, const Record *record,
                                            const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                            std::vector<Record> *records);

    static void DbmsSecurityRebuildUserRoles(RTContext *ctx, const Record *record,
                                             const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                             std::vector<Record> *records);

    static void DbmsSecurityAddUserRoles(RTContext *ctx, const Record *record,
                                         const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                         std::vector<Record> *records);

    static void DbPluginLoadPlugin(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                   const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbPluginDeletePlugin(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                     const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbPluginListPlugin(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                   const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbPluginListUserPlugins(RTContext *ctx, const Record *record,
                                        const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                        std::vector<Record> *records);

    static void DbPluginGetPluginInfo(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                      const VEC_STR_V2 &yield_items, std::vector<Record> *records);
    static void DbPluginCallPlugin(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                   const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbImportorDataImportor(RTContext *ctx, const Record *record,
                                       const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                       std::vector<Record> *records);

    static void DbImportorFullImportor(RTContext *ctx, const Record *record,
                                       const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                       std::vector<Record> *records);

    static void DbImportorFullFileImportor(RTContext *ctx, const Record *record,
                                           const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                           std::vector<Record> *records);

    static void DbImportorSchemaImportor(RTContext *ctx, const Record *record,
                                         const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                         std::vector<Record> *records);

    static void DbDeleteIndex(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                              const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbDeleteEdgeIndex(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                  const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbFlushDB(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                          const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbDropDB(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                         const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbTaskListTasks(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbTaskTerminateTask(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                    const VEC_STR_V2 &yield_items, std::vector<Record> *records);
    static void DbListLabelIndexes(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                   const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbMonitorServerInfo(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                    const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbMonitorTuGraphInfo(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                     const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbmsHaClusterInfo(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                  const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbCreateEdgeLabelByJson(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                        const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbCreateVertexLabelByJson(RTContext *ctx, const Record *record,
                                          const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                          std::vector<Record> *records);

    static void DbUpsertVertex(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                               const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbUpsertEdge(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                             const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbUpsertVertexByJson(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                     const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbUpsertEdgeByJson(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                   const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbAddVertexCompositeIndex(RTContext *ctx, const Record *record,
                                          const VEC_EXPR_V2 &args, const VEC_STR_V2 &yield_items,
                                          std::vector<Record> *records);

    static void DbDropAllVertex(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void DbDeleteCompositeIndex(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                       const VEC_STR_V2 &yield_items, std::vector<Record> *records);
    
    static void DbmsGraphGetGraphSchema(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                        const VEC_STR_V2 &yield_items, std::vector<Record> *records);
};

class AlgoFuncV2 {
 public:
    static void ShortestPath(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                             const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void AllShortestPaths(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                                 const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void NativeExtract(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                              const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void PageRank(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                         const VEC_STR_V2 &yield_items, std::vector<Record> *records);

    static void Jaccard(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                        const VEC_STR_V2 &yield_items, std::vector<Record> *records);
};

class SpatialFuncV2 {
 public:
    static void Distance(RTContext *ctx, const Record *record, const VEC_EXPR_V2 &args,
                         const VEC_STR_V2 &yield_items, std::vector<Record> *records);
};

struct ProcedureV2 {
    /* <name, <index, type>> */
    typedef std::vector<std::pair<std::string, std::pair<int, lgraph_api::LGraphType>>> SIG_SPEC;
    typedef SA_FUNC_V2 FUNC;
    std::string proc_name;
    lgraph_api::SigSpec signature;
    FUNC function;
    bool read_only;
    bool separate_txn;  // do this procedure in separate txn

    ProcedureV2(std::string name, FUNC func, const SIG_SPEC &args, const SIG_SPEC &results,
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

    ProcedureV2(std::string name, FUNC func, bool ro = true, bool st = false)
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
            str.append(p.name).append("::").append(ResultTypeNamesV2.at(p.type));
            args.emplace(p.index, str);
        }
        for (auto &r : signature.result_list) {
            std::string str;
            str.append(r.name).append("::").append(ResultTypeNamesV2.at(r.type));
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

static std::vector<ProcedureV2> global_procedures_v2 = {
    ProcedureV2("db.subgraph", BuiltinProcedureV2::DbSubgraph,
                ProcedureV2::SIG_SPEC{{"vids", {0, lgraph_api::LGraphType::LIST}}},
                ProcedureV2::SIG_SPEC{{"subgraph", {0, lgraph_api::LGraphType::STRING}}}),

    ProcedureV2("db.vertexLabels", BuiltinProcedureV2::DbVertexLabels, ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"label", {0, lgraph_api::LGraphType::STRING}}}),

    ProcedureV2("db.edgeLabels", BuiltinProcedureV2::DbEdgeLabels, ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"label", {0, lgraph_api::LGraphType::STRING}}}),

    ProcedureV2("db.indexes", BuiltinProcedureV2::DbIndexes, ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{
                    {"label", {0, lgraph_api::LGraphType::STRING}},
                    {"field", {1, lgraph_api::LGraphType::STRING}},
                    {"label_type", {2, lgraph_api::LGraphType::STRING}},
                    {"unique", {3, lgraph_api::LGraphType::BOOLEAN}},
                    {"pair_unique", {4, lgraph_api::LGraphType::BOOLEAN}},
                }),

    ProcedureV2("db.listLabelIndexes", BuiltinProcedureV2::DbListLabelIndexes,
                ProcedureV2::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                      {"label_type", {1, lgraph_api::LGraphType::LIST}}},
                ProcedureV2::SIG_SPEC{
                    {"label", {0, lgraph_api::LGraphType::STRING}},
                    {"field", {1, lgraph_api::LGraphType::STRING}},
                    {"unique", {2, lgraph_api::LGraphType::BOOLEAN}},
                    {"pair_unique", {3, lgraph_api::LGraphType::BOOLEAN}},
                }),

    ProcedureV2("db.propertyKeys", BuiltinProcedureV2::DbPropertyKeys, ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"propertyKey", {0, lgraph_api::LGraphType::STRING}}}),

    ProcedureV2("db.warmup", BuiltinProcedureV2::DbWarmUp, ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"time_used", {0, lgraph_api::LGraphType::STRING}}}, true,
                true),

    ProcedureV2("db.createVertexLabel", BuiltinProcedureV2::DbCreateVertexLabel,
                ProcedureV2::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                      {"field_specs", {1, lgraph_api::LGraphType::LIST}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.createLabel", BuiltinProcedureV2::DbCreateLabel,
                ProcedureV2::SIG_SPEC{{"label_type", {0, lgraph_api::LGraphType::STRING}},
                                      {"label_name", {1, lgraph_api::LGraphType::STRING}},
                                      {"extra", {2, lgraph_api::LGraphType::STRING}},
                                      {"field_specs", {3, lgraph_api::LGraphType::LIST}}},
                ProcedureV2::SIG_SPEC{}, false, true),

    ProcedureV2("db.getLabelSchema", BuiltinProcedureV2::DbGetLabelSchema,
                ProcedureV2::SIG_SPEC{{"label_type", {0, lgraph_api::LGraphType::STRING}},
                                      {"label_name", {1, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"name", {0, lgraph_api::LGraphType::STRING}},
                                      {"type", {1, lgraph_api::LGraphType::STRING}},
                                      {"optional", {2, lgraph_api::LGraphType::BOOLEAN}}},
                true, false),

    ProcedureV2("db.getVertexSchema", BuiltinProcedureV2::DbGetVertexSchema,
                ProcedureV2::SIG_SPEC{{"label", {0, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"schema", {0, lgraph_api::LGraphType::MAP}}}, true, false),

    ProcedureV2("db.getEdgeSchema", BuiltinProcedureV2::DbGetEdgeSchema,
                ProcedureV2::SIG_SPEC{{"label", {0, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"schema", {0, lgraph_api::LGraphType::MAP}}}, true, false),

    ProcedureV2("db.deleteLabel", BuiltinProcedureV2::DbDeleteLabel,
                ProcedureV2::SIG_SPEC{{"label_type", {0, lgraph_api::LGraphType::STRING}},
                                      {"label_name", {1, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{
                    {"", {0, lgraph_api::LGraphType::NUL}},
                },
                false, true),

    ProcedureV2("db.alterLabelDelFields", BuiltinProcedureV2::DbAlterLabelDelFields,
                ProcedureV2::SIG_SPEC{{"label_type", {0, lgraph_api::LGraphType::STRING}},
                                      {"label_name", {1, lgraph_api::LGraphType::STRING}},
                                      {"del_fields", {2, lgraph_api::LGraphType::LIST}}},
                ProcedureV2::SIG_SPEC{
                    {"record_affected", {0, lgraph_api::LGraphType::INTEGER}},
                },
                false, true),

    ProcedureV2("db.alterLabelAddFields", BuiltinProcedureV2::DbAlterLabelAddFields,
                ProcedureV2::SIG_SPEC{
                    {"label_type", {0, lgraph_api::LGraphType::STRING}},
                    {"label_name", {1, lgraph_api::LGraphType::STRING}},
                    {"add_field_spec_values", {2, lgraph_api::LGraphType::LIST}},
                },
                ProcedureV2::SIG_SPEC{
                    {"record_affected", {0, lgraph_api::LGraphType::INTEGER}},
                },
                false, true),

    ProcedureV2("db.alterLabelModFields", BuiltinProcedureV2::DbAlterLabelModFields,
                ProcedureV2::SIG_SPEC{{"label_type", {0, lgraph_api::LGraphType::STRING}},
                                      {"label_name", {1, lgraph_api::LGraphType::STRING}},
                                      {"mod_field_specs", {2, lgraph_api::LGraphType::LIST}}},
                ProcedureV2::SIG_SPEC{
                    {"record_affected", {0, lgraph_api::LGraphType::INTEGER}},
                },
                false, true),

    ProcedureV2("db.createEdgeLabel", BuiltinProcedureV2::DbCreateEdgeLabel,
                ProcedureV2::SIG_SPEC{{"type_name", {0, lgraph_api::LGraphType::STRING}},
                                      {"field_specs", {1, lgraph_api::LGraphType::LIST}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.addIndex", BuiltinProcedureV2::DbAddVertexIndex,
                ProcedureV2::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                      {"field_name", {1, lgraph_api::LGraphType::STRING}},
                                      {"unique", {2, lgraph_api::LGraphType::BOOLEAN}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.addEdgeIndex", BuiltinProcedureV2::DbAddEdgeIndex,
                ProcedureV2::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                      {"field_name", {1, lgraph_api::LGraphType::STRING}},
                                      {"unique", {2, lgraph_api::LGraphType::BOOLEAN}},
                                      {"pair_unique", {2, lgraph_api::LGraphType::BOOLEAN}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.addFullTextIndex", BuiltinProcedureV2::DbAddFullTextIndex,
                ProcedureV2::SIG_SPEC{{"is_vertex", {0, lgraph_api::LGraphType::BOOLEAN}},
                                      {"label_name", {1, lgraph_api::LGraphType::STRING}},
                                      {"field_name", {2, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.deleteFullTextIndex", BuiltinProcedureV2::DbDeleteFullTextIndex,
                ProcedureV2::SIG_SPEC{{"is_vertex", {0, lgraph_api::LGraphType::BOOLEAN}},
                                      {"label_name", {1, lgraph_api::LGraphType::STRING}},
                                      {"field_name", {2, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.rebuildFullTextIndex", BuiltinProcedureV2::DbRebuildFullTextIndex,
                ProcedureV2::SIG_SPEC{{"vertex_labels", {0, lgraph_api::LGraphType::STRING}},
                                      {"edge_labels", {1, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.fullTextIndexes", BuiltinProcedureV2::DbFullTextIndexes,
                ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"is_vertex", {0, lgraph_api::LGraphType::BOOLEAN}},
                                      {"label", {1, lgraph_api::LGraphType::STRING}},
                                      {"field", {2, lgraph_api::LGraphType::STRING}}}),

    ProcedureV2("db.addEdgeConstraints", BuiltinProcedureV2::DbAddEdgeConstraints,
                ProcedureV2::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                      {"constraints", {1, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.clearEdgeConstraints", BuiltinProcedureV2::DbClearEdgeConstraints,
                ProcedureV2::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.procedures", BuiltinProcedureV2::DbmsProcedures, ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"name", {0, lgraph_api::LGraphType::STRING}},
                                      {"signature", {1, lgraph_api::LGraphType::STRING}},
                                      {"read_only", {2, lgraph_api::LGraphType::BOOLEAN}}},
                true, false),

    ProcedureV2("dbms.meta.countDetail", BuiltinProcedureV2::DbmsMetaCountDetail,
                ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"is_vertex", {0, lgraph_api::LGraphType::BOOLEAN}},
                                      {"label", {1, lgraph_api::LGraphType::STRING}},
                                      {"count", {2, lgraph_api::LGraphType::INTEGER}}},
                true, false),
    ProcedureV2("dbms.meta.count", BuiltinProcedureV2::DbmsMetaCount, ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"type", {1, lgraph_api::LGraphType::STRING}},
                                      {"number", {2, lgraph_api::LGraphType::INTEGER}}},
                true, false),
    ProcedureV2("dbms.meta.refreshCount", BuiltinProcedureV2::DbmsMetaRefreshCount,
                ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),
    ProcedureV2("dbms.security.changePassword", BuiltinProcedureV2::DbmsSecurityChangePassword,
                ProcedureV2::SIG_SPEC{
                    {"current_password", {0, lgraph_api::LGraphType::STRING}},
                    {"new_password", {1, lgraph_api::LGraphType::STRING}},
                },
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.security.changeUserPassword",
                BuiltinProcedureV2::DbmsSecurityChangeUserPassword,
                ProcedureV2::SIG_SPEC{
                    {"user_name", {0, lgraph_api::LGraphType::STRING}},
                    {"new_password", {1, lgraph_api::LGraphType::STRING}},
                },
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.security.createUser", BuiltinProcedureV2::DbmsSecurityCreateUser,
                ProcedureV2::SIG_SPEC{{"user_name", {0, lgraph_api::LGraphType::STRING}},
                                      {"password", {1, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.security.deleteUser", BuiltinProcedureV2::DbmsSecurityDeleteUser,
                ProcedureV2::SIG_SPEC{{"user_name", {0, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.security.setUserMemoryLimit",
                BuiltinProcedureV2::DbmsSecuritySetUserMemoryLimit,
                ProcedureV2::SIG_SPEC{
                    {"user_name", {0, lgraph_api::LGraphType::STRING}},
                    {"MemoryLimit", {1, lgraph_api::LGraphType::INTEGER}},
                },
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.security.listUsers", BuiltinProcedureV2::DbmsSecurityListUsers,
                ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{
                    {"user_name", {0, lgraph_api::LGraphType::STRING}},
                    {"user_info", {1, lgraph_api::LGraphType::MAP}},
                },
                true, true),

    ProcedureV2("dbms.security.showCurrentUser", BuiltinProcedureV2::DbmsSecurityShowCurrentUser,
                ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"current_user", {0, lgraph_api::LGraphType::STRING}}}, true,
                true),

    ProcedureV2("dbms.security.listAllowedHosts", BuiltinProcedureV2::DbmsSecurityHostWhitelistList,
                ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"host", {0, lgraph_api::LGraphType::STRING}}}, true, true),

    ProcedureV2("dbms.security.deleteAllowedHosts",
                BuiltinProcedureV2::DbmsSecurityHostWhitelistDelete,
                ProcedureV2::SIG_SPEC{{"hosts", {0, lgraph_api::LGraphType::LIST}}},
                ProcedureV2::SIG_SPEC{{"record_affected", {0, lgraph_api::LGraphType::INTEGER}}},
                false, true),

    ProcedureV2("dbms.security.addAllowedHosts", BuiltinProcedureV2::DbmsSecurityHostWhitelistAdd,
                ProcedureV2::SIG_SPEC{{"hosts", {0, lgraph_api::LGraphType::LIST}}},
                ProcedureV2::SIG_SPEC{{"num_added", {0, lgraph_api::LGraphType::INTEGER}}}, false,
                true),

    ProcedureV2("dbms.graph.createGraph", BuiltinProcedureV2::DbmsGraphCreateGraph,
                ProcedureV2::SIG_SPEC{{"graph_name", {0, lgraph_api::LGraphType::STRING}},
                                      {"description", {1, lgraph_api::LGraphType::STRING}},
                                      {"max_size_GB", {2, lgraph_api::LGraphType::INTEGER}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.graph.deleteGraph", BuiltinProcedureV2::DbmsGraphDeleteGraph,
                ProcedureV2::SIG_SPEC{{"graph_name", {0, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.graph.modGraph", BuiltinProcedureV2::DbmsGraphModGraph,
                ProcedureV2::SIG_SPEC{{"graph_name", {0, lgraph_api::LGraphType::STRING}},
                                      {"config", {1, lgraph_api::LGraphType::MAP}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.graph.listGraphs", BuiltinProcedureV2::DbmsGraphListGraphs,
                ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"graph_name", {0, lgraph_api::LGraphType::STRING}},
                                      {"configuration", {1, lgraph_api::LGraphType::MAP}}},
                true, true),

    ProcedureV2("dbms.graph.listUserGraphs", BuiltinProcedureV2::DbmsGraphListUserGraphs,
                ProcedureV2::SIG_SPEC{{"user_name", {0, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"graph_name", {0, lgraph_api::LGraphType::STRING}},
                                      {"configuration", {1, lgraph_api::LGraphType::MAP}}},
                true, true),

    ProcedureV2("dbms.graph.getGraphInfo", BuiltinProcedureV2::DbmsGraphGetGraphInfo,
                ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"graph_name", {0, lgraph_api::LGraphType::STRING}},
                                      {"configuration", {1, lgraph_api::LGraphType::MAP}}},
                true, true),

    ProcedureV2("dbms.system.info", BuiltinProcedureV2::DbmsSystemInfo, ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"name", {0, lgraph_api::LGraphType::STRING}},
                                      {"value", {1, lgraph_api::LGraphType::ANY}}},
                true, true),

    ProcedureV2("dbms.config.list", BuiltinProcedureV2::DbmsConfigList, ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"name", {0, lgraph_api::LGraphType::STRING}},
                                      {"value", {1, lgraph_api::LGraphType::ANY}}},
                true, true),

    ProcedureV2("dbms.config.update", BuiltinProcedureV2::DbmsConfigUpdate,
                ProcedureV2::SIG_SPEC{{"updates", {0, lgraph_api::LGraphType::MAP}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.takeSnapshot", BuiltinProcedureV2::DbmsTakeSnapshot, ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"path", {0, lgraph_api::LGraphType::STRING}}}, false),

    ProcedureV2("dbms.listBackupFiles", BuiltinProcedureV2::DbmsListBackupLogFiles,
                ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"file", {0, lgraph_api::LGraphType::STRING}}}, true),
    // algo
    ProcedureV2("algo.shortestPath", AlgoFuncV2::ShortestPath,
                ProcedureV2::SIG_SPEC{
                    {"startNode", {0, lgraph_api::LGraphType::NODE}},
                    {"endNode", {1, lgraph_api::LGraphType::NODE}},
                    {"config", {2, lgraph_api::LGraphType::MAP}},
                },
                ProcedureV2::SIG_SPEC{
                    {"nodeCount", {0, lgraph_api::LGraphType::INTEGER}},
                    {"totalCost", {1, lgraph_api::LGraphType::FLOAT}},
                    {"path", {2, lgraph_api::LGraphType::STRING}},
                }),

    ProcedureV2("algo.allShortestPaths", AlgoFuncV2::AllShortestPaths,
                ProcedureV2::SIG_SPEC{
                    {"startNode", {0, lgraph_api::LGraphType::NODE}},
                    {"endNode", {1, lgraph_api::LGraphType::NODE}},
                    {"config", {2, lgraph_api::LGraphType::MAP}},
                },
                ProcedureV2::SIG_SPEC{
                    {"nodeIds", {0, lgraph_api::LGraphType::LIST}},
                    {"relationshipIds", {1, lgraph_api::LGraphType::LIST}},
                    {"cost", {2, lgraph_api::LGraphType::LIST}},
                }),

    ProcedureV2("algo.native.extract", AlgoFuncV2::NativeExtract,
                ProcedureV2::SIG_SPEC{
                    {"id", {0, lgraph_api::LGraphType::ANY}},
                    {"config", {1, lgraph_api::LGraphType::MAP}},
                },
                ProcedureV2::SIG_SPEC{
                    {"value", {0, lgraph_api::LGraphType::ANY}},
                }),

    ProcedureV2("algo.pagerank", AlgoFuncV2::PageRank,
                ProcedureV2::SIG_SPEC{
                    {"num_iterations", {0, lgraph_api::LGraphType::INTEGER}},
                },
                ProcedureV2::SIG_SPEC{{"node", {0, lgraph_api::LGraphType::NODE}},
                                      {"pr", {1, lgraph_api::LGraphType::FLOAT}}}),

    ProcedureV2("algo.jaccard", AlgoFuncV2::Jaccard,
                ProcedureV2::SIG_SPEC{
                    {"lhs", {0, lgraph_api::LGraphType::ANY}},
                    {"rhs", {0, lgraph_api::LGraphType::ANY}},
                },
                ProcedureV2::SIG_SPEC{
                    {"similarity", {0, lgraph_api::LGraphType::FLOAT}},
                }),

    // spatial
    ProcedureV2("spatial.distance", SpatialFuncV2::Distance,
                ProcedureV2::SIG_SPEC{
                  {"Spatial1", {0, lgraph_api::LGraphType::STRING}},
                  {"Spatial2", {1, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"distance", {0, lgraph_api::LGraphType::DOUBLE}}}),

    ProcedureV2("dbms.security.listRoles", BuiltinProcedureV2::DbmsSecurityListRoles,
                ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{
                    {"role_name", {0, lgraph_api::LGraphType::STRING}},
                    {"role_info", {1, lgraph_api::LGraphType::MAP}},
                },
                true, true),

    ProcedureV2("dbms.security.createRole", BuiltinProcedureV2::DbmsSecurityCreateRole,
                ProcedureV2::SIG_SPEC{{"role_name", {0, lgraph_api::LGraphType::STRING}},
                                      {"desc", {1, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.security.deleteRole", BuiltinProcedureV2::DbmsSecurityDeleteRole,
                ProcedureV2::SIG_SPEC{{"role_name", {0, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.security.getUserInfo", BuiltinProcedureV2::DbmsSecurityGetUserInfo,
                ProcedureV2::SIG_SPEC{{"user", {0, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"user_info", {0, lgraph_api::LGraphType::MAP}}}, true, true),

    ProcedureV2(
        "dbms.security.getUserMemoryUsage", BuiltinProcedureV2::DbmsSecurityGetUserMemoryUsage,
        ProcedureV2::SIG_SPEC{{"user", {0, lgraph_api::LGraphType::STRING}}},
        ProcedureV2::SIG_SPEC{{"memory_usage", {0, lgraph_api::LGraphType::INTEGER}}}, true, true),

    ProcedureV2("dbms.security.getUserPermissions",
                BuiltinProcedureV2::DbmsSecurityGetUserPermissions,
                ProcedureV2::SIG_SPEC{{"user", {0, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"user_info", {0, lgraph_api::LGraphType::MAP}}}, true, true),

    ProcedureV2("dbms.security.getRoleInfo", BuiltinProcedureV2::DbmsSecurityGetRoleInfo,
                ProcedureV2::SIG_SPEC{{"role", {0, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"role_info", {0, lgraph_api::LGraphType::MAP}}}, true, true),

    ProcedureV2("dbms.security.disableRole", BuiltinProcedureV2::DbmsSecurityDisableRole,
                ProcedureV2::SIG_SPEC{{"role", {0, lgraph_api::LGraphType::STRING}},
                                      {"disable", {1, lgraph_api::LGraphType::BOOLEAN}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.security.modRoleDesc", BuiltinProcedureV2::DbmsSecurityModRoleDesc,
                ProcedureV2::SIG_SPEC{{"role", {0, lgraph_api::LGraphType::STRING}},
                                      {"description", {1, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.security.rebuildRoleAccessLevel",
                BuiltinProcedureV2::DbmsSecurityRebuildRoleAccessLevel,
                ProcedureV2::SIG_SPEC{{"role", {0, lgraph_api::LGraphType::STRING}},
                                      {"access_level", {1, lgraph_api::LGraphType::MAP}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.security.modRoleAccessLevel",
                BuiltinProcedureV2::DbmsSecurityModRoleAccessLevel,
                ProcedureV2::SIG_SPEC{{"role", {0, lgraph_api::LGraphType::STRING}},
                                      {"access_level", {1, lgraph_api::LGraphType::MAP}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.security.modRoleFieldAccessLevel",
                BuiltinProcedureV2::DbmsSecurityModRoleFieldAccessLevel,
                ProcedureV2::SIG_SPEC{{"role", {0, lgraph_api::LGraphType::STRING}},
                                      {"graph", {0, lgraph_api::LGraphType::STRING}},
                                      {"label", {0, lgraph_api::LGraphType::STRING}},
                                      {"field", {0, lgraph_api::LGraphType::STRING}},
                                      {"label_type", {0, lgraph_api::LGraphType::STRING}},
                                      {"field_access_level", {0, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.security.disableUser", BuiltinProcedureV2::DbmsSecurityDisableUser,
                ProcedureV2::SIG_SPEC{{"user", {0, lgraph_api::LGraphType::STRING}},
                                      {"disable", {1, lgraph_api::LGraphType::BOOLEAN}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.security.setCurrentDesc", BuiltinProcedureV2::DbmsSecuritySetCurrentDesc,
                ProcedureV2::SIG_SPEC{{"description", {0, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.security.setUserDesc", BuiltinProcedureV2::DbmsSecuritySetUserDesc,
                ProcedureV2::SIG_SPEC{{"user", {0, lgraph_api::LGraphType::STRING}},
                                      {"description", {1, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.security.deleteUserRoles", BuiltinProcedureV2::DbmsSecurityDeleteUserRoles,
                ProcedureV2::SIG_SPEC{{"user", {0, lgraph_api::LGraphType::STRING}},
                                      {"roles", {1, lgraph_api::LGraphType::LIST}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.security.rebuildUserRoles", BuiltinProcedureV2::DbmsSecurityRebuildUserRoles,
                ProcedureV2::SIG_SPEC{{"user", {0, lgraph_api::LGraphType::STRING}},
                                      {"roles", {1, lgraph_api::LGraphType::LIST}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.security.addUserRoles", BuiltinProcedureV2::DbmsSecurityAddUserRoles,
                ProcedureV2::SIG_SPEC{{"user", {0, lgraph_api::LGraphType::STRING}},
                                      {"roles", {1, lgraph_api::LGraphType::LIST}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.plugin.loadPlugin", BuiltinProcedureV2::DbPluginLoadPlugin,
                ProcedureV2::SIG_SPEC{{"plugin_type", {0, lgraph_api::LGraphType::STRING}},
                                      {"plugin_name", {1, lgraph_api::LGraphType::STRING}},
                                      {"plugin_content", {2, lgraph_api::LGraphType::ANY}},
                                      {"code_type", {3, lgraph_api::LGraphType::STRING}},
                                      {"plugin_description", {4, lgraph_api::LGraphType::STRING}},
                                      {"read_only", {5, lgraph_api::LGraphType::BOOLEAN}},
                                      {"version", {6, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.plugin.deletePlugin", BuiltinProcedureV2::DbPluginDeletePlugin,
                ProcedureV2::SIG_SPEC{{"plugin_type", {0, lgraph_api::LGraphType::STRING}},
                                      {"plugin_name", {1, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.plugin.getPluginInfo", BuiltinProcedureV2::DbPluginGetPluginInfo,
                ProcedureV2::SIG_SPEC{{"plugin_type", {0, lgraph_api::LGraphType::STRING}},
                                      {"plugin_name", {1, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"plugin_description", {0, lgraph_api::LGraphType::MAP}}},
                true, true),

    ProcedureV2("db.plugin.listPlugin", BuiltinProcedureV2::DbPluginListPlugin,
                ProcedureV2::SIG_SPEC{{"plugin_type", {0, lgraph_api::LGraphType::STRING}},
                                      {"plugin_version", {1, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"plugin_description", {0, lgraph_api::LGraphType::MAP}}},
                false, true),

    ProcedureV2("db.plugin.listUserPlugins", BuiltinProcedureV2::DbPluginListUserPlugins,
                ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"graph", {0, lgraph_api::LGraphType::STRING}},
                                      {"plugins", {1, lgraph_api::LGraphType::MAP}}},
                true, true),

    ProcedureV2("db.plugin.callPlugin", BuiltinProcedureV2::DbPluginCallPlugin,
                ProcedureV2::SIG_SPEC{{"plugin_type", {0, lgraph_api::LGraphType::STRING}},
                                      {"plugin_name", {1, lgraph_api::LGraphType::STRING}},
                                      {"param", {2, lgraph_api::LGraphType::STRING}},
                                      {"timeout", {3, lgraph_api::LGraphType::DOUBLE}},
                                      {"in_process", {4, lgraph_api::LGraphType::BOOLEAN}}},
                ProcedureV2::SIG_SPEC{{"result", {0, lgraph_api::LGraphType::STRING}}}, false,
                true),

    ProcedureV2("db.importor.dataImportor", BuiltinProcedureV2::DbImportorDataImportor,
                ProcedureV2::SIG_SPEC{{"description", {0, lgraph_api::LGraphType::STRING}},
                                      {"content", {1, lgraph_api::LGraphType::STRING}},
                                      {"continue_on_error", {2, lgraph_api::LGraphType::BOOLEAN}},
                                      {"thread_nums", {3, lgraph_api::LGraphType::INTEGER}},
                                      {"delimiter", {4, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.importor.fullImportor", BuiltinProcedureV2::DbImportorFullImportor,
                ProcedureV2::SIG_SPEC{{"conf", {0, lgraph_api::LGraphType::MAP}}},
                ProcedureV2::SIG_SPEC{{"result", {0, lgraph_api::LGraphType::STRING}}}, false,
                true),

    ProcedureV2("db.importor.fullFileImportor", BuiltinProcedureV2::DbImportorFullFileImportor,
                ProcedureV2::SIG_SPEC{{"graph_name", {0, lgraph_api::LGraphType::STRING}},
                                      {"path", {1, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.importor.schemaImportor", BuiltinProcedureV2::DbImportorSchemaImportor,
                ProcedureV2::SIG_SPEC{{"description", {0, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.deleteIndex", BuiltinProcedureV2::DbDeleteIndex,
                ProcedureV2::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                      {"field_name", {1, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.deleteEdgeIndex", BuiltinProcedureV2::DbDeleteEdgeIndex,
                ProcedureV2::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                      {"field_name", {1, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.flushDB", BuiltinProcedureV2::DbFlushDB, ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.dropDB", BuiltinProcedureV2::DbDropDB, ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.task.listTasks", BuiltinProcedureV2::DbTaskListTasks, ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{
                    {"tasks_info", {1, lgraph_api::LGraphType::MAP}},
                },
                true, true),

    ProcedureV2("dbms.task.terminateTask", BuiltinProcedureV2::DbTaskTerminateTask,
                ProcedureV2::SIG_SPEC{{"task_id", {0, lgraph_api::LGraphType::STRING}}},
                ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2(
        "db.monitor.tuGraphInfo", BuiltinProcedureV2::DbMonitorTuGraphInfo, ProcedureV2::SIG_SPEC{},
        ProcedureV2::SIG_SPEC{{"request", {0, lgraph_api::LGraphType::STRING}}}, false, true),

    ProcedureV2("db.monitor.serverInfo", BuiltinProcedureV2::DbMonitorServerInfo,
                ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"cpu", {0, lgraph_api::LGraphType::STRING}},
                                      {"memory", {1, lgraph_api::LGraphType::STRING}},
                                      {"disk_rate", {2, lgraph_api::LGraphType::STRING}},
                                      {"disk_storage", {3, lgraph_api::LGraphType::STRING}}},
                false, true),

    ProcedureV2("dbms.ha.clusterInfo", BuiltinProcedureV2::DbmsHaClusterInfo,
                ProcedureV2::SIG_SPEC{},
                ProcedureV2::SIG_SPEC{{"cluster_info", {0, lgraph_api::LGraphType::LIST}},
                                      {"is_master", {1, lgraph_api::LGraphType::BOOLEAN}}},
                true, true),

    ProcedureV2("db.createEdgeLabelByJson", BuiltinProcedureV2::DbCreateEdgeLabelByJson,
              ProcedureV2::SIG_SPEC{{"json_data", {0, lgraph_api::LGraphType::STRING}}},
              ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.createVertexLabelByJson", BuiltinProcedureV2::DbCreateVertexLabelByJson,
              ProcedureV2::SIG_SPEC{{"json_data", {0, lgraph_api::LGraphType::STRING}}},
              ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.upsertVertex", BuiltinProcedureV2::DbUpsertVertex,
              ProcedureV2::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"list_data", {1, lgraph_api::LGraphType::STRING}}},
              ProcedureV2::SIG_SPEC{
                  {"total", {0, lgraph_api::LGraphType::INTEGER}},
                  {"data_error", {1, lgraph_api::LGraphType::INTEGER}},
                  {"index_conflict", {2, lgraph_api::LGraphType::INTEGER}},
                  {"insert", {3, lgraph_api::LGraphType::INTEGER}},
                  {"update", {4, lgraph_api::LGraphType::INTEGER}},
              },
              false, true),

    ProcedureV2("db.upsertEdgeByJson", BuiltinProcedureV2::DbUpsertEdgeByJson,
              ProcedureV2::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"start_spec", {1, lgraph_api::LGraphType::STRING}},
                                  {"end_spec", {2, lgraph_api::LGraphType::STRING}},
                                  {"list_data", {3, lgraph_api::LGraphType::STRING}}},
              ProcedureV2::SIG_SPEC{
                  {"total", {0, lgraph_api::LGraphType::INTEGER}},
                  {"data_error", {1, lgraph_api::LGraphType::INTEGER}},
                  {"index_conflict", {2, lgraph_api::LGraphType::INTEGER}},
                  {"insert", {3, lgraph_api::LGraphType::INTEGER}},
                  {"update", {4, lgraph_api::LGraphType::INTEGER}},
              },
              false, true),

    ProcedureV2("db.upsertVertexByJson", BuiltinProcedureV2::DbUpsertVertexByJson,
              ProcedureV2::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"list_data", {1, lgraph_api::LGraphType::STRING}}},
              ProcedureV2::SIG_SPEC{
                  {"total", {0, lgraph_api::LGraphType::INTEGER}},
                  {"data_error", {1, lgraph_api::LGraphType::INTEGER}},
                  {"index_conflict", {2, lgraph_api::LGraphType::INTEGER}},
                  {"insert", {3, lgraph_api::LGraphType::INTEGER}},
                  {"update", {4, lgraph_api::LGraphType::INTEGER}},
              },
              false, true),

    ProcedureV2("db.addVertexCompositeIndex", BuiltinProcedureV2::DbAddVertexCompositeIndex,
              ProcedureV2::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"field_names", {1, lgraph_api::LGraphType::LIST}},
                                  {"unique", {2, lgraph_api::LGraphType::BOOLEAN}}},
              ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.dropAllVertex", BuiltinProcedureV2::DbDropAllVertex, ProcedureV2::SIG_SPEC{},
              ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("db.deleteCompositeIndex", BuiltinProcedureV2::DbDeleteCompositeIndex,
              ProcedureV2::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"field_name", {1, lgraph_api::LGraphType::LIST}}},
              ProcedureV2::SIG_SPEC{{"", {0, lgraph_api::LGraphType::NUL}}}, false, true),

    ProcedureV2("dbms.graph.getGraphSchema", BuiltinProcedureV2::DbmsGraphGetGraphSchema,
              ProcedureV2::SIG_SPEC{},
              ProcedureV2::SIG_SPEC{{"schema", {0, lgraph_api::LGraphType::STRING}}},
              true, true),

    ProcedureV2("db.upsertEdge", BuiltinProcedureV2::DbUpsertEdge,
              ProcedureV2::SIG_SPEC{{"label_name", {0, lgraph_api::LGraphType::STRING}},
                                  {"start_spec", {1, lgraph_api::LGraphType::STRING}},
                                  {"end_spec", {2, lgraph_api::LGraphType::STRING}},
                                  {"list_data", {3, lgraph_api::LGraphType::STRING}}},
              ProcedureV2::SIG_SPEC{
                  {"total", {0, lgraph_api::LGraphType::INTEGER}},
                  {"data_error", {1, lgraph_api::LGraphType::INTEGER}},
                  {"index_conflict", {2, lgraph_api::LGraphType::INTEGER}},
                  {"insert", {3, lgraph_api::LGraphType::INTEGER}},
                  {"update", {4, lgraph_api::LGraphType::INTEGER}},
              },
              false, true)};

class ProcedureTableV2 {
    std::unordered_map<std::string, ProcedureV2> ptable_;

 public:
    ProcedureTableV2() { RegisterStandaloneFuncs(); }

    void RegisterStandaloneFuncs() {
        for (auto &p : global_procedures_v2) {
            ptable_.emplace(p.proc_name, p);
        }
    }

    ProcedureV2 *GetProcedureV2(const std::string &name) {
        auto it = ptable_.find(name);
        if (it == ptable_.end()) return nullptr;
        return &it->second;
    }
};

static ProcedureTableV2 global_ptable_v2;
}  // namespace cypher
