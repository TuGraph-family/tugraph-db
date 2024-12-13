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
#include "cypher/execution_plan/runtime_context.h"

namespace cypher {

static const int SPEC_MEMBER_SIZE = 3;

typedef std::vector<Entry> VEC_EXPR;
typedef std::vector<std::string> VEC_STR;
// TODO(anyone) procedure context

enum class ProcedureResultType : char {
    Value = 0,
    Node,
    Relationship
};

struct ProcedureResult {
    std::any data;
    ProcedureResultType type;
    explicit ProcedureResult(Value d)
        : data(std::move(d)), type(ProcedureResultType::Value) {}
    explicit ProcedureResult(graphdb::Vertex d)
        : data(std::move(d)), type(ProcedureResultType::Node) {}
    Value AsValue() {
        if (type != ProcedureResultType::Value) THROW_CODE(UnknownError, "AsValue, but data is not Value");
        return std::any_cast<const Value&>(data);
    }
    graphdb::Vertex AsNode() {
        if (type != ProcedureResultType::Node) THROW_CODE(UnknownError, "AsNode, but data is not Node");
        return std::any_cast<const graphdb::Vertex&>(data);
    }
};

class AlgoFunc {
   public:
    static void PageRank(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                               const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
};

class LogFunc {
   public:
    static void QueryLog(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                         const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void SetLevel(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                         const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
};

class BuiltinProcedure {
public:
    static void DbmsProcedures(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                               const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void DbLabels(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                               const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void DbRelationshipTypes(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                         const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void DbCreateUniquePropertyConstraint(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                    const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void DbDeleteUniquePropertyConstraint(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                                 const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void DbIndexFullTextCreateNodeIndex(
        cypher::RTContext *ctx, const cypher::Record *record,
        const cypher::VEC_EXPR &args, const cypher::VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void DbIndexFullTextQueryNodes(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                             const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void DbIndexFullTextDeleteIndex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                          const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void DbIndexFullTextApplyWal(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                           const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void DbIndexVectorCreateNodeIndex(
        cypher::RTContext *ctx, const cypher::Record *record,
        const cypher::VEC_EXPR &args, const cypher::VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void DbIndexVectorKnnSearchNodes(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                          const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void DbIndexVectorApplyWal(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                        const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void DbIndexVectorDeleteIndex(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                           const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void DbmsGraphCreateGraph(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                          const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void DbmsGraphDeleteGraph(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                     const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void DbmsGraphListGraph(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                                     const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void DbDropDB(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                         const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
    static void DbShowIndexes(RTContext *ctx, const Record *record, const VEC_EXPR &args,
                         const VEC_STR &yield_items, std::vector<std::vector<ProcedureResult>> *records);
};

typedef std::function<void(RTContext *, const Record *, const VEC_EXPR &, const VEC_STR &,
                           std::vector<std::vector<ProcedureResult>> *records)>
    SA_FUNC;

struct Parameter {
    std::string name;  ///> name of the parameter
    int index;         ///> index of the parameter list in which the parameter stay
    ProcedureResultType type;   ///> type of the parameter
};

struct SigSpec {
    std::vector<Parameter> input_list;   ///> input parameter list
    std::vector<Parameter> result_list;  ///> return parameter list
};

struct Procedure {
    /* <name, <index, type>> */
    typedef std::vector<std::pair<std::string, std::pair<int, ProcedureResultType>>> SIG_SPEC;
    typedef SA_FUNC FUNC;
    std::string proc_name;
    SigSpec signature;
    FUNC function;

    Procedure(std::string name, FUNC func, const SIG_SPEC &args, const SIG_SPEC &results)
        : proc_name(std::move(name)), function(std::move(func)) {
        std::vector<Parameter> input_list;
        std::transform(args.cbegin(), args.cend(), std::back_inserter(input_list),
                       [](auto arg) -> Parameter {
                           return {
                               .name = arg.first,
                               .index = arg.second.first,
                               .type = arg.second.second,
                           };
                       });

        std::vector<Parameter> yield_items;
        std::transform(results.cbegin(), results.cend(), std::back_inserter(yield_items),
                       [](auto result) -> Parameter {
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

    Procedure(std::string name, FUNC func)
        : proc_name(std::move(name)), function(std::move(func)) {
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

    std::string Signature() const;
};

class ProcedureTable {
    std::unordered_map<std::string, Procedure> ptable_;

 public:
    ProcedureTable() { RegisterStandaloneFuncs(); }

    void RegisterStandaloneFuncs();

    Procedure *GetProcedure(const std::string &name) {
        auto it = ptable_.find(name);
        if (it == ptable_.end()) return nullptr;
        return &it->second;
    }
};

extern ProcedureTable global_ptable;

}  // namespace cypher
