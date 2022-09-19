/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

//
// Created by wt on 19-1-10.
//
#pragma once

#include "parser/data_typedef.h"
#include "execution_plan/runtime_context.h"
#include "graph/graph.h"
#include <iostream>

namespace cypher {

/* argument and return value type */
static const std::map<lgraph::ResultElementType, std::string> ResultTypeNames = {
    {lgraph::ResultElementType::NUL, "NUL"},
    {lgraph::ResultElementType::INTEGER, "INTEGER"},
    {lgraph::ResultElementType::FLOAT, "FLOAT"},
    {lgraph::ResultElementType::DOUBLE, "DOUBLE"},
    {lgraph::ResultElementType::BOOLEAN, "BOOLEAN"},
    {lgraph::ResultElementType::STRING, "STRING"},
    {lgraph::ResultElementType::NODE, "NODE"},
    {lgraph::ResultElementType::RELATIONSHIP, "RELATIONSHIP"},
    {lgraph::ResultElementType::PATH, "PATH"},
    {lgraph::ResultElementType::LIST, "LIST"},
    {lgraph::ResultElementType::MAP, "MAP"},
    {lgraph::ResultElementType::FIELD, "FIELD"},
    {lgraph::ResultElementType::GRAPH_ELEMENT, "GRAPH_ELEMENT"},
    {lgraph::ResultElementType::COLLECTION, "COLLECTION"},
    {lgraph::ResultElementType::ANY, "ANY"}};

static const std::unordered_map<std::string, lgraph::AccessLevel> ValidAccessLevels = {
    {"NONE", lgraph::AccessLevel::NONE},
    {"READ", lgraph::AccessLevel::READ},
    {"WRITE", lgraph::AccessLevel::WRITE},
    {"FULL", lgraph::AccessLevel::FULL}};

static const std::unordered_map<std::string, lgraph::FieldAccessLevel> ValidFieldAccessLevels = {
    {"NONE", lgraph::FieldAccessLevel::NONE},
    {"READ", lgraph::FieldAccessLevel::READ},
    {"WRITE", lgraph::FieldAccessLevel::WRITE}
};

static const std::unordered_map<std::string, lgraph::PluginManager::PluginType> ValidPluginType = {
    {"CPP", lgraph::PluginManager::PluginType::CPP},
    {"PY", lgraph::PluginManager::PluginType::PYTHON}};

static const std::unordered_map<std::string, lgraph::plugin::CodeType> ValidPluginCodeType = {
    {"PY", lgraph::plugin::CodeType::PY},
    {"SO", lgraph::plugin::CodeType::SO},
    {"CPP", lgraph::plugin::CodeType::CPP},
    {"ZIP", lgraph::plugin::CodeType::ZIP}};

static const int SPEC_MEMBER_SIZE = 3;

typedef std::vector<parser::Expression> VEC_EXPR;
typedef std::vector<std::string> VEC_STR;
// TODO: procedure context // NOLINT
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

