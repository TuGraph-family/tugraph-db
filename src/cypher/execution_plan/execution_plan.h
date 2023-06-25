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
// Created by wt on 6/12/18.
//
#pragma once

#include <vector>
#include <unordered_map>
#include "cypher/execution_plan/ops/op.h"
#include "cypher/filter/filter.h"
#include "cypher/graph/graph.h"
#include "cypher/parser/clause.h"
#include "lgraph/lgraph.h"

namespace lgraph {
class StateMachine;
}

namespace cypher {

// key为pattern graph中的点id,value为label值
typedef std::map<NodeID, std::string> SchemaNodeMap;
// key为pattern graph中的边id,value为(源点id,终点id,边label值,边方向)的四元组
typedef std::map<RelpID, std::tuple<NodeID, NodeID, std::set<std::string>, parser::LinkDirection>>
    SchemaRelpMap;
// typedef std::map<RelpID,std::tuple<NodeID,NodeID,std::set<std::string>,LinkDirection>>
// SchemaRelpMap;
typedef std::pair<SchemaNodeMap, SchemaRelpMap> SchemaGraphMap;

class ExecutionPlan {
    // global member
    parser::CmdType _cmd_type = parser::CmdType::QUERY;
    bool _read_only = true;
    OpBase *_root = nullptr;
    // resultset of CURRENT part being built, todo: alloc for every part
    ResultInfo _result_info;
    // query parts local member
    std::vector<PatternGraph> _pattern_graphs;
    lgraph::SchemaInfo *_schema_info = nullptr;

    void _AddScanOp(const parser::QueryPart &part, const SymbolTable *sym_tab, Node *node,
                    std::vector<OpBase *> &ops, bool skip_arg_op);

    void _BuildArgument(const parser::QueryPart &part, PatternGraph &pattern_graph, OpBase *&root);

    void _BuildStandaloneCallOp(const parser::QueryPart &part, const PatternGraph &pattern_graph,
                                OpBase *&root);

    void _BuildExpandOps(const parser::QueryPart &part, PatternGraph &pattern_graph, OpBase *&root);

    void _BuildUnwindOp(const parser::QueryPart &part, const PatternGraph &pattern_graph,
                        OpBase *&root);

    void _BuildInQueryCallOp(const parser::QueryPart &part, const PatternGraph &pattern_graph,
                             OpBase *&root);

    void _BuildCreateOp(const parser::QueryPart &part, PatternGraph &pattern_graph, OpBase *&root);

    void _BuildMergeOp(const parser::QueryPart &part, PatternGraph &pattern_graph, OpBase *&root);

    void _BuildDeleteOp(const parser::QueryPart &part, PatternGraph &pattern_graph, OpBase *&root);

    void _BuildSetOp(const parser::QueryPart &part, PatternGraph &pattern_graph, OpBase *&root);

    void _BuildRemoveOp(const parser::QueryPart &part, PatternGraph &pattern_graph, OpBase *&root);

    void _BuildReturnOps(const parser::QueryPart &part, const PatternGraph &pattern_graph,
                         OpBase *&root);

    void _BuildWithOps(const parser::QueryPart &part, const PatternGraph &pattern_graph,
                       OpBase *&root);

    void _BuildClause(const parser::Clause &clause, const parser::QueryPart &part,
                      PatternGraph &pattern_graph, OpBase *&root);

    void _PlaceFilterOps(const parser::QueryPart &part, OpBase *&root);

    void _PlaceFilter(std::shared_ptr<lgraph::Filter> f, OpBase *&root);

    bool _PlaceFilterToNode(std::shared_ptr<lgraph::Filter> &f, OpBase *node);

    void _MergeFilter(OpBase *&root);

    bool _WorkWithoutTransaction(const parser::SglQuery &stmt) const;

    DISABLE_COPY(ExecutionPlan);
    DISABLE_MOVE(ExecutionPlan);

 public:
    ExecutionPlan() = default;

    ExecutionPlan &Clone(const ExecutionPlan &rhs);

    ~ExecutionPlan();

    OpBase *BuildPart(const parser::QueryPart &part, int part_id);

    OpBase *BuildSgl(const parser::SglQuery &stmt, size_t parts_offset);

    void Build(const std::vector<parser::SglQuery> &stmt, parser::CmdType cmd);

    void Validate(cypher::RTContext *ctx);

    void Reset();

    const ResultInfo &GetResultInfo() const;

    void SetSchemaInfo(lgraph::SchemaInfo *schema_info) { _schema_info = schema_info; }

    OpBase *Root() { return _root; }

    bool ReadOnly() const { return _read_only; }

    parser::CmdType CommandType() const { return _cmd_type; }

    int Execute(RTContext *ctx);

    std::string DumpPlan(int indent, bool statistics) const;

    std::string DumpGraph() const;  // dump pattern graph
};
}  // namespace cypher