    static void DbCreateVertexLabel(RTContext *ctx, const Record *record, const VEC_EXPR &args,
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

    static void DbmsProcedures(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                               const VEC_STR &yield_items, std::vector<Record> *records);

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

    static void DbmsGraphGetGraphInfo(RTContext *ctx, const Record *record, const VEC_EXPR &args,
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

    static void DbPluginGetPluginInfo(RTContext *ctx, const Record *record,
                                                 const VEC_EXPR &args, const VEC_STR &yield_items,
                                                 std::vector<Record> *records);
    static void DbPluginCallPlugin(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                   const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbImportorDataImportor(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                       const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbImportorSchemaImportor(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                         const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbDeleteIndex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                              const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbDeleteEdgeIndex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                              const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbFlushDB(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                          const VEC_STR &yield_items, std::vector<Record> *records);

    static void DbDropDB(RTContext *ctx, const Record *record, const VEC_EXPR &args,
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
};

struct Procedure {
    /* <name, <index, type>> */
    typedef std::vector<std::pair<std::string, std::pair<int, lgraph::ResultElementType>>> SIG_SPEC;
    typedef SA_FUNC FUNC;
    std::string name;
    FUNC function;
    SIG_SPEC argument;
    SIG_SPEC result;
    bool read_only;
    bool separate_txn;  // do this procedure in separate txn

    Procedure(const std::string &nm, const FUNC &func, const SIG_SPEC &args, const SIG_SPEC &res,
              bool ro = true, bool st = false)
        : name(nm), function(func), argument(args), result(res), read_only(ro), separate_txn(st) {}

    Procedure(const std::string &nm, const FUNC &func, bool ro = true, bool st = false)
        : name(nm), function(func), read_only(ro), separate_txn(st) {}

    bool ContainsYieldItem(const std::string &item) {
        for (auto &r : result) {
            if (r.first == item) {
                return true;
            }
        }
        return false;
    }

    std::string Signature() const {
        std::string s;
        std::map<int, std::string> args, res;
        for (auto &p : argument) {
            std::string str;
            str.append(p.first).append("::").append(ResultTypeNames.at(p.second.second));
            args.emplace(p.second.first, str);
        }
        for (auto &r : result) {
            std::string str;
            str.append(r.first).append("::").append(ResultTypeNames.at(r.second.second));
            res.emplace(r.second.first, str);
        }
        s.append(name).append("(");
        size_t i = 0;
        for (auto &a : args) {
            s.append(a.second);
            if (++i < argument.size()) s.append(",");
        }
        s.append(") :: (");
        i = 0;
        for (auto &r : res) {
            s.append(r.second);
            if (++i < result.size()) s.append(",");
        }
        s.append(")");
        return s;
    }
};

static std::vector<Procedure> global_procedures = {
    Procedure("db.vertexLabels", BuiltinProcedure::DbVertexLabels, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"label", {0, lgraph::ResultElementType::STRING}}}),

    Procedure("db.edgeLabels", BuiltinProcedure::DbEdgeLabels, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"edgeLabels", {0, lgraph::ResultElementType::STRING}}}),

    Procedure("db.indexes", BuiltinProcedure::DbIndexes, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{
                  {"label", {0, lgraph::ResultElementType::STRING}},
                  {"field", {1, lgraph::ResultElementType::STRING}},
                  {"unique", {2, lgraph::ResultElementType::BOOLEAN}},
              }),

    Procedure("db.listLabelIndexes", BuiltinProcedure::DbListLabelIndexes, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{
                  {"label", {0, lgraph::ResultElementType::STRING}},
                  {"field", {1, lgraph::ResultElementType::STRING}},
                  {"unique", {2, lgraph::ResultElementType::BOOLEAN}},
              }),

    Procedure("db.propertyKeys", BuiltinProcedure::DbPropertyKeys, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"propertyKey", {0, lgraph::ResultElementType::STRING}}}),

    Procedure("db.warmup", BuiltinProcedure::DbWarmUp, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"time_used", {0, lgraph::ResultElementType::STRING}}}, true,
              true),

    Procedure("db.createVertexLabel", BuiltinProcedure::DbCreateVertexLabel,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph::ResultElementType::STRING}},
                                  {"field_specs", {1, lgraph::ResultElementType::LIST}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("db.createLabel", BuiltinProcedure::DbCreateLabel,
              Procedure::SIG_SPEC{{"label_type", {0, lgraph::ResultElementType::STRING}},
                                  {"label_name", {1, lgraph::ResultElementType::STRING}},
                                  {"extra", {2, lgraph::ResultElementType::STRING}},
                                  {"field_specs", {3, lgraph::ResultElementType::LIST}}},
              Procedure::SIG_SPEC{}, false, true),

    Procedure("db.getLabelSchema", BuiltinProcedure::DbGetLabelSchema,
              Procedure::SIG_SPEC{{"label_type", {0, lgraph::ResultElementType::STRING}},
                                  {"label_name", {1, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"name", {0, lgraph::ResultElementType::STRING}},
                                  {"type", {1, lgraph::ResultElementType::STRING}},
                                  {"optional", {2, lgraph::ResultElementType::BOOLEAN}}},
              true, false),

    Procedure("db.getVertexSchema", BuiltinProcedure::DbGetVertexSchema,
              Procedure::SIG_SPEC{{"label", {0, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"schema", {0, lgraph::ResultElementType::MAP}}}, true, false),

    Procedure("db.getEdgeSchema", BuiltinProcedure::DbGetEdgeSchema,
              Procedure::SIG_SPEC{{"label", {0, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"schema", {0, lgraph::ResultElementType::MAP}}}, true, false),

    Procedure("db.deleteLabel", BuiltinProcedure::DbDeleteLabel,
              Procedure::SIG_SPEC{{"label_type", {0, lgraph::ResultElementType::STRING}},
                                  {"label_name", {1, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{
                  {"", {0, lgraph::ResultElementType::NUL}},
              },
              false, true),

    Procedure("db.alterLabelDelFields", BuiltinProcedure::DbAlterLabelDelFields,
              Procedure::SIG_SPEC{{"label_type", {0, lgraph::ResultElementType::STRING}},
                                  {"label_name", {1, lgraph::ResultElementType::STRING}},
                                  {"del_fields", {2, lgraph::ResultElementType::LIST}}},
              Procedure::SIG_SPEC{
                  {"record_affected", {0, lgraph::ResultElementType::INTEGER}},
              },
              false, true),

    Procedure("db.alterLabelAddFields", BuiltinProcedure::DbAlterLabelAddFields,
              Procedure::SIG_SPEC{
                  {"label_type", {0, lgraph::ResultElementType::STRING}},
                  {"label_name", {1, lgraph::ResultElementType::STRING}},
                  {"add_field_spec_values", {2, lgraph::ResultElementType::LIST}},
              },
              Procedure::SIG_SPEC{
                  {"record_affected", {0, lgraph::ResultElementType::INTEGER}},
              },
              false, true),

    Procedure("db.alterLabelModFields", BuiltinProcedure::DbAlterLabelModFields,
              Procedure::SIG_SPEC{{"label_type", {0, lgraph::ResultElementType::STRING}},
                                  {"label_name", {1, lgraph::ResultElementType::STRING}},
                                  {"mod_field_specs", {2, lgraph::ResultElementType::LIST}}},
              Procedure::SIG_SPEC{
                  {"record_affected", {0, lgraph::ResultElementType::INTEGER}},
              },
              false, true),

    Procedure("db.createEdgeLabel", BuiltinProcedure::DbCreateEdgeLabel,
              Procedure::SIG_SPEC{{"type_name", {0, lgraph::ResultElementType::STRING}},
                                  {"field_specs", {1, lgraph::ResultElementType::LIST}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("db.addIndex", BuiltinProcedure::DbAddVertexIndex,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph::ResultElementType::STRING}},
                                  {"field_name", {1, lgraph::ResultElementType::STRING}},
                                  {"is_unique",  {2, lgraph::ResultElementType::BOOLEAN}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("db.addEdgeIndex", BuiltinProcedure::DbAddEdgeIndex,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph::ResultElementType::STRING}},
                                  {"field_name", {1, lgraph::ResultElementType::STRING}},
                                  {"is_unique", {2, lgraph::ResultElementType::BOOLEAN}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("db.addFullTextIndex", BuiltinProcedure::DbAddFullTextIndex,
              Procedure::SIG_SPEC{{"is_vertex", {0, lgraph::ResultElementType::BOOLEAN}},
                                  {"label_name", {1, lgraph::ResultElementType::STRING}},
                                  {"field_name", {2, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("db.deleteFullTextIndex", BuiltinProcedure::DbDeleteFullTextIndex,
              Procedure::SIG_SPEC{{"is_vertex", {0, lgraph::ResultElementType::BOOLEAN}},
                                  {"label_name", {1, lgraph::ResultElementType::STRING}},
                                  {"field_name", {2, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("db.rebuildFullTextIndex", BuiltinProcedure::DbRebuildFullTextIndex,
              Procedure::SIG_SPEC{{"vertex_labels", {0, lgraph::ResultElementType::STRING}},
                                  {"edge_labels", {1, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("db.fullTextIndexes", BuiltinProcedure::DbFullTextIndexes, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"is_vertex", {0, lgraph::ResultElementType::BOOLEAN}},
                      {"label", {1, lgraph::ResultElementType::STRING}},
                      {"field", {2, lgraph::ResultElementType::STRING}}}),

    Procedure("dbms.procedures", BuiltinProcedure::DbmsProcedures, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"name", {0, lgraph::ResultElementType::STRING}},
                                  {"signature", {1, lgraph::ResultElementType::STRING}},
                                  {"read_only", {2, lgraph::ResultElementType::BOOLEAN}}}
              , true, false),

    Procedure("dbms.security.changePassword", BuiltinProcedure::DbmsSecurityChangePassword,
              Procedure::SIG_SPEC{
                  {"current_password", {0, lgraph::ResultElementType::STRING}},
                  {"new_password", {1, lgraph::ResultElementType::STRING}},
              },
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.security.changeUserPassword", BuiltinProcedure::DbmsSecurityChangeUserPassword,
              Procedure::SIG_SPEC{
                  {"user_name", {0, lgraph::ResultElementType::STRING}},
                  {"new_password", {1, lgraph::ResultElementType::STRING}},
              },
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.security.createUser", BuiltinProcedure::DbmsSecurityCreateUser,
              Procedure::SIG_SPEC{{"user_name", {0, lgraph::ResultElementType::STRING}},
                                  {"password", {1, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.security.deleteUser", BuiltinProcedure::DbmsSecurityDeleteUser,
              Procedure::SIG_SPEC{{"user_name", {0, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.security.setUserMemoryLimit", BuiltinProcedure::DbmsSecuritySetUserMemoryLimit,
              Procedure::SIG_SPEC{
                  {"user_name", {0, lgraph::ResultElementType::STRING}},
                  {"MemoryLimit", {1, lgraph::ResultElementType::INTEGER}},
              },
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.security.listUsers", BuiltinProcedure::DbmsSecurityListUsers,
              Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{
                  {"user_name", {0, lgraph::ResultElementType::STRING}},
                  {"user_info", {1, lgraph::ResultElementType::MAP}},
              },
              true, true),

    Procedure("dbms.security.showCurrentUser", BuiltinProcedure::DbmsSecurityShowCurrentUser,
              Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"current_user", {0, lgraph::ResultElementType::STRING}}}, true,
              true),

    Procedure("dbms.security.listAllowedHosts", BuiltinProcedure::DbmsSecurityHostWhitelistList,
              Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"host", {0, lgraph::ResultElementType::STRING}}}, true, true),

    Procedure("dbms.security.deleteAllowedHosts", BuiltinProcedure::DbmsSecurityHostWhitelistDelete,
              Procedure::SIG_SPEC{{"hosts", {0, lgraph::ResultElementType::LIST}}},
              Procedure::SIG_SPEC{{"record_affected", {0, lgraph::ResultElementType::INTEGER}}},
              false, true),

    Procedure("dbms.security.addAllowedHosts", BuiltinProcedure::DbmsSecurityHostWhitelistAdd,
              Procedure::SIG_SPEC{{"hosts", {0, lgraph::ResultElementType::LIST}}},
              Procedure::SIG_SPEC{{"num_added", {0, lgraph::ResultElementType::INTEGER}}}, false,
              true),

    Procedure("dbms.graph.createGraph", BuiltinProcedure::DbmsGraphCreateGraph,
              Procedure::SIG_SPEC{{"graph_name", {0, lgraph::ResultElementType::STRING}},
                                  {"description", {1, lgraph::ResultElementType::STRING}},
                                  {"max_size_GB", {2, lgraph::ResultElementType::INTEGER}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.graph.deleteGraph", BuiltinProcedure::DbmsGraphDeleteGraph,
              Procedure::SIG_SPEC{{"graph_name", {0, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.graph.modGraph", BuiltinProcedure::DbmsGraphModGraph,
              Procedure::SIG_SPEC{{"graph_name", {0, lgraph::ResultElementType::STRING}},
                                  {"config", {1, lgraph::ResultElementType::MAP}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.graph.listGraphs", BuiltinProcedure::DbmsGraphListGraphs, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"graph_name", {0, lgraph::ResultElementType::STRING}},
                                  {"configuration", {1, lgraph::ResultElementType::MAP}}},
              true, true),

    Procedure("dbms.graph.getGraphInfo", BuiltinProcedure::DbmsGraphGetGraphInfo,
              Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"graph_name", {0, lgraph::ResultElementType::STRING}},
                                  {"configuration", {1, lgraph::ResultElementType::MAP}}},
              true, true),

    Procedure("dbms.config.list", BuiltinProcedure::DbmsConfigList, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"name", {0, lgraph::ResultElementType::STRING}},
                                  {"value", {1, lgraph::ResultElementType::FIELD}}},
              true, true),

    Procedure("dbms.config.update", BuiltinProcedure::DbmsConfigUpdate,
              Procedure::SIG_SPEC{{"updates", {0, lgraph::ResultElementType::MAP}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.takeSnapshot", BuiltinProcedure::DbmsTakeSnapshot, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"path", {0, lgraph::ResultElementType::STRING}}}, false),

    Procedure("dbms.listBackupFiles", BuiltinProcedure::DbmsListBackupLogFiles,
              Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"file", {0, lgraph::ResultElementType::STRING}}}, true),
    // algo
    Procedure("algo.shortestPath", AlgoFunc::ShortestPath,
              Procedure::SIG_SPEC{
                  {"startNode", {0, lgraph::ResultElementType::NODE}},
                  {"endNode", {1, lgraph::ResultElementType::NODE}},
                  {"config", {2, lgraph::ResultElementType::MAP}},
              },
              Procedure::SIG_SPEC{
                  {"nodeCount", {0, lgraph::ResultElementType::INTEGER}},
                  {"totalCost", {1, lgraph::ResultElementType::FLOAT}},
                  {"path", {2, lgraph::ResultElementType::STRING}},
              }),

    Procedure("algo.allShortestPaths", AlgoFunc::AllShortestPaths,
              Procedure::SIG_SPEC{
                  {"startNode", {0, lgraph::ResultElementType::NODE}},
                  {"endNode", {1, lgraph::ResultElementType::NODE}},
                  {"config", {2, lgraph::ResultElementType::MAP}},
              },
              Procedure::SIG_SPEC{
                  {"nodeIds", {0, lgraph::ResultElementType::LIST}},
                  {"relationshipIds", {1, lgraph::ResultElementType::LIST}},
                  {"cost", {2, lgraph::ResultElementType::LIST}},
              }),

    Procedure("algo.native.extract", AlgoFunc::NativeExtract,
              Procedure::SIG_SPEC{
                  {"id", {0, lgraph::ResultElementType::ANY}},
                  {"config", {1, lgraph::ResultElementType::MAP}},
              },
              Procedure::SIG_SPEC{
                  {"value", {0, lgraph::ResultElementType::ANY}},
              }),

    Procedure("algo.pagerank", AlgoFunc::PageRank,
              Procedure::SIG_SPEC{
                  {"num_iterations", {0, lgraph::ResultElementType::INTEGER}},
              },
              Procedure::SIG_SPEC{{"node", {0, lgraph::ResultElementType::NODE}},
                                  {"pr", {1, lgraph::ResultElementType::FLOAT}}}),

    Procedure("dbms.security.listRoles", BuiltinProcedure::DbmsSecurityListRoles,
              Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{
                  {"role_name", {0, lgraph::ResultElementType::STRING}},
                  {"role_info", {1, lgraph::ResultElementType::MAP}},
              },
              true, true),

    Procedure("dbms.security.createRole", BuiltinProcedure::DbmsSecurityCreateRole,
              Procedure::SIG_SPEC{{"role_name", {0, lgraph::ResultElementType::STRING}},
                                  {"desc", {1, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.security.deleteRole", BuiltinProcedure::DbmsSecurityDeleteRole,
              Procedure::SIG_SPEC{{"role_name", {0, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.security.getUserInfo", BuiltinProcedure::DbmsSecurityGetUserInfo,
              Procedure::SIG_SPEC{{"user", {0, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"user_info", {0, lgraph::ResultElementType::MAP}}}, true, true),

    Procedure("dbms.security.getUserMemoryUsage", BuiltinProcedure::DbmsSecurityGetUserMemoryUsage,
              Procedure::SIG_SPEC{{"user", {0, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"memory_usage", {0, lgraph::ResultElementType::INTEGER}}}, true,
              true),

    Procedure("dbms.security.getUserPermissions", BuiltinProcedure::DbmsSecurityGetUserPermissions,
              Procedure::SIG_SPEC{{"user", {0, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"user_info", {0, lgraph::ResultElementType::MAP}}}, true, true),

    Procedure("dbms.security.getRoleInfo", BuiltinProcedure::DbmsSecurityGetRoleInfo,
              Procedure::SIG_SPEC{{"role", {0, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"role_info", {0, lgraph::ResultElementType::MAP}}}, true, true),

    Procedure("dbms.security.disableRole", BuiltinProcedure::DbmsSecurityDisableRole,
              Procedure::SIG_SPEC{{"role", {0, lgraph::ResultElementType::STRING}},
                                  {"disable", {1, lgraph::ResultElementType::BOOLEAN}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.security.modRoleDesc", BuiltinProcedure::DbmsSecurityModRoleDesc,
              Procedure::SIG_SPEC{{"role", {0, lgraph::ResultElementType::STRING}},
                                  {"description", {1, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.security.rebuildRoleAccessLevel",
              BuiltinProcedure::DbmsSecurityRebuildRoleAccessLevel,
              Procedure::SIG_SPEC{{"role", {0, lgraph::ResultElementType::STRING}},
                                  {"access_level", {1, lgraph::ResultElementType::MAP}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.security.modRoleAccessLevel", BuiltinProcedure::DbmsSecurityModRoleAccessLevel,
              Procedure::SIG_SPEC{{"role", {0, lgraph::ResultElementType::STRING}},
                                  {"access_level", {1, lgraph::ResultElementType::MAP}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.security.modRoleFieldAccessLevel", BuiltinProcedure::DbmsSecurityModRoleFieldAccessLevel,
              Procedure::SIG_SPEC{
                  {"role", {0, lgraph::ResultElementType::STRING}},
                  {"graph", {0, lgraph::ResultElementType::STRING}},
                  {"label", {0, lgraph::ResultElementType::STRING}},
                  {"field", {0, lgraph::ResultElementType::STRING}},
                  {"label_type", {0, lgraph::ResultElementType::STRING}},
                  {"field_access_level", {0, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.security.disableUser", BuiltinProcedure::DbmsSecurityDisableUser,
              Procedure::SIG_SPEC{{"user", {0, lgraph::ResultElementType::STRING}},
                                  {"disable", {1, lgraph::ResultElementType::BOOLEAN}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.security.setCurrentDesc", BuiltinProcedure::DbmsSecuritySetCurrentDesc,
              Procedure::SIG_SPEC{{"description", {0, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.security.setUserDesc", BuiltinProcedure::DbmsSecuritySetUserDesc,
              Procedure::SIG_SPEC{{"user", {0, lgraph::ResultElementType::STRING}},
                                  {"description", {1, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.security.deleteUserRoles", BuiltinProcedure::DbmsSecurityDeleteUserRoles,
              Procedure::SIG_SPEC{{"user", {0, lgraph::ResultElementType::STRING}},
                                  {"roles", {1, lgraph::ResultElementType::LIST}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.security.rebuildUserRoles", BuiltinProcedure::DbmsSecurityRebuildUserRoles,
              Procedure::SIG_SPEC{{"user", {0, lgraph::ResultElementType::STRING}},
                                  {"roles", {1, lgraph::ResultElementType::LIST}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.security.addUserRoles", BuiltinProcedure::DbmsSecurityAddUserRoles,
              Procedure::SIG_SPEC{{"user", {0, lgraph::ResultElementType::STRING}},
                                  {"roles", {1, lgraph::ResultElementType::LIST}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("db.plugin.loadPlugin", BuiltinProcedure::DbPluginLoadPlugin,
              Procedure::SIG_SPEC{{"plugin_type", {0, lgraph::ResultElementType::STRING}},
                                  {"plugin_name", {1, lgraph::ResultElementType::STRING}},
                                  {"plugin_content", {2, lgraph::ResultElementType::STRING}},
                                  {"code_type", {3, lgraph::ResultElementType::STRING}},
                                  {"plugin_description", {4, lgraph::ResultElementType::STRING}},
                                  {"read_only", {5, lgraph::ResultElementType::BOOLEAN}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("db.plugin.deletePlugin", BuiltinProcedure::DbPluginDeletePlugin,
              Procedure::SIG_SPEC{{"plugin_type", {0, lgraph::ResultElementType::STRING}},
                                  {"plugin_name", {1, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("db.plugin.getPluginInfo", BuiltinProcedure::DbPluginGetPluginInfo,
              Procedure::SIG_SPEC{{"plugin_type", {0, lgraph::ResultElementType::STRING}},
                                  {"plugin_name", {1, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"plugin_description", {0, lgraph::ResultElementType::MAP}}}, false, true),

    Procedure("db.plugin.listPlugin", BuiltinProcedure::DbPluginListPlugin,
              Procedure::SIG_SPEC{{"plugin_type", {0, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"plugin_description", {0, lgraph::ResultElementType::MAP}}},
              false, true),

    Procedure("db.plugin.listUserPlugins", BuiltinProcedure::DbPluginListUserPlugins, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"graph", {0, lgraph::ResultElementType::STRING}},
                                  {"plugins", {1, lgraph::ResultElementType::MAP}}
              }, true, true),

    Procedure("db.plugin.callPlugin", BuiltinProcedure::DbPluginCallPlugin,
              Procedure::SIG_SPEC{{"plugin_type", {0, lgraph::ResultElementType::STRING}},
                                  {"plugin_name", {1, lgraph::ResultElementType::STRING}},
                                  {"param", {2, lgraph::ResultElementType::STRING}},
                                  {"timeout", {3, lgraph::ResultElementType::DOUBLE}},
                                  {"in_process", {4, lgraph::ResultElementType::BOOLEAN}}},
              Procedure::SIG_SPEC{{"result", {0, lgraph::ResultElementType::STRING}}}, false, true),

    Procedure("db.importor.dataImportor", BuiltinProcedure::DbImportorDataImportor,
              Procedure::SIG_SPEC{{"description", {0, lgraph::ResultElementType::STRING}},
                                  {"content", {1, lgraph::ResultElementType::STRING}},
                                  {"continue_on_error", {2, lgraph::ResultElementType::BOOLEAN}},
                                  {"thread_nums", {3, lgraph::ResultElementType::INTEGER}},
                                  {"delimiter", {4, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("db.importor.schemaImportor", BuiltinProcedure::DbImportorSchemaImportor,
              Procedure::SIG_SPEC{{"description", {0, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("db.deleteIndex", BuiltinProcedure::DbDeleteIndex,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph::ResultElementType::STRING}},
                                  {"field_name", {1, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("db.deleteEdgeIndex", BuiltinProcedure::DbDeleteEdgeIndex,
              Procedure::SIG_SPEC{{"label_name", {0, lgraph::ResultElementType::STRING}},
                                  {"field_name", {1, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("db.flushDB", BuiltinProcedure::DbFlushDB, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("db.dropDB", BuiltinProcedure::DbDropDB, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("dbms.task.listTasks", BuiltinProcedure::DbTaskListTasks, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{
                  {"tasks_info", {1, lgraph::ResultElementType::MAP}},
              },true, true),

    Procedure("dbms.task.terminateTask", BuiltinProcedure::DbTaskTerminateTask,
              Procedure::SIG_SPEC{{"task_id", {0, lgraph::ResultElementType::STRING}}},
              Procedure::SIG_SPEC{{"", {0, lgraph::ResultElementType::NUL}}}, false, true),

    Procedure("db.monitor.tuGraphInfo", BuiltinProcedure::DbMonitorTuGraphInfo, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"request", {0, lgraph::ResultElementType::STRING}}}, false, true),

    Procedure("db.monitor.serverInfo", BuiltinProcedure::DbMonitorServerInfo, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"cpu", {0, lgraph::ResultElementType::STRING}},
                                  {"memory", {1, lgraph::ResultElementType::STRING}},
                                  {"disk_rate", {2, lgraph::ResultElementType::STRING}},
                                  {"disk_storage", {3, lgraph::ResultElementType::STRING}}
              },false, true),

    Procedure("dbms.ha.clusterInfo", BuiltinProcedure::DbmsHaClusterInfo, Procedure::SIG_SPEC{},
              Procedure::SIG_SPEC{{"cluster_info", {0, lgraph::ResultElementType::LIST}}
              },  true, true)
};

class ProcedureTable {
    std::unordered_map<std::string, Procedure> ptable_;

 public:
    ProcedureTable() { RegisterStandaloneFuncs(); }

    void RegisterStandaloneFuncs() {
        for (auto &p : global_procedures) {
            ptable_.emplace(p.name, p);
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
